/*=============================================================================
    NCShopWnd.cpp - an NPC's shop window and its item grids.
    Stock: 0x100a39e0 .. 0x100a7fee. WindowProc (0x10057b10) and SetTarget (0x10057ba0) of
    the grid, WindowProc (0x100b56e0), ShowWindow (0x100a5d70) and SetListID (0x100b5800) of
    the window are copies of bodies other classes keep the addresses of, and so are SetMode
    (0x100b3880) and SetSide (0x100b3890) of the grid (hooked as NCStoreItemWnd's).
=============================================================================*/
#include "nwindow/NCShopWnd.h"
#include "nwindow/NCConsole.h"
#include "nwindow/NCButton.h"
#include "nwindow/NCDialogBox.h"
#include "nwindow/NCFrameCtrl.h"
#include "nwindow/NCGDevice.h"
#include "nwindow/NCManorShopWnd.h"
#include "nwindow/NCStoreWnd.h"
#include "nwindow/NClasses.gen.h"
#include "nwindow/NConsoleWnd.h"
#include "nwindow/NDialogInfo.h"

enum { DIALOG_SHOP_AMOUNT = 2 };     // NDialogInfo::ID (name ours): an amount for the owner
enum { MSG_HowManyToMove = 72 };     // systemmsg "How many $s1(s) do you want to move?"
enum { ITEM_Adena = 57 };

// sysstring labels
enum
{
    STR_Adena = 134,
    STR_ItemsOnSale = 137,
    STR_Inventory = 138,
    STR_ItemsToBuy = 139,
    STR_OK = 140,
    STR_Cancel = 141,
    STR_BuyPrice = 142,              // "Price"
    STR_SellPrice = 143,             // "Price"
    STR_TipPrice = 322,              // "Price"
    STR_ForEach = 468,
    STR_TipAdena = 469,              // "Adena"
};

// the shop window
static NCShopWnd* ShopWnd()
{
    return GDesktop->ShopWnd;
}

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

// how many of a stack to move (the adena's question allows large amounts); the answer goes to Owner's OnDialogResult.
// Inline in each stock caller; the name buffer is the stock frame's 64 characters. C2 gives the box the most it may
// take (Unknown74: the entry's Count, or its TradeCount selling from the player's grid): Max, set when bSetMax.
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

// The tool tip of an entry of this grid, by its side and the window's mode (C2: the same ladder is inline in each
// stock caller): the price shown depends on them. Nothing for a side or mode out of range.
static FORCEINLINE void ShopFillToolTip(NCShopItemWnd* Wnd, NItemEntry* Item)
{
    INT Extra;
    if (Wnd->Side == SHOPSIDE_Shop)
    {
        if (Wnd->Mode == SHOPMODE_Buy)
            Extra = 7;
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

// a stack joins the target's entry of the same item id when there is one, else a copy holding Count joins the target
// (inline three times in the stock MoveItem)
static FORCEINLINE void AddToTarget(NCShopItemWnd* Target, NItemEntry* Item, INT Count)
{
    if (Item->IsStackable())
    {
        for (INT i = 0; i < Target->Items.Num(); i++)
        {
            if (Target->Items(i)->ID == Item->ID)
            {
                Target->Items(i)->TradeCount += Count;
                ShopFillToolTip(Target, Target->Items(i));
                return;
            }
        }
    }
    Target->AddCopy(Item, Count);
}

/*-----------------------------------------------------------------------------
    NCShopItemWnd.
-----------------------------------------------------------------------------*/

// 0x100cba90
NCShopItemWnd::NCShopItemWnd()
{
    Target = NULL;
}
NW_HOOK_CTOR(0x100cba90, NCShopItemWnd)

// 0x100cbb30
NCShopItemWnd::NCShopItemWnd(INT InColumns, INT InVisibleRows, INT InMaxItems, INT InCellSize)
    : NCItemWnd(InColumns, InVisibleRows, InMaxItems, InCellSize)
{
    Target = NULL;
}
NW_HOOK_CTOR_ARGS(0x100cbb30, NCShopItemWnd, (INT A, INT B, INT C, INT D), (A, B, C, D))

// 0x100cbae0
NW_HOOK_DTOR(0x100cbae0, NCShopItemWnd)

// 0x100c0e40 (vslot 25) - the body is shared with the other grids' (NCStoreItemWnd, NCTradeItemWnd, NCDeliverItemWnd), the
// stock keeps one address, which this hooks: the double click is the virtual at slot 73, the dialog box's answer the
// one at slot 74
INT NCShopItemWnd::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (NCItemWnd::WindowProc(Msg, wParam, lParam))
        return 1;
    if (Msg == WM_LBUTTONDBLCLK)
        return OnLButtonDblClick(wParam, lParam) != 0;
    if (Msg == NWM_DIALOG_RESULT)
        return OnDialogResult(wParam, lParam) != 0;
    return 0;
}
NW_HOOK_METHOD(0x100c0e40, NCShopItemWnd, WindowProc)

// 0x100c46f0 (vslot 37) - the body is shared with the other grids' (NCStoreItemWnd, NCDeliverItemWnd, the private
// stores'): the stock keeps one address, which this hooks
INT NCShopItemWnd::OnDestroy()
{
    NCItemWnd::OnDestroy();
    return 1;
}
NW_HOOK_METHOD(0x100c46f0, NCShopItemWnd, OnDestroy)

// stock body shared with NCTradeItemWnd::SetKind (0x10057ba0)
void NCShopItemWnd::SetTarget(NCShopItemWnd* InTarget)
{
    Target = InTarget;
}

// stock body shared with NCStoreItemWnd's (0x100b3880)
void NCShopItemWnd::SetMode(INT InMode)
{
    Mode = InMode;
}

// stock body shared with NCStoreItemWnd's (0x100b3890)
void NCShopItemWnd::SetSide(INT InSide)
{
    Side = InSide;
}


// 0x100cbb70 (vslot 62) - the carried item dropped onto another shop grid goes there (that grid becomes the
// target): how many first for a stack (from the NPC's goods when buying, whatever its amount), else one
INT NCShopItemWnd::OnLButtonUp(WPARAM wParam, LPARAM lParam)
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
        if (E && E->Wnd && E->Wnd->IsA(NClass::NCShopItemWnd()))
        {
            Target = (NCShopItemWnd*)E->Wnd;
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
NW_HOOK_METHOD(0x100cbb70, NCShopItemWnd, OnLButtonUp)

// 0x100cbed0 (vslot 73) - the item goes to the target: how many first for a stack (any stack when buying), else one
INT NCShopItemWnd::OnLButtonDblClick(WPARAM wParam, LPARAM lParam)
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
NW_HOOK_METHOD(0x100cbed0, NCShopItemWnd, OnLButtonDblClick)

// 0x100cc1c0 (vslot 74) - the amount typed (wParam; when selling at most the entry's) goes to the target. The stock
// code reads Items(Selected) without a bounds check.
INT NCShopItemWnd::OnDialogResult(WPARAM wParam, LPARAM lParam)
{
    NItemEntry* Item = Items(Selected);
    if (Item && wParam)
    {
        INT Count = (INT)wParam;
        if (Mode == SHOPMODE_Sell && Count >= Item->TradeCount)
            Count = Item->TradeCount;
        MoveItem(Item, Count);
    }
    return 1;
}
NW_HOOK_METHOD(0x100cc1c0, NCShopItemWnd, OnDialogResult)

// 0x100cac30 - a new entry, its icon from its name ("NWindow.BlackTexture" when missing, written back into Info), its
// amount the item's count
UBOOL NCShopItemWnd::AddItem(NItemInfo* Info)
{
    if (Items.Num() >= MaxItems)
        return 0;
    Info->Icon = FindTexture(Info->IconName);
    if (!Info->Icon)
        Info->Icon = FindTexture(TEXT("NWindow.BlackTexture"));
    NItemEntry* Item = new NItemEntry(Info);
    Item->TradeCount = Info->Count;
    ShopFillToolTip(this, Item);
    Items.AddItem(Item);
    UpdateScrollRange();
    return 1;
}
NW_HOOK_METHOD(0x100cac30, NCShopItemWnd, AddItem)

// 0x100cad80 - a copy of another grid's entry holding Count
UBOOL NCShopItemWnd::AddCopy(NItemEntry* Src, INT Count)
{
    if (Items.Num() >= MaxItems)
        return 0;
    NItemEntry* Item = new NItemEntry;
    Item->Copy(Src);
    Item->TradeCount = Count;
    ShopFillToolTip(this, Item);
    Items.AddItem(Item);
    UpdateScrollRange();
    return 1;
}
NW_HOOK_METHOD(0x100cad80, NCShopItemWnd, AddCopy)

// 0x100caea0 - the item goes to the target. Selling, the target gets the amount (the NPC's list or the inventory)
// and this entry loses it; buying, the NPC's goods stay (the list of purchases gets them) and what goes back is only
// taken off. An emptied entry leaves the grid (not freed, as in the stock code); then the total of the shop window
// shown, else the other window's (Desktop +0xE4, 0x10092da0).
void NCShopItemWnd::MoveItem(NItemEntry* Item, INT Count)
{
    if (!Target)
        return;
    UnselectAll();
    if (Mode == SHOPMODE_Sell)
    {
        // the stock code has one identical branch for each side
        if (Target->Side == SHOPSIDE_Shop || Target->Side == SHOPSIDE_Player)
        {
            AddToTarget(Target, Item, Count);
            if (Item->IsStackable() && Item->TradeCount > Count)
            {
                Item->TradeCount -= Count;
                ShopFillToolTip(this, Item);
            }
            else
                Items.Remove(Selected, 1);
        }
    }
    else if (Mode == SHOPMODE_Buy)
    {
        if (Target->Side == SHOPSIDE_Shop)
        {
            if (Item->IsStackable() && Item->TradeCount > Count)
            {
                Item->TradeCount -= Count;
                ShopFillToolTip(this, Item);
            }
            else
                Items.Remove(Selected, 1);
        }
        else if (Target->Side == SHOPSIDE_Player)
            AddToTarget(Target, Item, Count);
    }
    if (ShopWnd() && ShopWnd()->IsEnabled() && ShopWnd()->IsShowWindow())
        ShopWnd()->UpdateTotalPrice();
    else
        ((NCManorShopWnd*)GDesktop->NewWndE4)->UpdateTotalPrice();   // 0x10092da0, the manor shop (desktop +0xE4)
    Selected = -1;
    UpdateScrollRange();
}
NW_HOOK_METHOD(0x100caea0, NCShopItemWnd, MoveItem)

NW_CHECK_VTABLE(NCShopItemWnd, 0x10125080, 75)

/*-----------------------------------------------------------------------------
    NCShopWnd.
-----------------------------------------------------------------------------*/

// 0x100c9fe0 - Mode is left as allocated
NCShopWnd::NCShopWnd()
{
    PlayerItems = NULL;
    ShopItems = NULL;
    Unknown9C = 0;
    BackTex = NULL;
    UnknownA4 = 0;
    UpButton = NULL;
    DownButton = NULL;
    OKButton = NULL;
    CancelButton = NULL;
    UnknownB8 = NULL;
    Adena = 0;
    TotalPrice = 0;
    ListID = -1;
}
NW_HOOK_CTOR(0x100c9fe0, NCShopWnd)

// 0x10092370 - the destructor (the base's only; the manor shop's calls it too)
NW_HOOK_DTOR(0x10092370, NCShopWnd)


// stock body shared with NCStoreWnd::WindowProc (0x1006ed90), which keeps the address: the buttons
INT NCShopWnd::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (NCWnd::WindowProc(Msg, wParam, lParam))
        return 1;
    if (Msg == NWM_COMMAND)
    {
        if (wParam == NID_STORE_OK)
            return OnOK() != 0;
        if (wParam == NID_STORE_CANCEL)
            return OnCancel() != 0;
        if (wParam == NID_STORE_UP)
            return OnUp() != 0;
        if (wParam == NID_STORE_DOWN)
            return OnDown() != 0;
        if (wParam == NID_STORE_CLOSE)
            return OnClose() != 0;
    }
    return 0;
}

// 0x100cc210 (vslot 36) - title bar, the grids (6 x 3 icons, 160 items at most: the NPC's on top, the player's
// below), the arrows between them, OK and Cancel (C1: 200 items at most)
INT NCShopWnd::OnCreate()
{
    NCWnd::OnCreate();
    BackTex = FindTexture(TEXT("L2UI.StoreWnd.StoreBack"));

    FrameCtrl = new NCFrameCtrl(1);
    FrameID = NID_STORE_FRAME;
    FrameCtrl->ID = NID_STORE_FRAME;
    FrameCtrl->SetWindowRect(0, 0, 240, 21);
    CreateChild(FrameCtrl, NWS_VISIBLE);

    ShopItems = new NCShopItemWnd(6, 3, 160, 32);
    ShopItems->SetWindowRect(16, 46, 222, 102);
    CreateChild(ShopItems, NWS_VISIBLE | 0x20);   // 0x20: meaning unknown
    ShopItems->SetSide(SHOPSIDE_Shop);
    PlayerItems = new NCShopItemWnd(6, 3, 160, 32);
    PlayerItems->SetWindowRect(16, 180, 222, 102);
    CreateChild(PlayerItems, NWS_VISIBLE | 0x20);
    PlayerItems->SetSide(SHOPSIDE_Player);
    ShopItems->SetTarget(PlayerItems);
    PlayerItems->SetTarget(ShopItems);

    // the up arrow's pushed texture is its normal one, as in the stock code
    UpButton = new NCButton(TEXT("L2UI.StoreWnd.StoreUpBtn1"), TEXT("L2UI.StoreWnd.StoreUpBtn1"), 0);
    UpButton->ID = NID_STORE_UP;
    UpButton->SetWindowRect(194, 158, 14, 13);
    CreateChild(UpButton, NWS_VISIBLE);
    DownButton = new NCButton(TEXT("L2UI.StoreWnd.StoreDownBtn1"), TEXT("L2UI.StoreWnd.StoreDownBtn2"), 0);
    DownButton->ID = NID_STORE_DOWN;
    DownButton->SetWindowRect(210, 158, 14, 13);
    CreateChild(DownButton, NWS_VISIBLE);

    OKButton = new NCButton(TEXT("L2UI.Control.DefaultButton"), TEXT("L2UI.Control.DefaultButton_click"), 0);
    OKButton->ID = NID_STORE_OK;
    OKButton->SetWindowRect(85, 331, 67, 19);
    CreateChild(OKButton, NWS_VISIBLE);
    OKButton->SetTextID(STR_OK);
    CancelButton = new NCButton(TEXT("L2UI.Control.DefaultButton"), TEXT("L2UI.Control.DefaultButton_click"), 0);
    CancelButton->ID = NID_STORE_CANCEL;
    CancelButton->SetWindowRect(157, 331, 67, 19);
    CreateChild(CancelButton, NWS_VISIBLE);
    CancelButton->SetTextID(STR_Cancel);
    return 1;
}
NW_HOOK_METHOD(0x100cc210, NCShopWnd, OnCreate)

// 0x100ca4c0 (vslot 37) - one body with NCStoreWnd's (and NCGMStoreWnd's)
INT NCShopWnd::OnDestroy()
{
    DestroyChild(this, PlayerItems);
    DestroyChild(this, ShopItems);
    DestroyChild(this, UpButton);
    DestroyChild(this, DownButton);
    DestroyChild(this, OKButton);
    DestroyChild(this, CancelButton);
    DestroyChild(this, UnknownB8);
    NCWnd::OnDestroy();
    return 1;
}
NW_HOOK_METHOD(0x100ca4c0, NCShopWnd, OnDestroy)

// stock body shared with NCEditBox's (0x100a5d70): the layout comes with SetMode
void NCShopWnd::ShowWindow()
{
    NCWnd::ShowWindow();
}

// 0x100c9fb0 (vslot 44)
void NCShopWnd::HideWindow()
{
    Clear();
    NCWnd::HideWindow();
}
NW_HOOK_METHOD(0x100c9fb0, NCShopWnd, HideWindow)

// 0x100ca050 (vslot 55) - the grids' labels by mode, the total's and the adena's labels right-aligned at 132, their
// values at 222, thousands separated (the total, 64 bits, through "%d": its low half); the buffers are the stock frame's
INT NCShopWnd::OnPaint(NCGDevice* Device)
{
    TCHAR Text[128];       // C1: 1024 characters each
    TCHAR TotalText[128];
    TCHAR AdenaText[128];
    SIZE Size;
    Device->BeginPaint(ScreenX, ScreenY);
    Device->SetClip(ScreenX, ScreenY, Width, Height);
    if (BackTex)
        Device->DrawTexture(0, 21, 240, 344, 0, 0, 240, 344, BackTex);
    if (Mode == SHOPMODE_Buy)
    {
        Device->DrawText(15, 25, 0xFFFFFFFF, *SysString(STR_ItemsOnSale), 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
        Device->DrawText(15, 159, 0xFFFFFFFF, *SysString(STR_ItemsToBuy), 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
        NCGDevice::GetTextExtent(&Size, *SysString(STR_BuyPrice), 0, 0, 0, 0, 0);
        Device->DrawText(132 - Size.cx, 294, 0xFFFFFFFF, *SysString(STR_BuyPrice), 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
        NCGDevice::GetTextExtent(&Size, *SysString(STR_Adena), 0, 0, 0, 0, 0);
        Device->DrawText(132 - Size.cx, 312, 0xFFFFFFFF, *SysString(STR_Adena), 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
    }
    else if (Mode == SHOPMODE_Sell)
    {
        Device->DrawText(15, 25, 0xFFFFFFFF, *SysString(STR_Inventory), 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
        Device->DrawText(15, 159, 0xFFFFFFFF, *SysString(STR_ItemsOnSale), 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
        NCGDevice::GetTextExtent(&Size, *SysString(STR_SellPrice), 0, 0, 0, 0, 0);
        Device->DrawText(132 - Size.cx, 294, 0xFFFFFFFF, *SysString(STR_SellPrice), 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
        NCGDevice::GetTextExtent(&Size, *SysString(STR_Adena), 0, 0, 0, 0, 0);
        Device->DrawText(132 - Size.cx, 312, 0xFFFFFFFF, *SysString(STR_Adena), 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
    }
    appSprintf(Text, TEXT("%d"), TotalPrice);
    FormatNumber(TotalText, Text);
    appSprintf(Text, TEXT("%s"), TotalText);
    NCGDevice::GetTextExtent(&Size, Text, 0, 0, 0, 0, 0);
    Device->DrawText(222 - Size.cx, 294, 0xFFFFFFFF, Text, 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
    appSprintf(Text, TEXT("%d"), Adena);
    FormatNumber(AdenaText, Text);
    appSprintf(Text, TEXT("%s"), AdenaText);
    NCGDevice::GetTextExtent(&Size, Text, 0, 0, 0, 0, 0);
    Device->DrawText(222 - Size.cx, 311, 0xFFFFFFFF, Text, 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
    Device->EndPaint();
    Device->ReleaseClip();
    return 1;
}
NW_HOOK_METHOD(0x100ca050, NCShopWnd, OnPaint)

// the entries of a grid that hold something
static FORCEINLINE INT CountFilled(NCShopItemWnd* Grid)
{
    INT Num = 0;
    for (INT i = 0; i < Grid->Items.Num(); i++)
        if (Grid->Items(i)->TradeCount > 0)
            Num++;
    return Num;
}

// 0x100ca710 (vslot 67) - the lower grid's list to the server: list id, count, then (object id, item id, amount) of
// each item sold or (item id, amount) of each item bought; the window closes
INT NCShopWnd::OnOK()
{
    if (Mode == SHOPMODE_Sell)
    {
        INT Num = CountFilled(ShopItems);
        L2ParamStack Params(Num * 3 + 2);
        Params.PushBack((void*)(INT_PTR)ListID);
        Params.PushBack((void*)(INT_PTR)Num);
        for (INT i = 0; i < ShopItems->Items.Num(); i++)
        {
            if (ShopItems->Items(i)->TradeCount > 0)
            {
                Params.PushBack((void*)(INT_PTR)ShopItems->Items(i)->ObjectID);
                Params.PushBack((void*)(INT_PTR)ShopItems->Items(i)->ID);
                Params.PushBack((void*)(INT_PTR)ShopItems->Items(i)->TradeCount);
            }
        }
        GConsole->RequestSellItem(Params);
    }
    else if (Mode == SHOPMODE_Buy)
    {
        INT Num = CountFilled(PlayerItems);
        L2ParamStack Params(Num * 3 + 2);
        Params.PushBack((void*)(INT_PTR)ListID);
        Params.PushBack((void*)(INT_PTR)Num);
        for (INT i = 0; i < PlayerItems->Items.Num(); i++)
        {
            if (PlayerItems->Items(i)->TradeCount > 0)
            {
                Params.PushBack((void*)(INT_PTR)PlayerItems->Items(i)->ID);
                Params.PushBack((void*)(INT_PTR)PlayerItems->Items(i)->TradeCount);
            }
        }
        GConsole->RequestBuyItem(Params);
    }
    HideWindow();
    return 1;
}
NW_HOOK_METHOD(0x100ca710, NCShopWnd, OnOK)

// 0x100c9fc0 (vslot 68)
INT NCShopWnd::OnCancel()
{
    Clear();
    HideWindow();
    return 1;
}
NW_HOOK_METHOD(0x100c9fc0, NCShopWnd, OnCancel)

// vslot 66 - OnCancel's stock body (0x100a5d90 serves both slots)
INT NCShopWnd::OnClose()
{
    Clear();
    HideWindow();
    return 1;
}

// the arrows: the selected item of Grid goes to its target, how many first for a stack of more
// than one - or for any stack with bAnyStack, the down arrow on the NPC's list, whose stock
// copy has no count test (inline in the stock OnUp / OnDown, twice each)
static FORCEINLINE void MoveSelected(NCShopItemWnd* Grid, UBOOL bAnyStack = 0)
{
    INT Sel = Grid->GetSelected();
    if (Sel < 0 || Sel >= Grid->Items.Num())
        return;
    NItemEntry* Item = Grid->Items(Sel);
    if (!Item)
        return;
    if (Item->IsStackable() && (bAnyStack || Item->TradeCount > 1))
        AskAmount(Grid, Item, Item->Count, 1);
    else
        Grid->MoveItem(Item, 1);
}

// 0x100cb420 (vslot 69) - the up arrow: from the lower grid
INT NCShopWnd::OnUp()
{
    if (Mode == SHOPMODE_Buy)
        MoveSelected(PlayerItems);
    else if (Mode == SHOPMODE_Sell)
        MoveSelected(ShopItems);
    return 1;
}
NW_HOOK_METHOD(0x100cb420, NCShopWnd, OnUp)

// 0x100cb760 (vslot 70) - the down arrow: from the upper grid
INT NCShopWnd::OnDown()
{
    if (Mode == SHOPMODE_Buy)
        MoveSelected(ShopItems, 1);
    else if (Mode == SHOPMODE_Sell)
        MoveSelected(PlayerItems);
    return 1;
}
NW_HOOK_METHOD(0x100cb760, NCShopWnd, OnDown)

// 0x100c9f60 - one body with the setters of other windows' +0xC4 (the private stores')
void NCShopWnd::SetAdena(INT InAdena)
{
    Adena = InAdena;
}
NW_HOOK_METHOD(0x100c9f60, NCShopWnd, SetAdena)

// stock body shared with NCStoreWnd::SetAdena (0x100b5800)
void NCShopWnd::SetListID(INT InListID)
{
    ListID = InListID;
}

// 0x100c9f70 - kept out of line like the stock code (HideWindow, OnCancel)
__declspec(noinline) void NCShopWnd::Clear()
{
    ListID = -1;
    Adena = 0;
    TotalPrice = 0;
    PlayerItems->Clear();
    ShopItems->Clear();
}
NW_HOOK_METHOD(0x100c9f70, NCShopWnd, Clear)

// 0x100ca610 (out of line like the stock code) - buying, the NPC's grid on top and the
// purchases below; selling, the inventory on top and what is sold below
__declspec(noinline) void NCShopWnd::UpdateLayout()
{
    if (Mode == SHOPMODE_Buy)
    {
        ShopItems->UpdateScrollRange();
        PlayerItems->UpdateScrollRange();
        // the stock code fills { LocalX, LocalY, LocalW, LocalH }; SetScreenPos reads x and y
        POINT Origin;
        Origin.x = LocalX;
        Origin.y = LocalY;
        ShopItems->SetScreenPos(16, 46, &Origin);
        PlayerItems->SetScreenPos(16, 180, &Origin);
    }
    else if (Mode == SHOPMODE_Sell)
    {
        ShopItems->UpdateScrollRange();
        PlayerItems->UpdateScrollRange();
        POINT Origin;
        Origin.x = LocalX;
        Origin.y = LocalY;
        ShopItems->SetScreenPos(16, 180, &Origin);
        PlayerItems->SetScreenPos(16, 46, &Origin);
    }
    else
        return;
    ShopItems->SetSize(222, 102);
    PlayerItems->SetSize(222, 102);
}
NW_HOOK_METHOD(0x100ca610, NCShopWnd, UpdateLayout)

// 0x100ca960 - the sum of price x amount over the lower grid, in 64 bits
void NCShopWnd::UpdateTotalPrice()
{
    TotalPrice = 0;
    if (Mode == SHOPMODE_Buy)
    {
        for (INT i = 0; i < PlayerItems->Items.Num(); i++)
            TotalPrice += (SQWORD)PlayerItems->Items(i)->GetUnknownC20() * PlayerItems->Items(i)->TradeCount;
    }
    else if (Mode == SHOPMODE_Sell)
    {
        for (INT i = 0; i < ShopItems->Items.Num(); i++)
            TotalPrice += (SQWORD)ShopItems->Items(i)->GetUnknownC20() * ShopItems->Items(i)->TradeCount;
    }
}
NW_HOOK_METHOD(0x100ca960, NCShopWnd, UpdateTotalPrice)

// one grid's entries get their name and description again from the item tables, and their tool tips (the stock code
// repeats this for each grid, with the CRT's wcscpy). The tool tip's price follows the side and mode of Ladder: the
// stock code takes them from the NPC's grid for the player's entries too.
static FORCEINLINE void ReloadItemTexts(NCShopItemWnd* Grid, NCShopItemWnd* Ladder)
{
    for (INT i = 0; i < Grid->Items.Num(); i++)
    {
        NItemEntry* Item = Grid->Items(i);
        if (Item)
        {
            wcscpy(Item->Name, GL2GameData.GetItemName(Item->ID));
            wcscpy(Item->Description, GL2GameData.GetItemDescription(Item->ID));
            ShopFillToolTip(Ladder, Item);
        }
    }
}

// 0x100caa50 - after the item texts were reloaded (NConsoleWnd::ReloadGameTexts)
void NCShopWnd::ReloadTexts()
{
    ReloadItemTexts(ShopItems, ShopItems);
    ReloadItemTexts(PlayerItems, ShopItems);
}
NW_HOOK_METHOD(0x100caa50, NCShopWnd, ReloadTexts)

// 0x100cb3c0
void NCShopWnd::SetMode(INT InMode)
{
    Mode = InMode;
    ShopItems->SetMode(Mode);
    PlayerItems->SetMode(Mode);
    UpdateLayout();
}
NW_HOOK_METHOD(0x100cb3c0, NCShopWnd, SetMode)

// 0x100cb400
UBOOL NCShopWnd::AddShopItem(NItemInfo* Info)
{
    return ShopItems->AddItem(Info);
}
NW_HOOK_METHOD(0x100cb400, NCShopWnd, AddShopItem)

// 0x100cb410
UBOOL NCShopWnd::AddPlayerItem(NItemInfo* Info)
{
    return PlayerItems->AddItem(Info);
}
NW_HOOK_METHOD(0x100cb410, NCShopWnd, AddPlayerItem)

NW_CHECK_VTABLE(NCShopWnd, 0x1011bb98, 72)
