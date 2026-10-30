#pragma once
// NCManorShopWnd - the manor's seed and crop shop (UClass "CManorShopWnd", 0x110; desktop +0xE4, NCConsole::NewWndE4):
// C2's copy of NCShopWnd (NCShopWnd.h): the same two grids with the arrows between them, the total and the player's
// adena, OK and Cancel, the same texts and requests. The class declares NCShopWnd as its super and then repeats the
// shop's fields (+0xD4 .. +0x10F, the shop's +0x98 .. +0xD3 moved by 0x3C): its constructor runs the shop's (which
// sets the inherited copies) and sets none of its own, and every body below works on the copies. The console's
// manor virtuals (NConsoleWnd 65..68: StartManorBuyList / StartManorSellList, AddManorBuyList / AddManorSellList)
// open it in one mode or the other (Clear, SetMode, SetListID, SetAdena) and fill the grids (AddShopItem /
// AddPlayerItem).
//
// What differs from the shop's code: the grids (NCManorShopItemWnd.h) move whole entries (the NPC's goods do not
// stay), the window procedure repeats the OK button's test after the shop's procedure (which has answered it: the
// repeat is never reached), and there is no ReloadTexts.

#include "nwindow/NCShopWnd.h"
#include "nwindow/NCManorShopItemWnd.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCManorShopWnd : public NCShopWnd
{
    NW_DECLARE_CLASS(NCManorShopWnd, NCShopWnd, 0, 0x101C13C8)

    // the fields repeat the shop's (and hide them: the inherited ones are the shop's, unused here)
    NCManorShopItemWnd* PlayerItems; // +0xD4 SHOPSIDE_Player: below when buying, on top when selling (the shop's +0x98)
    NCManorShopItemWnd* ShopItems;   // +0xD8 SHOPSIDE_Shop: on top when buying, below when selling (+0x9C)
    INT UnknownDC;                   // +0xDC (+0xA0)
    UTexture* BackTex;               // +0xE0 "L2UI.StoreWnd.StoreBack" (+0xA4)
    INT UnknownE4;                   // +0xE4 (+0xA8)
    NCButton* UpButton;              // +0xE8 (+0xAC)
    NCButton* DownButton;            // +0xEC (+0xB0)
    NCButton* OKButton;              // +0xF0 (+0xB4)
    NCButton* CancelButton;          // +0xF4 (+0xB8)
    NCWnd* UnknownF8;                // +0xF8 OnDestroy's, never created (+0xBC)
    INT Mode;                        // +0xFC SHOPMODE_* (+0xC0)
    INT ListID;                      // +0x100 (-1 after Clear) the NPC's list, sent back with the request (+0xC4)
    INT Adena;                       // +0x104 the player's (+0xC8)
    SQWORD TotalPrice;               // +0x108 UpdateTotalPrice (+0xCC)

    NCManorShopWnd();                                                // 0x100923f0
    // ~NCManorShopWnd                                               // 0x10092440

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x10092280
    virtual INT OnCreate();                                          // 36 0x100940f0
    virtual INT OnDestroy();                                         // 37 0x10092b50
    virtual void HideWindow();                                       // 44 0x10092340
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x10092490
    // names ours (the same slots as NCShopWnd's)
    virtual INT OnOK();                                              // 67 0x10092900
    virtual INT OnCancel();                                          // 68 0x10092350
    virtual INT OnUp();                                              // 69 0x100932f0
    virtual INT OnDown();                                            // 70 0x10093630
    virtual INT OnClose();                                           // 71 0x10092350 (OnCancel's body)

    // names ours (the console calls most of these)
    void SetListID(INT InListID);                                    // 0x100922e0
    void SetAdena(INT InAdena);                                      // 0x100922f0
    __declspec(noinline) void Clear();                               // 0x10092300 grids emptied, list id -1, adena and total 0
    __declspec(noinline) void UpdateLayout();                        // 0x10092ca0 the grids placed by mode
    __declspec(noinline) void UpdateTotalPrice();                    // 0x10092da0
    void SetMode(INT InMode);                                        // 0x10093290 the grids' too, then the layout
    UBOOL AddShopItem(NItemInfo* Info);                              // 0x100932d0
    UBOOL AddPlayerItem(NItemInfo* Info);                            // 0x100932e0
};
NW_ASSERT_OFFSET(NCManorShopWnd, PlayerItems, 0xD4);
NW_ASSERT_OFFSET(NCManorShopWnd, ShopItems, 0xD8);
NW_ASSERT_OFFSET(NCManorShopWnd, BackTex, 0xE0);
NW_ASSERT_OFFSET(NCManorShopWnd, UpButton, 0xE8);
NW_ASSERT_OFFSET(NCManorShopWnd, UnknownF8, 0xF8);
NW_ASSERT_OFFSET(NCManorShopWnd, Mode, 0xFC);
NW_ASSERT_OFFSET(NCManorShopWnd, ListID, 0x100);
NW_ASSERT_OFFSET(NCManorShopWnd, Adena, 0x104);
NW_ASSERT_OFFSET(NCManorShopWnd, TotalPrice, 0x108);
NW_ASSERT_SIZE(NCManorShopWnd, 0x110);

#pragma pack(pop)
