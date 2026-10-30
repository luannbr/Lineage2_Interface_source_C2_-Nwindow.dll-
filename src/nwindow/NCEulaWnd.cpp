/*=============================================================================
    NCEulaWnd.cpp - the license agreement box of the login screen (C2 only). Stock:
    0x1009a990 .. 0x1009aaa6 and 0x1009b650 .. 0x1009bdc9.
=============================================================================*/
#include "nwindow/NCEulaWnd.h"
#include "nwindow/NCAuthWnd.h"
#include "nwindow/NCButton.h"
#include "nwindow/NCEditBox.h"
#include "nwindow/NCFrameCtrl.h"
#include "nwindow/NCGDevice.h"
#include "nwindow/NCLoginWnd.h"
#include "nwindow/NCNoticeWnd.h"
#include "nwindow/NConsoleWnd.h"

// ids (names ours)
enum
{
    NID_EULA_FRAME = 0xA000F9,
    NID_EULA_AGREE = 0xA000FA,
    NID_EULA_DISAGREE = 0xA000FB,
};
enum { SOUND_Click = 0 };

// 0x1009b650
NCEulaWnd::NCEulaWnd()
{
    BackTex = NULL;
}
NW_HOOK_CTOR(0x1009b650, NCEulaWnd)

// 0x1009b6a0
NW_HOOK_DTOR(0x1009b6a0, NCEulaWnd)

// 0x1009a990 (vslot 25)
INT NCEulaWnd::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (NCWnd::WindowProc(Msg, wParam, lParam))
        return 1;
    if (Msg == NWM_COMMAND)
    {
        if (wParam == NID_EULA_AGREE)
            return OnAgree() != 0;
        if (wParam == NID_EULA_DISAGREE)
            return OnDisagree() != 0;
    }
    return 0;
}
NW_HOOK_METHOD(0x1009a990, NCEulaWnd, WindowProc)

// 0x1009aa10 - like NCWarningWnd::SetText
void NCEulaWnd::SetText(const TCHAR* Text)
{
    if (Text && *Text)
        InfoWnd->SetText(Text);
}
NW_HOOK_METHOD(0x1009aa10, NCEulaWnd, SetText)

// 0x1009aa30
void NCEulaWnd::ClearText()
{
    InfoWnd->ClearText();
}
NW_HOOK_METHOD(0x1009aa30, NCEulaWnd, ClearText)

// 0x1009aa40 (vslot 68) - back to the login box: the connection is dropped, the boxes emptied
// and the account line focused
INT NCEulaWnd::OnDisagree()
{
    GConsole->Engine->NetHandler->AuthReconnect();
    GConsole->LoginWnd->ResetWindows();
    GConsole->LoginWnd->ShowWindow();
    GConsole->LoginWnd->AuthWnd->AccountEdit->SetFocus();
    GConsole->SetState(L2CS_Login);
    return 1;
}
NW_HOOK_METHOD(0x1009aa40, NCEulaWnd, OnDisagree)

// 0x1009b6f0 (vslot 36) - title bar, the text and the two buttons of a 532 x 424 box
INT NCEulaWnd::OnCreate()
{
    NCWnd::OnCreate();
    BackTex = FindTexture(TEXT("L2UI.etcWnd.Announce_back"));

    FrameCtrl = new NCFrameCtrl(1);
    FrameID = NID_EULA_FRAME;
    FrameCtrl->ID = NID_EULA_FRAME;
    FrameCtrl->SetWindowRect(0, 0, 532, 21);
    CreateChild(FrameCtrl, NWS_VISIBLE);

    InfoWnd = new NCNoticeInfoWnd();
    InfoWnd->SetWindowRect(6, 24, 525, 373);
    CreateChild(InfoWnd, NWS_VISIBLE);

    AgreeButton = new NCButton(TEXT("L2UI.control.defaultbutton"), TEXT("L2UI.control.defaultbutton_click"), 0);
    AgreeButton->ID = NID_EULA_AGREE;
    AgreeButton->SetWindowRect(166, 400, 67, 19);
    CreateChild(AgreeButton, NWS_VISIBLE);
    AgreeButton->SetTextID(599);

    DisagreeButton = new NCButton(TEXT("L2UI.control.defaultbutton"), TEXT("L2UI.control.defaultbutton_click"), 0);
    DisagreeButton->ID = NID_EULA_DISAGREE;
    DisagreeButton->SetWindowRect(299, 400, 67, 19);
    CreateChild(DisagreeButton, NWS_VISIBLE);
    DisagreeButton->SetTextID(600);
    return 1;
}
NW_HOOK_METHOD(0x1009b6f0, NCEulaWnd, OnCreate)

// 0x1009b9b0 (vslot 37) - guarded in C2; the title bar goes with NCWnd::OnDestroy
INT NCEulaWnd::OnDestroy()
{
    guard(NCEulaWnd::OnDestroy);
    if (InfoWnd)
    {
        RemoveChildWindow(InfoWnd->Entry);
        delete InfoWnd;
        InfoWnd = NULL;
    }
    if (AgreeButton)
    {
        RemoveChildWindow(AgreeButton->Entry);
        delete AgreeButton;
        AgreeButton = NULL;
    }
    if (DisagreeButton)
    {
        RemoveChildWindow(DisagreeButton->Entry);
        delete DisagreeButton;
        DisagreeButton = NULL;
    }
    NCWnd::OnDestroy();
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x1009b9b0, NCEulaWnd, OnDestroy)

// 0x1009bab0 (vslot 55) - the picture starts below the 21-pixel title bar; the guard carries
// NCNoticeWnd's name in the stock code (the function map had named this NCNoticeWnd::OnPaint)
INT NCEulaWnd::OnPaint(NCGDevice* Device)
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
NW_HOOK_METHOD(0x1009bab0, NCEulaWnd, OnPaint)

// What agreeing does, written out twice in the stock code (OnAgree and OnKeyDown): with an
// EULA text the announcements box takes its place with that text, else the server list is
// asked for
static FORCEINLINE void EulaAnswer(NCEulaWnd* Wnd)
{
    if (GL2GameData.EulaText.Len() > 0)
    {
        Wnd->ClearText();
        NCNoticeWnd* Notice = GConsole->LoginWnd->NoticeWnd;
        GConsole->LoginWnd->EulaWnd->HideWindow();
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

// 0x1009bb90 (vslot 67)
INT NCEulaWnd::OnAgree()
{
    EulaAnswer(this);
    return 1;
}
NW_HOOK_METHOD(0x1009bb90, NCEulaWnd, OnAgree)

// 0x1009bca0 (vslot 58) - Enter agrees, with the click sound
INT NCEulaWnd::OnKeyDown(WPARAM wParam, LPARAM lParam)
{
    if (wParam == VK_RETURN)
    {
        EulaAnswer(this);
        PlayInterfaceSound(SOUND_Click);
    }
    NCWnd::OnKeyDown(wParam, lParam);
    return 1;
}
NW_HOOK_METHOD(0x1009bca0, NCEulaWnd, OnKeyDown)

NW_CHECK_VTABLE(NCEulaWnd, 0x1011d938, 69)
