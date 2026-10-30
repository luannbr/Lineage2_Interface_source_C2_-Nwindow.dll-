# How to modify the interface

[Português](MODIFICAR_INTERFACE.md) · **English** · [Русский](MODIFY_INTERFACE.ru.md)

A practical guide to changing the C2 interface in this source: how the windows are built, how to add a feature
layer (this repository **has none**) and step-by-step recipes (a new shortcut bar, a button, a window, a chat
colour, a tool-tip line).

**Read first:** only the two faithful targets exist here, `NWindowDecomp` (hook mode) and `NWindowStandalone`. Both
must stay equal to the original, because that is what the judges compare. The C1 project solved this with a "phase
6": a third target (`NWindowL2`, `NW_FEATURES=1`) and the new code in `src/features/`; in C2 that layer has not been
built yet. The recipes below say how to build it on top of the existing classes; none of them has been implemented
or tested **on C2**. The `#if NW_FEATURES` blocks found in some files (`NCShortCutWnd.cpp`, `NConsoleWndEvents.cpp`,
`NCChatWnd.cpp`...) are call sites inherited from C1: the headers they include are not here, and the addresses and
layouts of those features are C1's.

## 1. How the windows are built

### The hierarchy

| Class | File | Role |
|---|---|---|
| `NCObject : UObject` | `include/nwindow/NCObject.h` | base of everything; `FindTexture(TEXT("Package.Group.Name"))` loads a texture; `SysString(i)`, `PlayInterfaceSound(id)` |
| `NCWnd` (0x98 bytes) | `include/nwindow/NCWnd.h` | every window: position (`LocalX/Y/W/H` relative to the parent, `ScreenX/Y`, `Width`, `Height` absolute), style, children, focus, title bar (`FrameCtrl`), minimised icon, `ToolTipSkin` (new in C2) |
| `NControl : NCWnd` (0x1A4) | `include/nwindow/NControl.h` | every control; has an `ID` and tells its parent with `NWM_COMMAND` (wParam = `ID`) |
| `NCButton`, `NCExButton`, `NCPushButton`, `NCRadioButton` | `include/nwindow/NCButton.h` | buttons with normal/pushed textures (`SetNormalTex`, `SetPushedTex`, `SetText`, `SetTextID`) |
| `NCEditBox`, `NCListBox`, `NCScrollWnd`, `NCComboBox`, `NCTabControl`, `NCSliderBar`, `NCProgressCtrl`, `NCListCtrl`, `NCMultiEdit`, `NCHtmlViewer`... | `include/nwindow/` | the other controls |
| `NC*Wnd` | `include/nwindow/` | the game windows (inventory, chat, shortcut bar, status, clan, stores...) |
| `NCVirtualWndMain` | `include/nwindow/NCVirtualWndMain.h` | owner of every window: registration, z-order, hit tests, focus, message dispatch, cursors |
| `NConsoleWnd : UL2ConsoleWnd` | `include/nwindow/NConsoleWnd.h` | the engine's console (0x35B8 bytes, 335 slots: network and UI events); `GConsole` |
| `NCConsole` | `include/nwindow/NCConsole.h` | the in-game **desktop** (`GConsole->GetDesktop()`): parent of every in-game window, with a pointer to each (`ShortCutWnd`, `ChatWnd`, `InventoryWnd`, `SystemMenuWnd`...) |

### Creating a window or a control

The pattern of the original code (`NCSystemMenuWnd::OnCreate` in `src/nwindow/NCSystemMenuWnd.cpp`):

```cpp
NCButton* Button = new NCButton(TEXT("L2UI.Control.DefaultButton"), TEXT("L2UI.Control.DefaultButton_click"), 0);
Button->ID = NID_SYSMENU_HELP;          // what arrives in the parent's NWM_COMMAND
Button->SetWindowRect(86, 61, 67, 19);  // relative to the parent
CreateChild(Button, NWS_VISIBLE);       // registers it, makes the position absolute, sends WM_CREATE -> OnCreate
Button->SetTextID(0x91);                // text from the client's SysString table
```

- `new` of a package class is UE2's `operator new` (the object is a `UObject`, allocated with the size of the
  registered `UClass`).
- `CreateChild(child, style)` (`NCWnd`, inline) = `AddChildWindow` + `Entry` + screen position + `WM_CREATE`. The
  desktop also uses the hidden variants `CreateHiddenChild` / `CreateChildHidden` (`src/nwindow/NCConsole.cpp`:
  `AddChildWindow`, `HideWindow`, then the attach).
- Styles (`NWS_*` in `NCWnd.h`): `NWS_VISIBLE`, `NWS_TOPMOST` (separate list, on top), `NWS_TITLEBAR`
  (`NCWnd::OnCreate` makes an `NCFrameCtrl` title bar), `NWS_MINIMIZABLE`, `NWS_NOCLOSE`, `NWS_MODAL` (new in C2).
- Destroying: in the owner's `OnDestroy`, `RemoveChildWindow(Child->Entry); delete Child;` (see
  `NCSystemMenuWnd::OnDestroy`). `NCWnd::OnDestroy` only destroys the title bar, the icon and the `ToolTipSkin`.

### Messages, painting, tool tips

- Everything arrives through `WindowProc(Msg, wParam, lParam)` (vslot 25) with Win32 message numbers; `NCWnd`
  dispatches to the virtuals: `OnCreate` (36), `OnDestroy` (37), `OnPaint(NCGDevice*)` (55), `OnDrawToolTip` (56),
  `OnKeyDown`, `OnMouseMove`, `OnLButtonDown/Up`, `OnRButtonDown/Up`, `OnMove` (`WM_MOVE`), `OnMouseWheel`.
- A control tells its parent with `NWM_COMMAND` (`0x501`), `wParam` = the control's `ID`. The parent handles it in
  its `WindowProc` (example: `NCSystemMenuWnd::WindowProc`).
- Painting (`OnPaint`) uses the `NCGDevice` (`include/nwindow/NCGDevice.h`, the C2 signatures in its top comment):
  `SetClip`, `BeginPaint(ScreenX, ScreenY)`, `DrawTexture`, `DrawRect`, `DrawText`, `EndPaint`, `ReleaseClip`.
  Textures are loaded once in `OnCreate` with `FindTexture` (the client's `L2UI.*` packages).
- Tool tip: `OnDrawToolTip(NCGDevice*)` of the window under the mouse, called after painting.

### Console and desktop

`NConsoleWnd` (the object the engine creates from `L2.ini`) receives the server and game events (its 335 slots,
split by subject into `src/nwindow/NConsoleWnd*.cpp`) and owns the `NCConsole` desktop. `NCConsole::OnCreate`
creates every game window (positions from the screen size) and `NCConsole::OnDestroy` destroys them.
`NConsoleWnd::Tick` (`NConsoleWndInput.cpp`) runs every frame.

## 2. How to add a feature layer (not there yet)

The way C1 did it, which keeps the judges valid:

1. **A new target in `CMakeLists.txt`**, leaving the two faithful ones alone. The function
   `nwd_standalone_dll_of(objs target name dir)` already exists for this: make an `OBJECT` library with the same
   sources as `nwd_standalone_objs` plus `src/features/*.cpp`, the same options (`${NWD_CXX_FLAGS} /Ob1`,
   `NW_HOOK_MODE=0`) and `NW_FEATURES=1`, and call `nwd_standalone_dll_of(nwd_l2_objs NWindowL2 NWindow
   "${CMAKE_BINARY_DIR}/l2")`.
2. **The inherited call sites.** With `NW_FEATURES=1` the `#if NW_FEATURES` blocks already in `src/nwindow` start to
   compile and include `features/*.h` files that are not here. Delete the ones you will not port; port the ones you
   want from the [C1 project](https://github.com/luannbr/Lineage2_Interface_source_C1_-Nwindow.dll-)
   (`src/features/`), fixing addresses and layouts for C2 (C1's never apply).
3. **The feature code** in `include/features/<Feature>.h` + `src/features/<Feature>.cpp`, in `namespace nwf` and
   `#if NW_FEATURES`. Decompiled code gets only the call site, in an `#if NW_FEATURES` block with the comment
   `// phase 6: <Feature> ...`; without the block the code must read the same as the original.
4. **State** in globals of the feature's `.cpp`, never in new fields of an original class (its size is registered
   with the engine). **Settings** in the registry (`HKCU\Software\...`), never in a file next to the DLL.
5. **Useful entry points**: `NCConsole::OnCreate`/`OnDestroy` (create/destroy windows), `NConsoleWnd::Tick` (every
   frame), a window's `WindowProc`/`OnPaint`/`OnDrawToolTip`/`OnLButtonUp`, `NConsoleWnd::Say2` (chat). The
   standalone's `DllMain` is in `src/standalone/NWindowPackage.cpp`.
6. **A new class** (a window with fields of its own): in C1 it is `NW_DECLARE_L2_CLASS` / `NW_IMPLEMENT_L2_CLASS`
   (`include/features/NwL2Class.h` of the C1 project, to be ported) plus a line in `tests/standalone/l2_extras.csv`;
   `python tools/verify_standalone.py --dll build/l2/Release/NWindow.dll --l2` accepts exactly those extra classes.
7. **Proof**: the hook-mode and standalone cycles stay green (`tools\port\cycle.bat`,
   `python tools\cycle.py --game-source --standalone --no-install`), the new target passes `verify_standalone.py
   --l2`, and the in-game test is done on a copy of the `system` folder.

## 3. Recipe: a new shortcut bar (as in C5 / Interlude)

Not implemented on C2. The idea (C5's, already done as an experiment on C1) fits C2's classes without changing the
protocol, the server or the textures:

**The idea:** the extra bar **owns no slots**. It is one more `NCShortCutWnd` instance showing another page of the
same shortcut table (`Cols × Pages`, 10 × 10 on the main bar). A shortcut's slot is `page × 10 + position`, a number
the server stores and sends back; a bar that only shows another page changes nothing in the protocol. The class
keeps its size (0x208) and no new `UClass` is needed.

**Steps:**

1. **Feature files:** `include/features/ShortCutBars.h` / `src/features/ShortCutBars.cpp` (`namespace nwf`,
   `#if NW_FEATURES`): `ShortCutBars_Create(NCWnd* Desktop, NCShortCutWnd* Main)`, `ShortCutBars_Destroy`,
   `ShortCutBars_OnTick`, `ShortCutBars_OnCommand`... The bars and the button live in globals of the `.cpp`.
2. **Create** in `NCConsole::OnCreate`, right after the main bar (`ShortCutWnd = new NCShortCutWnd(10, 10, 32);
   SetWindowRect(Width - 45, Height / 2 - 218, 44, 437); CreateChildHidden(this, ShortCutWnd, 0x1022);`): for each
   extra bar, `new NCShortCutWnd(Main->Cols, Main->Pages, Main->IconSize)`, `SetWindowRect` and the same hidden
   registration. The bar's stock `OnCreate` allocates tables of its own (`Items = new NShortCutItem[Cols * Pages]`,
   `Keys = new NShortCutKey[...]`): free them (`delete[]`) and point to the main bar's (`Bar->Items = Main->Items`,
   `Bar->Keys = Main->Keys`). `Bar->Page` = the first page shown. Hide the extra bars' `RotateButton`.
3. **Destroy** in `NCConsole::OnDestroy`, **before** `DestroyChild(this, ShortCutWnd)`: on the extra bars set
   `Items` and `Keys` to `NULL` (they belong to the main bar; otherwise the stock `OnDestroy` frees them twice), then
   `RemoveChildWindow(Bar->Entry)` and `delete Bar`.
4. **Follow the main bar** every frame (from the feature's tick): the same orientation (`bHorizontal`, and
   `UpdateLayout()` — vslot 68 — when it changes; C2 also has the game-pad layout, `NID_SHORTCUT_PAD`), a position
   next to the main bar through `SetWindowRect`, shown only while the main bar is.
5. **Expand button** (optional): an `NCPushButton` child of the main bar, with an `ID` after the stock ones (the
   bar's go up to `0xFF000009`), handled in the `NWM_COMMAND` branch of `NCShortCutWnd::WindowProc`.
6. **Dragging between bars:** in `NCShortCutWnd::OnLButtonUp`, the stock branch that **deletes** a shortcut dropped
   off the bar (`GConsole->RequestShortCutDel`) first checks whether the point is over another bar and, if so, moves
   the shortcut to the slot there (registration + deletion through the stock protocol).
7. **Dragging an extra bar's handle moves the group:** a `WM_MOVE` of an extra bar is passed on to the main bar
   (`NCWnd::Snap` pulls the group; set the main bar's `FrameCtrl` grab point accordingly).

What only the game shows: position, art (the bar background has the turn button's icon painted into the texture)
and dragging. The server's packet filter may drop shortcut registrations sent too fast.

## 4. Recipe: a new button in an existing window

Example: one more button in the system menu (`NCSystemMenuWnd`, six buttons in one column in C2).

1. `include/features/MyButton.h` / `src/features/MyButton.cpp`: `MyButton_Create(NCWnd* Owner)`,
   `MyButton_OnCommand(NCWnd* Owner, WPARAM ID)`, `MyButton_Destroy(NCWnd* Owner)`; the button pointer in a global.
2. Create it at the end of `NCSystemMenuWnd::OnCreate` (`#if NW_FEATURES` / `// phase 6: MyButton` /
   `nwf::MyButton_Create(this);`). Inside: `new NCButton(normal, pushed, 0)`, an `ID` no control of that window uses
   (see the `NID_SYSMENU_*` enum, `0xF0F006..0xF0F023`), `SetWindowRect` inside the window (the buttons sit at x 86,
   23 pixels apart from y 38), `Owner->CreateChild(Button, NWS_VISIBLE)`, `SetText`.
3. Handle the click in the `NWM_COMMAND` branch of `NCSystemMenuWnd::WindowProc`:
   `if (nwf::MyButton_OnCommand(this, wParam)) return 1;` (in `#if NW_FEATURES`).
4. Destroy it in `NCSystemMenuWnd::OnDestroy`, before `NCWnd::OnDestroy()`: `RemoveChildWindow(Button->Entry);
   delete Button;`.
5. If the window has to grow: the `SetWindowRect(Width - 290, 250, 240, 192)` in `NCConsole::OnCreate` and the
   background painting in its `OnPaint` — always in `#if NW_FEATURES`, leaving the stock branch intact.

## 5. Recipe: a new window

1. A window with fields or virtuals of its own is a new class, registered only in the feature target (section 2,
   item 6): `class NCMyWnd : public NCWnd { ... };`.
2. Override the virtuals you need: `OnCreate` (call `NCWnd::OnCreate()` first, load textures with `FindTexture`,
   create the title bar and the controls), `OnDestroy` (destroy the children, then `NCWnd::OnDestroy()`),
   `WindowProc` (`NWM_COMMAND`), `OnPaint`.
3. Create it on the desktop when needed (`SetWindowRect(0, 0, W, H)`, `GConsole->GetDesktop()` as the parent in
   `CreateChild(window, NWS_VISIBLE)`, `HideWindow()`), and show it with `ShowWindow()` +
   `Desktop->BringChildToTop(Entry)`.
4. Models in the code: `NCSystemMenuWnd` (decorated title bar `NCFrameCtrl(1)`, buttons), C2's recipe windows
   (`NCRecipeBookWnd`, `NCRecipeShopWnd`, scrolling lists) and, in the C1 project, `src/features/craft/`.

## 6. Recipe: a new chat colour

The server sends no colour: it sends the chat **type** (`Say2`), and the client picks the colour in
`NConsoleWnd::Say2` (`src/nwindow/NConsoleWndEvents.cpp`). C2 colours types 1 (shout, `0xFFFF7200`), 2 (tell,
`0xFFFF00FF`), 3 (party, `0xFF00FF00`), 4 (clan, `0xFF7D77FF`), 6/7 (petition, `0xFF80FFFF`), 8 (trade,
`0xFFEAA5F5`), 9 (alliance, `0xFF77FF99`) and 10 (announcement); everything else falls into the last `else`, white.

1. In the last `else` of `Say2`, in `#if NW_FEATURES`, ask the feature (`nwf::ChatColors_Find(Type, &Color)` in C1:
   the call site is already there) and call `GetDesktop()->Say(ObjectID, Name, Text, Color, 0)`.
2. The line still goes to the chat as a normal one (All tab, Normal filter). Do not reuse types with side effects
   (TELL adds the speaker to the tell list).
3. Changing the colour of a type C2 **already** colours means changing a faithful branch: do it under
   `#if NW_FEATURES` inside `Say2`, keeping the original colour in the `#else` branch.

## 7. Recipe: a new line in the item tool tip

The full item tool tip is drawn by the routines of `include/nwindow/NItemToolTip.h`, which the item grids include:
`SetTexts`/`SetStoreTexts` fill `ToolTipName` and `ToolTipText` (the text that **sizes** the box) and `DrawLines`
draws the lines, 13 pixels apart (`DrawTypeLines`, then `DrawTailLines`, which ends with the description).

1. Add the line in both places, in `#if NW_FEATURES`: in the sizing text (so the box grows) and in the drawing (for
   example in `DrawTailLines`, before the description, with `DrawLabel`/`DrawValue` and `Y += 13`).
2. The routines are templates instantiated by each grid with different parameters (inventory, paperdoll, trade,
   warehouse, pet, private stores): check every copy you change.
3. For another window's tool tip: its `OnDrawToolTip`; one hook gives the extra height, another draws the line.

## 8. The build and test loop

```bat
set NWD_CLIENT_SYSTEM=<client>\system_nwtest
tools\port\cycle.bat
python tools\cycle.py --game-source --standalone --no-install
```

1. Build with no error and no new warning.
2. Run both cycles: they prove hook mode and the standalone did not change (the judges compare them with the
   original).
3. Test in the game: close the game, keep the `NWindow.dll` of a **copy** of the `system` folder aside and copy the
   DLL you built over it. The cycle does not draw the interface: position, art and dragging only show in the game.
4. To diagnose a standalone crash: `NWindow_crash.log` next to `l2.exe` and
   `python tools\port\map_addr.py --log NWindow_crash.log`.

## 9. Safety rules

- Never add fields or new virtual slots to an original class: its size is registered with the engine and the
  original code indexes into it and its vtable. Overriding a slot the class already inherits is fine. Feature state
  goes into globals of the `.cpp` or into a new class of the feature target.
- In decompiled code, only the call site, in `#if NW_FEATURES` with `// phase 6: ...`; the stock branch stays as it
  is.
- Any run-time patch of another module checks the bytes first and switches the feature off with a log line when
  they do not match.
- Same protocol as the original client: the server does not change; respect its packet filter limits.
- Test on a copy of the client, never in the folder you play from; no game binary goes into the repository.
- Small commits, one subject each, with the cycle green.
