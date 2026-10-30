/*=============================================================================
    NCObserverWnd.cpp - the observer mode box (C2 only). Stock: 0x100991b0 .. 0x10099205,
    0x10099210 .. 0x10099252, 0x10099290 .. 0x100992f4 and 0x10099b70 .. 0x10099e1f.
=============================================================================*/
#include "nwindow/NCObserverWnd.h"
#include "nwindow/NCButton.h"
#include "nwindow/NCFrameCtrl.h"
#include "nwindow/NCGDevice.h"
#include "nwindow/NConsoleWnd.h"

// ids (names ours)
enum
{
    NID_OBSERVER_EXIT = 0x1012,
    NID_OBSERVER_FRAME = 0xB000F0,
};
enum { SYSSTR_ObserverExit = 611 };   // the button's text (name ours)

// 0x10099b70 - the button comes with OnCreate
NCObserverWnd::NCObserverWnd()
{
    BackTex = NULL;
    ExitButton = NULL;
}
NW_HOOK_CTOR(0x10099b70, NCObserverWnd)

// 0x10099bc0
NW_HOOK_DTOR(0x10099bc0, NCObserverWnd)

// 0x100991b0 (vslot 25)
INT NCObserverWnd::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (NCWnd::WindowProc(Msg, wParam, lParam))
        return 1;
    if (Msg == NWM_COMMAND && wParam == NID_OBSERVER_EXIT)
        return OnExit() != 0;
    return 0;
}
NW_HOOK_METHOD(0x100991b0, NCObserverWnd, WindowProc)

// 0x10099290 (vslot 67) - the server is asked to end the observation (no arguments)
INT NCObserverWnd::OnExit()
{
    L2ParamStack Params(10);
    GConsole->RequestObserverEndPacket(Params);
    return 1;
}
NW_HOOK_METHOD(0x10099290, NCObserverWnd, OnExit)

// 0x10099210 (vslot 37) - guarded in C2; nothing of its own to free
INT NCObserverWnd::OnDestroy()
{
    guard(NCObserverWnd::OnDestroy);
    NCWnd::OnDestroy();
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x10099210, NCObserverWnd, OnDestroy)

// 0x10099c90 (vslot 36) - a title bar of the window's width and the button under the middle of the panel
INT NCObserverWnd::OnCreate()
{
    NCWnd::OnCreate();
    DWORD H = LocalH;
    DWORD W = LocalW;

    FrameCtrl = new NCFrameCtrl(1);
    FrameCtrl->ID = NID_OBSERVER_FRAME;
    FrameCtrl->SetWindowRect(0, 0, W, 21);
    CreateChild(FrameCtrl, NWS_VISIBLE);

    BackTex = FindTexture(TEXT("L2UI.ObserverWnd.ObserverWndBack"));

    ExitButton = new NCButton(TEXT("L2UI.control.DefaultButton"), TEXT("L2UI.control.DefaultButton_click"), 0);
    ExitButton->ID = NID_OBSERVER_EXIT;
    ExitButton->SetWindowRect((W >> 1) - 32, (H >> 1) + 1, 64, 19);
    CreateChild(ExitButton, NWS_VISIBLE);
    ExitButton->SetText(*SysString(SYSSTR_ObserverExit));
    return 1;
}
NW_HOOK_METHOD(0x10099c90, NCObserverWnd, OnCreate)

// 0x10099c10 (vslot 55) - the panel (120 x 34) under the title bar; the clip goes on before the origin
INT NCObserverWnd::OnPaint(NCGDevice* Device)
{
    Device->SetClip(ScreenX, ScreenY, Width, Height);
    Device->BeginPaint(ScreenX, ScreenY);
    if (BackTex)
        Device->DrawTexture(0, 21, 120, 34, 0, 0, 120, 34, BackTex);
    Device->EndPaint();
    Device->ReleaseClip();
    return 1;
}
NW_HOOK_METHOD(0x10099c10, NCObserverWnd, OnPaint)

NW_CHECK_VTABLE(NCObserverWnd, 0x1011d168, 68)
