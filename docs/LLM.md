# Guide for AI coding agents (LLMs)

Read this before you touch the code. It condenses `README.en.md`, `docs/PLANO.md`, `docs/PLANO_C2.md`,
`docs/CONVENCOES.md`, `docs/notas/C2_Agentes.md`, `docs/notas/C2_Porte.md` and `docs/notas/C2_Fase5.md` (the notes
are in Portuguese except `C2_Agentes.md`; code comments are in English). When this file and the code disagree, the
code and the judges win.

## 1. What the code base is

A C++ rebuild of the Lineage II C2 `NWindow.dll` (the UI DLL; stock build TimeDateStamp `0x4227239B`, VC++ 7.0
`13.00.9466`, md5 `0111f6578f6482f73980f6268221c03e`, ImageBase `0x10000000`, 225 classes, 227 exports). It was
ported from the C1 project (same tools, same method; C1 addresses never apply here). One source, two CMake targets:

| Target | Define | Role |
|---|---|---|
| `NWindowDecomp` | `NW_HOOK_MODE=1` | hook DLL loaded by the stock `NWindow.dll`; each `NW_HOOK_*` puts a `jmp ours` at a stock entry |
| `NWindowStandalone` | `NW_HOOK_MODE=0` | the standalone `NWindow.dll`; what the standalone judges compare with stock |

Both need `-DNWD_GAME_SOURCES=ON` (`NW_GAME_SOURCE=1`) to compile `src/nwindow`; the CMake default `OFF` builds the
hook runtime alone. There is **no** phase-6 feature layer (`NW_FEATURES`, `src/features`, `NWindowL2`) in this
tree: the `#if NW_FEATURES` blocks left in some files are call sites inherited from the C1 project and are never
compiled (their headers are not here). See `docs/MODIFY_INTERFACE.md` before adding new behaviour.

Key places:

| Path | Content |
|---|---|
| `include/nwindow/`, `src/nwindow/` | decompiled game code, **one class per file**, file = class name (`NConsoleWnd` is split into `NConsoleWnd*.cpp` by subject) |
| `include/ue2/` | C2 Core/Engine declarations (`UE2Core.h`, `UE2Engine.h`, generated vtables in `gen/*.vt.inc`) |
| `src/re/` | hook runtime: `NwHook.h` (macros), installer, log, ini, `NwCrtCompat.cpp` |
| `src/standalone/` | `NwStandalone.h` (empty hook macros), `Classes.gen.cpp` (225 `IMPLEMENT_CLASS`), `NwGlobals.cpp`, `NWindow.def`, `NWindowPackage.cpp` (DllMain), `NwGameGuard.cpp`, `NwTrace.cpp`, `NwCrashLog.cpp` |
| `db/` | `functions.csv` (generated, never edit) + hand-written `names.csv`, `vslots.csv`, `verify_notes.csv`, `verify_data_notes.csv`, `verified.csv`, `morto.csv`; port maps `fmap.csv`, `amap.csv`, `layout.json`, `vslots.json` |
| `analysis/` | facts from the stock binary (`classes.json`/`classes.md`: size, parent, vtable of the 225 classes; `eh.json`; `engine/*.json` Engine class layouts; `engine_vtables.txt`); `analysis/ghidra/` (not in git) = Ghidra export, `decomp/<va>.c` and `asm/<va>.asm` per function |
| `tools/port/` | the C1 → C2 port: `packet.py` (work packet of a function), `verify_data.py`, `check_breakers.py`, `emu/` (unicorn differential tests), `cycle.bat` |
| `orig/` | the user's stock binaries (not in git) |

Object model: everything is a UE2 `UObject`. `NCObject : UObject` → `NCCmdTarget` → `NCWnd` (0x98 bytes, the
window tree) → `NControl` (0x1A4, controls) and the game windows `NC*Wnd`. `NConsoleWnd : UL2ConsoleWnd` (0x35B8
bytes, 335 vtable slots: the engine's network/UI callbacks) owns the window manager (`NCVirtualWndMain`) and the
in-game desktop `NCConsole` (`GetDesktop()`), whose `OnCreate` builds every in-game window.

## 2. How a function is (re)implemented

1. Pick a function in `db/functions.csv` (`status`: `done` hooked, `todo` without a hook and without a reason,
   `conv`/`fase5`/`lib`/`part` hookless for a reason). Today the `todo` ones are the nProtect GameGuard library
   (not ported on purpose) and a handful of tiny folded bodies.
2. Read `analysis/ghidra/decomp/<va>.c`, then **check the assembly** (`analysis/ghidra/asm/<va>.asm`,
   `python tools/disasm.py <va|Class::Method>`): Ghidra gets parameter counts wrong (check `ret N`) and does not
   know vtable slots. Helpers: `tools/xrefs.py`, `tools/vtable.py <Class>`, `tools/engine_vtables.py <Class>`,
   `python tools/port/packet.py <Class::Method|va>` (the C1 body, the C1 → C2 instruction diff with the C2 field
   names, Ghidra's C2 decompile; needs `NWD_C1_ROOT`).
3. Write it in `src/nwindow/<Class>.cpp`, UE2 style, **C2** address in a comment, hook macro right after:

   ```cpp
   // 0x10097850 (vslot 36) - C2 order: the unnamed button A, Help, button B, Options, Restart, Exit
   INT NCSystemMenuWnd::OnCreate()
   {
       NCWnd::OnCreate();
       ...
       return 1;
   }
   NW_HOOK_METHOD(0x10097850, NCSystemMenuWnd, OnCreate)
   ```

   Declarations carry `// 0x...` too; invented names carry `(name ours)` (and `(nome nosso)` in `db/names.csv`).
   Layouts: `NW_ASSERT_OFFSET(Class, Field, 0x..)` for every known field and `NW_ASSERT_SIZE(Class, 0x..)` = the
   `IMPLEMENT_CLASS` size from `analysis/classes.md`; unknown fields are `UnknownXX` with the offset in the name.
   Each class has `NW_DECLARE_CLASS(Class, Super, Flags, StaticClassVA)` and `NW_CHECK_VTABLE(Class, StockVtableVA,
   Slots)`.
4. Run `tools\port\cycle.bat` (with `NWD_CLIENT_SYSTEM` set) until green (section 4).
5. Only after an in-game test: a line in `db/verified.csv`.

Hook macros (`src/re/NwHook.h`; empty in standalone, `src/standalone/NwStandalone.h`):

| Macro | Use |
|---|---|
| `NW_HOOK_METHOD(va, Class, Method)` | virtual or non-virtual method (installs a non-virtual forwarder, so a virtual never loops through the stock vtable) |
| `NW_HOOK_METHOD_SIG(va, Class, Method, Sig)` | overloaded method, `Sig = R (Class::*)(A...)` |
| `NW_HOOK_VARARGS(va, Class, Method, Sig)` | `...` method (cdecl, `this` on the stack); must not be virtual |
| `NW_HOOK_FUNC(va, Func)` | free/static function, same convention as stock |
| `NW_HOOK_CTOR(va, Class)`, `NW_HOOK_CTOR_ARGS(va, Class, (Params), (Args))`, `NW_HOOK_DTOR(va, Class)` | constructors / destructor |
| `NW_NAKED ... NW_STOCK(va)` | declared but not decompiled: jumps into stock code (hook mode only; an `int3` in standalone) |
| `NW_UNPORTED()` / `NW_UNPORTED_NORET()` | a body nobody ported (an `int3`; `check_breakers.py` makes sure no hook reaches one) |
| `nw::StockCall<R>(va, obj, args...)`, `nw::Field<T>(obj, off)`, `nw::Addr(va)` | scaffolding for code not declared yet (hook mode only) |
| `NW_GLOBAL(Type, va)` | a stock global (hook mode); standalone defines it in `src/standalone/NwGlobals.cpp` (`tools/gen_globals.py`) |
| `NW_TRACE(fmt, ...)` | standalone only: one line in `NWindow_trace.log`; empty in hook mode |

Globals are declared in two branches: `#if NW_HOOK_MODE #define GX NW_GLOBAL(T, va) #else extern T GX; #endif`.
`NW_STOCK`, `nw::StockCall` and `NW_GLOBAL` have **no definition** in standalone: if one escapes `#if
NW_HOOK_MODE`, the standalone link fails and names it (`python tools/standalone_report.py` turns the link errors
into a work list).

## 3. Traps (each one has bitten this project)

- **C1 addresses are wrong everywhere** in C2 (globals, tables, `StockCall` targets, vtable slots: `UObject` has 25
  slots in C2, not 24). Take every address from the C2 assembly; `db/amap.csv` maps C1 → C2 data addresses.
- **The C1 → C2 function map can pair a function with the wrong C2 twin** where C2 inserted functions: a body that
  verifies can still be the wrong function; compare `guard` names and the slots called.
- **C2 changed many `TCHAR[]` buffers into `FString`** and moved fields; `FL2GameData`, `User`, `NItemEntry` are
  stale in places. `verify_data.py` names the offsets the stock code uses that ours does not.
- **Never hook a scalar deleting destructor**: the stock one calls the (already hooked) destructor.
- **Virtual overloads are laid out in reverse declaration order** by MSVC: declare the higher slot first (see
  `UCanvas` in `include/ue2/UE2Engine.h`). `NW_CHECK_VTABLE` + `verify_standalone --checks source` catch it.
- **Calling conventions**: thiscall through `__fastcall` with a dummy `edx`; varargs members are cdecl; `ret N` in
  the stock assembly is the truth for the stack bytes. `verify_hooks.py` compares it.
- **Register assumptions (`/GL`)**: a stock caller may keep `ecx`/`edx`/`eax` alive across a direct call it "knows"
  does not touch them. `tools/regcheck.py --game-src` lists such callers: hook them together with the callee, or
  leave the callee stock.
- **ICF folding**: identical functions share one address in the stock image. One hook serves all; the `notes`
  column shows the cases. Folded bodies that belong to several classes must be defined in each class for the
  standalone (`NCDialogBox`/`NCGaraDialogBox` was such a case).
- **Engine imports** must produce exactly the exported decorated name (`CORE_API`/`ENGINE_API`). `TCHAR` is
  `unsigned short` (`/Zc:wchar_t-`), which makes `PBG`/`PAG` manglings match. The C2 `Engine.dll` has more than
  8,192 exports: `tools/gen_import_defs.py` reads the table by hand.
- **Compiler flags are part of the ABI**: `/Zc:wchar_t- /GR- /EHa /arch:IA32 /Oy-`, static CRT; the standalone
  objects use `/Ob1` (limited inlining like the stock build).
- **x87 arithmetic**: integer × float constant / integer must be written in `DOUBLE`, or results above 2^24 differ.
- **CRT differences**: keep `legacy_stdio_float_rounding.obj` (`NWD_CRT_COMPAT_OBJS`) and `src/re/NwCrtCompat.cpp`;
  never use UCRT `_wtol/_wtoi/_ttoi/atol/atoi` (use `NWtol`/`NAtol`) nor `iswctype`/`isw*` (use `NIswctype`).
- **The standalone `InternalConstructor`** is `new ((EInternal*)X) TClass`: every class needs a constructor callable
  without arguments, with the defaults the stock `InternalConstructor` pushes (`tools/port/check_internal_ctors.py`).
- **Plain C++ in game code**: no STL, lambdas, `auto` or new templates in `src/nwindow`; UE2 types (`INT`, `DWORD`,
  `TCHAR`, `UBOOL`, `TEXT()`, `FString`, `TArray`, `appMalloc`), `guard/unguard` where the original has them, with
  the original string.
- **Faithful before pretty**: reproduce odd stock behaviour exactly. Fixes are separate, annotated changes.
- **Layout is frozen**: never add a field or a new virtual slot to a stock class. Its size is registered with Core
  (`UClass` size from `classes.json`) and stock code / the engine index into it.
- **Inline helpers the stock compiler inlined** are `FORCEINLINE` (`NCWnd::CreateChild`, UE2 `operator new`).
- **Worktrees**: `tools/port/new_worktree.sh` links `analysis/ghidra` and `ghidra/census` as junctions; remove a
  worktree only with `tools/port/remove_worktree.sh`, never with a recursive delete.
- **Edits with backslashes**: do not edit files through shell one-liners (Git Bash rewrites arguments that look
  like paths, `//` included); use a file-editing tool or a script file.

## 4. The judges and what "green" means

`tools\port\cycle.bat` runs `python tools/cycle.py --game-source --no-install` — build → smoke (`nwd_smoke.exe`, every
hook `ok` in `build/smoke_system/NWindowDecomp.log`) → `verify_hooks.py` (ret bytes, vtable slots, imports per
function vs stock; accepted differences only with a reason in `db/verify_notes.csv`) → `regcheck.py --game-src` →
`build_db.py --game-src` — then `tools/port/verify_data.py` (constants, field offsets, strings per function;
`db/verify_data_notes.csv`) and `tools/port/check_breakers.py` (no hook reaches an `int3` body). The standalone
steps are `python tools/cycle.py --game-source --standalone --no-install`: `verify_standalone.py --checks source`
→ `NWindowStandalone` build → `verify_standalone.py --dll` (PE, imports, resources, exports, classes, vtables) →
`smoke_standalone.py` (loaded like the engine loads it, output identical to stock). Extra proofs:
`python tools/gen_globals.py --check <dll> <map>` (globals read back from the standalone), the unicorn tests in
`tools/port/emu/t_*.py`. `difftest.py` is not usable on C2 yet (its cases use C1 addresses; it refuses the image).

**Green = exit code 0** of those commands, with no new compiler warning, `0 unexplained` in verify, 0 regcheck
problems, 0 breakers. A difference is never "fixed" by editing a notes file without understanding it.
Prerequisites: `orig/NWindow.dll`, `orig/Core.dll`, `orig/Engine.dll`; the Ghidra export
(`bash tools/ghidra/run_full.sh`, `NWD_GHIDRA_HOME`); a copy of a C2 `system` folder in `NWD_CLIENT_SYSTEM`. Do not
commit what the cycle regenerates (`db/functions.csv`, `db/verify_data.csv`, `docs/PROGRESSO.md`) together with a
code change unless asked. The cycle does **not** draw the UI: visual changes need an in-game test by a human.

## 5. Adding new behaviour

There is no feature layer yet. If you add one, follow the C1 project's rules (described in
`docs/MODIFY_INTERFACE.md`, section 2): a define (`NW_FEATURES`) that only a new CMake target sets, feature code in
its own files under `namespace nwf`, only the call site in decompiled code (`#if NW_FEATURES` + `// phase 6: ...`),
state in globals of the feature's `.cpp` (never new fields in stock classes), settings in the registry, no file
next to the DLL. Hook mode and `NWindowStandalone` must stay byte-for-byte what the judges compare with stock.
The leftover `#if NW_FEATURES` call sites in `src/nwindow` name C1 features whose code is not in this tree and
whose addresses/layouts are C1's: do not turn the define on without porting or deleting them.

## 6. Working rules

- **Stay faithful to the original first.** Reproduce, then improve in a separate, annotated change.
- **Prove every change with the judges**; never weaken a judge or a notes file to make a change pass.
- **Small commits**, one subject each, message saying what was checked.
- Do not install into the folder the user plays from; in-game tests only with the user's consent.
- Never commit game binaries (`orig/`, `*.dll`, `*.exe`, game packages; see `.gitignore`).
- Keep the address comment and the `NW_HOOK_*` together; keep `(name ours)` until something confirms a name.
- When unsure about a stock behaviour, read the assembly again rather than guessing; write down what was verified
  and what was inferred.
