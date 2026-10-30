/*=============================================================================
    NCCheckBox.cpp - check box. Stock (C2): 0x10004530 .. 0x10004b90.
=============================================================================*/
#include "nwindow/NCCheckBox.h"
#include "nwindow/NCGDevice.h"

#include <wchar.h>

// 0x10004630 - the textures come in OnCreate; the label is an FString now (an empty one is left empty)
NCCheckBox::NCCheckBox(const TCHAR* InLabel, UBOOL bInChecked, UBOOL bInLabelLeft)
{
    if (InLabel && *InLabel)
        Label = InLabel;
    BoxW = 12;
    BoxH = 12;
    CheckedTex = NULL;
    UncheckedTex = NULL;
    bDisabled = 0;
    bChecked = bInChecked;
    bLabelLeft = bInLabelLeft;
}
NW_HOOK_CTOR_ARGS(0x10004630, NCCheckBox, (const TCHAR* A, UBOOL B, UBOOL C), (A, B, C))

// 0x100047a0 - label from the SysString table, "Check Box" when missing
NCCheckBox::NCCheckBox(INT LabelID, UBOOL bInChecked, UBOOL bInLabelLeft)
{
    if (*SysString(LabelID))
        Label = *SysString(LabelID);
    else
        Label = TEXT("Check Box");
    BoxW = 12;
    BoxH = 12;
    bLabelLeft = bInLabelLeft;
    CheckedTex = NULL;
    UncheckedTex = NULL;
    bDisabled = 0;
    bChecked = bInChecked;
}
NW_HOOK_CTOR_ARGS(0x100047a0, NCCheckBox, (INT A, UBOOL B, UBOOL C), (A, B, C))

// 0x10004730 - ConditionalDestroy, ~FString (Label), ~NControl
NW_HOOK_DTOR(0x10004730, NCCheckBox)

// 0x10090b60 (vslot 25) - the stock body is NCButton's
INT NCCheckBox::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    return NControl::WindowProc(Msg, wParam, lParam) != 0;
}

// 0x10004530 (vslot 36) - NControl's is not called: no Disable texture
INT NCCheckBox::OnCreate()
{
    CheckedTex = FindTexture(TEXT("L2UI.Control.CheckBox_Checked"));
    UncheckedTex = FindTexture(TEXT("L2UI.Control.CheckBox"));
    UncheckedDisabledTex = FindTexture(TEXT("L2UI.Control.CheckBox_Unable"));
    CheckedDisabledTex = FindTexture(TEXT("L2UI.Control.CheckBox_Checked_unable"));
    return 1;
}
NW_HOOK_METHOD(0x10004530, NCCheckBox, OnCreate)

// 0x10004580 (vslot 37) - guarded in C2 (C1: NCButton's body)
INT NCCheckBox::OnDestroy()
{
    guard(NCCheckBox::OnDestroy);
    NControl::OnDestroy();
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x10004580, NCCheckBox, OnDestroy)

// 0x100048a0 (vslot 55)
INT NCCheckBox::OnPaint(NCGDevice* Device)
{
    guard(NCCheckBox::OnPaint);
    Device->BeginPaint(ScreenX, ScreenY);
    Device->SetClip(ScreenX, ScreenY, Width, Height);
    UTexture* Box;
    if (bDisabled)
        Box = bChecked ? CheckedDisabledTex : UncheckedDisabledTex;
    else
        Box = bChecked ? CheckedTex : UncheckedTex;
    DWORD Color = bDisabled ? 0xFF808080 : 0xFFFFFFFF;
    if (bLabelLeft)
    {
        SIZE Size;
        NCGDevice::GetTextExtent(&Size, *Label, 0, 0, 0, 0, 0, L2FONT_Normal);
        Device->DrawTexture(Size.cx + 4, 0, BoxW, BoxH, 0, 0, BoxW, BoxH, Box);
        Device->DrawText(0, 0, Color, *Label, 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0, 0);
    }
    else
    {
        Device->DrawTexture(0, 0, BoxW, BoxH, 0, 0, BoxW, BoxH, Box);
        Device->DrawText(BoxW + 4, 0, Color, *Label, 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0, 0);
    }
    Device->ReleaseClip();
    Device->EndPaint();
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x100048a0, NCCheckBox, OnPaint)

// 0x10004600 (vslot 61)
INT NCCheckBox::OnLButtonDown(WPARAM wParam, LPARAM lParam)
{
    if (!bDisabled)
    {
        bChecked = !bChecked;
        NotifyParent(bChecked);
    }
    return 1;
}
NW_HOOK_METHOD(0x10004600, NCCheckBox, OnLButtonDown)

NW_CHECK_VTABLE(NCCheckBox, 0x101090f8, 68)
