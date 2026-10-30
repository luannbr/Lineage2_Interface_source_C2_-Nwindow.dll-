/*=============================================================================
    NCTutorialViewerWnd.cpp - the tutorial window and its question mark. Stock: 0x100bcdb0 ..
    0x100bd0dc, 0x100bd120 .. 0x100bd339 (WindowProc, OnClose and OnDestroy are bodies the
    linker folded: NCNPCHtmlViewer hooks the first two).
=============================================================================*/
#include "nwindow/NCTutorialViewerWnd.h"
#include "nwindow/NCHtmlViewer.h"
#include "nwindow/NCFrameCtrl.h"
#include "nwindow/NCGDevice.h"
#include "nwindow/NConsoleWnd.h"

// title bar id, blink timer (names ours)
enum
{
    NID_TUTORIAL_FRAME = (INT)0xFFF00001,
    TIMER_TutorialBlink = 0xFA0D06,
};

/*-----------------------------------------------------------------------------
    NCTutorialViewerWnd.
-----------------------------------------------------------------------------*/

// 0x100de720
NCTutorialViewerWnd::NCTutorialViewerWnd()
{
    Unknown98 = 0;
    BackTex = NULL;
}
NW_HOOK_CTOR(0x100de720, NCTutorialViewerWnd)

// 0x100de770
NW_HOOK_DTOR(0x100de770, NCTutorialViewerWnd)

// 0x1002fd40 (vslot 25) - one body with NCNPCHtmlViewer's (hooked there); C2: no enabled test
INT NCTutorialViewerWnd::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (NCWnd::WindowProc(Msg, wParam, lParam))
        return 1;
    if (Msg == (UINT)NWM_HTML_CLOSE)
        return OnClose(wParam, lParam) != 0;
    return 0;
}

// 0x100de7c0 (vslot 36) - like the NPC dialog, with a 300 px title bar and a 503 px viewer, on its own
// background (C2)
INT NCTutorialViewerWnd::OnCreate()
{
    NCWnd::OnCreate();
    INT W = LocalW;

    FrameCtrl = new NCFrameCtrl(1);
    FrameID = NID_TUTORIAL_FRAME;
    FrameCtrl->ID = NID_TUTORIAL_FRAME;
    FrameCtrl->SetWindowRect(0, 0, 300, 21);
    CreateChild(FrameCtrl, NWS_VISIBLE);

    BackTex = FindTexture(TEXT("L2UI.TutorialWnd.TutorialBack"));

    HtmlViewer = new NCTutorialHtmlViewer;
    HtmlViewer->SetWindowRect(12, 28, W - 12, 503);
    CreateChild(HtmlViewer, NWS_VISIBLE);
    HtmlViewer->SetOwner(Entry);
    return 1;
}
NW_HOOK_METHOD(0x100de7c0, NCTutorialViewerWnd, OnCreate)

// 0x100de930 (vslot 37) - guarded in C2
INT NCTutorialViewerWnd::OnDestroy()
{
    guard(NCTutorialViewerWnd::OnDestroy);
    if (HtmlViewer)
    {
        RemoveChildWindow(HtmlViewer->Entry);
        delete HtmlViewer;
        HtmlViewer = NULL;
    }
    NCWnd::OnDestroy();
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x100de930, NCTutorialViewerWnd, OnDestroy)

// 0x100de9e0 (vslot 55) - the background starts below the title bar; the guard carries
// NCNPCHtmlViewer's name in the stock code
INT NCTutorialViewerWnd::OnPaint(NCGDevice* Device)
{
    guard(NCNPCHtmlViewer::OnPaint);
    Device->BeginPaint(ScreenX, ScreenY);
    Device->SetClip(ScreenX, ScreenY, Width, Height);
    if (BackTex)
        Device->DrawTexture(0, 21, 300, 512, 0, 0, 300, 512, BackTex);
    Device->EndPaint();
    Device->ReleaseClip();
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x100de9e0, NCTutorialViewerWnd, OnPaint)

// 0x1002fda0 (vslot 67) - the stock body is NCNPCHtmlViewer's (hooked there): a page command closed the dialog
INT NCTutorialViewerWnd::OnClose(WPARAM wParam, LPARAM lParam)
{
    HideWindow();
    return 1;
}

// 0x100de3d0 - an empty text leaves the page as it is
void NCTutorialViewerWnd::ShowPage(TCHAR* Text)
{
    if (Text && *Text)
        HtmlViewer->LoadHtml(Text);
}
NW_HOOK_METHOD(0x100de3d0, NCTutorialViewerWnd, ShowPage)

NW_CHECK_VTABLE(NCTutorialViewerWnd, 0x10128a10, 68)

/*-----------------------------------------------------------------------------
    NCTutorialQuestionWnd.
-----------------------------------------------------------------------------*/

// 0x100deac0 - QuestionID is set by whoever shows it
NCTutorialQuestionWnd::NCTutorialQuestionWnd()
{
    HelpTex = NULL;
    HelpClickTex = NULL;
    bLit = 0;
}
NW_HOOK_CTOR(0x100deac0, NCTutorialQuestionWnd)

// 0x100deb20
NW_HOOK_DTOR(0x100deb20, NCTutorialQuestionWnd)

// stock body shared with NControl's (0x10004e90)
INT NCTutorialQuestionWnd::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (Msg != WM_PAINT && !IsEnabled())
        return 0;
    return NCWnd::WindowProc(Msg, wParam, lParam) != 0;
}

// 0x100de4c0 (vslot 35) - blinks
INT NCTutorialQuestionWnd::OnTimer(INT InTimerID)
{
    if (InTimerID == TIMER_TutorialBlink)
        bLit = !bLit;
    return 1;
}
NW_HOOK_METHOD(0x100de4c0, NCTutorialQuestionWnd, OnTimer)

// 0x100de3f0 (vslot 36)
INT NCTutorialQuestionWnd::OnCreate()
{
    NCWnd::OnCreate();
    HelpTex = FindTexture(TEXT("L2UI.etcWnd.TutorialHelp"));
    HelpClickTex = FindTexture(TEXT("L2UI.etcWnd.TutorialHelp_click"));
    TimerID = SetTimer(TIMER_TutorialBlink, 500, 0);
    return 1;
}
NW_HOOK_METHOD(0x100de3f0, NCTutorialQuestionWnd, OnCreate)

// 0x100deb70 (vslot 55) - no guard; the clip goes on after the origin
INT NCTutorialQuestionWnd::OnPaint(NCGDevice* Device)
{
    Device->BeginPaint(ScreenX, ScreenY);
    Device->SetClip(ScreenX, ScreenY, Width, Height);
    Device->DrawTexture(0, 0, Width, Height, 0, 0, Width, Height, bLit ? HelpTex : HelpClickTex);
    Device->EndPaint();
    Device->ReleaseClip();
    return 1;
}
NW_HOOK_METHOD(0x100deb70, NCTutorialQuestionWnd, OnPaint)

// 0x100de440 (vslot 61)
INT NCTutorialQuestionWnd::OnLButtonDown(WPARAM wParam, LPARAM lParam)
{
    if (IsInWindow(LOWORD(lParam), HIWORD(lParam)))
        bLit = 0;
    return 1;
}
NW_HOOK_METHOD(0x100de440, NCTutorialQuestionWnd, OnLButtonDown)

// 0x100de470 (vslot 62) - a release on it asks for the page and hides it
INT NCTutorialQuestionWnd::OnLButtonUp(WPARAM wParam, LPARAM lParam)
{
    if (IsInWindow(LOWORD(lParam), HIWORD(lParam)))
    {
        GConsole->RequestTutorialQuestionMark(QuestionID);
        HideWindow();
    }
    bLit = 1;
    return 1;
}
NW_HOOK_METHOD(0x100de470, NCTutorialQuestionWnd, OnLButtonUp)

NW_CHECK_VTABLE(NCTutorialQuestionWnd, 0x10128bd0, 67)
