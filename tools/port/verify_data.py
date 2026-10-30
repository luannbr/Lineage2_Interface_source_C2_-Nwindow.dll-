#!/usr/bin/env python3
"""What tools/verify_hooks.py does not look at: the DATA a function touches.

verify_hooks compares the pop on return, the virtual slots and the imports; a copied C1 body can
still read a field at the C1 offset, compare with a changed constant or show another string and pass
all three. Here the same two sides (the stock C2 function, our compiled one, found through the
hook log and the PDB) are reduced to
  consts   immediate values >= 0x20 that are not addresses (and the float constants read from .rdata)
  disps    [reg+disp] displacements >= 0x20 of non-stack operands (field offsets)
  strs     the wide strings it refers to
and compared as sets, following one-call forwarders on both sides like verify_hooks. A function
passes when every set is equal. Compiler differences exist (a folded constant, a strength reduction),
so a miss is a reason to look, not a proof of a bug: db/verify_data_notes.csv (va,explanation) lists
the ones looked at.

Writes db/verify_data.csv (c2_va,name,status,detail); exit code 0 always (the generator decides).

usage: python tools/port/verify_data.py [--log build/smoke_system/NWindowDecomp.log]"""
from __future__ import annotations

import argparse
import bisect
import csv
import re
import sys
from pathlib import Path

import pefile
from capstone import CS_ARCH_X86, CS_MODE_32, Cs
from capstone.x86 import X86_OP_IMM, X86_OP_MEM

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tools"))
import verify_hooks as vh  # noqa: E402
from nwd.image import Image  # noqa: E402


class Side:
    """One module: read(va, n), image range, wide-string reader."""

    def __init__(self, read, lo, hi, text_lo, text_hi, extent, rdata):
        self.read, self.lo, self.hi = read, lo, hi
        self.text_lo, self.text_hi = text_lo, text_hi
        self.extent, self.rdata = extent, rdata

    def wstr(self, va):
        out = []
        for i in range(200):
            b = self.read(va + 2 * i, 2)
            if len(b) < 2:
                return None
            c = b[0] | (b[1] << 8)
            if c == 0:
                return "".join(out) if len(out) >= 3 else None
            if not 32 <= c < 0x2000:
                return None
            out.append(chr(c))
        return None


def data_features(md, side: Side, start: int, depth: int = 0):
    consts, disps, strs = set(), set(), set()
    calls = []
    ext = side.extent(start)
    for a, b in ext:
        for ins in vh.decode_skipping_tables(md, vh.Code(side.read, {}, lambda v: ext, lambda v: True), a, b):
            mn = ins.mnemonic
            is_branch = mn == "call" or mn.startswith("j")
            for op in ins.operands:
                if op.type == X86_OP_IMM:
                    v = op.imm & 0xFFFFFFFF
                    if is_branch:
                        if mn in ("call", "jmp") and side.text_lo <= v < side.text_hi and not any(x <= v < y for x, y in ext):
                            calls.append(v)
                        continue
                    if side.lo <= v < side.hi:
                        s = side.wstr(v) if not (side.text_lo <= v < side.text_hi) else None
                        if s:
                            strs.add(s)
                    elif v >= 0x20 and v != 0xFFFFFFFF and mn not in ("sub", "add") or (v >= 0x40 and mn in ("sub", "add")):
                        # stack adjustments are compiler business: add/sub esp
                        if mn in ("sub", "add") and ins.op_str.startswith("esp"):
                            continue
                        consts.add(v)
                elif op.type == X86_OP_MEM:
                    mm = op.mem
                    d = mm.disp & 0xFFFFFFFF
                    base = ins.reg_name(mm.base) if mm.base else ""
                    if mm.base == 0 and not mm.index and side.lo <= d < side.hi:
                        if mn.startswith("f") or mn.endswith(("ss", "sd")):
                            n = op.size if op.size in (4, 8) else 4
                            consts.add(("f", bytes(side.read(d, n)).hex()))
                        s = side.wstr(d)
                        if s:
                            strs.add(s)
                    elif base and base not in ("esp", "ebp") and 0x20 <= d < 0x10000:
                        disps.add(d)
    if depth < 3 and len(calls) == 1 and not consts and not disps and not strs:
        return data_features(md, side, calls[0], depth + 1)
    return consts, disps, strs


def fmt(s):
    return "{" + ", ".join(sorted(("%x" % v) if isinstance(v, int) else (v if isinstance(v, str) else "%s:%s" % v)
                                  for v in s)) + "}"


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--log", type=Path, default=ROOT / "build" / "smoke_system" / "NWindowDecomp.log")
    ap.add_argument("--dll", type=Path, default=ROOT / "build" / "bin" / "Release" / "NWindowDecomp.dll")
    a = ap.parse_args()
    md = Cs(CS_ARCH_X86, CS_MODE_32)
    md.detail = True

    stock = Image()
    granges = {}
    for r in csv.DictReader((ROOT / "analysis/ghidra/functions.tsv").open(encoding="utf-8"), delimiter="\t"):
        granges[int(r["va"], 16)] = [tuple(int(x, 16) for x in rg.split("-")) for rg in r["ranges"].split(",")]
    for r in csv.DictReader(l for l in (ROOT / "db/not_functions.csv").open(encoding="utf-8") if not l.startswith("#")):
        owner, frag = int(r["part_of"], 16), int(r["va"], 16)
        if owner in granges and frag in granges:
            granges[owner] = granges[owner] + granges[frag]
    t_lo, t_hi = stock.sect_range(".text")
    s_side = Side(stock.read, stock.base, stock.base + stock.pe.OPTIONAL_HEADER.SizeOfImage, t_lo, t_hi,
                  lambda va: [(x, y + 1) for x, y in granges.get(va, [(va, va + 0x3F)])], None)

    pe = pefile.PE(str(a.dll))
    base = pe.OPTIONAL_HEADER.ImageBase
    mem = pe.get_memory_mapped_image()
    symbols = vh.our_symbols(a.dll)
    starts = [x for x, _ in symbols]
    o_text = next(s for s in pe.sections if s.Name.startswith(b".text"))
    o_lo, o_hi = base + o_text.VirtualAddress, base + o_text.VirtualAddress + o_text.Misc_VirtualSize

    def o_extent(va):
        i = bisect.bisect_right(starts, va)
        return [(va, starts[i] if i < len(starts) else o_hi)]
    o_side = Side(lambda va, n: mem[va - base:va - base + n], base, base + pe.OPTIONAL_HEADER.SizeOfImage,
                  o_lo, o_hi, o_extent, None)

    notes = {}
    npath = ROOT / "db" / "verify_data_notes.csv"
    if npath.exists():
        for r in csv.DictReader(l for l in npath.open(encoding="utf-8") if not l.startswith("#")):
            notes[r["va"].lower()] = r["explanation"]

    rows = []
    for line in a.log.read_text(encoding="latin-1").splitlines():
        m = vh.LOG_RE.match(line)
        if m:
            rows.append((int(m.group(1), 16), m.group(2), int(m.group(3), 16)))
    out = []
    n_ok = n_diff = n_note = 0
    for va, name, rva in sorted(rows):
        sc, sd, ss = data_features(md, s_side, va)
        oc, od, os_ = data_features(md, o_side, base + rva)
        detail = []
        for label, x, y in (("consts", sc, oc), ("disps", sd, od), ("strs", ss, os_)):
            if x != y:
                detail.append("%s stock-only %s ours-only %s" % (label, fmt(x - y), fmt(y - x)))
        key = "%08x" % va
        if not detail:
            status = "ok"
            n_ok += 1
        elif key in notes:
            status = "note"
            n_note += 1
        else:
            status = "diff"
            n_diff += 1
        out.append((key, name, status, "; ".join(detail)[:400]))
    with (ROOT / "db" / "verify_data.csv").open("w", encoding="utf-8", newline="") as f:
        w = csv.writer(f)
        w.writerow(["c2_va", "name", "status", "detail"])
        w.writerows(out)
    print("%d ok, %d explained, %d differ (db/verify_data.csv)" % (n_ok, n_note, n_diff))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
