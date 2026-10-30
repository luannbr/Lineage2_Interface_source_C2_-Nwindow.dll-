/*=============================================================================
    NCHtmlButton.cpp - NCHtmlButton and NCHtmlPushButton, the buttons of the HTML pages.
    Stock: 0x1002fee0 .. 0x1002ff16, 0x10030840 .. 0x10030fb9 (WindowProc is a body the linker
    folded with the other buttons').
=============================================================================*/
#include "nwindow/NCHtmlButton.h"
#include "nwindow/NCHtmlViewer.h"
#include "nwindow/NCGDevice.h"

// the face and the label as NCButton::OnPaint draws them, the enabled state read from the
// field (the same code in both classes of the stock build)
static FORCEINLINE void DrawHtmlButton(NCButton* Button, NCGDevice* Device)
{
    if (Button->bReleased)
        Device->DrawTexture(0, 0, Button->Width, Button->Height, 0, 0, Button->Width,
                            Button->Height, Button->NormalTex);
    else
        Device->DrawTexture(0, 0, Button->Width, Button->Height, 0, 0, Button->Width,
                            Button->Height, Button->PushedTex);

    SIZE Size;
    const TCHAR* Label = NULL;
    // C2 (edited by the foundation lane when NCButton::Text became an FString and SysString moved to
    // +0x4B4E8 of the game data: the same label code as NCButton::OnPaint)
    if (Button->TextID >= 0)
    {
        if (*Button->SysString(Button->TextID))
        {
            NCGDevice::GetTextExtent(&Size, *Button->SysString(Button->TextID), 0, 0, 0, 0, 0, L2FONT_Normal);
            Label = *Button->SysString(Button->TextID);
        }
    }
    else if (Button->Text.Num() && *Button->Text.GetCharData())
    {
        NCGDevice::GetTextExtent(&Size, *Button->Text, 0, 0, 0, 0, 0, L2FONT_Normal);
        Label = *Button->Text;
    }
    if (Label)
        Device->DrawText(((DWORD)Button->Width >> 1) + Button->TextOffsetX - Size.cx / 2,
                         ((DWORD)Button->Height >> 1) + Button->TextOffsetY - Size.cy / 2,
                         Button->bEnabled ? 0xFFFFFFFF : 0xFF7E7E7E, Label, 0, 0, 0,
                         L2FONT_Normal, 0, 0, 0, 0, 0, 0, 0);
}

/*-----------------------------------------------------------------------------
    NCHtmlButton.
-----------------------------------------------------------------------------*/

// 0x1003de60
NCHtmlButton::NCHtmlButton()
{
}
NW_HOOK_CTOR(0x1003de60, NCHtmlButton)

// 0x1003df00
NCHtmlButton::NCHtmlButton(const TCHAR* NormalTexName, const TCHAR* PushedTexName,
                           const TCHAR* InCommand, UBOOL bInAcceptKey)
    : NCButton(NormalTexName, PushedTexName, bInAcceptKey)
{
    Command = InCommand;
}
NW_HOOK_CTOR_ARGS(0x1003df00, NCHtmlButton,
                  (const TCHAR* A, const TCHAR* B, const TCHAR* C, UBOOL D), (A, B, C, D))

// 0x1003df30
NCHtmlButton::NCHtmlButton(const TCHAR* NormalTexName, const TCHAR* PushedTexName)
    : NCButton(NormalTexName, PushedTexName, 0)
{
}
NW_HOOK_CTOR_ARGS(0x1003df30, NCHtmlButton, (const TCHAR* A, const TCHAR* B), (A, B))

// 0x1003deb0 (the hook was at 0x1003adf0, which is NCDynamicLinker's destructor: its scalar deleting
// destructor 0x1003adc0 is in that class's vtable 0x10110a70; the C1 -> C2 map had paired them)
NW_HOOK_DTOR(0x1003deb0, NCHtmlButton)

// 0x1003a790 (vslot 25) - the stock body is NCExButton's (hooked in NCButton.cpp); C2: no enabled test
INT NCHtmlButton::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    return NCButton::WindowProc(Msg, wParam, lParam) != 0;
}

// 0x1003b200 (vslot 55) - clipped to the viewer; the origin goes on before the clip
INT NCHtmlButton::OnPaint(NCGDevice* Device)
{
    guard(NCHtmlButton::OnPaint);
    Device->BeginPaint(ScreenX, ScreenY);
    NCWnd* Viewer = EnclosingHtmlViewer(this);
    if (Viewer)
        Device->SetClip(Viewer->ScreenX, Viewer->ScreenY, Viewer->Width, Viewer->Height);
    DrawHtmlButton(this, Device);
    Device->ReleaseClip();
    Device->EndPaint();
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x1003b200, NCHtmlButton, OnPaint)

// 0x1003a4c0 (vslot 62) - a release inside reports Command, even when disabled, without sound
INT NCHtmlButton::OnLButtonUp(WPARAM wParam, LPARAM lParam)
{
    if (IsInWindow(LOWORD(lParam), HIWORD(lParam)))
        NotifyParent((LPARAM)Command);
    bReleased = 1;
    return 1;
}
NW_HOOK_METHOD(0x1003a4c0, NCHtmlButton, OnLButtonUp)

NW_CHECK_VTABLE(NCHtmlButton, 0x10111178, 68)

/*-----------------------------------------------------------------------------
    NCHtmlPushButton.
-----------------------------------------------------------------------------*/

// 0x1003df60
NCHtmlPushButton::NCHtmlPushButton()
{
}
NW_HOOK_CTOR(0x1003df60, NCHtmlPushButton)

// 0x1003e000 - NCPushButton's default constructor, then the textures
NCHtmlPushButton::NCHtmlPushButton(const TCHAR* NormalTexName, const TCHAR* PushedTexName)
{
    SetNormalTex(NormalTexName);
    SetPushedTex(PushedTexName);
}
NW_HOOK_CTOR_ARGS(0x1003e000, NCHtmlPushButton, (const TCHAR* A, const TCHAR* B), (A, B))

// 0x1003dfb0
NW_HOOK_DTOR(0x1003dfb0, NCHtmlPushButton)

// 0x1003b540 (vslot 55) - clipped to the first ancestor that is not a control
INT NCHtmlPushButton::OnPaint(NCGDevice* Device)
{
    guard(NCHtmlPushButton::OnPaint);
    Device->BeginPaint(ScreenX, ScreenY);
    NCWnd* Owner = FirstNonControlParent(this);
    if (Owner)
        Device->SetClip(Owner->ScreenX, Owner->ScreenY, Owner->Width, Owner->Height);
    DrawHtmlButton(this, Device);
    Device->ReleaseClip();
    Device->EndPaint();
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x1003b540, NCHtmlPushButton, OnPaint)

NW_CHECK_VTABLE(NCHtmlPushButton, 0x10111288, 68)
