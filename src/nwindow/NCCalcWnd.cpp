/*=============================================================================
    NCCalcWnd.cpp - the calculator window (C2 only). Stock: 0x100de690 .. 0x100de71a (WindowProc,
    SetFocus), 0x100decb0 .. 0x100df12f, 0x100df450 .. 0x100df47f and 0x100df550 .. 0x100df872.
=============================================================================*/
#include "nwindow/NCCalcWnd.h"
#include "nwindow/NCButton.h"
#include "nwindow/NCCalcEditCtrl.h"
#include "nwindow/NCFrameCtrl.h"
#include "nwindow/NCGDevice.h"

// ids (names ours)
enum
{
    NID_CALC_CLOSE = (INT)0xFF01FF00,
    NID_CALC_KEY = (INT)0xFF01FF01,      // NotifyID = the key
    NID_CALC_DISPLAY = (INT)0xFF01FF02,
};
enum { SYSSTR_CalcClose = 646 };         // the close button's text (name ours)

#if NW_HOOK_MODE
// the textures of the keys: pairs of names (up, down), 19 of them, in the stock image
#define GCalcKeyTextures (reinterpret_cast<const TCHAR* const (*)[2]>(::nw::Addr(0x1018D4C0)))
#else
extern const TCHAR* const GCalcKeyTextures[][2];
#endif

// a child window goes (repeated inline in the stock OnDestroys)
template <class T> static FORCEINLINE void DestroyChild(NCWnd* Parent, T*& Child)
{
    if (Child)
    {
        Parent->RemoveChildWindow(Child->Entry);
        delete Child;
        Child = NULL;
    }
}

// 0x100decb0 - the controls come with OnCreate
NCCalcWnd::NCCalcWnd()
{
}
NW_HOOK_CTOR(0x100decb0, NCCalcWnd)

// 0x100ded00
NW_HOOK_DTOR(0x100ded00, NCCalcWnd)

// 0x100de690 (vslot 25)
INT NCCalcWnd::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (NCWnd::WindowProc(Msg, wParam, lParam))
        return 1;
    if (Msg == NWM_COMMAND)
    {
        if ((INT)wParam == NID_CALC_CLOSE)
            return OnClose() != 0;
        if ((INT)wParam == NID_CALC_KEY)
            return OnKey(lParam) != 0;
    }
    return 0;
}
NW_HOOK_METHOD(0x100de690, NCCalcWnd, WindowProc)

// 0x100de710 (vslot 29) - the display takes the focus
void NCCalcWnd::SetFocus()
{
    Display->SetFocus();
}
NW_HOOK_METHOD(0x100de710, NCCalcWnd, SetFocus)

// 0x100df100 - the display's Reset
void NCCalcWnd::Reset()
{
    Display->Reset();
}
NW_HOOK_METHOD(0x100df100, NCCalcWnd, Reset)

// 0x100dee50 (vslot 67) - a key of the keypad: the character it stands for goes to the display
// (WM_CHAR), the backspace as WM_KEYDOWN, "00" as two zeros; the two clears act on the display
// directly. The display has the focus afterwards.
INT NCCalcWnd::OnKey(INT Key)
{
    switch (Key)
    {
    case 0:
        Display->SendTo(Display->Entry, WM_CHAR, '7', 0);
        break;
    case 1:
        Display->SendTo(Display->Entry, WM_CHAR, '8', 0);
        break;
    case 2:
        Display->SendTo(Display->Entry, WM_CHAR, '9', 0);
        break;
    case 3:
        Display->SendTo(Display->Entry, WM_CHAR, '+', 0);
        break;
    case 4:
        Display->ShowZero();
        Display->SetFocus();
        return 1;
    case 5:
        Display->SendTo(Display->Entry, WM_CHAR, '4', 0);
        break;
    case 6:
        Display->SendTo(Display->Entry, WM_CHAR, '5', 0);
        break;
    case 7:
        Display->SendTo(Display->Entry, WM_CHAR, '6', 0);
        break;
    case 8:
        Display->SendTo(Display->Entry, WM_CHAR, '-', 0);
        break;
    case 9:
        Display->Reset();
        Display->SetFocus();
        return 1;
    case 10:
        Display->SendTo(Display->Entry, WM_CHAR, '1', 0);
        break;
    case 11:
        Display->SendTo(Display->Entry, WM_CHAR, '2', 0);
        break;
    case 12:
        Display->SendTo(Display->Entry, WM_CHAR, '3', 0);
        break;
    case 13:
        Display->SendTo(Display->Entry, WM_CHAR, '*', 0);
        break;
    case 14:
        Display->SendTo(Display->Entry, WM_KEYDOWN, VK_BACK, 0);
        break;
    case 16:
        Display->SendTo(Display->Entry, WM_CHAR, '0', 0);
        // falls through: the second zero
    case 15:
        Display->SendTo(Display->Entry, WM_CHAR, '0', 0);
        break;
    case 17:
        Display->SendTo(Display->Entry, WM_CHAR, '/', 0);
        break;
    case 18:
        Display->SendTo(Display->Entry, WM_CHAR, '=', 0);
        break;
    }
    Display->SetFocus();
    return 1;
}
NW_HOOK_METHOD(0x100dee50, NCCalcWnd, OnKey)

// 0x100df450 (vslot 68) - closing empties the calculator
INT NCCalcWnd::OnClose()
{
    Reset();
    HideWindow();
    return 1;
}
NW_HOOK_METHOD(0x100df450, NCCalcWnd, OnClose)

// 0x100df470 (vslot 43) - it opens empty
void NCCalcWnd::ShowWindow()
{
    NCWnd::ShowWindow();
    Reset();
}
NW_HOOK_METHOD(0x100df470, NCCalcWnd, ShowWindow)

// 0x100df550 (vslot 36) - the title bar, the display and the keypad in rows of five (the
// "00" key is wider and the last row has four keys), then the close button under them
INT NCCalcWnd::OnCreate()
{
    INT KeyW = 32;
    INT X;
    BackTex = FindTexture(TEXT("l2ui.calcwnd.calcwnd_back"));

    FrameCtrl = new NCFrameCtrl(1);
    FrameCtrl->SetWindowRect(0, 0, 240, 21);
    CreateChild(FrameCtrl, NWS_VISIBLE);

    Display = new NCCalcEditCtrl();
    Display->ID = NID_CALC_DISPLAY;
    Display->SetWindowRect(37, 42, 166, 16);
    CreateChild(Display, NWS_VISIBLE);
    nw::Field<INT>(Display, 0x1CC) = 0;   // NCEditBox's Unknown1CC (new in C2)

    INT Y = 63;
    for (INT i = 0; i < 19; i++)
    {
        if (i == 0 || i == 5 || i == 10 || i == 15)
        {
            X = 37;
            if (i != 0)
                Y += 26;
        }
        else
            X = X + 2 + KeyW;
        KeyW = (INT)(((i != 16) - 1) & 0x22) + 32;
        Keys[i] = new NCButton(GCalcKeyTextures[i][0], GCalcKeyTextures[i][1], 0);
        Keys[i]->ID = NID_CALC_KEY;
        Keys[i]->NotifyID = i;
        Keys[i]->SetWindowRect(X, Y, KeyW, 24);
        CreateChild(Keys[i], NWS_VISIBLE);
    }

    CloseButton = new NCButton(TEXT("L2UI.control.DefaultButton"), TEXT("L2UI.control.DefaultButton_click"), 0);
    CloseButton->ID = NID_CALC_CLOSE;
    CloseButton->SetWindowRect(90, Y + 34, 67, 19);
    CreateChild(CloseButton, NWS_VISIBLE);
    CloseButton->SetTextID(SYSSTR_CalcClose);
    return 1;
}
NW_HOOK_METHOD(0x100df550, NCCalcWnd, OnCreate)

// 0x100ded50 (vslot 37) - guarded in C2; NCWnd's OnDestroy is not called
INT NCCalcWnd::OnDestroy()
{
    guard(NCCalcWnd::OnDestroy);
    DestroyChild(this, Display);
    for (INT i = 0; i < 19; i++)
        DestroyChild(this, Keys[i]);
    DestroyChild(this, CloseButton);
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x100ded50, NCCalcWnd, OnDestroy)

// 0x100df020 (vslot 55) - the panel under the title bar (240 x 194); guarded
INT NCCalcWnd::OnPaint(NCGDevice* Device)
{
    guard(NCCalcWnd::OnPaint);
    Device->BeginPaint(ScreenX, ScreenY);
    Device->SetClip(ScreenX, ScreenY, Width, Height);
    if (BackTex)
        Device->DrawTexture(0, 21, 240, 194, 0, 0, 240, 194, BackTex);
    Device->EndPaint();
    Device->ReleaseClip();
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x100df020, NCCalcWnd, OnPaint)

NW_CHECK_VTABLE(NCCalcWnd, 0x10128ce8, 69)
