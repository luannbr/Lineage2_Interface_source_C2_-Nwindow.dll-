/*=============================================================================
    NCRecipeBookItemWnd.cpp - the recipe book's icon grid (new in C2).
    Stock: 0x100c4960 .. 0x100c4ce9 (the mouse handlers), 0x100c69e0 (AddItem), 0x100c6f40 .. 0x100c6ff9
    (constructors). WindowProc (0x100c4610) is the body shared with NCInvenItemWnd.
=============================================================================*/
#include "nwindow/NCRecipeBookItemWnd.h"
#include "nwindow/NCRecipeBookWnd.h"
#include "nwindow/NCConsole.h"
#include "nwindow/NConsoleWnd.h"
#include "nwindow/NDialogInfo.h"
#include "nwindow/NClasses.gen.h"

enum { DIALOG_RECIPE_DELETE = 0x22 };  // NDialogInfo::ID (name ours): delete the recipe (Param1 its object id), the answer is the console's
enum { MSG_DeleteRecipe = 0x4a };      // systemmsg line the box shows

// 0x100c6f40
NCRecipeBookItemWnd::NCRecipeBookItemWnd()
{
}
NW_HOOK_CTOR(0x100c6f40, NCRecipeBookItemWnd)

// 0x100c6fe0 - the four NCItemWnd arguments (columns, visible rows, most items, cell size)
NCRecipeBookItemWnd::NCRecipeBookItemWnd(INT InColumns, INT InVisibleRows, INT InMaxItems, INT InCellSize)
    : NCItemWnd(InColumns, InVisibleRows, InMaxItems, InCellSize)
{
}
NW_HOOK_CTOR_ARGS(0x100c6fe0, NCRecipeBookItemWnd, (INT A, INT B, INT C, INT D), (A, B, C, D))

// 0x100c6f90
NW_HOOK_DTOR(0x100c6f90, NCRecipeBookItemWnd)

// stock body shared with NCInvenItemWnd::WindowProc (0x100c4610), which keeps the address: the double click
INT NCRecipeBookItemWnd::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (NCItemWnd::WindowProc(Msg, wParam, lParam))
        return 1;
    if (Msg == WM_LBUTTONDBLCLK)
        return OnLButtonDblClick(wParam, lParam) != 0;
    return 0;
}

// 0x100c69e0 - a new entry from the item record (icon from its name, "NWindow.BlackTexture" when missing, written back
// into Info), the tool tip its name line (NTT_ItemName). 0 when the grid is full.
UBOOL NCRecipeBookItemWnd::AddItem(NItemInfo* Info)
{
    if (Items.Num() >= MaxItems)
        return 0;
    Info->Icon = FindTexture(Info->IconName);
    if (!Info->Icon)
        Info->Icon = FindTexture(TEXT("NWindow.BlackTexture"));
    NItemEntry* Item = new NItemEntry(Info);
    Item->FillToolTip(1, 0);
    Items.AddItem(Item);
    UpdateScrollRange();
    return 1;
}
NW_HOOK_METHOD(0x100c69e0, NCRecipeBookItemWnd, AddItem)

// 0x100c4960 (vslot 61) - a click picks the recipe up
INT NCRecipeBookItemWnd::OnLButtonDown(WPARAM wParam, LPARAM lParam)
{
    Selected = HitTest(LOWORD(lParam), HIWORD(lParam));
    HoverIndex = Selected;
    if (Selected >= 0 && Selected < Items.Num())
    {
        UnselectAll();
        Items(Selected)->Select();
        bDragging = 1;
        return 1;
    }
    return 1;
}
NW_HOOK_METHOD(0x100c4960, NCRecipeBookItemWnd, OnLButtonDown)

// 0x100c49d0 (vslot 60) - the hovered recipe; with the window focused the carried recipe's icon is the cursor, and the
// parent book learns whether the cursor is over its trash icon
INT NCRecipeBookItemWnd::OnMouseMove(WPARAM wParam, LPARAM lParam)
{
    INT X = LOWORD(lParam);
    INT Y = HIWORD(lParam);
    HoverIndex = HitTest(X, Y);
    if (IsFocused())
    {
        if (!bDragging)
        {
            UnlockCursor();
            SetCursor(0, NULL);
        }
        else if (Selected >= 0 && Selected < Items.Num())
        {
            SetCursor(4, Items(Selected)->GetIcon());
            LockCursor();
        }
        NCRecipeBookWnd* Book = (NCRecipeBookWnd*)Entry->Parent->Wnd;
        Book->bTrashActive = PtInRect(X, Y, Book->TrashX + Book->ScreenX, Book->TrashY + Book->ScreenY,
                                      Book->TrashW, Book->TrashH) != 0;
    }
    return 1;
}
NW_HOOK_METHOD(0x100c49d0, NCRecipeBookItemWnd, OnMouseMove)

// 0x100c4ad0 (vslot 62) - the recipe dropped on the trash icon of the book asks to be deleted (a yes / no box, ID 0x22,
// Param1 the recipe's object id)
INT NCRecipeBookItemWnd::OnLButtonUp(WPARAM wParam, LPARAM lParam)
{
    INT X = LOWORD(lParam);
    INT Y = HIWORD(lParam);
    if (!GConsole->IsCtrlDown())
    {
        UnlockCursor();
        SetCursor(0, NULL);
    }
    if (Selected >= 0 && Selected < Items.Num())
    {
        NWndEntry* E = FindWindowAt(X, Y);
        if (E && E->Wnd && E->Wnd->IsA(NClass::NCRecipeBookWnd()))
        {
            NCRecipeBookWnd* Book = (NCRecipeBookWnd*)E->Wnd;
            if (Book->HitTrash(X, Y))
            {
                TCHAR Name[64];
                appSprintf(Name, TEXT("%s"), Items(Selected)->GetName());
                NDialogInfo Info;
                Info.Owner = Entry;
                Info.ID = DIALOG_RECIPE_DELETE;
                Info.bMainBox = 1;
                Info.SetSystemMessage(MSG_DeleteRecipe);
                Info.SetButtons(Name, NULL);
                Info.SetParams(Items(Selected)->ObjectID, -1);
                GConsole->ShowDialog(&Info);
            }
        }
    }
    bDragging = 0;
    return 1;
}
NW_HOOK_METHOD(0x100c4ad0, NCRecipeBookItemWnd, OnLButtonUp)

// 0x100c4c70 (vslot 73) - a double click asks the server for the recipe's ingredients and closes the grid
INT NCRecipeBookItemWnd::OnLButtonDblClick(WPARAM wParam, LPARAM lParam)
{
    Selected = HitTest(LOWORD(lParam), HIWORD(lParam));
    if (Selected >= 0 && Selected < Items.Num())
    {
        NItemEntry* Item = Items(Selected);
        if (Item)
        {
            GConsole->RequestRecipeItemMakeInfo(Item->ObjectID);
            HideWindow();
        }
    }
    UnselectAll();
    Selected = -1;
    bDragging = 0;
    return 1;
}
NW_HOOK_METHOD(0x100c4c70, NCRecipeBookItemWnd, OnLButtonDblClick)

NW_CHECK_VTABLE(NCRecipeBookItemWnd, 0x10124538, 74)
