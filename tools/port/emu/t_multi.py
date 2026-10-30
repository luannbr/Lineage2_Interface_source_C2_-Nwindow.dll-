# Differential test of NCMultiEdit (the multi-line box) and its line list: OnKeyDown, OnChar, InsertChar, InsertString,
# RecalcRows, CountRows, GetRowStart / GetRowEnd / GetRowLine, SetCaretX, NLineList::GetLine / RemoveAt / Clear /
# TotalLength. The stock function and ours run side by side under unicorn on random boxes (a random list of lines, a
# caret, a composition), with the console, the text measurement, the Thai splitter and the allocator stubbed alike on
# both sides; the service calls are logged and compared, and so is everything the box owns afterwards.
# Written by the w2edit lane; see docs/notas/C2_Porte.md and t_edit.py.
#   python tools/port/emu/t_multi.py [cases] [seed]
import sys, os, random
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import stubs
from stubs import *
from t_edit import EditEmu, thai_split, cluster_words, THAI_MARKS, wfmt, char_w

stubs.FN_POP.update({'UpdateScrollBar': 0, 'GMalloc0': 8, 'GFree': 4})

STOCK_M = {'keydown': 0x10024010, 'char': 0x10023940, 'insertchar': 0x10021E10, 'insertstring': 0x10020390,
           'recalc': 0x1001FE50, 'rowstart': 0x10020EA0, 'rowend': 0x10021220, 'rowline': 0x100215E0,
           'caretx': 0x10020D60, 'countrows': 0x1001FC00, 'getline': 0x1001FA10, 'removeat': 0x10021D20,
           'clear': 0x10021DE0, 'totallen': 0x1001F9A0}
OURS_M = {'keydown': '?OnKeyDown@NCMultiEdit@@', 'char': '?OnChar@NCMultiEdit@@', 'insertchar': '?InsertChar@NCMultiEdit@@',
          'insertstring': '?InsertString@NCMultiEdit@@', 'recalc': '?RecalcRows@NCMultiEdit@@',
          'rowstart': '?GetRowStart@NCMultiEdit@@', 'rowend': '?GetRowEnd@NCMultiEdit@@',
          'rowline': '?GetRowLine@NCMultiEdit@@', 'caretx': '?SetCaretX@NCMultiEdit@@',
          'countrows': '?CountRows@NCMultiEdit@@', 'getline': '?GetLine@NLineList@@', 'removeat': '?RemoveAt@NLineList@@',
          'clear': '?Clear@NLineList@@', 'totallen': '?TotalLength@NLineList@@'}
G = dict(Width=0x4C, IMEManager=0x1A4, Lines=0x1BC, Comp=0x1E0, CharH=0x1F0, LineGap=0x1F4, CaretLine=0x1FC,
         CaretCol=0x200, bCaretOn=0x204, VisibleRows=0x208, TopLine=0x20C, NumLines=0x210, CaretRow=0x214, CaretX=0x218,
         bCompRow=0x21C, TotalRows=0x26C, bFocus=0x40)


class MultiEmu(EditEmu):
    def __init__(self, kind):
        super().__init__(kind)
        self.mvt = 0x1010E080 if kind == 'stock' else [v for s, v in self.symbols.items() if s == '??_7NCMultiEdit@@6B@'][0]
        for nm, va_off in (('UpdateScrollBar', 0x110),):
            stub = STUB_BASE + 0xA000
            self.uc.mem_write(stub, b'\xC3')
            self.stubs[stub] = ('fn:' + nm, None)
            self.write32(self.mvt + va_off, stub)
        for i, nm in enumerate(('GMalloc0', 'GFree')):
            stub = STUB_BASE + 0xA100 + 16 * i
            self.uc.mem_write(stub, b'\xC3')
            self.stubs[stub] = ('fn:' + nm, None)
            self.write32(self.gmalloc_vt + (0 if nm == 'GMalloc0' else 8), stub)
        # the allocator's other slots stay zero

    def do_import(self, nm):
        if nm == '?appSprintf@@YAHPAGPBGZZ':
            dst, fmt = self.arg(0), self.wstr(self.arg(1))
            s = wfmt(self, fmt, [self.stack_arg(2 + i) for i in range(4)])
            self.write_wstr_at(dst, s)
            return len(s)
        if nm == '?Insert@FArray@@QAEXHHH@Z':
            raise RuntimeError('FArray::Insert')
        return super().do_import(nm)

    def do_fn(self, nm):
        if nm == 'UpdateScrollBar':
            self.log.append(('UpdateScrollBar',))
            return 1
        if nm == 'GMalloc0':
            p = self.alloc(self.arg(0))
            self.uc.mem_write(p, bytes(self.arg(0)))
            self.log.append(('Malloc', self.arg(0)))
            return p
        if nm == 'GFree':
            self.log.append(('Free',))
            return None
        return super().do_fn(nm)


def addrs(e):
    if e.kind == 'stock':
        return STOCK_M
    out = {}
    for k, pre in OURS_M.items():
        c = [v for s, v in e.symbols.items() if s.startswith(pre) and 'unwind' not in s and 'ehhandler' not in s]
        assert len(c) == 1, (k, c)
        out[k] = c[0]
    return out


def make_box(e, st):
    p = e.alloc(0x280)
    e.uc.mem_write(p, bytes(0x280))
    e.write32(p, e.mvt)
    mgr = e.alloc(0x1C0)
    e.uc.mem_write(mgr, bytes(0x1C0))
    e.write32(p + G['IMEManager'], mgr)
    for k in ('Width', 'CharH', 'LineGap', 'CaretLine', 'CaretCol', 'bCaretOn', 'VisibleRows', 'TopLine', 'NumLines',
              'CaretRow', 'CaretX', 'bCompRow', 'TotalRows', 'bFocus'):
        e.write32(p + G[k], st[k] & 0xFFFFFFFF)
    e.fs_set(p + G['Comp'], st['Comp'])
    lines = st['lines']
    nodes = []
    for t in lines:
        n = e.alloc(0x14)
        e.uc.mem_write(n, bytes(0x14))
        e.fs_set(n, t)
        nodes.append(n)
    for i, n in enumerate(nodes):
        e.write32(n + 0xC, nodes[i - 1] if i > 0 else 0)
        e.write32(n + 0x10, nodes[i + 1] if i + 1 < len(nodes) else 0)
    L = p + G['Lines']
    e.write32(L, len(nodes)); e.write32(L + 8, nodes[0]); e.write32(L + 0x10, nodes[-1])
    ci = st['cur']
    e.write32(L + 0xC, nodes[ci]); e.write32(L + 0x14, ci)
    return p


def dump_box(e, p):
    d = {}
    L = p + G['Lines']
    d['Count'] = e.rds32(L)
    texts = []
    n = e.read32(L + 8)
    idx = {}
    i = 0
    while n and i < 50:
        idx[n] = i
        texts.append(e.fs_get(n))
        n = e.read32(n + 0x10)
        i += 1
    d['texts'] = texts
    d['CurIndex'] = e.rds32(L + 0x14)
    d['Current'] = idx.get(e.read32(L + 0xC), 'x%x' % (e.read32(L + 0xC) and 1))
    d['Tail'] = idx.get(e.read32(L + 0x10), 'x%x' % (e.read32(L + 0x10) and 1))
    d['Head'] = idx.get(e.read32(L + 8), 'x%x' % (e.read32(L + 8) and 1))
    for k in ('CaretLine', 'CaretCol', 'CaretRow', 'CaretX', 'TopLine', 'NumLines', 'TotalRows', 'bCompRow', 'VisibleRows'):
        d[k] = e.rds32(p + G[k])
    d['Comp'] = e.fs_get(p + G['Comp'])
    return d, idx


def call(e, A, p, c):
    op = c['op']
    if op == 'key':
        return e.call(A['keydown'], this=p, args=[c['vk'], 0])
    if op == 'char':
        return e.call(A['char'], this=p, args=[c['ch'], 0])
    if op == 'insertchar':
        return e.call(A['insertchar'], this=p, args=[c['ch']])
    if op == 'insertstring':
        return e.call(A['insertstring'], this=p, args=[e.put_wstr(c['text']), c['res']])
    if op == 'recalc':
        return e.call(A['recalc'], this=p, args=[c['par']])
    if op == 'rowstart':
        return e.call(A['rowstart'], this=p, args=[c['row']])
    if op == 'rowline':
        return e.call(A['rowline'], this=p, args=[c['row']])
    if op == 'rowend':
        return e.call(A['rowend'], this=p, args=[c['start'], c['line']])
    if op == 'caretx':
        return e.call(A['caretx'], this=p, args=[c['line'], c['col']])
    if op == 'countrows':
        return e.call(A['countrows'], this=p, args=[c['line'], c['bcomp']])
    L = p + G['Lines']
    if op == 'getline':
        return e.call(A['getline'], this=L, args=[c['idx'] & 0xFFFFFFFF])
    if op == 'removeat':
        return e.call(A['removeat'], this=L, args=[c['idx'] & 0xFFFFFFFF])
    if op == 'clear':
        return e.call(A['clear'], this=L)
    return e.call(A['totallen'], this=L)


def run_case(e, A, c):
    mark = len(e.log)
    e.set_lang(c['lang'])
    e.ime_type = c['ime']
    e.not_composing = c['nc']
    p = make_box(e, c['st'])
    outs = []
    for step in c['steps']:
        r = call(e, A, p, step)
        d, idx = dump_box(e, p)
        if step['op'] == 'getline':
            r = idx.get(r, 'null' if r == 0 else 'x')
        outs.append({'ret': r if step['op'] not in ('removeat', 'clear', 'insertchar', 'insertstring', 'caretx', 'recalc') else None,
                     'state': d, 'log': e.log[mark:]})
        mark = len(e.log)
    return outs


def rand_line(r, lang, last):
    alpha = 'abMW 01.' + ('กดัิ่' if lang == 5 else '')
    t = ''.join(r.choice(alpha) for _ in range(r.choice([0, 1, 3, 6, 10, 14, 20])))
    return t if last else t + '\n'


def rand_case(r):
    lang = r.choice([0, 1, 5, 5])
    n = r.choice([1, 1, 2, 3, 5])
    lines = [rand_line(r, lang, i == n - 1) for i in range(n)]
    cl = r.randrange(n)
    cc = r.randint(0, len(lines[cl]) - (1 if lines[cl].endswith('\n') else 0))
    st = dict(Width=r.choice([60, 80, 120]), CharH=13, LineGap=2, CaretLine=cl, CaretCol=cc, bCaretOn=r.randint(0, 1),
              VisibleRows=r.choice([2, 3, 5]), TopLine=0, NumLines=n, CaretRow=0, CaretX=0, bCompRow=0, TotalRows=1,
              bFocus=r.randint(0, 1), lines=lines, cur=r.randrange(n),
              Comp=r.choice(['', '', '', 'ab', 'Mก']))
    c = dict(lang=lang, ime=r.choice([1, 1, 18, 9, 10]) if lang != 5 else r.choice([1, 13, 18]),
             nc=r.choice([1, 1, 1, 1, 0]), st=st)
    steps = [{'op': 'recalc', 'par': r.randint(0, 1)}]
    for _ in range(r.choice([1, 2, 3, 4, 6, 8])):
        op = r.choice(['key', 'key', 'key', 'key', 'key', 'key', 'insertchar', 'insertstring', 'recalc', 'rowstart', 'rowend', 'rowline',
                       'caretx', 'countrows', 'getline', 'getline', 'removeat', 'clear', 'totallen', 'char'])
        s = {'op': op}
        if op == 'key':
            s['vk'] = r.choice([8, 8, 0xD, 0x23, 0x24, 0x25, 0x25, 0x26, 0x27, 0x27, 0x28, 0x28, 0x2E, 0x2E])
        elif op in ('insertchar', 'char'):
            s['ch'] = r.choice([0x41, 0x4D, 0x20, 0xA, 0xD, 0xE01, 0xE31, 0xE48, 0x30, 5]) if op == 'insertchar' else r.choice([0x41, 0x20, 0xE01, 0xE31, 0xE48, 0xE33, 5, 0x81, 0x90])
        elif op == 'insertstring':
            s['text'] = r.choice(['', 'a', 'MW', 'abc def', 'กั'])
            s['res'] = r.randint(0, 1)
        elif op == 'recalc':
            s['par'] = r.randint(0, 1)
        elif op in ('rowstart', 'rowline'):
            s['row'] = r.randint(-1, 8)
        elif op == 'rowend':
            s['start'] = r.randint(0, 6)
            s['line'] = r.randint(-1, n + 1)
        elif op == 'caretx':
            s['line'] = r.randint(-1, n)
            s['col'] = r.randint(0, 12)
        elif op == 'countrows':
            s['line'] = r.randint(-1, n)
            s['bcomp'] = r.randint(0, 1)
        elif op in ('getline', 'removeat'):
            s['idx'] = r.randint(-2, n + 1)
        steps.append(s)
    c['steps'] = steps
    return c


def main(n, seed):
    r = random.Random(seed)
    cases = [rand_case(r) for _ in range(n)]
    es = MultiEmu('stock')
    eo = MultiEmu('ours')
    As, Ao = addrs(es), addrs(eo)
    es.heap_mark, eo.heap_mark = es.heap_ptr, eo.heap_ptr
    bad = err = 0
    for i, c in enumerate(cases):
        es.heap_ptr, eo.heap_ptr = es.heap_mark, eo.heap_mark
        try:
            a = run_case(es, As, c)
        except Exception as ex:
            a = [{'exc': str(ex)[:160]}]
        try:
            b = run_case(eo, Ao, c)
        except Exception as ex:
            b = [{'exc': str(ex)[:160]}]
        if 'exc' in a[-1] or 'exc' in b[-1]:
            if ('exc' in a[-1]) != ('exc' in b[-1]) or len(a) != len(b):
                bad += 1
                if bad <= 8:
                    print('--- case %d exception mismatch' % i, ascii(c['steps']), a[-1].get('exc'), '|', b[-1].get('exc'))
            err += 1
        elif a != b:
            bad += 1
            if bad <= 8:
                print('--- case', i, ascii({k: v for k, v in c.items() if k not in ('st', 'steps')}))
                print(' state in:', ascii(c['st']))
                print(' steps:', ascii(c['steps']))
                for si, (x, y) in enumerate(zip(a, b)):
                    if x != y:
                        print(' step', si, ascii(c['steps'][si]))
                        for k in x:
                            if x[k] != y[k]:
                                print('  differs:', k)
                                print('   stock:', ascii(x[k])[:700])
                                print('   ours :', ascii(y[k])[:700])
                        break
        es.log.clear(); eo.log.clear()
    print('cases', n, 'differ', bad, 'both raised', err)


if __name__ == '__main__':
    main(int(sys.argv[1]) if len(sys.argv) > 1 else 300, int(sys.argv[2]) if len(sys.argv) > 2 else 1)
