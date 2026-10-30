#pragma once
// NCCalcWnd - the calculator (UClass "CCalcWnd", 0xF0; C2 only): a title bar, the display
// (NCCalcEditCtrl) and a keypad of 19 buttons whose clicks are sent to the display as typed
// characters, plus a close button. The window takes the keyboard focus for its display.

#include "nwindow/NCWnd.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCButton;
class NCCalcEditCtrl;

class NCCalcWnd : public NCWnd
{
    NW_DECLARE_CLASS(NCCalcWnd, NCWnd, 0, 0x101E0710)

    UTexture* BackTex;               // +0x98 "l2ui.calcwnd.calcwnd_back"
    NCCalcEditCtrl* Display;         // +0x9C (name ours)
    NCButton* Keys[19];              // +0xA0 the keypad, NotifyID = the key's number (name ours):
                                     //   0 7, 1 8, 2 9, 3 +, 4 clear entry, 5 4, 6 5, 7 6, 8 -, 9 clear all,
                                     //   10 1, 11 2, 12 3, 13 *, 14 backspace, 15 0, 16 00, 17 /, 18 =
    NCButton* CloseButton;           // +0xEC text 646 (name ours)

    NCCalcWnd();                                                     // 0x100decb0
    // ~NCCalcWnd                                                    // 0x100ded00

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x100de690
    virtual void SetFocus();                                         // 29 0x100de710
    virtual INT OnCreate();                                          // 36 0x100df550
    virtual INT OnDestroy();                                         // 37 0x100ded50
    virtual void ShowWindow();                                       // 43 0x100df470
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x100df020
    virtual INT OnKey(INT Key);                                      // 67 0x100dee50 (name ours)
    virtual INT OnClose();                                           // 68 0x100df450 (name ours)

    // name ours
    __declspec(noinline) void Reset();                               // 0x100df100 the display's Reset
};
NW_ASSERT_OFFSET(NCCalcWnd, BackTex, 0x98);
NW_ASSERT_OFFSET(NCCalcWnd, Display, 0x9C);
NW_ASSERT_OFFSET(NCCalcWnd, Keys, 0xA0);
NW_ASSERT_OFFSET(NCCalcWnd, CloseButton, 0xEC);
NW_ASSERT_SIZE(NCCalcWnd, 0xF0);

#pragma pack(pop)
