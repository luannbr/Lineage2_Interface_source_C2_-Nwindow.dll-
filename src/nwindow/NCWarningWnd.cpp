/*=============================================================================
    NCWarningWnd.cpp - the login screen's warning box. Stock: 0x10079580 .. 0x100796df and
    0x10079d70 .. 0x1007a12f (NCNoticeInfoWnd and NCNoticeWnd sit in between, NCNoticeWnd.cpp).
=============================================================================*/
#include "nwindow/NCWarningWnd.h"
#include "nwindow/NCButton.h"
#include "nwindow/NCFrameCtrl.h"
#include "nwindow/NCGDevice.h"
#include "nwindow/NCNoticeWnd.h"
#include "nwindow/NConsoleWnd.h"

// ids (names ours)
enum
{
    NID_WARNING_OK = 0xF8,
    NID_WARNING_FRAME = 0xA000F9,
};
enum { TIMER_WARNING = 0xF030D1 };  // nothing handles it (OnTimer returns 1)
enum { SOUND_Click = 0 };

// 0x1009b1f0 - the timer result and the controls are left as allocated
NCWarningWnd::NCWarningWnd()
{
    BackTex = NULL;
    Unknown98 = 0;
}
NW_HOOK_CTOR(0x1009b1f0, NCWarningWnd)

// 0x1009b240
NW_HOOK_DTOR(0x1009b240, NCWarningWnd)

// 0x1009a850 (vslot 25) - C2: no enabled test any more
INT NCWarningWnd::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (NCWnd::WindowProc(Msg, wParam, lParam))
        return 1;
    if (Msg == NWM_COMMAND && wParam == NID_WARNING_OK)
        return OnOK() != 0;
    return 0;
}
NW_HOOK_METHOD(0x1009a850, NCWarningWnd, WindowProc)

// 0x1009a8b0 (the fmap row says 0x1009a820: that one is NCNoticeWnd's twin with the text window at +0xa0)
void NCWarningWnd::SetText(const TCHAR* Text)
{
    if (Text && *Text)
        InfoWnd->SetText(Text);
}
NW_HOOK_METHOD(0x1009a8b0, NCWarningWnd, SetText)

// 0x1009a8d0
void NCWarningWnd::ClearText()
{
    InfoWnd->ClearText();
}
NW_HOOK_METHOD(0x1009a8d0, NCWarningWnd, ClearText)

// 0x1009a8e0 (vslot 67)
INT NCWarningWnd::OnOK()
{
    GConsole->RequestServerList();
    return 1;
}
NW_HOOK_METHOD(0x1009a8e0, NCWarningWnd, OnOK)

// 0x1009a900 (vslot 58) - Enter works as OK, then the key goes on to NCWnd
INT NCWarningWnd::OnKeyDown(WPARAM wParam, LPARAM lParam)
{
    if (wParam == VK_RETURN)
    {
        GConsole->RequestServerList();
        PlayInterfaceSound(SOUND_Click);
    }
    NCWnd::OnKeyDown(wParam, lParam);
    return 1;
}
NW_HOOK_METHOD(0x1009a900, NCWarningWnd, OnKeyDown)

// 0x1009a940 (vslot 43) - a one-minute timer nothing listens to
void NCWarningWnd::ShowWindow()
{
    NCWnd::ShowWindow();
    TimerResult = SetTimer(TIMER_WARNING, 60000, NULL);
    Unknown98 = 0;
}
NW_HOOK_METHOD(0x1009a940, NCWarningWnd, ShowWindow)

// 0x1009a970 (vslot 44)
void NCWarningWnd::HideWindow()
{
    NCWnd::HideWindow();
    KillTimer(TIMER_WARNING);
    Unknown98 = 0;
}
NW_HOOK_METHOD(0x1009a970, NCWarningWnd, HideWindow)

// 0x1009b290 (vslot 36) - title bar, text and OK button of a 532 x 424 box
INT NCWarningWnd::OnCreate()
{
    NCWnd::OnCreate();
    BackTex = FindTexture(TEXT("L2UI.etcWnd.Announce_back"));

    FrameCtrl = new NCFrameCtrl(1);
    FrameID = NID_WARNING_FRAME;
    FrameCtrl->ID = NID_WARNING_FRAME;
    FrameCtrl->SetWindowRect(0, 0, 532, 21);
    CreateChild(FrameCtrl, NWS_VISIBLE);

    InfoWnd = new NCNoticeInfoWnd();
    InfoWnd->SetWindowRect(6, 24, 525, 373);
    CreateChild(InfoWnd, NWS_VISIBLE);

    OKButton = new NCButton(TEXT("L2UI.control.defaultbutton"), TEXT("L2UI.control.defaultbutton_click"), 0);
    OKButton->ID = NID_WARNING_OK;
    OKButton->SetWindowRect(239, 400, 67, 19);
    CreateChild(OKButton, NWS_VISIBLE);
    OKButton->SetTextID(140);  // "OK"
    return 1;
}
NW_HOOK_METHOD(0x1009b290, NCWarningWnd, OnCreate)

// 0x1009b4a0 (vslot 37) - guarded in C2; the title bar goes with NCWnd::OnDestroy
INT NCWarningWnd::OnDestroy()
{
    guard(NCWarningWnd::OnDestroy);
    if (InfoWnd)
    {
        RemoveChildWindow(InfoWnd->Entry);
        delete InfoWnd;
        InfoWnd = NULL;
    }
    if (OKButton)
    {
        RemoveChildWindow(OKButton->Entry);
        delete OKButton;
        OKButton = NULL;
    }
    NCWnd::OnDestroy();
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x1009b4a0, NCWarningWnd, OnDestroy)

// 0x1009b570 (vslot 55) - the background starts below the 21-pixel title bar; the guard
// carries NCNoticeWnd's name in the stock code
INT NCWarningWnd::OnPaint(NCGDevice* Device)
{
    guard(NCNoticeWnd::OnPaint);
    Device->SetClip(ScreenX, ScreenY, Width, Height);
    Device->BeginPaint(ScreenX, ScreenY);
    if (BackTex)
        Device->DrawTexture(0, 21, Width, Height, 0, 0, Width, Height, BackTex);
    Device->EndPaint();
    Device->ReleaseClip();
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x1009b570, NCWarningWnd, OnPaint)

NW_CHECK_VTABLE(NCWarningWnd, 0x1011d7c8, 68)
