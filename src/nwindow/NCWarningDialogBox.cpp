/*=============================================================================
    NCWarningDialogBox.cpp - the small warning box of the console (C2 only). Stock: 0x100090e0 ..
    0x100094cf, and SetText at 0x10006e00 (between NCDialogBox's functions).
=============================================================================*/
#include "nwindow/NCWarningDialogBox.h"
#include "nwindow/NCButton.h"
#include "nwindow/NCGDevice.h"

// 0x100090e0 - the texture and the button come with OnCreate
NCWarningDialogBox::NCWarningDialogBox()
{
    bExit = 0;
}
NW_HOOK_CTOR(0x100090e0, NCWarningDialogBox)

// 0x10009170
NW_HOOK_DTOR(0x10009170, NCWarningDialogBox)

// 0x100827a0 (vslot 25) - the stock body is one with NCKeyCryptWarningDialogBox's (hooked there)
INT NCWarningDialogBox::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (NCDialogBox::WindowProc(Msg, wParam, lParam))
        return 1;
    if (Msg == NWM_COMMAND && wParam == NID_DIALOG_OK)
        return OnOK() != 0;
    return 0;
}

// 0x10006e00 - a text is taken only while there is none (an empty FString holds 0 or 1 characters)
void NCWarningDialogBox::SetText(const TCHAR* InText)
{
    if (Text.Num() == 0 || Text.Num() == 1)
        Text = InText;
}
NW_HOOK_METHOD(0x10006e00, NCWarningDialogBox, SetText)

// 0x100092f0 (vslot 36) - the OK button under the text, centered by the box's LocalW
INT NCWarningDialogBox::OnCreate()
{
    NCDialogBox::OnCreate();
    BackTex = FindTexture(TEXT("L2UI.EtcWnd.Popup"));
    DWORD W = LocalW;

    OKButton = new NCButton(TEXT("L2UI.Control.DefaultButton"), TEXT("L2UI.Control.DefaultButton_click"), 0);
    OKButton->ID = NID_DIALOG_OK;
    OKButton->SetWindowRect((W >> 1) - 33, 65, 67, 19);
    OKButton->SetText(*SysString(140));  // "OK"
    CreateChild(OKButton, NWS_VISIBLE);
    return 1;
}
NW_HOOK_METHOD(0x100092f0, NCWarningDialogBox, OnCreate)

// 0x100093e0 (vslot 37) - guarded in C2
INT NCWarningDialogBox::OnDestroy()
{
    guard(NCWarningDialogBox::OnDestroy);
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
NW_HOOK_METHOD(0x100093e0, NCWarningDialogBox, OnDestroy)

// 0x100091e0 (vslot 55) - the popup picture (a 205 x 87 source) and the text at (5, 5)
INT NCWarningDialogBox::OnPaint(NCGDevice* Device)
{
    guard(NCWarningDialogBox::OnPaint);
    Device->SetClip(ScreenX, ScreenY, Width, Height);
    Device->BeginPaint(ScreenX, ScreenY);
    if (BackTex)
        Device->DrawTexture(0, 0, Width, Height, 0, 0, 205, 87, BackTex);
    if (Text.Num() != 0)
        Device->DrawText(5, 5, 0xFFFFFFFF, *Text, Width - 10, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0, 0);
    Device->EndPaint();
    Device->ReleaseClip();
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x100091e0, NCWarningDialogBox, OnPaint)

// 0x10009490 (vslot 68) - OK: a fatal warning ends the client, any other closes the box
INT NCWarningDialogBox::OnOK()
{
    if (bExit)
    {
        appRequestExit(0);
        return 1;
    }
    HideModal();
    bExit = 0;
    return 1;
}
NW_HOOK_METHOD(0x10009490, NCWarningDialogBox, OnOK)

NW_CHECK_VTABLE(NCWarningDialogBox, 0x1010a060, 69)
