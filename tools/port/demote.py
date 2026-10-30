#!/usr/bin/env python3
"""Turn compile errors of copied bodies into demotions: db/demote.csv (c2_va, reason).

A copied C1 body that does not compile against the C2 headers (a field that is gone, a changed
signature, an API the C2 Engine does not have) is not the same code: gen.py makes it a stub.
Reads the cl log, finds the generated unit around each error line (the last `// 0xVA` comment line
before it) and records its C2 address. Errors outside a unit (headers) are listed, not demoted.

usage: python tools/port/demote.py <cl log> [--stage build/gen_stage]
"""
from __future__ import annotations

import argparse
import csv
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
ERR = re.compile(r"^(?P<file>[^(\r\n]+)\((?P<line>\d+)\): error (?P<code>C\d+): (?P<msg>.*?)(?: \(compilando.*)?$")
ADDR = re.compile(r"^// 0x([0-9a-fA-F]{8})\b")


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("log")
    ap.add_argument("--stage", default=str(ROOT / "build" / "gen_stage"))
    a = ap.parse_args()
    stage = Path(a.stage)
    path = ROOT / "db" / "demote.csv"
    have = {}
    if path.exists():
        for r in csv.DictReader(path.open(encoding="utf-8")):
            have[r["c2_va"]] = r["reason"]
    new, loose = {}, []
    texts = {}
    for raw in open(a.log, encoding="cp1252", errors="replace"):
        m = ERR.match(raw.rstrip())
        if not m:
            continue
        f = Path(m.group("file").strip())
        if f.suffix != ".cpp":
            loose.append(raw.strip())
            continue
        name = f.name
        if name not in texts:
            p = stage / "src" / "nwindow" / name
            texts[name] = p.read_text(encoding="utf-8", errors="replace").split("\n") if p.exists() else []
        lines = texts[name]
        n = int(m.group("line")) - 1
        va = None
        helper = None
        # the unit around the error ends at its NW_HOOK_ line: the first one below the error, when no
        # other definition starts in between
        for k in range(min(n, len(lines) - 1), min(n + 400, len(lines))):
            hk = re.match(r"^NW_HOOK_\w+\(\s*0x([0-9A-Fa-f]{8})", lines[k])
            if hk:
                va = hk.group(1).lower()
                break
            if k > n and (lines[k].startswith("NW_NAKED") or lines[k].startswith("// 0x")):
                break
        for k in ([] if va else range(min(n, len(lines) - 1), -1, -1)):
            if k < n and lines[k].startswith("NW_HOOK_"):
                break      # the line belongs to what follows the previous unit: a helper of the intro
            am = ADDR.match(lines[k])
            if am:
                # an unported stub header is not a copied body
                if "NOT PORTED" in lines[k]:
                    break
                va = am.group(1).lower()
                break
        # a helper function of the intro (static ... Name(...) at column 0) around the error?
        for k in range(min(n, len(lines) - 1), -1, -1):
            l = lines[k]
            hm = re.match(r"^(?:static|inline|FORCEINLINE|__forceinline|template)\b[^;{]*?\b(\w+)\s*\(", l)
            if hm:
                helper = hm.group(1)
                break
            if l.startswith("}") and k < n:
                break
        if va is None and helper is None:
            # a definition without a hook (the stock body is shared, the vtable still needs ours)
            for k in range(min(n, len(lines) - 1), -1, -1):
                dm = re.match(r"^[\w:<>\*&~ ,]+?\b(\w+::~?\w+)\s*\(", lines[k])
                if dm and not lines[k].startswith(("NW_", "//", "#")):
                    helper = "fn:" + dm.group(1)
                    break
                if lines[k].startswith("}") and k < n:
                    break
        if helper and helper.startswith("fn:"):
            key = "helper:" + helper
            if key not in have:
                new.setdefault(key, "%s: %s" % (m.group("code"), m.group("msg")[:100]))
        elif helper and (va is None or lines[n].strip() and not any(lines[j].startswith("NW_HOOK_") for j in range(k, n))):
            key = "helper:" + helper
            if key not in have:
                new.setdefault(key, "%s: %s" % (m.group("code"), m.group("msg")[:100]))
        elif va is None:
            loose.append(raw.strip())
        elif va not in have:
            new.setdefault(va, "%s: %s" % (m.group("code"), m.group("msg")[:100]))
    with path.open("a" if have else "w", encoding="utf-8", newline="") as fh:
        w = csv.writer(fh)
        if not have:
            w.writerow(["c2_va", "reason"])
        for va, why in sorted(new.items()):
            w.writerow([va, why])
    print("demoted %d new units (%d known); %d errors outside a copied body" % (len(new), len(have), len(loose)))
    for l in sorted(set(loose))[:15]:
        print("  ", l[:170])
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
