#pragma once
// NCPrivateBuyWnd - the private store (buy) window (UClass "CPrivateBuyWnd", 0xD4): the sell
// window's twin (NCPrivateShopWnd.h, the same fields at the same places; several bodies are
// shared) for a store that buys. Setting one's own up (Mode 1): the inventory on top as a
// catalog, the wish list below with the prices offered and the amounts wanted; at someone's
// (Mode 0): the items the store wants on top, the ones sold to it below. The desktop keeps it
// at +0x134.
//
// NCPrivateBuyItemWnd - its two grids (UClass "CPrivateBuyItemWnd", 0x12C), Side and Mode as in
// NCPrivateShopItemWnd; NItemEntry::UnknownC34 is the amount the store still wants. At
// someone's store the inventory's items the player lacks (count 0) are covered.

#include "nwindow/NCItemWnd.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCButton;

// what the dialog box sends the buy store windows (NCDialogBox::OnOK), names ours
enum
{
    NWM_BUYSTORE_PRICE = (INT)0xF000000B,      // dialog 0x26: the price typed
    NWM_BUYSTORE_AMOUNT = (INT)0xF000000C,     // dialog 0x27: the amount typed
    NWM_BUYSTORE_PRICE_OK = (INT)0xF000000D,   // dialog 0x29: the unusual price confirmed
    NWM_BUYSTORE_PRICE_CANCEL = (INT)0xF000000E, // never sent
    NWM_BUYSTORE_SELL_OK = (INT)0xF000000F,    // dialog 0x2A: selling at unusual prices confirmed
};

class NCPrivateBuyItemWnd : public NCItemWnd
{
    NW_DECLARE_CLASS(NCPrivateBuyItemWnd, NCItemWnd, 0, 0x101D1F78)

    UTexture* CoverTex;              // +0x110 "NWindow.Chat", then "NWindow.ChatBack" (OnPaint)
                                     //   over the items the player lacks
    NCPrivateBuyItemWnd* Partner;    // +0x114 the other grid (SetPartner), or the one an item
                                     //   was carried onto
    INT Mode;                        // +0x118 the store window's (SetMode)
    INT Side;                        // +0x11C 0: the store's list, 1: the player's side (SetSide)
    INT Unknown128;                  // +0x120 cleared, never read here

    NCPrivateBuyItemWnd();                                           // 0x100bb2a0
    NCPrivateBuyItemWnd(INT InColumns, INT InVisibleRows, INT InMaxItems, INT InCellSize); // 0x100bb390
    // ~NCPrivateBuyItemWnd                                          // 0x100bb340

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x100b9050
    virtual INT OnDestroy();                                         // 37 0x100c46f0 (shared)
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x100b9390
    virtual INT OnLButtonUp(WPARAM wParam, LPARAM lParam);           // 62 0x100bb410
    // names ours
    virtual INT OnLButtonDblClick(WPARAM wParam, LPARAM lParam);     // 73 0x100bb7b0
    virtual INT OnPriceCancel(WPARAM wParam, LPARAM lParam) { return 1; } // 74 0x100d2e70 (folded)
    virtual INT OnPrice(WPARAM wParam, LPARAM lParam);               // 75 0x100b9750
    virtual INT OnAmount(WPARAM wParam, LPARAM lParam);              // 76 0x100bbaa0
    virtual INT OnPriceOK(WPARAM wParam, LPARAM lParam);             // 77 0x100b9650

    // names ours
    void SetPartner(NCPrivateBuyItemWnd* InPartner);                 // 0x100d8aa0 (shared with the
                                                                     //   NPC shops' grids)
    void SetMode(INT InMode);                                        // 0x100d8ab0 (shared)
    void SetSide(INT InSide);                                        // 0x100b9130
    UBOOL AddItem(NItemInfo* Info);                                  // 0x100ba6e0 0 when full
    __declspec(noinline) UBOOL AddEntry(NItemEntry* Src, INT Count);                      // 0x100ba800 a copy, 0 when full
    __declspec(noinline) void MoveItem(NItemEntry* Item, INT Count);                      // 0x100ba8f0 to Partner
};
NW_ASSERT_OFFSET(NCPrivateBuyItemWnd, Partner, 0x114);
NW_ASSERT_OFFSET(NCPrivateBuyItemWnd, Side, 0x11C);
NW_ASSERT_SIZE(NCPrivateBuyItemWnd, 0x124);

// 4-byte packing: TotalPrice sits at +0xCC
#pragma pack(push, 4)
class NCPrivateBuyWnd : public NCWnd
{
    NW_DECLARE_CLASS(NCPrivateBuyWnd, NCWnd, 0, 0x101D1A58)

    NCPrivateBuyItemWnd* PlayerItems;    // +0x98 Side 1: the inventory (Mode 1), the items sold
                                         //   (Mode 0)
    NCPrivateBuyItemWnd* StoreItems;     // +0x9C Side 0: the items wanted
    INT Unknown9C;                       // +0xA0
    UTexture* BackTex;                   // +0xA4 "L2UI.StoreWnd.StoreBack"
    INT UnknownA4;                       // +0xA8
    NCButton* UpButton;                  // +0xAC
    NCButton* DownButton;                // +0xB0
    NCButton* StartButton;               // +0xB4 "Start" (Mode 1) / "OK" (Mode 0)
    NCButton* MessageButton;             // +0xB8 Mode 1 only
    NCButton* AbortButton;               // +0xBC Mode 1 only
    NCWnd* UnknownBC;                    // +0xC0 a child the destructor would remove; not made
    INT Mode;                            // +0xC4 0: selling to someone's store, 1: one's own
    INT OwnerID;                         // +0xC8 (-1) object id of the store's owner (Mode 0)
    INT Adena;                           // +0xCC the player's (SetAdena)
    SQWORD TotalPrice;                   // +0xD0 the list's (UpdateTotal)

    NCPrivateBuyWnd();                                               // 0x100b9940
    // ~NCPrivateBuyWnd                                              // 0x100b99e0

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x100b9140
    virtual INT OnCreate();                                          // 36 0x100bbb20
    virtual INT OnDestroy();                                         // 37 0x100bc8a0
    virtual void ShowWindow();                                       // 43 0x100b92b0
    virtual void HideWindow();                                       // 44 0x100bc2c0
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x100b9a30
    // names ours
    virtual INT OnStart();                                           // 67 0x100b9fa0
    virtual INT OnUp();                                              // 68 0x100bade0
    virtual INT OnDown();                                            // 69 0x100bb080
    virtual INT OnClose();                                           // 70 0x100b9330
    virtual INT OnMessage();                                         // 71 0x100ba430
    virtual INT OnAbort();                                           // 72 0x100b9360
    virtual INT OnSellConfirmed(WPARAM wParam, LPARAM lParam);       // 73 0x100ba520

    // names ours
    void SetAdena(INT InAdena);                                      // 0x1009d910 (C1 only: no C2 match) (hooked as the
                                                                     //   sell window's)
    void SetOwnerID(INT InOwnerID);                                  // 0x100c9f60 (NCShopWnd::
                                                                     //   SetAdena's body)
    __declspec(noinline) void Reset();                                                    // 0x1009d920 (C1 only: no C2 match) (the sell window's)
    __declspec(noinline) void UpdateLayout();                                             // 0x1009df20 (C1 only: no C2 match) (the sell window's)
    void SetMode(INT InMode);                                        // 0x100bad60
    void AddStoreItem(NItemInfo* Info);                              // 0x100bada0
    UBOOL AddPlayerItem(NItemInfo* Info);                            // 0x100badd0
    __declspec(noinline) UBOOL CheckTotal(INT ItemID, INT Count, INT Price);              // 0x100bcd50 (name ours; shared by the sell and buy windows)
    __declspec(noinline) void UpdateTotal();                                              // 0x100ba340
};
#pragma pack(pop)
NW_ASSERT_OFFSET(NCPrivateBuyWnd, BackTex, 0xA4);
NW_ASSERT_OFFSET(NCPrivateBuyWnd, Mode, 0xC4);
NW_ASSERT_OFFSET(NCPrivateBuyWnd, TotalPrice, 0xD0);
NW_ASSERT_SIZE(NCPrivateBuyWnd, 0xD8);

#pragma pack(pop)