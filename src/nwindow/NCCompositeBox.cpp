/*=============================================================================
    NCCompositeBox.cpp - IME composition window. Stock (C2): 0x10003090 .. 0x10003fff.
=============================================================================*/
#include "nwindow/NCCompositeBox.h"
#include "nwindow/NCGDevice.h"

// 0x10003ac0
NCCompositeBox::NCCompositeBox()
    : BackTex(NULL)
{
    CaretPos = 0;
    bShowCaret = 0;
    FirstChar = 0;
}
NW_HOOK_CTOR(0x10003ac0, NCCompositeBox)

// 0x10003bd0
NCCompositeBox::NCCompositeBox(INT W, INT H, INT X, INT Y, UBOOL bInShowCaret)
{
    bShowCaret = bInShowCaret;
    FirstChar = 0;
    CaretPos = 0;
    SetWindowRect(X, Y, W, H);
}
NW_HOOK_CTOR_ARGS(0x10003bd0, NCCompositeBox, (INT A, INT B, INT C, INT D, UBOOL E), (A, B, C, D, E))

// 0x10003b60
NW_HOOK_DTOR(0x10003b60, NCCompositeBox)

// 0x10043ef0 (vslot 25) - the stock body is NControl's (C2: NCWnd's alone, no enabled test)
INT NCCompositeBox::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    return NCWnd::WindowProc(Msg, wParam, lParam) != 0;
}

// 0x10003090 (vslot 36)
INT NCCompositeBox::OnCreate()
{
    NCWnd::OnCreate();
    BackTex = FindTexture(TEXT("NWindow.BlackTexture"));
    Text.Empty();
    return 1;
}
NW_HOOK_METHOD(0x10003090, NCCompositeBox, OnCreate)

// 0x10003130 (vslot 37) - guarded in C2
INT NCCompositeBox::OnDestroy()
{
    guard(NCCompositeBox::OnDestroy);
    NCWnd::OnDestroy();
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x10003130, NCCompositeBox, OnDestroy)

// 0x10003c60 (vslot 55) - the width drawn is unsigned in the stock code
INT NCCompositeBox::OnPaint(NCGDevice* Device)
{
    guard(NCCompositeBox::OnPaint);
    Device->BeginPaint(ScreenX, ScreenY);
    Device->SetClip(ScreenX, ScreenY, Width, Height);
    if (BackTex)
        Device->DrawStretchedTex(0, 0, Width, Height, BackTex);
    DWORD X = 0;
    INT i;
    for (i = FirstChar; i < Text.Len(); i++)
    {
        if (i == CaretPos && bShowCaret)
            Device->DrawText(X + 1, 3, 0xFFFFFFFF, TEXT("|"), 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0, 0);
        X += Device->DrawText(X + 3, 3, 0xFFFFFFFF, *Text.Mid(i, 1), 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0,
                              0, 0);
        if (X > 97)
            break;
    }
    if (i == CaretPos && bShowCaret)
        Device->DrawText(X + 1, 3, 0xFFFFFFFF, TEXT("|"), 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0, 0);
    Device->ReleaseClip();
    Device->EndPaint();
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x10003c60, NCCompositeBox, OnPaint)

// 0x100030c0
void NCCompositeBox::CaretLeft()
{
    if (CaretPos > 0)
        CaretPos--;
    if (FirstChar > CaretPos)
        FirstChar = CaretPos;
}
NW_HOOK_METHOD(0x100030c0, NCCompositeBox, CaretLeft)

// 0x10003e40 - measures with a device created (and destroyed) just for the call, as in the
// stock code
void NCCompositeBox::CaretRight()
{
    if (Text.Len() > CaretPos)
        CaretPos++;
    NCGDevice* Device = new NCGDevice;
    SIZE Size;
    Device->GetTextExtent(&Size, *Text.Mid(FirstChar, CaretPos - FirstChar), 0, 0, 0, 0, 0, L2FONT_Normal);
    while (Size.cx > 97)
    {
        FirstChar++;
        Device->GetTextExtent(&Size, *Text.Mid(FirstChar, CaretPos - FirstChar), 0, 0, 0, 0, 0,
                              L2FONT_Normal);
    }
    delete Device;
}
NW_HOOK_METHOD(0x10003e40, NCCompositeBox, CaretRight)

// 0x10003fb0 - scrolls back to the start, then forward until the caret fits
void NCCompositeBox::SetText(const TCHAR* InText)
{
    if (!InText)
        return;
    FirstChar = 0;
    Text = FString(InText);
    NCGDevice* Device = new NCGDevice;
    SIZE Size;
    Device->GetTextExtent(&Size, *Text.Mid(FirstChar, CaretPos - FirstChar), 0, 0, 0, 0, 0, L2FONT_Normal);
    while (Size.cx > 97)
    {
        FirstChar++;
        Device->GetTextExtent(&Size, *Text.Mid(FirstChar, CaretPos - FirstChar), 0, 0, 0, 0, 0,
                              L2FONT_Normal);
    }
    delete Device;
}
NW_HOOK_METHOD(0x10003fb0, NCCompositeBox, SetText)

// 0x100030f0 - a copy
FString NCCompositeBox::GetText()
{
    return Text;
}
NW_HOOK_METHOD(0x100030f0, NCCompositeBox, GetText)

// 0x10003120
void NCCompositeBox::SetShowCaret(UBOOL bShow)
{
    bShowCaret = bShow;
}
NW_HOOK_METHOD(0x10003120, NCCompositeBox, SetShowCaret)

NW_CHECK_VTABLE(NCCompositeBox, 0x10108d08, 67)
