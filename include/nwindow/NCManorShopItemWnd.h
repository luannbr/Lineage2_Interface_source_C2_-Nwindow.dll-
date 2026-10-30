#pragma once
// NCManorShopItemWnd - the item grids of the manor's shop (UClass "CManorShopItemWnd", 0x11C; NCManorShopWnd.h): C2's
// copy of NCShopItemWnd (NCShopWnd.h) under NCItemWnd, with its own Target / Mode / Side. What differs from the shop's
// grid: an entry that moves always goes to the Target and leaves this grid (the NPC's goods do not stay), whatever the
// mode and the side; the tool tips of the entries (the price line: C2 passes 5 where the shop passes 7 for the NPC's
// goods when buying); the amount typed is clamped to the entry's in both modes (and the stock code checks the
// selection).

#include "nwindow/NCItemWnd.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCManorShopItemWnd : public NCItemWnd
{
    NW_DECLARE_CLASS(NCManorShopItemWnd, NCItemWnd, 0, 0x101C0EA8)

    NCManorShopItemWnd* Target;      // +0x110 where its items go (the other grid)
    INT Mode;                        // +0x114 the window's (SHOPMODE_*, NCShopWnd.h)
    INT Side;                        // +0x118 SHOPSIDE_* (SetSide 0x100d8ab0 writes it, the tool tip ladder reads it)

    NCManorShopItemWnd();                                            // 0x10093960
    NCManorShopItemWnd(INT InColumns, INT InVisibleRows, INT InMaxItems, INT InCellSize); // 0x10093a00 (name ours)
    // ~NCManorShopItemWnd                                           // 0x100939b0

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x100c0e40 (shared with the other grids)
    virtual INT OnDestroy();                                         // 37 0x100c46f0 (shared)
    virtual INT OnLButtonUp(WPARAM wParam, LPARAM lParam);           // 62 0x10093a40
    // names ours (the same slots as NCShopItemWnd's)
    virtual INT OnLButtonDblClick(WPARAM wParam, LPARAM lParam);     // 73 0x10093da0 WM_LBUTTONDBLCLK
    virtual INT OnDialogResult(WPARAM wParam, LPARAM lParam);        // 74 0x10094090

    // names ours; the three setters are the bodies the shop's grids share (the stock code of the manor's window calls those)
    void SetTarget(NCManorShopItemWnd* InTarget);                    // 0x100d8aa0 (shared)
    void SetMode(INT InMode);                                        // 0x1002f6a0 (shared)
    void SetSide(INT InSide);                                        // 0x100d8ab0 (shared)
    UBOOL AddItem(NItemInfo* Info);                                  // 0x10092ea0 0 when full
    UBOOL AddCopy(NItemEntry* Src, INT Count);                       // 0x10092ff0 0 when full
    void MoveItem(NItemEntry* Item, INT Count);                      // 0x10093110 to Target
};
NW_ASSERT_OFFSET(NCManorShopItemWnd, Target, 0x110);
NW_ASSERT_OFFSET(NCManorShopItemWnd, Mode, 0x114);
NW_ASSERT_OFFSET(NCManorShopItemWnd, Side, 0x118);
NW_ASSERT_SIZE(NCManorShopItemWnd, 0x11C);

#pragma pack(pop)
