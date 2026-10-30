/*=============================================================================
    NCPrivateBuyWnd.cpp - the private store (buy) window and its grids. Stock:
    0x100955a0 .. 0x100981a0 (the grids' tool tips), 0x10098190 .. 0x1009ad8d, and
    HideWindow (0x1009d960); the bodies it shares with the sell window are hooked here or
    there (NCPrivateShopWnd.cpp).
=============================================================================*/
#include "nwindow/NCPrivateBuyWnd.h"
#include "nwindow/NCButton.h"
#include "nwindow/NCConsole.h"
#include "nwindow/NCFrameCtrl.h"
#include "nwindow/NCGDevice.h"
#include "nwindow/NConsoleWnd.h"
#include "nwindow/NDialogInfo.h"
#include "nwindow/NItemToolTip.h"

// NWM_COMMAND ids (names ours; the sell window's)
enum
{
    NID_BUYSTORE_ABORT = 0xF0,
    NID_BUYSTORE_FRAME = 0xF3,
    NID_BUYSTORE_START = 0xF4,
    NID_BUYSTORE_UP = 0xF6,
    NID_BUYSTORE_DOWN = 0xF7,
    NID_BUYSTORE_CLOSE = 0xF8,
    NID_BUYSTORE_MESSAGE = 0xF9,
};

// NDialogInfo::ID of the questions (names ours)
enum
{
    DIALOG_BUYSTORE_PRICE = 0x2A,           // C1: 0x26 (the five ids moved up by four)
    DIALOG_BUYSTORE_AMOUNT = 0x2B,
    DIALOG_BUYSTORE_MESSAGE = 0x2C,
    DIALOG_BUYSTORE_PRICE_CAUTION = 0x2D,
    DIALOG_BUYSTORE_SELL_CAUTION = 0x2E,
};

// systemmsg
enum
{
    MSG_MoveHowMany = 72,       // "How many $s1(s) do you want to move?"
    MSG_StoreMessage = 334,     // "Please enter store message."
    MSG_PriceCaution = 569,     // "Caution--the item price greatly differs from the shop's
                                //   standard price. Do you wish to continue?"
    MSG_BuyHowMany = 570,       // "How many $s1 (s) do you wish to purchase?"
    MSG_BuyHowMany2 = 571,      // "How many $s1 (s) do you want to purchase?"
    MSG_SetBuyPrice = 585,      // "Please decide on the price."
};

// sysstring
enum
{
    STR_Adena = 134,
    STR_ItemsOnSale = 137,
    STR_Inventory = 138,
    STR_OK = 140,
    STR_Price = 142,
    STR_PriceSell = 143,
    STR_Message = 384,
    STR_Abort = 385,
    STR_Start = 428,
    STR_WishList = 502,
    STR_ForSale = 503,
};

// a child window goes (repeated inline in the stock OnDestroys)
template <class T> static FORCEINLINE void DestroyStoreChild(NCWnd* Parent, T*& Child)
{
    if (Child)
    {
        Parent->RemoveChildWindow(Child->Entry);
        delete Child;
        Child = NULL;
    }
}

// a price 5 times away from the item's reference price, either way (the reference 0: none)
static FORCEINLINE UBOOL IsUnusualPrice(NItemEntry* Item)
{
    return Item->Unknown1CA8 > 0
        && (Item->UnknownC20 <= Item->Unknown1CA8 / 5 || Item->UnknownC20 >= Item->Unknown1CA8 * 5);
}

// the desktop's box asks how many of the item (NWM_BUYSTORE_AMOUNT answers the grid); C2 hands it the most it takes
static FORCEINLINE void AskAmount(NCWnd* Grid, NItemEntry* Item, INT MsgID, INT Max)
{
    TCHAR Name[64];
    appSprintf(Name, TEXT("%s"), Item->GetName());
    NDialogInfo Info;
    Info.ID = DIALOG_BUYSTORE_AMOUNT;
    Info.Type = 3;
    Info.bInput = 1;
    Info.bMainBox = 1;
    Info.Owner = Grid->Entry;
    Info.SetSystemMessage(MsgID);
    Info.SetButtons(Name, NULL);
    Info.Unknown74 = Max;
    Info.bLargeAmount = 0;
    GConsole->ShowDialog(&Info);
}

// ... the price offered, up to 100,000,000 (NWM_BUYSTORE_PRICE answers)
static FORCEINLINE void AskPrice(NCWnd* Grid)
{
    NDialogInfo Info;
    Info.ID = DIALOG_BUYSTORE_PRICE;
    Info.Type = 3;
    Info.bInput = 1;
    Info.bMainBox = 1;
    Info.Owner = Grid->Entry;
    Info.SetSystemMessage(MSG_SetBuyPrice);
    Info.bLargeAmount = 1;
    GConsole->ShowDialog(&Info);
}

/*-----------------------------------------------------------------------------
    NCPrivateBuyItemWnd.
-----------------------------------------------------------------------------*/

// 0x100bb2a0
NCPrivateBuyItemWnd::NCPrivateBuyItemWnd()
{
    CoverTex = FindTexture(TEXT("NWindow.Chat"));
    Partner = NULL;
    Unknown128 = 0;
}
NW_HOOK_CTOR(0x100bb2a0, NCPrivateBuyItemWnd)

// 0x100bb390
NCPrivateBuyItemWnd::NCPrivateBuyItemWnd(INT InColumns, INT InVisibleRows, INT InMaxItems, INT InCellSize)
    : NCItemWnd(InColumns, InVisibleRows, InMaxItems, InCellSize)
{
    CoverTex = FindTexture(TEXT("NWindow.Chat"));
    Partner = NULL;
    Unknown128 = 0;
}
NW_HOOK_CTOR_ARGS(0x100bb390, NCPrivateBuyItemWnd, (INT A, INT B, INT C, INT D), (A, B, C, D))

// 0x100bb340
NW_HOOK_DTOR(0x100bb340, NCPrivateBuyItemWnd)

// 0x100b9050 (vslot 25) - the double click and the dialog box's answers (the price, the amount, the unusual price
// confirmed or not)
INT NCPrivateBuyItemWnd::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (NCItemWnd::WindowProc(Msg, wParam, lParam))
        return 1;
    if (Msg == WM_LBUTTONDBLCLK)
        return OnLButtonDblClick(wParam, lParam) != 0;
    if (Msg == NWM_BUYSTORE_PRICE_CANCEL)
        return OnPriceCancel(wParam, lParam) != 0;
    if (Msg == NWM_BUYSTORE_PRICE)
        return OnPrice(wParam, lParam) != 0;
    if (Msg == NWM_BUYSTORE_AMOUNT)
        return OnAmount(wParam, lParam) != 0;
    if (Msg == NWM_BUYSTORE_PRICE_OK)
        return OnPriceOK(wParam, lParam) != 0;
    return 0;
}
NW_HOOK_METHOD(0x100b9050, NCPrivateBuyItemWnd, WindowProc)

// stock body shared with NCDeliverItemWnd's (0x100a39e0)
INT NCPrivateBuyItemWnd::OnDestroy()
{
    NCItemWnd::OnDestroy();
    return 1;
}

// 0x100b9390 (vslot 55) - NCItemWnd's icons; at someone's store the items the player lacks
// (count 0) get a cover, its texture looked up again each time. The stock guard carries
// NCItemWnd's name.
INT NCPrivateBuyItemWnd::OnPaint(NCGDevice* Device)
{
    guard(NCItemWnd::OnPaint);
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
        if (Mode == 0 && Item->Count == 0)
        {
            CoverTex = FindTexture(TEXT("NWindow.ChatBack"));
            if (Item->IsSelected())
                Device->DrawTexture(X - 1, Y - 1, 34, 34, 0, 0, 34, 34, SelectTex);
            if (Item->GetIcon())
                Device->DrawTexture(X, Y, CellSize, CellSize, 0, 0, CellSize, CellSize, Item->GetIcon());
            Device->DrawTexture(X, Y, CellSize, CellSize, 0, 0, CellSize, CellSize, CoverTex);
        }
        else
        {
            if (Item->IsSelected())
                Device->DrawTexture(X - 1, Y - 1, 34, 34, 0, 0, 34, 34, SelectTex);
            if (Item->GetIcon())
                Device->DrawTexture(X, Y, CellSize, CellSize, 0, 0, CellSize, CellSize, Item->GetIcon());
        }
    }
    Device->EndPaint();
    Device->ReleaseClip();
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x100b9390, NCPrivateBuyItemWnd, OnPaint)

// 0x100bb410 (vslot 62) - NCPrivateShopItemWnd::OnLButtonUp for the buy store: at someone's store an item the player
// lacks stays; going on the wish list asks the price, a stack coming off it how many
INT NCPrivateBuyItemWnd::OnLButtonUp(WPARAM wParam, LPARAM lParam)
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
            if (E && E->Wnd && E->Wnd->IsA(NCPrivateBuyItemWnd::StaticClass()))
            {
                Partner = (NCPrivateBuyItemWnd*)E->Wnd;
                NItemEntry* Item = Items(Selected);
                if (Mode == 0)
                {
                    if (Item->Count > 0)
                    {
                        if (Item->IsStackable())
                            AskAmount(this, Item, MSG_MoveHowMany, Item->TradeCount);
                        else
                            MoveItem(Item, 1);
                    }
                }
                else if (Partner->Side == 0)
                    AskPrice(this);
                else if (Item->IsStackable() && Item->Count > 1)
                    AskAmount(this, Item, MSG_BuyHowMany2, Item->TradeCount);
                else
                    MoveItem(Item, 1);
            }
        }
    }
    bDragging = 0;
    return 1;
}
NW_HOOK_METHOD(0x100bb410, NCPrivateBuyItemWnd, OnLButtonUp)

// 0x100bb7b0 (vslot 73) - OnLButtonUp's moves, onto Partner (a stack coming off the list asks how many whatever its
// count)
INT NCPrivateBuyItemWnd::OnLButtonDblClick(WPARAM wParam, LPARAM lParam)
{
    bNoDrag = 1;
    Selected = HitTest(LOWORD(lParam), HIWORD(lParam));
    if (Selected >= 0 && Selected < Items.Num())
    {
        NItemEntry* Item = Items(Selected);
        if (Mode == 0)
        {
            if (Item->Count > 0)
            {
                if (Item->IsStackable())
                    AskAmount(this, Item, MSG_MoveHowMany, Item->TradeCount);
                else
                    MoveItem(Item, 1);
            }
        }
        else if (Partner->Side == 0)
            AskPrice(this);
        else if (Item->IsStackable())
            AskAmount(this, Item, MSG_BuyHowMany2, Item->TradeCount);
        else
            MoveItem(Item, 1);
    }
    bDragging = 0;
    return 1;
}
NW_HOOK_METHOD(0x100bb7b0, NCPrivateBuyItemWnd, OnLButtonDblClick)

// 0x100b9750 (vslot 75) - the price offered for the selected item (setting one's own store up): it takes it; a price 5
// times away from the reference asks again, then how many are wanted (always; 0: the box stays)
INT NCPrivateBuyItemWnd::OnPrice(WPARAM wParam, LPARAM lParam)
{
    NItemEntry* Item = Items(Selected);
    INT Price = (INT)wParam;
    if (!Item)
        return 1;
    if (!Price)
        return 1;
    if (Mode == 1 && Price > 0)
    {
        INT Reference = Item->Unknown1CA8;
        Item->UnknownC20 = Price;
        if (Reference < 1 || (Reference / 5 < Price && Price < Reference * 5))
        {
            AskAmount(this, Item, MSG_BuyHowMany, Item->Count);
            return 0;
        }
        NDialogInfo Info;
        Info.Owner = Entry;
        Info.ID = DIALOG_BUYSTORE_PRICE_CAUTION;
        Info.bMainBox = 1;
        Info.bInput = 0;
        Info.SetSystemMessage(MSG_PriceCaution);
        Info.bLargeAmount = 0;
        GConsole->ShowDialog(&Info);
        return 0;
    }
    return 1;
}
NW_HOOK_METHOD(0x100b9750, NCPrivateBuyItemWnd, OnPrice)

// 0x100bbaa0 (vslot 76) - the amount typed: that many of the selected item move (setting up, at most 100,000,000; at
// someone's store C2 refuses more than it wants with a system message, C1 clamped)
INT NCPrivateBuyItemWnd::OnAmount(WPARAM wParam, LPARAM lParam)
{
    NItemEntry* Item = Items(Selected);
    INT Amount = (INT)wParam;
    if (Item && Amount)
    {
        if (Mode == 1 && Amount > 0)
        {
            Unknown128 = 0;
            if (Amount >= 100000000)
                Amount = 100000000;
        }
        else
        {
            if (Mode != 0)
                return 1;
            if (Amount < 1)
                return 1;
            if (Item->UnknownC34 < Amount)
            {
                GConsole->ShowSystemMessage(0x40C, -1);   // 0x1006a8c0
                return 1;
            }
        }
        MoveItem(Item, Amount);
    }
    return 1;
}
NW_HOOK_METHOD(0x100bbaa0, NCPrivateBuyItemWnd, OnAmount)

// 0x100b9650 (vslot 77) - the unusual price confirmed: how many are wanted
INT NCPrivateBuyItemWnd::OnPriceOK(WPARAM wParam, LPARAM lParam)
{
    AskAmount(this, Items(Selected), MSG_BuyHowMany, Items(Selected)->Count);
    return 0;
}
NW_HOOK_METHOD(0x100b9650, NCPrivateBuyItemWnd, OnPriceOK)

// stock body shared with NCStoreItemWnd::SetMode (0x100b3880)
void NCPrivateBuyItemWnd::SetPartner(NCPrivateBuyItemWnd* InPartner)
{
    Partner = InPartner;
}

// stock body shared with NCStoreItemWnd::SetSide (0x100b3890)
void NCPrivateBuyItemWnd::SetMode(INT InMode)
{
    Mode = InMode;
}

// 0x100b9130
void NCPrivateBuyItemWnd::SetSide(INT InSide)
{
    Side = InSide;
}
NW_HOOK_METHOD(0x100b9130, NCPrivateBuyItemWnd, SetSide)


// the tool tip of an entry by the grid's side: the player's side (1) and the list's (0) read different columns (the
// flags 0x802 / 8 are the buy store's)
static FORCEINLINE void PrivateBuyToolTip(NItemEntry* Item, INT Side)
{
    if (Side == 1)
        Item->FillToolTip(2, 0);
    else if (Side == 0)
        Item->FillToolTip(0x802, 8);
}

// 0x100ba6e0 - the icon comes from its name ("NWindow.BlackTexture" when missing), written back into Info; the whole
// count is there
UBOOL NCPrivateBuyItemWnd::AddItem(NItemInfo* Info)
{
    if (Items.Num() >= MaxItems)
        return 0;
    Info->Icon = FindTexture(Info->IconName);
    if (!Info->Icon)
        Info->Icon = FindTexture(TEXT("NWindow.BlackTexture"));
    NItemEntry* Item = new NItemEntry(Info);
    Item->TradeCount = Info->Count;
    PrivateBuyToolTip(Item, Side);
    Items.AddItem(Item);
    UpdateScrollRange();
    return 1;
}
NW_HOOK_METHOD(0x100ba6e0, NCPrivateBuyItemWnd, AddItem)

// 0x100ba800 - a copy of Src with Count there
UBOOL NCPrivateBuyItemWnd::AddEntry(NItemEntry* Src, INT Count)
{
    if (Items.Num() >= MaxItems)
        return 0;
    NItemEntry* Item = new NItemEntry;
    Item->Copy(Src);
    Item->TradeCount = Count;
    PrivateBuyToolTip(Item, Side);
    Items.AddItem(Item);
    UpdateScrollRange();
    return 1;
}
NW_HOOK_METHOD(0x100ba800, NCPrivateBuyItemWnd, AddEntry)

// 0x100bcd50 (the sell window's NCPrivateShopWnd::CheckTotal shares the body; not hooked here)
UBOOL NCPrivateBuyWnd::CheckTotal(INT ItemID, INT Count, INT Price)
{
    INT Total = 0;
    INT Sum = 0;
    for (INT i = 0; i < Count; i++)
    {
        Sum += Price;
        if (Sum > 1000000000)
            return 0;
    }
    if (StoreItems)
    {
        for (INT i = 0; i < StoreItems->Items.Num(); i++)
        {
            NItemEntry* Entry = StoreItems->Items(i);
            if (Entry->ID == ItemID)
                Total += Entry->TradeCount * Price;
            else
                Total += Entry->GetUnknownC20() * Entry->TradeCount;
        }
    }
    return Total + Sum <= 1000000000;
}

// the entry of the partner's list a stack joins: the same item id (inline in the stock MoveItem)
static FORCEINLINE NItemEntry* FindBuyStackIn(NCPrivateBuyItemWnd* Grid, NItemEntry* Item)
{
    if (Item->IsStackable())
    {
        for (INT i = 0; i < Grid->Items.Num(); i++)
            if (Grid->Items(i)->ID == Item->ID)
                return Grid->Items(i);
    }
    return NULL;
}

// 0x100ba8f0 - Count of the selected Item to Partner. Setting one's own store up, the catalog keeps its item (C2 first
// checks that the wish list's total stays within a billion): a stack joins the same item on the wish list (taking this
// price), single items replace the entries of the same item and enchant there (re-reading the entry that slides into the
// removed one's place) and come as one entry each, 100 at most; off the list the entry just shrinks or leaves. At
// someone's store a stack joins the same item on the other side or comes as a copy, the amount wanted moving along, and
// shrinks or leaves (not freed).
void NCPrivateBuyItemWnd::MoveItem(NItemEntry* Item, INT Count)
{
    if (!Partner)
        return;
    UnselectAll();
    if (Mode == 1)
    {
        if (Partner->Side == 0)
        {
            if (!GDesktop->PrivateBuyWnd->CheckTotal(Item->ObjectID, Count, Item->GetUnknownC20()))
                return;
            if (!Item->IsStackable())
            {
                for (INT i = 0; i < Partner->Items.Num(); i++)
                {
                    NItemEntry* Listed = Partner->Items(i);
                    if (Listed->ID == Item->ID && Listed->Enchant == Item->Enchant)
                    {
                        Partner->Items.Remove(i, 1);
                        i--;
                    }
                }
                if (Count > 100)
                    Count = 100;
                for (; Count > 0; Count--)
                    Partner->AddEntry(Item, 1);
            }
            else
            {
                NItemEntry* Same = FindBuyStackIn(Partner, Item);
                if (Same)
                {
                    Same->TradeCount += Count;
                    Same->UnknownC20 = Item->UnknownC20;
                    PrivateBuyToolTip(Same, Partner->Side);
                }
                else
                    Partner->AddEntry(Item, Count);
            }
        }
        else if (Partner->Side == 1)
        {
            if (Item->IsStackable() && Count < Item->TradeCount)
            {
                Item->TradeCount -= Count;
                PrivateBuyToolTip(Item, Side);
            }
            else
                Items.Remove(Selected, 1);
        }
    }
    else if (Mode == 0 && (Partner->Side == 0 || Partner->Side == 1))
    {
        NItemEntry* Same = FindBuyStackIn(Partner, Item);
        if (Same)
        {
            Same->TradeCount += Count;
            Same->UnknownC34 += Count;
            PrivateBuyToolTip(Same, Partner->Side);
        }
        else
            Partner->AddEntry(Item, Count);
        if (Item->IsStackable() && Count < Item->TradeCount)
        {
            Item->UnknownC34 -= Count;
            Item->TradeCount -= Count;
            PrivateBuyToolTip(Item, Side);
        }
        else
            Items.Remove(Selected, 1);
    }
    GDesktop->PrivateBuyWnd->UpdateTotal();
    Selected = -1;
    UpdateScrollRange();
}
NW_HOOK_METHOD(0x100ba8f0, NCPrivateBuyItemWnd, MoveItem)

NW_CHECK_VTABLE(NCPrivateBuyItemWnd, 0x10122d40, 78)

/*-----------------------------------------------------------------------------
    NCPrivateBuyWnd.
-----------------------------------------------------------------------------*/

// 0x100b9940 - Mode and the total are left as allocated
NCPrivateBuyWnd::NCPrivateBuyWnd()
{
    PlayerItems = NULL;
    StoreItems = NULL;
    Unknown9C = 0;
    BackTex = NULL;
    UnknownA4 = 0;
    UpButton = NULL;
    DownButton = NULL;
    StartButton = NULL;
    MessageButton = NULL;
    AbortButton = NULL;
    UnknownBC = NULL;
    Adena = 0;
    OwnerID = -1;
}
NW_HOOK_CTOR(0x100b9940, NCPrivateBuyWnd)

// 0x100b99e0
NW_HOOK_DTOR(0x100b99e0, NCPrivateBuyWnd)

// 0x100b9140 (vslot 25) - the buttons and the unusual prices confirmed
INT NCPrivateBuyWnd::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (NCWnd::WindowProc(Msg, wParam, lParam))
        return 1;
    if (Msg == NWM_COMMAND)
    {
        if (wParam == NID_BUYSTORE_START)
            return OnStart() != 0;
        if (wParam == NID_BUYSTORE_UP)
            return OnUp() != 0;
        if (wParam == NID_BUYSTORE_DOWN)
            return OnDown() != 0;
        if (wParam == NID_BUYSTORE_CLOSE)
            return OnClose() != 0;
        if (wParam == NID_BUYSTORE_MESSAGE)
            return OnMessage() != 0;
        if (wParam == NID_BUYSTORE_ABORT)
            return OnAbort() != 0;
    }
    else if (Msg == NWM_BUYSTORE_SELL_OK)
        return OnSellConfirmed(wParam, lParam) != 0;
    return 0;
}
NW_HOOK_METHOD(0x100b9140, NCPrivateBuyWnd, WindowProc)

// 0x100b92b0 (vslot 43) - also the sell window's: Message and Abort only for one's own store,
// whose button starts it ("OK" at someone's)
void NCPrivateBuyWnd::ShowWindow()
{
    NCWnd::ShowWindow();
    if (Mode == 1)
    {
        AbortButton->ShowWindow();
        MessageButton->ShowWindow();
        StartButton->SetTextID(STR_Start);
    }
    else if (Mode == 0)
    {
        AbortButton->HideWindow();
        MessageButton->HideWindow();
        StartButton->SetTextID(STR_OK);
    }
}
NW_HOOK_METHOD(0x100b92b0, NCPrivateBuyWnd, ShowWindow)

// 0x100b9330 (vslot 70) - closing one's own store also tells the server (NetHandler slot 204)
INT NCPrivateBuyWnd::OnClose()
{
    if (Mode == 1)
        GConsole->RequestPrivateStoreBuyManageCancel();
    Reset();
    HideWindow();
    return 1;
}
NW_HOOK_METHOD(0x100b9330, NCPrivateBuyWnd, OnClose)

// 0x100b9360 (vslot 72) - Abort: the store closes (NetHandler slot 205)
INT NCPrivateBuyWnd::OnAbort()
{
    GConsole->RequestPrivateStoreQuitBuy();
    Reset();
    HideWindow();
    return 1;
}
NW_HOOK_METHOD(0x100b9360, NCPrivateBuyWnd, OnAbort)

// 0x100b9a30 (vslot 55) - NCPrivateShopWnd::OnPaint with the buy store's labels. No guard in the stock code; the
// buffers are the stock frame's 128 characters (C1: 1024).
INT NCPrivateBuyWnd::OnPaint(NCGDevice* Device)
{
    Device->BeginPaint(ScreenX, ScreenY);
    Device->SetClip(ScreenX, ScreenY, Width, Height);
    if (BackTex)
        Device->DrawTexture(0, 21, 240, 344, 0, 0, 240, 344, BackTex);
    SIZE Size;
    if (Mode == 0)
    {
        Device->DrawText(15, 25, 0xFFFFFFFF, *SysString(STR_ForSale), 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
        Device->DrawText(15, 159, 0xFFFFFFFF, *SysString(STR_ItemsOnSale), 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
        NCGDevice::GetTextExtent(&Size, *SysString(STR_PriceSell), 0, 0, 0, 0, 0);
        Device->DrawText(132 - Size.cx, 294, 0xFFFFFFFF, *SysString(STR_PriceSell), 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
        NCGDevice::GetTextExtent(&Size, *SysString(STR_Adena), 0, 0, 0, 0, 0);
        Device->DrawText(132 - Size.cx, 312, 0xFFFFFFFF, *SysString(STR_Adena), 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
    }
    else if (Mode == 1)
    {
        Device->DrawText(15, 25, 0xFFFFFFFF, *SysString(STR_Inventory), 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
        Device->DrawText(15, 159, 0xFFFFFFFF, *SysString(STR_WishList), 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
        NCGDevice::GetTextExtent(&Size, *SysString(STR_Price), 0, 0, 0, 0, 0);
        Device->DrawText(132 - Size.cx, 294, 0xFFFFFFFF, *SysString(STR_Price), 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
        NCGDevice::GetTextExtent(&Size, *SysString(STR_Adena), 0, 0, 0, 0, 0);
        Device->DrawText(132 - Size.cx, 312, 0xFFFFFFFF, *SysString(STR_Adena), 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
    }

    // "%d" takes the total's low half, as in the stock code
    TCHAR Buf[128];
    TCHAR Total[128];
    TCHAR AdenaText[128];
    appSprintf(Buf, TEXT("%d"), TotalPrice);
    FormatNumber(Total, Buf);
    appSprintf(Buf, TEXT("%s"), Total);
    NCGDevice::GetTextExtent(&Size, Buf, 0, 0, 0, 0, 0);
    Device->DrawText(222 - Size.cx, 294, 0xFFFFFFFF, Buf, 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
    appSprintf(Buf, TEXT("%d"), Adena);
    FormatNumber(AdenaText, Buf);
    appSprintf(Buf, TEXT("%s"), AdenaText);
    NCGDevice::GetTextExtent(&Size, Buf, 0, 0, 0, 0, 0);
    Device->DrawText(222 - Size.cx, 311, 0xFFFFFFFF, Buf, 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
    Device->EndPaint();
    Device->ReleaseClip();
    return 1;
}
NW_HOOK_METHOD(0x100b9a30, NCPrivateBuyWnd, OnPaint)

// 0x100bc8a0 (vslot 37) - also the sell window's
INT NCPrivateBuyWnd::OnDestroy()
{
    DestroyStoreChild(this, PlayerItems);
    DestroyStoreChild(this, StoreItems);
    DestroyStoreChild(this, UpButton);
    DestroyStoreChild(this, DownButton);
    DestroyStoreChild(this, StartButton);
    DestroyStoreChild(this, UnknownBC);
    DestroyStoreChild(this, MessageButton);
    DestroyStoreChild(this, AbortButton);
    NCWnd::OnDestroy();
    return 1;
}
NW_HOOK_METHOD(0x100bc8a0, NCPrivateBuyWnd, OnDestroy)

// 0x100b9fa0 - Start: the store opens with the wish list (its count, then the item id, enchant, count and price of
// each; NetHandler slot 203). OK at someone's store: the items chosen are sold (the owner's object id first, then
// object id, item id, enchant, count and price; slot 208), unusual prices asking first (C2: the box takes no type of its
// own).
INT NCPrivateBuyWnd::OnStart()
{
    if (Mode == 1)
    {
        INT Count = 0;
        for (INT i = 0; i < StoreItems->Items.Num(); i++)
            if (StoreItems->Items(i)->TradeCount > 0)
                Count++;
        L2ParamStack Params(Count * 4 + 1);
        Params.PushBack((void*)(INT_PTR)Count);
        for (INT i = 0; i < StoreItems->Items.Num(); i++)
        {
            NItemEntry* Item = StoreItems->Items(i);
            if (Item->TradeCount > 0)
            {
                Params.PushBack((void*)(INT_PTR)Item->ID);
                Params.PushBack((void*)(INT_PTR)Item->Enchant);
                Params.PushBack((void*)(INT_PTR)Item->TradeCount);
                Params.PushBack((void*)(INT_PTR)Item->UnknownC20);
            }
        }
        GConsole->RequestSetPrivateStoreListBuy(Params);
        Reset();
        HideWindow();
    }
    else if (Mode == 0)
    {
        INT Count = 0;
        for (INT i = 0; i < PlayerItems->Items.Num(); i++)
            if (PlayerItems->Items(i)->TradeCount > 0)
                Count++;
        L2ParamStack Params(Count * 5 + 2);
        Params.PushBack((void*)(INT_PTR)OwnerID);
        Params.PushBack((void*)(INT_PTR)Count);
        UBOOL bUsual = 1;
        for (INT i = 0; i < PlayerItems->Items.Num(); i++)
        {
            NItemEntry* Item = PlayerItems->Items(i);
            if (Item->TradeCount > 0)
            {
                if (IsUnusualPrice(Item))
                    bUsual = 0;
                Params.PushBack((void*)(INT_PTR)Item->ObjectID);
                Params.PushBack((void*)(INT_PTR)Item->ID);
                Params.PushBack((void*)(INT_PTR)Item->Enchant);
                Params.PushBack((void*)(INT_PTR)Item->TradeCount);
                Params.PushBack((void*)(INT_PTR)Item->UnknownC20);
            }
        }
        if (bUsual)
        {
            GConsole->RequestPrivateStoreSell(Params);
            Reset();
            HideWindow();
        }
        else
        {
            NDialogInfo Info;
            Info.ID = DIALOG_BUYSTORE_SELL_CAUTION;
            Info.Owner = Entry;
            Info.bMainBox = 1;
            Info.bInput = 0;
            Info.SetSystemMessage(MSG_PriceCaution);
            Info.bLargeAmount = 0;
            GConsole->ShowDialog(&Info);
        }
    }
    return 1;
}
NW_HOOK_METHOD(0x100b9fa0, NCPrivateBuyWnd, OnStart)

// 0x100ba340 - also the sell window's: the list's price, each entry's price times its count, in 64 bits
void NCPrivateBuyWnd::UpdateTotal()
{
    TotalPrice = 0;
    if (Mode == 0)
    {
        for (INT i = 0; i < PlayerItems->Items.Num(); i++)
            TotalPrice += (SQWORD)PlayerItems->Items(i)->GetUnknownC20() * PlayerItems->Items(i)->TradeCount;
    }
    else if (Mode == 1)
    {
        for (INT i = 0; i < StoreItems->Items.Num(); i++)
            TotalPrice += (SQWORD)StoreItems->Items(i)->GetUnknownC20() * StoreItems->Items(i)->TradeCount;
    }
}
NW_HOOK_METHOD(0x100ba340, NCPrivateBuyWnd, UpdateTotal)

// 0x100ba430 - Message: the store's message, typed in the desktop's box (dialog 0x2c hands it to the console); C2 puts
// the message the player's pawn has now in the box (the FString at +1488 of the pawn)
INT NCPrivateBuyWnd::OnMessage()
{
    NDialogInfo Info;
    Info.ID = DIALOG_BUYSTORE_MESSAGE;
    Info.Owner = PlayerItems->Entry;
    Info.bMainBox = 1;
    Info.bInput = 1;
    Info.SetSystemMessage(MSG_StoreMessage);
    Info.bLargeAmount = 0;
    Info.Type = 0;
    const TCHAR* Text = NULL;
    User* Me = GConsole->GetMyUser();
    if (Me && Me->Pawn)
        Text = *nw::Field<FString>(Me->Pawn, 0x1488);
    Info.SetText60(Text);
    GConsole->ShowDialog(&Info);
    return 1;
}
NW_HOOK_METHOD(0x100ba430, NCPrivateBuyWnd, OnMessage)

// 0x100ba520 (vslot 73) - selling at the unusual prices confirmed
INT NCPrivateBuyWnd::OnSellConfirmed(WPARAM wParam, LPARAM lParam)
{
    if (Mode == 0)
    {
        INT Count = 0;
        for (INT i = 0; i < PlayerItems->Items.Num(); i++)
            if (PlayerItems->Items(i)->TradeCount > 0)
                Count++;
        L2ParamStack Params(Count * 5 + 2);
        Params.PushBack((void*)(INT_PTR)OwnerID);
        Params.PushBack((void*)(INT_PTR)Count);
        for (INT i = 0; i < PlayerItems->Items.Num(); i++)
        {
            NItemEntry* Item = PlayerItems->Items(i);
            if (Item->TradeCount > 0)
            {
                Params.PushBack((void*)(INT_PTR)Item->ObjectID);
                Params.PushBack((void*)(INT_PTR)Item->ID);
                Params.PushBack((void*)(INT_PTR)Item->Enchant);
                Params.PushBack((void*)(INT_PTR)Item->TradeCount);
                Params.PushBack((void*)(INT_PTR)Item->UnknownC20);
            }
        }
        GConsole->RequestPrivateStoreSell(Params);
    }
    Reset();
    HideWindow();
    return 1;
}
NW_HOOK_METHOD(0x100ba520, NCPrivateBuyWnd, OnSellConfirmed)

// 0x100bad60 - both grids take the mode; they move to its places
void NCPrivateBuyWnd::SetMode(INT InMode)
{
    Mode = InMode;
    StoreItems->SetMode(Mode);
    PlayerItems->SetMode(Mode);
    UpdateLayout();
}
NW_HOOK_METHOD(0x100bad60, NCPrivateBuyWnd, SetMode)

// 0x100bada0 - one's own list adds up again
void NCPrivateBuyWnd::AddStoreItem(NItemInfo* Info)
{
    StoreItems->AddItem(Info);
    if (Mode == 1)
        UpdateTotal();
}
NW_HOOK_METHOD(0x100bada0, NCPrivateBuyWnd, AddStoreItem)

// 0x100badd0
UBOOL NCPrivateBuyWnd::AddPlayerItem(NItemInfo* Info)
{
    return PlayerItems->AddItem(Info);
}
NW_HOOK_METHOD(0x100badd0, NCPrivateBuyWnd, AddPlayerItem)

// 0x100bade0 - the up arrow: the selected item of the lower grid goes up (a stack of more than one asks how many; at
// someone's store an item the player lacks stays)
INT NCPrivateBuyWnd::OnUp()
{
    if (Mode == 0)
    {
        INT Sel = PlayerItems->GetSelected();
        if (Sel >= 0 && Sel < PlayerItems->Items.Num())
        {
            NItemEntry* Item = PlayerItems->Items(Sel);
            if (Item && Item->Count > 0)
            {
                if (Item->IsStackable() && Item->TradeCount > 1)
                    AskAmount(PlayerItems, Item, MSG_MoveHowMany, Item->TradeCount);
                else
                    PlayerItems->MoveItem(Item, 1);
            }
        }
    }
    else if (Mode == 1)
    {
        INT Sel = StoreItems->GetSelected();
        if (Sel >= 0 && Sel < StoreItems->Items.Num())
        {
            NItemEntry* Item = StoreItems->Items(Sel);
            if (Item)
            {
                if (Item->IsStackable() && Item->TradeCount > 1)
                    AskAmount(StoreItems, Item, MSG_BuyHowMany2, Item->TradeCount);
                else
                    StoreItems->MoveItem(Item, 1);
            }
        }
    }
    return 1;
}
NW_HOOK_METHOD(0x100bade0, NCPrivateBuyWnd, OnUp)

// 0x100bb080 - the down arrow: at someone's store the selected item comes down (a stack asks how many; one the player
// lacks stays); setting one's own up, the price first
INT NCPrivateBuyWnd::OnDown()
{
    if (Mode == 0)
    {
        INT Sel = StoreItems->GetSelected();
        if (Sel >= 0 && Sel < StoreItems->Items.Num())
        {
            NItemEntry* Item = StoreItems->Items(Sel);
            if (Item && Item->Count > 0)
            {
                if (Item->IsStackable())
                    AskAmount(StoreItems, Item, MSG_MoveHowMany, Item->TradeCount);
                else
                    StoreItems->MoveItem(Item, 1);
            }
        }
    }
    else if (Mode == 1)
    {
        INT Sel = PlayerItems->GetSelected();
        if (Sel >= 0 && Sel < PlayerItems->Items.Num() && PlayerItems->Items(Sel))
            AskPrice(PlayerItems);
    }
    return 1;
}
NW_HOOK_METHOD(0x100bb080, NCPrivateBuyWnd, OnDown)

// 0x100bbb20 (vslot 36) - NCPrivateShopWnd::OnCreate with the buy grids (160 items at most, C1: 80)
INT NCPrivateBuyWnd::OnCreate()
{
    NCWnd::OnCreate();
    BackTex = FindTexture(TEXT("L2UI.StoreWnd.StoreBack"));

    FrameCtrl = new NCFrameCtrl(1);
    FrameID = NID_BUYSTORE_FRAME;
    FrameCtrl->ID = NID_BUYSTORE_FRAME;
    FrameCtrl->SetWindowRect(0, 0, 240, 21);
    CreateChild(FrameCtrl, NWS_VISIBLE);

    StoreItems = new NCPrivateBuyItemWnd(6, 3, 160, 32);
    StoreItems->SetWindowRect(17, 180, 222, 102);
    CreateChild(StoreItems, NWS_VISIBLE | 0x20);   // 0x20: meaning unknown
    StoreItems->SetSide(0);

    PlayerItems = new NCPrivateBuyItemWnd(6, 3, 160, 32);
    PlayerItems->SetWindowRect(17, 46, 222, 102);
    CreateChild(PlayerItems, NWS_VISIBLE | 0x20);
    PlayerItems->SetSide(1);

    StoreItems->SetPartner(PlayerItems);
    PlayerItems->SetPartner(StoreItems);

    UpButton = new NCButton(TEXT("L2UI.StoreWnd.StoreUpBtn1"), TEXT("L2UI.StoreWnd.StoreUpBtn1"), 0);
    UpButton->ID = NID_BUYSTORE_UP;
    UpButton->SetWindowRect(194, 158, 14, 13);
    CreateChild(UpButton, NWS_VISIBLE);

    DownButton = new NCButton(TEXT("L2UI.StoreWnd.StoreDownBtn1"), TEXT("L2UI.StoreWnd.StoreDownBtn2"), 0);
    DownButton->ID = NID_BUYSTORE_DOWN;
    DownButton->SetWindowRect(210, 158, 14, 13);
    CreateChild(DownButton, NWS_VISIBLE);

    StartButton = new NCButton(TEXT("L2UI.Control.DefaultButton"), TEXT("L2UI.Control.DefaultButton_click"), 0);
    StartButton->ID = NID_BUYSTORE_START;
    StartButton->SetWindowRect(164, 331, 67, 19);
    CreateChild(StartButton, NWS_VISIBLE);
    StartButton->SetTextID(STR_Start);

    MessageButton = new NCButton(TEXT("L2UI.Control.DefaultButton"), TEXT("L2UI.Control.DefaultButton_click"), 0);
    MessageButton->ID = NID_BUYSTORE_MESSAGE;
    MessageButton->SetWindowRect(87, 331, 67, 19);
    CreateChild(MessageButton, NWS_VISIBLE);
    MessageButton->SetTextID(STR_Message);

    AbortButton = new NCButton(TEXT("L2UI.Control.DefaultButton"), TEXT("L2UI.Control.DefaultButton_click"), 0);
    AbortButton->ID = NID_BUYSTORE_ABORT;
    AbortButton->SetWindowRect(10, 331, 67, 19);
    CreateChild(AbortButton, NWS_VISIBLE);
    AbortButton->SetTextID(STR_Abort);
    return 1;
}
NW_HOOK_METHOD(0x100bbb20, NCPrivateBuyWnd, OnCreate)

// 0x100bc2c0 (vslot 44) - also the sell window's
void NCPrivateBuyWnd::HideWindow()
{
    Reset();
    NCWnd::HideWindow();
}
NW_HOOK_METHOD(0x100bc2c0, NCPrivateBuyWnd, HideWindow)

// stock body shared with NCPrivateShopWnd's (0x1009d910)
void NCPrivateBuyWnd::SetAdena(INT InAdena)
{
    Adena = InAdena;
}

// stock body shared with NCShopWnd::SetAdena (0x100a5d20); the console sets it
void NCPrivateBuyWnd::SetOwnerID(INT InOwnerID)
{
    OwnerID = InOwnerID;
}

// stock body shared with NCPrivateShopWnd's (0x1009d920)
void NCPrivateBuyWnd::Reset()
{
    OwnerID = -1;
    Adena = 0;
    TotalPrice = 0;
    PlayerItems->Clear();
    StoreItems->Clear();
}

// stock body shared with NCPrivateShopWnd's (0x1009df20)
void NCPrivateBuyWnd::UpdateLayout()
{
    // the stock code fills { LocalX, LocalY, LocalW, LocalH }; SetScreenPos reads x and y
    POINT Origin;
    if (Mode == 0)
    {
        StoreItems->UpdateScrollRange();
        PlayerItems->UpdateScrollRange();
        Origin.x = LocalX;
        Origin.y = LocalY;
        StoreItems->SetScreenPos(16, 46, &Origin);
        PlayerItems->SetScreenPos(16, 180, &Origin);
    }
    else if (Mode == 1)
    {
        StoreItems->UpdateScrollRange();
        PlayerItems->UpdateScrollRange();
        Origin.x = LocalX;
        Origin.y = LocalY;
        StoreItems->SetScreenPos(16, 180, &Origin);
        PlayerItems->SetScreenPos(16, 46, &Origin);
    }
    else
        return;
    StoreItems->SetSize(222, 102);
    PlayerItems->SetSize(222, 102);
}

NW_CHECK_VTABLE(NCPrivateBuyWnd, 0x10122c18, 74)
