#pragma once
// NCEquipItemWnd - the inventory's paperdoll (UClass "CEquipItemWnd", 0x254; built like the
// item windows but on NCWnd): the equipped items in 14 slots of a 5 x 3 grid, 35 pixels apart
// (the console tells the slot of each item: NConsoleWnd::GetEquipSlot; 14 and 15 are the
// two-handed weapon and the full armor, drawn over two slots). A double click or the right
// button takes an item off; dragged onto the inventory's page it is taken off too, onto the
// shortcut bar it gets a shortcut, onto the pet inventory it is given to the pet, onto the
// trash / crystallize icons or the ground it asks first (NWM_DIALOG_RESULT answers).

#include "nwindow/NCItemWnd.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

// A tattoo of the paperdoll (name ours; 0x1A38 bytes, `new` in NCEquipItemWnd::AddHenna 0x1007F980): an item entry
// (its icon, name and tool tip come from the tattoo's FL2HennaData) and the box it is drawn in and hit-tested by
// (a RECT by value: PtInRect, vslot 41), relative to the window: x 0x6C, y 27 per slot, 24 x 24.
struct NHennaEntry : public NItemEntry
{
    INT BoxX;                  // +0x1A28
    INT BoxY;                  // +0x1A2C (the constructor clears it first)
    INT BoxW;                  // +0x1A30
    INT BoxH;                  // +0x1A34

    NHennaEntry()
    {
        BoxY = 0;
        BoxX = 0;
        BoxW = 0;
        BoxH = 0;
    }
};
NW_ASSERT_OFFSET(NHennaEntry, BoxX, 0x1A28);
NW_ASSERT_OFFSET(NHennaEntry, BoxH, 0x1A34);
NW_ASSERT_SIZE(NHennaEntry, 0x1A38);

class NCEquipItemWnd : public NCWnd
{
    NW_DECLARE_CLASS(NCEquipItemWnd, NCWnd, 0, 0x101B8DF0)

    // C2 layout (sizeof 0x250): from the constructor 0x1007F730, OnCreate 0x1007B400 and the other functions
    INT SelectedIndex;               // +0x98 (-1) index in Items of the item picked up
    UBOOL bNoDrag;                   // +0x9C the next click selects without picking up
    TArray<NItemEntry*> Items;       // +0xA0
    TArray<NHennaEntry*> Hennas;     // +0xAC new in C2 (name ours): the tattoos, drawn in a column of their own
                                     //   (OnPaint: boxes of 26 pixels at x = 108, 27 apart)
    BYTE _UnknownB8[0xF4 - 0xB8];    // +0xB8 (new in C2; the constructor leaves it)
    UTexture* SelectTex;             // +0xF4 "l2ui.nwindow.item_click"
    UTexture* ToolTipTex;            // +0xF8 "L2UI.tooltip.mini-tool01"
    UTexture* DualCapTex;            // +0xFC "L2UI.InventoryWnd.Icon_dualcap" over the second
                                     //   slot of a two-slot item
    UTexture* HennaBackTex;          // +0x100 "L2UI.InventoryWnd.Henna_Icon_Back" (new in C2)
    UBOOL bDragging;                 // +0x104 the selected item is being carried
    UBOOL bHasUnknown1C98;           // +0x108 an item with NItemEntry::Unknown1C98 is equipped
                                     //   (AddItem): a bow is then drawn in slot 5
    INT Unknown104;                  // +0x10C (5)
    INT Unknown108;                  // +0x110 (3)
    INT Unknown10C;                  // +0x114 nothing reads it (the constructor leaves it)
    INT Unknown110;                  // +0x118 nothing reads it
    INT NumSlots;                    // +0x11C (14) also the most items it keeps
    INT CellSize;                    // +0x120 (constructor's argument, 32)
    INT Selected;                    // +0x124 slot, -1 = none
    INT HoverSlot;                   // +0x128 slot under the mouse, -1 = none
    UBOOL bEnchantMode;              // +0x12C (SetEnchantMode; the constructor clears it)
    FVector DropLocation;            // +0x130 the ground point under the mouse when an item is let go over the game
                                     //   (OnLButtonUp: UGameEngine::MouseHitLocation; the drop question carries its own copy)
    BYTE _Unknown13C[0x24C - 0x13C]; // +0x13C nothing reads or writes it (C1's pending-question flags are gone)
    INT HoverHenna;                  // +0x24C (-1) the tattoo under the mouse (name ours)

    NCEquipItemWnd(INT InCellSize = 32);                                  // 0x1007f730
    // ~NCEquipItemWnd                                               // 0x10078240 (the two arrays, the base)

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x1007b3a0
    virtual INT OnCreate();                                          // 36 0x1007b400
    virtual INT OnDestroy();                                         // 37 0x1007f7e0
    virtual void ShowWindow();                                       // 43 0x1007b460 (folded)
    virtual void HideWindow();                                       // 44 0x1007b470 (folded)
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x1007b5c0
    virtual INT OnDrawToolTip(NCGDevice* Device);                    // 56 0x10080480
    virtual INT OnMouseMove(WPARAM wParam, LPARAM lParam);           // 60 0x1007d250
    virtual INT OnLButtonDown(WPARAM wParam, LPARAM lParam);         // 61 0x1007d0c0
    virtual INT OnLButtonUp(WPARAM wParam, LPARAM lParam);           // 62 0x1007d3e0
    virtual INT OnRButtonDown(WPARAM wParam, LPARAM lParam);         // 63 0x1007def0
    // names ours
    virtual INT GetEquipSlot(INT BodyPart, INT ObjectID);            // 67 0x1007b490 NConsoleWnd::GetEquipSlot
    virtual INT OnLButtonDblClick(WPARAM wParam, LPARAM lParam);     // 68 0x1007e380

    // names ours
    void SetEnchantMode(UBOOL bOn);                                  // 0x1007b480
    void UnselectAll();                                              // 0x1007baa0
    INT HitTest(INT X, INT Y);                                       // 0x1007bad0 slot; -1 outside (C1: 0)
    INT HitTestHenna(INT X, INT Y);                                  // 0x1007bbd0 the tattoo, -1 outside (name ours; stock)
    UBOOL UpdateItem(NItemInfo* Info);                               // 0x1007bc60 0: not here
    __declspec(noinline) UBOOL HasItem(INT ObjectID);                                     // 0x1007bd60
    __declspec(noinline) UBOOL FindItem(INT ObjectID, INT* Index);                        // 0x1007bdb0
    UBOOL AddItem(NItemInfo* Info);                                  // 0x1007e5e0 0 when full
    UBOOL RemoveItem(INT ObjectID);                                  // 0x1007f8b0 deletes the item with this object id
    void AddHenna(INT HennaID, INT bUsable);                         // 0x1007f980 (names ours; the console's slot 137 goes through
                                                                     //   NCInventoryWnd::AddHenna 0x10080250) a tattoo from the game data
    void ClearHennas();                                              // 0x10080740 (the console's slot 136, NCInventoryWnd 0x100807a0)
    void Clear();                                                    // 0x1007f920
};
NW_ASSERT_OFFSET(NCEquipItemWnd, Items, 0xA0);
NW_ASSERT_OFFSET(NCEquipItemWnd, Hennas, 0xAC);
NW_ASSERT_OFFSET(NCEquipItemWnd, SelectTex, 0xF4);
NW_ASSERT_OFFSET(NCEquipItemWnd, HennaBackTex, 0x100);
NW_ASSERT_OFFSET(NCEquipItemWnd, NumSlots, 0x11C);
NW_ASSERT_OFFSET(NCEquipItemWnd, bEnchantMode, 0x12C);
NW_ASSERT_OFFSET(NCEquipItemWnd, HoverHenna, 0x24C);
NW_ASSERT_SIZE(NCEquipItemWnd, 0x250);

#pragma pack(pop)