/*=============================================================================
    NCManorShopItemWnd.cpp - the item grids of the manor's shop (C2 only; a copy of NCShopItemWnd).
    Stock: 0x10092ea0 .. 0x10093287 (AddItem, AddCopy, MoveItem, between the window's functions), 0x10093960 ..
    0x100939ff (constructor, destructor), 0x10093a00 .. 0x10094090 + 0x5f (constructor with the sizes, OnLButtonUp,
    OnLButtonDblClick, OnDialogResult). WindowProc (0x100c0e40), OnDestroy (0x100c46f0), SetTarget (0x100d8aa0),
    SetMode (0x1002f6a0) and SetSide (0x100d8ab0) are copies of bodies other classes keep the addresses of.
=============================================================================*/
#include "nwindow/NCManorShopItemWnd.h"
#include "nwindow/NCShopWnd.h"
#include "nwindow/NCManorShopWnd.h"
#include "nwindow/NCConsole.h"
#include "nwindow/NCDialogBox.h"
#include "nwindow/NCGDevice.h"
#include "nwindow/NClasses.gen.h"
#include "nwindow/NConsoleWnd.h"
#include "nwindow/NDialogInfo.h"

enum { DIALOG_SHOP_AMOUNT = 2 };     // NDialogInfo::ID (name ours): an amount for the owner (the shop's)
enum { MSG_HowManyToMove = 72 };     // systemmsg "How many $s1(s) do you want to move?"
enum { ITEM_Adena = 57 };

// how many of a stack to move (the adena's question allows large amounts); the answer goes to Owner's OnDialogResult.
// Inline in each stock caller; the name buffer is the stock frame's 64 characters. The box gets the most it may take
// (Unknown74: Max), set when bSetMax. The same as the shop's.
static FORCEINLINE void AskAmount(NCWnd* Owner, NItemEntry* Item, INT Max, UBOOL bSetMax)
{
    TCHAR Name[64];
    appSprintf(Name, TEXT("%s"), Item->GetName());
    NDialogInfo Info;
    Info.ID = DIALOG_SHOP_AMOUNT;
    Info.Type = 3;
    Info.bInput = 1;
    Info.bMainBox = 1;
    Info.Owner = Owner->Entry;
    Info.SetSystemMessage(MSG_HowManyToMove);
    Info.SetButtons(Name, NULL);
    if (bSetMax)
        Info.Unknown74 = Max;
    Info.bLargeAmount = Item->ID == ITEM_Adena;
    GConsole->ShowDialog(&Info);
}

// The tool tip of an entry of this grid, by its side and the grid's mode (inline in each stock caller): the shop's
// ladder with one constant changed (5 for the NPC's goods when buying, where the shop passes 7). Nothing for a side or
// mode out of range.
static FORCEINLINE void ManorFillToolTip(NCManorShopItemWnd* Wnd, NItemEntry* Item)
{
    INT Extra;
    if (Wnd->Side == SHOPSIDE_Shop)
    {
        if (Wnd->Mode == SHOPMODE_Buy)
            Extra = 5;
        else if (Wnd->Mode == SHOPMODE_Sell)
            Extra = 8;
        else
            return;
    }
    else if (Wnd->Side == SHOPSIDE_Player)
    {
        if (Wnd->Mode == SHOPMODE_Buy)
            Extra = 4;
        else if (Wnd->Mode == SHOPMODE_Sell)
            Extra = 8;
        else
            return;
    }
    else
        return;
    Item->FillToolTip(2, Extra);
}

// 0x10093960
NCManorShopItemWnd::NCManorShopItemWnd()
{
    Target = NULL;
}
NW_HOOK_CTOR(0x10093960, NCManorShopItemWnd)

// 0x10093a00 (name ours)
NCManorShopItemWnd::NCManorShopItemWnd(INT InColumns, INT InVisibleRows, INT InMaxItems, INT InCellSize)
    : NCItemWnd(InColumns, InVisibleRows, InMaxItems, InCellSize)
{
    Target = NULL;
}
NW_HOOK_CTOR_ARGS(0x10093a00, NCManorShopItemWnd, (INT A, INT B, INT C, INT D), (A, B, C, D))

// 0x100939b0
NW_HOOK_DTOR(0x100939b0, NCManorShopItemWnd)

// 0x100c0e40 (vslot 25) - the body is shared with the other grids' (NCShopItemWnd, NCStoreItemWnd, NCTradeItemWnd,
// NCDeliverItemWnd), the stock keeps one address (which NCShopItemWnd hooks): the double click is the virtual at slot
// 73, the dialog box's answer the one at slot 74
INT NCManorShopItemWnd::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (NCItemWnd::WindowProc(Msg, wParam, lParam))
        return 1;
    if (Msg == WM_LBUTTONDBLCLK)
        return OnLButtonDblClick(wParam, lParam) != 0;
    if (Msg == NWM_DIALOG_RESULT)
        return OnDialogResult(wParam, lParam) != 0;
    return 0;
}

// 0x100c46f0 (vslot 37) - the body is shared with the other grids' (the stock keeps one address, which NCShopItemWnd hooks)
INT NCManorShopItemWnd::OnDestroy()
{
    NCItemWnd::OnDestroy();
    return 1;
}

// stock body shared with the other grids' setter of +0x110 (0x100d8aa0)
void NCManorShopItemWnd::SetTarget(NCManorShopItemWnd* InTarget)
{
    Target = InTarget;
}

// stock body shared with NCHtmlViewer::SetOwner and the other grids' setter of +0x114 (0x1002f6a0)
void NCManorShopItemWnd::SetMode(INT InMode)
{
    Mode = InMode;
}

// stock body shared with the other grids' setter of +0x118 (0x100d8ab0)
void NCManorShopItemWnd::SetSide(INT InSide)
{
    Side = InSide;
}

// 0x10093a40 (vslot 62) - the carried item dropped onto another manor grid goes there (that grid becomes the
// target): how many first for a stack (from the NPC's goods when buying, whatever its amount), else one
INT NCManorShopItemWnd::OnLButtonUp(WPARAM wParam, LPARAM lParam)
{
    INT X = LOWORD(lParam);
    INT Y = HIWORD(lParam);
    if (!GConsole->IsCtrlDown())
    {
        UnlockCursor();
        SetCursor(0, NULL);
    }
    if (Selected >= 0 && Selected < Items.Num() && bDragging && !IsInWindow(X, Y))
    {
        NWndEntry* E = FindWindowAt(X, Y);
        if (E && E->Wnd && E->Wnd->IsA(NClass::NCManorShopItemWnd()))
        {
            Target = (NCManorShopItemWnd*)E->Wnd;
            NItemEntry* Item = Items(Selected);
            if (Mode == SHOPMODE_Buy && Target->Side == SHOPSIDE_Player)
            {
                if (Item->IsStackable())
                {
                    AskAmount(this, Item, Item->Count, Item->ID == ITEM_Adena);
                    bDragging = 0;
                    return 1;
                }
            }
            else if (Item->IsStackable() && Item->TradeCount > 1)
            {
                AskAmount(this, Item, Item->ID == ITEM_Adena ? Item->Count : Item->TradeCount, 1);
                bDragging = 0;
                return 1;
            }
            MoveItem(Item, 1);
        }
    }
    bDragging = 0;
    return 1;
}
NW_HOOK_METHOD(0x10093a40, NCManorShopItemWnd, OnLButtonUp)

// 0x10093da0 (vslot 73) - the item goes to the target: how many first for a stack (any stack when buying), else one
INT NCManorShopItemWnd::OnLButtonDblClick(WPARAM wParam, LPARAM lParam)
{
    bNoDrag = 1;
    Selected = HitTest(LOWORD(lParam), HIWORD(lParam));
    if (Selected >= 0 && Selected < Items.Num())
    {
        NItemEntry* Item = Items(Selected);
        if (Mode == SHOPMODE_Buy)
        {
            if (Item->IsStackable())
            {
                AskAmount(this, Item, Item->Count, 1);
                bDragging = 0;
                return 1;
            }
        }
        else if (Item->IsStackable() && Item->TradeCount > 1)
        {
            AskAmount(this, Item, Item->Count, 1);
            bDragging = 0;
            return 1;
        }
        MoveItem(Item, 1);
    }
    bDragging = 0;
    return 1;
}
NW_HOOK_METHOD(0x10093da0, NCManorShopItemWnd, OnLButtonDblClick)

// 0x10094090 (vslot 74) - the amount typed (wParam) goes to the target, at most the entry's in both modes. Unlike the
// shop's, the stock code checks the selection.
INT NCManorShopItemWnd::OnDialogResult(WPARAM wParam, LPARAM lParam)
{
    if (Selected >= 0 && Selected < Items.Num())
    {
        NItemEntry* Item = Items(Selected);
        if (Item && wParam)
        {
            INT Count = (INT)wParam;
            if (Mode == SHOPMODE_Sell)
            {
                if (Count >= Item->TradeCount)
                    Count = Item->TradeCount;
            }
            if (Mode == SHOPMODE_Buy)
            {
                if (Count >= Item->TradeCount)
                    Count = Item->TradeCount;
            }
            MoveItem(Item, Count);
        }
    }
    return 1;
}
NW_HOOK_METHOD(0x10094090, NCManorShopItemWnd, OnDialogResult)

// 0x10092ea0 (name ours) - a new entry, its icon from its name ("NWindow.BlackTexture" when missing, written back
// into Info), its amount the item's count
UBOOL NCManorShopItemWnd::AddItem(NItemInfo* Info)
{
    if (Items.Num() >= MaxItems)
        return 0;
    Info->Icon = FindTexture(Info->IconName);
    if (!Info->Icon)
        Info->Icon = FindTexture(TEXT("NWindow.BlackTexture"));
    NItemEntry* Item = new NItemEntry(Info);
    Item->TradeCount = Info->Count;
    ManorFillToolTip(this, Item);
    Items.AddItem(Item);
    UpdateScrollRange();
    return 1;
}
NW_HOOK_METHOD(0x10092ea0, NCManorShopItemWnd, AddItem)

// 0x10092ff0 (name ours) - a copy of another grid's entry holding Count
UBOOL NCManorShopItemWnd::AddCopy(NItemEntry* Src, INT Count)
{
    if (Items.Num() >= MaxItems)
        return 0;
    NItemEntry* Item = new NItemEntry;
    Item->Copy(Src);
    Item->TradeCount = Count;
    ManorFillToolTip(this, Item);
    Items.AddItem(Item);
    UpdateScrollRange();
    return 1;
}
NW_HOOK_METHOD(0x10092ff0, NCManorShopItemWnd, AddCopy)

// 0x10093110 (name ours) - the item goes to the target, whatever the mode and the side (the shop's grid keeps the
// NPC's goods): a stack joins the target's entry of the same item id when there is one (its tool tip with no price
// line), else a copy holding Count joins it. This entry loses the amount, and leaves the grid when it is emptied (not
// freed, as in the stock code). Then the total of the shop window shown (the NPC's shop, when that is the one), else
// the manor window's (Desktop +0xE4, 0x10092da0).
void NCManorShopItemWnd::MoveItem(NItemEntry* Item, INT Count)
{
    if (!Target)
        return;
    UnselectAll();
    INT Index = 0;
    UBOOL bJoined = 0;
    if (Item->IsStackable())
    {
        for (Index = 0; Index < Target->Items.Num(); Index++)
        {
            if (Target->Items(Index)->ID == Item->ID)
            {
                bJoined = 1;
                break;
            }
        }
    }
    if (bJoined)
    {
        Target->Items(Index)->TradeCount += Count;
        Target->Items(Index)->FillToolTip(2, 0);
    }
    else
        Target->AddCopy(Item, Count);
    if (Item->IsStackable() && Item->TradeCount > Count)
    {
        Item->TradeCount -= Count;
        Item->FillToolTip(2, 0);
    }
    else
        Items.Remove(Selected, 1);
    NCShopWnd* ShopWnd = GDesktop->ShopWnd;
    if (ShopWnd && ShopWnd->IsEnabled() && ShopWnd->IsShowWindow())
        ShopWnd->UpdateTotalPrice();
    else
        static_cast<NCManorShopWnd*>(GDesktop->NewWndE4)->UpdateTotalPrice();
    Selected = -1;
    UpdateScrollRange();
}
NW_HOOK_METHOD(0x10093110, NCManorShopItemWnd, MoveItem)

NW_CHECK_VTABLE(NCManorShopItemWnd, 0x1011bdd8, 75)
