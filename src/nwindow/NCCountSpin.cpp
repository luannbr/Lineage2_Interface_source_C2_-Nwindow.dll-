/*=============================================================================
    NCCountSpin.cpp - number with up / down buttons. Stock: 0x100050a0 .. 0x100051e9,
    0x100052b0 .. 0x1000572e (with the destructor).
=============================================================================*/
#include "nwindow/NCCountSpin.h"
#include "nwindow/NCButton.h"
#include "nwindow/NCGDevice.h"

#include <stdlib.h>
#include <wchar.h>

// button ids (names ours)
enum
{
    NID_COUNTSPIN_UP = 0x100A,
    NID_COUNTSPIN_DOWN = 0x100B,
};

// the code sent to the parent with each step (NWM_COMMAND)
enum
{
    COUNTSPIN_Up = 1,
    COUNTSPIN_Down = 2,
};

// 0x10005c00 - NControl's constructor is inlined in the stock code
NCCountSpin::NCCountSpin(INT InMin, INT InMax, const TCHAR* InLabel)
{
    TextX = 5;
    TextY = 5;
    BackTex = NULL;
    UpButton = NULL;
    DownButton = NULL;
    bCancelStep = 0;
    Min = InMin;
    Max = InMax;
    Value = InMin;
    _itow(InMin, ValueText, 10);
    Label = InLabel ? InLabel : TEXT("Default");
}
NW_HOOK_CTOR_ARGS(0x10005c00, NCCountSpin, (INT A, INT B, const TCHAR* C), (A, B, C))

// 0x10005d20
NW_HOOK_DTOR(0x10005d20, NCCountSpin)

// 0x10005a40 (vslot 25) - NControl's, then the buttons' clicks; C2: no enabled test
INT NCCountSpin::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (NControl::WindowProc(Msg, wParam, lParam))
        return 1;
    if (Msg == NWM_COMMAND)
    {
        if (wParam == NID_COUNTSPIN_UP)
            return OnUp() != 0;
        if (wParam == NID_COUNTSPIN_DOWN)
            return OnDown() != 0;
    }
    return 0;
}
NW_HOOK_METHOD(0x10005a40, NCCountSpin, WindowProc)

// 0x10005ac0 (vslot 68)
INT NCCountSpin::OnUp()
{
    if (Value < Max)
    {
        Value++;
        NotifyParent(COUNTSPIN_Up);
        if (bCancelStep)
        {
            Value--;
            bCancelStep = 0;
        }
        _itow(Value, ValueText, 10);
    }
    return 1;
}
NW_HOOK_METHOD(0x10005ac0, NCCountSpin, OnUp)

// 0x10005b20 (vslot 69)
INT NCCountSpin::OnDown()
{
    if (Value > Min)
    {
        Value--;
        NotifyParent(COUNTSPIN_Down);
        if (bCancelStep)
        {
            Value++;
            bCancelStep = 0;
        }
        _itow(Value, ValueText, 10);
    }
    return 1;
}
NW_HOOK_METHOD(0x10005b20, NCCountSpin, OnDown)

// 0x10005d90 (vslot 55) - the background, the label, the number 5 pixels after the label's
// width (what DrawText returns)
INT NCCountSpin::OnPaint(NCGDevice* Device)
{
    guard(NCCountSpin::OnPaint);
    Device->SetClip(ScreenX, ScreenY, Width, Height);
    Device->BeginPaint(ScreenX, ScreenY);
    if (BackTex)
        Device->DrawStretchedTex(0, 0, Width, Height, BackTex);
    INT LabelW = Device->DrawText(TextX, TextY, 0xFFFFFFFF, *Label, 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0, 0);
    Device->DrawText(TextX + LabelW + 5, TextY, 0xFFFFFFFF, ValueText, 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0,
                     0, 0, 0);
    Device->EndPaint();
    Device->ReleaseClip();
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x10005d90, NCCountSpin, OnPaint)

// 0x10005ec0 (vslot 36) - NCWnd's, not NControl's (no disabled texture); the buttons stacked
// at the right, each as wide as the control's height less 10 and half that tall, repeating
// every 180 ms while held
INT NCCountSpin::OnCreate()
{
    NCWnd::OnCreate();
    BackTex = FindTexture(TEXT("NWindow.EditBack"));
    INT W = LocalW;
    INT H = LocalH;
    INT ButtonH = (DWORD)(H - 10) >> 1;

    UpButton = new NCPushButton(TEXT("NWindow.WhiteTexture"), TEXT("NWindow.BlackTexture"));
    UpButton->ID = NID_COUNTSPIN_UP;
    UpButton->SetWindowRect(W - H + 5, 3, H - 10, ButtonH);
    CreateChild(UpButton, NWS_VISIBLE);
    UpButton->RepeatDelay = 180;

    DownButton = new NCPushButton(TEXT("NWindow.WhiteTexture"), TEXT("NWindow.BlackTexture"));
    DownButton->ID = NID_COUNTSPIN_DOWN;
    DownButton->SetWindowRect(W - H + 5, H - ButtonH - 3, H - 10, ButtonH);
    CreateChild(DownButton, NWS_VISIBLE);
    DownButton->RepeatDelay = 180;
    return 1;
}
NW_HOOK_METHOD(0x10005ec0, NCCountSpin, OnCreate)

// 0x10006060 (vslot 37) - guarded in C2
INT NCCountSpin::OnDestroy()
{
    guard(NCCountSpin::OnDestroy);
    if (UpButton)
    {
        RemoveChildWindow(UpButton->Entry);
        delete UpButton;
        UpButton = NULL;
    }
    if (DownButton)
    {
        RemoveChildWindow(DownButton->Entry);
        delete DownButton;
        DownButton = NULL;
    }
    NCWnd::OnDestroy();
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x10006060, NCCountSpin, OnDestroy)

NW_CHECK_VTABLE(NCCountSpin, 0x10109678, 70)
