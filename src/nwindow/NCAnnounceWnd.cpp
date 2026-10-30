/*=============================================================================
    NCAnnounceWnd.cpp - the announce box of the login screen (C2 only). Stock: 0x1009aab0 ..
    0x1009ab0f (WindowProc) and 0x1009bdd0 .. 0x1009c39f.
=============================================================================*/
#include "nwindow/NCAnnounceWnd.h"
#include "nwindow/NCButton.h"
#include "nwindow/NCFrameCtrl.h"
#include "nwindow/NCGDevice.h"
#include "nwindow/NCLoginWnd.h"
#include "nwindow/NCNoticeWnd.h"
#include "nwindow/NConsoleWnd.h"

// ids (names ours)
enum
{
    NID_ANNOUNCE_FRAME = 0xA000F9,
    NID_ANNOUNCE_OK = 0xA000FC,
};
enum { SOUND_Click = 0 };

// 0x1009bdd0
NCAnnounceWnd::NCAnnounceWnd()
{
    BackTex = NULL;
}
NW_HOOK_CTOR(0x1009bdd0, NCAnnounceWnd)

// 0x1009be20
NW_HOOK_DTOR(0x1009be20, NCAnnounceWnd)

// 0x1009aab0 (vslot 25)
INT NCAnnounceWnd::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (NCWnd::WindowProc(Msg, wParam, lParam))
        return 1;
    if (Msg == NWM_COMMAND && wParam == NID_ANNOUNCE_OK)
        return OnOK() != 0;
    return 0;
}
NW_HOOK_METHOD(0x1009aab0, NCAnnounceWnd, WindowProc)

// 0x1009be70 (vslot 36) - title bar and OK button of a 532 x 424 box
INT NCAnnounceWnd::OnCreate()
{
    NCWnd::OnCreate();
    BackTex = FindTexture(TEXT("L2UI.etcWnd.Announce_cn"));

    FrameCtrl = new NCFrameCtrl(1);
    FrameID = NID_ANNOUNCE_FRAME;
    FrameCtrl->ID = NID_ANNOUNCE_FRAME;
    FrameCtrl->SetWindowRect(0, 0, 532, 21);
    CreateChild(FrameCtrl, NWS_VISIBLE);

    OKButton = new NCButton(TEXT("L2UI.control.defaultbutton"), TEXT("L2UI.control.defaultbutton_click"), 0);
    OKButton->ID = NID_ANNOUNCE_OK;
    OKButton->SetWindowRect(239, 400, 67, 19);
    CreateChild(OKButton, NWS_VISIBLE);
    OKButton->SetTextID(140);  // "OK"
    return 1;
}
NW_HOOK_METHOD(0x1009be70, NCAnnounceWnd, OnCreate)

// 0x1009bff0 (vslot 37) - guarded in C2; the title bar goes with NCWnd::OnDestroy
INT NCAnnounceWnd::OnDestroy()
{
    guard(NCAnnounceWnd::OnDestroy);
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
NW_HOOK_METHOD(0x1009bff0, NCAnnounceWnd, OnDestroy)

// 0x1009c0a0 (vslot 55) - the picture starts below the 21-pixel title bar
INT NCAnnounceWnd::OnPaint(NCGDevice* Device)
{
    guard(NCAnnounceWnd::OnPaint);
    Device->SetClip(ScreenX, ScreenY, Width, Height);
    Device->BeginPaint(ScreenX, ScreenY);
    if (BackTex)
        Device->DrawTexture(0, 21, Width, Height, 0, 0, Width, Height, BackTex);
    Device->EndPaint();
    Device->ReleaseClip();
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x1009c0a0, NCAnnounceWnd, OnPaint)

// What OK does, written out twice in the stock code (OnOK and OnKeyDown): with an EULA text
// the announcements box takes its place with that text, else the server list is asked for
static FORCEINLINE void AnnounceAnswer(NCAnnounceWnd* Wnd)
{
    if (GL2GameData.EulaText.Len() > 0)
    {
        NCNoticeWnd* Notice = GConsole->LoginWnd->NoticeWnd;
        GConsole->LoginWnd->AnnounceWnd->HideWindow();
        Notice->ShowWindow();
        Notice->SetFocus();
        Notice->ClearText();
        Notice->SetText(*GL2GameData.EulaText);
        GConsole->LoginWnd->NoticeWnd->Type = Wnd->Type;
        GConsole->SetState(L2CS_Login);
    }
    else
        GConsole->RequestServerList();
}

// 0x1009c180 (vslot 67)
INT NCAnnounceWnd::OnOK()
{
    AnnounceAnswer(this);
    return 1;
}
NW_HOOK_METHOD(0x1009c180, NCAnnounceWnd, OnOK)

// 0x1009c280 (vslot 58) - Enter does what OK does, with the click sound
INT NCAnnounceWnd::OnKeyDown(WPARAM wParam, LPARAM lParam)
{
    if (wParam == VK_RETURN)
    {
        AnnounceAnswer(this);
        PlayInterfaceSound(SOUND_Click);
    }
    NCWnd::OnKeyDown(wParam, lParam);
    return 1;
}
NW_HOOK_METHOD(0x1009c280, NCAnnounceWnd, OnKeyDown)

NW_CHECK_VTABLE(NCAnnounceWnd, 0x1011daa8, 68)
