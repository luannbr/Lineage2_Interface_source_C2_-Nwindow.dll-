#pragma once
// NCPrivateShopWnd - the private store (sell) window (UClass "CPrivateShopWnd", 0xD4), in two
// modes: setting one's own store up (Mode 1: the inventory on top, the items on sale with
// their prices below; Start, Message, Abort) or buying at someone's (Mode 0: the store's items
// on top, the ones chosen below; OK). The arrows and drags move items between the two grids
// (how many first, for a stack; the price first, going on sale), and the window adds up the
// price of the list that is sent. The console fills it (SetMode, the items, SetAdena); the
// desktop (NCConsole) keeps it at +0x130.
//
// NCPrivateShopItemWnd - its two grids (UClass "CPrivateShopItemWnd", 0x128): Side 0 is the
// store's list, Side 1 the player's side. An entry's count is the amount there (NItemEntry::
// TradeCount), its price NItemEntry::UnknownC20 (Unknown1CA8: the item's reference price, a
// price 5 times away from it asks first). The questions go through the desktop's dialog box,
// which answers with the NWM_STORE_* messages.

#include "nwindow/NCItemWnd.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCButton;

// what the dialog box sends the store windows (NCDialogBox::OnOK / OnCancel), names ours
enum
{
    NWM_STORE_PRICE = (INT)0xF0000006,         // dialog 0x21: the price typed
    NWM_STORE_AMOUNT = (INT)0xF0000007,        // dialog 0x22: the amount typed
    NWM_STORE_PRICE_OK = (INT)0xF0000008,      // dialog 0x24: the unusual price confirmed
    NWM_STORE_PRICE_CANCEL = (INT)0xF0000009,  // dialog 0x24 cancelled
    NWM_STORE_BUY_OK = (INT)0xF000000A,        // dialog 0x25: buying at unusual prices confirmed
};

class NCPrivateShopItemWnd : public NCItemWnd
{
    NW_DECLARE_CLASS(NCPrivateShopItemWnd, NCItemWnd, 0, 0x101D29C8)

    NCPrivateShopItemWnd* Partner;   // +0x110 the other grid (SetPartner), or the one an item
                                     //   was carried onto
    INT Mode;                        // +0x114 the store window's (SetMode)   // C2: offset derived, no evidence
    INT Side;                        // +0x118 0: the store's list, 1: the player's side (SetSide)   // C2: offset derived, no evidence
    INT Unknown124;                  // +0x11C cleared, never read here

    NCPrivateShopItemWnd();                                          // 0x100bdbd0
    NCPrivateShopItemWnd(INT InColumns, INT InVisibleRows, INT InMaxItems, INT InCellSize); // 0x100bdc70
    // ~NCPrivateShopItemWnd                                         // 0x100bdc20

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x100bc0c0
    virtual INT OnDestroy();                                         // 37 0x100c46f0 (shared)
    virtual INT OnLButtonUp(WPARAM wParam, LPARAM lParam);           // 62 0x100bdcb0
    // names ours
    virtual INT OnLButtonDblClick(WPARAM wParam, LPARAM lParam);     // 73 0x100be020
    virtual INT OnPriceCancel(WPARAM wParam, LPARAM lParam) { return 1; } // 74 0x100d2e70 (folded)
    virtual INT OnPrice(WPARAM wParam, LPARAM lParam);               // 75 0x100be420
    virtual INT OnAmount(WPARAM wParam, LPARAM lParam);              // 76 0x100be630
    virtual INT OnPriceOK(WPARAM wParam, LPARAM lParam);             // 77 0x100be2f0

    // names ours
    void SetPartner(NCPrivateShopItemWnd* InPartner);                // 0x1006ee70 (hooked as
                                                                     //   NCTradeItemWnd::SetKind)
    void SetMode(INT InMode);                                        // 0x100d8aa0 (shared with the
                                                                     //   NPC shops' grids)
    void SetSide(INT InSide);                                        // 0x100d8ab0 (shared)
    UBOOL AddItem(NItemInfo* Info);                                  // 0x100bd070 0 when full
    __declspec(noinline) UBOOL AddEntry(NItemEntry* Src, INT Count);                      // 0x100bd190 a copy, 0 when full
    __declspec(noinline) void MoveItem(NItemEntry* Item, INT Count);                      // 0x100bd280 to Partner
};
NW_ASSERT_OFFSET(NCPrivateShopItemWnd, Partner, 0x110);
NW_ASSERT_OFFSET(NCPrivateShopItemWnd, Side, 0x118);
NW_ASSERT_SIZE(NCPrivateShopItemWnd, 0x120);

// 4-byte packing: TotalPrice sits at +0xCC
#pragma pack(push, 4)
class NCPrivateShopWnd : public NCWnd
{
    NW_DECLARE_CLASS(NCPrivateShopWnd, NCWnd, 0, 0x101D24A0)

    NCPrivateShopItemWnd* PlayerItems;   // +0x98 Side 1: the inventory (Mode 1), the items
                                         //   chosen (Mode 0)
    NCPrivateShopItemWnd* StoreItems;    // +0x9C Side 0: the items on sale
    INT Unknown9C;                       // +0xA0   // C2: offset derived, no evidence
    UTexture* BackTex;                   // +0xA4 "L2UI.StoreWnd.StoreBack"
    INT UnknownA4;                       // +0xA8   // C2: offset derived, no evidence
    NCButton* UpButton;                  // +0xAC
    NCButton* DownButton;                // +0xB0
    NCButton* StartButton;               // +0xB4 "Start" (Mode 1) / "OK" (Mode 0)
    NCButton* MessageButton;             // +0xB8 Mode 1 only
    NCButton* AbortButton;               // +0xBC Mode 1 only
    NCWnd* UnknownBC;                    // +0xC0 a child the destructor would remove; not made   // C2: offset derived, no evidence
    INT Mode;                            // +0xC4 0: buying at someone's store, 1: one's own (SetMode)
    INT OwnerID;                         // +0xC8 (-1) object id of the store's owner (Mode 0)
    INT Adena;                           // +0xCC the player's (SetAdena)   // C2: offset derived, no evidence
    SQWORD TotalPrice;                   // +0xD0 the list's (UpdateTotal)   // C2: offset derived, no evidence

    NCPrivateShopWnd();                                              // 0x100bc330
    // ~NCPrivateShopWnd                                             // 0x100bc3e0

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x100bc1a0
    virtual INT OnCreate();                                          // 36 0x100be6b0
    virtual INT OnDestroy();                                         // 37 0x100bc8a0 (shared)
    virtual void ShowWindow();                                       // 43 0x100b92b0 (shared)
    virtual void HideWindow();                                       // 44 0x100bc2c0 (shared)
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x100bc430
    // names ours
    virtual INT OnStart();                                           // 67 0x100bca20
    virtual INT OnUp();                                              // 68 0x100bd730
    virtual INT OnDown();                                            // 69 0x100bd9c0
    virtual INT OnClose();                                           // 70 0x100bc2d0
    virtual INT OnMessage();                                         // 71 0x100bce10
    virtual INT OnAbort();                                           // 72 0x100bc300
    virtual INT OnBuyConfirmed(WPARAM wParam, LPARAM lParam);        // 73 0x100bcf00

    // names ours
    void SetAdena(INT InAdena);                                      // 0x1009d910 (C1 only: no C2 match)
    void SetOwnerID(INT InOwnerID);                                  // 0x100c9f60 (NCShopWnd::
                                                                     //   SetAdena's body)
    __declspec(noinline) void Reset();                                                    // 0x1009d920 (C1 only: no C2 match) (also the buy
                                                                     //   window's)
    __declspec(noinline) void UpdateLayout();                                             // 0x1009df20 (C1 only: no C2 match)
    void SetMode(INT InMode);                                        // 0x100bd6b0
    void AddStoreItem(NItemInfo* Info);                              // 0x100bd6f0
    UBOOL AddPlayerItem(NItemInfo* Info);                            // 0x100bd720
    __declspec(noinline) UBOOL CheckTotal(INT ItemID, INT Count, INT Price);              // 0x100bcd50 (name ours; shared by the sell and buy windows)
    __declspec(noinline) void UpdateTotal();                                              // 0x100ba340 (shared)
};
#pragma pack(pop)
NW_ASSERT_OFFSET(NCPrivateShopWnd, BackTex, 0xA4);
NW_ASSERT_OFFSET(NCPrivateShopWnd, Mode, 0xC4);
NW_ASSERT_OFFSET(NCPrivateShopWnd, TotalPrice, 0xD0);
NW_ASSERT_SIZE(NCPrivateShopWnd, 0xD8);

#pragma pack(pop)