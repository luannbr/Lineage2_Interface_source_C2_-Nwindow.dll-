#!/usr/bin/env python3
"""Carry the C1 verify notes (db/c1/verify_notes.csv: a difference of tools/verify_hooks.py that
was checked by hand and is not a bug) over to the C2 functions the generator ported from the same
body: same source, same compiler difference. db/verify_notes.csv keeps what it has; notes already
there win. The note text is prefixed `C1:` so a reader knows it was inherited, not re-checked.

usage: python tools/port/carry_notes.py"""
from __future__ import annotations

import csv
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]


def main() -> int:
    rows = lambda p: [r for r in csv.DictReader(l for l in p.open(encoding="utf-8") if not l.startswith("#"))]
    c1 = {int(r["va"], 16): r["explanation"] for r in rows(ROOT / "db" / "c1" / "verify_notes.csv")}
    ported = {int(r["c1_va"], 16): r["c2_va"] for r in rows(ROOT / "db" / "gen_report.csv") if r["action"] == "ported"}
    path = ROOT / "db" / "verify_notes.csv"
    have = {}
    head = []
    if path.exists():
        for l in path.open(encoding="utf-8"):
            if l.startswith("#"):
                head.append(l.rstrip("\n"))
        have = {r["va"].lower(): r["explanation"] for r in rows(path)}
    added = 0
    for v1, why in c1.items():
        v2 = ported.get(v1)
        if v2 and v2 not in have:
            have[v2] = "C1: " + why
            added += 1
    with path.open("w", encoding="utf-8", newline="") as f:
        for l in head or ["# Differences tools/verify_hooks.py reports that were checked by hand and are not bugs.",
                          "# One line per stock address; 'C1:' marks a note inherited from the C1 project (same body)."]:
            f.write(l + "\n")
        w = csv.writer(f)
        w.writerow(["va", "explanation"])
        for va in sorted(have):
            w.writerow([va, have[va]])
    print("%d notes carried over, %d in db/verify_notes.csv" % (added, len(have)))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
