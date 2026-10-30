/*=============================================================================
    NCNoticeWnd.cpp - the announcements box and its text. Stock: 0x10079480 .. 0x1007956d,
    0x100796e0 .. 0x10079d69 (NCWarningWnd's functions sit in between and after).
=============================================================================*/
#include "nwindow/NCNoticeWnd.h"
#include "nwindow/NCButton.h"
#include "nwindow/NCFrameCtrl.h"
#include "nwindow/NCGDevice.h"
#include "nwindow/NConsoleWnd.h"

#include <wchar.h>

// NWM_COMMAND id of OK, title bar id (names ours)
enum
{
    NID_NOTICE_OK = 0xF9,
    NID_NOTICE_FRAME = 0xA000F9,
};

// interface sounds (FL2NetNotify::OnInterfacePlaySound)
enum { SOUND_Click = 0 };

/*-----------------------------------------------------------------------------
    NCNoticeInfoWnd.
-----------------------------------------------------------------------------*/

// 0x1009ab10 - lines 16 pixels apart in the scroll window
NCNoticeInfoWnd::NCNoticeInfoWnd()
{
    Text[0] = 0;
    Unknown0C8 = 16;
}
NW_HOOK_CTOR(0x1009ab10, NCNoticeInfoWnd)

// 0x1009ab70
NW_HOOK_DTOR(0x1009ab70, NCNoticeInfoWnd)

// vslot 25: the stock slot is 0x10019810, the body shared with NCItemWnd's (hooked there): the base's answer as a
// plain 0 / 1 (C2 has no enabled test here)
INT NCNoticeInfoWnd::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    return NCScrollWnd::WindowProc(Msg, wParam, lParam) != 0;
}

// 0x1002ce20 (vslot 36) - the stock body is shared with NCQuestInfoWnd's (folded)
INT NCNoticeInfoWnd::OnCreate()
{
    NCScrollWnd::OnCreate();
    return 1;
}
NW_HOOK_METHOD(0x1002ce20, NCNoticeInfoWnd, OnCreate)

// 0x1009a700 
INT NCNoticeInfoWnd::OnDestroy()
{
    guard(NCNoticeInfoWnd::OnDestroy);
    NCScrollWnd::OnDestroy();
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x1009a700, NCNoticeInfoWnd, OnDestroy)

// 0x1009abc0 - the text wrapped at the width less the scroll bar, shifted by the
// scroll position (NCGDevice's origin stack); its height sets the scroll range. The clip goes
// on before BeginPaint.
INT NCNoticeInfoWnd::OnPaint(NCGDevice* Device)
{
    guard(NCNoticeInfoWnd::OnPaint);
    Device->SetClip(ScreenX, ScreenY, Width, Height);
    Device->BeginPaint(ScreenX, ScreenY);
    const TCHAR* Str = Text;
    if (Str && *Str)
    {
        Device->PushOrigin(ScrollX, ScrollY);
        Device->DrawText(2, 5, 0xFFFFFFFF, Text, Width - 20, 0, 1, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
        SIZE Size;
        NCGDevice::GetTextExtent(&Size, Text, Width - 20, 0, 1, 0, 0);
        SetContentHeight(Size.cy);
        Device->PopOrigin();
    }
    Device->EndPaint();
    Device->ReleaseClip();
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x1009abc0, NCNoticeInfoWnd, OnPaint)

// 0x1009a780 - an empty text leaves the old one (wcscpy, as in the stock code)
__declspec(noinline) void NCNoticeInfoWnd::SetText(const TCHAR* InText)
{
    if (InText && *InText)
        wcscpy(Text, InText);
}
NW_HOOK_METHOD(0x1009a780, NCNoticeInfoWnd, SetText)

// 0x1009a7b0
__declspec(noinline) void NCNoticeInfoWnd::ClearText()
{
    Text[0] = 0;
}
NW_HOOK_METHOD(0x1009a7b0, NCNoticeInfoWnd, ClearText)

NW_CHECK_VTABLE(NCNoticeInfoWnd, 0x1011d458, 70)

/*-----------------------------------------------------------------------------
    NCNoticeWnd.
-----------------------------------------------------------------------------*/

// 0x1009acf0
NCNoticeWnd::NCNoticeWnd()
{
    BackTex = NULL;
    Type = 0;
}
NW_HOOK_CTOR(0x1009acf0, NCNoticeWnd)

// 0x1009ad40
NW_HOOK_DTOR(0x1009ad40, NCNoticeWnd)

// 0x1009a7c0 (vslot 25)
INT NCNoticeWnd::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (NCWnd::WindowProc(Msg, wParam, lParam))
        return 1;
    if (Msg == NWM_COMMAND && wParam == NID_NOTICE_OK)
        return OnOK() != 0;
    return 0;
}
NW_HOOK_METHOD(0x1009a7c0, NCNoticeWnd, WindowProc)

// 0x1009ad90 (vslot 36) - title bar 532 wide, the text under it, OK at the bottom
INT NCNoticeWnd::OnCreate()
{
    NCWnd::OnCreate();
    BackTex = FindTexture(TEXT("L2UI.etcWnd.Announce_back"));

    FrameCtrl = new NCFrameCtrl(1);
    FrameID = NID_NOTICE_FRAME;
    FrameCtrl->ID = NID_NOTICE_FRAME;
    FrameCtrl->SetWindowRect(0, 0, 532, 21);
    CreateChild(FrameCtrl, NWS_VISIBLE);

    InfoWnd = new NCNoticeInfoWnd;
    InfoWnd->SetWindowRect(6, 24, 525, 373);
    CreateChild(InfoWnd, NWS_VISIBLE);

    OKButton = new NCButton(TEXT("L2UI.control.defaultbutton"),
                            TEXT("L2UI.control.defaultbutton_click"), 0);
    OKButton->ID = NID_NOTICE_OK;
    OKButton->SetWindowRect(239, 400, 67, 19);
    CreateChild(OKButton, NWS_VISIBLE);
    OKButton->SetTextID(140);        // "OK"
    return 1;
}
NW_HOOK_METHOD(0x1009ad90, NCNoticeWnd, OnCreate)

// 0x1009afa0 (vslot 37)
INT NCNoticeWnd::OnDestroy()
{
    guard(NCNoticeWnd::OnDestroy);
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
NW_HOOK_METHOD(0x1009afa0, NCNoticeWnd, OnDestroy)

// 0x1009b070 (vslot 55) - under the 21-pixel title bar; the clip goes on before BeginPaint
INT NCNoticeWnd::OnPaint(NCGDevice* Device)
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
NW_HOOK_METHOD(0x1009b070, NCNoticeWnd, OnPaint)

// 0x1009b190 (vslot 58) - Enter does what OK does, with the click sound
INT NCNoticeWnd::OnKeyDown(WPARAM wParam, LPARAM lParam)
{
    if (wParam == VK_RETURN)
    {
        if (Type && (Type & 1))
            GConsole->ShowWarningWnd();
        else
            GConsole->RequestServerList();
        PlayInterfaceSound(SOUND_Click);
    }
    NCWnd::OnKeyDown(wParam, lParam);
    return 1;
}
NW_HOOK_METHOD(0x1009b190, NCNoticeWnd, OnKeyDown)

// 0x1009b150 (vslot 67)
INT NCNoticeWnd::OnOK()
{
    if (!Type)
        GConsole->RequestServerList();
    else if (Type & 1)
        GConsole->ShowWarningWnd();
    else
        GConsole->RequestServerList();
    return 1;
}
NW_HOOK_METHOD(0x1009b150, NCNoticeWnd, OnOK)

// 0x1009a820 (the fmap row calls it NCWarningWnd::SetText, whose twin it is: NCWarningWnd.cpp has 0x1009a8b0) -
// the text goes to the scroll window at +0xA0 unless it is empty
void NCNoticeWnd::SetText(const TCHAR* InText)
{
    if (InText && *InText)
        InfoWnd->SetText(InText);
}
NW_HOOK_METHOD(0x1009a820, NCNoticeWnd, SetText)

// 0x1009a840 (the fmap row calls it NCWarningWnd::ClearText, as above)
void NCNoticeWnd::ClearText()
{
    InfoWnd->ClearText();
}
NW_HOOK_METHOD(0x1009a840, NCNoticeWnd, ClearText)

NW_CHECK_VTABLE(NCNoticeWnd, 0x1011d5a8, 68)
