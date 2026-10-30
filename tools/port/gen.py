#!/usr/bin/env python3
"""Generate the C2 headers and sources from the C1 ones (hook mode).

For every file of the C1 tree (include/nwindow/*.h, src/nwindow/*.cpp) writes the C2 version into
this tree. The C1 files are never changed by hand here: they are the input.

Headers (line based; only the lines the maps touch are rewritten):
  * NW_DECLARE_CLASS: the C2 UClass address;
  * fields: `// +0xOFF` moved by db/layout.json, `BYTE _UnknownXXX[n]` fillers for the bytes C2
    added, a trailing filler up to the C2 sizeof; NW_ASSERT_OFFSET / NW_ASSERT_SIZE follow;
  * methods: `// <slot> 0xVA` becomes the C2 slot (db/vslots.json) and address; a virtual that exists
    only in C2 gets an `NewVirtual<slot>()` placeholder at its place in the order.
Sources (units = everything up to an NW_HOOK_* line):
  * a unit whose function is eligible (db/eligible.csv: category A/B, every displacement explained
    by the layout, no field without evidence, no construction of a class whose constructor is not
    ported) is copied with its address, vslot comment and NW_HOOK_* address rewritten;
  * every other matched unit becomes a stub: `NW_NAKED <signature> NW_STOCK(c2 va)` (a jump into the
    stock function) or, for a constructor, a body that breaks: the class still needs its
    constructor defined so the compiled vtable exists;
  * NW_CHECK_VTABLE lines get the C2 vtable address and slot count.
Writes db/gen_report.csv: one row per unit (file, class, name, c1_va, c2_va, action, reason).

usage: python tools/port/gen.py [--only NCButton.h,NCButton.cpp ...]"""
from __future__ import annotations

import argparse
import collections
import csv
import json
import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import hdr  # noqa: E402
from common import ROOT, C1_ROOT, read_csv  # noqa: E402
from layout import load_classes  # noqa: E402

HOOK = re.compile(r"^NW_HOOK_(\w+)\(\s*(0x[0-9A-Fa-f]{8})\s*,\s*([^\n]*)$")
ADDR_COMMENT = re.compile(r"^//\s*0x([0-9a-fA-F]{8})\b")
CHECK_VT = re.compile(r"^(NW_CHECK_VTABLE(?:_FOR)?)\(\s*(\w+)\s*,(\s*\w+\s*,)?\s*(0x[0-9A-Fa-f]{8})\s*,\s*(\d+)\s*\)")
GLOBAL_DEF = re.compile(r"#define\s+(G\w+)\s+NW_GLOBAL\(.*?,\s*(0x[0-9A-Fa-f]{8})\s*\)")
IDENT = re.compile(r"[A-Za-z_]\w*")
BYVALUE = re.compile(r"[(,]\s*(?:const\s+)?(?:FString|NString|FName|NStlWString|NStdString|TArray<[^>]*>)\s+\w+")


class Data:
    def __init__(self):
        self.c1 = load_classes(C1_ROOT)
        self.c2 = load_classes(ROOT)
        self.layout = json.load(open(ROOT / "db" / "layout.json", encoding="utf-8"))
        self.vslots = json.load(open(ROOT / "db" / "vslots.json", encoding="utf-8"))
        self.fmap = {}
        for r in csv.DictReader((ROOT / "db" / "fmap.csv").open(encoding="utf-8")):
            self.fmap[int(r["c1_va"], 16)] = r
        self.elig = {}
        for r in csv.DictReader((ROOT / "db" / "eligible.csv").open(encoding="utf-8")):
            self.elig[int(r["c1_va"], 16)] = r
        self.c2kind = {}
        fp = ROOT / "db" / "functions.csv"
        if fp.exists():
            for r in csv.DictReader(fp.open(encoding="utf-8")):
                self.c2kind[int(r["va"], 16)] = r["kind"]
        self.amap = {}
        self.code_amap = {}
        for r in csv.DictReader((ROOT / "db" / "amap.csv").open(encoding="utf-8")):
            self.amap[int(r["c1"], 16)] = int(r["c2"], 16)
            if r["kind"] == "code" and float(r["share"]) >= 0.9:
                self.code_amap[int(r["c1"], 16)] = int(r["c2"], 16)
        # chain of derived (no evidence) field names per class, and the global names without a map
        self.derived = collections.defaultdict(set)
        self.derived_any = set()
        for n, c in self.layout.items():
            for r in c["fields"]:
                if r["status"].startswith("derived") or r["c2"] is None:
                    self.derived[n].add(r["name"])
                    self.derived_any.add(r["name"])
        self.bad_globals = set()
        for p in sorted((C1_ROOT / "include").rglob("*.h")):
            for m in GLOBAL_DEF.finditer(p.read_text(encoding="utf-8", errors="replace")):
                if int(m.group(2), 16) not in self.amap:
                    self.bad_globals.add(m.group(1))
        # engine-parent slot counts (the engine classes are not in classes.json)
        self.engine_n1, self.engine_n2 = engine_counts(C1_ROOT), engine_counts(ROOT)
        self.ctor_chain_bad = {}
        self.cand_mode = False
        self.cand_min_sim = 0.6
        self.cand_set = set()
        self.demoted = {}
        dp = ROOT / "db" / "demote.csv"
        if dp.exists():
            for r in csv.DictReader(dp.open(encoding="utf-8")):
                self.demoted[r["c2_va"]] = r["reason"]
        self.bad_helpers = {k[7:] for k in self.demoted if k.startswith("helper:")}

    def map_literal(self, va):
        r = self.fmap.get(va)
        if r:
            return int(r["c2_va"], 16)
        return self.amap.get(va)

    def chain_derived(self, cls):
        out = set()
        while cls in self.c1:
            out |= self.derived.get(cls, set())
            cls = self.c1[cls]["super"]
        return out

    def parent_slots(self, cls):
        """(n1, n2) vtable slot counts of the parent of cls."""
        p = self.c1[cls]["super"]
        if p in self.c1 and p in self.c2:
            return len(self.c1[p].get("vtable_slots", [])), len(self.c2[p].get("vtable_slots", []))
        return self.engine_n1.get(p), self.engine_n2.get(p)


def engine_counts(root: Path):
    out = {}
    p = root / "analysis" / "engine_vtables.txt"
    if not p.exists():
        return out
    for l in p.open(encoding="utf-8", errors="replace"):
        m = re.match(r"== \w+ \?\?_7(\w+)@@6B\w*@+ \((\d+) slots\)", l)
        if m and m.group(1) not in out:
            out[m.group(1)] = int(m.group(2))
    return out


def iv(x):
    """classes.json holds addresses as '0x...' strings (or ints in a few records)."""
    return x if isinstance(x, int) else int(x, 16)


def hexs(v, w=0):
    return "0x%0*X" % (w, v)


# ----------------------------------------------------------------------------------------------
# headers
# ----------------------------------------------------------------------------------------------
def gen_header(D: Data, path: Path, report):
    lines, cds = hdr.parse(path)
    edits = {}      # line index -> replacement text (may contain several lines)
    before = collections.defaultdict(list)   # line index -> lines inserted before it
    after = collections.defaultdict(list)    # line index -> lines inserted after it

    for cd in cds:
        n = cd.name
        if n not in D.c1 or n not in D.c2:
            continue
        c1, c2 = D.c1[n], D.c2[n]
        lay = D.layout.get(n)
        vs = D.vslots.get(n)
        if cd.decl_line >= 0:
            lines_decl = lines[cd.decl_line]
            edits[cd.decl_line] = re.sub(r"(0x[0-9A-Fa-f]{8})(\s*\))", lambda m: hexs(iv(c2["static_class"]), 8) + m.group(2),
                                         lines_decl, count=1)
        # --- fields
        newoff = {}
        if lay:
            fl = sorted(cd.fields, key=lambda f: f.off)
            recs = {(r["name"], r["c1"]): r for r in lay["fields"]}
            parent = c1["super"]
            prev_end = D.c2[parent]["size"] if parent in D.c2 else None
            last_line = None
            items = []
            chk = prev_end
            for f in fl:
                r = recs.get((f.name, f.off))
                if r is not None and r["c2"] is None:
                    # no place in C2 (the class was rewritten, or the field is gone): not declared
                    edits[f.line] = "%s// C2: not placed (no evidence for where it went): %s" % (f.indent, lines[f.line].strip())
                    D.derived[n].add(f.name)
                    D.derived_any.add(f.name)
                    continue
                if r is None:
                    continue
                if chk is not None and r["c2"] < chk and r["status"].startswith("derived"):
                    # no evidence for it and its derived place is taken: the field is gone in C2
                    edits[f.line] = "%s// C2: field gone or unplaced (derived +0x%X is inside the previous field): %s" % (
                        f.indent, r["c2"], lines[f.line].strip())
                    D.derived[n].add(f.name)
                    D.derived_any.add(f.name)
                    continue
                items.append((f, r))
                chk = r["c2"] + min(r["ext"], 4)
            # the compiler lays fields out in declaration order: declare them in the C2 order, in the
            # slots (lines) the C1 declarations had
            slots = [f.line for f, _ in items]
            items.sort(key=lambda it: it[1]["c2"])
            for k, (f, r) in enumerate(items):
                line = slots[k]
                off2 = r["c2"]
                nxt = items[k + 1][1]["c2"] if k + 1 < len(items) else c2["size"]
                ext2 = nxt - off2
                if f.name.startswith("_Unknown") and not f.tail and " - " in f.arr:
                    # a C1 pad: the same bytes in C2 are whatever lies between the neighbours
                    edits[line] = ("%sBYTE _Unknown%X[0x%X];  // +0x%X (a C1 pad, its C2 extent)" % (f.indent, off2, ext2, off2)
                                   if ext2 > 0 else "%s// C2: pad gone: %s" % (f.indent, lines[f.line].strip()))
                    ext_used = max(ext2, 0)
                elif ext2 < r["ext"] and r["ext"] >= 8 and ext2 > 0:
                    # an array or struct that is smaller in C2 (TCHAR[128] became an FString): the C1 type no
                    # longer fits, so the bytes are a filler and the name is unusable in copied bodies
                    edits[line] = "%sBYTE _C2Retyped_%s[0x%X];  // +0x%X C2: %s no longer fits (C1: %s)" % (
                        f.indent, f.name, ext2, off2, f.name, lines[f.line].strip().split("//")[0].strip())
                    D.derived[n].add(f.name)
                    D.derived_any.add(f.name)
                    ext_used = ext2
                else:
                    ext_used = r["ext"]
                    newoff[f.name] = off2
                    text = lines[f.line]
                    text = re.sub(r"\+0x[0-9A-Fa-f]+", "+" + hexs(off2).replace("0X", "0x").upper().replace("0X", "0x"), text, count=1)
                    if r["status"].startswith("derived"):
                        text = text.rstrip() + "   // C2: offset derived, no evidence"
                    edits[line] = text
                if prev_end is not None and off2 > prev_end:
                    before[line].append("%s// +0x%X (new in C2)" % (f.indent + "BYTE _Unknown%X[0x%X - 0x%X]; " % (prev_end, off2, prev_end), prev_end))
                prev_end = off2 + ext_used
                last_line = line
            if last_line is None and prev_end is not None and c2["size"] > prev_end and cd.decl_line >= 0:
                after[cd.decl_line].append("    BYTE _UnknownTail%X[0x%X - 0x%X];  // +0x%X (new in C2, up to sizeof)" % (
                    prev_end, c2["size"], prev_end, prev_end))
            elif last_line is not None and prev_end is not None and c2["size"] > prev_end:
                after[last_line].append("    BYTE _UnknownTail%X[0x%X - 0x%X];  // +0x%X (new in C2, up to sizeof)" % (
                    prev_end, c2["size"], prev_end, prev_end))
        # --- methods
        nvirt = []
        n1p, n2p = D.parent_slots(n)
        vlines = []
        for m in cd.methods:
            t = lines[m.line]
            if m.kind == "virtual" and vs and m.slot is not None:
                s2 = vs["map"].get(str(m.slot))
                if s2 is None:
                    edits[m.line] = re.sub(r"//\s*\d+\s+0x[0-9a-f]{8}", "// C2: slot gone (was %d)" % m.slot, t, count=1)
                    continue
                slots = c2.get("vtable_slots", [])
                va2 = slots[s2]["va"] if s2 < len(slots) and not slots[s2].get("import") else None
                new = "%d 0x%08x" % (s2, va2) if va2 else "%d" % s2
                edits[m.line] = re.sub(r"//\s*\d+\s+0x[0-9a-f]{8}", "// " + new, t, count=1)
                if n2p is not None and s2 >= n2p:
                    vlines.append((s2, m.line))
            elif m.kind in ("method", "ctor", "dtor"):
                va2 = None
                if m.kind == "ctor" and "(" in t and re.search(r"\(\s*\)", t.split("//")[0]) and c2.get("ctor"):
                    va2 = iv(c2["ctor"])
                else:
                    r = D.fmap.get(m.va)
                    if r:
                        va2 = int(r["c2_va"], 16)
                if va2:
                    edits[m.line] = re.sub(r"0x[0-9a-f]{8}", "0x%08x" % va2, t, count=1)
                else:
                    edits[m.line] = re.sub(r"(0x[0-9a-f]{8})", r"\1 (C1 only: no C2 match)", t, count=1)
        # --- placeholders for virtuals only C2 has, inside this class's own slot range
        if vs and n2p is not None:
            own_new = sorted(x["slot"] for x in vs["new"] if x["slot"] >= n2p)
            if own_new:
                vl = sorted(vlines)
                for s2 in own_new:
                    va = c2["vtable_slots"][s2]["va"] if s2 < len(c2["vtable_slots"]) else 0
                    txt = "    virtual void NewVirtual%d() { NW_UNPORTED(); }  // %d 0x%08x (C2 only, not named yet)" % (s2, s2, va)
                    nxt = next((ln for sl, ln in vl if sl > s2), None)
                    if nxt is not None:
                        before[nxt].append(txt)
                    else:
                        # after the last virtual line, else at the end of the class body
                        anchor = max((ln for sl, ln in vl), default=None)
                        if anchor is not None:
                            after[anchor].append(txt)
                        else:
                            before[cd.close_line].append(txt)
        # --- asserts
        for (ln, kind, cls, fld, val) in cd.asserts:
            if kind == "SIZE" and cls == n:
                edits[ln] = "NW_ASSERT_SIZE(%s, %s);" % (n, hexs(c2["size"]).replace("0X", "0x"))
            elif kind == "OFFSET" and cls == n:
                if fld in newoff:
                    edits[ln] = "NW_ASSERT_OFFSET(%s, %s, %s);" % (n, fld, hexs(newoff[fld]).replace("0X", "0x"))
                else:
                    edits[ln] = "// " + lines[ln] + "   // C2: field not mapped"
    out = []
    for i, t in enumerate(lines):
        out.extend(before.get(i, []))
        out.append(edits.get(i, t))
        out.extend(after.get(i, []))
    if cds and any(cd.name in D.c2 for cd in cds):
        # the original packs the classes at 4 (a DOUBLE or SQWORD at a 4-aligned offset, which C2 has
        # where C1 happened to have it 8-aligned)
        last_inc = max((i for i, t in enumerate(out[:60]) if t.startswith("#include")), default=None)
        if last_inc is not None:
            out.insert(last_inc + 1, "#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets")
            out.append("#pragma pack(pop)")
    return "\n".join(out)


# ----------------------------------------------------------------------------------------------
# sources
# ----------------------------------------------------------------------------------------------
def split_units(lines):
    """[(intro lines, function lines (up to and including the hook line), hook match)] + tail."""
    units = []
    start = 0
    for i, l in enumerate(lines):
        if i < start:
            continue
        m = HOOK.match(l)
        if not m:
            continue
        end = i
        depth = l.count("(") - l.count(")")
        while depth > 0 and end + 1 < len(lines):      # NW_HOOK_CTOR_ARGS spans two lines
            end += 1
            depth += lines[end].count("(") - lines[end].count(")")
        chunk = lines[start:end + 1]
        # the function starts at the last `// 0xVA` comment line before the definition
        s = None
        for k in range(len(chunk) - 2, -1, -1):
            if ADDR_COMMENT.match(chunk[k]):
                s = k
                break
        if s is None:
            # fallback: the definition ends with the last column-0 `}`; its signature follows the previous blank line
            close = max((k for k in range(len(chunk) - 1) if chunk[k].startswith("}")), default=None)
            if close is None:
                s = 0
            else:
                k = close
                while k > 0 and not chunk[k].startswith("{"):
                    k -= 1
                while k > 0 and chunk[k - 1].strip():
                    k -= 1
                s = k
        units.append((chunk[:s], chunk[s:], m))
        start = end + 1
    return units, lines[start:]


def signature_of(fn_lines):
    """The definition's head: lines after the comment block up to the opening brace (no initializer list)."""
    head = []
    for l in fn_lines:
        if l.startswith("//") or not l.strip():
            continue
        if l.startswith("{") or l.rstrip().endswith("{"):
            break
        head.append(l)
        if l.strip().endswith((")", ") const")) and not l.strip().endswith(","):
            break
    sig = " ".join(x.strip() for x in head)
    sig = re.sub(r"__declspec\(\w+\)\s*", "", sig)
    # drop a constructor initializer list
    depth = 0
    for i, ch in enumerate(sig):
        if ch == "(":
            depth += 1
        elif ch == ")":
            depth -= 1
            if depth == 0:
                rest = sig[i + 1:].strip()
                sig = sig[:i + 1] + (" const" if rest.startswith("const") else "")
                break
    return sig


def gen_source(D: Data, path: Path, hdr_info, report, ctor_bad):
    lines = path.read_text(encoding="utf-8", errors="replace").split("\n")
    units, tail = split_units(lines)
    out = []

    def fix_vt(l):
        m = CHECK_VT.match(l)
        if not m:
            return l
        cls = m.group(2)
        c2 = D.c2.get(cls)
        if not c2 or not c2.get("vtable"):
            return "// C2: no vtable for " + l
        return "%s(%s,%s 0x%08x, %d)%s" % (m.group(1), cls, m.group(3) or "", iv(c2["vtable"]), len(c2.get("vtable_slots", [])),
                                       l[m.end():])

    def hide_helpers(block):
        """A helper of the intro that does not compile against the C2 headers goes inside #if 0."""
        if not D.bad_helpers:
            return block
        res, k = [], 0
        while k < len(block):
            l = block[k]
            fm = re.match(r"^[\w:<>\*&~ ,]+?\b(\w+::~?\w+)\s*\(", l)
            if fm and not l.startswith(("NW_", "//", "#")) and ("fn:" + fm.group(1)) in D.bad_helpers:
                # a definition the vtable needs but that does not compile: an empty body that breaks
                j = k
                while j < len(block) and not block[j].startswith("}"):
                    j += 1
                head = l.split("{")[0].rstrip()
                res.append("// C2: does not compile against the C2 headers (db/demote.csv); stub:")
                res.append(head)
                res.append("{")
                res.append("    NW_UNPORTED_NORET();")
                res.append("}")
                k = j + 1
                continue
            hm = re.match(r"^(?:static|inline|FORCEINLINE|__forceinline)\b[^;{]*?\b(\w+)\s*\(", l)
            if hm and hm.group(1) in D.bad_helpers:
                j = k
                while j < len(block) and not block[j].startswith("}"):
                    j += 1
                res.append("#if 0  // C2: does not compile against the C2 headers (db/demote.csv)")
                res.extend(block[k:j + 1])
                res.append("#endif")
                k = j + 1
                continue
            res.append(l)
            k += 1
        return res

    for intro, fn, hm in units:
        out.extend(fix_vt(l) for l in hide_helpers(intro))
        kind, va1s, rest = hm.group(1), hm.group(2), hm.group(3)
        va1 = int(va1s, 16)
        args = [a.strip() for a in rest.rstrip(")").split(",")]
        cls = args[0] if kind not in ("FUNC",) else ""
        meth = args[1] if kind in ("METHOD", "METHOD_SIG", "VARARGS", "VARARGS_SLOT") and len(args) > 1 else ""
        name = ("%s::%s" % (cls, meth)) if meth else (cls + "::" + cls if kind.startswith("CTOR") else cls + "::~" + cls if kind == "DTOR" else args[0])
        # the C2 address
        va2 = None
        fm = D.fmap.get(va1)
        if fm:
            va2 = int(fm["c2_va"], 16)
        elif va1 in D.code_amap:
            va2 = D.code_amap[va1]     # no function-map pair, but the callers' operands pair it
        # a virtual: the C2 vtable slot of the class is the authority
        mi = hdr_info.get(cls, {}).get(va1)
        if mi is not None and mi.kind == "virtual" and mi.slot is not None and cls in D.vslots and cls in D.c2:
            s2 = D.vslots[cls]["map"].get(str(mi.slot))
            slots = D.c2[cls].get("vtable_slots", [])
            if s2 is not None and s2 < len(slots) and not slots[s2].get("import"):
                v = slots[s2]["va"]
                if va2 is not None and v != va2:
                    report.append((path.name, cls, name, va1s, "%08x" % va2, "note", "slot says %08x, fmap says %08x" % (v, va2)))
                va2 = v
        if kind == "CTOR" and cls in D.c2 and D.c2[cls].get("ctor"):
            va2 = iv(D.c2[cls]["ctor"])
        ok, why = decide(D, kind, cls, va1, va2, fn, ctor_bad)
        if ok:
            new = []
            for l in fn:
                m = HOOK.match(l)
                if m:
                    l = "NW_HOOK_%s(0x%08x, %s" % (m.group(1), va2, m.group(3))
                elif ADDR_COMMENT.match(l) and not new:
                    pass
                new.append(l)
            # first comment line: address and vslot
            for k, l in enumerate(new):
                if ADDR_COMMENT.match(l):
                    l = re.sub(r"0x[0-9a-fA-F]{8}", "0x%08x" % va2, l, count=1)
                    if mi is not None and mi.kind == "virtual" and mi.slot is not None and cls in D.vslots:
                        s2 = D.vslots[cls]["map"].get(str(mi.slot))
                        if s2 is not None:
                            l = re.sub(r"\(vslot \d+\)", "(vslot %d)" % s2, l)
                    new[k] = l
                    break
            for k, l in enumerate(new):
                if HOOK.match(l) or l.lstrip().startswith("//"):
                    continue
                code_part, sep, cmt = l.partition("//")
                code_part = re.sub(r"\b0x(10[0-9A-Fa-f]{6})\b",
                                   lambda m: "0x%08x" % D.map_literal(int(m.group(1), 16)), code_part)
                new[k] = code_part + sep + cmt
            if va2 in D.cand_set:
                new.insert(0, "// C2 CANDIDATE: the C2 function is not the same code as the C1 one; kept only while it matches the stock one (verify_data)")
            out.extend(new)
            report.append((path.name, cls, name, va1s, "%08x" % va2, "candidate" if va2 in D.cand_set else "ported", ""))
        else:
            stub = make_stub(kind, cls, fn, va2, va1s, mi.text if mi is not None else None, meth, cls in D.c2)
            if stub:
                out.extend(["// 0x%08x  NOT PORTED (%s)" % (va2 or va1, why)] + stub)
            report.append((path.name, cls, name, va1s, ("%08x" % va2) if va2 else "", "stub" if stub else "skipped", why))
    out.extend(fix_vt(l) for l in tail)
    return "\n".join(out)


def decl_to_def(cls, decl):
    """`virtual INT OnClan();  // ...` of a header -> `INT Cls::OnClan()` (no default arguments)."""
    d = decl.split("//")[0].strip().rstrip(";").strip()
    d = re.sub(r"^(virtual|static|inline)\s+", "", d)
    d = re.sub(r"\s*=\s*[^,)]+(?=[,)])", "", d)                   # default arguments
    m = re.search(r"(~?\w+)\s*\(", d)
    if not m:
        return None
    return d[:m.start()] + cls + "::" + d[m.start():]


def make_stub(kind, cls, fn, va2, va1s, decl=None, meth="", is_uclass=True):
    code = [l for l in fn if l.strip() and not l.lstrip().startswith("//") and not l.startswith("NW_HOOK_")]
    has_body = any("{" in l for l in fn)
    macro_def = (not has_body and meth and any(re.search(r"\b%s\b" % re.escape(meth), l) for l in code))
    if kind == "DTOR":
        if is_uclass or not has_body:
            return []      # the destructor is inline in NW_DECLARE_CLASS
        sig = signature_of(fn)
        return ["%s" % sig, "{", "    NW_UNPORTED();", "}"] if sig else []
    if has_body:
        sig = signature_of(fn)
    elif macro_def and decl:
        sig = decl_to_def(cls, decl)         # the C1 body is a macro expansion: the header has the signature
    else:
        return []          # a hook line only: the body is defined by another unit
    if not sig:
        return []
    if kind.startswith("CTOR"):
        return ["%s" % sig, "{", "    NW_UNPORTED();", "}"]
    if BYVALUE.search(sig) or va2 is None:
        # a naked function cannot take a class with a destructor by value, and with no C2 address
        # there is nothing to jump to: a definition that breaks keeps the vtable and the linker happy
        return ["%s" % sig, "{", "    NW_UNPORTED_NORET();", "}"]
    return ["NW_NAKED %s NW_STOCK(0x%08x)" % (sig, va2)]


def indiv(D: Data, va1, va2):
    """Individually eligible: matched, category A/B, every operand explained."""
    el = D.elig.get(va1)
    if va2 is None:
        return False, "no C2 counterpart"
    if D.c2kind.get(va2) in ("thunk", "crt", "eh", "fragment"):
        return False, "the C2 function is a %s (the code moved out of NWindow)" % D.c2kind[va2]
    if "%08x" % va2 in D.demoted:
        return False, "does not compile against the C2 headers: " + D.demoted["%08x" % va2]
    fm = D.fmap.get(va1)
    if D.cand_mode and fm and fm["kind"] == "game" and fm["cat"] in ("C", "D") and float(fm["sim"]) >= D.cand_min_sim             and (el is None or el["ok"] == "1"):
        # a candidate: the C2 function is not the same code, the C1 body is tried anyway and kept only if the
        # compiled result matches the stock function in every set tools/port/verify_data.py compares
        D.cand_set.add(va2)
        return True, ""
    if el is None:
        return False, "not in eligible.csv (category D or not a game function)"
    if el["ok"] != "1":
        return False, "eligible.csv: " + el["reason"]
    if el["cat"] not in ("A", "B"):
        return False, "category " + el["cat"]
    return True, ""


def decide(D: Data, kind, cls, va1, va2, fn, ctor_bad):
    ok, why = indiv(D, va1, va2)
    if ok and kind.startswith("CTOR") and cls in D.ctor_chain_bad:
        return False, "a base class constructor is not ported (" + D.ctor_chain_bad[cls] + ")"
    if ok:
        ok, why = body_checks(D, cls, fn, ctor_bad)
    return ok, why


def body_checks(D: Data, cls, fn, ctor_bad):
    body = "\n".join(fn)
    code = re.sub(r"//[^\n]*", "", re.sub(r"/\*.*?\*/", "", body, flags=re.S))
    idents = set(IDENT.findall(code))
    badh = idents & D.bad_helpers
    if badh:
        return False, "uses a helper that does not compile in C2: " + ",".join(sorted(badh))
    bad = idents & D.bad_globals
    if bad:
        return False, "global without a C2 address: " + ",".join(sorted(bad))
    chain = D.chain_derived(cls)
    hit = {i for i in chain if re.search(r"(?<![\w>.])" + re.escape(i) + r"\b", code)}
    hit |= {i for i in D.derived_any if re.search(r"(->|\.)" + re.escape(i) + r"\b", code)}
    if hit:
        return False, "field without evidence: " + ",".join(sorted(hit))
    for m in re.finditer(r"\bnew\s+(\w+)", code):
        if m.group(1) in ctor_bad:
            return False, "constructs " + m.group(1) + " (constructor not ported)"
    for m in re.finditer(r"\b0x(10[0-9A-Fa-f]{6})\b", code):
        if D.map_literal(int(m.group(1), 16)) is None:
            return False, "literal address 0x%s without a C2 counterpart" % m.group(1)
    return True, ""


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--only", default="")
    ap.add_argument("--candidates", action="store_true",
                    help="also copy the C1 body of matched functions that are not the same code (category C/D)")
    ap.add_argument("--min-sim", type=float, default=0.6)
    ap.add_argument("--out", default=str(ROOT))
    a = ap.parse_args()
    D = Data()
    D.cand_mode, D.cand_min_sim = a.candidates, a.min_sim
    out_root = Path(a.out)
    only = {x for x in a.only.split(",") if x}
    hdr_files = [p for p in sorted((C1_ROOT / "include" / "nwindow").glob("*.h")) if ".gen." not in p.name]
    src_files = [p for p in sorted((C1_ROOT / "src" / "nwindow").glob("*.cpp")) if ".gen." not in p.name]

    # methods of every header by (class, c1 va), for the slot lookup of the sources
    hdr_info = collections.defaultdict(dict)
    for p in hdr_files:
        _, cds = hdr.parse(p)
        for cd in cds:
            for m in cd.methods:
                if m.va is not None:
                    hdr_info[cd.name].setdefault(m.va, m)

    # constructors: a ported constructor runs the base constructors of OUR classes, so every
    # constructor of every ancestor must be ported too; a class with a constructor that cannot
    # be ported cannot be constructed by copied code either (`new X`)
    cls_ctors = collections.defaultdict(list)     # class -> [individually eligible?]
    for p in src_files:
        lines_ = p.read_text(encoding="utf-8", errors="replace").split("\n")
        units_, _ = split_units(lines_)
        for intro, fn, hm in units_:
            if hm.group(1).startswith("CTOR"):
                c = hm.group(3).split(",")[0].strip().rstrip(")")
                va1 = int(hm.group(2), 16)
                fm = D.fmap.get(va1)
                va2 = int(fm["c2_va"], 16) if fm else None
                if hm.group(1) == "CTOR" and c in D.c2 and D.c2[c].get("ctor"):
                    va2 = iv(D.c2[c]["ctor"])
                ok, _ = indiv(D, va1, va2)
                if ok:
                    ok, _ = body_checks(D, c, fn, set())
                cls_ctors[c].append(ok)
    all_ok = {c: all(v) for c, v in cls_ctors.items()}

    def chain_ok(c):
        """(True, '') when c and every ancestor of ours has all its constructors ported."""
        while c is not None:
            if c in all_ok and not all_ok[c]:
                return False, c
            c = D.c1[c]["super"] if c in D.c1 else None
        return True, ""

    ctor_bad = set()
    for c in set(cls_ctors) | set(D.c1):
        if not chain_ok(c)[0]:
            ctor_bad.add(c)
        parent = D.c1[c]["super"] if c in D.c1 else None
        okp, badp = chain_ok(parent)
        if not okp:
            D.ctor_chain_bad[c] = badp

    report = []
    (out_root / "include" / "nwindow").mkdir(parents=True, exist_ok=True)
    (out_root / "src" / "nwindow").mkdir(parents=True, exist_ok=True)
    for p in hdr_files:
        if only and p.name not in only:
            continue
        txt = gen_header(D, p, report)
        if p.name == "NWindow.h":
            txt = txt.replace("#pragma warning(disable : 4291)", "#pragma warning(disable : 4291)\n\n"
                              "// a function the C2 port has not reimplemented yet: it must never run (hook mode never reaches it)\n"
                              "#define NW_UNPORTED() __debugbreak()\n"
                              "// the same for a function that returns a value (nothing after it is reachable: no return needed);\n"
                              "// constructors use the plain one, or the optimizer drops the vtable store and the vtable with it\n"
                              "#define NW_UNPORTED_NORET() do { __debugbreak(); __assume(0); } while (0)", 1)
        (out_root / "include" / "nwindow" / p.name).write_text(txt, encoding="utf-8", newline="\n")
    for p in src_files:
        if only and p.name not in only:
            continue
        txt = gen_source(D, p, hdr_info, report, ctor_bad)
        (out_root / "src" / "nwindow" / p.name).write_text(txt, encoding="utf-8", newline="\n")
    with (ROOT / "db" / "gen_report.csv").open("w", encoding="utf-8", newline="") as f:
        w = csv.writer(f)
        w.writerow(["file", "class", "name", "c1_va", "c2_va", "action", "reason"])
        w.writerows(report)
    if a.candidates:
        with (ROOT / "db" / "candidates.csv").open("w", encoding="utf-8", newline="") as f:
            w = csv.writer(f)
            w.writerow(["c2_va"])
            for v in sorted(D.cand_set):
                w.writerow(["%08x" % v])
    cnt = collections.Counter(r[5] for r in report)
    print("units:", dict(cnt))
    why = collections.Counter(r[6].split(":")[0] for r in report if r[5] != "ported")
    print("not ported because:", why.most_common(10))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
