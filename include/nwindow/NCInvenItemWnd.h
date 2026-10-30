#pragma once
// NCInvenItemWnd - a page of the inventory's items (UClass "CInvenItemWnd", 0x13C; the
// inventory has two: "Item" and "Quest", 6 x 4 icons of 32 pixels, 160 at most). An item is
// used with a double click or the right button; dragged inside the page it changes place,
// onto the paperdoll it is equipped, onto the shortcut bar it gets a shortcut, onto the pet
// window it is given to the pet, onto the inventory's trash / crystallize icons or the ground
// it asks first (the answer comes back as NWM_DIALOG_RESULT). With an enchant scroll in use
// (SetEnchantMode) the next item released is the one to enchant.

#include "nwindow/NCItemWnd.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCInvenItemWnd : public NCItemWnd
{
    NW_DECLARE_CLASS(NCInvenItemWnd, NCItemWnd, 0, 0x101B9310)

    // C2 (sizeof 0x124): the four "question pending" flags of C1 are gone; the questions go through
    // NDialogInfo / NConsoleWnd::ShowDialog and the answers come back through the dialog box itself
    UBOOL bEnchantMode;              // +0x110 (SetEnchantMode) the cursor stays the scroll's
    UBOOL bDblClicked;               // +0x114 the release after a double click moves nothing
    FVector DropLocation;            // +0x118 the ground point under the mouse (OnLButtonUp: UGameEngine::MouseHitLocation +0x200)

    NCInvenItemWnd();                                                // 0x1007fb00
    NCInvenItemWnd(INT InColumns, INT InVisibleRows, INT InMaxItems, INT InCellSize); // 0x1007fb20
    // ~NCInvenItemWnd                                               // 0x100782e0

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x100c4610 (folded with NCPetInvenItemWnd's)
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x1007c470
    virtual INT OnMouseMove(WPARAM wParam, LPARAM lParam);           // 60 0x1007c630
    virtual INT OnLButtonDown(WPARAM wParam, LPARAM lParam);         // 61 0x1007c070
    virtual INT OnLButtonUp(WPARAM wParam, LPARAM lParam);           // 62 0x1007e790
    virtual INT OnRButtonDown(WPARAM wParam, LPARAM lParam);         // 63 0x1007be00
    // names ours
    virtual INT OnLButtonDblClick(WPARAM wParam, LPARAM lParam);     // 73 0x1007c100 (new in C2) WM_LBUTTONDBLCLK

    // names ours
    void SetEnchantMode(UBOOL bOn);                                  // 0x100d8aa0 (folded with NCStoreItemWnd::SetMode: not hooked here)
    UBOOL UpdateItem(NItemInfo* Info);                               // 0x1007c340 0: not here
    INT GetAdena();                                                  // 0x1007c440
    UBOOL AddItem(NItemInfo* Info);                                  // 0x1007f370 0 when full
    UBOOL RemoveItem(INT ObjectID);                                  // 0x1007fb60
    // NCPetInvenItemWnd's bodies (0x100b35b0, 0x100b3600): unhooked copies
    __declspec(noinline) UBOOL HasItem(INT ObjectID);
    __declspec(noinline) UBOOL FindItem(INT ObjectID, INT* Index);
};
NW_ASSERT_OFFSET(NCInvenItemWnd, bEnchantMode, 0x110);
NW_ASSERT_OFFSET(NCInvenItemWnd, bDblClicked, 0x114);
NW_ASSERT_OFFSET(NCInvenItemWnd, DropLocation, 0x118);
NW_ASSERT_SIZE(NCInvenItemWnd, 0x124);

#pragma pack(pop)