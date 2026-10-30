#pragma once
// NCQuestItemWnd - the quest window's "Item" page (UClass "CQuestItemWnd", 0x118): the quest
// items, 6 x 4 icons of 32 pixels (at most 80). The selected item's tool tip lists its name
// (enchant, count), slot and description. An item dropped on the quest window's drop zone
// asks whether to destroy it (how many, when stacked).

#include "nwindow/NCItemWnd.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCQuestItemWnd : public NCItemWnd
{
    NW_DECLARE_CLASS(NCQuestItemWnd, NCItemWnd, 0, 0x101D2EE8)

    NCQuestItemWnd();                                                // 0x100c03b0
    NCQuestItemWnd(INT InColumns, INT InVisibleRows, INT InMaxItems, INT InCellSize); // 0x100c0450
    // ~NCQuestItemWnd                                               // 0x100c0400

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x100bec50
    virtual INT OnCreate();                                          // 36 0x100c46e0
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x100bf450
    virtual INT OnLButtonUp(WPARAM wParam, LPARAM lParam);           // 62 0x100bf610
    virtual void DrawItemToolTip(NCGDevice* Device, NItemEntry* Item, INT X, INT Y); // 72 0x10019c40
    // names ours
    virtual INT OnDialogResult(WPARAM wParam, LPARAM lParam);        // 73 0x100bf870

    // names ours
    UBOOL AddItem(NItemInfo* Info);                                  // 0x100c00a0 0 when full
    void SetToolTip(NItemEntry* Item);                               // 0x1009fef0 (C1 only: no C2 match)
};
NW_ASSERT_SIZE(NCQuestItemWnd, 0x110);

#pragma pack(pop)