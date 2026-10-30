/*=============================================================================
    NTownMapWnd.cpp - the town map window (C2 only). Stock: 0x10098e80 .. 0x10098ec2,
    0x10098fe0 .. 0x100991af and 0x100998a0 .. 0x10099b6f.
=============================================================================*/
#include "nwindow/NTownMapWnd.h"
#include "nwindow/NCFrameCtrl.h"
#include "nwindow/NCGDevice.h"
#include "nwindow/NConsoleWnd.h"

enum { NID_TOWNMAP_FRAME = 0xB000F0 };   // the title bar's id (name ours)

// 0x100998a0 - the picture is 512 x 512, the view 328 x 329; the icon's place is left as allocated
NTownMapWnd::NTownMapWnd()
{
    BackTex = NULL;
    MapTex = NULL;
    IconTex = NULL;
    DragX = 0;
    DragY = 0;
    ScrollX = 0;
    ScrollY = 0;
    bDragging = 0;
    MapW = 512;
    MapH = 512;
    ViewW = 328;
    ViewH = 329;
}
NW_HOOK_CTOR(0x100998a0, NTownMapWnd)

// 0x10099940
NW_HOOK_DTOR(0x10099940, NTownMapWnd)

// 0x100991a0 (vslot 30) - the stock body is NCMinimapWnd's (hooked there): a drag in progress
// lets the cursor go
void NTownMapWnd::KillFocus()
{
    UnlockCursor();
    NCWnd::KillFocus();
}

// 0x10098e80 (vslot 37) - guarded in C2
INT NTownMapWnd::OnDestroy()
{
    guard(NTownMapWnd::OnDestroy);
    NCWnd::OnDestroy();
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x10098e80, NTownMapWnd, OnDestroy)

// 0x10098f00 (name ours; the console's ShowTownMap calls it) - the town's picture by name and the player's place
// in it (a place outside the picture counts as its corner), the view centred on the place and then kept inside
// the picture. A NULL name changes nothing.
void NTownMapWnd::SetMap(const TCHAR* TexName, INT X, INT Y)
{
    if (!TexName)
        return;
    MapTex = FindTexture(TexName);
    if (X >= 0 && X <= MapW)
        IconX = X;
    else
        IconX = 0;
    if (Y >= 0 && Y <= MapH)
        IconY = Y;
    else
        IconY = 0;
    ScrollX = IconX - ViewW / 2;
    ScrollY = IconY - ViewH / 2;
    if (ScrollX < 0)
        ScrollX = 0;
    else if (ScrollX > MapW - ViewW)
        ScrollX = MapW - ViewW;
    if (ScrollY < 0)
        ScrollY = 0;
    else if (ScrollY > MapH - ViewH)
        ScrollY = MapH - ViewH;
}
NW_HOOK_METHOD(0x10098f00, NTownMapWnd, SetMap)

// 0x10098fe0 (vslot 61) - a press inside the view starts a drag and locks the cursor (the point
// is taken relative to the view: 3 and 21 pixels of frame off, and pulled in to its edge)
INT NTownMapWnd::OnLButtonDown(WPARAM wParam, LPARAM lParam)
{
    INT X = ((INT)(SHORT)LOWORD(lParam) - ScreenX) - 3;
    INT Y = ((INT)(SHORT)HIWORD(lParam) - ScreenY) - 21;
    if (X < 4)
        X = 3;
    if (Y < 22)
        Y = 21;
    if (X > 3 && X <= Width && Y > 21 && Y <= Height)
    {
        DragX = X;
        DragY = Y;
        bDragging = 1;
        SetCursor(3, NULL);
        GConsole->LockCursor();
    }
    return 1;
}
NW_HOOK_METHOD(0x10098fe0, NTownMapWnd, OnLButtonDown)

// 0x10099060 (vslot 62)
INT NTownMapWnd::OnLButtonUp(WPARAM wParam, LPARAM lParam)
{
    GConsole->UnlockCursor();
    SetCursor(0, NULL);
    bDragging = 0;
    return 1;
}
NW_HOOK_METHOD(0x10099060, NTownMapWnd, OnLButtonUp)

// 0x10099090 (vslot 60) - the drag moves the view by the distance the pointer went, kept inside the picture
INT NTownMapWnd::OnMouseMove(WPARAM wParam, LPARAM lParam)
{
    if (IsFocused())
    {
        INT X = ((INT)(SHORT)LOWORD(lParam) - ScreenX) - 3;
        INT Y = ((INT)(SHORT)HIWORD(lParam) - ScreenY) - 21;
        if (X < 4)
            X = 3;
        if (Y < 22)
            Y = 21;
        if (bDragging && X > 3 && X <= Width && Y > 21 && Y <= Height)
        {
            INT OldY = DragY;
            ScrollX += DragX - X;
            DragY = Y;
            ScrollY += OldY - Y;
            DragX = X;
            if (ScrollX < 0)
                ScrollX = 0;
            else if (MapW - ViewW < ScrollX)
                ScrollX = MapW - ViewW;
            if (ScrollY < 0)
            {
                ScrollY = 0;
                return 1;
            }
            if (MapH - ViewH < ScrollY)
                ScrollY = MapH - ViewH;
        }
    }
    return 1;
}
NW_HOOK_METHOD(0x10099090, NTownMapWnd, OnMouseMove)

// 0x10099990 (vslot 36) - a 334 x 21 title bar, the frame and the player's mark
INT NTownMapWnd::OnCreate()
{
    NCWnd::OnCreate();
    FrameCtrl = new NCFrameCtrl(1);
    FrameCtrl->ID = NID_TOWNMAP_FRAME;
    FrameCtrl->SetWindowRect(0, 0, 334, 21);
    CreateChild(FrameCtrl, NWS_VISIBLE);
    BackTex = FindTexture(TEXT("L2UI.MinimapWnd.MinimapBack"));
    IconTex = FindTexture(TEXT("town_map.icon_i00"));
    return 1;
}
NW_HOOK_METHOD(0x10099990, NTownMapWnd, OnCreate)

// 0x10099a60 (vslot 55) - the part of the picture, the mark where it falls in the view, and the
// frame over both; the clip is set before the origin and keeps the last 3 rows out
INT NTownMapWnd::OnPaint(NCGDevice* Device)
{
    Device->SetClip(ScreenX, ScreenY, Width, Height - 3);
    Device->BeginPaint(ScreenX, ScreenY);
    if (MapTex)
        Device->DrawTexture(3, 21, ViewW, ViewH, ScrollX, ScrollY, ViewW, ViewH, MapTex);
    if (IconTex)
        Device->DrawTexture((IconX - ScrollX) + 3, (IconY - ScrollY) + 21, 32, 32, 0, 0, 32, 32, IconTex);
    if (BackTex)
        Device->DrawTexture(0, 21, 334, 333, 0, 0, 334, 333, BackTex);
    Device->EndPaint();
    Device->ReleaseClip();
    return 1;
}
NW_HOOK_METHOD(0x10099a60, NTownMapWnd, OnPaint)

NW_CHECK_VTABLE(NTownMapWnd, 0x1011d038, 67)
