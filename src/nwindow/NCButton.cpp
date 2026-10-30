/*=============================================================================
    NCButton.cpp - NCButton, NCExButton, NCPushButton, NCRadioButton.
    Stock: 0x10001000 .. 0x10002550, plus WindowProc 0x100051f0 / 0x100301f0 and
    OnDestroy 0x100189e0 (bodies the linker folded with other classes).
=============================================================================*/
#include "nwindow/NCButton.h"
#include "nwindow/NCGDevice.h"
#include "nwindow/NConsoleWnd.h"

#include <wchar.h>

// interface sounds (FL2NetNotify::OnInterfacePlaySound)
enum { SOUND_Click = 0, SOUND_ClickDisabled = 2 };

// timer of the push button auto-repeat
enum { TIMER_PushRepeat = 0xFF000000 };

/*-----------------------------------------------------------------------------
    NCButton.
-----------------------------------------------------------------------------*/

// 0x10002340 - C2 adds the tool tip data (ToolTip, constructed first) and makes Text an FString
NCButton::NCButton()
{
    TextID = -1;
    ToolTipID = -1;
    bReleased = 1;
    NormalTex = NULL;
    PushedTex = NULL;
    bAcceptKey = 0;
    NormalTexH = 0;
    NormalTexW = 0;
    PushedTexH = 0;
    PushedTexW = 0;
    bMouseOver = 0;
    bToolTipAbove = 1;
    TextOffsetX = 0;
    TextOffsetY = 0;
    ToolTipTex = FindTexture(TEXT("L2UI.tooltip.mini-tool01"));
}
NW_HOOK_CTOR(0x10002340, NCButton)

// 0x10002440 - the textures are not checked: FindTexture falls back to NWindow.BlackTexture
NCButton::NCButton(const TCHAR* NormalTexName, const TCHAR* PushedTexName, UBOOL bInAcceptKey)
{
    bReleased = 1;
    NormalTex = FindTexture(NormalTexName);
    NormalTexW = NormalTex->USize;
    NormalTexH = NormalTex->VSize;
    PushedTex = FindTexture(PushedTexName);
    PushedTexW = PushedTex->USize;
    PushedTexH = PushedTex->VSize;
    bAcceptKey = bInAcceptKey;
    bMouseOver = 0;
    bToolTipAbove = 1;
    TextID = -1;
    ToolTipID = -1;
    TextOffsetX = 0;
    TextOffsetY = 0;
    ToolTipTex = FindTexture(TEXT("L2UI.tooltip.mini-tool01"));
}
NW_HOOK_CTOR_ARGS(0x10002440, NCButton, (const TCHAR* A, const TCHAR* B, UBOOL C), (A, B, C))

// 0x10002650 - ConditionalDestroy, ~FString (Text), ~NToolTipInfo (ToolTip), ~NControl
NW_HOOK_DTOR(0x10002650, NCButton)

// 0x10090b60 (vslot 25) - one body for the buttons (C2 dropped the enabled test)
INT NCButton::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    return NControl::WindowProc(Msg, wParam, lParam) != 0;
}
NW_HOOK_METHOD(0x10090b60, NCButton, WindowProc)

// 0x10001140 (vslot 37) - C2 guards it and empties the label
INT NCButton::OnDestroy()
{
    guard(NCButton::OnDestroy);
    NControl::OnDestroy();
    Text.Empty();
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x10001140, NCButton, OnDestroy)

// 0x10001ca0 (vslot 55) - texture stretched over the button, label centered (grey when
// disabled). Width and Height are halved unsigned, as in the stock code.
INT NCButton::OnPaint(NCGDevice* Device)
{
    guard(NCButton::OnPaint);
    Device->BeginPaint(ScreenX, ScreenY);
    Device->SetClip(ScreenX, ScreenY, Width, Height);
    if (bReleased)
        Device->DrawTexture(0, 0, Width, Height, 0, 0, Width, Height, NormalTex);
    else
        Device->DrawTexture(0, 0, Width, Height, 0, 0, Width, Height, PushedTex);

    SIZE Size;
    const TCHAR* Label = NULL;
    if (TextID >= 0)
    {
        if (*SysString(TextID))
        {
            NCGDevice::GetTextExtent(&Size, *SysString(TextID), 0, 0, 0, 0, 0, L2FONT_Normal);
            Label = *SysString(TextID);
        }
    }
    else if (Text.Num() && *Text.GetCharData())
    {
        NCGDevice::GetTextExtent(&Size, *Text, 0, 0, 0, 0, 0, L2FONT_Normal);
        Label = *Text;
    }
    if (Label)
        Device->DrawText(((DWORD)Width >> 1) + TextOffsetX - Size.cx / 2,
                         ((DWORD)Height >> 1) + TextOffsetY - Size.cy / 2,
                         IsEnabled() ? 0xFFFFFFFF : 0xFF7E7E7E, Label, 0, 0, 0, L2FONT_Normal, 0, 0,
                         0, 0, 0, 0, 0);
    Device->ReleaseClip();
    Device->EndPaint();
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x10001ca0, NCButton, OnPaint)

// 0x10002710 (vslot 56) - C2 draws the tool tip with the window's skin: the text is measured (the result is not
// used), then the skin gets a copy of ToolTip by value and the button's rectangle
INT NCButton::OnDrawToolTip(NCGDevice* Device)
{
    Device->BeginPaint(ScreenX, ScreenY);
    Device->SetClip(ScreenX, ScreenY, Width, Height);
    if (ToolTipID >= 0)
    {
        if (*SysString(ToolTipID) && bMouseOver)
        {
            SIZE Size;
            NCGDevice::GetTextExtent(&Size, *SysString(ToolTipID), 0, 0, 0, 0, 0, L2FONT_Normal);
            if (ToolTipSkin)
            {
                INT OffsetX = 0;
                INT OffsetY = 0;
                NToolTipRect Wnd;
                Wnd.X = ScreenX;
                Wnd.Y = ScreenY;
                Wnd.W = Width;
                Wnd.H = Height;
                ToolTipSkin->Draw(Device, ToolTip, &OffsetX, &OffsetY, Wnd);
            }
        }
    }
    Device->ReleaseClip();
    Device->EndPaint();
    return 1;
}
NW_HOOK_METHOD(0x10002710, NCButton, OnDrawToolTip)

// 0x100011d0 (vslot 58) - Enter / Space click it while it has the focus
INT NCButton::OnKeyDown(WPARAM wParam, LPARAM lParam)
{
    if ((wParam == VK_RETURN || wParam == VK_SPACE) && bAcceptKey == 1)
    {
        PlayInterfaceSound(IsEnabled() ? SOUND_Click : SOUND_ClickDisabled);
        NotifyParent(0);
    }
    return 1;
}
NW_HOOK_METHOD(0x100011d0, NCButton, OnKeyDown)

// 0x100010b0 (vslot 60) - dragging out of the button releases it without a click
INT NCButton::OnMouseMove(WPARAM wParam, LPARAM lParam)
{
    if ((wParam & MK_LBUTTON) && IsInWindow(LOWORD(lParam), HIWORD(lParam)))
        bReleased = 0;
    else
        bReleased = 1;
    bMouseOver = PtInRect(LOWORD(lParam), HIWORD(lParam), ScreenX, ScreenY, Width, Height);
    return 1;
}
NW_HOOK_METHOD(0x100010b0, NCButton, OnMouseMove)

// 0x10001000 (vslot 61)
INT NCButton::OnLButtonDown(WPARAM wParam, LPARAM lParam)
{
    if (IsInWindow(LOWORD(lParam), HIWORD(lParam)))
    {
        bReleased = 0;
        PlayInterfaceSound(IsEnabled() ? SOUND_Click : SOUND_ClickDisabled);
    }
    return 1;
}
NW_HOOK_METHOD(0x10001000, NCButton, OnLButtonDown)

// 0x10001060 (vslot 62) - the click: released inside, enabled
INT NCButton::OnLButtonUp(WPARAM wParam, LPARAM lParam)
{
    if (IsInWindow(LOWORD(lParam), HIWORD(lParam)) && IsEnabled())
        NotifyParent(NotifyID == -1 ? ID : NotifyID);
    bReleased = 1;
    return 1;
}
NW_HOOK_METHOD(0x10001060, NCButton, OnLButtonUp)

// 0x10001220 - kept only if the string exists (FString::operator* never returns NULL, so
// always, in practice). C2 reads the SysString table at +0x4B4E8 of the game data (SysString helper)
void NCButton::SetTextID(INT InID)
{
    if (*SysString(InID))
        TextID = InID;
}
NW_HOOK_METHOD(0x10001220, NCButton, SetTextID)

// 0x10001250 - C2: Text is an FString
void NCButton::SetText(const TCHAR* InText)
{
    if (InText && *InText)
    {
        TextID = -1;
        Text = InText;
    }
}
NW_HOOK_METHOD(0x10001250, NCButton, SetText)

// 0x10001280
UBOOL NCButton::SetNormalTex(const TCHAR* Name)
{
    if (Name && *Name)
    {
        NormalTex = FindTexture(Name);
        NormalTexW = NormalTex->USize;
        NormalTexH = NormalTex->VSize;
        return 1;
    }
    NormalTex = NULL;
    return 1;
}
NW_HOOK_METHOD(0x10001280, NCButton, SetNormalTex)

// 0x100012d0 - unlike SetNormalTex, an empty name still looks the texture up
UBOOL NCButton::SetPushedTex(const TCHAR* Name)
{
    if (Name)
    {
        PushedTex = FindTexture(Name);
        PushedTexW = PushedTex->USize;
        PushedTexH = PushedTex->VSize;
        return 1;
    }
    PushedTex = NULL;
    return 1;
}
NW_HOOK_METHOD(0x100012d0, NCButton, SetPushedTex)

// 0x10001320 - C2 also gives the text to the tool tip data, wrapped at 0x400
void NCButton::SetToolTip(INT InID, UBOOL bAbove)
{
    if (InID >= 0)
    {
        if (*SysString(InID))
        {
            bToolTipAbove = bAbove;
            ToolTipID = InID;
            ToolTip.SetText(*SysString(InID), 0x400);
        }
    }
}
NW_HOOK_METHOD(0x10001320, NCButton, SetToolTip)

NW_CHECK_VTABLE(NCButton, 0x101083c8, 68)

/*-----------------------------------------------------------------------------
    NCExButton: reports press (1) and release (0).
-----------------------------------------------------------------------------*/

// 0x10002830
NCExButton::NCExButton()
{
}
NW_HOOK_CTOR(0x10002830, NCExButton)

// 0x100028d0
NCExButton::NCExButton(const TCHAR* NormalTexName, const TCHAR* PushedTexName, UBOOL bInAcceptKey)
    : NCButton(NormalTexName, PushedTexName, bInAcceptKey)
{
}
NW_HOOK_CTOR_ARGS(0x100028d0, NCExButton, (const TCHAR* A, const TCHAR* B, UBOOL C), (A, B, C))

// 0x10002880
NW_HOOK_DTOR(0x10002880, NCExButton)

// 0x1003a790 (vslot 25) - one body for NCExButton, NCPushButton and NCRadioButton
INT NCExButton::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    return NCButton::WindowProc(Msg, wParam, lParam) != 0;
}
NW_HOOK_METHOD(0x1003a790, NCExButton, WindowProc)

// 0x10001390 (vslot 61) - reports even when the press is outside the button
INT NCExButton::OnLButtonDown(WPARAM wParam, LPARAM lParam)
{
    if (IsInWindow(LOWORD(lParam), HIWORD(lParam)))
        bReleased = 0;
    NotifyParent(1);
    return 1;
}
NW_HOOK_METHOD(0x10001390, NCExButton, OnLButtonDown)

// 0x100013d0 (vslot 62)
INT NCExButton::OnLButtonUp(WPARAM wParam, LPARAM lParam)
{
    NotifyParent(0);
    bReleased = 1;
    return 1;
}
NW_HOOK_METHOD(0x100013d0, NCExButton, OnLButtonUp)

NW_CHECK_VTABLE(NCExButton, 0x101084d8, 68)

/*-----------------------------------------------------------------------------
    NCPushButton: reports on press, then every RepeatDelay ms while held.
-----------------------------------------------------------------------------*/

// 0x100029d0
NCPushButton::NCPushButton()
{
    bHeld = 0;
    RepeatDelay = 100;
    bAutoRepeat = 1;
}
NW_HOOK_CTOR(0x100029d0, NCPushButton)

// 0x10002a80 - RepeatDelay and bAutoRepeat stay as allocated (zero): no auto-repeat
NCPushButton::NCPushButton(const TCHAR* NormalTexName, const TCHAR* PushedTexName)
{
    bHeld = 0;
    SetNormalTex(NormalTexName);
    SetPushedTex(PushedTexName);
}
NW_HOOK_CTOR_ARGS(0x10002a80, NCPushButton, (const TCHAR* A, const TCHAR* B), (A, B))

// 0x10002a30
NW_HOOK_DTOR(0x10002a30, NCPushButton)

// vslot 25: the stock slot holds NCExButton's body above (0x1003a790, one body for the three)
INT NCPushButton::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    return NCButton::WindowProc(Msg, wParam, lParam) != 0;
}

// 0x10001560 (vslot 35)
INT NCPushButton::OnTimer(INT TimerID)
{
    if (TimerID == (INT)TIMER_PushRepeat && bHeld)
        NotifyParent(0);
    return 1;
}
NW_HOOK_METHOD(0x10001560, NCPushButton, OnTimer)

// 0x10001590 (vslot 37) - guarded in C2 (the NCRadioButton one, 0x10001780, is a bare jump)
INT NCPushButton::OnDestroy()
{
    guard(NCPushButton::OnDestroy);
    NCButton::OnDestroy();
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x10001590, NCPushButton, OnDestroy)

// 0x100014f0 (vslot 60)
INT NCPushButton::OnMouseMove(WPARAM wParam, LPARAM lParam)
{
    if ((wParam & MK_LBUTTON) && IsFocused() && IsInWindow(LOWORD(lParam), HIWORD(lParam)))
    {
        bHeld = 1;
        bReleased = 0;
        return 1;
    }
    bHeld = 0;
    bReleased = 1;
    return 1;
}
NW_HOOK_METHOD(0x100014f0, NCPushButton, OnMouseMove)

// 0x10001440 (vslot 61) - no hit test; NotifyID is sent as is (-1 is not replaced by ID)
INT NCPushButton::OnLButtonDown(WPARAM wParam, LPARAM lParam)
{
    bReleased = 0;
    bHeld = 1;
    PlayInterfaceSound(IsEnabled() ? SOUND_Click : SOUND_ClickDisabled);
    NotifyParent(NotifyID);
    if (bAutoRepeat)
        SetTimer(TIMER_PushRepeat, RepeatDelay, NULL);
    return 1;
}
NW_HOOK_METHOD(0x10001440, NCPushButton, OnLButtonDown)

// 0x100014b0 (vslot 62)
INT NCPushButton::OnLButtonUp(WPARAM wParam, LPARAM lParam)
{
    bReleased = 1;
    bHeld = 0;
    if (bAutoRepeat)
        KillTimer(TIMER_PushRepeat);
    ReleaseFocus();
    return 1;
}
NW_HOOK_METHOD(0x100014b0, NCPushButton, OnLButtonUp)

NW_CHECK_VTABLE(NCPushButton, 0x101086f8, 68)

/*-----------------------------------------------------------------------------
    NCRadioButton: stays down once clicked and reports Value.
-----------------------------------------------------------------------------*/

// 0x10002ae0
NCRadioButton::NCRadioButton()
{
    CreateHeight = 0;
    CreateWidth = 0;
    bReleased = 1;
    Value = -1;
}
NW_HOOK_CTOR(0x10002ae0, NCRadioButton)

// 0x10002ba0 - sets the textures twice (base constructor, then the setters), as the stock code
NCRadioButton::NCRadioButton(const TCHAR* NormalTexName, const TCHAR* PushedTexName)
    : NCButton(NormalTexName, PushedTexName, 0)
{
    Value = -1;
    CreateHeight = 0;
    CreateWidth = 0;
    SetNormalTex(NormalTexName);
    SetPushedTex(PushedTexName);
}
NW_HOOK_CTOR_ARGS(0x10002ba0, NCRadioButton, (const TCHAR* A, const TCHAR* B), (A, B))

// 0x10002b50
NW_HOOK_DTOR(0x10002b50, NCRadioButton)

// vslot 25: the stock slot holds NCExButton's body above (0x1003a790, one body for the three)
INT NCRadioButton::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    return NCButton::WindowProc(Msg, wParam, lParam) != 0;
}

// 0x10001750 (vslot 36)
INT NCRadioButton::OnCreate()
{
    NCButton::OnCreate();
    CreateWidth = Width;
    CreateHeight = Height;
    return 1;
}
NW_HOOK_METHOD(0x10001750, NCRadioButton, OnCreate)

// 0x10001780 (vslot 37) - a bare jump to NCButton's, folded in the stock image
INT NCRadioButton::OnDestroy()
{
    NCButton::OnDestroy();
    return 1;
}
NW_HOOK_METHOD(0x10001780, NCRadioButton, OnDestroy)

// 0x10001f90 (vslot 56) - the tip covers the button itself (C2 checks ToolTipID >= 0)
INT NCRadioButton::OnDrawToolTip(NCGDevice* Device)
{
    Device->BeginPaint(ScreenX, ScreenY);
    Device->SetClip(ScreenX, ScreenY, Width, Height);
    if (ToolTipID >= 0)
    {
        if (*SysString(ToolTipID) && bMouseOver)
        {
            SIZE Size;
            NCGDevice::GetTextExtent(&Size, *SysString(ToolTipID), 0, 0, 0, 0, 0, L2FONT_Normal);
            Device->SetClip(ScreenX, ScreenY, Size.cx, Size.cy);
            if (ToolTipTex)
                Device->DrawStretchedTex(0, 0, Size.cx, Size.cy, ToolTipTex);
            Device->DrawText(0, 0, 0xFFFFFFFF, *SysString(ToolTipID), 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0,
                             0, 0, 0);
            Device->ReleaseClip();
        }
    }
    Device->ReleaseClip();
    Device->EndPaint();
    return 1;
}
NW_HOOK_METHOD(0x10001f90, NCRadioButton, OnDrawToolTip)

// 0x100016f0 (vslot 60) - C2: only while enabled
INT NCRadioButton::OnMouseMove(WPARAM wParam, LPARAM lParam)
{
    if (IsEnabled())
        bMouseOver = PtInRect(LOWORD(lParam), HIWORD(lParam), ScreenX, ScreenY, Width, Height);
    return 1;
}
NW_HOOK_METHOD(0x100016f0, NCRadioButton, OnMouseMove)

// 0x10001610 (vslot 61) - a button without a label never reports. C2: nothing while disabled; an empty
// Text is looked at without checking that it has a buffer (as the stock code does)
INT NCRadioButton::OnLButtonDown(WPARAM wParam, LPARAM lParam)
{
    if (!IsEnabled())
        return 1;
    if (!bReleased)
    {
        if (bToggle)
            bReleased = 1;
        return 1;
    }
    if (TextID >= 0)
    {
        if (*SysString(TextID) && !bToggle)
            NotifyParent(Value);
    }
    else if (*Text.GetCharData() && !bToggle)
        NotifyParent(Value);
    bReleased = 0;
    return 1;
}
NW_HOOK_METHOD(0x10001610, NCRadioButton, OnLButtonDown)

// 0x100016c0 (vslot 62) - C2: only while enabled
INT NCRadioButton::OnLButtonUp(WPARAM wParam, LPARAM lParam)
{
    if (IsEnabled() && bToggle)
        ReleaseFocus();
    return 1;
}
NW_HOOK_METHOD(0x100016c0, NCRadioButton, OnLButtonUp)

NW_CHECK_VTABLE(NCRadioButton, 0x10108808, 68)
