#pragma once
// NCWarningWnd - the login screen's "Warning" box (UClass "CWarningWnd", 0xA8; NCLoginWnd
// creates it): a text in an NCNoticeInfoWnd and an OK button (or Enter) that asks for the
// server list. A copy of NCNoticeWnd's code: even its OnPaint guard is NCNoticeWnd's.

#include "nwindow/NCWnd.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCButton;
class NCNoticeInfoWnd;

class NCWarningWnd : public NCWnd
{
    NW_DECLARE_CLASS(NCWarningWnd, NCWnd, 0, 0x101C5BD8)

    INT TimerResult;                 // +0x98 SetTimer's (ShowWindow)
    INT Unknown98;                   // +0x9C (0) cleared by ShowWindow / HideWindow
    UTexture* BackTex;               // +0xA0 "L2UI.etcWnd.Announce_back" below the title bar
    NCNoticeInfoWnd* InfoWnd;        // +0xA4 the text
    NCButton* OKButton;              // +0xA8

    NCWarningWnd();                                                  // 0x1009b1f0
    // ~NCWarningWnd                                                 // 0x1009b240

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x1009a850
    virtual INT OnTimer(INT TimerID) { return 1; }                   // 35 0x100ab570 (folded)
    virtual INT OnCreate();                                          // 36 0x1009b290
    virtual INT OnDestroy();                                         // 37 0x1009b4a0
    virtual void ShowWindow();                                       // 43 0x1009a940
    virtual void HideWindow();                                       // 44 0x1009a970
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x1009b570
    virtual INT OnKeyDown(WPARAM wParam, LPARAM lParam);             // 58 0x1009a900
    // names ours
    virtual INT OnOK();                                              // 67 0x1009a8e0 server list

    void SetText(const TCHAR* Text);                                 // 0x1009a820 empty: ignored
    void ClearText();                                                // 0x1009a840
};
NW_ASSERT_OFFSET(NCWarningWnd, TimerResult, 0x98);
NW_ASSERT_OFFSET(NCWarningWnd, BackTex, 0xA0);
NW_ASSERT_OFFSET(NCWarningWnd, OKButton, 0xA8);
NW_ASSERT_SIZE(NCWarningWnd, 0xAC);

#pragma pack(pop)