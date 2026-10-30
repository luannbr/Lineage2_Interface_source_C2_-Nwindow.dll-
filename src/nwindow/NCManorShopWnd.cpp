/*=============================================================================
    NCManorShopWnd.cpp - the manor's seed and crop shop (C2 only; a copy of NCShopWnd).
    Stock: 0x10092280 .. 0x10092ca0 (WindowProc, the setters, Clear, HideWindow, OnCancel, constructor, destructor,
    OnPaint, OnOK, OnDestroy), 0x10092ca0 .. 0x10092e90 (UpdateLayout, UpdateTotalPrice), 0x10093290 .. 0x10093960
    (SetMode, AddShopItem, AddPlayerItem, OnUp, OnDown), 0x100940f0 .. 0x100945d0 (OnCreate); the grids' functions
    among them are in NCManorShopItemWnd.cpp. The scalar deleting destructors (0x10092410, and the shop's 0x100923c0
    that sits before it) are not hooked. The shop's own destructor (0x10092370, NCShopWnd::~NCShopWnd) lies in the same
    range and is called by this one.
=============================================================================*/
#include "nwindow/NCManorShopWnd.h"
#include "nwindow/NCConsole.h"
#include "nwindow/NCButton.h"
#include "nwindow/NCDialogBox.h"
#include "nwindow/NCFrameCtrl.h"
#include "nwindow/NCGDevice.h"
#include "nwindow/NCStoreWnd.h"
#include "nwindow/NClasses.gen.h"
#include "nwindow/NConsoleWnd.h"
#include "nwindow/NDialogInfo.h"

enum { DIALOG_SHOP_AMOUNT = 2 };     // NDialogInfo::ID (name ours): an amount for the owner (the shop's)
enum { MSG_HowManyToMove = 72 };     // systemmsg "How many $s1(s) do you want to move?"
enum { ITEM_Adena = 57 };

// sysstring labels (the shop's)
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
};

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

// 0x100923f0 - the shop's constructor sets the inherited fields; this window's own are left as allocated
NCManorShopWnd::NCManorShopWnd()
{
}
NW_HOOK_CTOR(0x100923f0, NCManorShopWnd)

// 0x10092440 (the shop's destructor, 0x10092370, follows)
NW_HOOK_DTOR(0x10092440, NCManorShopWnd)

// 0x10092280 (vslot 25) - the shop's window procedure (stock 0x1006ed90, shared with the warehouse's; it answers the
// OK button itself), then the OK button again: the same call, never reached
INT NCManorShopWnd::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (NCShopWnd::WindowProc(Msg, wParam, lParam))
        return 1;
    if (Msg == NWM_COMMAND && wParam == NID_STORE_OK)
        return OnOK() != 0;
    return 0;
}
NW_HOOK_METHOD(0x10092280, NCManorShopWnd, WindowProc)

// 0x100922e0 (name ours) - the console sets the list id the request sends back
void NCManorShopWnd::SetListID(INT InListID)
{
    ListID = InListID;
}
NW_HOOK_METHOD(0x100922e0, NCManorShopWnd, SetListID)

// 0x100922f0 (name ours)
void NCManorShopWnd::SetAdena(INT InAdena)
{
    Adena = InAdena;
}
NW_HOOK_METHOD(0x100922f0, NCManorShopWnd, SetAdena)

// 0x10092300 (name ours) - kept out of line like the stock code (HideWindow, OnCancel); the grids' Clear is virtual 70
__declspec(noinline) void NCManorShopWnd::Clear()
{
    ListID = -1;
    Adena = 0;
    TotalPrice = 0;
    PlayerItems->Clear();
    ShopItems->Clear();
}
NW_HOOK_METHOD(0x10092300, NCManorShopWnd, Clear)

// 0x10092340 (vslot 44)
void NCManorShopWnd::HideWindow()
{
    Clear();
    NCWnd::HideWindow();
}
NW_HOOK_METHOD(0x10092340, NCManorShopWnd, HideWindow)

// 0x10092350 (vslot 68)
INT NCManorShopWnd::OnCancel()
{
    Clear();
    HideWindow();
    return 1;
}
NW_HOOK_METHOD(0x10092350, NCManorShopWnd, OnCancel)

// vslot 71 - OnCancel's stock body (0x10092350 serves both slots)
INT NCManorShopWnd::OnClose()
{
    Clear();
    HideWindow();
    return 1;
}

// 0x10092490 (vslot 55) - the grids' labels by mode, the total's and the adena's labels right-aligned at 132, their
// values at 222, thousands separated (the total, 64 bits, through "%d": its low half); the buffers are the stock
// frame's. The shop's OnPaint on this window's fields.
INT NCManorShopWnd::OnPaint(NCGDevice* Device)
{
    TCHAR Text[128];
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
NW_HOOK_METHOD(0x10092490, NCManorShopWnd, OnPaint)

// the entries of a grid that hold something
static FORCEINLINE INT CountFilled(NCManorShopItemWnd* Grid)
{
    INT Num = 0;
    for (INT i = 0; i < Grid->Items.Num(); i++)
        if (Grid->Items(i)->TradeCount > 0)
            Num++;
    return Num;
}

// 0x10092900 (vslot 67) - the lower grid's list to the server, through the shop's requests (RequestSellItem /
// RequestBuyItem): list id, count, then (object id, item id, amount) of each item sold or (item id, amount) of each
// item bought; the window closes
INT NCManorShopWnd::OnOK()
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
NW_HOOK_METHOD(0x10092900, NCManorShopWnd, OnOK)

// 0x10092b50 (vslot 37) - the children go, in field order, then the base
INT NCManorShopWnd::OnDestroy()
{
    DestroyChild(this, PlayerItems);
    DestroyChild(this, ShopItems);
    DestroyChild(this, UpButton);
    DestroyChild(this, DownButton);
    DestroyChild(this, OKButton);
    DestroyChild(this, CancelButton);
    DestroyChild(this, UnknownF8);
    NCWnd::OnDestroy();
    return 1;
}
NW_HOOK_METHOD(0x10092b50, NCManorShopWnd, OnDestroy)

// 0x10092ca0 (name ours; out of line like the stock code) - buying, the NPC's grid on top and the purchases below;
// selling, the inventory on top and what is sold below
__declspec(noinline) void NCManorShopWnd::UpdateLayout()
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
NW_HOOK_METHOD(0x10092ca0, NCManorShopWnd, UpdateLayout)

// 0x10092da0 (name ours) - the sum of price x amount over the lower grid, in 64 bits (the grids' MoveItem calls it
// when the shop window is not the one shown)
void NCManorShopWnd::UpdateTotalPrice()
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
NW_HOOK_METHOD(0x10092da0, NCManorShopWnd, UpdateTotalPrice)

// 0x10093290 (name ours) - the grids' mode too (the NPC's first), then the layout
void NCManorShopWnd::SetMode(INT InMode)
{
    Mode = InMode;
    ShopItems->SetMode(Mode);
    PlayerItems->SetMode(Mode);
    UpdateLayout();
}
NW_HOOK_METHOD(0x10093290, NCManorShopWnd, SetMode)

// 0x100932d0 (name ours; a tail jump into the grid's AddItem)
UBOOL NCManorShopWnd::AddShopItem(NItemInfo* Info)
{
    return ShopItems->AddItem(Info);
}
NW_HOOK_METHOD(0x100932d0, NCManorShopWnd, AddShopItem)

// 0x100932e0 (name ours; a tail jump into the grid's AddItem)
UBOOL NCManorShopWnd::AddPlayerItem(NItemInfo* Info)
{
    return PlayerItems->AddItem(Info);
}
NW_HOOK_METHOD(0x100932e0, NCManorShopWnd, AddPlayerItem)

// the arrows: the selected item of Grid goes to its target, how many first for a stack of more than one - or for any
// stack with bAnyStack, the down arrow on the NPC's list, whose stock copy has no count test (inline in the stock OnUp /
// OnDown, twice each)
static FORCEINLINE void MoveSelected(NCManorShopItemWnd* Grid, UBOOL bAnyStack = 0)
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

// 0x100932f0 (vslot 69) - the up arrow: from the lower grid
INT NCManorShopWnd::OnUp()
{
    if (Mode == SHOPMODE_Buy)
        MoveSelected(PlayerItems);
    else if (Mode == SHOPMODE_Sell)
        MoveSelected(ShopItems);
    return 1;
}
NW_HOOK_METHOD(0x100932f0, NCManorShopWnd, OnUp)

// 0x10093630 (vslot 70) - the down arrow: from the upper grid
INT NCManorShopWnd::OnDown()
{
    if (Mode == SHOPMODE_Buy)
        MoveSelected(ShopItems, 1);
    else if (Mode == SHOPMODE_Sell)
        MoveSelected(PlayerItems);
    return 1;
}
NW_HOOK_METHOD(0x10093630, NCManorShopWnd, OnDown)

// 0x100940f0 (vslot 36) - title bar, the grids (6 x 3 icons, 160 items at most: the NPC's on top, the player's
// below), the arrows between them, OK and Cancel: the shop's OnCreate on this window's fields and grids
INT NCManorShopWnd::OnCreate()
{
    NCWnd::OnCreate();
    BackTex = FindTexture(TEXT("L2UI.StoreWnd.StoreBack"));

    FrameCtrl = new NCFrameCtrl(1);
    FrameID = NID_STORE_FRAME;
    FrameCtrl->ID = NID_STORE_FRAME;
    FrameCtrl->SetWindowRect(0, 0, 240, 21);
    CreateChild(FrameCtrl, NWS_VISIBLE);

    ShopItems = new NCManorShopItemWnd(6, 3, 160, 32);
    ShopItems->SetWindowRect(16, 46, 222, 102);
    CreateChild(ShopItems, NWS_VISIBLE | 0x20);   // 0x20: meaning unknown
    ShopItems->SetSide(SHOPSIDE_Shop);
    PlayerItems = new NCManorShopItemWnd(6, 3, 160, 32);
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
NW_HOOK_METHOD(0x100940f0, NCManorShopWnd, OnCreate)

NW_CHECK_VTABLE(NCManorShopWnd, 0x1011bcb8, 72)
