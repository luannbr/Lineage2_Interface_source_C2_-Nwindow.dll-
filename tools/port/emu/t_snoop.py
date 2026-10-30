"""Differential test of the GM snoop code (stock against ours): NCGMSnoopWnd::AddLine (0x100B7260), NCGMSnoopManager::AddLine
(0x100B74F0) and NCGMSnoopManager::OpenSnoop (0x100B7550). The manager and its four windows are models (the id at +0xC0, the
seven check boxes at +0xA4.. with their flag at +0x1C0, the list at +0x9C); the virtual calls (AddString, ShowWindow, SetTitle) and
the dialog functions are stubs that log what they get. Random snoop ids, filters, chat types and texts are run by both.
usage: python tools/port/emu/t_snoop.py <cases> <seed>   (needs build/bin/Release/NWindowDecomp.dll and its .map; NWD_OURS=<dir> tests a snapshot)"""
import sys, random, os, struct
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import emu as _emu
import stubs
from stubs import *
from t_recipe import RecipeEmu, sym

stubs.FN_POP.update({'WAddString': 0xC, 'WShow': 0, 'WSetTitle': 4, 'DlgCtor': 0, 'DlgSetMsg': 4, 'DlgShow': 4, 'DlgDtor': 0})
# the stock functions stubbed and the same ones in ours (a substring of the decorated name)
stubs.STOCK_FN.update({0x10006E60: 'DlgCtor', 0x10006500: 'DlgSetMsg', 0x1005B310: 'DlgShow', 0x100067D0: 'DlgDtor'})
stubs.OURS_FN.update({'DlgCtor': ('NDialogInfo@@', '??0NDialogInfo@@QAE@XZ'), 'DlgSetMsg': ('@NDialogInfo@@', '?SetSystemMessage@NDialogInfo@@'),
                      'DlgShow': ('@NConsoleWnd@@', '?ShowDialog@NConsoleWnd@@'), 'DlgDtor': ('NDialogInfo@@', '??1NDialogInfo@@')})


class SnoopEmu(RecipeEmu):
    def __init__(self, kind):
        # ours has no out-of-line ~NDialogInfo (it is inline): leave the name out for it
        if kind == 'ours':
            stubs.OURS_FN.pop('DlgDtor', None)
        super().__init__(kind)
        self.wvt = self.alloc(0x200)
        self.lvt = self.alloc(0x200)
        for vt, slot, name, off in ((self.wvt, 43, 'WShow', 0xC000), (self.wvt, 50, 'WSetTitle', 0xC010),
                                    (self.lvt, 69, 'WAddString', 0xC020)):
            addr = STUB_BASE + off
            self.uc.mem_write(addr, b'\xC3')
            self.stubs[addr] = ('fn:' + name, None)
            self.write32(vt + 4 * slot, addr)

    def do_fn(self, nm):
        t = self.this()
        esp = self.reg(UC_X86_REG_ESP)
        a = lambda i: self.read32(esp + 4 + 4 * i)
        if nm == 'WAddString':
            self.log.append(('AddString', t, self.wstr(a(0)), a(1), self.rds32(esp + 12)))
            return None
        if nm == 'WShow':
            self.log.append(('ShowWindow', t))
            return None
        if nm == 'WSetTitle':
            self.log.append(('SetTitle', t, self.wstr(a(0))))
            return None
        if nm == 'DlgCtor':
            self.uc.mem_write(t, bytes(0x78))
            self.write32(t, 0xFFFFFFFF)
            self.log.append(('DlgCtor',))
            return t
        if nm == 'DlgSetMsg':
            self.log.append(('DlgSetMsg', a(0)))
            return None
        if nm == 'DlgShow':
            self.log.append(('DlgShow', [self.rds32(a(0) + o) for o in range(0, 0x20, 4)]))
            return None
        if nm == 'DlgDtor':
            return None
        return super().do_fn(nm)


def make_manager(e, c):
    mgr = e.alloc(0xA8)
    e.uc.mem_write(mgr, bytes(0xA8))
    wins = []
    for i in range(4):
        w = e.alloc(0xC4)
        e.uc.mem_write(w, bytes(0xC4))
        e.write32(w, e.wvt)
        lst = e.alloc(0x20)
        e.write32(lst, e.lvt)
        e.write32(w + 0x9C, lst)
        for k in range(7):
            cb = e.alloc(0x1D0)
            e.uc.mem_write(cb, bytes(0x1D0))
            e.write32(cb + 0x1C0, c['checks'][i][k])
            e.write32(w + 0xA4 + 4 * k, cb)
        e.write32(w + 0xC0, c['ids'][i] & 0xFFFFFFFF)
        e.write32(mgr + 0x98 + 4 * i, w)
        wins.append(w)
    return mgr, wins


def rand_case(r):
    return {
        'ids': [r.choice([-1, -1, 7, 8, 9, 10, 11]) for _ in range(4)],
        'checks': [[r.choice([0, 1, 1, 2]) for _ in range(7)] for _ in range(4)],
        'id': r.choice([7, 8, 9, 10, 11, 12, 13, -1]),
        'type': r.choice([0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, -1]),
        'color': r.choice([0xFFFFFFFF, 0xFFFF7200, 0xFF00FF00]),
        'text': r.choice(['hello', 'a: b', '']),
        'name': r.choice(['Zubei', 'Somebody Long Name']),
        'extra': r.choice([0, 0, 5]),
    }


def run(kind, which, cases):
    e = SnoopEmu(kind)
    if kind == 'stock':
        fn = {'wline': 0x100B7260, 'mline': 0x100B74F0, 'open': 0x100B7550}[which]
    else:
        fn = sym(e, {'wline': '?AddLine@NCGMSnoopWnd@@', 'mline': '?AddLine@NCGMSnoopManager@@', 'open': '?OpenSnoop@NCGMSnoopManager@@'}[which])
    e.heap_mark = e.heap_ptr
    outs = []
    for c in cases:
        e.heap_ptr = e.heap_mark
        e.log.clear()
        mgr, wins = make_manager(e, c)
        text = e.put_wstr(c['text'])
        name = e.put_wstr(c['name'])
        buf = e.alloc(0x200)
        if which == 'wline':
            e.call(fn, this=wins[0], args=[text, c['color'], c['type'] & 0xFFFFFFFF, c['extra']])
        elif which == 'mline':
            e.call(fn, this=mgr, args=[c['id'] & 0xFFFFFFFF, text, c['color'], c['type'] & 0xFFFFFFFF, c['extra']])
        else:
            e.call(fn, this=mgr, args=[c['id'] & 0xFFFFFFFF, name])
        ids = [e.rds32(w + 0xC0) for w in wins]
        outs.append((list(e.log), ids))
    return outs


def main(n, seed):
    r = random.Random(seed)
    cases = [rand_case(r) for _ in range(n)]
    for which in ('wline', 'mline', 'open'):
        a = run('stock', which, cases)
        b = run('ours', which, cases)
        bad = sum(1 for x, y in zip(a, b) if x != y)
        for i, (x, y) in enumerate(zip(a, b)):
            if x != y:
                print(which, 'case', i, cases[i], '\n stock', x, '\n ours ', y)
                break
        print(which, 'cases', n, 'differ', bad, 'with log', sum(1 for x in a if x[0]))


if __name__ == '__main__':
    main(int(sys.argv[1]), int(sys.argv[2]))
