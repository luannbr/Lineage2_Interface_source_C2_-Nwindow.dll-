"""Parser of the class declarations in include/nwindow/*.h (the conventions of docs/CONVENCOES.md).

Only what the C1 -> C2 generator needs:
  class X : public Y { NW_DECLARE_CLASS(X, Y, flags, StaticClassVA) ... };
  fields     `TYPE name[arr]; // +0xOFF text`
  methods    `... name(args); // [slot] 0xVA [text]`   (virtual, ctor, plain) and the commented
             destructor `// ~X   // 0xVA`
  asserts    NW_ASSERT_OFFSET(X, field, 0xOFF); NW_ASSERT_SIZE(X, 0xSIZE);
Everything else in the file is kept as it is (lines are never rewritten by the parser).
"""
from __future__ import annotations

import re
from dataclasses import dataclass, field
from pathlib import Path

CLASS_RE = re.compile(r"^(class|struct)\s+(\w+)\s*(?::\s*public\s+(\w+))?\s*$")
DECL_RE = re.compile(r"NW_DECLARE_(?:ABSTRACT_)?CLASS\(\s*(\w+)\s*,\s*(\w+)\s*,\s*([^,]+?)\s*,\s*(0x[0-9A-Fa-f]+)\s*\)")
FIELD_RE = re.compile(r"^(\s+)(?!return|typedef|enum|friend|static|virtual|public|private|protected|case|using)"
                      r"([A-Za-z_][\w:<>\*\&\s,]*?[\s\*\&])(\w+)(\[[^\]]*\])?\s*;\s*//\s*\+0x([0-9A-Fa-f]+)\b(.*)$")
VA_TAIL = re.compile(r"//\s*(?:(\d+)\s+)?(0x[0-9a-f]{8})\b(.*)$")
PAD_RE = re.compile(r"^(\s+)BYTE\s+(_Unknown\w*)\[\s*(0x[0-9A-Fa-f]+)\s*-\s*(0x[0-9A-Fa-f]+)\s*\];\s*$")
ASSERT_OFF = re.compile(r"^NW_ASSERT_OFFSET\(\s*(\w+)\s*,\s*(\w+)\s*,\s*(0x[0-9A-Fa-f]+)\s*\);")
ASSERT_SIZE = re.compile(r"^NW_ASSERT_SIZE\(\s*(\w+)\s*,\s*(0x[0-9A-Fa-f]+)\s*\);")


@dataclass
class Field:
    line: int
    name: str
    typ: str
    arr: str
    off: int
    indent: str
    tail: str


@dataclass
class Method:
    line: int
    name: str
    kind: str            # virtual | ctor | dtor | method
    slot: int | None
    va: int | None
    text: str


@dataclass
class ClassDecl:
    name: str
    parent: str | None
    open_line: int       # the line of `{`
    close_line: int      # the line of `};`
    decl_line: int = -1
    static_va: int = 0
    fields: list = field(default_factory=list)
    methods: list = field(default_factory=list)
    asserts: list = field(default_factory=list)   # (line, 'OFFSET'|'SIZE', class, name|None, value)


def parse(path: Path):
    lines = path.read_text(encoding="utf-8", errors="replace").split("\n")
    out = []
    i = 0
    n = len(lines)
    while i < n:
        m = CLASS_RE.match(lines[i].rstrip())
        if m and i + 1 < n and lines[i + 1].strip() == "{":
            cd = ClassDecl(m.group(2), m.group(3), i + 1, -1)
            j = i + 2
            depth = 1
            while j < n:
                s = lines[j]
                if s.startswith("};") and depth == 1:
                    cd.close_line = j
                    break
                depth += s.count("{") - s.count("}") if depth > 1 or "{" in s else 0
                depth = max(depth, 1)
                _classify(cd, j, s)
                j += 1
            # asserts follow the class
            k = cd.close_line + 1
            while k < n and (lines[k].startswith("NW_ASSERT") or not lines[k].strip()
                             or lines[k].startswith(("#pragma", "#if", "#else", "#endif", "//"))):
                a = ASSERT_OFF.match(lines[k])
                b = ASSERT_SIZE.match(lines[k])
                if a:
                    cd.asserts.append((k, "OFFSET", a.group(1), a.group(2), int(a.group(3), 16)))
                elif b:
                    cd.asserts.append((k, "SIZE", b.group(1), None, int(b.group(2), 16)))
                elif lines[k].strip() and not lines[k].startswith(("#pragma", "#if", "#else", "#endif", "//")):
                    break
                k += 1
            out.append(cd)
            i = cd.close_line + 1 if cd.close_line > 0 else i + 1
        else:
            i += 1
    return lines, out


def _classify(cd: ClassDecl, j: int, s: str):
    m = DECL_RE.search(s)
    if m:
        cd.decl_line = j
        cd.static_va = int(m.group(4), 16)
        return
    f = FIELD_RE.match(s) if "(" not in s.split("//")[0] else None
    if f:
        cd.fields.append(Field(j, f.group(3), f.group(2).strip(), f.group(4) or "", int(f.group(5), 16),
                               f.group(1), f.group(6)))
        return
    pm = PAD_RE.match(s)
    if pm:
        # a pad written as BYTE _UnknownXX[0xHI - 0xLO]; starts at LO, no offset comment
        cd.fields.append(Field(j, pm.group(2), "BYTE", "[%s - %s]" % (pm.group(3), pm.group(4)), int(pm.group(4), 16),
                               pm.group(1), ""))
        return
    st = s.strip()
    t = VA_TAIL.search(s)
    if st.startswith("//") and "~" in st and t:
        cd.methods.append(Method(j, "~" + cd.name, "dtor", None, int(t.group(2), 16), s))
        return
    if st.startswith("//") or not t or "(" not in st:
        return
    head = st.split("//")[0]
    mm = re.search(r"([~\w]+|operator\s*\S+)\s*\(", head)
    if not mm:
        return
    name = mm.group(1)
    slot = int(t.group(1)) if t.group(1) else None
    kind = "virtual" if head.lstrip().startswith("virtual") else "ctor" if name == cd.name else "method"
    cd.methods.append(Method(j, name, kind, slot, int(t.group(2), 16), s))
