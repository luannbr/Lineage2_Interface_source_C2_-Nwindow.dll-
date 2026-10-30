#pragma once
// NCVIPShopItemListBox - the item list of the VIP shop window (UClass "CVIPShopItemListBox",
// 0x1F4). Same rows as NCTrainListBox (its code is shared where identical): the icon, the name
// and, below it, the description, plus "X <count>" for type 5 entries.

#include "nwindow/NCListBox.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

// one item row (names ours; 0x518 bytes in C2, 0x318 in C1). The VIP shop window passes a copy of
// its own record (NVIPShopRecord, NCVIPShopWnd.h); the console builds one per product the server lists.
// C2 layout from the constructor (0x10087590 inlines it), AddEntry 0x10086d90 (the copy) and OnPaint
// 0x100861a0: Name and Desc are 256 characters now (C1: 128), IconName stays 128.
struct NVIPShopEntry
{
    INT ProductID;             // +0x000 what the purchase request names
    INT ItemID;                // +0x004 (C2: the paint asks the additional name of it)
    TCHAR Name[256];           // +0x008 the item's
    TCHAR Desc[256];           // +0x208
    UTexture* Icon;            // +0x408 (NCVIPShopWnd::AddRecord finds it by IconName)
    TCHAR IconName[128];       // +0x40C never copied by the list box
    INT BodyPart;              // +0x50C never copied by the list box
    INT Type;                  // +0x510 5: Count is shown
    INT Count;                 // +0x514

    NVIPShopEntry()
    {
        Name[0] = 0;
        Desc[0] = 0;
        IconName[0] = 0;
        Icon = NULL;
        ProductID = 0;
        ItemID = 0;
        BodyPart = 0;
        Type = 0;
        Count = 0;
    }
};
NW_ASSERT_OFFSET(NVIPShopEntry, Desc, 0x208);
NW_ASSERT_OFFSET(NVIPShopEntry, Icon, 0x408);
NW_ASSERT_OFFSET(NVIPShopEntry, IconName, 0x40C);
NW_ASSERT_OFFSET(NVIPShopEntry, Type, 0x510);
NW_ASSERT_SIZE(NVIPShopEntry, 0x518);

class NCVIPShopItemListBox : public NCListBox
{
    NW_DECLARE_CLASS(NCVIPShopItemListBox, NCListBox, 0, 0x101BC160)

    TArray<NVIPShopEntry*> Entries;  // +0x1EC

    NCVIPShopItemListBox();                                          // 0x10087610
    NCVIPShopItemListBox(INT InMaxLines, INT InVisibleLines, INT Unused); // 0x100876d0
    // ~NCVIPShopItemListBox                                         // 0x10087670

    // stock bodies shared with NCTrainListBox (hooked there) except OnPaint
    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x100dc140 (NCDebugWnd's)
    virtual INT OnCreate();                                          // 36 0x100dc160
    virtual INT OnDestroy();                                         // 37 0x10086cc0
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x100861a0
    virtual INT OnLButtonDown(WPARAM wParam, LPARAM lParam);         // 61 0x10079050
    virtual INT HitTest(INT X, INT Y);                               // 68 0x10084690
    virtual void Clear();                                            // 70 0x100c3310

    // names ours
    void UpdateEntryScrollBar();                                     // 0x100c1110 (shared with NCTrainListBox)
    void AddEntry(const NVIPShopEntry* Src);                         // 0x10086d90
};
NW_ASSERT_OFFSET(NCVIPShopItemListBox, Entries, 0x1EC);
NW_ASSERT_SIZE(NCVIPShopItemListBox, 0x1F8);

#pragma pack(pop)