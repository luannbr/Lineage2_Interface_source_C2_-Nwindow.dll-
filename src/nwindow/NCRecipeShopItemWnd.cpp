/*=============================================================================
    NCRecipeShopItemWnd.cpp - the item grids of the recipe shop's manage window (new in C2).
    Stock: 0x100c3010 .. 0x100c3c4a (AddItem, AddCopy, MoveItem, the constructors, the mouse
    handlers), 0x100c38f0 (the four-argument constructor).
    WindowProc (0x100c0e40) and OnDestroy (0x100c46f0) are bodies shared with NCShopItemWnd.
=============================================================================*/
#include "nwindow/NCRecipeShopItemWnd.h"
#include "nwindow/NCConsole.h"
#include "nwindow/NConsoleWnd.h"
#include "nwindow/NCDialogBox.h"
#include "nwindow/NDialogInfo.h"
#include "nwindow/NClasses.gen.h"

enum { DIALOG_RECIPE_PRICE = 0x23 };  // NDialogInfo::ID (name ours): a price for the owner (the answer is OnDialogResult)
enum { MSG_PriceToBuy = 0x142 };      // systemmsg line the price box shows (OnDialogResult's twin in NCRecipeShopWnd)
enum { MSG_PriceToBuyDrop = 0x3c3 };  // the same box when the entry was dropped on the grid

// the price question (inline in the three stock callers): the entry's name goes through sprintf into a 64-character
// buffer that nothing reads afterwards, the box asks for a number of up to 100,000,000 and answers Owner
static FORCEINLINE void AskPrice(NCRecipeShopItemWnd* Owner, NItemEntry* Item, INT MsgID)
{
    TCHAR Name[64];
    appSprintf(Name, TEXT("%s"), Item->GetName());
    NDialogInfo Info;
    Info.ID = DIALOG_RECIPE_PRICE;
    Info.Type = 3;
    Info.Owner = Owner->Entry;
    Info.bMainBox = 1;
    Info.bInput = 1;
    Info.SetSystemMessage(MsgID);
    Info.bLargeAmount = 1;
    GConsole->ShowDialog(&Info);
}

// 0x100c3850
NCRecipeShopItemWnd::NCRecipeShopItemWnd()
{
    Target = NULL;
}
NW_HOOK_CTOR(0x100c3850, NCRecipeShopItemWnd)

// 0x100c38f0 - the four NCItemWnd arguments (columns, visible rows, most items, cell size)
NCRecipeShopItemWnd::NCRecipeShopItemWnd(INT InColumns, INT InVisibleRows, INT InMaxItems, INT InCellSize)
    : NCItemWnd(InColumns, InVisibleRows, InMaxItems, InCellSize)
{
    Target = NULL;
}
NW_HOOK_CTOR_ARGS(0x100c38f0, NCRecipeShopItemWnd, (INT A, INT B, INT C, INT D), (A, B, C, D))

// 0x100c38a0
NW_HOOK_DTOR(0x100c38a0, NCRecipeShopItemWnd)

// stock body shared with NCShopItemWnd::WindowProc (0x100c0e40), which keeps the address: the double click and the
// dialog box's answer
INT NCRecipeShopItemWnd::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (NCItemWnd::WindowProc(Msg, wParam, lParam))
        return 1;
    if (Msg == WM_LBUTTONDBLCLK)
        return OnLButtonDblClick(wParam, lParam) != 0;
    if (Msg == NWM_DIALOG_RESULT)
        return OnDialogResult(wParam, lParam) != 0;
    return 0;
}

// stock body shared with NCShopItemWnd::OnDestroy (0x100c46f0), which keeps the address
INT NCRecipeShopItemWnd::OnDestroy()
{
    NCItemWnd::OnDestroy();
    return 1;
}

// stock 0x100d8aa0 (the body NCStoreItemWnd::SetTarget and NCDeliverItemWnd::SetTarget share, hooked on the first)
void NCRecipeShopItemWnd::SetTarget(NCRecipeShopItemWnd* InTarget)
{
    Target = InTarget;
}

// stock 0x1002f6a0 (the body NCHtmlViewer::SetOwner shares, hooked there)
void NCRecipeShopItemWnd::SetMode(INT InMode)
{
    Mode = InMode;
}

// 0x100c3010 - a new entry from the item record (icon from its name, "NWindow.BlackTexture" when missing, written back
// into Info), its TradeCount the record's +0x171C; the tool tip is the name line (NTT_ItemName), with the count line
// (NTTO_Count) on the Mode 0 grid. 0 when the grid is full.
UBOOL NCRecipeShopItemWnd::AddItem(NItemInfo* Info)
{
    if (MaxItems <= Items.Num())
        return 0;
    Info->Icon = FindTexture(Info->IconName);
    if (!Info->Icon)
        Info->Icon = FindTexture(TEXT("NWindow.BlackTexture"));
    NItemEntry* Item = new NItemEntry(Info);
    Item->TradeCount = Info->Count;
    if (Mode == 0)
        Item->FillToolTip(1, 0x20);
    else if (Mode == 1)
        Item->FillToolTip(1, 0);
    Items.AddItem(Item);
    UpdateScrollRange();
    return 1;
}
NW_HOOK_METHOD(0x100c3010, NCRecipeShopItemWnd, AddItem)

// 0x100c3130 - a copy of another grid's entry (the move of a recipe into the Mode 0 grid), same tool tip as AddItem
UBOOL NCRecipeShopItemWnd::AddCopy(NItemEntry* Src)
{
    if (MaxItems <= Items.Num())
        return 0;
    NItemEntry* Item = new NItemEntry;
    Item->Copy(Src);
    if (Mode == 0)
        Item->FillToolTip(1, 0x20);
    else if (Mode == 1)
        Item->FillToolTip(1, 0);
    Items.AddItem(Item);
    UpdateScrollRange();
    return 1;
}
NW_HOOK_METHOD(0x100c3130, NCRecipeShopItemWnd, AddCopy)

// 0x100c3210 - the entry goes to the other grid: a copy into the Mode 0 grid unless it holds the recipe already (the ids
// are compared), out of this grid (not freed, as in the stock code) when the target is the Mode 1 grid
void NCRecipeShopItemWnd::MoveItem(NItemEntry* Item)
{
    if (Target)
    {
        UnselectAll();
        if (Target->Mode == 0)
        {
            for (INT i = 0; i < Target->Items.Num(); i++)
                if (Target->Items(i)->ID == Item->ID)
                    goto Done;
            Target->AddCopy(Item);
        }
        else if (Target->Mode == 1)
            Items.Remove(Selected, 1);
Done:
        Selected = -1;
        UpdateScrollRange();
    }
}
NW_HOOK_METHOD(0x100c3210, NCRecipeShopItemWnd, MoveItem)

// 0x100c3930 (vslot 62) - the carried entry dropped onto the other grid: the Mode 0 grid asks the price first, the Mode 1
// grid takes it back at once
INT NCRecipeShopItemWnd::OnLButtonUp(WPARAM wParam, LPARAM lParam)
{
    INT X = LOWORD(lParam);
    INT Y = HIWORD(lParam);
    if (!GConsole->IsCtrlDown())
    {
        UnlockCursor();
        SetCursor(0, NULL);
    }
    if (Selected >= 0 && Selected < Items.Num() && bDragging)
    {
        if (!IsInWindow(X, Y))
        {
            NWndEntry* E = FindWindowAt(X, Y);
            if (E && E->Wnd && E->Wnd->IsA(NClass::NCRecipeShopItemWnd()))
            {
                Target = (NCRecipeShopItemWnd*)E->Wnd;
                NItemEntry* Item = Items(Selected);
                if (Target->Mode == 0)
                    AskPrice(this, Item, MSG_PriceToBuyDrop);
                else if (Target->Mode == 1)
                    MoveItem(Item);
            }
        }
    }
    bDragging = 0;
    return 1;
}
NW_HOOK_METHOD(0x100c3930, NCRecipeShopItemWnd, OnLButtonUp)

// 0x100c3ae0 (vslot 73) - a double click moves the entry the same way (the target is the other grid already)
INT NCRecipeShopItemWnd::OnLButtonDblClick(WPARAM wParam, LPARAM lParam)
{
    bNoDrag = 1;
    Selected = HitTest(LOWORD(lParam), HIWORD(lParam));
    if (Selected >= 0 && Selected < Items.Num())
    {
        NItemEntry* Item = Items(Selected);
        if (Target->Mode == 0)
            AskPrice(this, Item, MSG_PriceToBuy);
        else if (Target->Mode == 1)
            MoveItem(Item);
    }
    bDragging = 0;
    return 1;
}
NW_HOOK_METHOD(0x100c3ae0, NCRecipeShopItemWnd, OnLButtonDblClick)

// 0x100c3c20 (vslot 74) - the price typed (wParam) goes into the selected entry, which then moves to the other grid; the
// stock code reads Items(Selected) without a bounds check
INT NCRecipeShopItemWnd::OnDialogResult(WPARAM wParam, LPARAM lParam)
{
    NItemEntry* Item = Items(Selected);
    if (Item)
    {
        Item->UnknownC20 = (INT)wParam;
        MoveItem(Item);
    }
    return 1;
}
NW_HOOK_METHOD(0x100c3c20, NCRecipeShopItemWnd, OnDialogResult)

NW_CHECK_VTABLE(NCRecipeShopItemWnd, 0x10123b38, 75)
