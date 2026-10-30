#!/usr/bin/env python3
"""Hooked functions that can reach a function whose body is `NW_UNPORTED()` (an int3).

A copied or ported body that calls `Class::Method()` calls OUR definition of it; when that definition is
an NW_UNPORTED stub (no C2 address was known, or the function is not ported) the process breaks the first
time the call runs. The cycle cannot see it (it never runs the hooks). This walks the compiled
NWindowDecomp.dll: every function that starts with int3 is a breaker, every `call rel32` / `jmp rel32`
inside a hooked function (and, transitively, inside the non-hooked functions it calls in our module) that
lands on a breaker is reported with the path.

usage: python tools/port/check_breakers.py [--log build/smoke_system/NWindowDecomp.log]"""
from __future__ import annotations

import argparse
import bisect
import re
import sys
from pathlib import Path

import pefile

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tools"))
import verify_hooks as vh  # noqa: E402


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--log", type=Path, default=ROOT / "build" / "smoke_system" / "NWindowDecomp.log")
    ap.add_argument("--dll", type=Path, default=ROOT / "build" / "bin" / "Release" / "NWindowDecomp.dll")
    a = ap.parse_args()
    pe = pefile.PE(str(a.dll))
    base = pe.OPTIONAL_HEADER.ImageBase
    mem = pe.get_memory_mapped_image()
    text = next(s for s in pe.sections if s.Name.startswith(b".text"))
    lo, hi = base + text.VirtualAddress, base + text.VirtualAddress + text.Misc_VirtualSize
    symbols = vh.our_symbols(a.dll)
    starts = [x for x, _ in symbols]
    label = dict(symbols)

    def fn_of(va):
        i = bisect.bisect_right(starts, va) - 1
        return starts[i] if i >= 0 else None

    def body(f):
        i = starts.index(f)
        end = starts[i + 1] if i + 1 < len(starts) else hi
        return f, end

    breakers = {f for f in starts if lo <= f < hi and mem[f - base] == 0xCC and mem[f - base + 1] in (0xCC, 0x90, 0xC3, 0xCC)
                and bytes(mem[f - base:f - base + 1]) == b"\xcc"}
    # a breaker is a function that BEGINS with int3 followed by padding or ret (the whole body is the break)
    real = set()
    for f in breakers:
        _, end = body(f)
        b = bytes(mem[f - base:end - base])
        if b[:1] == b"\xcc" and all(c in (0xCC, 0x90, 0xC3) for c in b[:16]):
            real.add(f)
    hooked = {}
    for line in a.log.read_text(encoding="latin-1").splitlines():
        m = vh.LOG_RE.match(line)
        if m:
            hooked[base + int(m.group(3), 16)] = m.group(2)

    def callees(f):
        f0, end = body(f)
        data = bytes(mem[f0 - base:end - base])
        out = set()
        for m in re.finditer(rb"[\xe8\xe9](....)", data, re.S):
            rel = int.from_bytes(m.group(1), "little", signed=True)
            t = f0 + m.start() + 5 + rel
            if lo <= t < hi and t in starts:
                out.add(t)
        return out

    bad = []
    for h, name in sorted(hooked.items()):
        seen, stack = {h}, [(h, [h])]
        while stack:
            f, path = stack.pop()
            for t in callees(f):
                if t in real:
                    bad.append((name, [label.get(x, "%x" % x) for x in path + [t]]))
                elif t not in seen and t not in hooked and len(path) < 4:
                    seen.add(t)
                    stack.append((t, path + [t]))
    print("%d breakers (int3 bodies) in %d functions; %d hooked functions reach one" % (
        len(real), len(starts), len({b[0] for b in bad})))
    for name, path in bad[:200]:
        print("  %s -> %s" % (name, " -> ".join(path[1:])[:200]))
    return 1 if bad else 0


if __name__ == "__main__":
    raise SystemExit(main())
