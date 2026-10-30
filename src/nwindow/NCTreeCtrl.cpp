/*=============================================================================
    NCTreeCtrl.cpp - a scrolling tree view (C2 only). Stock: 0x1002a9a0 .. 0x1002b08f (constructor,
    destructor and the virtuals).
=============================================================================*/
#include "nwindow/NCTreeCtrl.h"
#include "nwindow/NCSliderBar.h"
#include "nwindow/NCTree.h"
#include "nwindow/NCGDevice.h"

enum { NID_TREECTRL_TREE = 0xA00001 };   // the inner tree's id (name ours)

// 0x1002a9a0 - the scroll bar's 16 pixels (NCScrollWnd's field that the stock constructor sets here)
NCTreeCtrl::NCTreeCtrl()
{
    Unknown0C8 = 16;
}
NW_HOOK_CTOR(0x1002a9a0, NCTreeCtrl)

// 0x1002a9f0
NW_HOOK_DTOR(0x1002a9f0, NCTreeCtrl)

// 0x1002aa40 (vslot 36) - the tree, as wide as the window less the scroll bar
INT NCTreeCtrl::OnCreate()
{
    NCScrollWnd::OnCreate();
    Tree = new NCTree();
    Tree->ID = NID_TREECTRL_TREE;
    Tree->SetWindowRect(0, 0, Width - 16, Height);
    CreateChild(Tree, NWS_VISIBLE);
    return 1;
}
NW_HOOK_METHOD(0x1002aa40, NCTreeCtrl, OnCreate)

// 0x1002ab00 (vslot 37) - guarded in C2
INT NCTreeCtrl::OnDestroy()
{
    guard(NCTreeCtrl::OnDestroy);
    if (Tree)
    {
        RemoveChildWindow(Tree->Entry);
        delete Tree;
        Tree = NULL;
    }
    NCScrollWnd::OnDestroy();
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x1002ab00, NCTreeCtrl, OnDestroy)

// 0x1002ac40 (vslot 71) - an entry whose place (its size at creation plus the scroll) falls outside
// the tree window is hidden, any other shown; the same for the entries under it
void NCTreeCtrl::ShowVisible(NCTVItem* Item)
{
    if (Item)
    {
        if (Item->CopyW + ScrollX < 0 || Tree->LocalW < Item->CopyW + ScrollX || Item->CopyH + ScrollY < 0 ||
            Tree->LocalH < Item->CopyH + ScrollY)
            Item->HideWindow();
        else
            Item->ShowWindow();
        for (INT i = 0; i < Item->Children.Num(); i++)
            ShowVisible(Item->Children(i));
    }
}
NW_HOOK_METHOD(0x1002ac40, NCTreeCtrl, ShowVisible)

// 0x1002abb0 (vslot 55) - the content height follows the tree (and 21 more); the entries are shown or
// hidden by where they are
INT NCTreeCtrl::OnPaint(NCGDevice* Device)
{
    Device->BeginPaint(ScreenX, ScreenY);
    Device->SetClip(ScreenX, ScreenY, Width, Height);
    SetContentHeight(Tree->ContentHeight + 21);
    if (Tree->Root)
        ShowVisible(Tree->Root);
    Device->ReleaseClip();
    Device->EndPaint();
    return 1;
}
NW_HOOK_METHOD(0x1002abb0, NCTreeCtrl, OnPaint)

// 0x1002ace0 (vslot 67) - one line up: the tree window moves down by a line
INT NCTreeCtrl::ScrollUp()
{
    if (ScrollY < 0)
    {
        DeltaX = ScrollX;
        DeltaY = ScrollY;
        ScrollY += LineStep;
        if (0 < ScrollY)
            ScrollY = 0;
        ScrollBar->SetValue(-ScrollY);
        DeltaX = DeltaX - ScrollX;
        DeltaY = DeltaY - ScrollY;
        Tree->MoveWindow(0, LineStep);
        Tree->ScrollOffset -= LineStep;
    }
    return 1;
}
NW_HOOK_METHOD(0x1002ace0, NCTreeCtrl, ScrollUp)

// 0x1002ada0 (vslot 68) - one line down
INT NCTreeCtrl::ScrollDown()
{
    if (MinScrollY < ScrollY)
    {
        DeltaX = ScrollX;
        DeltaY = ScrollY;
        ScrollY -= LineStep;
        if (ScrollY < MinScrollY)
            ScrollY = MinScrollY;
        ScrollBar->SetValue(-ScrollY);
        DeltaX = DeltaX - ScrollX;
        DeltaY = DeltaY - ScrollY;
        Tree->MoveWindow(0, -LineStep);
        Tree->ScrollOffset += LineStep;
    }
    return 1;
}
NW_HOOK_METHOD(0x1002ada0, NCTreeCtrl, ScrollDown)

// 0x1002ae70 (vslot 69) - the bar was moved to Value
INT NCTreeCtrl::OnScrollBar(INT Value)
{
    DeltaX = ScrollX;
    DeltaY = ScrollY;
    ScrollY = -Value;
    DeltaY = DeltaY - ScrollY;
    DeltaX = DeltaX - ScrollX;
    Tree->MoveWindow(0, -DeltaY);
    Tree->ScrollOffset += DeltaY;
    return 1;
}
NW_HOOK_METHOD(0x1002ae70, NCTreeCtrl, OnScrollBar)

// 0x1002aef0 (vslot 66) - three lines per notch of the wheel
INT NCTreeCtrl::OnMouseWheel(WPARAM wParam, LPARAM lParam)
{
    SHORT Notches = (SHORT)((SHORT)HIWORD(wParam) / 120);
    DeltaX = ScrollX;
    DeltaY = ScrollY;
    if (Notches < 1)
    {
        if (MinScrollY < ScrollY)
        {
            ScrollY = ScrollY + LineStep * Notches * 3;
            if (ScrollY < MinScrollY)
                ScrollY = MinScrollY;
            ScrollBar->SetValue(-ScrollY);
        }
    }
    else if (ScrollY < 0)
    {
        ScrollY = ScrollY + LineStep * Notches * 3;
        if (0 < ScrollY)
            ScrollY = 0;
        ScrollBar->SetValue(-ScrollY);
    }
    DeltaY = DeltaY - ScrollY;
    DeltaX = DeltaX - ScrollX;
    Tree->MoveWindow(0, -DeltaY);
    Tree->ScrollOffset += DeltaY;
    return 1;
}
NW_HOOK_METHOD(0x1002aef0, NCTreeCtrl, OnMouseWheel)

// 0x1002b020 (vslot 70) - the tree emptied and brought back to its place, the scroll at the top
void NCTreeCtrl::Clear()
{
    Tree->Clear();
    Tree->MoveWindow(-Tree->LocalX, -Tree->LocalY);
    Tree->ScrollOffset = 0;
    ScrollY = 0;
    ScrollX = 0;
    DeltaY = 0;
    DeltaX = 0;
    MinScrollY = 0;
    ScrollBar->ThumbPos = 0;
}
NW_HOOK_METHOD(0x1002b020, NCTreeCtrl, Clear)

NW_CHECK_VTABLE(NCTreeCtrl, 0x1010e8d0, 72)
