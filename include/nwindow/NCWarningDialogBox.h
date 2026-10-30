#pragma once
// NCWarningDialogBox - the small box with a text and an OK button (UClass "CWarningDialogBox",
// 0x1BC; C2 only; NConsoleWnd +0x3554, made by Initialize): the console shows it modal with a
// text (SetText keeps the first one it gets until the box is closed). OK closes it, or, when
// the box was raised for a fatal reason (bExit), ends the client.

#include "nwindow/NCDialogBox.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCButton;

class NCWarningDialogBox : public NCDialogBox
{
    NW_DECLARE_CLASS(NCWarningDialogBox, NCDialogBox, 0, 0x1019BEB0)

    UTexture* BackTex;               // +0x1A4 "L2UI.EtcWnd.Popup"
    NCButton* OKButton;              // +0x1A8 id 0x1004, centered, text SysString 140
    FString Text;                    // +0x1AC the message (name ours)
    INT bExit;                       // +0x1B8 OK ends the client (name ours; set from outside)

    NCWarningDialogBox();                                            // 0x100090e0
    // ~NCWarningDialogBox                                           // 0x10009170

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x100827a0 (folded with
                                                                     //    NCKeyCryptWarningDialogBox's)
    virtual INT OnCreate();                                          // 36 0x100092f0
    virtual INT OnDestroy();                                         // 37 0x100093e0
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x100091e0
    virtual INT OnOK();                                              // 68 0x10009490 (name ours)

    // name ours: a text only while none is set
    void SetText(const TCHAR* InText);                               // 0x10006e00
};
NW_ASSERT_OFFSET(NCWarningDialogBox, BackTex, 0x1A4);
NW_ASSERT_OFFSET(NCWarningDialogBox, OKButton, 0x1A8);
NW_ASSERT_OFFSET(NCWarningDialogBox, Text, 0x1AC);
NW_ASSERT_OFFSET(NCWarningDialogBox, bExit, 0x1B8);
NW_ASSERT_SIZE(NCWarningDialogBox, 0x1BC);

#pragma pack(pop)
