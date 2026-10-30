#pragma once
// NCSelectDeliverWnd - "Send package", first step (UClass "CSelectDeliverWnd", 0x8CC): a
// combo box with the characters a package can go to (8 at most, the console adds them), OK
// asks the server for what can be sent to the one chosen, Cancel closes.
//
// NCDeliverWnd - the package window (UClass "CDeliverWnd", 0xC0): the warehouse's layout
// (NCStoreWnd.h) with one mode: the inventory on top, what is sent below, 1000 adena a slot.
// OK sends the list to the character chosen (TargetID). The console opens it with the
// target and the player's adena and fills the upper grid.
//
// NCDeliverItemWnd - its grids (UClass "CDeliverItemWnd", 0x11C): the warehouse's grid
// without the mode and side fields; the tool tips are the inventory's with 128-character
// buffers (NItemToolTip.h; the box's text buffer has 1024, see DrawItemToolTip).
//
// Desktop (NCConsole) +0x138 is NCDeliverWnd, +0x13C NCSelectDeliverWnd.

#include "nwindow/NCItemWnd.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCButton;
class NCComboBox;

// NWM_COMMAND ids of NCSelectDeliverWnd (names ours; NCDeliverWnd uses NCStoreWnd's)
enum
{
    NID_SELDELIVER_FRAME = 0xE432,
    NID_SELDELIVER_TARGETS = 0xE433,  // the combo box: lParam the entry chosen
    NID_SELDELIVER_OK = 0xE434,
    NID_SELDELIVER_CANCEL = 0xE435,
};

class NCDeliverItemWnd : public NCItemWnd
{
    NW_DECLARE_CLASS(NCDeliverItemWnd, NCItemWnd, 0, 0x101B2700)

    NCDeliverItemWnd* Target;        // +0x110 where its items go (the other grid)

    NCDeliverItemWnd();                                              // 0x100702e0
    NCDeliverItemWnd(INT InColumns, INT InVisibleRows, INT InMaxItems, INT InCellSize); // 0x10070380
    // ~NCDeliverItemWnd                                             // 0x10070330

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x100c0e40 (shared)
    virtual INT OnDestroy();                                         // 37 0x100c46f0 (shared)
    virtual INT OnLButtonUp(WPARAM wParam, LPARAM lParam);           // 62 0x100703c0
    // names ours
    virtual INT OnLButtonDblClick(WPARAM wParam, LPARAM lParam);     // 73 0x100705f0 WM_LBUTTONDBLCLK
    virtual INT OnDialogResult(WPARAM wParam, LPARAM lParam);        // 74 0x10070790

    // names ours
    void SetTarget(NCDeliverItemWnd* InTarget);                      // 0x100d8aa0 (shared: NCStoreItemWnd::SetTarget hooks it)
    UBOOL AddItem(NItemInfo* Info);                                  // 0x1006fc70 0 when full
    UBOOL AddCopy(NItemEntry* Src, INT Count);                       // 0x1006fd80 0 when full
    void MoveItem(NItemEntry* Item, INT Count);                      // 0x1006fe60 to Target
};
NW_ASSERT_OFFSET(NCDeliverItemWnd, Target, 0x110);
NW_ASSERT_SIZE(NCDeliverItemWnd, 0x114);

class NCDeliverWnd : public NCWnd
{
    NW_DECLARE_CLASS(NCDeliverWnd, NCWnd, 0, 0x101B3150)

    NCDeliverItemWnd* InventoryItems; // +0x98 on top
    NCDeliverItemWnd* SendItems;     // +0x9C below: what is sent
    UTexture* BackTex;               // +0xA0 "L2UI.StoreWnd.StoreBack"
    NCButton* UpButton;              // +0xA4
    NCButton* DownButton;            // +0xA8
    NCButton* OKButton;              // +0xAC
    NCButton* CancelButton;          // +0xB0
    NCWnd* UnknownB0;                // +0xB4 OnDestroy's, never created
    INT Adena;                       // +0xB8 the player's
    INT Fee;                         // +0xBC UpdateFee
    INT TargetID;                    // +0xC0 (-1) the character the package goes to

    NCDeliverWnd();                                                  // 0x1006f160
    // ~NCDeliverWnd                                                 // 0x1006f1f0

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x1006ed90 (shared)
    virtual INT OnCreate();                                          // 36 0x100707d0
    virtual INT OnDestroy();                                         // 37 0x1006f550
    virtual void ShowWindow();                                       // 43 0x1006eed0
    virtual void HideWindow();                                       // 44 0x1006eee0
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x1006f240
    // names ours
    virtual INT OnOK();                                              // 67 0x1006f6a0
    virtual INT OnCancel();                                          // 68 0x1006eef0
    virtual INT OnUp();                                              // 69 0x1006ffa0
    virtual INT OnDown();                                            // 70 0x10070140
    virtual INT OnClose();                                           // 71 0x1006eef0 (OnCancel's body)

    // names ours (the console calls most of these)
    void SetAdena(INT InAdena);                                      // 0x1006ee70
    void SetTargetID(INT InTargetID);                                // 0x1006ee80
    void Clear();                                                    // 0x1006ee90 grids emptied,
                                                                     //   target -1, adena and fee 0
    __declspec(noinline) void UpdateFee();                                                // 0x1006ef10
    UBOOL AddInventoryItem(NItemInfo* Info);                         // 0x1006ff90
};
NW_ASSERT_OFFSET(NCDeliverWnd, BackTex, 0xA0);
NW_ASSERT_OFFSET(NCDeliverWnd, TargetID, 0xC0);
NW_ASSERT_SIZE(NCDeliverWnd, 0xC4);

// one character a package can go to (C2: the name holds 24 characters, C1: 128)
struct NDeliverTarget
{
    INT ID;                          // +0x00 sent back with the request
    TCHAR Name[24];                  // +0x04 the combo box's line (AddTarget copies without a length check)
};
NW_ASSERT_SIZE(NDeliverTarget, 0x34);

class NCSelectDeliverWnd : public NCWnd
{
    NW_DECLARE_CLASS(NCSelectDeliverWnd, NCWnd, 0, 0x101B2C28)

    UTexture* BackTex;               // +0x98 "L2UI.etcwnd.NPCHtml_BACK"
    NCComboBox* TargetCombo;         // +0x9C
    NCButton* OKButton;              // +0xA0
    NCButton* CancelButton;          // +0xA4
    INT Selected;                    // +0xA8 (-1) entry chosen in the combo box
    NDeliverTarget Targets[8];       // +0xAC
    INT NumTargets;                  // +0x24C

    NCSelectDeliverWnd();                                            // 0x1006f7e0
    // ~NCSelectDeliverWnd                                           // 0x1006f840

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x1006ef40
    virtual INT OnCreate();                                          // 36 0x1006f890
    virtual INT OnDestroy();                                         // 37 0x1006fc10
    virtual void ShowWindow();                                       // 43 0x1006f080
    virtual void HideWindow();                                       // 44 0x1006f090
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x1006fb50
    // names ours
    virtual INT OnSelect(LPARAM Index);                              // 67 0x1006f140
    virtual INT OnOK();                                              // 68 0x1006f0a0
    virtual INT OnCancel();                                          // 69 0x100a0830 (folded)

    // names ours (the console)
    void AddTarget(INT ID, const TCHAR* Name);                       // 0x1006efe0 8 at most
    void Clear();                                                    // 0x1006f050
};
NW_ASSERT_OFFSET(NCSelectDeliverWnd, Selected, 0xA8);
// NW_ASSERT_OFFSET(NCSelectDeliverWnd, Targets, 0xA8);   // C2: field not mapped
NW_ASSERT_OFFSET(NCSelectDeliverWnd, NumTargets, 0x24C);
NW_ASSERT_SIZE(NCSelectDeliverWnd, 0x250);

#pragma pack(pop)