/*=============================================================================
    NCProgressCtrl.cpp - count-down bar. Stock: 0x10005240 .. 0x100052ab,
    0x10005730 .. 0x10005a7e.
=============================================================================*/
#include "nwindow/NCProgressCtrl.h"
#include "nwindow/NCGDevice.h"

// 0x10006130
NCProgressCtrl::NCProgressCtrl()
{
    Unknown1A8 = 1;
    bStopped = 1;
    BarTex = NULL;
    BackTex = NULL;
    Border = 2;
    Pos = 0;
    Max = 0;
    BackTex = FindTexture(TEXT("NWindow.BlackTexture"));
    BarTex = FindTexture(TEXT("NWindow.WhiteTexture"));
}
NW_HOOK_CTOR(0x10006130, NCProgressCtrl)

// 0x10006270
NCProgressCtrl::NCProgressCtrl(const TCHAR* BackTexName, const TCHAR* BarTexName, INT InBorder)
{
    Unknown1A8 = 1;
    bStopped = 1;
    BarTex = NULL;
    BackTex = NULL;
    Border = 2;
    Pos = 0;
    Max = 0;
    BackTex = FindTexture(BackTexName ? BackTexName : TEXT("NWindow.BlackTexture"));
    BarTex = FindTexture(BarTexName ? BarTexName : TEXT("NWindow.WhiteTexture"));
    Border = InBorder;
}
NW_HOOK_CTOR_ARGS(0x10006270, NCProgressCtrl, (const TCHAR* A, const TCHAR* B, INT C), (A, B, C))

// 0x10006220
NW_HOOK_DTOR(0x10006220, NCProgressCtrl)

// stock body shared with NCButton's (0x100051f0)
INT NCProgressCtrl::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (Msg != WM_PAINT && !IsEnabled())
        return 0;
    return NControl::WindowProc(Msg, wParam, lParam) != 0;
}

// 0x10005b80 - `return 1` shared by 6 slots of several classes
NW_HOOK_METHOD(0x10005b80, NCProgressCtrl, OnCreate)

// 0x10006360 (vslot 55) - clip first, origin after, as in the stock code
INT NCProgressCtrl::OnPaint(NCGDevice* Device)
{
    DWORD Elapsed = (INT)(GetDeltaTime() * 1000.f);
    INT BarW = Width - Border * 2;
    if (!bStopped)
    {
        Pos += Elapsed;
        if (Pos >= Max)
        {
            NotifyParent(0);
            Pos = Max;
            bStopped = 1;
        }
    }
    DWORD Done = Pos * BarW / Max;
    Device->SetClip(ScreenX, ScreenY, Width, Height);
    Device->BeginPaint(ScreenX, ScreenY);
    if (BackTex)
        Device->DrawTexture(0, 0, Width, Height, 0, 0, Width, Height, BackTex);
    if (BarTex)
        Device->DrawTexture(Border, Border, BarW - Done, Height - Border * 2, 0, 0, Width, Height,
                            BarTex);
    Device->EndPaint();
    Device->ReleaseClip();
    return 1;
}
NW_HOOK_METHOD(0x10006360, NCProgressCtrl, OnPaint)

// 0x10005b90
void NCProgressCtrl::SetRemaining(DWORD Remaining)
{
    Pos = Max - Remaining;
}
NW_HOOK_METHOD(0x10005b90, NCProgressCtrl, SetRemaining)

// 0x10005bb0
void NCProgressCtrl::SetMax(DWORD InMax)
{
    Max = InMax;
}
NW_HOOK_METHOD(0x10005bb0, NCProgressCtrl, SetMax)

// 0x10005bc0
void NCProgressCtrl::Reset()
{
    Pos = 0;
    bStopped = 1;
}
NW_HOOK_METHOD(0x10005bc0, NCProgressCtrl, Reset)

// 0x10005be0
void NCProgressCtrl::Stop()
{
    bStopped = 1;
}
NW_HOOK_METHOD(0x10005be0, NCProgressCtrl, Stop)

// 0x10005bf0
void NCProgressCtrl::Start()
{
    bStopped = 0;
}
NW_HOOK_METHOD(0x10005bf0, NCProgressCtrl, Start)

NW_CHECK_VTABLE(NCProgressCtrl, 0x10109810, 68)
