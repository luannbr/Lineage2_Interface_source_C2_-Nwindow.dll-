#pragma once
// NCTradeWnd - the trade window (UClass "CTradeWnd", 0x100): the player's inventory on top,
// the player's offer and the partner's under it with their names, a down arrow that offers the
// selected inventory item, OK and Cancel (both answer the server: RequestTradeDone). The
// console fills it: Reset and the names when a trade starts, the three grids from the server's
// lists, UpdateInventoryItem for changes to the inventory; the texts again after a language
// change (ReloadTexts).
//
// NCTradeItemWnd - those grids (UClass "CTradeItemWnd", 0x11C). Its UClass names NCWnd as the
// super, but its constructors call NCItemWnd's and its vtable inherits NCItemWnd's bodies. In
// the inventory grid (Kind 0) an item is offered by carrying it onto the offer grid or by a
// double click (how many first, for a stack); the other two grids only show their items. An
// entry's count there is the amount offered (NItemEntry +0xC48); the tool tips are the
// inventory's (NItemToolTip.h).
//
// Desktop (NCConsole) +0xE0 is this window.

#include "nwindow/NCItemWnd.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCButton;

// NWM_COMMAND ids (names ours)
enum
{
    NID_TRADE_FRAME = 0xF3,
    NID_TRADE_OK = 0xF4,
    NID_TRADE_CANCEL = 0xF5,
    NID_TRADE_ADD = 0xF6,            // the down arrow
};

// NCTradeItemWnd::Kind (names ours)
enum
{
    TRADEGRID_Inventory = 0,
    TRADEGRID_MyOffer = 1,
    TRADEGRID_PartnerOffer = 2,
};

class NCTradeItemWnd : public NCItemWnd
{
    NW_DECLARE_CLASS(NCTradeItemWnd, NCItemWnd, 0, 0x101DE830)

    INT Kind;                        // +0x110 TRADEGRID_* (SetKind; the constructors leave it)

    NCTradeItemWnd();                                                // 0x100dba50
    NCTradeItemWnd(INT InColumns, INT InVisibleRows, INT InMaxItems, INT InCellSize); // 0x100dbaf0
    // ~NCTradeItemWnd                                               // 0x100dbaa0

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x100c0e40 (shared)
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x100da900
    virtual INT OnMouseMove(WPARAM wParam, LPARAM lParam);           // 60 0x100daa90
    virtual INT OnLButtonUp(WPARAM wParam, LPARAM lParam);           // 62 0x100dab40
    virtual void Clear();                                            // 70 0x100dbb20
    // names ours
    virtual INT OnLButtonDblClick(WPARAM wParam, LPARAM lParam);     // 73 0x100dade0 WM_LBUTTONDBLCLK
    virtual INT OnDialogResult(WPARAM wParam, LPARAM lParam);        // 74 0x100db0f0

    // names ours
    void SetKind(INT InKind);                                        // 0x100d8aa0 (one body with the
                                                                     //   shop windows' grids')
    UBOOL AddItem(NItemInfo* Info);                                  // 0x100db8c0 0 when full
    UBOOL UpdateItem(NItemInfo* Info);                               // 0x100dafe0 0: not here
    UBOOL RemoveItem(INT ObjectID);                                  // 0x10093f40 (C1 only: no C2 match) (shared)
};
NW_ASSERT_OFFSET(NCTradeItemWnd, Kind, 0x110);
NW_ASSERT_SIZE(NCTradeItemWnd, 0x114);

class NCTradeWnd : public NCWnd
{
    NW_DECLARE_CLASS(NCTradeWnd, NCWnd, 0, 0x101DED58)

    NCTradeItemWnd* InventoryItems;  // +0x98 6 x 4 icons, 80 at most
    NCTradeItemWnd* MyItems;         // +0x9C 6 icons
    NCTradeItemWnd* PartnerItems;    // +0xA0 6 icons
    UTexture* BackTex;               // +0xA4 "L2UI.storeWnd.TradeBack"
    NCButton* AddButton;             // +0xA8
    NCButton* OKButton;              // +0xAC
    NCButton* CancelButton;          // +0xB0
    FString MyName;                  // +0xB4 C2: an FString (C1: TCHAR[20])
    FString PartnerName;             // +0xC0 over the partner's (Reset clears it)

    NCTradeWnd();                                                    // 0x100db1d0
    // ~NCTradeWnd                                                   // 0x100db290

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x100da740
    virtual INT OnCreate();                                          // 36 0x100dbc00
    virtual INT OnDestroy();                                         // 37 0x100db420
    virtual void ShowWindow();                                       // 43 0x100da830
    virtual void HideWindow();                                       // 44 0x1007b470 (folded)
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x100db310
    // names ours
    virtual INT OnOK();                                              // 67 0x100da8c0
    virtual INT OnCancel();                                          // 68 0x100da8e0
    virtual INT OnAdd();                                             // 69 0x100db540 the down arrow

    // names ours (the console's trade handlers call these)
    void Reset();                                                    // 0x100da7e0
    void SetMyName(const TCHAR* Name);                               // 0x100da860
    void SetPartnerName(const TCHAR* Name);                          // 0x100da890
    UBOOL AddInventoryItem(NItemInfo* Info);                         // 0x100dba20
    UBOOL AddMyItem(NItemInfo* Info);                                // 0x100dba30
    UBOOL AddPartnerItem(NItemInfo* Info);                           // 0x100dba40
    UBOOL UpdateInventoryItem(INT Op, NItemInfo* Info);              // 0x100ba3a0 (C1 only: no C2 match) 1 add, 3 change,
                                                                     //   2 remove
    void ReloadTexts();                                              // 0x100db720 item names and
                                                                     //   descriptions, tool tips
};
NW_ASSERT_OFFSET(NCTradeWnd, BackTex, 0xA4);
NW_ASSERT_OFFSET(NCTradeWnd, MyName, 0xB4);
NW_ASSERT_OFFSET(NCTradeWnd, PartnerName, 0xC0);
NW_ASSERT_SIZE(NCTradeWnd, 0xCC);

#pragma pack(pop)