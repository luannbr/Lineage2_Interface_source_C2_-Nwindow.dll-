#pragma once
// NCPetInventoryWnd - the Items page of the pet window (UClass "CPetInventoryWnd", 0x9C): the
// pet's inventory in an NCPetInvenItemWnd grid, 6 x 2 icons of 32 pixels (160 at most). The
// console fills it through NCPetWnd (AddInventoryItem / UpdateInventoryItem / ClearInventory).
//
// NCPetInvenItemWnd - that grid (UClass "CPetInvenItemWnd", 0x134): equipped items get a frame
// behind their icon; a double click or the right button uses an item (the pet equips / eats
// it), dragging it inside the grid changes its place, onto the player's inventory or paperdoll
// gives it back (how many, for a stack), onto the shortcut bar registers it. The tool tips are
// the inventory's (NItemToolTip.h).

#include "nwindow/NCItemWnd.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCPetInvenItemWnd : public NCItemWnd
{
    NW_DECLARE_CLASS(NCPetInvenItemWnd, NCItemWnd, 0, 0x101CE1B8)

    UTexture* EquippedTex;           // +0x110 "l2ui.PetWnd.petitem_click" behind equipped items
    // the questions asked last (names ours; OnLButtonUp sets them)
    UBOOL bDropPending;              // +0x114 on the ground (nothing here sets it: the
                                     //   inventory's code, kept)
    UBOOL bToOwnerPending;           // +0x118 back to the player
    UBOOL bDblClicked;               // +0x11C the release after a double click moves nothing
    FVector DropLocation;            // +0x120 (never written here: the constructor leaves it)

    NCPetInvenItemWnd();                                             // 0x100b61b0
    NCPetInvenItemWnd(INT InColumns, INT InVisibleRows, INT InMaxItems, INT InCellSize); // 0x100b6260
    // ~NCPetInvenItemWnd                                            // 0x100b6210

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x100c4610
    virtual INT OnCreate();                                          // 36 0x100b2b60
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x100b3180
    virtual INT OnLButtonDown(WPARAM wParam, LPARAM lParam);         // 61 0x100b3330
    virtual INT OnLButtonUp(WPARAM wParam, LPARAM lParam);           // 62 0x100b5c10
    virtual INT OnRButtonDown(WPARAM wParam, LPARAM lParam);         // 63 0x100b3450
    virtual void Clear();                                            // 70 0x100b62a0
    // names ours
    virtual INT OnLButtonDblClick(WPARAM wParam, LPARAM lParam);     // 73 0x100b33c0 WM_LBUTTONDBLCLK

    // names ours
    __declspec(noinline) UBOOL AddItem(NItemInfo* Info);             // 0x100b6080 (C1 0x10093e50) 0 when full
    UBOOL UpdateItem(NItemInfo* Info);                               // 0x100b34b0 0: not here
    UBOOL RemoveItem(INT ObjectID);                                  // 0x10093f40 (C1 only: no C2 match) (NCTradeItemWnd's too)
    __declspec(noinline) UBOOL HasItem(INT ObjectID);                                     // 0x100b35b0
    __declspec(noinline) UBOOL FindItem(INT ObjectID, INT* Index);                        // 0x100b3600
};
NW_ASSERT_OFFSET(NCPetInvenItemWnd, EquippedTex, 0x110);
NW_ASSERT_OFFSET(NCPetInvenItemWnd, DropLocation, 0x120);
NW_ASSERT_SIZE(NCPetInvenItemWnd, 0x12C);

class NCPetInventoryWnd : public NCWnd
{
    NW_DECLARE_CLASS(NCPetInventoryWnd, NCWnd, 0, 0x101CEC00)

    UTexture* BackTex;               // +0x98 "L2UI.PetWnd.pet03" (the Action page's)   // C2: offset derived, no evidence
    NCPetInvenItemWnd* Items;        // +0x9C

    NCPetInventoryWnd();                                             // 0x100b3650
    // ~NCPetInventoryWnd                                            // 0x100b36a0

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x10043ef0 (folded)
    virtual INT OnCreate();                                          // 36 0x100b6300
    virtual INT OnDestroy();                                         // 37 0x100b36f0 (folded)
    virtual INT OnPaint(NCGDevice* Device);                          // C2: slot gone (was 50) (folded)

    // names ours (NCPetWnd passes the console's calls on)
    UBOOL AddItem(NItemInfo* Info);                                  // 0x100b6170 (C1 0x10094010)
    UBOOL UpdateItem(INT Op, NItemInfo* Info);                       // 0x100b63c0 (C1 0x10094030) 1 add, 2 change,
                                                                     //   3 remove
    NItemEntry* GetItem(INT ObjectID);                               // 0x100b37a0 NULL: none
    void ClearItems();                                               // 0x100b2b90 (C1 0x100bab50) (shared with
                                                                     //   NCTrainListWnd's Clear)
};
NW_ASSERT_OFFSET(NCPetInventoryWnd, Items, 0x9C);
NW_ASSERT_SIZE(NCPetInventoryWnd, 0xA0);

#pragma pack(pop)