import sys
import os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from stubs import *

def run(kind):
    e = TipEmu(kind)
    if kind == 'stock':
        fmt, col = 0x100273C0, 0x10027470
    else:
        fmt = [v for k, v in e.symbols.items() if k.startswith('?FormatNumber@NToolTipInfo@@')][0]
        col = [v for k, v in e.symbols.items() if k.startswith('?NumberColor@NToolTipInfo@@')][0]
    out = []
    for n in [0, 5, 12, 123, 999, 1000, 1234, 12345, 123456, 1234567, 12345678, 123456789, 2147483647, -1, -12, -123, -1234, -123456]:
        p = e.call(fmt, args=[n & 0xFFFFFFFF])
        s = e.wstr(p)
        c = e.call(col, args=[p])
        out.append((n, s, hex(c)))
    return out

a = run('stock')
b = run('ours')
for x, y in zip(a, b):
    print(x, 'OK' if x == y else 'DIFF %s' % (y,))
