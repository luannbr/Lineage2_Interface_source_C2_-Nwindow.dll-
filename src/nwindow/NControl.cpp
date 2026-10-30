/*=============================================================================
    NControl.cpp - base control. Stock: 0x10001810 .. 0x100018a0, 0x10004e90 .. 0x10005080.
=============================================================================*/
#include "nwindow/NControl.h"

#include <wchar.h>

// the stock code calls the CRT's wcscpy (0x100e78e8); the intrinsic would copy inline
#pragma function(wcscpy)

// 0x10001c30
NControl::NControl()
{
    ID = 0;
    DisableTexture = NULL;
    Caption[0] = 0;
    NotifyID = -1;
}
NW_HOOK_CTOR(0x10001c30, NControl)

// 0x10001be0 - NW_DECLARE_CLASS destructor
NW_HOOK_DTOR(0x10001be0, NControl)

// 0x10043ef0 (vslot 25) - C2: NCWnd's alone (C1 repeated its enabled test first)
INT NControl::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    return NCWnd::WindowProc(Msg, wParam, lParam) != 0;
}
NW_HOOK_METHOD(0x10043ef0, NControl, WindowProc)

// 0x10005940 (vslot 36)
INT NControl::OnCreate()
{
    NCWnd::OnCreate();
    if (!DisableTexture)
        DisableTexture = FindTexture(TEXT("NWindow.Disable"));
    return 1;
}
NW_HOOK_METHOD(0x10005940, NControl, OnCreate)

// 0x10005970 (vslot 37) - C2 guards it and no longer destroys the shared "NWindow.Disable" texture
INT NControl::OnDestroy()
{
    guard(NControl::OnDestroy);
    NCWnd::OnDestroy();
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x10005970, NControl, OnDestroy)

/*-----------------------------------------------------------------------------
    Parent communication (names ours).
-----------------------------------------------------------------------------*/

// 0x10005820 - NWM_COMMAND(ID, Code) to the parent
UBOOL NControl::NotifyParent(LPARAM Code)
{
    NCWnd* Parent = ParentWnd();
    if (!Parent)
        return 0;
    Parent->WindowProc(NWM_COMMAND, ID, Code);
    return 1;
}
NW_HOOK_METHOD(0x10005820, NControl, NotifyParent)

// 0x10005860 - the parent takes the focus back
void NControl::ReleaseFocus()
{
    if (NCWnd* Parent = ParentWnd())
        Parent->SetFocus();
    KillFocus();
}
NW_HOOK_METHOD(0x10005860, NControl, ReleaseFocus)

// 0x10005890
void NControl::HideParent()
{
    if (NCWnd* Parent = ParentWnd())
        Parent->HideWindow();
    KillFocus();
}
NW_HOOK_METHOD(0x10005890, NControl, HideParent)

// 0x100058c0 - does not use this
UBOOL NControl::SendTo(NWndEntry* Target, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (!Target || !Target->Wnd)
        return 0;
    return Target->Wnd->WindowProc(Msg, wParam, lParam) != 0;
}
NW_HOOK_METHOD(0x100058c0, NControl, SendTo)

// 0x10005900 - the parent's answer is dropped
UBOOL NControl::SendToParent(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    NCWnd* Parent = ParentWnd();
    if (!Parent)
        return 0;
    Parent->WindowProc(Msg, wParam, lParam);
    return 1;
}
NW_HOOK_METHOD(0x10005900, NControl, SendToParent)

// 0x100059f0 - no length check, as in the stock code (Caption holds 128)
void NControl::SetCaption(const TCHAR* Text)
{
    if (Text)
        wcscpy(Caption, Text);
}
NW_HOOK_METHOD(0x100059f0, NControl, SetCaption)

// 0x10005a10 - NULL when empty
const TCHAR* NControl::GetCaption()
{
    const TCHAR* Text = Caption;
    if (!Text || !*Text)
        return NULL;
    return Text;
}
NW_HOOK_METHOD(0x10005a10, NControl, GetCaption)

NW_CHECK_VTABLE(NControl, 0x101081e8, 68)
