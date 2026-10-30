#pragma once
// NCShortCutWnd - the shortcut bar (UClass "CShortCutWnd", 0x214; desktop +0xE4): Pages pages
// of Cols slots (10 x 10), each an item, a skill or an action the server registered
// (NConsoleWnd hands them over: SetShortCut). F1 .. F10 (or 1 .. 0 while the chat line needs
// Enter) or a click uses the slot of the current page; the arrows turn the pages, the corner
// button turns the bar between vertical (44 x 417) and horizontal. A slot dragged off the bar
// is deleted on the server. Each slot's key waits before it works again: until released,
// until its skill's cool time is over, or a fixed time (shots, attack, pick up).

#include "nwindow/NControl.h"
#include "nwindow/NCCoolTimeIcon.h"
#include "nwindow/NCItemWnd.h"        // NItemEntry: the slots are built on it
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

struct NItemInfo;
class NCPushButton;

// NWM_COMMAND ids of the bar's controls
enum
{
    NID_SHORTCUT_FRAME = (INT)0xFF000003,
    NID_SHORTCUT_NEXT = (INT)0xFF000004,
    NID_SHORTCUT_PREV = (INT)0xFF000005,
    NID_SHORTCUT_ROTATE = (INT)0xFF000008,
    NID_SHORTCUT_PAD = (INT)0xFF000009,      // new in C2 (the id the WindowProc tests; the button's own id is set in OnCreate)
};

// NShortCutItem::Type (names ours)
enum { SHORTCUT_Item = 1, SHORTCUT_Skill = 2, SHORTCUT_Action = 3, SHORTCUT_Macro = 4 /* new in C2 */ };

// A slot of the bar (name ours; 0x1A7C bytes in C2, an array of Cols * Pages): an item entry (the item, skill, action or
// macro the slot holds: Type +0x190C is a SHORTCUT_*, ObjectID +0x1914, ID +0x1918, UnknownC18 +0x191C the skill's level,
// Count +0x1920, Enchant +0x193C, bStackable +0x1944, the tool tip +0x19B0) with the cool time icon and three more
// fields. Layout from the constructors (0x100CF500, 0x100CF560), CopyFrom (0x100CC700) and Clear (0x100CC7F0); the
// C1 struct (0xA80 bytes) had its own copies of those fields.
struct NShortCutItem : public NItemEntry
{
    NCCoolTimeIcon CoolTime;       // +0x1A28
    INT Owner;                      // +0x1A70 items: 1 the player's inventory, 2 the pet's (UseShortCut); copied by CopyFrom, left by Clear
    INT Unknown1A74;               // +0x1A74 NItemInfo +0x1768 (the constructor), copied by CopyFrom, cleared by Clear
    UBOOL bUsed;                   // +0x1A78 the slot holds something (cleared by the constructors and by Clear)

    NShortCutItem();                                                 // 0x100cf500
    NShortCutItem(const NItemInfo& Info);                            // 0x100cf560
    // ~NShortCutItem                                                // 0x100cf5d0 (the cool time's empty destructor 0x100ab5e0,
                                                                     //   then the entry's: its tool tip)
    __declspec(noinline) void CopyFrom(NShortCutItem* Src);                               // 0x100cc700 the entry's Copy, then the slot's fields; the
                                                                     //   cool time stops
    __declspec(noinline) void Clear();                               // 0x100cc7f0 the entry's Reset, the three fields; the cool time stops
};
NW_ASSERT_OFFSET(NShortCutItem, CoolTime, 0x1A28);
NW_ASSERT_OFFSET(NShortCutItem, bUsed, 0x1A78);
NW_ASSERT_SIZE(NShortCutItem, 0x1A7C);

// When the key of a slot works again (name ours; 0x14 bytes, an array of Cols * Pages).
struct NShortCutKey
{
    DWORD Elapsed;                 // +0x00 ms since the last use
    DWORD Interval;                // +0x04 ms it waits (shots, attack: 200, pick up: 500)
    UBOOL bReady;                  // +0x08 the key uses the slot
    UBOOL bOnKeyUp;                // +0x0C ready again when the key is released
    UBOOL bOnCoolTime;             // +0x10 ready again when the cool time is over

    NShortCutKey() : Elapsed(0), Interval(0), bReady(1), bOnKeyUp(0), bOnCoolTime(0) {}

    // 0x100cd610 (name ours): the key that waits for its release is ready again (the mouse's release uses it like the
    // key's)
    __declspec(noinline) void OnKeyUp();
};
NW_ASSERT_SIZE(NShortCutKey, 0x14);

class NCShortCutWnd : public NControl
{
    NW_DECLARE_CLASS(NCShortCutWnd, NControl, 0, 0x101D95D8)

    // C2 layout, from the constructor (0x100cd630), OnCreate (0x100cf630) and the accesses of every member
    // function (names of the C1 header kept where the use is the same). The C1 header's offsets were derived
    // by shifting and were wrong from +0x1A4 on.
    INT bPadMode;                          // +0x1A4 new in C2 (name ours): the game pad layout is on (OnPadClicked; the options
                                           //   window compares it with 1)
    NShortCutKey* Keys;                    // +0x1A8 new NShortCutKey[Cols * Pages]
    NShortCutItem* Items;                  // +0x1AC new NShortCutItem[Cols * Pages]: 0x1A7C bytes each in C2 (C1: 0xA80)
    UTexture* BackTex;                     // +0x1B0 "L2UI.ShortcutWnd.Back"
    UTexture* MaskTex;                     // +0x1B4 "L2UI.ShortcutWnd.mask" (not drawn)
    UTexture* ToolTipTex;                  // +0x1B8 "L2UI.tooltip.mini-tool01"
    UTexture* IconBackTex;                 // +0x1BC "l2ui.nwindow.icon_back" frame of a slot (constructor)
    UTexture* IconClickTex;                // +0x1C0 "l2ui.nwindow.icon_click" over the pressed one (constructor)
    UTexture* ToggleTex;                   // +0x1C4 "L2_SkillTime.ToggleEffect.ToggleEffect001"
    NCPushButton* NextButton;              // +0x1C8 right / up arrow
    NCPushButton* PrevButton;              // +0x1CC left / down arrow
    NCPushButton* RotateButton;            // +0x1D0 vertical / horizontal
    NCPushButton* PadButton;               // +0x1D4 new in C2 (name ours): "L2UI.JoypadShortcutWnd.Joypad_topadv", hidden
                                           //   until the game pad is in use (the options window shows / hides it)
    UBOOL bPressed;                        // +0x1D8 mouse button down on PressedSlot
    UBOOL bDragging;                       // +0x1DC
    UBOOL bHorizontal;                     // +0x1E0
    INT Cols;                              // +0x1E4 slots per page (10)
    INT Pages;                             // +0x1E8 (10)
    INT Page;                              // +0x1EC
    INT IconSize;                          // +0x1F0 (32)
    INT PressedSlot;                       // +0x1F4 -1 = none
    INT HoverSlot;                         // +0x1F8 slot under the mouse, -1 = none
    INT PadHold;                           // +0x1FC new in C2 (name ours): 0, 1 = left hold, 2 = right hold (SetPadHoldL / R)
    UTexture* PadHoldLTex;                 // +0x200 "L2UI.JoypadShortcutWnd.Joypad_HoldL" (new in C2)
    UTexture* PadHoldRTex;                 // +0x204 "L2UI.JoypadShortcutWnd.Joypad_HoldR" (new in C2)

    // the slot and its cool time (the stock Tick works on the cool time at +0x1A28)
    NShortCutItem* ItemAt(INT Index) { return &Items[Index]; }
    NCCoolTimeIcon* ItemCoolTime(INT Index) { return &Items[Index].CoolTime; }

    NCShortCutWnd(INT InCols = 10, INT InPages = 10, INT InIconSize = 32);          // 0x100cd630
    // ~NCShortCutWnd                                                // 0x100cd740

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x100cc820
    virtual INT OnCreate();                                          // 36 0x100cf630
    virtual INT OnDestroy();                                         // 37 0x100cfb00
    virtual void ShowWindow();                                       // 43 0x100cd5d0
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x100ce740
    virtual INT OnDrawToolTip(NCGDevice* Device);                    // 56 0x100cfc80
    virtual INT OnMouseMove(WPARAM wParam, LPARAM lParam);           // 60 0x100ce820
    virtual INT OnLButtonDown(WPARAM wParam, LPARAM lParam);         // 61 0x100ce7c0
    virtual INT OnLButtonUp(WPARAM wParam, LPARAM lParam);           // 62 0x100ce9d0
#if NW_FEATURES
    // phase 6: AutoShot (NWindow_C1 src/inject/AutoShot.cpp): not stock, the stock bar leaves
    // the right button to NCWnd's no-op
    virtual INT OnRButtonDown(WPARAM wParam, LPARAM lParam);         // 58
#endif
    // names ours
    virtual void UpdateLayout();                                     // 68 0x100ccf40 after a turn: the bar's size, the textures and
                                                                     //   the buttons of the vertical / horizontal / game pad layout
    virtual void SetPadHoldL(bool bHold);                            // 69 0x100cd430 PadHold = bHold
    virtual void SetPadHoldR(bool bHold);                            // 70 0x100cd450 PadHold = bHold ? 2 : 0
    virtual INT OnNextClicked();                                     // 71 0x100cf460
    virtual INT OnPrevClicked();                                     // 72 0x100cf480
    virtual INT OnRotateClicked();                                   // 73 0x100cd220 (was 65)
    virtual INT OnPadClicked();                                      // 74 0x100cd470 the game pad layout on / off

    // names ours; the console calls most of them (UL2ConsoleWnd slots 115 .. 119, 263, 264,
    // Tick, HookShortCutKey, ReloadGameTexts, ResetWindowPositions)
    INT UpdateItem(INT ObjectID);                                    // 0x100ccb30 inventory
    INT UpdatePetItem(INT ObjectID);                                 // 0x100ccc50 pet inventory
    INT SlotAt(INT X, INT Y);                                        // 0x100ccd50 -1 = none
    void ReloadNames();                                              // 0x100cd250 new language: the name and the tool tip of every slot again
    INT PageSlot(INT Index);                                         // 0x100ccf10
    INT DeleteShortCut(INT Slot);                                    // 0x100cee40
    void SetPage(INT NewPage);                                       // 0x100ceea0
    void Tick(FLOAT DeltaTime);                                      // 0x100cef10
    INT OnShortCutKeyUp(UINT Key, LPARAM lParam);                    // 0x100cefd0
    INT OnShortCutKeyDown(UINT Key, LPARAM lParam);                  // 0x100cf080 (the console's HookShortCutKey calls it)
    void NextPage();                                                 // 0x100cf380
    void PrevPage();                                                 // 0x100cf3f0
    void ClearShortCuts();                                           // 0x100cf4a0
    INT SetShortCut(NItemInfo* Info, INT Owner);                     // 0x100cfe90 (the console's AddShortCutItem calls it)
    // names ours (all three layouts, from the C2 code)
    INT SlotAtPoint(INT X, INT Y);                                   // 0x100ccef0 the slot under a screen point by the layout (bPadMode): SlotAt, PadSlotAt or -1
    INT PadSlotAt(INT X, INT Y);                                     // 0x100cce40 the game pad layout's four slots (PadHold * 4 + 0 .. 3), -1 none
    INT UpdateMacro(INT MacroID, INT Mode);                          // 0x100cc8d0 the console's UpdateMacroShortCut: Mode 2, the slots of the macro take its icon, name and tool tip; 0, they are deleted
    void PaintBar(NCGDevice* Device);                                // 0x100cd790 the vertical / horizontal bar (OnPaint's)
    void PaintPad(NCGDevice* Device);                                // 0x100ce160 the game pad layout (OnPaint's)
};
NW_ASSERT_OFFSET(NCShortCutWnd, Keys, 0x1A8);
NW_ASSERT_OFFSET(NCShortCutWnd, BackTex, 0x1B0);
NW_ASSERT_OFFSET(NCShortCutWnd, IconBackTex, 0x1BC);
NW_ASSERT_OFFSET(NCShortCutWnd, NextButton, 0x1C8);
NW_ASSERT_OFFSET(NCShortCutWnd, PadButton, 0x1D4);
NW_ASSERT_OFFSET(NCShortCutWnd, Cols, 0x1E4);
NW_ASSERT_OFFSET(NCShortCutWnd, HoverSlot, 0x1F8);
NW_ASSERT_OFFSET(NCShortCutWnd, PadHoldRTex, 0x204);
NW_ASSERT_SIZE(NCShortCutWnd, 0x208);

#pragma pack(pop)