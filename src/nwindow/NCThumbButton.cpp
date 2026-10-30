/*=============================================================================
    NCThumbButton.cpp - the handle of the slider control (C2 only). Stock: 0x100013f0 ..
    0x1000143d, 0x10002900 .. 0x10002a2f (between NCExButton's and NCPushButton's constructors).
=============================================================================*/
#include "nwindow/NCThumbButton.h"

// 0x10002900
NCThumbButton::NCThumbButton()
{
}
NW_HOOK_CTOR(0x10002900, NCThumbButton)

// 0x100029a0
NCThumbButton::NCThumbButton(const TCHAR* NormalTexName, const TCHAR* PushedTexName, UBOOL bInAcceptKey)
    : NCButton(NormalTexName, PushedTexName, bInAcceptKey)
{
}
NW_HOOK_CTOR_ARGS(0x100029a0, NCThumbButton, (const TCHAR* A, const TCHAR* B, UBOOL C), (A, B, C))

// 0x10002950
NW_HOOK_DTOR(0x10002950, NCThumbButton)

// 0x10001410 (vslot 60) - dragged: the pointer's x goes to the slider
INT NCThumbButton::OnMouseMove(WPARAM wParam, LPARAM lParam)
{
    if (wParam & MK_LBUTTON)
        NotifyParent(LOWORD(lParam));
    return 1;
}
NW_HOOK_METHOD(0x10001410, NCThumbButton, OnMouseMove)

// 0x100013f0 (vslot 61)
INT NCThumbButton::OnLButtonDown(WPARAM wParam, LPARAM lParam)
{
    bReleased = 0;
    return 1;
}
NW_HOOK_METHOD(0x100013f0, NCThumbButton, OnLButtonDown)

// 0x10001430 (vslot 62)
INT NCThumbButton::OnLButtonUp(WPARAM wParam, LPARAM lParam)
{
    bReleased = 1;
    return 1;
}
NW_HOOK_METHOD(0x10001430, NCThumbButton, OnLButtonUp)

NW_CHECK_VTABLE(NCThumbButton, 0x101085e8, 68)
