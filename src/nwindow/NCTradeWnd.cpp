/*=============================================================================
    NCTradeWnd.cpp - the trade window and its item grids.
    Stock: 0x100b72f0 .. 0x100b9152, 0x100b9170 .. 0x100ba99e; 0x10057ba0 (SetKind, shared).
    NCTradeItemWnd's WindowProc (0x10057b10) and RemoveItem (0x10093f40) are copies of bodies
    NCInvenItemWnd and NCPetInvenItemWnd keep the addresses of.
=============================================================================*/
#include "nwindow/NCTradeWnd.h"
#include "nwindow/NCButton.h"
#include "nwindow/NCDialogBox.h"
#include "nwindow/NCFrameCtrl.h"
#include "nwindow/NCGDevice.h"
#include "nwindow/NConsoleWnd.h"
#include "nwindow/NDialogInfo.h"
#include "nwindow/NClasses.gen.h"

// NDialogInfo::ID (names ours): both ask the inventory grid for an amount (NCDialogBox::OnOK)
enum
{
    DIALOG_TRADE_ADD = 2,            // the down arrow's question
    DIALOG_TRADE_MOVE = 8,           // the grid's (item carried onto the offer, double click)
};

enum { MSG_HowManyToMove = 72 };     // systemmsg "How many $s1(s) do you want to move?"
enum { ITEM_Adena = 57 };

// sysstring labels
enum
{
    STR_OK = 140,
    STR_Cancel = 141,
};

// NConsoleWnd::RequestAddTradeItem (0x10043e60, guard name): the item offered, by value
static FORCEINLINE void RequestAddTradeItem(NItemRequest Request)
{
    GConsole->RequestAddTradeItem(Request);
}

// how many of a stack to offer (the adena's question allows large amounts); the answer goes to Owner's OnDialogResult.
// Inline in each stock caller; the name buffer is the stock frame's 64 characters. C2 gives the box the most it may take
// (Unknown74: the entry's Count).
static FORCEINLINE void AskTradeAmount(NCWnd* Owner, INT DialogID, NItemEntry* Item)
{
    TCHAR Name[64];
    appSprintf(Name, TEXT("%s"), Item->GetName());
    NDialogInfo Info;
    Info.Type = 3;
    Info.bInput = 1;
    Info.bMainBox = 1;
    Info.ID = DialogID;
    Info.Owner = Owner->Entry;
    Info.SetSystemMessage(MSG_HowManyToMove);
    Info.SetButtons(Name, NULL);
    Info.Unknown74 = Item->Count;
    Info.bLargeAmount = Item->ID == ITEM_Adena;
    GConsole->ShowDialog(&Info);
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

/*-----------------------------------------------------------------------------
    NCTradeItemWnd.
-----------------------------------------------------------------------------*/

// 0x100dba50 - Kind is left as allocated
NCTradeItemWnd::NCTradeItemWnd()
{
}
NW_HOOK_CTOR(0x100dba50, NCTradeItemWnd)

// 0x100dbaf0
NCTradeItemWnd::NCTradeItemWnd(INT InColumns, INT InVisibleRows, INT InMaxItems, INT InCellSize)
    : NCItemWnd(InColumns, InVisibleRows, InMaxItems, InCellSize)
{
}
NW_HOOK_CTOR_ARGS(0x100dbaf0, NCTradeItemWnd, (INT A, INT B, INT C, INT D), (A, B, C, D))

// 0x100dbaa0
NW_HOOK_DTOR(0x100dbaa0, NCTradeItemWnd)

// stock body shared with NCInvenItemWnd's (0x10057b10): the double click and the dialog box's
// answer
INT NCTradeItemWnd::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (Msg != WM_PAINT && !IsEnabled())
        return 0;
    if (NCItemWnd::WindowProc(Msg, wParam, lParam))
        return 1;
    if (Msg == WM_LBUTTONDBLCLK)
        return OnLButtonDblClick(wParam, lParam) != 0;
    if (Msg == NWM_DIALOG_RESULT)
        return OnDialogResult(wParam, lParam) != 0;
    return 0;
}

// the grid's kind (+110): one body with the other grids' setters of their +110 (NCStoreItemWnd::SetTarget, 0x100d8aa0,
// keeps the address)
void NCTradeItemWnd::SetKind(INT InKind)
{
    Kind = InKind;
}


// 0x100da900 (vslot 55) - NCItemWnd's icons; only the inventory grid shows the selection. No
// guard in the stock code.
INT NCTradeItemWnd::OnPaint(NCGDevice* Device)
{
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
        if (!Kind && Item->IsSelected())
            Device->DrawTexture(X - 1, Y - 1, 34, 34, 0, 0, 34, 34, SelectTex);
        if (Item->GetIcon())
            Device->DrawTexture(X, Y, CellSize, CellSize, 0, 0, CellSize, CellSize, Item->GetIcon());
    }
    Device->EndPaint();
    Device->ReleaseClip();
    return 1;
}
NW_HOOK_METHOD(0x100da900, NCTradeItemWnd, OnPaint)

// 0x100daa90 (vslot 60) - NCItemWnd's (the carried item as the cursor), in the inventory grid
// only
INT NCTradeItemWnd::OnMouseMove(WPARAM wParam, LPARAM lParam)
{
    HoverIndex = HitTest(LOWORD(lParam), HIWORD(lParam));
    if (!Kind && IsFocused())
    {
        if (bDragging)
        {
            if (Selected >= 0 && Selected < Items.Num())
            {
                SetCursor(4, Items(Selected)->GetIcon());
                LockCursor();
                return 1;
            }
        }
        else if (!GConsole->IsCtrlDown())
        {
            UnlockCursor();
            SetCursor(0, NULL);
        }
    }
    return 1;
}
NW_HOOK_METHOD(0x100daa90, NCTradeItemWnd, OnMouseMove)

// 0x100dab40 (vslot 62) - an inventory item carried onto the player's offer grid is offered: how many first for a
// stack (the adena's question allows large amounts), else one. The name buffer is the stock frame's 64 characters.
INT NCTradeItemWnd::OnLButtonUp(WPARAM wParam, LPARAM lParam)
{
    if (Kind)
        return 1;
    INT X = LOWORD(lParam);
    INT Y = HIWORD(lParam);
    if (!GConsole->IsCtrlDown())
    {
        UnlockCursor();
        SetCursor(0, NULL);
    }
    if (Selected < 0 || Selected >= Items.Num())
    {
        bDragging = 0;
        return 1;
    }
    if (bDragging && !IsInWindow(X, Y))
    {
        NWndEntry* E = FindWindowAt(X, Y);
        if (E && E->Wnd && E->Wnd->IsA(NClass::NCTradeItemWnd())
            && ((NCTradeItemWnd*)E->Wnd)->Kind == TRADEGRID_MyOffer)
        {
            NItemEntry* Item = Items(Selected);
            if (Item->IsStackable() && Item->TradeCount > 1)
            {
                AskTradeAmount(this, DIALOG_TRADE_MOVE, Item);
                bDragging = 0;
                return 1;
            }
            NItemRequest Request;
            Request.ObjectID = Item->ObjectID;
            Request.Count = 1;
            RequestAddTradeItem(Request);
            UnselectAll();
            Selected = -1;
        }
    }
    bDragging = 0;
    return 1;
}
NW_HOOK_METHOD(0x100dab40, NCTradeItemWnd, OnLButtonUp)

// 0x100dade0 (vslot 73) - the inventory item is offered: how many first for a stack, else one. In the offer grids
// nothing happens and the selection stays.
INT NCTradeItemWnd::OnLButtonDblClick(WPARAM wParam, LPARAM lParam)
{
    bNoDrag = 1;
    bDragging = 0;
    Selected = HitTest(LOWORD(lParam), HIWORD(lParam));
    if (Selected >= 0 && Selected < Items.Num())
    {
        if (Kind)
        {
            bDragging = 0;
            return 1;
        }
        NItemEntry* Item = Items(Selected);
        if (Item->IsStackable() && Item->TradeCount > 1)
            AskTradeAmount(this, DIALOG_TRADE_MOVE, Item);
        else
        {
            NItemRequest Request;
            Request.ObjectID = Item->ObjectID;
            Request.Count = 1;
            RequestAddTradeItem(Request);
            UnselectAll();
        }
    }
    Selected = -1;
    bDragging = 0;
    return 1;
}
NW_HOOK_METHOD(0x100dade0, NCTradeItemWnd, OnLButtonDblClick)

// 0x100db0f0 (vslot 74) - the amount typed (wParam) of the selected item is offered; more than the entry holds is
// refused with a system message (C1 clamped it). The stock code reads Items(Selected) without a bounds check.
INT NCTradeItemWnd::OnDialogResult(WPARAM wParam, LPARAM lParam)
{
    NItemEntry* Item = Items(Selected);
    if (Item && wParam)
    {
        if ((INT)wParam > Item->TradeCount)
        {
            GConsole->ShowSystemMessage(0x40C, -1);   // 0x1006a8c0
            return 1;
        }
        NItemRequest Request;
        Request.ObjectID = Item->ObjectID;
        Request.Count = (INT)wParam;
        RequestAddTradeItem(Request);
        UnselectAll();
        Selected = -1;
    }
    return 1;
}
NW_HOOK_METHOD(0x100db0f0, NCTradeItemWnd, OnDialogResult)

// 0x100dbb20 (vslot 70) - frees the items and drops the selection; unlike NCItemWnd's, the scroll position stays
void NCTradeItemWnd::Clear()
{
    for (INT i = 0; i < Items.Num(); i++)
        if (Items(i))
            delete Items(i);
    Items.Empty();
    Selected = -1;
}
NW_HOOK_METHOD(0x100dbb20, NCTradeItemWnd, Clear)

// 0x100db8c0 - an entry of the same object id that stacks gets the amount added (the full check comes first);
// otherwise a new entry, its icon from its name ("NWindow.BlackTexture" when missing, written back into Info), its
// count the amount
UBOOL NCTradeItemWnd::AddItem(NItemInfo* Info)
{
    if (Items.Num() >= MaxItems)
        return 0;
    for (INT i = 0; i < Items.Num(); i++)
    {
        NItemEntry* Old = Items(i);
        if (Old->ObjectID == Info->ObjectID && Old->IsStackable())
        {
            Old->TradeCount += Info->Count;
            Old->FillToolTip(2, 0);
            return 1;
        }
    }
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
NW_HOOK_METHOD(0x100db8c0, NCTradeItemWnd, AddItem)

// 0x100dafe0 - the entry with the same object id takes Info's values and amount (through a new entry that is never
// freed, as in the stock code)
UBOOL NCTradeItemWnd::UpdateItem(NItemInfo* Info)
{
    for (INT i = 0; i < Items.Num(); i++)
    {
        NItemEntry* Old = Items(i);
        if (Info->ObjectID == Old->ObjectID)
        {
            Info->Icon = FindTexture(Info->IconName);
            if (!Info->Icon)
                Info->Icon = FindTexture(TEXT("NWindow.BlackTexture"));
            NItemEntry* New = new NItemEntry(Info);
            New->TradeCount = Info->Count;
            New->FillToolTip(2, 0);
            Old->Copy(New);
            return 1;
        }
    }
    return 0;
}
NW_HOOK_METHOD(0x100dafe0, NCTradeItemWnd, UpdateItem)

// 0x100dbb90 - the entry of the object id goes and is freed (the pet's inventory grid has its own copy, 0x10093f40)
UBOOL NCTradeItemWnd::RemoveItem(INT ObjectID)
{
    for (INT i = 0; i < Items.Num(); i++)
    {
        if (Items(i)->ObjectID == ObjectID)
        {
            NItemEntry* Item = Items(i);
            Items.Remove(i, 1);
            if (Item)
                delete Item;
            return 1;
        }
    }
    return 0;
}

NW_HOOK_METHOD(0x100dbb90, NCTradeItemWnd, RemoveItem)

NW_CHECK_VTABLE(NCTradeItemWnd, 0x10127790, 75)

/*-----------------------------------------------------------------------------
    NCTradeWnd.
-----------------------------------------------------------------------------*/

// 0x100db290 - the partner's and the player's names
NW_HOOK_DTOR(0x100db290, NCTradeWnd)

// 0x100db1d0 - the names are FStrings now
NCTradeWnd::NCTradeWnd()
{
    InventoryItems = NULL;
    MyItems = NULL;
    PartnerItems = NULL;
    BackTex = NULL;
    AddButton = NULL;
    OKButton = NULL;
    CancelButton = NULL;
}
NW_HOOK_CTOR(0x100db1d0, NCTradeWnd)


// 0x100da740 (vslot 25) - the buttons
INT NCTradeWnd::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (NCWnd::WindowProc(Msg, wParam, lParam))
        return 1;
    if (Msg == NWM_COMMAND)
    {
        if (wParam == NID_TRADE_OK)
            return OnOK() != 0;
        if (wParam == NID_TRADE_CANCEL)
            return OnCancel() != 0;
        if (wParam == NID_TRADE_ADD)
            return OnAdd() != 0;
    }
    return 0;
}
NW_HOOK_METHOD(0x100da740, NCTradeWnd, WindowProc)

// 0x100dbc00 (vslot 36) - title bar, the grids (the inventory 6 x 4 icons, the offers one row each; 160 items at most,
// C1: 80), the down arrow between the inventory and the player's offer, OK and Cancel
INT NCTradeWnd::OnCreate()
{
    NCWnd::OnCreate();
    BackTex = FindTexture(TEXT("L2UI.storeWnd.TradeBack"));

    FrameCtrl = new NCFrameCtrl(1);
    FrameID = NID_TRADE_FRAME;
    FrameCtrl->ID = NID_TRADE_FRAME;
    FrameCtrl->SetWindowRect(0, 0, 240, 21);
    CreateChild(FrameCtrl, NWS_VISIBLE);

    InventoryItems = new NCTradeItemWnd(6, 4, 160, 32);
    InventoryItems->SetWindowRect(16, 33, 220, 137);
    CreateChild(InventoryItems, NWS_VISIBLE | 0x20);   // 0x20: meaning unknown
    InventoryItems->SetKind(TRADEGRID_Inventory);

    MyItems = new NCTradeItemWnd(6, 1, 160, 32);
    MyItems->SetWindowRect(16, 210, 220, 32);
    CreateChild(MyItems, NWS_VISIBLE | 0x20);
    MyItems->SetKind(TRADEGRID_MyOffer);

    PartnerItems = new NCTradeItemWnd(6, 1, 160, 32);
    PartnerItems->SetWindowRect(16, 282, 220, 32);
    CreateChild(PartnerItems, NWS_VISIBLE | 0x20);
    PartnerItems->SetKind(TRADEGRID_PartnerOffer);

    AddButton = new NCButton(TEXT("L2UI.StoreWnd.StoreDownBtn1"), TEXT("L2UI.StoreWnd.StoreDownBtn2"), 0);
    AddButton->ID = NID_TRADE_ADD;
    AddButton->SetWindowRect(210, 183, 14, 13);
    CreateChild(AddButton, NWS_VISIBLE);

    OKButton = new NCButton(TEXT("L2UI.Control.DefaultButton"), TEXT("L2UI.Control.DefaultButton_click"), 0);
    OKButton->ID = NID_TRADE_OK;
    OKButton->SetWindowRect(85, 336, 67, 19);
    CreateChild(OKButton, NWS_VISIBLE);
    OKButton->SetTextID(STR_OK);

    CancelButton = new NCButton(TEXT("L2UI.Control.DefaultButton"), TEXT("L2UI.Control.DefaultButton_click"), 0);
    CancelButton->ID = NID_TRADE_CANCEL;
    CancelButton->SetWindowRect(157, 336, 67, 19);
    CreateChild(CancelButton, NWS_VISIBLE);
    CancelButton->SetTextID(STR_Cancel);
    return 1;
}
NW_HOOK_METHOD(0x100dbc00, NCTradeWnd, OnCreate)

// 0x100db420 (vslot 37)
INT NCTradeWnd::OnDestroy()
{
    DestroyChild(this, InventoryItems);
    DestroyChild(this, MyItems);
    DestroyChild(this, PartnerItems);
    DestroyChild(this, AddButton);
    DestroyChild(this, OKButton);
    DestroyChild(this, CancelButton);
    NCWnd::OnDestroy();
    return 1;
}
NW_HOOK_METHOD(0x100db420, NCTradeWnd, OnDestroy)

// 0x100da830 (vslot 43) - the grids' scroll ranges follow what they got while hidden
void NCTradeWnd::ShowWindow()
{
    NCWnd::ShowWindow();
    InventoryItems->UpdateScrollRange();
    MyItems->UpdateScrollRange();
    PartnerItems->UpdateScrollRange();
}
NW_HOOK_METHOD(0x100da830, NCTradeWnd, ShowWindow)

// stock body shared with NCChatWnd's (0x1003dab0)
void NCTradeWnd::HideWindow()
{
    NCWnd::HideWindow();
}

// 0x100db310 (vslot 55) - the background under the title bar and the two names in white
INT NCTradeWnd::OnPaint(NCGDevice* Device)
{
    Device->BeginPaint(ScreenX, ScreenY);
    Device->SetClip(ScreenX, ScreenY, Width, Height);
    if (BackTex)
        Device->DrawTexture(0, 21, 240, 344, 0, 0, 240, 344, BackTex);
    if (MyName.Num())
        Device->DrawText(16, 185, 0xFFFFFFFF, *MyName, 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
    if (PartnerName.Num())
        Device->DrawText(16, 256, 0xFFFFFFFF, *PartnerName, 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
    Device->EndPaint();
    Device->ReleaseClip();
    return 1;
}
NW_HOOK_METHOD(0x100db310, NCTradeWnd, OnPaint)

// 0x100da8c0 (vslot 67)
INT NCTradeWnd::OnOK()
{
    GConsole->RequestTradeDone(1);
    return 1;
}
NW_HOOK_METHOD(0x100da8c0, NCTradeWnd, OnOK)

// 0x100da8e0 (vslot 68)
INT NCTradeWnd::OnCancel()
{
    GConsole->RequestTradeDone(0);
    return 1;
}
NW_HOOK_METHOD(0x100da8e0, NCTradeWnd, OnCancel)

// 0x100db540 (vslot 69) - the down arrow offers the selected inventory item: how many first for a stack (the
// inventory grid asks and gets the answer), else one. The name buffer is the stock frame's 64 characters.
INT NCTradeWnd::OnAdd()
{
    INT Sel = InventoryItems->GetSelected();
    if (Sel < 0 || Sel >= InventoryItems->Items.Num())
        return 1;
    NItemEntry* Item = InventoryItems->Items(Sel);
    if (!Item)
        return 1;
    if (Item->IsStackable() && Item->TradeCount > 1)
    {
        AskTradeAmount(InventoryItems, DIALOG_TRADE_ADD, Item);
        return 1;
    }
    NItemRequest Request;
    Request.ObjectID = Item->ObjectID;
    Request.Count = 1;
    RequestAddTradeItem(Request);
    return 1;
}
NW_HOOK_METHOD(0x100db540, NCTradeWnd, OnAdd)

// 0x100da7e0 - a trade starts or ends: the grids emptied, the partner's name cleared (the player's is set again by the
// start); C2 clears both names
void NCTradeWnd::Reset()
{
    InventoryItems->Clear();
    MyItems->Clear();
    PartnerItems->Clear();
    PartnerName.Empty();
    MyName.Empty();
}
NW_HOOK_METHOD(0x100da7e0, NCTradeWnd, Reset)

// 0x100da860
void NCTradeWnd::SetMyName(const TCHAR* Name)
{
    if (Name && *Name)
        MyName = Name;
    else
        MyName.Empty();
}
NW_HOOK_METHOD(0x100da860, NCTradeWnd, SetMyName)

// 0x100da890
void NCTradeWnd::SetPartnerName(const TCHAR* Name)
{
    if (Name && *Name)
        PartnerName = Name;
    else
        PartnerName.Empty();
}
NW_HOOK_METHOD(0x100da890, NCTradeWnd, SetPartnerName)

// 0x100dba20
UBOOL NCTradeWnd::AddInventoryItem(NItemInfo* Info)
{
    return InventoryItems->AddItem(Info);
}
NW_HOOK_METHOD(0x100dba20, NCTradeWnd, AddInventoryItem)

// 0x100dba30
UBOOL NCTradeWnd::AddMyItem(NItemInfo* Info)
{
    return MyItems->AddItem(Info);
}
NW_HOOK_METHOD(0x100dba30, NCTradeWnd, AddMyItem)

// 0x100dba40
UBOOL NCTradeWnd::AddPartnerItem(NItemInfo* Info)
{
    return PartnerItems->AddItem(Info);
}
NW_HOOK_METHOD(0x100dba40, NCTradeWnd, AddPartnerItem)

// 0x100dc0c0 (C2: an anonymous function of its own, C1 0x100ba3a0 was inline in the console) - a change of the player's
// inventory while trading: 1 add, 3 change, 2 remove (the object id of the item)
UBOOL NCTradeWnd::UpdateInventoryItem(INT Op, NItemInfo* Info)
{
    if (Op == 1)
    {
        InventoryItems->AddItem(Info);
        return 1;
    }
    if (Op == 3)
    {
        InventoryItems->UpdateItem(Info);
        return 1;
    }
    if (Op == 2)
        InventoryItems->RemoveItem(Info->ObjectID);
    return 1;
}
NW_HOOK_METHOD(0x100dc0c0, NCTradeWnd, UpdateInventoryItem)

// one grid's entries get their name and description again from the item tables, and their
// tool tips (the stock code repeats this for each grid)
static FORCEINLINE void ReloadItemTexts(NCTradeItemWnd* Grid)
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

// 0x100db720 - after the item texts were reloaded (NConsoleWnd::ReloadGameTexts)
void NCTradeWnd::ReloadTexts()
{
    ReloadItemTexts(InventoryItems);
    ReloadItemTexts(MyItems);
    ReloadItemTexts(PartnerItems);
}
NW_HOOK_METHOD(0x100db720, NCTradeWnd, ReloadTexts)

NW_CHECK_VTABLE(NCTradeWnd, 0x10127678, 70)
