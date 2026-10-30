/*=============================================================================
    NCBoardWnd.cpp - the community board and its page viewer. Stock: 0x10026420 .. 0x10026488,
    0x10026490 .. 0x100264b0, 0x10026510 .. 0x10026630, 0x100266b0 .. 0x1002699d,
    0x10027460 .. 0x100276d3, 0x10027880 .. 0x10027ac6, 0x10027ad0 .. 0x10027b71,
    0x100296b0 .. 0x1002976f, 0x10029a40 .. 0x10029fbc, 0x1002a890 .. 0x1002a89a (the viewer's
    slot 72 and the board's ShowWindow are bodies the linker folded with other windows').
=============================================================================*/
#include "nwindow/NCBoardWnd.h"
#include "nwindow/NCButton.h"
#include "nwindow/NCFrameCtrl.h"
#include "nwindow/NCIcon.h"
#include "nwindow/NHtmlLayout.h"
#include "nwindow/NCGDevice.h"
#include "nwindow/NConsoleWnd.h"

#include <string.h>

enum
{
    NID_BOARD_FRAME = (INT)0xF0F01420,
    NID_BOARD_BUTTON1 = (INT)0xF0F01422,  // .. 0xF0F01427
    NID_BBS_HIDE = (INT)0xF0F01411,
};

enum { STR_BoardButton1 = 377 };  // .. 382
enum { SOUND_Click = 0 };

enum
{
    MSG_IncorrectName = 204,           // "Incorrect name. Please try again. "
    MSG_TitleTooLong = 328,            // "Please input title using less than 128 characters."
    MSG_TooLong3000 = 329,             // "Please input contents using less than 3000 characters."
};

/*-----------------------------------------------------------------------------
    NCBBSHtmlViewer.
-----------------------------------------------------------------------------*/

// 0x10033590
NCBBSHtmlViewer::NCBBSHtmlViewer()
{
    Unknown110 = 0;
    NumPostPieces = 0;
    NumFormPieces = 0;
    NumPageParts = 0;
    bLocked = 0;
}
NW_HOOK_CTOR(0x10033590, NCBBSHtmlViewer)

// 0x10033600
NW_HOOK_DTOR(0x10033600, NCBBSHtmlViewer)

// 0x1002fb00 (vslot 25) - the console's NWM_COMMAND that hides the viewer
INT NCBBSHtmlViewer::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (NCHtmlViewer::WindowProc(Msg, wParam, lParam))
        return 1;
    if (Msg == NWM_COMMAND && wParam == (WPARAM)NID_BBS_HIDE)
        return OnHideCommand() != 0;
    return 0;
}
NW_HOOK_METHOD(0x1002fb00, NCBBSHtmlViewer, WindowProc)

// 0x1002fb60 (vslot 36) - NCScrollWnd's, not NCHtmlViewer's (CreateWidth / CreateHeight stay 0)
INT NCBBSHtmlViewer::OnCreate()
{
    NCScrollWnd::OnCreate();
    BoardBackTex = FindTexture(TEXT("L2UI.bbs_back"));
    return 1;
}
NW_HOOK_METHOD(0x1002fb60, NCBBSHtmlViewer, OnCreate)

// 0x100348c0 (vslot 37) - guarded in C2
INT NCBBSHtmlViewer::OnDestroy()
{
    guard(NCBBSHtmlViewer::OnDestroy);
    NCHtmlViewer::OnDestroy();
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x100348c0, NCBBSHtmlViewer, OnDestroy)

// 0x10030e60 (vslot 43)
void NCBBSHtmlViewer::ShowWindow()
{
    NCHtmlViewer::ShowWindow();
}
NW_HOOK_METHOD(0x10030e60, NCBBSHtmlViewer, ShowWindow)

// 0x10030d60 (vslot 55) - NCHtmlViewer's, on the board's frame and clipped inside it
INT NCBBSHtmlViewer::OnPaint(NCGDevice* Device)
{
    Device->BeginPaint(ScreenX, ScreenY);
    Device->SetClip(ScreenX, ScreenY, Width, Height);
    Device->DrawTexture(0, 0, Width, Height, 0, 0, Width, Height, BackTex ? BackTex : BoardBackTex);
    Device->SetClip(ScreenX + 3, ScreenY + 10, Width - 6, Height - 20);
    Device->PushOrigin(ScrollX, ScrollY);
    if (Doc && bLoaded)
    {
        Doc->Draw(Device);
        if (bLoaded)
            SetContentHeight(Doc->GetContentHeight());
    }
    Device->PopOrigin();
    Device->EndPaint();
    Device->ReleaseClip();
    return 1;
}
NW_HOOK_METHOD(0x10030d60, NCBBSHtmlViewer, OnPaint)

// 0x10030e70 (vslot 70) - the value of the control whose caption is Name. A missing value says the
// title (329: the contents) is too long (sic); a one-line box may not hold the characters the
// console rejects (204); any value is 2..3000 bytes in the ANSI code page (C2: no bad word
// filter any more).
TCHAR* NCBBSHtmlViewer::GetControlValue(const TCHAR* Name)
{
    ANSICHAR Ansi[6000];
    if (!Name)
        return NULL;
    INT N = Controls.Num();
    if (N < 1)
        return NULL;
    INT i = 0;
    NControl* Control;
    for (;;)
    {
        Control = Controls(i);
        const TCHAR* Caption = Control->GetCaption();
        if (Caption && !wcscmp(Caption, Name))
            break;
        i++;
        if (N <= i)
            return NULL;
    }
    INT Message;
    if (!Control->GetValue())
        Message = Control->IsA(NClass::NCHtmlMultiEdit()) ? MSG_TooLong3000 : MSG_TitleTooLong;
    else
    {
        if (!Control->IsA(NClass::NCHtmlMultiEdit()) && GConsole->HasInvalidChars((TCHAR*)Control->GetValue()))
        {
            GConsole->ShowSystemMessage(MSG_IncorrectName, 0xFFB09B79);
            return NULL;
        }
        WideCharToMultiByte(CP_ACP, 0, Control->GetValue(), -1, Ansi, 6000, NULL, NULL);
        INT Length = (INT)strlen(Ansi);
        if (Length > 1 && Length < 3001)
            return (TCHAR*)Control->GetValue();
        Message = Control->IsA(NClass::NCHtmlMultiEdit()) ? MSG_TooLong3000 : MSG_TitleTooLong;
    }
    GConsole->ShowSystemMessage(Message, 0xFFB09B79);
    return NULL;
}
NW_HOOK_METHOD(0x10030e70, NCBBSHtmlViewer, GetControlValue)

// 0x1002fbe0 (vslot 72) - locked once the console accepts a command; the sound plays even for an
// empty link; a command also forgets the pieces of a page being received (C2)
INT NCBBSHtmlViewer::OnActionLink(TCHAR* Action)
{
    if (!bLocked)
    {
        NumPostPieces = 0;
        NumFormPieces = 0;
        NumPageParts = 0;
        wcscpy(GHtmlLinkTarget, Action);
        if (GHtmlLinkTarget[0])
        {
            INT Result = GConsole->RunHtmlCommand(GHtmlLinkTarget, this);
            if (Result == 2)
            {
                if (Owner)
                    Owner->Wnd->WindowProc(NWM_HTML_CLOSE, 0, 0);
                else
                    HideWindow();
            }
            else if (Result == 1)
                bLocked = 1;
        }
        PlayInterfaceSound(SOUND_Click);
    }
    return 1;
}
NW_HOOK_METHOD(0x1002fbe0, NCBBSHtmlViewer, OnActionLink)

// 0x1002fc90 (vslot 73) - the same for a button (no emptiness test, no sound)
INT NCBBSHtmlViewer::OnButton(TCHAR* Action)
{
    if (!bLocked)
    {
        NumPostPieces = 0;
        NumFormPieces = 0;
        NumPageParts = 0;
        wcscpy(GHtmlLinkTarget, Action);
        INT Result = GConsole->RunHtmlCommand(GHtmlLinkTarget, this);
        if (Result == 2)
        {
            if (Owner)
            {
                Owner->Wnd->WindowProc(NWM_HTML_CLOSE, 0, 0);
                return 1;
            }
            HideWindow();
            return 1;
        }
        if (Result == 1)
            bLocked = 1;
    }
    return 1;
}
NW_HOOK_METHOD(0x1002fc90, NCBBSHtmlViewer, OnButton)

// stock body shared with NCOptionWnd::OnCancel (0x10059d00)
INT NCBBSHtmlViewer::OnHideCommand()
{
    HideWindow();
    return 1;
}

NW_CHECK_VTABLE(NCBBSHtmlViewer, 0x101103d8, 78)

/*-----------------------------------------------------------------------------
    NCBoardWnd.
-----------------------------------------------------------------------------*/

// 0x10031270
NCBoardWnd::NCBoardWnd()
{
    bLocked = 0;
}
NW_HOOK_CTOR(0x10031270, NCBoardWnd)

// 0x100312c0
NW_HOOK_DTOR(0x100312c0, NCBoardWnd)

// 0x1002fe10 (vslot 25) - the six buttons; C2 does not test IsEnabled first
INT NCBoardWnd::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (NCWnd::WindowProc(Msg, wParam, lParam))
        return 1;
    if (Msg == NWM_COMMAND)
    {
        if (wParam == (WPARAM)NID_BOARD_BUTTON1)
            return OnButton1() != 0;
        if (wParam == (WPARAM)(NID_BOARD_BUTTON1 + 1))
            return OnButton2() != 0;
        if (wParam == (WPARAM)(NID_BOARD_BUTTON1 + 2))
            return OnButton3() != 0;
        if (wParam == (WPARAM)(NID_BOARD_BUTTON1 + 3))
            return OnButton4() != 0;
        if (wParam == (WPARAM)(NID_BOARD_BUTTON1 + 4))
            return OnButton5() != 0;
        if (wParam == (WPARAM)(NID_BOARD_BUTTON1 + 5))
            return OnButton6() != 0;
    }
    return 0;
}
NW_HOOK_METHOD(0x1002fe10, NCBoardWnd, WindowProc)

// the buttons' command, sent while the board is not locked; an accepted one locks it
static FORCEINLINE INT RunBoardCommand(NCBoardWnd* Board, TCHAR* Command)
{
    if (Command[0] && !Board->bLocked)
    {
        if (GConsole->RunHtmlCommand(Command, Board->HtmlViewer) == 1)
            Board->bLocked = 1;
    }
    return 1;
}

// 0x1002ff00 (vslot 67)
INT NCBoardWnd::OnButton1()
{
    return RunBoardCommand(this, Commands[0]);
}
NW_HOOK_METHOD(0x1002ff00, NCBoardWnd, OnButton1)

// 0x1002ff50 (vslot 68)
INT NCBoardWnd::OnButton2()
{
    return RunBoardCommand(this, Commands[1]);
}
NW_HOOK_METHOD(0x1002ff50, NCBoardWnd, OnButton2)

// 0x1002ffa0 (vslot 69)
INT NCBoardWnd::OnButton3()
{
    return RunBoardCommand(this, Commands[2]);
}
NW_HOOK_METHOD(0x1002ffa0, NCBoardWnd, OnButton3)

// 0x1002fff0 (vslot 70) - not stopped by the lock; an accepted command disables this button
INT NCBoardWnd::OnButton4()
{
    if (Commands[3][0])
    {
        if (GConsole->RunHtmlCommand(Commands[3], HtmlViewer) == 1)
            Buttons[3]->EnableWindow(0);
    }
    return 1;
}
NW_HOOK_METHOD(0x1002fff0, NCBoardWnd, OnButton4)

// 0x10030040 (vslot 71)
INT NCBoardWnd::OnButton5()
{
    return RunBoardCommand(this, Commands[4]);
}
NW_HOOK_METHOD(0x10030040, NCBoardWnd, OnButton5)

// 0x10030090 (vslot 72)
INT NCBoardWnd::OnButton6()
{
    return RunBoardCommand(this, Commands[5]);
}
NW_HOOK_METHOD(0x10030090, NCBoardWnd, OnButton6)

// one of the six buttons above the page (repeated inline in the stock OnCreate)
static FORCEINLINE void CreateBoardButton(NCBoardWnd* Board, INT Index, INT X)
{
    Board->Buttons[Index] = new NCButton(TEXT("sek.cbui91"), TEXT("sek.cbui93"), 0);
    Board->Buttons[Index]->ID = NID_BOARD_BUTTON1 + Index;
    Board->Buttons[Index]->SetWindowRect(X, 24, 50, 16);
    Board->CreateChild(Board->Buttons[Index], NWS_VISIBLE);
}

// 0x10033920 (vslot 36) - title bar, page, then the buttons above it. C2 moves the second button to the right end
// (x 310; the others follow the first from x 70) and gives it text 403 instead of 378
INT NCBoardWnd::OnCreate()
{
    NCWnd::OnCreate();
    FrameCtrl = new NCFrameCtrl(1);
    FrameID = NID_BOARD_FRAME;
    FrameCtrl->ID = NID_BOARD_FRAME;
    FrameCtrl->SetWindowRect(0, 0, 646, 21);
    CreateChild(FrameCtrl, NWS_VISIBLE);

    BackTex = FindTexture(TEXT("sek.cbui354"));

    HtmlViewer = new NCBBSHtmlViewer;
    HtmlViewer->SetWindowRect(0, 39, 646, 486);
    CreateChild(HtmlViewer, NWS_VISIBLE);
    HtmlViewer->SetOwner(Entry);

    CreateBoardButton(this, 0, 10);
    CreateBoardButton(this, 1, 310);
    CreateBoardButton(this, 2, 70);
    CreateBoardButton(this, 3, 130);
    CreateBoardButton(this, 4, 190);
    CreateBoardButton(this, 5, 250);
    Buttons[0]->SetTextID(STR_BoardButton1);
    Buttons[1]->SetTextID(403);
    Buttons[2]->SetTextID(379);
    Buttons[3]->SetTextID(380);
    Buttons[4]->SetTextID(381);
    Buttons[5]->SetTextID(382);
    return 1;
}
NW_HOOK_METHOD(0x10033920, NCBoardWnd, OnCreate)

// 0x10031310 (vslot 37) - the viewer goes, the buttons are enabled again (all six, without asking whether they are
// there) and go; guarded in C2 (the stock frame names NCBoardWnd::OnDestroy and holds the whole body)
INT NCBoardWnd::OnDestroy()
{
    guard(NCBoardWnd::OnDestroy);
    if (HtmlViewer)
    {
        RemoveChildWindow(HtmlViewer->Entry);
        if (HtmlViewer)
            delete HtmlViewer;
        HtmlViewer = NULL;
    }
    for (INT i = 0; i < 6; i++)
        Buttons[i]->EnableWindow(1);
    for (INT i = 0; i < 6; i++)
    {
        if (Buttons[i])
        {
            RemoveChildWindow(Buttons[i]->Entry);
            if (Buttons[i])
                delete Buttons[i];
            Buttons[i] = NULL;
        }
    }
    NCWnd::OnDestroy();
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x10031310, NCBoardWnd, OnDestroy)

// stock body shared with NCSystemMenuWnd's (0x10078950): shown again from its minimized icon
void NCBoardWnd::ShowWindow()
{
    NCWnd::ShowWindow();
    if (Icon)
    {
        SetWindowRect(Icon->ScreenX, Icon->ScreenY, Width, Height);
        Icon->SetWindowRect(0, 0, 0, 0);
        Icon->HideWindow();
    }
}

// 0x10031520 (vslot 55) - the strip under the title bar; the guard names the NPC dialog, as
// in the stock code
INT NCBoardWnd::OnPaint(NCGDevice* Device)
{
    guard(NCNPCHtmlViewer::OnPaint);
    Device->BeginPaint(ScreenX, ScreenY);
    Device->SetClip(ScreenX, ScreenY, Width, Height);
    if (BackTex)
        Device->DrawTexture(0, 21, 646, 18, 0, 0, 646, 18, BackTex);
    Device->EndPaint();
    Device->ReleaseClip();
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x10031520, NCBoardWnd, OnPaint)

NW_CHECK_VTABLE(NCBoardWnd, 0x1010fe48, 73)
