/*=============================================================================
    NCOptionWnd.cpp - the options window. Stock: 0x1007de70 .. 0x1007df36, 0x100817c0 ..
    0x100820fa, plus OnCancel 0x10059d00 (the body NCMatchOptionWnd and two other windows
    share).
=============================================================================*/
#include "nwindow/NCOptionWnd.h"
#include "nwindow/NCVideoOptionWnd.h"
#include "nwindow/NCAudioOptionWnd.h"
#include "nwindow/NCGameOptionWnd.h"
#include "nwindow/NCButton.h"
#include "nwindow/NCFrameCtrl.h"
#include "nwindow/NCTabControl.h"
#include "nwindow/NCGDevice.h"
#include "nwindow/NConsoleWnd.h"

// 0x100a43d0 - only BackTex is cleared
NCOptionWnd::NCOptionWnd()
{
    BackTex = NULL;
}
NW_HOOK_CTOR(0x100a43d0, NCOptionWnd)

// 0x100a4420
NW_HOOK_DTOR(0x100a4420, NCOptionWnd)

// 0x100a0790 (vslot 25) - C2 no longer refuses the messages of a disabled window here
INT NCOptionWnd::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (NCWnd::WindowProc(Msg, wParam, lParam))
        return 1;
    if (Msg == NWM_COMMAND)
    {
        if (wParam == NID_OPTION_OK)
            return OnOK() != 0;
        if (wParam == NID_OPTION_CANCEL)
            return OnCancel() != 0;
        if (wParam == NID_OPTION_APPLY)
            return OnApply() != 0;
    }
    return 0;
}
NW_HOOK_METHOD(0x100a0790, NCOptionWnd, WindowProc)

// a page of the window (repeated inline in the stock OnCreate); C2 gives the video and game pages
// the style bit 0x10 (not named yet), the audio page none
static FORCEINLINE void PlacePage(NCWnd* Parent, NCWnd* Page, DWORD Style)
{
    Page->SetWindowRect(0, 53, 208, 270);
    Parent->CreateChild(Page, Style);
}

// one button along the bottom (repeated inline in the stock OnCreate)
static FORCEINLINE NCButton* CreateOptionButton(NCWnd* Parent, INT ID, INT X, INT TextID)
{
    NCButton* Button = new NCButton(TEXT("L2UI.Control.DefaultButton"),
                                    TEXT("L2UI.Control.DefaultButton_click"), 0);
    Button->ID = ID;
    Button->SetWindowRect(X, 337, 67, 19);
    Parent->CreateChild(Button, NWS_VISIBLE);
    Button->SetTextID(TextID);
    return Button;
}

// 0x100a4470 (vslot 36)
INT NCOptionWnd::OnCreate()
{
    NCWnd::OnCreate();
    FrameCtrl = new NCFrameCtrl(1);
    FrameID = NID_OPTION_FRAME;
    FrameCtrl->ID = NID_OPTION_FRAME;
    FrameCtrl->SetWindowRect(0, 0, 240, 21);
    CreateChild(FrameCtrl, NWS_VISIBLE);
    BackTex = FindTexture(TEXT("L2UI.etcWnd.OptionBack"));

    VideoWnd = new NCVideoOptionWnd;
    PlacePage(this, VideoWnd, 0x10 | NWS_VISIBLE);
    AudioWnd = new NCAudioOptionWnd;
    PlacePage(this, AudioWnd, NWS_VISIBLE);
    GameWnd = new NCGameOptionWnd;
    PlacePage(this, GameWnd, 0x10 | NWS_VISIBLE);

    TabCtrl = new NCTabControl;
    TabCtrl->ID = NID_OPTION_TABS;
    TabCtrl->SetWindowRect(15, 23, 207, 16);
    CreateChild(TabCtrl, NWS_VISIBLE);
    TabCtrl->AddTab(0xCE, VideoWnd, 69, 16, TEXT("L2UI.NWindow.Tab2"), TEXT("L2UI.NWindow.Tab1"),
                    0, -1);
    TabCtrl->AddTab(0xCF, AudioWnd, 69, 16, TEXT("L2UI.NWindow.Tab2"), TEXT("L2UI.NWindow.Tab1"),
                    0, -1);
    TabCtrl->AddTab(0xD0, GameWnd, 69, 16, TEXT("L2UI.NWindow.Tab2"), TEXT("L2UI.NWindow.Tab1"),
                    0, -1);

    OKButton = CreateOptionButton(this, NID_OPTION_OK, 14, 0x8C);
    CancelButton = CreateOptionButton(this, NID_OPTION_CANCEL, 87, 0x8D);
    ApplyButton = CreateOptionButton(this, NID_OPTION_APPLY, 159, 0xDA);
    return 1;
}
NW_HOOK_METHOD(0x100a4470, NCOptionWnd, OnCreate)

// a child window goes (repeated inline in the stock OnDestroy)
template <class T> static FORCEINLINE void DestroyChild(NCWnd* Parent, T*& Child)
{
    if (Child)
    {
        Parent->RemoveChildWindow(Child->Entry);
        delete Child;
        Child = NULL;
    }
}

// 0x100a4ae0 (vslot 37)
INT NCOptionWnd::OnDestroy()
{
    DestroyChild(this, VideoWnd);
    DestroyChild(this, AudioWnd);
    DestroyChild(this, GameWnd);
    DestroyChild(this, OKButton);
    DestroyChild(this, CancelButton);
    DestroyChild(this, ApplyButton);
    DestroyChild(this, TabCtrl);
    DestroyChild(this, UnusedB4);
    NCWnd::OnDestroy();
    return 1;
}
NW_HOOK_METHOD(0x100a4ae0, NCOptionWnd, OnDestroy)

// 0x100a52a0 (vslot 43) - C2: each page first empties its lists and clears its boxes (ClearItems), then
// fills its controls (InitItems / InitVolumes); the tab control shows the current page last
void NCOptionWnd::ShowWindow()
{
    NCWnd::ShowWindow();
    VideoWnd->ClearItems();
    AudioWnd->ClearItems();
    GameWnd->ClearItems();
    VideoWnd->InitItems();
    AudioWnd->InitVolumes();
    GameWnd->InitItems();
    TabCtrl->ShowCurrentPage();
}
NW_HOOK_METHOD(0x100a52a0, NCOptionWnd, ShowWindow)

// 0x100a4a00 (vslot 55) - under the title bar; the clip is set before the origin
INT NCOptionWnd::OnPaint(NCGDevice* Device)
{
    guard(NCOptionWnd::OnPaint);
    Device->SetClip(ScreenX, ScreenY, Width, Height);
    Device->BeginPaint(ScreenX, ScreenY);
    if (BackTex)
        Device->DrawTexture(0, 21, 240, 344, 0, 0, 240, 344, BackTex);
    Device->EndPaint();
    Device->ReleaseClip();
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x100a4a00, NCOptionWnd, OnPaint)

// every page applies its settings (repeated in OnOK and OnApply)
static FORCEINLINE void ApplyPages(NCOptionWnd* Wnd)
{
    Wnd->VideoWnd->Apply();
    Wnd->AudioWnd->Apply();
    Wnd->GameWnd->Apply();
    GConfig->SetString(TEXT("FirstRun"), TEXT("FirstRun"), TEXT("1"), TEXT("Option.ini"));
}

// 0x100a4cc0 (vslot 67)
INT NCOptionWnd::OnOK()
{
    ApplyPages(this);
    HideWindow();
    return 1;
}
NW_HOOK_METHOD(0x100a4cc0, NCOptionWnd, OnOK)

// 0x100a0830 (vslot 68) - body shared with NCMatchOptionWnd's slot 63 and two other windows'
// (slots 64 and 72)
INT NCOptionWnd::OnCancel()
{
    HideWindow();
    return 1;
}
NW_HOOK_METHOD(0x100a0830, NCOptionWnd, OnCancel)

// 0x100a4d20 (vslot 69)
INT NCOptionWnd::OnApply()
{
    ApplyPages(this);
    return 1;
}
NW_HOOK_METHOD(0x100a4d20, NCOptionWnd, OnApply)

NW_CHECK_VTABLE(NCOptionWnd, 0x1011f270, 70)
