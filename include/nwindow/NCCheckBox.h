#pragma once
// NCCheckBox - check box with a label (UClass "CCheckBox", 0x2C4). A click toggles bChecked
// and reports it to the parent (NWM_COMMAND, lParam = new state), unless bDisabled.

#include "nwindow/NControl.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCCheckBox : public NControl
{
    NW_DECLARE_CLASS(NCCheckBox, NControl, 0, 0x10199AA0)

    UTexture* CheckedTex;            // +0x1A4 "L2UI.Control.CheckBox_Checked"
    UTexture* UncheckedTex;          // +0x1A8 "L2UI.Control.CheckBox"
    UTexture* UncheckedDisabledTex;  // +0x1AC "L2UI.Control.CheckBox_Unable"
    UTexture* CheckedDisabledTex;    // +0x1B0 "L2UI.Control.CheckBox_Checked_unable"
    FString Label;                   // +0x1B4 (C1: TCHAR Label[128])
    UBOOL bChecked;                  // +0x1C0
    UBOOL bDisabled;                 // +0x1C4 grey, clicks ignored
    UBOOL bLabelLeft;                // +0x1C8 label first, box after it
    INT BoxW;                        // +0x1CC (12)
    INT BoxH;                        // +0x1D0 (12)

    NCCheckBox(const TCHAR* InLabel = TEXT("default"), UBOOL bInChecked = 0,
               UBOOL bInLabelLeft = 0);                                  // 0x10003df0
    NCCheckBox(INT LabelID, UBOOL bInChecked, UBOOL bInLabelLeft);           // 0x100047a0 SysString
    // ~NCCheckBox                                                       // 0x10004730

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);      // 25 0x10090b60 (NCButton's body)
    virtual INT OnCreate();                                              // 36 0x10004530
    virtual INT OnDestroy();                                             // 37 0x10004580
    virtual INT OnPaint(NCGDevice* Device);                              // 55 0x100048a0
    virtual INT OnLButtonDown(WPARAM wParam, LPARAM lParam);             // 61 0x10004600
};
NW_ASSERT_OFFSET(NCCheckBox, Label, 0x1B4);
NW_ASSERT_OFFSET(NCCheckBox, bChecked, 0x1C0);
NW_ASSERT_OFFSET(NCCheckBox, BoxH, 0x1D0);
NW_ASSERT_SIZE(NCCheckBox, 0x1D4);

#pragma pack(pop)