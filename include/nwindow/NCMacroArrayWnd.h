#pragma once
// NCMacroArrayWnd - the grid of the macros in NCMacroListWnd (C2 only; UClass "CMacroArrayWnd", 0x11C): an item window
// whose cells are the macros' icons (6 columns, 32 pixel cells 3 apart; the cells are painted from the
// parent's NMacroEntry array, not from NItemEntry records of its own) with the macro's short name over the icon (up to four
// characters, two by two). A click selects a macro (and picks the icon up, as in every item window); letting go on
// the same cell runs the macro, on the delete / edit icons of the list window deletes / edits it, on a
// shortcut slot puts it there.
//
// Layout from the constructors 0x1008FA80 / 0x1008FB40 and the destructor 0x1008FAE0 (a TArray after NCItemWnd's, never used).

#include "nwindow/NCItemWnd.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

// the dialog that asks whether to delete the macro (NDialogInfo::ID; name ours)
enum { DIALOG_MACRO_DELETE = 0x44 };

class NCMacroArrayWnd : public NCItemWnd
{
    NW_DECLARE_CLASS(NCMacroArrayWnd, NCItemWnd, 0, 0x101BFA10)

    TArray<INT> Unknown110;          // +0x110 an array of dwords (name ours): constructed empty and destroyed (the destructor calls the pointer
                                     //   arrays' 0x1008F7B0 on it), never used by the code seen

    NCMacroArrayWnd();                                               // 0x1008FA80
    NCMacroArrayWnd(INT InColumns, INT InVisibleRows, INT InMaxItems, INT InCellSize);  // 0x1008FB40
    // ~NCMacroArrayWnd                                              // 0x1008FAE0

    // WindowProc (25): 0x100BEC50 (NCQuestItemWnd's, shared: the dialog's answer goes to vslot 73)
    // OnCreate (36): 0x100C46E0 (shared)
    virtual INT OnDestroy();                                         // 37 0x1008B630
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x1008CD70
    virtual INT OnDrawToolTip(NCGDevice* Device);                    // 56 0x10090990 (named OnPostPaint in the database)
    virtual INT OnMouseMove(WPARAM wParam, LPARAM lParam);           // 60 0x1008D050
    virtual INT OnLButtonDown(WPARAM wParam, LPARAM lParam);         // 61 0x1008CFC0
    virtual INT OnLButtonUp(WPARAM wParam, LPARAM lParam);           // 62 0x1008D250
    virtual void UnselectAll();                                      // 71 0x1008D790
    // names ours
    virtual INT OnDialogResult(WPARAM wParam, LPARAM lParam);        // 73 0x1008D130 the answer to the delete question

    // names ours
    __declspec(noinline) INT HitCell(INT X, INT Y);                                       // 0x1008CAD0 the cell (0..41) under a point, -1 none
};
NW_ASSERT_OFFSET(NCMacroArrayWnd, Unknown110, 0x110);
NW_ASSERT_SIZE(NCMacroArrayWnd, 0x11C);

#pragma pack(pop)
