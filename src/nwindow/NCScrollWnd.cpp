/*=============================================================================
    NCScrollWnd.cpp - vertically scrolling window. Stock: 0x10020f40 .. 0x10021790,
    plus the destructor (0x100168e0), OnPaint (0x10002830, folded) and OnScrollBar
    (0x100a0900).
=============================================================================*/
#include "nwindow/NCScrollWnd.h"
#include "nwindow/NCButton.h"
#include "nwindow/NCSliderBar.h"

// 0x10025590 - the controls and MinScrollY come with OnCreate / SetContentHeight
NCScrollWnd::NCScrollWnd()
{
    ScrollY = 0;
    ScrollX = 0;
    DeltaY = 0;
    DeltaX = 0;
    PageStep = 100;
    LineStep = 15;
    Unknown0C8 = 12;
}
NW_HOOK_CTOR(0x10025590, NCScrollWnd)

// 0x10019e10
NW_HOOK_DTOR(0x10019e10, NCScrollWnd)

// 0x100253d0 (vslot 25) - its own controls report here. C2: no enabled test
INT NCScrollWnd::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (NCWnd::WindowProc(Msg, wParam, lParam))
        return 1;
    if (Msg == NWM_COMMAND)
    {
        if ((WPARAM)ScrollUpID == wParam)
            return ScrollUp() != 0;
        if ((WPARAM)ScrollDownID == wParam)
            return ScrollDown() != 0;
        if ((WPARAM)ScrollBarID == wParam)
            return OnScrollBar((INT)lParam) != 0;
    }
    return 0;
}
NW_HOOK_METHOD(0x100253d0, NCScrollWnd, WindowProc)

// 0x10025610 (vslot 36) - up / down buttons and the slider along the right edge
INT NCScrollWnd::OnCreate()
{
    NCWnd::OnCreate();
    INT H = LocalH;
    INT W = LocalW;

    ScrollUpButton = new NCPushButton(TEXT("L2UI.NWindow.WindowScrollUp"),
                                      TEXT("L2UI.NWindow.WindowScrollUp"));
    ScrollUpID = GNextScrollCtrlID++;
    ScrollUpButton->ID = ScrollUpID;
    ScrollUpButton->SetWindowRect(W - 12, 0, 12, 12);
    CreateChild(ScrollUpButton, NWS_VISIBLE);

    ScrollDownButton = new NCPushButton(TEXT("L2UI.NWindow.WindowScrollDown"),
                                        TEXT("L2UI.NWindow.WindowScrollDown"));
    ScrollDownID = GNextScrollCtrlID++;
    ScrollDownButton->ID = ScrollDownID;
    ScrollDownButton->SetWindowRect(W - 12, H - 12, 12, 12);
    CreateChild(ScrollDownButton, NWS_VISIBLE);

    ScrollBar = new NCSliderBar(TEXT("L2UI.NWindow.WindowScrollBar1"),
                                TEXT("L2UI.NWindow.WindowScrollBar2"),
                                TEXT("L2UI.NWindow.WindowScrollBar3"));
    ScrollBarID = GNextScrollCtrlID++;
    ScrollBar->ID = ScrollBarID;
    ScrollBar->SetWindowRect(W - 12, 12, 12, H - 24);
    CreateChild(ScrollBar, NWS_VISIBLE);
    return 1;
}
NW_HOOK_METHOD(0x10025610, NCScrollWnd, OnCreate)

// 0x10025870 (vslot 37) - C2 guards it
INT NCScrollWnd::OnDestroy()
{
    guard(NCScrollWnd::OnDestroy);
    if (ScrollUpButton)
    {
        RemoveChildWindow(ScrollUpButton->Entry);
        delete ScrollUpButton;
        ScrollUpButton = NULL;
    }
    if (ScrollDownButton)
    {
        RemoveChildWindow(ScrollDownButton->Entry);
        delete ScrollDownButton;
        ScrollDownButton = NULL;
    }
    if (ScrollBar)
    {
        RemoveChildWindow(ScrollBar->Entry);
        delete ScrollBar;
        ScrollBar = NULL;
    }
    NCWnd::OnDestroy();
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x10025870, NCScrollWnd, OnDestroy)

// 0x10025470 (vslot 42) - the controls follow the right edge (size 0 x 0 keeps theirs)
void NCScrollWnd::SetSize(INT W, INT H)
{
    NCWnd::SetSize(W, H);
    ScrollUpButton->SetWindowRect(W - 12, 0, 0, 0);
    ScrollDownButton->SetWindowRect(W - 12, H - 12, 0, 0);
    ScrollBar->SetWindowRect(W - 12, 12, 12, H - 24);
}
NW_HOOK_METHOD(0x10025470, NCScrollWnd, SetSize)

// 0x100ab570 (vslot 55) - `return 1` shared with NCCandidateBox and others
NW_HOOK_METHOD(0x100ab570, NCScrollWnd, OnPaint)

// 0x10025970 (vslot 58) - the base handler runs twice, as in the stock code
INT NCScrollWnd::OnKeyDown(WPARAM wParam, LPARAM lParam)
{
    DeltaX = ScrollX;
    INT OldY = ScrollY;
    DeltaY = OldY;
    UBOOL bHandled = 0;
    switch (wParam)
    {
    case VK_PRIOR:
        if (ScrollY < 0)
        {
            ScrollY += PageStep;
            if (ScrollY > 0)
                ScrollY = 0;
        }
        bHandled = 1;
        break;
    case VK_NEXT:
        if (ScrollY > MinScrollY)
        {
            ScrollY -= PageStep;
            if (ScrollY < MinScrollY)
                ScrollY = MinScrollY;
        }
        bHandled = 1;
        break;
    case VK_UP:
        if (ScrollY < 0)
        {
            ScrollY += LineStep;
            if (ScrollY > 0)
                ScrollY = 0;
        }
        bHandled = 1;
        break;
    case VK_DOWN:
        if (ScrollY > MinScrollY)
        {
            ScrollY -= LineStep;
            if (ScrollY < MinScrollY)
                ScrollY = MinScrollY;
        }
        bHandled = 1;
        break;
    }
    if (ScrollY != OldY)
        ScrollBar->SetScrollPos(-ScrollY);
    DeltaY -= ScrollY;
    DeltaX -= ScrollX;
    NCWnd::OnKeyDown(wParam, lParam);
    return NCWnd::OnKeyDown(wParam, lParam) ? 1 : bHandled;
}
NW_HOOK_METHOD(0x10025970, NCScrollWnd, OnKeyDown)

// 0x100bf190 (vslot 66) - three lines per wheel notch (120)
INT NCScrollWnd::OnMouseWheel(WPARAM wParam, LPARAM lParam)
{
    SWORD Notches = (SWORD)((SWORD)HIWORD(wParam) / 120);
    DeltaX = ScrollX;
    DeltaY = ScrollY;
    if (Notches > 0)
    {
        if (ScrollY < 0)
        {
            ScrollY += LineStep * Notches * 3;
            if (ScrollY > 0)
                ScrollY = 0;
            ScrollBar->SetScrollPos(-ScrollY);
        }
    }
    else if (ScrollY > MinScrollY)
    {
        ScrollY += LineStep * Notches * 3;
        if (ScrollY < MinScrollY)
            ScrollY = MinScrollY;
        ScrollBar->SetScrollPos(-ScrollY);
    }
    DeltaX -= ScrollX;
    DeltaY -= ScrollY;
    return 1;
}
NW_HOOK_METHOD(0x100bf190, NCScrollWnd, OnMouseWheel)

// 0x10025ab0 (vslot 67)
INT NCScrollWnd::ScrollUp()
{
    if (ScrollY < 0)
    {
        DeltaX = ScrollX;
        DeltaY = ScrollY;
        ScrollY += LineStep;
        if (ScrollY > 0)
            ScrollY = 0;
        ScrollBar->SetScrollPos(-ScrollY);
        DeltaX -= ScrollX;
        DeltaY -= ScrollY;
        return 1;
    }
    DeltaY = 0;
    return 1;
}
NW_HOOK_METHOD(0x10025ab0, NCScrollWnd, ScrollUp)

// 0x10025b40 (vslot 68)
INT NCScrollWnd::ScrollDown()
{
    if (ScrollY > MinScrollY)
    {
        DeltaX = ScrollX;
        DeltaY = ScrollY;
        ScrollY -= LineStep;
        if (ScrollY < MinScrollY)
            ScrollY = MinScrollY;
        ScrollBar->SetScrollPos(-ScrollY);
        DeltaX -= ScrollX;
        DeltaY -= ScrollY;
        return 1;
    }
    DeltaY = 0;
    return 1;
}
NW_HOOK_METHOD(0x10025b40, NCScrollWnd, ScrollDown)

// 0x100bf130 (vslot 69) - the slider's value is the scroll position
INT NCScrollWnd::OnScrollBar(INT Value)
{
    DeltaX = ScrollX;
    DeltaY = ScrollY;
    ScrollY = -Value;
    DeltaX -= ScrollX;
    DeltaY -= ScrollY;
    return 1;
}
NW_HOOK_METHOD(0x100bf130, NCScrollWnd, OnScrollBar)

// 0x10025540 - MinScrollY = Height - content height (0 when it all fits)
void NCScrollWnd::SetContentHeight(INT ContentHeight)
{
    if (ContentHeight > Height)
    {
        MinScrollY = Height - ContentHeight;
        ScrollBar->SetScrollRange(ContentHeight, Height);
    }
    else
    {
        MinScrollY = 0;
        ScrollBar->SetScrollRange(0, Height);
    }
}
NW_HOOK_METHOD(0x10025540, NCScrollWnd, SetContentHeight)

// 0x100254d0 (name ours) - the slider and both scroll buttons follow one flag: shown, or hidden
void NCScrollWnd::ShowBars(INT bShow)
{
    if (bShow)
    {
        ScrollBar->ShowWindow();
        ScrollUpButton->ShowWindow();
        ScrollDownButton->ShowWindow();
        return;
    }
    ScrollBar->HideWindow();
    ScrollUpButton->HideWindow();
    ScrollDownButton->HideWindow();
}
NW_HOOK_METHOD(0x100254d0, NCScrollWnd, ShowBars)

// 0x100255e0 - the scroll buttons are created without auto-repeat (as NCListBox's)
void NCScrollWnd::SetRepeatDelay(INT Delay)
{
    if (ScrollUpButton && ScrollDownButton)
    {
        ScrollUpButton->RepeatDelay = Delay;
        ScrollDownButton->RepeatDelay = Delay;
    }
}
NW_HOOK_METHOD(0x100255e0, NCScrollWnd, SetRepeatDelay)

NW_CHECK_VTABLE(NCScrollWnd, 0x1010d6d8, 70)
