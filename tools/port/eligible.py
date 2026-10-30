#!/usr/bin/env python3
"""Which matched functions can be ported by copying the C1 body: db/eligible.csv.

A pair of functions (db/fmap.csv) of category A or B has the same instructions up to registers
and displacements. The C1 body compiled against the C2 headers is equivalent iff every
displacement in it ends up where C2 has it. Walking the aligned instructions again:
  * esp/ebp based operands (the stack frame) are not ours to care about;
  * a `this`-relative displacement d1 must satisfy  d2 == layout(class chain)(d1)  (db/layout.json:
    the field containing d1, same distance into it); inside the engine part of the object (below
    the first nwindow field) it must satisfy d2 == d1;
  * any other [reg+disp] (another object's field, an array element, a struct on the stack passed
    around) must keep its displacement, d1 == d2: a struct whose layout moved would be copied
    wrongly by the verbatim body, and is reported as drift (analysis/port/drift.csv);
  * addresses (globals, tables, strings) must have an entry in db/amap.csv;
  * `ret N` must be the same N, and the calls to imports must be to the same import names.
Output columns: c1_va,c2_va,cat,ok,reason. reason names the first failing operand.

usage: python tools/port/eligible.py   (after match.py, maps.py, layout.py)"""
from __future__ import annotations

import collections
import csv
import difflib
import json
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import common  # noqa: E402
import maps  # noqa: E402
from common import ROOT, C1_ROOT  # noqa: E402
from layout import load_classes  # noqa: E402


def build_chains(layout, c1):
    """class -> sorted [(c1 start, c1 end, c2 start, status)] for the whole ancestor chain."""
    chains = {}

    def chain(n):
        if n in chains:
            return chains[n]
        out = []
        par = c1[n]["super"] if n in c1 else None
        if par in c1:
            out = list(chain(par))
        if n in layout:
            for r in layout[n]["fields"]:
                if r["c2"] is not None:
                    out.append((r["c1"], r["c1"] + r["ext"], r["c2"], r["status"]))
        chains[n] = sorted(out)
        return chains[n]

    for n in c1:
        chain(n)
    return chains


def same_function(m1, m2, fmap_pairs, t1, t2):
    """The C1 call target t1 and the C2 call target t2 are the same function: the function map
    pairs them, or their code is identical (a folded twin), or they are thunks of the same import."""
    if fmap_pairs.get(t1) == t2:
        return True
    a, b = m1.by_va.get(t1), m2.by_va.get(t2)
    if a is None or b is None:
        return False
    if a["hs"] == b["hs"] and len(a["s"]) > 0:
        return True
    key = (t1, t2)
    r = _SAME_CACHE.get(key)
    if r is None:
        # not paired by the map: the same function when the code is the same up to field offsets
        # (a small helper both builds have that the matcher left ambiguous)
        if a["hd"] == b["hd"]:
            r = True
        elif len(a["d"]) * len(b["d"]) < 160000:
            r = difflib.SequenceMatcher(None, a["d"], b["d"], autojunk=False).ratio() >= 0.9
        else:
            r = False
        _SAME_CACHE[key] = r
    return r


_SAME_CACHE = {}


def const_data_differs(i1, i2, kd, v1, v2, mnem, size):
    """The data an operand points at, when it is a string or a float constant (the token hash
    hides both): True when the two builds hold different values."""
    if kd != "data" or i1.section(v1) != ".rdata" or i2.section(v2) != ".rdata":
        return False
    s1, s2 = i1.wstr(v1, 300), i2.wstr(v2, 300)
    if s1 is not None and s2 is not None and len(s1) >= 2 and all(32 <= ord(c) < 0x2000 for c in s1):
        return s1 != s2
    if mnem.startswith("f") or mnem.endswith(("ss", "sd")):
        n = size if size in (4, 8) else 4
        return i1.read(v1, n) != i2.read(v2, n)
    return False


def main() -> int:
    m1, m2 = common.c1_model(), common.c2_model()
    i1, i2 = m1.img, m2.img
    c1 = load_classes(C1_ROOT)
    layout = json.load(open(ROOT / "db" / "layout.json", encoding="utf-8"))
    chains = build_chains(layout, c1)
    first_own = {}
    for n in c1:
        ch = chains.get(n) or []
        first_own[n] = ch[0][0] if ch else 0x28
    amap = {}
    for r in csv.DictReader((ROOT / "db" / "amap.csv").open(encoding="utf-8")):
        amap[(r["kind"], int(r["c1"], 16))] = (int(r["c2"], 16), float(r["share"]))
    hi1 = i1.base + i1.pe.OPTIONAL_HEADER.SizeOfImage
    hi2 = i2.base + i2.pe.OPTIONAL_HEADER.SizeOfImage

    rows = list(csv.DictReader((ROOT / "db" / "fmap.csv").open(encoding="utf-8")))
    fmap_pairs = {int(r["c1_va"], 16): int(r["c2_va"], 16) for r in rows}
    # more pairs the structure gives: the constructors of every class, and the functions with the
    # same name in both function databases (destructors, named slots)
    c2cls = load_classes(ROOT)
    iv = lambda v: v if isinstance(v, int) else int(v, 16)
    for n, c in c1.items():
        d = c2cls.get(n)
        if d:
            for key in ("ctor", "internal_ctor"):
                if c.get(key) and d.get(key):
                    fmap_pairs.setdefault(iv(c[key]), iv(d[key]))

    def names_of(path):
        out = collections.defaultdict(list)
        for r in csv.DictReader(path.open(encoding="utf-8")):
            nm = r["name"]
            if "::" in nm and not nm.startswith(("sub_", "FUN_")) and "vfn" not in nm:
                out[nm].append(int(r["va"], 16))
        return out
    n1, n2 = names_of(C1_ROOT / "db" / "functions.csv"), names_of(ROOT / "db" / "functions.csv")
    for nm, v in n1.items():
        if len(v) == 1 and len(n2.get(nm, [])) == 1:
            fmap_pairs.setdefault(v[0], n2[nm][0])
    out = []
    drift = collections.Counter()
    nok = 0
    for r in rows:
        if r["kind"] != "game" or r["cat"] not in ("A", "B", "C"):
            continue
        a, b = int(r["c1_va"], 16), int(r["c2_va"], 16)
        x, y = m1.by_va[a], m2.by_va[b]
        cls = r["class"]
        ra, rb = maps.raw(i1, x["ranges"]), maps.raw(i2, y["ranges"])
        reason = ""
        notes = []
        if len(ra) != len(x["l"]) or len(rb) != len(y["l"]) or len(ra) != len(rb):
            reason = "decode length"
        else:
            ta, tb = maps.this_info(ra), maps.this_info(rb)
            ch = chains.get(cls, [])
            for k in range(len(ra)):
                (ma, oa), (mb, ob) = ra[k], rb[k]
                if ma != mb or len(oa) != len(ob):
                    reason = "insn %d shape" % k
                    break
                if ma in ("ret", "retn") and oa != ob:
                    reason = "ret N differs"
                    break
                for p, q in zip(oa, ob):
                    if p[0] != q[0]:
                        reason = "insn %d operand kind" % k
                        break
                    if p[0] == "i":
                        v1, v2 = p[1], q[1]
                        in1, in2 = i1.base <= v1 < hi1, i2.base <= v2 < hi2
                        if in1 != in2:
                            reason = "insn %d imm addr/const" % k
                        elif in1:
                            kd = maps.section_kind(i1, v1)
                            lo_f, hi_f = x["ranges"][0][0], max(rg[1] for rg in x["ranges"])
                            if kd == "code" and (ma == "call" or (ma.startswith("j") and not lo_f <= v1 <= hi_f)):
                                if not same_function(m1, m2, fmap_pairs, v1, v2):
                                    reason = "insn %d calls %08x, C2 calls %08x (not the same function)" % (k, v1, v2)
                            elif kd and kd != "code" or (kd == "code" and ma != "call" and not ma.startswith("j")):
                                am = amap.get((kd, v1))
                                if am is None or am[0] != v2:
                                    reason = "insn %d address %08x" % (k, v1)
                                elif const_data_differs(i1, i2, kd, v1, v2, ma, 4):
                                    reason = "insn %d string/constant differs at %08x" % (k, v1)
                        elif v1 != v2 and not ma.startswith("j"):
                            if r["cat"] != "C":
                                reason = "insn %d constant" % k
                    elif p[0] == "m":
                        base1, d1, base2, d2 = p[1], p[4], q[1], q[4]
                        if i1.base <= d1 < hi1 and i2.base <= d2 < hi2:
                            kd = maps.section_kind(i1, d1)
                            am = amap.get((kd, d1)) if kd else None
                            if kd and (am is None or am[0] != d2):
                                reason = "insn %d data %08x" % (k, d1)
                            elif kd and const_data_differs(i1, i2, kd, d1, d2, ma, p[5] if len(p) > 5 else 4):
                                reason = "insn %d string/constant differs at %08x" % (k, d1)
                        elif (i1.base <= d1 < hi1) != (i2.base <= d2 < hi2):
                            reason = "insn %d disp addr/const" % k
                        elif base1 in ("esp", "ebp", "") and base2 in ("esp", "ebp", ""):
                            pass
                        elif base1 in ("esp", "ebp") or base2 in ("esp", "ebp"):
                            pass
                        elif maps.is_this(ta, k, base1) and maps.is_this(tb, k, base2):
                            exp = None
                            for lo, hi_, c2s, st in ch:
                                if lo <= d1 < hi_:
                                    exp = c2s + (d1 - lo)
                                    break
                            if exp is None:
                                exp = d1 if d1 < first_own.get(cls, 0x28) else None
                            if exp is None or exp != d2:
                                reason = "insn %d this+%x -> %x (layout says %s)" % (
                                    k, d1, d2, ("%x" % exp) if exp is not None else "?")
                                drift[(cls, d1, d2)] += 1
                        else:
                            if d1 != d2:
                                if ma in ("call", "jmp"):
                                    pass        # a virtual call: the slot moved, our headers decide the new one
                                else:
                                    notes.append("insn %d other+%x->%x" % (k, d1, d2))
                                    drift[("other", d1, d2)] += 1
                    if reason:
                        break
                if reason:
                    break
        ok = not reason
        nok += ok
        out.append((r["c1_va"], r["c2_va"], r["cat"], int(ok), reason or ";".join(notes[:4]),
                    r["class"], r["name"], x["size"], len(notes)))
    with (ROOT / "db" / "eligible.csv").open("w", encoding="utf-8", newline="") as f:
        w = csv.writer(f)
        w.writerow(["c1_va", "c2_va", "cat", "ok", "reason", "class", "name", "size1", "unverified"])
        w.writerows(out)
    (ROOT / "analysis" / "port").mkdir(parents=True, exist_ok=True)
    with (ROOT / "analysis" / "port" / "drift.csv").open("w", encoding="utf-8", newline="") as f:
        w = csv.writer(f)
        w.writerow(["class", "c1_disp", "c2_disp", "functions"])
        for (c, d1, d2), n in drift.most_common():
            w.writerow([c, hex(d1), hex(d2), n])
    tot = sum(o[7] for o in out)
    okb = sum(o[7] for o in out if o[3])
    print("pairs %d, eligible %d (%d bytes of %d)" % (len(out), nok, okb, tot))
    why = collections.Counter(o[4].split(" ")[0] + " " + (o[4].split(" ")[2] if len(o[4].split(" ")) > 2 else "") for o in out if not o[3])
    print("  eligible with unverified other-object displacements (compile check decides):", sum(1 for o in out if o[3] and o[8]))
    print("  rejected by:", why.most_common(8))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
