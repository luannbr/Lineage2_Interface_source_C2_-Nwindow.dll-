/*=============================================================================
    NCInvenItemWnd.cpp - a page of the inventory's items. Stock: 0x10062360 .. 0x100641e7,
    0x10065060 .. 0x1006590f, 0x10067690 .. 0x1006819f and 0x10068440 .. 0x100684c9 (the
    paperdoll and the inventory sit in between), plus the destructor (0x1005fe00) and
    WindowProc (0x10057b10).
=============================================================================*/
#include "nwindow/NCInvenItemWnd.h"
#include "nwindow/NCConsole.h"
#include "nwindow/NCDialogBox.h"
#include "nwindow/NCEquipItemWnd.h"
#include "nwindow/NCGDevice.h"
#include "nwindow/NCInventoryWnd.h"
#include "nwindow/NCShortCutWnd.h"
#include "nwindow/NMacroEntry.h"
#include "nwindow/NClasses.gen.h"
#include "nwindow/NDialogInfo.h"
#include "nwindow/NDropDialogInfo.h"

enum { ITEM_ADENA = 57 };
enum { SOUND_PickUp = 3 };

// NDialogInfo::ID of the questions (names ours)
enum
{
    DIALOG_DROP_ITEM = 4,
    DIALOG_DESTROY_ITEM = 5,
    DIALOG_CRYSTALLIZE_ITEM = 6,
    DIALOG_GIVE_TO_PET = 54,    // 0x36 (C1: 50)
};

// systemmsg
enum
{
    MSG_DropHowMany = 71,       // "How many $s1(s) do you want to discard?"
    MSG_MoveHowMany = 72,       // "How many $s1(s) do you want to move?"
    MSG_DestroyHowMany = 73,    // "How many $s1(s) do you want to destroy?"
    MSG_Destroy = 74,           // "Destroy $s1. Do you want to continue?"
    MSG_CannotDropHere = 179,   // "You cannot discard items here."
    MSG_Crystallize = 336,      // "$s1 is crystallized. Do you want to continue?"
    MSG_AreYouSure = 365,       // "Are you sure?"
    MSG_Drop = 400,             // "Discard $s1. Do you wish to continue?"
    MSG_Place = 651,            // "Place $s1 in the current location and direction..."
};

static FORCEINLINE NCDialogBox* MainDialogBox()
{
    return GDesktop->MainDialogBox;
}

/// 0x1007fb00
NCInvenItemWnd::NCInvenItemWnd()
{
    bEnchantMode = 0;
    bDblClicked = 0;
}
NW_HOOK_CTOR(0x1007fb00, NCInvenItemWnd)

// 0x1007fb20 - the inventory's pages are 6 x 4 icons of 32 pixels, 160 at most; bEnchantMode is left as allocated
NCInvenItemWnd::NCInvenItemWnd(INT InColumns, INT InVisibleRows, INT InMaxItems, INT InCellSize)
    : NCItemWnd(InColumns, InVisibleRows, InMaxItems, InCellSize)
{
    bDblClicked = 0;
}
NW_HOOK_CTOR_ARGS(0x1007fb20, NCInvenItemWnd, (INT A, INT B, INT C, INT D), (A, B, C, D))

// 0x100782e0
NW_HOOK_DTOR(0x100782e0, NCInvenItemWnd)

// 0x100c4610 (vslot 25) - the stock body is shared with NCPetInvenItemWnd's (the double click is the virtual at
// slot 73 in both); C2 has no dialog result message any more
INT NCInvenItemWnd::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (NCItemWnd::WindowProc(Msg, wParam, lParam))
        return 1;
    if (Msg == WM_LBUTTONDBLCLK)
        return OnLButtonDblClick(wParam, lParam) != 0;
    return 0;
}
NW_HOOK_METHOD(0x100c4610, NCInvenItemWnd, WindowProc)

// 0x100d8aa0 - the stock body is shared with NCStoreItemWnd::SetMode and the other grids' setters of the dword at
// +0x110, which keeps the address
void NCInvenItemWnd::SetEnchantMode(UBOOL bOn)
{
    bEnchantMode = bOn;
}

// The shortcut bars forget a pending selection (C2: the bars' list sits behind the desktop, +0xF8; 0x10056a20
// counts them, 0x10060ce0 gets one, 0x1008b4b0 clears its slots and the flag at +0x9D4).
static void ClearShortCutSelections()
{
    if (GConsole)
    {
        for (INT i = 0; i < GConsole->GetMacroCount(); i++)
        {
            if (GConsole->GetMacro(i) && GConsole->GetMacro(i)->bRunning)
                GConsole->GetMacro(i)->Reset();
        }
    }
}

// Uses the item, the way a double click and the right button do in C2: an item with the flag at +0x1998 asks first
// when the player may crystallize (dialog 0x21, system message 0x31E with the name), one with a message id
// (GetUnknown19AC) shows that message (dialog 0x45), any other is used at once.
static FORCEINLINE void UseOrAsk(NCInvenItemWnd* Wnd, NItemEntry* Item)
{
    if (Item && Item->Unknown1998 && GConsole->GetMyUser()->bCanCrystallize)
    {
        TCHAR Name[64];
        appSprintf(Name, TEXT("%s"), Item->GetName());
        NDialogInfo Dialog;
        Dialog.Owner = Wnd->Entry;
        Dialog.ID = 0x21;
        Dialog.bMainBox = 1;
        Dialog.bInput = 0;
        Dialog.SetSystemMessage(0x31E);
        Dialog.SetButtons(Name, NULL);
        Dialog.bLargeAmount = 0;
        Dialog.SetParams(Item->ObjectID, -1);
        GConsole->ShowDialog(&Dialog);
    }
    else if (Item->GetUnknown19AC() > 0)
    {
        NDialogInfo Dialog;
        Dialog.Owner = Wnd->Entry;
        Dialog.ID = 0x45;
        Dialog.bMainBox = 1;
        Dialog.bInput = 0;
        Dialog.SetSystemMessage(Item->GetUnknown19AC());
        Dialog.SetParams(Item->ObjectID, -1);
        Dialog.bLargeAmount = 0;
        GConsole->ShowDialog(&Dialog);
    }
    else
        GConsole->RequestUseItem(Item->ObjectID);
}

// 0x1007be00 (vslot 63) - uses the item under the mouse (UseOrAsk) and clears the shortcut bars' selections
INT NCInvenItemWnd::OnRButtonDown(WPARAM wParam, LPARAM lParam)
{
    Selected = HitTest(LOWORD(lParam), HIWORD(lParam));
    if (Selected >= 0 && Selected < Items.Num())
    {
        UseOrAsk(this, Items(Selected));
        ClearShortCutSelections();
    }
    bDragging = 0;
    return 1;
}
NW_HOOK_METHOD(0x1007be00, NCInvenItemWnd, OnRButtonDown)

// 0x1007c070 (vslot 61) - NCItemWnd's, but the desktop's box is left alone and keeps no focus
INT NCInvenItemWnd::OnLButtonDown(WPARAM wParam, LPARAM lParam)
{
    INT Index = HitTest(LOWORD(lParam), HIWORD(lParam));
    HoverIndex = Index;
    Selected = Index;
    if (Index >= 0 && Index < Items.Num())
    {
        UnselectAll();
        Items(Selected)->Select();
        if (!bNoDrag)
        {
            bDragging = 1;
            PlayInterfaceSound(SOUND_PickUp);
            return 1;
        }
        bNoDrag = 0;
    }
    return 1;
}
NW_HOOK_METHOD(0x1007c070, NCInvenItemWnd, OnLButtonDown)

// 0x1007c100 (vslot 73) - uses the item (UseOrAsk); the release that follows moves nothing
INT NCInvenItemWnd::OnLButtonDblClick(WPARAM wParam, LPARAM lParam)
{
    bNoDrag = 1;
    bDragging = 0;
    Selected = HitTest(LOWORD(lParam), HIWORD(lParam));
    if (Selected >= 0 && Selected < Items.Num())
        UseOrAsk(this, Items(Selected));
    bDblClicked = 1;
    UnselectAll();
    bDragging = 0;
    Selected = -1;
    return 1;
}
NW_HOOK_METHOD(0x1007c100, NCInvenItemWnd, OnLButtonDblClick)

// 0x1007c340 - the item with Info's object id takes Info's fields and a new tool tip; the entry built for that is
// never freed, as in the stock code
UBOOL NCInvenItemWnd::UpdateItem(NItemInfo* Info)
{
    for (INT i = 0; i < Items.Num(); i++)
    {
        NItemEntry* Found = Items(i);
        if (Info->ObjectID == Found->ObjectID)
        {
            Info->Icon = FindTexture(Info->IconName);
            if (!Info->Icon)
                Info->Icon = FindTexture(TEXT("NWindow.BlackTexture"));
            NItemEntry* Updated = new NItemEntry(Info);
            Updated->FillToolTip(2, 0);
            Found->Copy(Updated);
            return 1;
        }
    }
    return 0;
}
NW_HOOK_METHOD(0x1007c340, NCInvenItemWnd, UpdateItem)

// 0x1007c440
INT NCInvenItemWnd::GetAdena()
{
    INT Adena = 0;
    for (INT i = 0; i < Items.Num(); i++)
    {
        NItemEntry* Item = Items(i);
        if (Item && Item->ID == ITEM_ADENA)
            Adena += Item->Count;
    }
    return Adena;
}
NW_HOOK_METHOD(0x1007c440, NCInvenItemWnd, GetAdena)

// 0x1007c470 (vslot 55) - NCQuestItemWnd's drawing with a clip one pixel taller; the stock
// guard carries NCItemWnd's name
INT NCInvenItemWnd::OnPaint(NCGDevice* Device)
{
    guard(NCItemWnd::OnPaint);
    Device->SetClip(ScreenX - 1, ScreenY - 1, Width + 2, Height + 3);
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
NW_HOOK_METHOD(0x1007c470, NCInvenItemWnd, OnPaint)

// 0x1007c630 (vslot 60) - NCItemWnd's cursor handling (the enchant scroll's cursor stays too), then the
// inventory's trash / crystallize icons light up under the mouse
INT NCInvenItemWnd::OnMouseMove(WPARAM wParam, LPARAM lParam)
{
    INT X = LOWORD(lParam);
    INT Y = HIWORD(lParam);
    HoverIndex = HitTest(X, Y);
    if (IsFocused())
    {
        if (bDragging)
        {
            if (Selected >= 0 && Selected < Items.Num())
            {
                SetCursor(4, Items(Selected)->GetIcon());
                LockCursor();
            }
        }
        else if (!GConsole->IsCtrlDown() && !bEnchantMode)
        {
            UnlockCursor();
            SetCursor(0, NULL);
        }
        NCInventoryWnd* Inventory = (NCInventoryWnd*)Entry->Parent->Wnd;
        Inventory->bOverTrash = PtInRect(X, Y, Inventory->ScreenX + Inventory->TrashX,
                                         Inventory->ScreenY + Inventory->TrashY, Inventory->TrashW,
                                         Inventory->TrashH) != 0;
        Inventory->bOverCrystallize = PtInRect(X, Y, Inventory->ScreenX + Inventory->CrystallizeX,
                                               Inventory->ScreenY + Inventory->CrystallizeY,
                                               Inventory->CrystallizeW, Inventory->CrystallizeH) != 0;
    }
    return 1;
}
NW_HOOK_METHOD(0x1007c630, NCInvenItemWnd, OnMouseMove)

// 0x1007e790 (vslot 62) - where the carried item is put down (see the header): inside the page it changes place, on the
// paperdoll it is equipped, on a shortcut bar it is registered, on the pet window it is given (a stack asks how many
// first), on the trash it is destroyed after a question (the full name of the item in it), on the crystallize icon it
// is crystallized after one, on the game it is dropped (or placed, for the etc items of type 8) after one unless a pawn
// is under the mouse; on a recipe book's item window nothing happens. C2: the questions carry the item's object id
// (SetParams) and the count (+0x74) instead of the window remembering them, and the drop's questions also the ground
// point (+0x78: NDialogInfo's trailing vector, see NDropDialogInfo in NCEquipItemWnd.cpp).
INT NCInvenItemWnd::OnLButtonUp(WPARAM wParam, LPARAM lParam)
{
    if (!GConsole->IsCtrlDown() && !bEnchantMode)
    {
        UnlockCursor();
        SetCursor(0, NULL);
    }
    if (Selected < 0 || Selected >= Items.Num())
    {
        bDragging = 0;
        return 1;
    }
    if (bDragging && !bDblClicked)
    {
        NItemEntry* Item = Items(Selected);
        if (!Item)
        {
            bDragging = 0;
            return 1;
        }
        if (bEnchantMode)
        {
            GConsole->RequestEnchantItem(Item->ObjectID);
            bDblClicked = 0;
            bDragging = 0;
            return 1;
        }
        INT X = LOWORD(lParam);
        INT Y = HIWORD(lParam);
        if (!IsInWindow(X, Y))
        {
            NWndEntry* Found = FindWindowAt(X, Y);
            NCWnd* Wnd = Found ? Found->Wnd : NULL;
            if (Wnd)
            {
                if (Wnd->IsA(NClass::NCEquipItemWnd()))
                {
                    // equipped
                    ((NCEquipItemWnd*)Wnd)->HitTest(X, Y);
                    GConsole->RequestUseItem(Items(Selected)->ObjectID);
                    UnselectAll();
                    Selected = -1;
                }
                else if (Wnd->IsA(NClass::NCShortCutWnd()))
                {
                    // 0x100CCEF0: the bar's slot under the point (by the bar's layout), -1 none
                    INT Slot = ((NCShortCutWnd*)Wnd)->SlotAtPoint(X, Y);
                    if (Slot >= 0)
                    {
                        L2ParamStack Params(10);
                        Params.PushBack((void*)1);  // an item
                        Params.PushBack((void*)(INT_PTR)Slot);
                        Params.PushBack((void*)(INT_PTR)Items(Selected)->ObjectID);
                        Params.PushBack((void*)1);
                        GConsole->RequestShortCutReg(Params);
                    }
                    UnselectAll();
                    Selected = -1;
                }
                else if (Wnd->IsA(NClass::NCPetWnd()))
                {
                    if (!Item->IsStackable() || Item->Count < 2)
                    {
                        L2ParamStack Params(10);
                        Params.PushBack((void*)(INT_PTR)Items(Selected)->ObjectID);
                        Params.PushBack((void*)1);
                        GConsole->RequestGiveItemToPet(Params);
                    }
                    else
                    {
                        TCHAR Name[256];
                        appSprintf(Name, TEXT("%s"), Item->GetName());
                        NDropDialogInfo Info;
                        Info.ID = DIALOG_GIVE_TO_PET;
                        Info.Type = 3;
                        Info.bInput = 1;
                        Info.bMainBox = 1;
                        Info.Owner = Entry;
                        Info.SetSystemMessage(MSG_MoveHowMany);
                        Info.SetButtons(Name, NULL);
                        Info.bLargeAmount = Item->ID == ITEM_ADENA;
                        Info.Unknown74 = Item->Count;
                        Info.SetParams(Item->ObjectID, -1);
                        GConsole->ShowDialog(&Info);
                    }
                }
                else if (Wnd->IsA(NClass::NCInventoryWnd()))
                {
                    NCInventoryWnd* Inventory = (NCInventoryWnd*)Wnd;
                    Inventory->bOverTrash = 0;
                    Inventory->bOverCrystallize = 0;
                    if (Inventory->IsInTrash(X, Y))
                    {
                        if (!Item->IsStackable() || Item->Count < 2)
                        {
                            TCHAR Name[256];
                            appSprintf(Name, TEXT("%s"), Item->MakeDisplayName());
                            NDropDialogInfo Info;
                            Info.ID = DIALOG_DESTROY_ITEM;
                            Info.bMainBox = 1;
                            Info.Owner = Entry;
                            Info.bInput = 0;
                            Info.SetSystemMessage(MSG_Destroy);
                            Info.SetButtons(Name, NULL);
                            Info.bLargeAmount = Item->ID == ITEM_ADENA;
                            Info.SetParams(Item->ObjectID, -1);
                            GConsole->ShowDialog(&Info);
                        }
                        else
                        {
                            TCHAR Name[256];
                            appSprintf(Name, TEXT("%s"), Item->MakeDisplayName());
                            NDropDialogInfo Info;
                            Info.ID = DIALOG_DESTROY_ITEM;
                            Info.Type = 3;
                            Info.bInput = 1;
                            Info.bMainBox = 1;
                            Info.Owner = Entry;
                            Info.SetSystemMessage(MSG_DestroyHowMany);
                            Info.SetButtons(Name, NULL);
                            Info.bLargeAmount = Item->ID == ITEM_ADENA;
                            Info.Unknown74 = Item->Count;
                            Info.SetParams(Item->ObjectID, -1);
                            GConsole->ShowDialog(&Info);
                        }
                    }
                    else if (Inventory->IsInCrystallize(X, Y))
                    {
                        NViewItemData* Data = NItemTable::Find(&Item->ID);
                        User* Me = GConsole->GetMyUser();
                        if (Me->bCanCrystallize && Data && Data->CrystalCount)
                        {
                            TCHAR Name[256];
                            appSprintf(Name, TEXT("%s"), Item->MakeDisplayName());
                            NDropDialogInfo Info;
                            Info.ID = DIALOG_CRYSTALLIZE_ITEM;
                            Info.Owner = Entry;
                            Info.bMainBox = 1;
                            Info.bInput = 0;
                            Info.SetSystemMessage(MSG_Crystallize);
                            Info.SetButtons(Name, NULL);
                            Info.bLargeAmount = 0;
                            Info.SetParams(Item->ObjectID, -1);
                            GConsole->ShowDialog(&Info);
                        }
                    }
                }
                else if (Wnd->IsA(NClass::NCRecipeBookItemWnd()))
                {
                    // nothing (C2: a window of the recipe book is a known target that takes no items)
                }
                else if (Wnd->IsA(NClass::NCConsole()))
                {
                    // on the ground, unless a pawn is under the mouse
                    if (!GConsole->GetTargetedActor(1))
                    {
                        TCHAR DropName[64];
                        appSprintf(DropName, TEXT("%s"), Item->GetName());
                        DropLocation = static_cast<UGameEngine*>(GConsole->Engine)->MouseHitLocation;
                        UBOOL bPlace = 0;
                        NViewItemData* Data = NItemTable::Find(&Item->ID);
                        if (Data && Data->Class == ITEMCLASS_Etc && Data->EtcType == 8)
                            bPlace = 1;
                        if (!Item->IsStackable() || Item->Count < 2)
                        {
                            NDropDialogInfo Info;
                            Info.ID = DIALOG_DROP_ITEM;
                            Info.Owner = Entry;
                            Info.bMainBox = 1;
                            Info.bInput = 0;
                            Info.SetSystemMessage(bPlace ? MSG_Place : MSG_Drop);
                            Info.SetButtons(DropName, NULL);
                            Info.bLargeAmount = 0;
                            Info.SetParams(Item->ObjectID, -1);
                            Info.Location = static_cast<UGameEngine*>(GConsole->Engine)->MouseHitLocation;
                            GConsole->ShowDialog(&Info);
                        }
                        else if (!bPlace)
                        {
                            NDropDialogInfo Info;
                            Info.ID = DIALOG_DROP_ITEM;
                            Info.Type = 3;
                            Info.bMainBox = 1;
                            Info.bInput = 1;
                            Info.Owner = Entry;
                            Info.SetSystemMessage(MSG_DropHowMany);
                            Info.SetButtons(DropName, NULL);
                            Info.bLargeAmount = Item->ID == ITEM_ADENA;
                            Info.Unknown74 = Item->Count;
                            Info.SetParams(Item->ObjectID, -1);
                            Info.Location = static_cast<UGameEngine*>(GConsole->Engine)->MouseHitLocation;
                            GConsole->ShowDialog(&Info);
                        }
                        else
                        {
                            NDropDialogInfo Info;
                            Info.ID = DIALOG_DROP_ITEM;
                            Info.Owner = Entry;
                            Info.bMainBox = 1;
                            Info.bInput = 0;
                            Info.SetSystemMessage(MSG_Place);
                            Info.SetButtons(DropName, NULL);
                            Info.bLargeAmount = 0;
                            Info.SetParams(Item->ObjectID, -1);
                            Info.Location = static_cast<UGameEngine*>(GConsole->Engine)->MouseHitLocation;
                            GConsole->ShowDialog(&Info);
                        }
                    }
                }
            }
        }
        else
        {
            // moved inside the page
            INT Target = HitTest(X, Y);
            if (Target != Selected && Target >= 0 && Selected >= 0 && Selected < Items.Num())
            {
                if (Target >= Items.Num())
                {
                    Items.Remove(Selected);
                    Items.AddItem(Item);
                    Selected = Items.Num() - 1;
                }
                else
                {
                    Items.Remove(Selected);
                    Items.Insert(Target);
                    Items(Target) = Item;
                }
                bDblClicked = 0;
                bDragging = 0;
                return 1;
            }
        }
    }
    bDblClicked = 0;
    bDragging = 0;
    return 1;
}
NW_HOOK_METHOD(0x1007e790, NCInvenItemWnd, OnLButtonUp)

// 0x1007f370 - 0 when the page is full
UBOOL NCInvenItemWnd::AddItem(NItemInfo* Info)
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
NW_HOOK_METHOD(0x1007f370, NCInvenItemWnd, AddItem)

// 0x1007fb60 (C1 0x10068120) - the entry is deleted and the scroll range updated
UBOOL NCInvenItemWnd::RemoveItem(INT ObjectID)
{
    for (INT i = 0; i < Items.Num(); i++)
    {
        if (Items(i)->ObjectID == ObjectID)
        {
            NItemEntry* Item = Items(i);
            Items.Remove(i);
            if (Item)
                delete Item;
            UpdateScrollRange();
            return 1;
        }
    }
    return 0;
}
NW_HOOK_METHOD(0x1007fb60, NCInvenItemWnd, RemoveItem)

// stock body shared with NCPetInvenItemWnd (0x10091c70), which keeps the address
UBOOL NCInvenItemWnd::HasItem(INT ObjectID)
{
    for (INT i = 0; i < Items.Num(); i++)
        if (ObjectID == Items(i)->ObjectID)
            return 1;
    return 0;
}

// stock body shared with NCPetInvenItemWnd (0x10091cc0), which keeps the address
UBOOL NCInvenItemWnd::FindItem(INT ObjectID, INT* Index)
{
    for (INT i = 0; i < Items.Num(); i++)
    {
        if (ObjectID == Items(i)->ObjectID)
        {
            *Index = i;
            return 1;
        }
    }
    return 0;
}

NW_CHECK_VTABLE(NCInvenItemWnd, 0x10117a60, 74)
