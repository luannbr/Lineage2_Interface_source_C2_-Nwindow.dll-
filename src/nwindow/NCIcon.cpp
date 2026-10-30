/*=============================================================================
    NCIcon.cpp - icon of a minimized window. Stock (C2): 0x10018e90 .. 0x100193e8.
=============================================================================*/
#include "nwindow/NCIcon.h"
#include "nwindow/NCGDevice.h"
#include "nwindow/NConsoleWnd.h"

enum { SOUND_Click = 0 };

// 0x100190a0
NCIcon::NCIcon()
{
    DragY = 0;
    DragX = 0;
    IconTex = NULL;
    bMouseOver = 0;
    Unknown1C0 = -1;
    bToolTipAbove = 0;
    ToolTipTex = FindTexture(TEXT("NWindow.ChatBack"));
    bDragged = 0;
}
NW_HOOK_CTOR(0x100190a0, NCIcon)

// 0x10019180 - ConditionalDestroy, ~FString (Name), ~NControl
NW_HOOK_DTOR(0x10019180, NCIcon)

// 0x10018e90 (vslot 25) - the double click goes to slot 68
INT NCIcon::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (NControl::WindowProc(Msg, wParam, lParam))
        return 1;
    if (Msg == WM_LBUTTONDBLCLK)
        return OnLButtonDblClick(wParam, lParam) != 0;
    return 0;
}
NW_HOOK_METHOD(0x10018e90, NCIcon, WindowProc)


// 0x100191f0 (vslot 55)
INT NCIcon::OnPaint(NCGDevice* Device)
{
    guard(NCIcon::OnPaint);
    Device->BeginPaint(ScreenX, ScreenY);
    Device->SetClip(ScreenX, ScreenY, Width, Height);
    if (IconTex)
        Device->DrawTexture(0, 0, 32, 32, 0, 0, 32, 32, IconTex);
    Device->ReleaseClip();
    Device->EndPaint();
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x100191f0, NCIcon, OnPaint)

// 0x100192c0 (vslot 56) - same box as the buttons' (NCGDevice::DrawToolTipBox); Name is an FString now
INT NCIcon::OnDrawToolTip(NCGDevice* Device)
{
    Device->BeginPaint(ScreenX, ScreenY);
    Device->SetClip(ScreenX, ScreenY, Width, Height);
    if (Name.Num() && bMouseOver)
        Device->DrawToolTipBox(this, *Name, ToolTipTex, bToolTipAbove);
    Device->ReleaseClip();
    Device->EndPaint();
    return 1;
}
NW_HOOK_METHOD(0x100192c0, NCIcon, OnDrawToolTip)

// 0x10018f50 (vslot 60) - dragged: WM_MOVE(delta) to itself (NCWnd::OnMove snaps it);
// DragMouse holds the delta during the call
INT NCIcon::OnMouseMove(WPARAM wParam, LPARAM lParam)
{
    INT X = (SWORD)LOWORD(lParam);
    INT Y = (SWORD)HIWORD(lParam);
    if (wParam & MK_LBUTTON)
    {
        DragMouseX = X - DragMouseX;
        DragMouseY = Y - DragMouseY;
        SendTo(Entry, WM_MOVE, 0, MAKELPARAM(DragMouseX, DragMouseY));
        DragMouseX = X;
        DragMouseY = Y;
        bDragged = 1;
        return 1;
    }
    bMouseOver = PtInRect(X, Y, ScreenX, ScreenY, Width, Height) != 0;
    return 1;
}
NW_HOOK_METHOD(0x10018f50, NCIcon, OnMouseMove)

// 0x10019000 (vslot 61)
INT NCIcon::OnLButtonDown(WPARAM wParam, LPARAM lParam)
{
    LockCursor();
    DragMouseX = (SWORD)LOWORD(lParam);
    DragX = DragMouseX - ScreenX;
    DragMouseY = (SWORD)HIWORD(lParam);
    DragY = DragMouseY - ScreenY;
    return 1;
}
NW_HOOK_METHOD(0x10019000, NCIcon, OnLButtonDown)

// 0x10019040 (vslot 62) - a click without a drag restores the window
INT NCIcon::OnLButtonUp(WPARAM wParam, LPARAM lParam)
{
    if (bDragged)
    {
        UnlockCursor();
        ReleaseFocus();
        bDragged = 0;
        return 1;
    }
    PlayInterfaceSound(SOUND_Click);
    Entry->Parent->Wnd->Restore();
    return 1;
}
NW_HOOK_METHOD(0x10019040, NCIcon, OnLButtonUp)

// 0x10018ef0 - the tool tip text is a copy of the SysString (C1 copied it into a TCHAR[128])
void NCIcon::SetIcon(const TCHAR* TexName, INT NameID, BYTE bAbove)
{
    if (TexName)
        IconTex = FindTexture(TexName);
    if (NameID >= 0)
        Name = *SysString(NameID);
    bToolTipAbove = bAbove;
}
NW_HOOK_METHOD(0x10018ef0, NCIcon, SetIcon)

NW_CHECK_VTABLE(NCIcon, 0x1010d540, 69)
