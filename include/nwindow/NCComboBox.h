#pragma once
// NCComboBox - drop-down list (UClass "CComboBox", 0x1F8). Closed, it shows the selected item
// and a drop button (id 0x100B); open, the control itself grows to hold the item list and
// a click picks the item under the mouse (NWM_COMMAND, lParam = index). Losing the focus
// closes it. Row height = height of L"\xAC00" (Hangul GA) in the default font.

#include "nwindow/NControl.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NString;
class NCButton;

enum { NID_COMBO_DROP = 0x100B };

class NCComboBox : public NControl
{
    NW_DECLARE_CLASS(NCComboBox, NControl, 0, 0x1019A4F8)

    INT TextX;                       // +0x1A4 (5)
    INT TextY;                       // +0x1A8 (4) also the row spacing of the open list
    TArray<NString*> Items;          // +0x1AC
    UTexture* LeftTex;               // +0x1B8 "L2UI.Control.Combobox1"   // C2: offset derived, no evidence
    UTexture* MidTex;                // +0x1BC "L2UI.Control.Combobox2"   // C2: offset derived, no evidence
    UTexture* LeftDisabledTex;       // +0x1C0 "..._unable"   // C2: offset derived, no evidence
    UTexture* MidDisabledTex;        // +0x1C4   // C2: offset derived, no evidence
    UTexture* ListLeftTex;           // +0x1C8 "L2UI.Control.Combobox_under1_1"   // C2: offset derived, no evidence
    UTexture* ListMidTex;            // +0x1CC "..._under1_2"   // C2: offset derived, no evidence
    UTexture* ListRightTex;          // +0x1D0 "..._under1_3"   // C2: offset derived, no evidence
    UTexture* ListBottomTex;         // +0x1D4 "L2UI.Control.Combobox_under2"   // C2: offset derived, no evidence
    UTexture* HighlightTex;          // +0x1D8 "NWindow.MenuHighlight"
    NCButton* DropButton;            // +0x1DC
    UBOOL bDropped;                  // +0x1E0
    INT ClosedW;                     // +0x1E4 size when closed (OnCreate)
    INT ClosedH;                     // +0x1E8
    SIZE RowSize;                    // +0x1EC extent of L"\xAC00": cy is the row height
    INT Selected;                    // +0x1F4 -1 = none
    INT Hover;                       // +0x1F8 item under the mouse in the open list

    NCComboBox();                                                    // 0x10005700
    // ~NCComboBox                                                   // 0x100057b0

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x10004c20
    virtual void KillFocus();                                        // 30 0x10004d80
    virtual INT OnCreate();                                          // 36 0x10004e30
    virtual INT OnDestroy();                                         // 37 0x100053b0
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x10004fd0
    virtual INT OnMouseMove(WPARAM wParam, LPARAM lParam);           // 60 0x10004c80
    virtual INT OnLButtonDown(WPARAM wParam, LPARAM lParam);         // 61 0x10004d10
    virtual INT DropDown();                                          // 68 0x10005380 (name ours)

    // names ours
    void SetSelected(INT Index);                                     // 0x10004dc0
    void UpdateSize();                                               // 0x10004dd0
    const TCHAR* GetItemText(INT Index);                             // 0x10005350
    void ClearItems();                                               // 0x100054b0
    void AddItem(INT SysStringID);                                   // 0x10005520
    void AddItem(const TCHAR* Text);                                 // 0x100055e0
};
NW_ASSERT_OFFSET(NCComboBox, Items, 0x1AC);
NW_ASSERT_OFFSET(NCComboBox, DropButton, 0x1DC);
NW_ASSERT_OFFSET(NCComboBox, RowSize, 0x1EC);
NW_ASSERT_OFFSET(NCComboBox, Hover, 0x1F8);
NW_ASSERT_SIZE(NCComboBox, 0x1FC);

#pragma pack(pop)