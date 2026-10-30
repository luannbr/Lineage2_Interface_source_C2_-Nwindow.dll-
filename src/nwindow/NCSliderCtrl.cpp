/*=============================================================================
    NCSliderCtrl.cpp - the slider control (C2 only). Stock: 0x10026140 .. 0x100264ed.
=============================================================================*/
#include "nwindow/NCSliderCtrl.h"
#include "nwindow/NCThumbButton.h"
#include "nwindow/NCGDevice.h"

enum { NID_SLIDER_THUMB = (INT)0xF0F01415 };   // the thumb's id (name ours)

// 0x10026200 - NControl's constructor is inlined in the stock code; the members are left as
// allocated
NCSliderCtrl::NCSliderCtrl()
{
}
NW_HOOK_CTOR(0x10026200, NCSliderCtrl)

// 0x10026270
NW_HOOK_DTOR(0x10026270, NCSliderCtrl)

// 0x10026140 (vslot 25)
INT NCSliderCtrl::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (NControl::WindowProc(Msg, wParam, lParam))
        return 1;
    if (Msg == NWM_COMMAND && (INT)wParam == NID_SLIDER_THUMB)
        return OnThumbMove(lParam) != 0;
    return 0;
}
NW_HOOK_METHOD(0x10026140, NCSliderCtrl, WindowProc)

// 0x100261a0 (vslot 68) - the thumb was dragged to X (screen): Pos follows, kept in 0..Max, and
// the thumb moves by the difference
INT NCSliderCtrl::OnThumbMove(INT X)
{
    X = X - ScreenX;
    INT Old = Pos;
    Pos = X;
    if (X < Max)
    {
        if (X < 1)
            Pos = 0;
    }
    else
        Pos = Max;
    Thumb->MoveWindow(Pos - Old, 0);
    return 1;
}
NW_HOOK_METHOD(0x100261a0, NCSliderCtrl, OnThumbMove)

// 0x100262c0 (vslot 36) - the thumb at Pos / Max of the width, centered on it
INT NCSliderCtrl::OnCreate()
{
    NControl::OnCreate();
    BackTex = FindTexture(TEXT("NWindow.BlackTexture"));
    INT W = LocalW;

    Thumb = new NCThumbButton(TEXT("NWindow.MenuHighlight"), TEXT("NWindow.WhiteTexture"), 0);
    Thumb->ID = NID_SLIDER_THUMB;
    Thumb->SetWindowRect((INT)((DWORD)(Pos * W) / (DWORD)Max) - ThumbW / 2, 0, ThumbW, ThumbH);
    CreateChild(Thumb, NWS_VISIBLE);
    return 1;
}
NW_HOOK_METHOD(0x100262c0, NCSliderCtrl, OnCreate)

// 0x100263c0 (vslot 37) - guarded in C2; NControl's OnDestroy is not called
INT NCSliderCtrl::OnDestroy()
{
    guard(NCSliderCtrl::OnDestroy);
    if (Thumb)
    {
        RemoveChildWindow(Thumb->Entry);
        delete Thumb;
        Thumb = NULL;
    }
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x100263c0, NCSliderCtrl, OnDestroy)

// 0x10026460 (vslot 55) - the origin first, then the clip
INT NCSliderCtrl::OnPaint(NCGDevice* Device)
{
    Device->BeginPaint(ScreenX, ScreenY);
    Device->SetClip(ScreenX, ScreenY, Width, Height);
    if (BackTex)
        Device->DrawTexture(0, 0, Width, Height, 0, 0, Width, Height, BackTex);
    Device->ReleaseClip();
    Device->EndPaint();
    return 1;
}
NW_HOOK_METHOD(0x10026460, NCSliderCtrl, OnPaint)

NW_CHECK_VTABLE(NCSliderCtrl, 0x1010e3b8, 69)
