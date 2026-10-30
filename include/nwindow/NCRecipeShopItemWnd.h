#pragma once
// NCRecipeShopItemWnd - the two item grids of the dwarven recipe shop's manage window (NCRecipeShopWnd.h;
// UClass "CRecipeShopItemWnd", 0x118; new in C2). Each grid is an NCItemWnd of recipes whose items the player
// moves to the other grid (drag and drop, or a double click): the upper grid ("Recipe List", Mode 1) holds the
// recipe book, the lower one ("Production List", Mode 0) the recipes put up for sale, each with its price.
// Moving into the Mode 0 grid asks the price first (a number dialog, NDialogInfo ID 0x23, answered through
// OnDialogResult), moving into the Mode 1 grid just takes the entry out of the lower grid.
//
// The entries are NItemEntry records (NCItemWnd.h); an entry's UnknownC20 (+0x1924) is the price the shop
// sends, TradeCount (+0x194C) the recipe's second number as the NItemInfo gave it (Info +0x171C).
//
// WindowProc (0x100c0e40) and OnDestroy (0x100c46f0) are the stock bodies shared with NCShopItemWnd
// (NCShopWnd.cpp hooks them).

#include "nwindow/NCItemWnd.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCRecipeShopItemWnd : public NCItemWnd
{
    NW_DECLARE_CLASS(NCRecipeShopItemWnd, NCItemWnd, 0, 0x101D48A8)

    NCRecipeShopItemWnd* Target;     // +0x110 (name ours) the other grid (stock 0x100d8aa0, NCStoreItemWnd::SetTarget)
    INT Mode;                        // +0x114 (name ours) 0: the grid that takes a price, 1: the grid that gives the
                                     //   entry back; set by OnCreate (stock 0x1002f6a0, which the HTML viewer's
                                     //   SetOwner shares), never by the constructors

    NCRecipeShopItemWnd();                                           // 0x100c3850
    NCRecipeShopItemWnd(INT InColumns, INT InVisibleRows, INT InMaxItems, INT InCellSize); // 0x100c38f0
    // ~NCRecipeShopItemWnd                                          // 0x100c38a0

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x100c0e40 (shared with NCShopItemWnd)
    virtual INT OnDestroy();                                         // 37 0x100c46f0 (shared with NCShopItemWnd)
    virtual INT OnLButtonUp(WPARAM wParam, LPARAM lParam);           // 62 0x100c3930
    // names ours
    virtual INT OnLButtonDblClick(WPARAM wParam, LPARAM lParam);     // 73 0x100c3ae0 WM_LBUTTONDBLCLK
    virtual INT OnDialogResult(WPARAM wParam, LPARAM lParam);        // 74 0x100c3c20 the price typed

    // names ours (the console calls AddItem through NCRecipeShopWnd's two forwarders)
    __declspec(noinline) UBOOL AddItem(NItemInfo* Info);             // 0x100c3010 0 when full
    __declspec(noinline) UBOOL AddCopy(NItemEntry* Src);             // 0x100c3130 0 when full
    __declspec(noinline) void MoveItem(NItemEntry* Item);            // 0x100c3210 to Target
    void SetTarget(NCRecipeShopItemWnd* InTarget);                   // 0x100d8aa0 (shared: NCDeliverItemWnd::SetTarget hooks it)
    void SetMode(INT InMode);                                        // 0x1002f6a0 (shared: NCHtmlViewer::SetOwner)
};
NW_ASSERT_OFFSET(NCRecipeShopItemWnd, Target, 0x110);
NW_ASSERT_OFFSET(NCRecipeShopItemWnd, Mode, 0x114);
NW_ASSERT_SIZE(NCRecipeShopItemWnd, 0x118);

#pragma pack(pop)
