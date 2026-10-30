/*=============================================================================
    NCQuestItemWnd.cpp - the quest window's item page. Stock: 0x1009fe70 .. 0x100a036d,
    0x100a0b40 .. 0x100a0fc5, 0x100a1490 .. 0x100a1578 and 0x100a17c0 .. 0x100a1889.
=============================================================================*/
#include "nwindow/NCQuestItemWnd.h"
#include "nwindow/NCDialogBox.h"
#include "nwindow/NCGDevice.h"
#include "nwindow/NCQuestWnd.h"
#include "nwindow/NConsoleWnd.h"
#include "nwindow/NDialogInfo.h"

enum { DIALOG_DESTROY_ITEM = 5 };  // NDialogInfo::ID (name ours)

// 0x100c03b0
NCQuestItemWnd::NCQuestItemWnd()
{
}
NW_HOOK_CTOR(0x100c03b0, NCQuestItemWnd)

// 0x100c0450 - the quest window's is 6 x 4 icons of 32 pixels, 80 at most
NCQuestItemWnd::NCQuestItemWnd(INT InColumns, INT InVisibleRows, INT InMaxItems, INT InCellSize)
    : NCItemWnd(InColumns, InVisibleRows, InMaxItems, InCellSize)
{
}
NW_HOOK_CTOR_ARGS(0x100c0450, NCQuestItemWnd, (INT A, INT B, INT C, INT D), (A, B, C, D))

// 0x100c0400
NW_HOOK_DTOR(0x100c0400, NCQuestItemWnd)

// 0x100bec50 (vslot 25) - NCItemWnd's, then only the destroy dialog's answer: the scroll buttons and the slider
// of this page do nothing (the wheel still scrolls)
INT NCQuestItemWnd::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (NCItemWnd::WindowProc(Msg, wParam, lParam))
        return 1;
    if (Msg == NWM_DIALOG_RESULT)
        return OnDialogResult(wParam, lParam) != 0;
    return 0;
}
NW_HOOK_METHOD(0x100bec50, NCQuestItemWnd, WindowProc)

// 0x100c46e0 (vslot 36)
INT NCQuestItemWnd::OnCreate()
{
    NCItemWnd::OnCreate();
    return 1;
}
NW_HOOK_METHOD(0x100c46e0, NCQuestItemWnd, OnCreate)

// 0x1009fef0  NOT PORTED (no C2 counterpart)
void NCQuestItemWnd::SetToolTip(NItemEntry* Item)
{
    NW_UNPORTED_NORET();
}

// vslot 72: the stock slot is 0x10019c40, `ret 0x10` shared with NCItemWnd's (hooked there): no tool tip
void NCQuestItemWnd::DrawItemToolTip(NCGDevice* Device, NItemEntry* Item, INT X, INT Y)
{
}

// 0x100bf450 (vslot 55) - NCItemWnd's, but the frame of the selected item goes over its icon
INT NCQuestItemWnd::OnPaint(NCGDevice* Device)
{
    guard(NCQuestItemWnd::OnPaint);
    Device->SetClip(ScreenX - 1, ScreenY - 1, Width + 2, Height + 2);
    Device->BeginPaint(ScreenX, ScreenY);
    INT X = 0;
    INT Y = 0;
    for (INT i = TopRow * Columns; i < Items.Num(); i++)
    {
        NItemEntry* Item = Items(i);
        if (!Item)
            continue;
        if (i % Columns == 0)
        {
            X = 0;
            if (i != TopRow * Columns)
                Y += CellSize + 3;
            if (VisibleRows * (CellSize + 1) <= Y)
                break;
        }
        else
            X += CellSize + 3;
        if (Item->GetIcon())
            Device->DrawTexture(X, Y, CellSize, CellSize, 0, 0, CellSize, CellSize, Item->GetIcon());
        if (Item->IsSelected())
            Device->DrawTexture(X - 1, Y - 1, 34, 34, 0, 0, 34, 34, SelectTex);
    }
    Device->EndPaint();
    Device->ReleaseClip();
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x100bf450, NCQuestItemWnd, OnPaint)

// 0x100bf610 (vslot 62) - the carried item is put down; over the quest window's drop zone it asks whether to
// destroy it (systemmsg 73 with the amount for a stack, else 74); the answer comes back as NWM_DIALOG_RESULT.
// The name buffer is the stock frame's 64 characters. C2: the stack's amount goes into the dialog's last field
INT NCQuestItemWnd::OnLButtonUp(WPARAM wParam, LPARAM lParam)
{
    guard(NCQuestItemWnd::OnLButtonUp);
    if (!GConsole->IsCtrlDown())
    {
        UnlockCursor();
        SetCursor(0, NULL);
    }
    if (Selected >= 0 && Selected < Items.Num() && bDragging)
    {
        INT X = LOWORD(lParam);
        INT Y = HIWORD(lParam);
        NWndEntry* E = FindWindowAt(X, Y);
        if (E && E->Wnd && E->Wnd->IsA(NCQuestWnd::StaticClass())
            && ((NCQuestWnd*)E->Wnd)->IsInDropZone(X, Y))
        {
            NItemEntry* Item = Items(Selected);
            TCHAR Name[64];
            if (Item->IsStackable() && Item->Count > 1)
            {
                appSprintf(Name, TEXT("%s"), Item->GetName());
                NDialogInfo Info;
                Info.ID = DIALOG_DESTROY_ITEM;
                Info.Type = 3;
                Info.Owner = Entry;
                Info.bMainBox = 1;
                Info.bInput = 1;
                Info.SetSystemMessage(73);  // "How many $s1(s) do you want to destroy?"
                Info.SetButtons(Name, NULL);
                Info.Unknown74 = Item->Count;
                GConsole->ShowDialog(&Info);
            }
            else
            {
                appSprintf(Name, TEXT("%s"), Item->GetName());
                NDialogInfo Info;
                Info.Owner = Entry;
                Info.ID = DIALOG_DESTROY_ITEM;
                Info.bMainBox = 1;
                Info.bInput = 0;
                Info.SetSystemMessage(74);  // "Destroy $s1. Do you want to continue?"
                Info.SetButtons(Name, NULL);
                GConsole->ShowDialog(&Info);
            }
        }
    }
    bDragging = 0;
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x100bf610, NCQuestItemWnd, OnLButtonUp)

// 0x100bf870 (vslot 73) - the destroy dialog's answer (wParam: the amount typed, 1 without an edit line): the
// request for the selected item
INT NCQuestItemWnd::OnDialogResult(WPARAM wParam, LPARAM lParam)
{
    if (Selected >= 0 && Selected < Items.Num())
    {
        NItemRequest Request;
        Request.ObjectID = Items(Selected)->ObjectID;
        Request.Count = (INT)wParam;
        NRequestDestroyItem(Request);
        UnselectAll();
        Selected = -1;
    }
    return 1;
}
NW_HOOK_METHOD(0x100bf870, NCQuestItemWnd, OnDialogResult)

// 0x100c00a0 - the icon comes from its name ("NWindow.BlackTexture" when missing) and is written back into Info;
// 0 when the page is full. C2: the tool tip is built by NItemEntry::FillToolTip (C1: SetToolTip)
UBOOL NCQuestItemWnd::AddItem(NItemInfo* Info)
{
    if (MaxItems <= Items.Num())
        return 0;
    Info->Icon = FindTexture(Info->IconName);
    if (!Info->Icon)
        Info->Icon = FindTexture(TEXT("NWindow.BlackTexture"));
    NItemEntry* Item = new NItemEntry(Info);
    Item->FillToolTip(2, 0);
    Items.AddItem(Item);
    UpdateScrollRange();
    return 1;
}
NW_HOOK_METHOD(0x100c00a0, NCQuestItemWnd, AddItem)

NW_CHECK_VTABLE(NCQuestItemWnd, 0x10123390, 74)
