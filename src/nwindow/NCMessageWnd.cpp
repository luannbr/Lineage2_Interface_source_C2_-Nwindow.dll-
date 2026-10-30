/*=============================================================================
    NCMessageWnd.cpp - the console's one-line message window. Stock: 0x10077fa0 .. 0x100781d0.
=============================================================================*/
#include "nwindow/NCMessageWnd.h"
#include "nwindow/NCGDevice.h"

#include <wchar.h>

// 0x100987c0 - NCWnd's constructor is inlined in the stock code
NCMessageWnd::NCMessageWnd()
    : BackTex(NULL)
{
}
NW_HOOK_CTOR(0x100987c0, NCMessageWnd)

// 0x10098850
NW_HOOK_DTOR(0x10098850, NCMessageWnd)


// 0x10004e90 (vslot 24) - stock body shared with NControl (hooked there)
INT NCMessageWnd::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (Msg != WM_PAINT && !IsEnabled())
        return 0;
    return NCWnd::WindowProc(Msg, wParam, lParam) != 0;
}

// 0x100986e0 (vslot 36)
INT NCMessageWnd::OnCreate()
{
    NCWnd::OnCreate();
    BackTex = FindTexture(TEXT("sek.cbui141"));
    return 1;
}
NW_HOOK_METHOD(0x100986e0, NCMessageWnd, OnCreate)

// 0x10098710 (vslot 37) - C2 guards it; NCWnd's OnDestroy and nothing else
INT NCMessageWnd::OnDestroy()
{
    guard(NCMessageWnd::OnDestroy);
    NCWnd::OnDestroy();
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x10098710, NCMessageWnd, OnDestroy)

// 0x10098790 (C1 0x10077fd0) - the text emptied: a lone tail jump to FString::Empty on the member at +0x9C
void NCMessageWnd::ClearText()
{
    Text.Empty();
}
NW_HOOK_METHOD(0x10098790, NCMessageWnd, ClearText)

// 0x100987a0 - NULL keeps the text
void NCMessageWnd::SetText(const TCHAR* NewText)
{
    if (NewText)
        Text = NewText;
}
NW_HOOK_METHOD(0x100987a0, NCMessageWnd, SetText)

// 0x100988c0 (vslot 55) - the clip opens before BeginPaint and closes after EndPaint; the
// halves of the window are unsigned, those of the text signed, as in the stock code. The
// text is tested by its element count (0 only when never set or emptied)
INT NCMessageWnd::OnPaint(NCGDevice* Device)
{
    Device->SetClip(ScreenX, ScreenY, Width, Height);
    Device->BeginPaint(ScreenX, ScreenY);
    if (BackTex)
        Device->DrawTexture(0, 0, Width, Height, 0, 0, Width, Height, BackTex);
    SIZE Size;
    NCGDevice::GetTextExtent(&Size, *Text, 0, 0, 0, 0, 0, L2FONT_Normal);
    if (Text.Num() && Size.cx < Width)
        Device->DrawText(((DWORD)Width >> 1) - Size.cx / 2, ((DWORD)Height >> 1) - Size.cy / 2,
                         0xFFFFFFFF, *Text, 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0, 0);
    else if (Text.Num())
        Device->DrawText(50, 8, 0xFFFFFFFF, *Text, GSEKScreenX - 100, 0, 0, L2FONT_Normal, 0, 0, 0,
                         0, 0, 0, 0);
    Device->EndPaint();
    Device->ReleaseClip();
    return 1;
}
NW_HOOK_METHOD(0x100988c0, NCMessageWnd, OnPaint)

NW_CHECK_VTABLE(NCMessageWnd, 0x1011cce8, 67)
