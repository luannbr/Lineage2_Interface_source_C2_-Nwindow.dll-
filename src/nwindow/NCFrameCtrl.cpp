/*=============================================================================
    NCFrameCtrl.cpp - title bar and resize edge. Stock (C2): 0x1000e700 .. 0x1000f290.
=============================================================================*/
#include "nwindow/NCFrameCtrl.h"
#include "nwindow/NCButton.h"
#include "nwindow/NCGDevice.h"
#include "nwindow/NCPrivateBuyWnd.h"
#include "nwindow/NCPrivateShopWnd.h"
#include "nwindow/NConsoleWnd.h"
#include "nwindow/NClasses.gen.h"

#include <wchar.h>

// interface sound played when a window is closed from its title bar
enum { SOUND_WindowClose = 6 };

/*-----------------------------------------------------------------------------
    NCFrameCtrl.
-----------------------------------------------------------------------------*/

// 0x1000ea80
NCFrameCtrl::NCFrameCtrl()
{
    CloseButton = NULL;
    MinimizeButton = NULL;
    LastMouseY = 0;
    LastMouseX = 0;
    TitleY = 0;
    TitleX = 0;
    GrabY = 0;
    GrabX = 0;
    BlackTex = FindTexture(TEXT("NWindow.BlackTexture"));
    TitleLeftTex = FindTexture(TEXT("L2UI.NWindow.WindowTitlebar1"));
    TitleMidTex = FindTexture(TEXT("L2UI.NWindow.WindowTitlebar2"));
    TitleRightTex = FindTexture(TEXT("L2UI.NWindow.WindowTitlebar3"));
    bDecorated = 1;
}
NW_HOOK_CTOR(0x1000ea80, NCFrameCtrl)

// 0x1000eb80
NCFrameCtrl::NCFrameCtrl(UBOOL bInDecorated)
{
    CloseButton = NULL;
    MinimizeButton = NULL;
    LastMouseY = 0;
    LastMouseX = 0;
    TitleY = 0;
    TitleX = 0;
    GrabY = 0;
    GrabX = 0;
    BlackTex = FindTexture(TEXT("NWindow.BlackTexture"));
    TitleLeftTex = FindTexture(TEXT("L2UI.NWindow.WindowTitlebar1"));
    TitleMidTex = FindTexture(TEXT("L2UI.NWindow.WindowTitlebar2"));
    TitleRightTex = FindTexture(TEXT("L2UI.NWindow.WindowTitlebar3"));
    bDecorated = bInDecorated;
}
NW_HOOK_CTOR_ARGS(0x1000eb80, NCFrameCtrl, (UBOOL A), (A))

// 0x1000e9e0 - ConditionalDestroy, ~FString (Title), ~NControl
NW_HOOK_DTOR(0x1000e9e0, NCFrameCtrl)

// 0x1000e700 (vslot 25) - the frame buttons report here. C2: no enabled test
INT NCFrameCtrl::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (NControl::WindowProc(Msg, wParam, lParam))
        return 1;
    if (Msg == NWM_COMMAND)
    {
        if (wParam == NID_FRAME_MINIMIZE)
            return OnMinimize() != 0;
        if (wParam == NID_FRAME_CLOSE)
            return OnClose() != 0;
    }
    return 0;
}
NW_HOOK_METHOD(0x1000e700, NCFrameCtrl, WindowProc)

// 0x1000ed90 (vslot 36) - the buttons of a decorated frame, right-aligned
INT NCFrameCtrl::OnCreate()
{
    if (bDecorated)
    {
        if (Entry->Parent->Wnd->GetStyle() & NWS_MINIMIZABLE)
        {
            MinimizeButton = new NCButton(TEXT("L2UI.NWindow.WindowMinimize"),
                                          TEXT("L2UI.NWindow.WindowMinimize_click"), 0);
            MinimizeButton->ID = NID_FRAME_MINIMIZE;
            MinimizeButton->SetWindowRect(Width - 45, 4, 16, 11);
            CreateChild(MinimizeButton, NWS_VISIBLE);
        }
        if (!(Entry->Parent->Wnd->GetStyle() & NWS_NOCLOSE))
        {
            CloseButton = new NCButton(TEXT("L2UI.NWindow.WindowClose"),
                                       TEXT("L2UI.NWindow.WindowClose_click"), 0);
            CloseButton->ID = NID_FRAME_CLOSE;
            CloseButton->SetWindowRect(Width - 24, 4, 16, 14);
            CreateChild(CloseButton, NWS_VISIBLE);
        }
    }
    return 1;
}
NW_HOOK_METHOD(0x1000ed90, NCFrameCtrl, OnCreate)

// 0x1000ef40 (vslot 37) - C2 guards it
INT NCFrameCtrl::OnDestroy()
{
    guard(NCFrameCtrl::OnDestroy);
    if (MinimizeButton)
    {
        RemoveChildWindow(MinimizeButton->Entry);
        delete MinimizeButton;
        MinimizeButton = NULL;
    }
    if (CloseButton)
    {
        RemoveChildWindow(CloseButton->Entry);
        delete CloseButton;
        CloseButton = NULL;
    }
    NControl::OnDestroy();
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x1000ef40, NCFrameCtrl, OnDestroy)

// 0x1000e7a0 (vslot 42) - a bare jump to NCWnd's
void NCFrameCtrl::SetSize(INT W, INT H)
{
    NCWnd::SetSize(W, H);
}
NW_HOOK_METHOD(0x1000e7a0, NCFrameCtrl, SetSize)

// 0x1000f010 (vslot 55) - left cap, right cap, stretched middle, title
INT NCFrameCtrl::OnPaint(NCGDevice* Device)
{
    guard(NCFrameCtrl::OnPaint);
    Device->BeginPaint(ScreenX, ScreenY);
    Device->SetClip(ScreenX, ScreenY, Width, Height);
    if (bDecorated)
    {
        if (TitleLeftTex)
            Device->DrawTexture(0, 0, 24, 21, 0, 0, 24, 21, TitleLeftTex);
        if (TitleRightTex)
            Device->DrawTexture(Width - 8, 0, 8, 21, 0, 0, 8, 21, TitleRightTex);
        if (TitleMidTex)
            Device->DrawStretchedTex(24, 0, Width - 32, 21, 0, 0, 32, 21, TitleMidTex);
        if (Title.Num())
            Device->DrawText(TitleX, TitleY, 0xFFFFFFFF, *Title, 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0,
                             0, 0, 0);
    }
    Device->ReleaseClip();
    Device->EndPaint();
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x1000f010, NCFrameCtrl, OnPaint)

// 0x1000e7b0 (vslot 60) - the move is sent as a delta; LastMouse holds the delta during the call
INT NCFrameCtrl::OnMouseMove(WPARAM wParam, LPARAM lParam)
{
    if (wParam & MK_LBUTTON)
    {
        INT X = (SWORD)LOWORD(lParam);
        INT Y = (SWORD)HIWORD(lParam);
        LastMouseX = X - LastMouseX;
        LastMouseY = Y - LastMouseY;
        SendToParent(WM_MOVE, 0, MAKELPARAM(LastMouseX, LastMouseY));
        LastMouseX = X;
        LastMouseY = Y;
    }
    return 1;
}
NW_HOOK_METHOD(0x1000e7b0, NCFrameCtrl, OnMouseMove)

// 0x1000e820 (vslot 61) - the parent gets the click too (it comes to the front, takes focus)
INT NCFrameCtrl::OnLButtonDown(WPARAM wParam, LPARAM lParam)
{
    LockCursor();
    LastMouseX = (SWORD)LOWORD(lParam);
    GrabX = LastMouseX - ScreenX;
    LastMouseY = (SWORD)HIWORD(lParam);
    GrabY = LastMouseY - ScreenY;
    SendToParent(WM_LBUTTONDOWN, wParam, lParam);
    return 1;
}
NW_HOOK_METHOD(0x1000e820, NCFrameCtrl, OnLButtonDown)

// 0x1000e880 (vslot 62)
INT NCFrameCtrl::OnLButtonUp(WPARAM wParam, LPARAM lParam)
{
    UnlockCursor();
    ReleaseFocus();
    return 1;
}
NW_HOOK_METHOD(0x1000e880, NCFrameCtrl, OnLButtonUp)

// 0x1000ec80 (vslot 68) - closing some windows also tells the server. C2 adds the recipe shop
INT NCFrameCtrl::OnClose()
{
    NCWnd* Parent = Entry->Parent->Wnd;
    if (Parent->IsA(NClass::NCPrivateShopWnd()))
    {
        if (static_cast<NCPrivateShopWnd*>(Parent)->Mode == 1)
            GConsole->RequestPrivateStoreQuitSell();
        static_cast<NCPrivateShopWnd*>(Entry->Parent->Wnd)->Reset();
    }
    else if (Parent->IsA(NClass::NCPrivateBuyWnd()))
    {
        if (static_cast<NCPrivateBuyWnd*>(Parent)->Mode == 1)
            GConsole->RequestPrivateStoreQuitBuy();
        // the stock code calls the sell window's body (one for both)
        reinterpret_cast<NCPrivateShopWnd*>(Entry->Parent->Wnd)->Reset();
    }
    else if (Parent->IsA(NClass::NCRecipeShopWnd()))
        GConsole->RequestRecipeShopManageQuit();   // 0x10054460, C2: the console's request to leave the recipe shop
                                                     //   (Engine's NetHandler vslot 206)
    else if (Parent->IsA(NClass::NCTradeWnd()))
        GConsole->RequestTradeDone(0);
    else if (Parent->IsA(NClass::NCMatchWnd()))
        GConsole->RequestPartyMatchList(3);              // leaves the room
    HideParent();
    PlayInterfaceSound(SOUND_WindowClose);
    return 1;
}
NW_HOOK_METHOD(0x1000ec80, NCFrameCtrl, OnClose)

// 0x1000e780 (vslot 69)
INT NCFrameCtrl::OnMinimize()
{
    Entry->Parent->Wnd->Minimize();
    return 1;
}
NW_HOOK_METHOD(0x1000e780, NCFrameCtrl, OnMinimize)

// 0x1000e8a0 - called by NCWnd::SetTitle (C1: titles of 256 characters or more were ignored)
void NCFrameCtrl::SetTitleText(const TCHAR* InTitle)
{
    if (InTitle)
    {
        Title = InTitle;
        SIZE Size;
        NCGDevice::GetTextExtent(&Size, *Title, 0, 0, 0, 0, 0, L2FONT_Normal);
        TitleX = 28;
        TitleY = ((DWORD)Height >> 1) - Size.cy / 2 + 2;
    }
}
NW_HOOK_METHOD(0x1000e8a0, NCFrameCtrl, SetTitleText)

NW_CHECK_VTABLE(NCFrameCtrl, 0x1010a568, 70)

/*-----------------------------------------------------------------------------
    NCResizeFrame.
-----------------------------------------------------------------------------*/

// 0x1000f170 - undecorated; InternalConstructor passes 0
NCResizeFrame::NCResizeFrame(INT InDirection)
{
    Direction = InDirection;
    bDecorated = 0;
    Unknown3DC = 0;
    bCursorLocked = 0;
    bDragging = 0;
}
NW_HOOK_CTOR_ARGS(0x1000f170, NCResizeFrame, (INT A), (A))

// 0x1000f1e0
NW_HOOK_DTOR(0x1000f1e0, NCResizeFrame)

// 0x1000e910 (vslot 25) - C2: NCFrameCtrl's alone (no enabled test)
INT NCResizeFrame::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    return NCFrameCtrl::WindowProc(Msg, wParam, lParam) != 0;
}
NW_HOOK_METHOD(0x1000e910, NCResizeFrame, WindowProc)

// 0x1000e9c0 (vslot 30)
void NCResizeFrame::KillFocus()
{
    NCWnd::KillFocus();
    bDragging = 0;
    bPressed = 0;
}
NW_HOOK_METHOD(0x1000e9c0, NCResizeFrame, KillFocus)

// 0x1000f230 (vslot 60) - resize cursor while over the edge; while dragging, the delta along
// the resize direction goes to the parent as NWM_COMMAND(ID, delta)
INT NCResizeFrame::OnMouseMove(WPARAM wParam, LPARAM lParam)
{
    INT X = (SWORD)LOWORD(lParam);
    INT Y = (SWORD)HIWORD(lParam);
    if (PtInRect(X, Y, ScreenX, ScreenY, Width, Height))
    {
        if (Direction == RESIZE_Vertical)
            SetCursor(5, NULL);
        else if (Direction == RESIZE_Horizontal)
            SetCursor(6, NULL);
        LockCursor();
        bCursorLocked = 1;
        if (!(wParam & MK_LBUTTON) || !bDragging)
            return 1;
    }
    else if (!bDragging || !(wParam & MK_LBUTTON))
    {
        if (bCursorLocked)
        {
            bCursorLocked = 0;
            UnlockCursor();
        }
        return 1;
    }
    LastMouseX = X - LastMouseX;
    LastMouseY = Y - LastMouseY;
    if (Direction == RESIZE_Vertical)
        NotifyParent(LastMouseY);
    else if (Direction == RESIZE_Horizontal)
        NotifyParent(LastMouseX);
    LastMouseX = X;
    LastMouseY = Y;
    return 1;
}
NW_HOOK_METHOD(0x1000f230, NCResizeFrame, OnMouseMove)

// 0x1000e930 (vslot 61)
INT NCResizeFrame::OnLButtonDown(WPARAM wParam, LPARAM lParam)
{
    INT X = (SWORD)LOWORD(lParam);
    INT Y = (SWORD)HIWORD(lParam);
    LastMouseX = X;
    LastMouseY = Y;
    if (PtInRect(X, Y, ScreenX, ScreenY, Width, Height))
    {
        bDragging = 1;
        bPressed = 1;
    }
    return 1;
}
NW_HOOK_METHOD(0x1000e930, NCResizeFrame, OnLButtonDown)

// 0x1000e9a0 (vslot 62)
INT NCResizeFrame::OnLButtonUp(WPARAM wParam, LPARAM lParam)
{
    bPressed = 0;
    bDragging = 0;
    return 1;
}
NW_HOOK_METHOD(0x1000e9a0, NCResizeFrame, OnLButtonUp)

NW_CHECK_VTABLE(NCResizeFrame, 0x1010a880, 70)
