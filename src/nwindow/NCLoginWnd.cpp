/*=============================================================================
    NCLoginWnd.cpp - the login screen. Stock: 0x10082a90 .. 0x10082cdd (0x10082c50 is a body
    the linker folded with NCSystemMenuWnd's), 0x100838c0 .. 0x1008462f (NCAuthWnd's functions sit in
    between).
=============================================================================*/
#include "nwindow/NCLoginWnd.h"
#include "nwindow/NCAuthWnd.h"
#include "nwindow/NCAnnounceWnd.h"
#include "nwindow/NCEulaWnd.h"
#include "nwindow/NCKeyCryptWarningDialogBox.h"
#include "nwindow/NCNoticeWnd.h"
#include "nwindow/NCServerSelectWnd.h"
#include "nwindow/NKeyCrypt.h"
#include "nwindow/NCWarningWnd.h"
#include "nwindow/NCButton.h"
#include "nwindow/NCCreditWnd.h"
#include "nwindow/NCMessageWnd.h"
#include "nwindow/NCGDevice.h"
#include "nwindow/NConsoleWnd.h"

// NWM_COMMAND ids of the buttons (names ours; "Lost Account" has none)
enum
{
    NID_LOGIN_OPTIONS = (INT)0xAFF12011,
    NID_LOGIN_NEWACCOUNT = (INT)0xAFF12012,
    NID_LOGIN_CREDITS = (INT)0xAFF12014,
};

// 0x100838c0 - only the logo is cleared; the rest is left as allocated
NCLoginWnd::NCLoginWnd()
{
    LogoTex = NULL;
}
NW_HOOK_CTOR(0x100838c0, NCLoginWnd)

// 0x10083910
NW_HOOK_DTOR(0x10083910, NCLoginWnd)

// 0x10082a90 (vslot 25) - C2 no longer refuses the messages of a disabled window here
INT NCLoginWnd::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (NCWnd::WindowProc(Msg, wParam, lParam))
        return 1;
    if (Msg == NWM_COMMAND)
    {
        if ((INT)wParam == NID_LOGIN_NEWACCOUNT)
            return OnNewAccount() != 0;
        if ((INT)wParam == NID_LOGIN_OPTIONS)
            return OnOptions() != 0;
        if ((INT)wParam == NID_LOGIN_CREDITS)
            return OnCredits() != 0;
    }
    return 0;
}
NW_HOOK_METHOD(0x10082a90, NCLoginWnd, WindowProc)

// 0x10083960 (vslot 36) - the boxes are centered on the screen (Width / Height halved
// unsigned, as in the stock code), the buttons stacked in the lower right corner
INT NCLoginWnd::OnCreate()
{
    NCWnd::OnCreate();
    FString LogoName;
    TCHAR Key[256];
    appSprintf(Key, TEXT("%s_LogoTexture"), GCountryName);
    if (!GConfig->GetString(GCountryName, Key, LogoName, TEXT("FontInfo.ini")))
        LogoName = TEXT("L2Font.start_logo-k");
    LogoTex = FindTexture(*LogoName);
    appSprintf(Key, TEXT("%s_MiniLogoTexture"), GCountryName);
    if (!GConfig->GetString(GCountryName, Key, LogoName, TEXT("FontInfo.ini")))
        LogoName = TEXT("L2Font.start_minilogo-k");
    MiniLogoTex = FindTexture(*LogoName);

    AuthWnd = new NCAuthWnd;
    AuthWnd->SetWindowRect(((DWORD)Width >> 1) - 107, ((DWORD)Height >> 1) - 63, 214, 148);
    CreateChild(AuthWnd, NWS_NOCLOSE | NWS_VISIBLE);
    AuthWnd->SetTitle(*GL2GameData.SysString[149]);            // "Log In"

    NoticeWnd = new NCNoticeWnd;                                // constructor 0x1009ACF0
    NoticeWnd->SetWindowRect(((DWORD)Width >> 1) - 266, ((DWORD)Height >> 1) - 213, 532, 424);
    CreateChild(NoticeWnd, NWS_NOCLOSE | NWS_VISIBLE);
    NoticeWnd->SetTitle(*GL2GameData.SysString[328]);          // "Announcements"

    WarningWnd = new NCWarningWnd;                              // constructor 0x1009B1F0
    WarningWnd->SetWindowRect(((DWORD)Width >> 1) - 266, ((DWORD)Height >> 1) - 213, 532, 424);
    CreateChild(WarningWnd, NWS_NOCLOSE | NWS_VISIBLE);
    WarningWnd->SetTitle(*GL2GameData.SysString[538]);         // "Warning"

    EulaWnd = new NCEulaWnd;                                   // constructor 0x1009B650
    EulaWnd->SetWindowRect(((DWORD)Width >> 1) - 266, ((DWORD)Height >> 1) - 213, 532, 424);
    CreateChild(EulaWnd, NWS_NOCLOSE | NWS_VISIBLE);
    EulaWnd->SetTitle(*GL2GameData.SysString[598]);            // the EULA's title

    AnnounceWnd = new NCAnnounceWnd;                           // constructor 0x1009BDD0
    AnnounceWnd->SetWindowRect(((DWORD)Width >> 1) - 266, ((DWORD)Height >> 1) - 212, 532, 424);
    CreateChild(AnnounceWnd, NWS_NOCLOSE | NWS_VISIBLE);

    ServerSelectWnd = new NCServerSelectWnd;
    ServerSelectWnd->SetWindowRect(((DWORD)Width >> 1) - 150, ((DWORD)Height >> 1) - 173, 300, 526);
    CreateChild(ServerSelectWnd, NWS_NOCLOSE | NWS_VISIBLE);
    ServerSelectWnd->SetTitle(*GL2GameData.SysString[449]);    // "Choose Server"

    NewAccountButton = new NCButton(TEXT("L2UI.ActionWnd.PledgeButten1"),
                                    TEXT("L2UI.ActionWnd.PledgeButten2"), 0);
    NewAccountButton->ID = NID_LOGIN_NEWACCOUNT;
    NewAccountButton->SetWindowRect(Width - 132, Height - 148, 99, 19);
    CreateChild(NewAccountButton, NWS_VISIBLE);
    NewAccountButton->SetTextID(154);                          // "New Account"
    NewAccountButton->EnableWindow(0);

    LostAccountButton = new NCButton(TEXT("L2UI.ActionWnd.PledgeButten1"),
                                     TEXT("L2UI.ActionWnd.PledgeButten2"), 0);
    LostAccountButton->SetWindowRect(Width - 132, Height - 124, 99, 19);
    CreateChild(LostAccountButton, NWS_VISIBLE);
    LostAccountButton->SetTextID(155);                         // "Lost Account"
    LostAccountButton->EnableWindow(0);

    OptionsButton = new NCButton(TEXT("L2UI.ActionWnd.PledgeButten1"),
                                 TEXT("L2UI.ActionWnd.PledgeButten2"), 0);
    OptionsButton->ID = NID_LOGIN_OPTIONS;
    OptionsButton->SetWindowRect(Width - 132, Height - 100, 99, 19);
    CreateChild(OptionsButton, NWS_VISIBLE);
    OptionsButton->SetTextID(156);                             // "Options"

    CreditsButton = new NCButton(TEXT("L2UI.ActionWnd.PledgeButten1"),
                                 TEXT("L2UI.ActionWnd.PledgeButten2"), 0);
    CreditsButton->ID = NID_LOGIN_CREDITS;
    CreditsButton->SetWindowRect(Width - 132, Height - 76, 99, 19);
    CreateChild(CreditsButton, NWS_VISIBLE);
    CreditsButton->SetTextID(805);                             // "Credits"

    KeyCryptWnd = new NCKeyCryptWarningDialogBox;              // constructor 0x10082CE0
    KeyCryptWnd->SetWindowRect((GSEKScreenX >> 1) - 101, (GSEKScreenY >> 1) - 43, 205, 87);
    CreateChild(KeyCryptWnd, NWS_VISIBLE);
    KeyCryptWnd->HideWindow();
    return 1;
}
NW_HOOK_METHOD(0x10083960, NCLoginWnd, OnCreate)

// 0x100842f0 (vslot 37)
INT NCLoginWnd::OnDestroy()
{
    if (AuthWnd)
    {
        RemoveChildWindow(AuthWnd->Entry);
        delete AuthWnd;
        AuthWnd = NULL;
    }
    if (NoticeWnd)
    {
        RemoveChildWindow(NoticeWnd->Entry);
        delete NoticeWnd;
        NoticeWnd = NULL;
    }
    if (WarningWnd)
    {
        RemoveChildWindow(WarningWnd->Entry);
        delete WarningWnd;
        WarningWnd = NULL;
    }
    if (EulaWnd)
    {
        RemoveChildWindow(EulaWnd->Entry);
        delete EulaWnd;
        EulaWnd = NULL;
    }
    if (AnnounceWnd)
    {
        RemoveChildWindow(AnnounceWnd->Entry);
        delete AnnounceWnd;
        AnnounceWnd = NULL;
    }
    if (ServerSelectWnd)
    {
        RemoveChildWindow(ServerSelectWnd->Entry);
        delete ServerSelectWnd;
        ServerSelectWnd = NULL;
    }
    if (NewAccountButton)
    {
        RemoveChildWindow(NewAccountButton->Entry);
        delete NewAccountButton;
        NewAccountButton = NULL;
    }
    if (LostAccountButton)
    {
        RemoveChildWindow(LostAccountButton->Entry);
        delete LostAccountButton;
        LostAccountButton = NULL;
    }
    if (OptionsButton)
    {
        RemoveChildWindow(OptionsButton->Entry);
        delete OptionsButton;
        OptionsButton = NULL;
    }
    if (CreditsButton)
    {
        RemoveChildWindow(CreditsButton->Entry);
        delete CreditsButton;
        CreditsButton = NULL;
    }
    if (KeyCryptWnd)
    {
        RemoveChildWindow(KeyCryptWnd->Entry);
        delete KeyCryptWnd;
        KeyCryptWnd = NULL;
    }
    NCWnd::OnDestroy();
    return 1;
}
NW_HOOK_METHOD(0x100842f0, NCLoginWnd, OnDestroy)

// 0x10082b30 (vslot 43) - only the login box stays up; the key crypt box is shown (and takes the
// focus) while the key crypt module (GL2UseKeyCrypt) is on and not yet registered
void NCLoginWnd::ShowWindow()
{
    NCWnd::ShowWindow();
    WarningWnd->HideWindow();
    NoticeWnd->HideWindow();
    EulaWnd->HideWindow();
    AnnounceWnd->HideWindow();
    ServerSelectWnd->HideWindow();
    if (GL2UseKeyCrypt)
    {
        NKeyCrypt* Crypt = GConsole->KeyCrypt;                  // NConsoleWnd +0x359C (new in C2)
        if (!Crypt || !Crypt->Open() || !GConsole->KeyCrypt->RegisterMessage())
        {
            ((NCKeyCryptWarningDialogBox*)KeyCryptWnd)->ShowModal();   // 0x10006DC0, shows the box on top, modal
            KeyCryptWnd->SetFocus();
        }
        else
            KeyCryptWnd->HideWindow();
    }
    else
        KeyCryptWnd->HideWindow();
}
NW_HOOK_METHOD(0x10082b30, NCLoginWnd, ShowWindow)

// 0x10082be0 (vslot 44) - with the key crypt module on, the driver is closed along with the screen
// (NKeyCrypt::Close on the object at NConsoleWnd +0x359C)
void NCLoginWnd::HideWindow()
{
    NCWnd::HideWindow();
    if (GL2UseKeyCrypt)
    {
        NKeyCrypt* Crypt = GConsole->KeyCrypt;
        if (Crypt)
            Crypt->Close();
    }
}
NW_HOOK_METHOD(0x10082be0, NCLoginWnd, HideWindow)

// 0x10084550 (vslot 55) - the small logo in the upper left corner (232 x 120 in C2), the big one
// centered at a twentieth of the height (neither is checked for NULL)
INT NCLoginWnd::OnPaint(NCGDevice* Device)
{
    guard(NCLoginWnd::OnPaint);
    Device->SetClip(ScreenX, ScreenY, Width, Height);
    Device->BeginPaint(ScreenX, ScreenY);
    Device->DrawTexture(0, 0, 232, 120, 0, 0, 232, 120, MiniLogoTex);
    Device->DrawTexture(((DWORD)Width >> 1) - 128, (DWORD)Height / 20, 256, 256, 0, 0, 256, 256,
                        LogoTex);
    Device->EndPaint();
    Device->ReleaseClip();
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x10084550, NCLoginWnd, OnPaint)

// stock body shared with NCSystemMenuWnd's (0x10082c50, hooked there): "Options" opens or closes the
// option window
INT NCLoginWnd::OnOptions()
{
    GConsole->ToggleOptionWnd();
    return 1;
}

// 0x10082c70 (vslot 69, new in C2) - "Credits": the login screen and the line under it go away,
// the credit window (NConsoleWnd +0x352C) takes the screen
INT NCLoginWnd::OnCredits()
{
    GConsole->LoginWnd->HideWindow();
    GConsole->MessageWnd->HideWindow();
    GConsole->CreditWnd->ShowWindow();
    ((NCCreditWnd*)GConsole->CreditWnd)->Start();               // 0x1006E650, reads the credit text and starts it
    GConsole->CreditWnd->SetFocus();
    nw::VirtualCall<void>(GConsole, 300);                       // NConsoleWnd vslot 300 (not named yet)
    return 1;
}
NW_HOOK_METHOD(0x10082c70, NCLoginWnd, OnCredits)

// 0x10082c10 - back to an empty login screen (failed login, server list cancelled...)
void NCLoginWnd::ResetWindows()
{
    AuthWnd->ClearPassword();
    NoticeWnd->ClearText();                                      // 0x1009a840
    WarningWnd->ClearText();                                     // 0x1009a8d0
    ((NCEulaWnd*)EulaWnd)->ClearText();                         // 0x1009aa30
    ServerSelectWnd->ClearServers();
}
NW_HOOK_METHOD(0x10082c10, NCLoginWnd, ResetWindows)

NW_CHECK_VTABLE(NCLoginWnd, 0x10119888, 70)
