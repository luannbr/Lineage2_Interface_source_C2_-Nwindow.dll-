#pragma once
// NCLoginWnd - the login screen (UClass "CLoginWnd", 0xCC; NConsoleWnd +0x3528): the game
// logo, the login box (NCAuthWnd), the announcement, warning, EULA and announce boxes, the server
// list (NCServerSelectWnd), the key crypt warning box and four buttons in the lower right corner:
// "New Account" and "Lost Account" (created disabled), "Options" and (new in C2) "Credits".

#include "nwindow/NCWnd.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCAuthWnd;
class NCNoticeWnd;
class NCServerSelectWnd;
class NCWarningWnd;
class NCButton;

class NCLoginWnd : public NCWnd
{
    NW_DECLARE_CLASS(NCLoginWnd, NCWnd, 0, 0x101BB718)

    // layout from the C2 OnCreate (0x10083960) and OnDestroy (0x100842f0)
    NCAuthWnd* AuthWnd;              // +0x98
    NCNoticeWnd* NoticeWnd;          // +0x9C "Announcements"
    NCWarningWnd* WarningWnd;        // +0xA0 "Warning"
    NCWnd* EulaWnd;                  // +0xA4 NCEulaWnd (new in C2; class not declared yet)
    NCWnd* AnnounceWnd;              // +0xA8 NCAnnounceWnd (new in C2; class not declared yet)
    NCServerSelectWnd* ServerSelectWnd; // +0xAC "Choose Server"
    NCWnd* KeyCryptWnd;              // +0xB0 NCKeyCryptWarningDialogBox (new in C2; not declared yet)
    UTexture* LogoTex;               // +0xB4 FontInfo.ini "<country>_LogoTexture"
    UTexture* MiniLogoTex;           // +0xB8 FontInfo.ini "<country>_MiniLogoTexture" (C1: "L2UI.LoginWnd.mini_logo")
    NCButton* NewAccountButton;      // +0xBC
    NCButton* LostAccountButton;     // +0xC0 (no id)
    NCButton* OptionsButton;         // +0xC4
    NCButton* CreditsButton;         // +0xC8 (new in C2; name ours) text 805

    NCLoginWnd();                                                    // 0x100838c0
    // ~NCLoginWnd                                                   // 0x10083910

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x10082a90
    virtual INT OnCreate();                                          // 36 0x10083960
    virtual INT OnDestroy();                                         // 37 0x100842f0
    virtual void ShowWindow();                                       // 43 0x10082b30
    virtual void HideWindow();                                       // 44 0x10082be0
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x10084550
    // names ours
    virtual INT OnNewAccount() { return 1; }                         // 67 0x10005b80 (folded)
    virtual INT OnOptions();                                         // 68 0x10082c50 (folded, hooked as NCSystemMenuWnd::OnOptions)
    virtual INT OnCredits();                                         // 69 0x10082c70 (new in C2)

    void ResetWindows();                                             // 0x10082c10 (name ours)
};
NW_ASSERT_OFFSET(NCLoginWnd, AuthWnd, 0x98);
NW_ASSERT_OFFSET(NCLoginWnd, ServerSelectWnd, 0xAC);
NW_ASSERT_OFFSET(NCLoginWnd, KeyCryptWnd, 0xB0);
NW_ASSERT_OFFSET(NCLoginWnd, LogoTex, 0xB4);
NW_ASSERT_OFFSET(NCLoginWnd, OptionsButton, 0xC4);
NW_ASSERT_OFFSET(NCLoginWnd, CreditsButton, 0xC8);
NW_ASSERT_SIZE(NCLoginWnd, 0xCC);

#pragma pack(pop)
