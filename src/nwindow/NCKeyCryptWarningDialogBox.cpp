/*=============================================================================
    NCKeyCryptWarningDialogBox.cpp - the keyboard protection warning (C2 only). Stock:
    0x100827a0 .. 0x100827f5 (WindowProc), 0x10082800 .. 0x1008280a and 0x10082ce0 .. 0x10083037.
=============================================================================*/
#include "nwindow/NCKeyCryptWarningDialogBox.h"
#include "nwindow/NCButton.h"
#include "nwindow/NCGDevice.h"

enum { SYSMSG_KeyCryptWarning = 1239 };   // the text of the box (name ours)

// 0x10082ce0
NCKeyCryptWarningDialogBox::NCKeyCryptWarningDialogBox()
{
    BackTex = NULL;
    OKButton = NULL;
}
NW_HOOK_CTOR(0x10082ce0, NCKeyCryptWarningDialogBox)

// 0x10082d30
NW_HOOK_DTOR(0x10082d30, NCKeyCryptWarningDialogBox)

// 0x100827a0 (vslot 25) - the stock body is shared with NCWarningDialogBox's
INT NCKeyCryptWarningDialogBox::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (NCDialogBox::WindowProc(Msg, wParam, lParam))
        return 1;
    if (Msg == NWM_COMMAND && wParam == NID_DIALOG_OK)
        return OnOK() != 0;
    return 0;
}
NW_HOOK_METHOD(0x100827a0, NCKeyCryptWarningDialogBox, WindowProc)

// 0x10082800 (vslot 68) - OK closes the box
INT NCKeyCryptWarningDialogBox::OnOK()
{
    HideModal();
    return 1;
}
NW_HOOK_METHOD(0x10082800, NCKeyCryptWarningDialogBox, OnOK)

// 0x10082d80 (vslot 36) - the OK button under the text (60 px down: one line less than the
// warning box's), centered by the box's LocalW
INT NCKeyCryptWarningDialogBox::OnCreate()
{
    NCDialogBox::OnCreate();
    BackTex = FindTexture(TEXT("L2UI.EtcWnd.Popup"));
    DWORD W = LocalW;

    OKButton = new NCButton(TEXT("L2UI.Control.DefaultButton"), TEXT("L2UI.Control.DefaultButton_click"), 0);
    OKButton->ID = NID_DIALOG_OK;
    OKButton->SetWindowRect((W >> 1) - 33, 60, 67, 19);
    OKButton->SetText(*SysString(140));  // "OK"
    CreateChild(OKButton, NWS_VISIBLE);
    return 1;
}
NW_HOOK_METHOD(0x10082d80, NCKeyCryptWarningDialogBox, OnCreate)

// 0x10082e70 (vslot 37) - guarded in C2
INT NCKeyCryptWarningDialogBox::OnDestroy()
{
    guard(NCKeyCryptWarningDialogBox::OnDestroy);
    if (OKButton)
    {
        RemoveChildWindow(OKButton->Entry);
        delete OKButton;
        OKButton = NULL;
    }
    NCDialogBox::OnDestroy();
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x10082e70, NCKeyCryptWarningDialogBox, OnDestroy)

// 0x10082f20 (vslot 55) - the popup picture (a 205 x 87 source) and the system message at (10, 10)
INT NCKeyCryptWarningDialogBox::OnPaint(NCGDevice* Device)
{
    guard(NCKeyCryptWarningDialogBox::OnPaint);
    Device->SetClip(ScreenX, ScreenY, Width, Height);
    Device->BeginPaint(ScreenX, ScreenY);
    if (BackTex)
        Device->DrawTexture(0, 0, Width, Height, 0, 0, 205, 87, BackTex);
    Device->DrawText(10, 10, 0xFFFFFFFF, *GL2GameData.SystemMsg[SYSMSG_KeyCryptWarning].Message, Width - 20, 0, 0,
                     L2FONT_Normal, 0, 0, 0, 0, 0, 0, 0);
    Device->EndPaint();
    Device->ReleaseClip();
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x10082f20, NCKeyCryptWarningDialogBox, OnPaint)

NW_CHECK_VTABLE(NCKeyCryptWarningDialogBox, 0x10119538, 69)
