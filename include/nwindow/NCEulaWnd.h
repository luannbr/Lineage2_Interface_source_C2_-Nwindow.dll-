#pragma once
// NCEulaWnd - the license agreement box of the login screen (UClass "CEulaWnd", 0xAC; C2 only;
// NCLoginWnd +0xA4): the text in an NCNoticeInfoWnd and two buttons, agree (or Enter), which
// goes on to the announcements (NCNoticeWnd, with the EULA text) when there is one or else
// asks for the server list, and the other one, which drops the connection and goes back to
// the login box.

#include "nwindow/NCWnd.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCButton;
class NCNoticeInfoWnd;

class NCEulaWnd : public NCWnd
{
    NW_DECLARE_CLASS(NCEulaWnd, NCWnd, 0, 0x101C5190)

    UTexture* BackTex;               // +0x98 "L2UI.etcWnd.Announce_back" below the title bar
    NCNoticeInfoWnd* InfoWnd;        // +0x9C the text
    NCButton* AgreeButton;           // +0xA0 text 599 (name ours)
    NCButton* DisagreeButton;        // +0xA4 text 600 (name ours)
    INT Type;                        // +0xA8 (name ours) set from outside; agreeing gives it to the notice box's Type

    NCEulaWnd();                                                     // 0x1009b650
    // ~NCEulaWnd                                                    // 0x1009b6a0

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x1009a990
    virtual INT OnCreate();                                          // 36 0x1009b6f0
    virtual INT OnDestroy();                                         // 37 0x1009b9b0
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x1009bab0
    virtual INT OnKeyDown(WPARAM wParam, LPARAM lParam);             // 58 0x1009bca0
    virtual INT OnAgree();                                           // 67 0x1009bb90 (name ours)
    virtual INT OnDisagree();                                        // 68 0x1009aa40 (name ours)

    // names ours (the text box's, like NCWarningWnd's)
    void SetText(const TCHAR* Text);                                 // 0x1009aa10 empty: ignored
    void ClearText();                                                // 0x1009aa30
};
NW_ASSERT_OFFSET(NCEulaWnd, BackTex, 0x98);
NW_ASSERT_OFFSET(NCEulaWnd, InfoWnd, 0x9C);
NW_ASSERT_OFFSET(NCEulaWnd, DisagreeButton, 0xA4);
NW_ASSERT_OFFSET(NCEulaWnd, Type, 0xA8);
NW_ASSERT_SIZE(NCEulaWnd, 0xAC);

#pragma pack(pop)
