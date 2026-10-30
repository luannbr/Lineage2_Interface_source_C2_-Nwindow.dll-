/*=============================================================================
    NCRadarWnd.cpp - the radar. Stock: 0x100a1c90 .. 0x100a1db5, 0x100a1df0 .. 0x100a1e4f.
=============================================================================*/
#include "nwindow/NCRadarWnd.h"
#include "nwindow/NCGDevice.h"

// 0x100c0d70
NCRadarWnd::NCRadarWnd()
{
}
NW_HOOK_CTOR(0x100c0d70, NCRadarWnd)

// 0x100c0dd0
NW_HOOK_DTOR(0x100c0dd0, NCRadarWnd)

// stock body shared with NControl's (0x10004e90)
INT NCRadarWnd::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (Msg != WM_PAINT && !IsEnabled())
        return 0;
    return NCWnd::WindowProc(Msg, wParam, lParam) != 0;
}

// 0x100c0be0 (vslot 36)
INT NCRadarWnd::OnCreate()
{
    NCWnd::OnCreate();
    return 1;
}
NW_HOOK_METHOD(0x100c0be0, NCRadarWnd, OnCreate)

// 0x100c0c90 (vslot 37)
INT NCRadarWnd::OnDestroy()
{
    guard(NCRadarWnd::OnDestroy);
    Targets.Empty();
    NCWnd::OnDestroy();
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x100c0c90, NCRadarWnd, OnDestroy)

// 0x100c0bf0 (vslot 55) - the radar coordinates are the world's
INT NCRadarWnd::OnPaint(NCGDevice* Device)
{
    Device->BeginPaint(ScreenX, ScreenY);
    Device->SetClip(ScreenX, ScreenY, Width, Height);
    Device->DrawRadar();
    for (INT i = 0; i < Targets.Num(); i++)
    {
        FMinimapTarget T = Targets(i);
        Device->DrawRadarTarget(T.X, T.Y);
    }
    Device->EndPaint();
    Device->ReleaseClip();
    return 1;
}
NW_HOOK_METHOD(0x100c0bf0, NCRadarWnd, OnPaint)

// 0x1009a540 (C1 0x100a1d70; the stock body sits beside NCMinimapWnd's methods and serves both windows: the
// console calls it with the radar and with the minimap) - the target list emptied
void NCRadarWnd::ClearTargets()
{
    Targets.Empty();
}
NW_HOOK_METHOD(0x1009a540, NCRadarWnd, ClearTargets)

// the stock body is NCMinimapWnd's (0x10079230)
void NCRadarWnd::AddTarget(INT X, INT Y, INT Z)
{
    INT i = Targets.Add();
    Targets(i).X = X;
    Targets(i).Y = Y;
    Targets(i).Z = Z;
}

// the stock body is NCMinimapWnd's (0x10079270), with its skip after a removed entry
void NCRadarWnd::RemoveTarget(INT X, INT Y, INT Z)
{
    for (INT i = 0; i < Targets.Num(); i++)
    {
        FMinimapTarget T = Targets(i);
        if (T.X == X && T.Y == Y && T.Z == Z)
            Targets.Remove(i, 1);
    }
}

NW_CHECK_VTABLE(NCRadarWnd, 0x10123670, 67)
