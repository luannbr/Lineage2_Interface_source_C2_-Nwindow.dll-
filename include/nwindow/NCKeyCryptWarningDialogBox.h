#pragma once
// NCKeyCryptWarningDialogBox - the keyboard protection warning of the login screen (UClass
// "CKeyCryptWarningDialogBox", 0x1AC; C2 only; NCLoginWnd +0xB0): nProtect's keyboard
// protection (npkcrypt) is not running; a box with system message 1239 and an OK button that
// closes it.

#include "nwindow/NCDialogBox.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCButton;

class NCKeyCryptWarningDialogBox : public NCDialogBox
{
    NW_DECLARE_CLASS(NCKeyCryptWarningDialogBox, NCDialogBox, 0, 0x101BB1F8)

    UTexture* BackTex;               // +0x1A4 "L2UI.EtcWnd.Popup"
    NCButton* OKButton;              // +0x1A8 id 0x1004, centered, text SysString 140

    NCKeyCryptWarningDialogBox();                                    // 0x10082ce0
    // ~NCKeyCryptWarningDialogBox                                   // 0x10082d30

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x100827a0
    virtual INT OnCreate();                                          // 36 0x10082d80
    virtual INT OnDestroy();                                         // 37 0x10082e70
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x10082f20
    virtual INT OnOK();                                              // 68 0x10082800 (name ours)
};
NW_ASSERT_OFFSET(NCKeyCryptWarningDialogBox, BackTex, 0x1A4);
NW_ASSERT_OFFSET(NCKeyCryptWarningDialogBox, OKButton, 0x1A8);
NW_ASSERT_SIZE(NCKeyCryptWarningDialogBox, 0x1AC);

#pragma pack(pop)
