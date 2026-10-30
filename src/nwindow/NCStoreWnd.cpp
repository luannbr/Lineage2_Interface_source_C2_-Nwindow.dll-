/*=============================================================================
    NCStoreWnd.cpp - the warehouse window, its item grids and the GM's view of a warehouse.
    Stock: 0x100b38a0 .. 0x100b72ee, 0x1005c630 .. 0x1005c67f (the destructor, emitted with
    NCGMStoreWnd), 0x1005bf30 .. 0x1005bfbe, 0x1005e9b0 .. 0x1005eaaf (NCGMStoreWnd). WindowProc (0x10057b10), OnDestroy
    (0x100a39e0) and SetTarget (0x10057ba0) of the grid and OnDestroy (0x100a6300) of the
    window are copies of bodies other classes keep the addresses of. SetMode (0x100b3880)
    and SetSide (0x100b3890) of the grid are one body each with the shop's and the private
    stores' grids, hooked here.
=============================================================================*/
#include "nwindow/NCStoreWnd.h"
#include "nwindow/NCConsole.h"
#include "nwindow/NCButton.h"
#include "nwindow/NCDialogBox.h"
#include "nwindow/NCFrameCtrl.h"
#include "nwindow/NCGDevice.h"
#include "nwindow/NConsoleWnd.h"
#include "nwindow/NDialogInfo.h"
#include "nwindow/NClasses.gen.h"

enum { DIALOG_STORE_AMOUNT = 3 };    // NDialogInfo::ID (name ours): an amount for the owner
enum { MSG_HowManyToMove = 72 };     // systemmsg "How many $s1(s) do you want to move?"
enum { ITEM_Adena = 57 };
enum { STORE_FEE_PER_SLOT = 30 };    // adena (name ours)

// sysstring labels
enum
{
    STR_StoredItems = 132,
    STR_ItemsToRetrieve = 133,
    STR_Adena = 134,
    STR_Fee = 135,
    STR_Inventory = 138,
    STR_OK = 140,
    STR_Cancel = 141,
};

// the warehouse window
static NCStoreWnd* StoreWnd()
{
    return GDesktop->StoreWnd;
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
// Inline in each stock caller; the name buffer is the stock frame's 64 characters. C2 gives the box the most it may take
// (Unknown74: Max, the entry's TradeCount or Count).
static FORCEINLINE void AskAmount(NCWnd* Owner, NItemEntry* Item, INT Max)
{
    TCHAR Name[64];
    appSprintf(Name, TEXT("%s"), Item->GetName());
    NDialogInfo Info;
    Info.ID = DIALOG_STORE_AMOUNT;
    Info.Type = 3;
    Info.bInput = 1;
    Info.bMainBox = 1;
    Info.Owner = Owner->Entry;
    Info.SetSystemMessage(MSG_HowManyToMove);
    Info.SetButtons(Name, NULL);
    Info.Unknown74 = Max;
    Info.bLargeAmount = Item->ID == ITEM_Adena;
    GConsole->ShowDialog(&Info);
}

/*-----------------------------------------------------------------------------
    NCStoreItemWnd.
-----------------------------------------------------------------------------*/

// 0x100d9d60
NCStoreItemWnd::NCStoreItemWnd()
{
    Target = NULL;
}
NW_HOOK_CTOR(0x100d9d60, NCStoreItemWnd)

// 0x100d9e00
NCStoreItemWnd::NCStoreItemWnd(INT InColumns, INT InVisibleRows, INT InMaxItems, INT InCellSize)
    : NCItemWnd(InColumns, InVisibleRows, InMaxItems, InCellSize)
{
    Target = NULL;
}
NW_HOOK_CTOR_ARGS(0x100d9e00, NCStoreItemWnd, (INT A, INT B, INT C, INT D), (A, B, C, D))

// 0x100d9db0
NW_HOOK_DTOR(0x100d9db0, NCStoreItemWnd)

// stock body shared with NCShopItemWnd::WindowProc (0x100c0e40), which keeps the address: the double click and the
// dialog box's answer
INT NCStoreItemWnd::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
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
INT NCStoreItemWnd::OnDestroy()
{
    NCItemWnd::OnDestroy();
    return 1;
}

// 0x100d8aa0 - the dword at +110 (the other grid): one body with other classes' setters of their +110 (NCInvenItemWnd's
// bEnchantMode among them), which keep the address
void NCStoreItemWnd::SetTarget(NCStoreItemWnd* InTarget)
{
    Target = InTarget;
}
NW_HOOK_METHOD(0x100d8aa0, NCStoreItemWnd, SetTarget)

// the grids' mode (+114): one body with NCHtmlViewer::SetOwner (0x1002f6a0), which keeps the address
void NCStoreItemWnd::SetMode(INT InMode)
{
    Mode = InMode;
}

// 0x100d8ab0 - the same (the private buy window's stock SetMode, 0x10099bf0, keeps edx)
void NCStoreItemWnd::SetSide(INT InSide)
{
    Side = InSide;
}
NW_HOOK_METHOD(0x100d8ab0, NCStoreItemWnd, SetSide)


// 0x100d9e40 (vslot 62) - the carried item dropped onto another warehouse grid goes there (that grid becomes the
// target): how many first for a stack, else one
INT NCStoreItemWnd::OnLButtonUp(WPARAM wParam, LPARAM lParam)
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
        if (E && E->Wnd && E->Wnd->IsA(NClass::NCStoreItemWnd()))
        {
            Target = (NCStoreItemWnd*)E->Wnd;
            NItemEntry* Item = Items(Selected);
            if (Item->IsStackable() && Item->TradeCount > 1)
                AskAmount(this, Item, Item->TradeCount);
            else
                MoveItem(Item, 1);
        }
    }
    bDragging = 0;
    return 1;
}
NW_HOOK_METHOD(0x100d9e40, NCStoreItemWnd, OnLButtonUp)

// 0x100da070 (vslot 73) - the item goes to the target: how many first for a stack, else one
INT NCStoreItemWnd::OnLButtonDblClick(WPARAM wParam, LPARAM lParam)
{
    bNoDrag = 1;
    Selected = HitTest(LOWORD(lParam), HIWORD(lParam));
    if (Selected >= 0 && Selected < Items.Num())
    {
        NItemEntry* Item = Items(Selected);
        if (Item->IsStackable() && Item->TradeCount > 1)
            AskAmount(this, Item, Item->Count);
        else
            MoveItem(Item, 1);
    }
    bDragging = 0;
    return 1;
}
NW_HOOK_METHOD(0x100da070, NCStoreItemWnd, OnLButtonDblClick)

// 0x100da210 (vslot 74) - the amount typed (wParam; at most the entry's) goes to the target (C2 checks that
// something is selected)
INT NCStoreItemWnd::OnDialogResult(WPARAM wParam, LPARAM lParam)
{
    if (Selected >= 0)
    {
        NItemEntry* Item = Items(Selected);
        if (Item && wParam)
        {
            INT Count = (INT)wParam;
            if (Count >= Item->TradeCount)
                Count = Item->TradeCount;
            MoveItem(Item, Count);
        }
    }
    return 1;
}
NW_HOOK_METHOD(0x100da210, NCStoreItemWnd, OnDialogResult)

// 0x100d93c0 - a new entry, its icon from its name ("NWindow.BlackTexture" when missing, written back into Info), its
// amount the item's count
UBOOL NCStoreItemWnd::AddItem(NItemInfo* Info)
{
    if (Items.Num() >= MaxItems)
        return 0;
    Info->Icon = FindTexture(Info->IconName);
    if (!Info->Icon)
        Info->Icon = FindTexture(TEXT("NWindow.BlackTexture"));
    NItemEntry* Item = new NItemEntry(Info);
    Item->TradeCount = Info->Count;
    Item->FillToolTip(2, 0);
    Items.AddItem(Item);
    UpdateScrollRange();
    return 1;
}
NW_HOOK_METHOD(0x100d93c0, NCStoreItemWnd, AddItem)

// 0x100d94d0 - a copy of another grid's entry holding Count
UBOOL NCStoreItemWnd::AddCopy(NItemEntry* Src, INT Count)
{
    if (Items.Num() >= MaxItems)
        return 0;
    NItemEntry* Item = new NItemEntry;
    Item->Copy(Src);
    Item->TradeCount = Count;
    Item->FillToolTip(2, 0);
    Items.AddItem(Item);
    UpdateScrollRange();
    return 1;
}
NW_HOOK_METHOD(0x100d94d0, NCStoreItemWnd, AddCopy)

// a stack joins the target's entry of the same item id when there is one (inline in the
// stock MoveItem)
static FORCEINLINE UBOOL AddToStack(NCStoreItemWnd* Target, NItemEntry* Item, INT Count)
{
    if (!Item->IsStackable())
        return 0;
    for (INT i = 0; i < Target->Items.Num(); i++)
    {
        if (Target->Items(i)->ID == Item->ID)
        {
            Target->Items(i)->TradeCount += Count;
            Target->Items(i)->FillToolTip(2, 0);
            return 1;
        }
    }
    return 0;
}

// 0x100d95b0 - Count of the item go to the target (onto its stack, or as a new entry); what is left stays, an emptied
// entry leaves the grid (not freed, as in the stock code); then the window's fee
void NCStoreItemWnd::MoveItem(NItemEntry* Item, INT Count)
{
    if (!Target)
        return;
    UnselectAll();
    if (!AddToStack(Target, Item, Count))
        Target->AddCopy(Item, Count);
    if (Item->IsStackable() && Item->TradeCount > Count)
    {
        Item->TradeCount -= Count;
        Item->FillToolTip(2, 0);
    }
    else
        Items.Remove(Selected, 1);
    StoreWnd()->UpdateFee();
    Selected = -1;
    UpdateScrollRange();
}
NW_HOOK_METHOD(0x100d95b0, NCStoreItemWnd, MoveItem)

NW_CHECK_VTABLE(NCStoreItemWnd, 0x10127510, 75)

/*-----------------------------------------------------------------------------
    NCStoreWnd.
-----------------------------------------------------------------------------*/

// 0x100d8ba0 - Mode is left as allocated
NCStoreWnd::NCStoreWnd()
{
    InventoryItems = NULL;
    StoredItems = NULL;
    Unknown9C = 0;
    BackTex = NULL;
    UnknownA4 = 0;
    UpButton = NULL;
    DownButton = NULL;
    OKButton = NULL;
    CancelButton = NULL;
    UnknownB8 = NULL;
    Adena = 0;
    Fee = 0;
}
NW_HOOK_CTOR(0x100d8ba0, NCStoreWnd)

// 0x10073bd0
NW_HOOK_DTOR(0x10073bd0, NCStoreWnd)

// 0x1006ed90 (vslot 25) - the buttons; one body with NCShopWnd's and NCDeliverWnd's (and the base of NCGMStoreWnd's)
INT NCStoreWnd::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
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
NW_HOOK_METHOD(0x1006ed90, NCStoreWnd, WindowProc)

// 0x100da250 (vslot 36) - title bar, the grids (6 x 3 icons, 200 items at most: the inventory's on top, the
// warehouse's below), the arrows between them, OK and Cancel
INT NCStoreWnd::OnCreate()
{
    NCWnd::OnCreate();
    BackTex = FindTexture(TEXT("L2UI.StoreWnd.StoreBack"));

    FrameCtrl = new NCFrameCtrl(1);
    FrameID = NID_STORE_FRAME;
    FrameCtrl->ID = NID_STORE_FRAME;
    FrameCtrl->SetWindowRect(0, 0, 240, 21);
    CreateChild(FrameCtrl, NWS_VISIBLE);

    StoredItems = new NCStoreItemWnd(6, 3, 200, 32);
    StoredItems->SetWindowRect(16, 180, 222, 102);
    CreateChild(StoredItems, NWS_VISIBLE | 0x20);   // 0x20: meaning unknown
    StoredItems->SetSide(0);
    InventoryItems = new NCStoreItemWnd(6, 3, 200, 32);
    InventoryItems->SetWindowRect(16, 46, 222, 102);
    CreateChild(InventoryItems, NWS_VISIBLE | 0x20);
    InventoryItems->SetSide(1);
    StoredItems->SetTarget(InventoryItems);
    InventoryItems->SetTarget(StoredItems);

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
NW_HOOK_METHOD(0x100da250, NCStoreWnd, OnCreate)

// stock body shared with NCShopWnd's (0x100a6300)
INT NCStoreWnd::OnDestroy()
{
    DestroyChild(this, InventoryItems);
    DestroyChild(this, StoredItems);
    DestroyChild(this, UpButton);
    DestroyChild(this, DownButton);
    DestroyChild(this, OKButton);
    DestroyChild(this, CancelButton);
    DestroyChild(this, UnknownB8);
    NCWnd::OnDestroy();
    return 1;
}

// 0x100d9080 (vslot 43)
void NCStoreWnd::ShowWindow()
{
    UpdateLayout();
    NCWnd::ShowWindow();
}
NW_HOOK_METHOD(0x100d9080, NCStoreWnd, ShowWindow)

// 0x100d8b30 (vslot 44)
void NCStoreWnd::HideWindow()
{
    Clear();
    NCWnd::HideWindow();
}
NW_HOOK_METHOD(0x100d8b30, NCStoreWnd, HideWindow)

// 0x100d8c00 (vslot 55) - the grids' labels by mode (withdrawing, the lower one's is drawn at y 1590, off the window,
// as in the stock code: 0x636 where 0x9F would do), the fee and the adena labels right-aligned at 132, their values
// at 222, thousands separated; the buffers are the stock frame's
INT NCStoreWnd::OnPaint(NCGDevice* Device)
{
    TCHAR Text[128];       // C1: 1024 characters each
    TCHAR FeeText[128];
    TCHAR AdenaText[128];
    SIZE Size;
    Device->BeginPaint(ScreenX, ScreenY);
    Device->SetClip(ScreenX, ScreenY, Width, Height);
    if (BackTex)
        Device->DrawTexture(0, 21, 240, 344, 0, 0, 240, 344, BackTex);
    if (Mode == STOREMODE_Deposit)
    {
        Device->DrawText(15, 25, 0xFFFFFFFF, *SysString(STR_Inventory), 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
        Device->DrawText(15, 159, 0xFFFFFFFF, *SysString(STR_StoredItems), 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
    }
    else if (Mode == STOREMODE_Withdraw)
    {
        Device->DrawText(15, 25, 0xFFFFFFFF, *SysString(STR_StoredItems), 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
        Device->DrawText(15, 1590, 0xFFFFFFFF, *SysString(STR_ItemsToRetrieve), 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
    }
    NCGDevice::GetTextExtent(&Size, *SysString(STR_Fee), 0, 0, 0, 0, 0);
    Device->DrawText(132 - Size.cx, 294, 0xFFFFFFFF, *SysString(STR_Fee), 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
    NCGDevice::GetTextExtent(&Size, *SysString(STR_Adena), 0, 0, 0, 0, 0);
    Device->DrawText(132 - Size.cx, 312, 0xFFFFFFFF, *SysString(STR_Adena), 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);

    appSprintf(Text, TEXT("%d"), Fee);
    FormatNumber(FeeText, Text);
    appSprintf(Text, TEXT("%s"), FeeText);
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
NW_HOOK_METHOD(0x100d8c00, NCStoreWnd, OnPaint)

// the ids and amounts of a grid's entries that hold something: count, then (id, amount) each
// (inline twice in the stock OnOK)
static FORCEINLINE void PushItemList(L2ParamStack& Params, NCStoreItemWnd* Grid, INT Num)
{
    Params.PushBack((void*)(INT_PTR)Num);
    for (INT i = 0; i < Grid->Items.Num(); i++)
    {
        if (Grid->Items(i)->TradeCount > 0)
        {
            Params.PushBack((void*)(INT_PTR)Grid->Items(i)->UnknownC34);
            Params.PushBack((void*)(INT_PTR)Grid->Items(i)->TradeCount);
        }
    }
}

// the entries of a grid that hold something
static FORCEINLINE INT CountFilled(NCStoreItemWnd* Grid)
{
    INT Num = 0;
    for (INT i = 0; i < Grid->Items.Num(); i++)
        if (Grid->Items(i)->TradeCount > 0)
            Num++;
    return Num;
}

// 0x100d9090 (vslot 67) - the lower grid's list to the server: deposited (the warehouse's side) or withdrawn (the
// inventory's side; any mode but deposit counts as withdraw and becomes 1); the window closes
INT NCStoreWnd::OnOK()
{
    if (Mode == STOREMODE_Deposit)
    {
        INT Num = CountFilled(StoredItems);
        L2ParamStack Params(Num * 3 + 2);
        PushItemList(Params, StoredItems, Num);
        GConsole->RequestWarehouseDeposit(Params);
    }
    else
    {
        Mode = STOREMODE_Withdraw;
        INT Num = CountFilled(InventoryItems);
        L2ParamStack Params(Num * 3 + 2);
        PushItemList(Params, InventoryItems, Num);
        GConsole->RequestWarehouseWithdraw(Params);
    }
    HideWindow();
    return 1;
}
NW_HOOK_METHOD(0x100d9090, NCStoreWnd, OnOK)

// 0x100d8b80 (vslot 68)
INT NCStoreWnd::OnCancel()
{
    Clear();
    HideWindow();
    return 1;
}
NW_HOOK_METHOD(0x100d8b80, NCStoreWnd, OnCancel)

// vslot 66 - OnCancel's stock body (0x100b5890 serves both slots)
INT NCStoreWnd::OnClose()
{
    Clear();
    HideWindow();
    return 1;
}

// the arrows: the selected item of Grid goes to its target, how many first for a stack
// (inline in the stock OnUp / OnDown, twice each)
static FORCEINLINE void MoveSelected(NCStoreItemWnd* Grid)
{
    INT Sel = Grid->GetSelected();
    if (Sel < 0 || Sel >= Grid->Items.Num())
        return;
    NItemEntry* Item = Grid->Items(Sel);
    if (!Item)
        return;
    if (Item->IsStackable() && Item->TradeCount > 1)
        AskAmount(Grid, Item, Item->Count);
    else
        Grid->MoveItem(Item, 1);
}

// 0x100d9700 (vslot 69) - the up arrow: from the lower grid (the inventory's side when withdrawing, the warehouse's
// when depositing)
INT NCStoreWnd::OnUp()
{
    if (Mode == STOREMODE_Withdraw)
        MoveSelected(InventoryItems);
    else if (Mode == STOREMODE_Deposit)
        MoveSelected(StoredItems);
    return 1;
}
NW_HOOK_METHOD(0x100d9700, NCStoreWnd, OnUp)

// 0x100d9a30 (vslot 70) - the down arrow: from the upper grid
INT NCStoreWnd::OnDown()
{
    if (Mode == STOREMODE_Withdraw)
        MoveSelected(StoredItems);
    else if (Mode == STOREMODE_Deposit)
        MoveSelected(InventoryItems);
    return 1;
}
NW_HOOK_METHOD(0x100d9a30, NCStoreWnd, OnDown)

// 0x100d8ac0
void NCStoreWnd::SetMode(INT InMode)
{
    Mode = InMode;
    StoredItems->SetMode(Mode);
    InventoryItems->SetMode(Mode);
}
NW_HOOK_METHOD(0x100d8ac0, NCStoreWnd, SetMode)

// 0x100d8af0 - one body with NCShopWnd's list id setter (+0xC0 there too)
void NCStoreWnd::SetAdena(INT InAdena)
{
    Adena = InAdena;
}
NW_HOOK_METHOD(0x100d8af0, NCStoreWnd, SetAdena)

// 0x100d8b00 - kept out of line like the stock code (HideWindow, OnCancel)
__declspec(noinline) void NCStoreWnd::Clear()
{
    Adena = 0;
    Fee = 0;
    InventoryItems->Clear();
    StoredItems->Clear();
}
NW_HOOK_METHOD(0x100d8b00, NCStoreWnd, Clear)

// 0x100d8b40 - 30 adena for each slot deposited; withdrawing is free
void NCStoreWnd::UpdateFee()
{
    Fee = 0;
    if (Mode == STOREMODE_Withdraw)
        Fee = 0;
    else if (Mode == STOREMODE_Deposit)
        Fee = StoredItems->Items.Num() * STORE_FEE_PER_SLOT;
}
NW_HOOK_METHOD(0x100d8b40, NCStoreWnd, UpdateFee)

// 0x100d8b70
INT NCStoreWnd::GetMode()
{
    return Mode;
}
NW_HOOK_METHOD(0x100d8b70, NCStoreWnd, GetMode)

// 0x100d8f80 (out of line like the stock code) - depositing, the inventory's grid on top and
// the warehouse's below; withdrawing, the other way round
__declspec(noinline) void NCStoreWnd::UpdateLayout()
{
    if (Mode == STOREMODE_Withdraw)
    {
        StoredItems->UpdateScrollRange();
        InventoryItems->UpdateScrollRange();
        // the stock code fills { LocalX, LocalY, LocalW, LocalH }; SetScreenPos reads x and y
        POINT Origin;
        Origin.x = LocalX;
        Origin.y = LocalY;
        StoredItems->SetScreenPos(16, 46, &Origin);
        InventoryItems->SetScreenPos(16, 180, &Origin);
    }
    else if (Mode == STOREMODE_Deposit)
    {
        StoredItems->UpdateScrollRange();
        InventoryItems->UpdateScrollRange();
        POINT Origin;
        Origin.x = LocalX;
        Origin.y = LocalY;
        StoredItems->SetScreenPos(16, 180, &Origin);
        InventoryItems->SetScreenPos(16, 46, &Origin);
    }
    else
        return;
    StoredItems->SetSize(222, 102);
    InventoryItems->SetSize(222, 102);
}
NW_HOOK_METHOD(0x100d8f80, NCStoreWnd, UpdateLayout)

// one grid's entries get their name and description again from the item tables, and their
// tool tips (the stock code repeats this for each grid)
static FORCEINLINE void ReloadItemTexts(NCStoreItemWnd* Grid)
{
    for (INT i = 0; i < Grid->Items.Num(); i++)
    {
        NItemEntry* Item = Grid->Items(i);
        if (Item)
        {
            appStrcpy(Item->Name, GL2GameData.GetItemName(Item->ID));
            appStrcpy(Item->Description, GL2GameData.GetItemDescription(Item->ID));
            Item->FillToolTip(2, 0);
        }
    }
}

// 0x100d92a0 - after the item texts were reloaded (NConsoleWnd::ReloadGameTexts)
void NCStoreWnd::ReloadTexts()
{
    ReloadItemTexts(StoredItems);
    ReloadItemTexts(InventoryItems);
}
NW_HOOK_METHOD(0x100d92a0, NCStoreWnd, ReloadTexts)

// 0x100d96e0
UBOOL NCStoreWnd::AddInventoryItem(NItemInfo* Info)
{
    return InventoryItems->AddItem(Info);
}
NW_HOOK_METHOD(0x100d96e0, NCStoreWnd, AddInventoryItem)

// 0x100d96f0
UBOOL NCStoreWnd::AddStoredItem(NItemInfo* Info)
{
    return StoredItems->AddItem(Info);
}
NW_HOOK_METHOD(0x100d96f0, NCStoreWnd, AddStoredItem)

NW_CHECK_VTABLE(NCStoreWnd, 0x10116a18, 72)

/*-----------------------------------------------------------------------------
    NCGMStoreWnd.
-----------------------------------------------------------------------------*/

// 0x10076650
NCGMStoreWnd::NCGMStoreWnd()
{
}
NW_HOOK_CTOR(0x10076650, NCGMStoreWnd)

// 0x100766a0
NW_HOOK_DTOR(0x100766a0, NCGMStoreWnd)

// 0x10073110 (vslot 25) - NCStoreWnd's, as a plain 0 / 1
INT NCGMStoreWnd::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    return NCStoreWnd::WindowProc(Msg, wParam, lParam) != 0;
}
NW_HOOK_METHOD(0x10073110, NCGMStoreWnd, WindowProc)

// 0x10073130 (vslot 36) - the warehouse window with its buttons disabled: a GM only looks
INT NCGMStoreWnd::OnCreate()
{
    NCStoreWnd::OnCreate();
    UpButton->EnableWindow(0);
    DownButton->EnableWindow(0);
    OKButton->EnableWindow(0);
    CancelButton->EnableWindow(0);
    return 1;
}
NW_HOOK_METHOD(0x10073130, NCGMStoreWnd, OnCreate)

// 0x100766f0 (vslot 55) - NCStoreWnd's inside a clip and origin of its own (set before and
// cleared after, as in the stock code)
INT NCGMStoreWnd::OnPaint(NCGDevice* Device)
{
    Device->SetClip(ScreenX, ScreenY, Width, Height);
    Device->BeginPaint(ScreenX, ScreenY);
    NCStoreWnd::OnPaint(Device);
    Device->EndPaint();
    Device->ReleaseClip();
    return 1;
}
NW_HOOK_METHOD(0x100766f0, NCGMStoreWnd, OnPaint)

NW_CHECK_VTABLE(NCGMStoreWnd, 0x10117390, 72)
