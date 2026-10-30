#pragma once
// NCVIPShopWnd - the VIP shop (UClass "CVIPShopWnd", 0xC4; desktop +0xD4, title "Store"): two
// pages. The list page (NCVIPShopItemListWnd) shows the products; a click on one opens the item
// page: the item's information (NCVIPShopItemInfoWnd) over the items it costs
// (NCVIPShopNeededItemWnd), with OK (buy) and Cancel (back to the list). The console fills it
// through its virtuals 158..161: Open (a list id, everything emptied), AddRecord for each
// product, AddNeededItem for each item a product costs, then the window shown on the list
// (ShowList). OK sends the list id and the product id (console virtual 162, the network
// handler's slot 186). Hiding the window empties it.
//
// In the game's own terms this is the multisell window (an exchange list: a product costs the
// "items required", chosen by list id + product id); the name VIP shop is the decompilation's.
//
// The three pages are NCWnd / NCScrollWnd children laid over one another; the item page is the
// window's own background with "Item information" and "Items required" (sysstring 564, 565).

#include "nwindow/NCItemWnd.h"
#include "nwindow/NCVIPShopItemListBox.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCButton;

// NWM_COMMAND ids (names ours)
enum
{
    NID_VIPSHOP_FRAME = 0xF0E400,
    NID_VIPSHOP_LIST = 0xF0E401,
    NID_VIPSHOP_OK = 0xF0E402,
    NID_VIPSHOP_CANCEL = 0xF0E403,
};

// An item a product costs (name ours; 0x30C bytes in C2, 0x20C in C1: Name is 256 characters now; built on the
// stack by the console's virtual 200 (AddVIPNeedItemList) and copied by NCVIPShopWnd::AddNeededItem, then by
// the needed items' page). Layout from AddNeededItem 0x100873f0 (the inlined constructor and the copy),
// NCVIPShopWnd::ShowItem 0x10087e40 and NCVIPShopNeededItemWnd::OnPaint 0x100867b0.
struct NVIPNeededItem
{
    INT ItemID;                // +0x000
    TCHAR Name[256];           // +0x004 the item's
    INT Count;                 // +0x204 how many
    UTexture* Icon;            // +0x208 (AddNeededItem finds it by IconName)
    TCHAR IconName[128];       // +0x20C

    NVIPNeededItem()
    {
        Name[0] = 0;
        IconName[0] = 0;
        Count = 0;
        Icon = NULL;
        ItemID = 0;
    }
};
NW_ASSERT_OFFSET(NVIPNeededItem, Name, 0x4);
NW_ASSERT_OFFSET(NVIPNeededItem, Count, 0x204);
NW_ASSERT_OFFSET(NVIPNeededItem, Icon, 0x208);
NW_ASSERT_OFFSET(NVIPNeededItem, IconName, 0x20C);
NW_ASSERT_SIZE(NVIPNeededItem, 0x30C);

// A product of the shop (name ours; 0x524 bytes, NCVIPShopWnd::AddRecord): the list row and the
// items it costs (never freed: Reset only empties the window's array)
struct NVIPShopRecord : public NVIPShopEntry
{
    TArray<NVIPNeededItem*> NeededItems;   // +0x518

    NVIPShopRecord();                                                // 0x10087590
};
NW_ASSERT_OFFSET(NVIPShopRecord, NeededItems, 0x518);
NW_ASSERT_SIZE(NVIPShopRecord, 0x524);

// the list page (UClass "CVIPShopItemListWnd", 0x9C)
class NCVIPShopItemListWnd : public NCWnd
{
    NW_DECLARE_CLASS(NCVIPShopItemListWnd, NCWnd, 0, 0x101BBC40)

    NCVIPShopItemListBox* ListBox;   // +0x98
    UTexture* BackTex;               // +0x9C "L2UI.VipWnd.VipWndBack1" (OnCreate 0x10087720, OnPaint 0x10086690)

    NCVIPShopItemListWnd();                                          // 0x10086540
    // ~NCVIPShopItemListWnd                                         // 0x10086590

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x10084760
    virtual INT OnCreate();                                          // 36 0x10087720
    virtual INT OnDestroy();                                         // 37 0x100865e0
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x10086690
    virtual INT OnSelect(INT Index);                                 // 67 0x100881d0 (name ours; C1 slot 62)

    // names ours
    __declspec(noinline) void Clear();                               // 0x100c11b0 (ListBox->Clear(), shared; the stock
                                                                     //   callers call it, they do not inline it)
    void AddEntry(const NVIPShopEntry* Src);                         // 0x10086eb0
};
NW_ASSERT_OFFSET(NCVIPShopItemListWnd, BackTex, 0x9C);
NW_ASSERT_SIZE(NCVIPShopItemListWnd, 0xA0);

// the item page's information (UClass "CVIPShopItemInfoWnd", 0x1D7C): the product's item as an
// item window entry, completed from the item table
class NCVIPShopItemInfoWnd : public NCScrollWnd
{
    NW_DECLARE_CLASS(NCVIPShopItemInfoWnd, NCScrollWnd, 0, 0x101BD0E0)

    NItemEntry Item;                 // +0xD0 (0x1A28 bytes; the constructor 0x100878b0 calls NItemEntry's, the destructor
                                     //   0x10087930 inlines ~NItemEntry: ToolTip at +0x1A80)
    INT Unknown1AF8;                 // +0x1AF8 nothing reads or writes it (C2 only; the class is 0x1AFC)

    NCVIPShopItemInfoWnd();                                          // 0x100878b0
    // ~NCVIPShopItemInfoWnd                                         // 0x10087930

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x10019810 (shared)
    virtual INT OnCreate();                                          // 36 0x1002ce20 (shared)
    virtual INT OnDestroy();                                         // 37 0x100847c0 (shared)
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x10086920

    // names ours
    __declspec(noinline) void Clear();                               // 0x10086110 (called, not inlined, by the stock code)
    void SetItem(NItemEntry* Src);                                   // 0x10087080
    INT DrawItemInfo(NCGDevice* Device, NItemEntry* Entry, INT X, INT Y); // 0x10084840 the Y
                                                                     //   under the texts
};
NW_ASSERT_OFFSET(NCVIPShopItemInfoWnd, Item, 0xD0);
NW_ASSERT_OFFSET(NCVIPShopItemInfoWnd, Unknown1AF8, 0x1AF8);
NW_ASSERT_SIZE(NCVIPShopItemInfoWnd, 0x1AFC);

// the item page's needed items (UClass "CVIPShopNeededItemWnd", 0xE4): icon, name and "X count",
// 34 pixels a row
class NCVIPShopNeededItemWnd : public NCScrollWnd
{
    NW_DECLARE_CLASS(NCVIPShopNeededItemWnd, NCScrollWnd, 0, 0x101BCBB8)

    TArray<NVIPNeededItem*> Items;   // +0xD0
    INT UnknownD8;                   // +0xDC (the constructor 0x100877f0 leaves it)
    INT UnknownDC;                   // +0xE0 (Clear: 0)
    INT UnknownE0;                   // +0xE4 (Clear: 0)

    NCVIPShopNeededItemWnd();                                        // 0x100877f0
    // ~NCVIPShopNeededItemWnd                                       // 0x10087850

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x10019810 (shared)
    virtual INT OnCreate();                                          // 36 0x1002ce20 (shared)
    virtual INT OnDestroy();                                         // 37 0x10086ec0
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x100867b0

    // names ours
    void AddItem(const NVIPNeededItem* Src);                         // 0x10086f90
    __declspec(noinline) void Clear();                               // 0x10087020 (called by the stock code)
};
NW_ASSERT_OFFSET(NCVIPShopNeededItemWnd, Items, 0xD0);
NW_ASSERT_SIZE(NCVIPShopNeededItemWnd, 0xE8);

// NCVIPShopWnd::Page (names ours)
enum
{
    VIPSHOPPAGE_List = 0,
    VIPSHOPPAGE_Item = 1,
};

class NCVIPShopWnd : public NCWnd
{
    NW_DECLARE_CLASS(NCVIPShopWnd, NCWnd, 0, 0x101BC690)

    NCVIPShopItemListWnd* ListWnd;   // +0x98
    NCVIPShopItemInfoWnd* InfoWnd;   // +0x9C
    NCVIPShopNeededItemWnd* NeededWnd; // +0xA0
    NCButton* OKButton;              // +0xA4 the item page only
    NCButton* CancelButton;          // +0xA8 the item page only
    UTexture* BackTex;               // +0xAC "L2UI.VipWnd.VipWndBack2" the item page
    INT Selected;                    // +0xB0 the product shown (index in Records), -1 none
    INT ListID;                      // +0xB4 (Open) sent back with the purchase
    INT Page;                        // +0xB8 VIPSHOPPAGE_*
    TArray<NVIPShopRecord*> Records; // +0xBC

    NCVIPShopWnd();                                                  // 0x10087990
    // ~NCVIPShopWnd                                                 // 0x100879f0

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x10086120
    virtual INT OnCreate();                                          // 36 0x10087a50
    virtual INT OnDestroy();                                         // 37 0x100869c0
    virtual void HideWindow();                                       // 44 0x10088090
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x10086b10
    // names ours
    virtual INT OnOK();                                              // 67 0x10086c10
    virtual INT OnCancel();                                          // 68 0x100874d0

    // names ours (the console calls the last four)
    __declspec(noinline) void ShowList();                            // 0x100872d0 (the stock OnCancel and the console call it)
    __declspec(noinline) void ShowItem();                            // 0x10087e40 Records(Selected)
    __declspec(noinline) void Reset();                               // 0x100873a0 (Open and HideWindow call it)
    void AddNeededItem(INT ProductID, const NVIPNeededItem* Src);    // 0x100873f0
    void Open(INT InListID);                                         // 0x100874e0
    void AddRecord(const NVIPShopEntry* Src);                        // 0x100880a0
};
NW_ASSERT_OFFSET(NCVIPShopWnd, BackTex, 0xAC);
NW_ASSERT_OFFSET(NCVIPShopWnd, Records, 0xBC);
NW_ASSERT_SIZE(NCVIPShopWnd, 0xC8);

#pragma pack(pop)