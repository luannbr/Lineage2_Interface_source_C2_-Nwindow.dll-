#pragma once
// NCStoreWnd - the warehouse window (UClass "CStoreWnd", 0xC8): two grids with the arrows
// between them, the fee and the player's adena under them, OK and Cancel. Items go from one
// grid to the other with the arrows, a drag or a double click (how many first, for a stack).
// Mode 0 deposits: the inventory on top, what goes into the warehouse below (30 adena a slot);
// mode 1 withdraws: the warehouse's items on top, what comes back below (no fee). OK sends the
// lower grid's list to the server. The console opens the window in one mode or the other
// (Clear, SetMode, SetAdena) and fills the grids (AddInventoryItem / AddStoredItem).
//
// NCStoreItemWnd - its grids (UClass "CStoreItemWnd", 0x124): an item goes to Target, the
// other grid (a drop onto another warehouse grid makes that one the target); an entry's count
// is the amount it holds (NItemEntry::TradeCount). The tool tips are the inventory's
// (NItemToolTip.h).
//
// Desktop (NCConsole) +0xCC is this window.
//
// NCGMStoreWnd - the same window for a GM looking at a character's warehouse (UClass
// "CGMStoreWnd", 0xC8): the arrows, OK and Cancel are disabled.

#include "nwindow/NCItemWnd.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCButton;

// NWM_COMMAND ids (names ours; NCShopWnd shares this window procedure)
enum
{
    NID_STORE_FRAME = 0xF3,
    NID_STORE_OK = 0xF4,
    NID_STORE_CANCEL = 0xF5,
    NID_STORE_UP = 0xF6,
    NID_STORE_DOWN = 0xF7,
    NID_STORE_CLOSE = 0xF8,          // handled, but no control of the window sends it
};

// NCStoreWnd::Mode (names ours)
enum
{
    STOREMODE_Deposit = 0,
    STOREMODE_Withdraw = 1,
};

class NCStoreItemWnd : public NCItemWnd
{
    NW_DECLARE_CLASS(NCStoreItemWnd, NCItemWnd, 0, 0x101DDDE8)

    NCStoreItemWnd* Target;          // +0x110 where its items go (the other grid)
    INT Mode;                        // +0x114 the window's (STOREMODE_*); the shop's grid reads   // C2: offset derived, no evidence
                                     //   its copy, this one does not
    INT Side;                        // +0x118 0 the warehouse's grid, 1 the inventory's (not
                                     //   read here either)

    NCStoreItemWnd();                                                // 0x100d9d60
    NCStoreItemWnd(INT InColumns, INT InVisibleRows, INT InMaxItems, INT InCellSize); // 0x100d9e00
    // ~NCStoreItemWnd                                               // 0x100d9db0

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x100c0e40 (shared)
    virtual INT OnDestroy();                                         // 37 0x100c46f0 (shared)
    virtual INT OnLButtonUp(WPARAM wParam, LPARAM lParam);           // 62 0x100d9e40
    // names ours
    virtual INT OnLButtonDblClick(WPARAM wParam, LPARAM lParam);     // 73 0x100da070 WM_LBUTTONDBLCLK
    virtual INT OnDialogResult(WPARAM wParam, LPARAM lParam);        // 74 0x100da210

    // names ours; the three setters are one body each with other grids' (shops, private
    // stores), the last two hooked here
    void SetTarget(NCStoreItemWnd* InTarget);                        // 0x100d8aa0 (shared)
    void SetMode(INT InMode);                                        // 0x1002f6a0 (shared with NCHtmlViewer::SetOwner)
    void SetSide(INT InSide);                                        // 0x100d8ab0 (shared)
    UBOOL AddItem(NItemInfo* Info);                                  // 0x100d93c0 0 when full
    UBOOL AddCopy(NItemEntry* Src, INT Count);                       // 0x100d94d0 0 when full
    void MoveItem(NItemEntry* Item, INT Count);                      // 0x100d95b0 to Target
};
NW_ASSERT_OFFSET(NCStoreItemWnd, Target, 0x110);
NW_ASSERT_OFFSET(NCStoreItemWnd, Side, 0x118);
NW_ASSERT_SIZE(NCStoreItemWnd, 0x11C);

class NCStoreWnd : public NCWnd
{
    NW_DECLARE_CLASS(NCStoreWnd, NCWnd, 0, 0x101DE310)

    NCStoreItemWnd* InventoryItems;  // +0x98 the inventory's side: on top when depositing,
                                     //   below (what comes back) when withdrawing
    NCStoreItemWnd* StoredItems;     // +0x9C the warehouse's side: below (what goes in) when
                                     //   depositing, on top when withdrawing
    INT Unknown9C;                   // +0xA0
    UTexture* BackTex;               // +0xA4 "L2UI.StoreWnd.StoreBack"
    INT UnknownA4;                   // +0xA8
    NCButton* UpButton;              // +0xAC
    NCButton* DownButton;            // +0xB0
    NCButton* OKButton;              // +0xB4
    NCButton* CancelButton;          // +0xB8
    NCWnd* UnknownB8;                // +0xBC OnDestroy's, never created
    INT Mode;                        // +0xC0 STOREMODE_*
    INT Adena;                       // +0xC4 the player's
    INT Fee;                         // +0xC8 UpdateFee

    NCStoreWnd();                                                    // 0x100d8ba0
    // ~NCStoreWnd                                                   // 0x10073bd0

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x1006ed90 (shared)
    virtual INT OnCreate();                                          // 36 0x100da250
    virtual INT OnDestroy();                                         // 37 0x100ca4c0 (shared)
    virtual void ShowWindow();                                       // 43 0x100d9080
    virtual void HideWindow();                                       // 44 0x100d8b30
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x100d8c00
    // names ours
    virtual INT OnOK();                                              // 67 0x100d9090
    virtual INT OnCancel();                                          // 68 0x100d8b80
    virtual INT OnUp();                                              // 69 0x100d9700
    virtual INT OnDown();                                            // 70 0x100d9a30
    virtual INT OnClose();                                           // 71 0x100d8b80 (OnCancel's body)

    // names ours (the console calls most of these)
    void SetMode(INT InMode);                                        // 0x100d8ac0 the grids' too
    void SetAdena(INT InAdena);                                      // 0x100d8af0 (shared)
    void Clear();                                                    // 0x100d8b00 grids emptied,
                                                                     //   adena and fee 0
    __declspec(noinline) void UpdateFee();                                                // 0x100d8b40
    INT GetMode();                                                   // 0x100d8b70
    void UpdateLayout();                                             // 0x100d8f80 the grids placed
                                                                     //   by mode
    void ReloadTexts();                                              // 0x100d92a0 item names and
                                                                     //   descriptions, tool tips
    UBOOL AddInventoryItem(NItemInfo* Info);                         // 0x100d96e0
    UBOOL AddStoredItem(NItemInfo* Info);                            // 0x100d96f0
};
NW_ASSERT_OFFSET(NCStoreWnd, BackTex, 0xA4);
NW_ASSERT_OFFSET(NCStoreWnd, Mode, 0xC0);
NW_ASSERT_SIZE(NCStoreWnd, 0xCC);

class NCGMStoreWnd : public NCStoreWnd
{
    NW_DECLARE_CLASS(NCGMStoreWnd, NCStoreWnd, 0, 0x101B45E8)

    NCGMStoreWnd();                                                  // 0x10076650
    // ~NCGMStoreWnd                                                 // 0x100766a0

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x10073110
    virtual INT OnCreate();                                          // 36 0x10073130
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x100766f0
};
NW_ASSERT_SIZE(NCGMStoreWnd, 0xCC);

#pragma pack(pop)