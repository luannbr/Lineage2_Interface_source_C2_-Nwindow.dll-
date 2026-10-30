"""Differential test of the viewers' FillItemInfo (stock 0x100aafe0 of NCPawnViewrWnd and 0x1009c530 of NCNpcViewrWnd against ours):
random item records of the three classes (weapon, armor, etc, and an unknown class) are turned into an NItemInfo by both; the whole
0x17A8 bytes of the NItemInfo, the calls of the stubbed services (FindTexture, the record's virtuals) and the string functions'
results must be equal.
usage: python tools/port/emu/t_viewitem.py <cases> <seed>   (needs build/bin/Release/NWindowDecomp.dll and its .map; NWD_OURS=<dir> tests a snapshot)"""
import sys, random, os, struct
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import emu as _emu
import stubs
from stubs import *
from t_recipe import RecipeEmu, sym

stubs.IMPORT_POP.update({'??DL2FName@@QBEPBGXZ': 0, '??DFName@@QBEPBGXZ': 0})
stubs.FN_POP.update({'RecSlot0': 0xC, 'RecSlot2': 0xC})

INFO_SIZE = 0x17A8
REC_SIZE = 0x230


class ViewItemEmu(RecipeEmu):
    def __init__(self, kind):
        super().__init__(kind)
        # a vtable of three entries: slot 0 and 2 are stubs, slot 1 is never called
        self.vtbl = self.alloc(16)
        for slot, name, off in ((0, 'RecSlot0', 0xA000), (2, 'RecSlot2', 0xA010)):
            addr = STUB_BASE + off
            self.uc.mem_write(addr, b'\xC3')
            self.stubs[addr] = ('fn:' + name, None)
            self.write32(self.vtbl + 4 * slot, addr)
        self.write32(self.vtbl + 4, 0)

    def name_of(self, idx):
        return 'N%d' % idx

    def do_import(self, nm):
        if nm in ('??DL2FName@@QBEPBGXZ', '??DFName@@QBEPBGXZ'):
            idx = self.read32(self.this())
            self.log.append((nm, idx))
            return self.static_str(self.name_of(idx))
        return super().do_import(nm)

    def do_fn(self, nm):
        if nm in ('RecSlot0', 'RecSlot2'):
            esp = self.reg(UC_X86_REG_ESP)
            out = self.read32(esp + 4)
            a = self.rds32(esp + 8)
            b = self.rds32(esp + 12)
            rec = self.this()
            klass = self.read32(rec + 4)
            idx = (3000 if nm == 'RecSlot0' else 5000) + 17 * a + b + 100 * klass
            self.write32(out, idx)
            self.log.append((nm, a, b, klass))
            return out
        return super().do_fn(nm)


def rand_record(r):
    klass = r.choice([0, 1, 2, 2, 0, 1, 7])
    ints = {}
    for off in range(0x08, REC_SIZE, 4):
        ints[off] = r.randint(-5, 400)
    for off in (0x34, 0x38, 0x3C, 0x40, 0x44):
        ints[off] = r.randint(0, 900)
    ints[0x04] = klass
    ints[0x88] = r.choice([0, 1, 2, 5, 5, 2, 8])
    desc = r.choice([None, '', 'A plain item.', 'Line one.\\nLine two', 'x' * 60])
    return {'ints': ints, 'desc': desc}


def make_record(e, c):
    p = e.alloc(REC_SIZE)
    e.uc.mem_write(p, bytes(REC_SIZE))
    for off, v in c['ints'].items():
        e.write32(p + off, v)
    e.write32(p, e.vtbl)
    if c['desc'] is None:
        e.write32(p + 0x48, 0); e.write32(p + 0x4C, 0); e.write32(p + 0x50, 0)
    else:
        e.fs_set(p + 0x48, c['desc'])
    return p


def run(kind, which, cases):
    e = ViewItemEmu(kind)
    if kind == 'stock':
        fn = {'pawn': 0x100AAFE0, 'npc': 0x1009C530}[which]
    else:
        cls = {'pawn': 'NCPawnViewrWnd', 'npc': 'NCNpcViewrWnd'}[which]
        fn = sym(e, '?FillItemInfo@%s@@' % cls)
    e.heap_mark = e.heap_ptr
    outs = []
    for c in cases:
        e.heap_ptr = e.heap_mark
        e.log.clear()
        wnd = e.alloc(0x400)
        e.uc.mem_write(wnd, bytes(0x400))
        info = e.alloc(INFO_SIZE)
        e.uc.mem_write(info, bytes(INFO_SIZE))
        rec = make_record(e, c)
        e.call(fn, this=wnd, args=[rec, info])
        outs.append((bytes(e.uc.mem_read(info, INFO_SIZE)), list(e.log)))
    return outs


def main(n, seed):
    r = random.Random(seed)
    cases = [rand_record(r) for _ in range(n)]
    for which in ('pawn', 'npc'):
        a = run('stock', which, cases)
        b = run('ours', which, cases)
        bad = 0
        for i, (x, y) in enumerate(zip(a, b)):
            if x != y:
                bad += 1
                if bad <= 3:
                    print(which, 'case', i, 'class', cases[i]['ints'][4], 'desc', cases[i]['desc'])
                    if x[0] != y[0]:
                        for off in range(0, INFO_SIZE, 2):
                            if x[0][off:off + 2] != y[0][off:off + 2]:
                                print('  info differs at 0x%x: stock %s ours %s' % (off, x[0][off:off + 8].hex(), y[0][off:off + 8].hex()))
                                break
                    if x[1] != y[1]:
                        for j, (p, q) in enumerate(zip(x[1], y[1])):
                            if p != q:
                                print('  log differs at', j, '\n   stock', p, '\n   ours ', q)
                                break
                        else:
                            print('  log lengths', len(x[1]), len(y[1]))
        print(which, 'cases', n, 'differ', bad)


if __name__ == '__main__':
    main(int(sys.argv[1]), int(sys.argv[2]))
