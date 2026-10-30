#pragma once
// NCInventoryWnd - the inventory (UClass "CInventoryWnd", 0xF8; the desktop, NCConsole, creates
// it in OnCreate and keeps it at +0xBC): the paperdoll (NCEquipItemWnd) on top, two tab pages
// of items below ("Item" and "Quest", both NCInvenItemWnd: the quest items, NItemInfo::Param
// 3, go to the second), the adena and the weight bar at the bottom, between the crystallize
// icon (shown with the skill) and the trash icon; the item windows light those up while an
// item is carried over them. The console fills it through AddItem / UpdateItem / ClearItems.

#include "nwindow/NCConsole.h"
#include "nwindow/NConsoleWnd.h"
#include "nwindow/NGameData.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCButton;
class NCEquipItemWnd;
class NCInvenItemWnd;
class NCTabControl;
struct NItemEntry;
struct NItemInfo;

class NCInventoryWnd : public NCWnd
{
    NW_DECLARE_CLASS(NCInventoryWnd, NCWnd, 0, 0x101B9840)

    UTexture* BackTex;               // +0x98 "L2UI.InventoryWnd.InventoryBack"   // C2: offset derived, no evidence
    UTexture* CrystallizeTex;        // +0x9C "L2UI.WindowIcon.Crystalize"   // C2: offset derived, no evidence
    UTexture* CrystallizeActiveTex;  // +0xA0 "L2UI.WindowIcon.Crystalize_active"   // C2: offset derived, no evidence
    UTexture* WeightTex[4];          // +0xA4 "L2UI.InventoryWnd.InventoryWT1".."4": the weight   // C2: offset derived, no evidence
                                     //   bar up to 50%, 66.66%, 80%, above
    UTexture* TrashTex;              // +0xB4 "L2UI.WindowIcon.DestroyItem"   // C2: offset derived, no evidence
    UTexture* TrashActiveTex;        // +0xB8 "L2UI.WindowIcon.DestroyItem_active"   // C2: offset derived, no evidence
    UTexture* AdenaTex;              // +0xBC "L2UI.WindowIcon.WindowIconAdena"   // C2: offset derived, no evidence
    UTexture* WeightIconTex;         // +0xC0 "L2UI.WindowIcon.Weight"   // C2: offset derived, no evidence
    NCInvenItemWnd* ItemWnd[2];      // +0xC4 the pages: 0 "Item", 1 "Quest"
    NCEquipItemWnd* EquipWnd;        // +0xCC
    NCTabControl* TabCtrl;           // +0xD0
    UBOOL bOverCrystallize;          // +0xD4 an item is carried over the crystallize icon
    UBOOL bOverTrash;                // +0xD8 ... over the trash icon
    INT TrashX;                      // +0xDC (190, 315, 32 x 32)
    INT TrashY;                      // +0xE0
    INT TrashW;                      // +0xE4
    INT TrashH;                      // +0xE8
    INT CrystallizeX;                // +0xEC (16, 315, 32 x 32)
    INT CrystallizeY;                // +0xF0
    INT CrystallizeW;                // +0xF4
    INT CrystallizeH;                // +0xF8
    NCButton* ManufactureButton;     // +0xFC new in C2 (name ours) "L2UI.WindowIcon.Manufacture", id 0xFE001C, at (44, 315):
                                     //   shown with the crystallize icon (ShowWindow)
    TArray<INT> ItemOrder;           // +0x100 new in C2 (name ours): the ids of the "Item" page's items as they stood when the
                                     //   window was last hidden (SaveItemOrder); RestoreItemOrder puts the page back in that order

    NCInventoryWnd();                                                // 0x1007f470
    // ~NCInventoryWnd                                               // 0x1007f560 (ItemOrder, the base)

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x1007b4a0
    virtual INT OnCreate();                                          // 36 0x1007fbf0
    virtual INT OnDestroy();                                         // 37 0x1007cbd0
    virtual void ShowWindow();                                       // 43 0x1007cda0
    virtual void HideWindow();                                       // 44 0x1007f6f0
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x1007c7a0
    virtual INT OnMouseMove(WPARAM wParam, LPARAM lParam);           // 60 0x1007cd20

    // names ours
    UBOOL IsInTrash(INT X, INT Y);                                   // 0x1007b500 edges included
    UBOOL IsInCrystallize(INT X, INT Y);                             // 0x1007b550
    void AddHenna(INT HennaID, INT bUsable);                         // 0x10080250 the paperdoll's (names ours)
    void ClearHennas();                                              // 0x100807a0
    void SetEnchantMode(UBOOL bOn);                                  // 0x1007ce40 enchant scroll
    void ReloadTexts();                                              // 0x1007cec0 names and tool tips
    NItemEntry* FindItem(INT ObjectID);                              // 0x1007d050 paperdoll, "Item" page
    void ClearItems();                                               // 0x100681a0 (C1 only: no C2 match)
    UBOOL AddItem(NItemInfo* Info);                                  // 0x1007f5c0
    UBOOL UpdateItem(NItemInfo* Info, INT Change);                   // 0x100800e0 1 added, 2 changed,
                                                                     //   3 removed
    virtual INT OnManufacture();                                     // 67 0x1007b5a0 (name ours) the manufacture button: the recipe book opens
    void SaveItemOrder();                                            // 0x1007f610 (name ours)
    void MoveLastToFront();                                          // 0x1007f690 (name ours) the page's last item first
    void RestoreItemOrder();                                         // 0x10080260 (name ours)
    void ApplyItemOrder();                                           // 0x10080440 (name ours) when the page has items
};
NW_ASSERT_OFFSET(NCInventoryWnd, WeightTex, 0xA4);
NW_ASSERT_OFFSET(NCInventoryWnd, ItemWnd, 0xC4);
NW_ASSERT_OFFSET(NCInventoryWnd, bOverCrystallize, 0xD4);
NW_ASSERT_OFFSET(NCInventoryWnd, TrashX, 0xDC);
NW_ASSERT_OFFSET(NCInventoryWnd, CrystallizeX, 0xEC);
NW_ASSERT_OFFSET(NCInventoryWnd, ManufactureButton, 0xFC);
NW_ASSERT_SIZE(NCInventoryWnd, 0x10C);

// The item table's search by the item id (NDataMap::FindRef, 0x1004dda0): the entry, NULL when
// there is none.
namespace NItemTable
{
    inline NViewItemData* Find(INT* ItemID)
    {
        return GL2GameData.ItemTable->FindRef(*ItemID);
    }
}

// a system line in the desktop's chat window
inline void DesktopSystemMessage(const TCHAR* Text)
{
    GDesktop->AddSystemText(Text, 0, 0xFFFFFFFF);
}

#pragma pack(pop)