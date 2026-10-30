#pragma once
// NCShopWnd - an NPC's shop (UClass "CShopWnd", 0xD0): two grids with the arrows between
// them, the total and the player's adena under them, OK and Cancel. Mode 0 buys: the NPC's
// goods on top (they stay there; a stack asks how many whatever its count), what is bought
// below; mode 1 sells: the inventory on top, what is sold below (at most what there is). The
// total is the price times the amount of each item below, in 64 bits (drawn with "%d": its
// low half). OK sends the lower grid's list to the server (RequestBuyItem /
// RequestSellItem). The console opens the window in one mode or the other (Clear, SetMode,
// SetListID, SetAdena) and fills the grids (AddShopItem / AddPlayerItem).
//
// NCShopItemWnd - its grids (UClass "CShopItemWnd", 0x124): the warehouse's (NCStoreWnd.h)
// with the prices (NItemEntry +0xC20) in the tool tips, and the rules of the mode and side
// when an item moves.
//
// Desktop (NCConsole) +0xD0 is this window.

#include "nwindow/NCItemWnd.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCButton;

// NWM_COMMAND ids (names ours): NCStoreWnd's (NID_STORE_*), the window procedure is one body

// NCShopWnd::Mode, NCShopItemWnd::Mode and Side (names ours)
enum
{
    SHOPMODE_Buy = 0,
    SHOPMODE_Sell = 1,
    SHOPSIDE_Shop = 0,               // the NPC's goods when buying, what is sold when selling
    SHOPSIDE_Player = 1,             // what is bought when buying, the inventory when selling
};

class NCShopItemWnd : public NCItemWnd
{
    NW_DECLARE_CLASS(NCShopItemWnd, NCItemWnd, 0, 0x101D8B88)

    NCShopItemWnd* Target;           // +0x110 where its items go (the other grid)
    INT Mode;                        // +0x114 the window's (SHOPMODE_*)
    INT Side;                        // +0x118 SHOPSIDE_*   // C2: offset derived, no evidence

    NCShopItemWnd();                                                 // 0x100cba90
    NCShopItemWnd(INT InColumns, INT InVisibleRows, INT InMaxItems, INT InCellSize); // 0x100cbb30
    // ~NCShopItemWnd                                                // 0x100cbae0

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x100c0e40 (shared with the other grids)
    virtual INT OnDestroy();                                         // 37 0x100c46f0 (shared)
    virtual INT OnLButtonUp(WPARAM wParam, LPARAM lParam);           // 62 0x100cbb70
    // names ours
    virtual INT OnLButtonDblClick(WPARAM wParam, LPARAM lParam);     // 73 0x100cbed0 WM_LBUTTONDBLCLK
    virtual INT OnDialogResult(WPARAM wParam, LPARAM lParam);        // 74 0x100cc1c0

    // names ours
    void SetTarget(NCShopItemWnd* InTarget);                         // 0x1006ee70 (shared)
    void SetMode(INT InMode);                                        // 0x100d8aa0 (shared)
    void SetSide(INT InSide);                                        // 0x100d8ab0 (shared)
    UBOOL AddItem(NItemInfo* Info);                                  // 0x100cac30 0 when full
    UBOOL AddCopy(NItemEntry* Src, INT Count);                       // 0x100cad80 0 when full
    void MoveItem(NItemEntry* Item, INT Count);                      // 0x100caea0 to Target
};
NW_ASSERT_OFFSET(NCShopItemWnd, Target, 0x110);
NW_ASSERT_OFFSET(NCShopItemWnd, Side, 0x118);
NW_ASSERT_SIZE(NCShopItemWnd, 0x11C);

class NCShopWnd : public NCWnd
{
    NW_DECLARE_CLASS(NCShopWnd, NCWnd, 0, 0x101D90B0)

    NCShopItemWnd* PlayerItems;      // +0x98 SHOPSIDE_Player: below when buying, on top when
                                     //   selling
    NCShopItemWnd* ShopItems;        // +0x9C SHOPSIDE_Shop: on top when buying, below when
                                     //   selling
    INT Unknown9C;                   // +0xA0
    UTexture* BackTex;               // +0xA4 "L2UI.StoreWnd.StoreBack"
    INT UnknownA4;                   // +0xA8
    NCButton* UpButton;              // +0xAC
    NCButton* DownButton;            // +0xB0
    NCButton* OKButton;              // +0xB4
    NCButton* CancelButton;          // +0xB8
    NCWnd* UnknownB8;                // +0xBC OnDestroy's, never created
    INT Mode;                        // +0xC0 SHOPMODE_*
    INT ListID;                      // +0xC4 (-1) the NPC's list, sent back with the request
    INT Adena;                       // +0xC8 the player's
    SQWORD TotalPrice;               // +0xCC UpdateTotalPrice

    NCShopWnd();                                                     // 0x100c9fe0
    // ~NCShopWnd                                                    // 0x100a5e50 (C1 only: no C2 match)

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x1006ed90 (shared)
    virtual INT OnCreate();                                          // 36 0x100cc210
    virtual INT OnDestroy();                                         // 37 0x100ca4c0
    virtual void ShowWindow();                                       // 43 0x1007b460 (folded)
    virtual void HideWindow();                                       // 44 0x100c9fb0
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x100ca050
    // names ours
    virtual INT OnOK();                                              // 67 0x100ca710
    virtual INT OnCancel();                                          // 68 0x100c9fc0
    virtual INT OnUp();                                              // 69 0x100cb420
    virtual INT OnDown();                                            // 70 0x100cb760
    virtual INT OnClose();                                           // 71 0x100c9fc0 (OnCancel's body)

    // names ours (the console calls most of these)
    void SetAdena(INT InAdena);                                      // 0x100c9f60
    void SetListID(INT InListID);                                    // 0x100d8af0 (shared)
    void Clear();                                                    // 0x100c9f70 grids emptied,
                                                                     //   list id -1, adena and
                                                                     //   total 0
    void UpdateLayout();                                             // 0x100ca610 the grids placed
                                                                     //   by mode
    __declspec(noinline) void UpdateTotalPrice();                                         // 0x100ca960
    void ReloadTexts();                                              // 0x100caa50 item names and
                                                                     //   descriptions, tool tips
    void SetMode(INT InMode);                                        // 0x100cb3c0 the grids' too,
                                                                     //   then the layout
    UBOOL AddShopItem(NItemInfo* Info);                              // 0x100cb400
    UBOOL AddPlayerItem(NItemInfo* Info);                            // 0x100cb410
};
NW_ASSERT_OFFSET(NCShopWnd, BackTex, 0xA4);
NW_ASSERT_OFFSET(NCShopWnd, Mode, 0xC0);
NW_ASSERT_OFFSET(NCShopWnd, TotalPrice, 0xCC);
NW_ASSERT_SIZE(NCShopWnd, 0xD4);

#pragma pack(pop)