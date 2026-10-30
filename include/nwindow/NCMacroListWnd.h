#pragma once
// NCMacroListWnd - the macro window (C2 only; UClass "CMacroListWnd", 0x6084): the grid of the macros (NCMacroArrayWnd),
// the "(n/24)" counter, a button that opens the editor for a new macro and one that opens the macro help. It also keeps
// the macros themselves: NMacroEntry objects in a TArray (what the grid shows, the console's tick runs and the server's
// list fills, NConsoleWnd::vfn132 0x10066d70) and 24 slots of the data as the server sent it (NMacroSlot, what the
// editor shows and the save request is built from).
//
// The desktop (NCConsole) keeps the three macro windows at +0xF8 (this), +0xFC (NCMacroEditWnd) and +0x100
// (NCMacroInfoWnd) (names ours; NCConsole.h declares them as NCWnd*).
//
// Layout from the constructor 0x1008F990, OnCreate 0x10090490, OnDestroy 0x10090820, OnPaint 0x1008C810 and the users of the
// slots (NCMacroInfoWnd::vfn067, NCMacroEditWnd::vfn067).

#include "nwindow/NCWnd.h"
#include "nwindow/NMacroEntry.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCButton;
class NCMacroArrayWnd;

// NWM_COMMAND ids of the macro window (names ours)
enum
{
    NID_MACROLIST_NEW = 0x100F,        // the button at the bottom left (text 0x28E): a new macro
    NID_MACROLIST_HELP = 0x100C,       // the button next to it (text 0x91): the macro help page
};

class NCMacroListWnd : public NCWnd
{
    NW_DECLARE_CLASS(NCMacroListWnd, NCWnd, 0, 0x101BE578)

    UTexture* BackTex;               // +0x98 "l2ui.macrownd.macro_list"
    UTexture* EditTex;               // +0x9C "L2UI.macrownd.macro_edit_button": the icon that opens the editor (drawn at EditX, EditY)
    UTexture* DeleteTex;             // +0xA0 "L2UI.WindowIcon.DestroyItem": the icon that deletes the macro (drawn at DeleteX, DeleteY)
    INT DeleteX;                     // +0xA4 (0xBD) the delete icon's rectangle in the window: x, y, w, h
    INT DeleteY;                     // +0xA8 (0x13A)
    INT DeleteW;                     // +0xAC (0x20)
    INT DeleteH;                     // +0xB0 (0x20)
    INT EditX;                       // +0xB4 (0x98) the edit icon's rectangle
    INT EditY;                       // +0xB8 (0x13A)
    INT EditW;                       // +0xBC (0x20)
    INT EditH;                       // +0xC0 (0x20)
    TArray<NMacroEntry*> Macros;     // +0xC4 the macros the server sent (at most NMACRO_MaxMacros)
    NCMacroArrayWnd* Array;          // +0xD0 the grid
    NMacroSlot Slots[NMACRO_MaxMacros]; // +0xD4 the data as sent (id + NMacroInfo); the first dword of NConsoleWnd::vfn132's
                                     //   by-value argument is the slot's id
    INT Unknown6074;                 // +0x6074 (not touched by the code seen)
    INT Selected;                    // +0x6078 the slot being edited; Macros.Num() for a new macro
    NCButton* NewButton;             // +0x607C
    NCButton* HelpButton;            // +0x6080

    NCMacroListWnd();                                                // 0x1008F990
    // ~NCMacroListWnd                                               // 0x1008FA20

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x1008B510
    virtual INT OnCreate();                                          // 36 0x10090490
    virtual INT OnDestroy();                                         // 37 0x10090820
    // ShowWindow (43): 0x1008C9C0, the body of NCSystemMenuWnd::ShowWindow (shared, hooked there)
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x1008C810
    // OnLButtonUp (62): 0x100D2E70 (folded `return 1`)
    // names ours
    virtual INT OnNew();                                             // 67 0x1008CA10
    virtual INT OnHelp();                                            // 68 0x1008B900 (shared with NCMacroEditWnd's)

    // names ours: what the console's events call (jumps in NConsoleWnd's slots 217 / 218 / 219: the running macro's line waits
    // for the command's answer, and the event cuts the wait short)
    __declspec(noinline) void OnPickUpDone();                                             // 0x1008C330 slot 219: the line is /pickup
    __declspec(noinline) void OnTargetDone();                                             // 0x1008C3C0 slot 218: the line is /target, /targetnext, /assist
    void OnSkillCoolTime(INT SkillID, INT TimeMS);                   // 0x1008C450 slot 217: the line is the skill (its cool time)
    __declspec(noinline) void SetIcon(INT Index);                                         // 0x1008C7A0
    __declspec(noinline) UBOOL HitDelete(INT X, INT Y);                                   // 0x1008B590 (X, Y) in the delete icon's rectangle
    __declspec(noinline) UBOOL HitEdit(INT X, INT Y);                                     // 0x1008B5E0 in the edit icon's rectangle
    __declspec(noinline) void AddMacro(NMacroEntry* Macro);                               // 0x1008F0F0
};
NW_ASSERT_OFFSET(NCMacroListWnd, BackTex, 0x98);
NW_ASSERT_OFFSET(NCMacroListWnd, DeleteX, 0xA4);
NW_ASSERT_OFFSET(NCMacroListWnd, EditX, 0xB4);
NW_ASSERT_OFFSET(NCMacroListWnd, Macros, 0xC4);
NW_ASSERT_OFFSET(NCMacroListWnd, Array, 0xD0);
NW_ASSERT_OFFSET(NCMacroListWnd, Slots, 0xD4);
NW_ASSERT_OFFSET(NCMacroListWnd, Selected, 0x6078);
NW_ASSERT_OFFSET(NCMacroListWnd, HelpButton, 0x6080);
NW_ASSERT_SIZE(NCMacroListWnd, 0x6084);

#pragma pack(pop)
