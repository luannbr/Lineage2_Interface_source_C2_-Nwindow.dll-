/*=============================================================================
    NCPetInventoryWnd.cpp - the pet window's Items page and its item grid.
    Stock: 0x1008eea0 .. 0x1008ef50, 0x1008ef60 .. 0x10090b4c, 0x10091830 .. 0x10091d06,
    0x10091d10 .. 0x10091f6f, 0x10091f70 .. 0x10091fa0, 0x10093a10 .. 0x10093e45,
    0x10093e50 .. 0x100940a6, 0x100941c0 .. 0x1009436d (ClearItems: a copy of the body
    NCTrainListWnd keeps the address of, 0x100bab50).
=============================================================================*/
#include "nwindow/NCPetInventoryWnd.h"
#include "nwindow/NCConsole.h"
#include "nwindow/NCDialogBox.h"
#include "nwindow/NCGDevice.h"
#include "nwindow/NCShortCutWnd.h"
#include "nwindow/NClasses.gen.h"
#include "nwindow/NConsoleWnd.h"
#include "nwindow/NDialogInfo.h"

// interface sounds (FL2NetNotify::OnInterfacePlaySound)
enum { SOUND_ItemPick = 3 };

enum { DIALOG_PET_ITEM_TO_OWNER = 0x37 };  // NDialogInfo::ID (name ours; C1: 0x33)
enum { MSG_PetItemToOwnerAdena = 536, MSG_PetItemToOwner = 537 };  // systemmsg: how much / how many to give back
enum { ITEM_Adena = 57 };

// a system line in the desktop's chat window (NCConsole::AddSystemText): Text when not empty
static FORCEINLINE void DesktopChatMessage(const TCHAR* Text, INT P2, INT P3)
{
    GDesktop->AddSystemText(Text, P2, P3);
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
    NCPetInvenItemWnd.
-----------------------------------------------------------------------------*/

// 0x100b61b0
NCPetInvenItemWnd::NCPetInvenItemWnd()
{
    bDropPending = 0;
    bToOwnerPending = 0;
    bDblClicked = 0;
}
NW_HOOK_CTOR(0x100b61b0, NCPetInvenItemWnd)

// 0x100b6260 - the pet's grid is 6 x 2 icons of 32 pixels, 160 at most; EquippedTex and DropLocation are left as allocated
NCPetInvenItemWnd::NCPetInvenItemWnd(INT InColumns, INT InVisibleRows, INT InMaxItems, INT InCellSize)
    : NCItemWnd(InColumns, InVisibleRows, InMaxItems, InCellSize)
{
    bDropPending = 0;
    bToOwnerPending = 0;
    bDblClicked = 0;
}
NW_HOOK_CTOR_ARGS(0x100b6260, NCPetInvenItemWnd, (INT A, INT B, INT C, INT D), (A, B, C, D))

// 0x100b6210
NW_HOOK_DTOR(0x100b6210, NCPetInvenItemWnd)

// stock body shared with NCInvenItemWnd::WindowProc (0x100c4610), which keeps the address
INT NCPetInvenItemWnd::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (NCItemWnd::WindowProc(Msg, wParam, lParam))
        return 1;
    if (Msg == WM_LBUTTONDBLCLK)
        return OnLButtonDblClick(wParam, lParam) != 0;
    return 0;
}

// 0x100b2b60 (vslot 36)
INT NCPetInvenItemWnd::OnCreate()
{
    NCItemWnd::OnCreate();
    EquippedTex = FindTexture(TEXT("l2ui.PetWnd.petitem_click"));
    return 1;
}
NW_HOOK_METHOD(0x100b2b60, NCPetInvenItemWnd, OnCreate)


// 0x100b3180 (vslot 55) - NCItemWnd's icons with a frame behind the equipped items (and the
// selected one); no guard in the stock code
INT NCPetInvenItemWnd::OnPaint(NCGDevice* Device)
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
        if (Item->bEquipped)
            Device->DrawTexture(X - 1, Y - 1, 34, 34, 0, 0, 34, 34, EquippedTex);
        if (Item->IsSelected())
            Device->DrawTexture(X - 1, Y - 1, 34, 34, 0, 0, 34, 34, SelectTex);
        if (Item->GetIcon())
            Device->DrawTexture(X, Y, CellSize, CellSize, 0, 0, CellSize, CellSize, Item->GetIcon());
    }
    Device->EndPaint();
    Device->ReleaseClip();
    return 1;
}
NW_HOOK_METHOD(0x100b3180, NCPetInvenItemWnd, OnPaint)

// 0x100b3330 (vslot 61) - NCItemWnd's, but the desktop's box is left alone and keeps no focus
INT NCPetInvenItemWnd::OnLButtonDown(WPARAM wParam, LPARAM lParam)
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
            PlayInterfaceSound(SOUND_ItemPick);
            return 1;
        }
        bNoDrag = 0;
    }
    return 1;
}
NW_HOOK_METHOD(0x100b3330, NCPetInvenItemWnd, OnLButtonDown)

// 0x100b33c0 (vslot 73) - the pet uses the item
INT NCPetInvenItemWnd::OnLButtonDblClick(WPARAM wParam, LPARAM lParam)
{
    bNoDrag = 1;
    bDragging = 0;
    Selected = HitTest(LOWORD(lParam), HIWORD(lParam));
    if (Selected >= 0 && Selected < Items.Num())
        GConsole->RequestPetUseItem(Items(Selected)->ObjectID);
    bDblClicked = 1;
    UnselectAll();
    Selected = -1;
    bDragging = 0;
    return 1;
}
NW_HOOK_METHOD(0x100b33c0, NCPetInvenItemWnd, OnLButtonDblClick)

// 0x100b3450 (vslot 63) - the pet uses the item (the selection stays)
INT NCPetInvenItemWnd::OnRButtonDown(WPARAM wParam, LPARAM lParam)
{
    Selected = HitTest(LOWORD(lParam), HIWORD(lParam));
    if (Selected >= 0 && Selected < Items.Num())
        GConsole->RequestPetUseItem(Items(Selected)->ObjectID);
    bDragging = 0;
    return 1;
}
NW_HOOK_METHOD(0x100b3450, NCPetInvenItemWnd, OnRButtonDown)


// 0x100b5c10 (vslot 62) - where the carried item is put down: inside the grid it changes place, on the player's inventory
// pages or paperdoll it is given back (a stack asks how many first), on a shortcut bar it is registered (as a pet item:
// the last number is 2). C2: the question carries the item's object id (SetParams) and the count (+0x74); no
// enchant mode here.
INT NCPetInvenItemWnd::OnLButtonUp(WPARAM wParam, LPARAM lParam)
{
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
    if (bDragging && !bDblClicked)
    {
        NItemEntry* Item = Items(Selected);
        if (!Item)
        {
            bDragging = 0;
            return 1;
        }
        INT X = LOWORD(lParam);
        INT Y = HIWORD(lParam);
        if (!IsInWindow(X, Y))
        {
            NWndEntry* Found = FindWindowAt(X, Y);
            if (Found && Found->Wnd)
            {
                NCWnd* Wnd = Found->Wnd;
                if (Wnd->IsA(NClass::NCInvenItemWnd()) || Wnd->IsA(NClass::NCEquipItemWnd()))
                {
                    // back to the player
                    bToOwnerPending = 1;
                    if (Item->IsStackable() && Item->Count > 1)
                    {
                        TCHAR Name[64];
                        appSprintf(Name, TEXT("%s"), Item->GetName());
                        NDialogInfo Info;
                        Info.ID = DIALOG_PET_ITEM_TO_OWNER;
                        Info.Type = 3;
                        Info.bInput = 1;
                        Info.bMainBox = 1;
                        Info.Owner = Entry;
                        if (Item->ID == ITEM_Adena)
                        {
                            Info.SetSystemMessage(MSG_PetItemToOwnerAdena);
                            Info.SetButtons(Name, NULL);
                            Info.bLargeAmount = 1;
                            Info.Unknown74 = Item->Count;
                            Info.SetParams(Item->ObjectID, -1);
                        }
                        else
                        {
                            Info.SetSystemMessage(MSG_PetItemToOwner);
                            Info.SetButtons(Name, NULL);
                            Info.bLargeAmount = 0;
                            Info.Unknown74 = Item->Count;
                            Info.SetParams(Item->ObjectID, -1);
                        }
                        GConsole->ShowDialog(&Info);
                    }
                    else
                    {
                        L2ParamStack Params(10);
                        Params.PushBack((void*)(INT_PTR)Items(Selected)->ObjectID);
                        Params.PushBack((void*)1);
                        Params.PushBack((void*)0);
                        GConsole->RequestGetItemFromPet(Params);
                    }
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
                        Params.PushBack((void*)2);  // the pet's
                        GConsole->RequestShortCutReg(Params);
                    }
                    UnselectAll();
                    Selected = -1;
                }
            }
        }
        else
        {
            // moved inside the grid
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
NW_HOOK_METHOD(0x100b5c10, NCPetInvenItemWnd, OnLButtonUp)

// 0x100b62a0 (vslot 70) - the entries are deleted
void NCPetInvenItemWnd::Clear()
{
    for (INT i = 0; i < Items.Num(); i++)
        if (Items(i))
            delete Items(i);
    Items.Empty();
}
NW_HOOK_METHOD(0x100b62a0, NCPetInvenItemWnd, Clear)

// 0x100b6080 (C1 0x10093e50; no C2 match, found by the callers of NCPetWnd's console entry) - the icon comes from
// its name ("NWindow.BlackTexture" when missing) and is written back into Info; the scroll range is not updated
// here. No guard (the stock frame only unwinds the new entry); the stock code tests the address of Info's name for
// NULL first: a compiler artifact, Info is never NULL there
__declspec(noinline) UBOOL NCPetInvenItemWnd::AddItem(NItemInfo* Info)
{
    if (Items.Num() >= MaxItems)
        return 0;
    Info->Icon = FindTexture(Info->IconName);
    if (!Info->Icon)
        Info->Icon = FindTexture(TEXT("NWindow.BlackTexture"));
    // the allocation is inline in the stock code: GMalloc->Malloc(0x1A28, "new")
    NItemEntry* Item = (NItemEntry*)appMalloc(sizeof(NItemEntry), TEXT("new"));
    if (Item)
        Item = new (Item) NItemEntry(Info);
    Item->FillToolTip(2, 0);
    Items.AddItem(Item);
    return 1;
}
NW_HOOK_METHOD(0x100b6080, NCPetInvenItemWnd, AddItem)

// 0x100b34b0 - the entry with the same object id takes Info's values (through a new entry that is never freed, as in
// the stock code)
UBOOL NCPetInvenItemWnd::UpdateItem(NItemInfo* Info)
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
            New->FillToolTip(2, 0);
            Old->Copy(New);
            return 1;
        }
    }
    return 0;
}
NW_HOOK_METHOD(0x100b34b0, NCPetInvenItemWnd, UpdateItem)

// C1 0x10093f40: C2 has no function for it; not hooked - one body with NCTradeItemWnd's
UBOOL NCPetInvenItemWnd::RemoveItem(INT ObjectID)
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

// 0x100b35b0 (NCInvenItemWnd has an unhooked copy)
UBOOL NCPetInvenItemWnd::HasItem(INT ObjectID)
{
    for (INT i = 0; i < Items.Num(); i++)
        if (ObjectID == Items(i)->ObjectID)
            return 1;
    return 0;
}
NW_HOOK_METHOD(0x100b35b0, NCPetInvenItemWnd, HasItem)

// 0x100b3600
UBOOL NCPetInvenItemWnd::FindItem(INT ObjectID, INT* Index)
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
NW_HOOK_METHOD(0x100b3600, NCPetInvenItemWnd, FindItem)

NW_CHECK_VTABLE(NCPetInvenItemWnd, 0x10122298, 74)

/*-----------------------------------------------------------------------------
    NCPetInventoryWnd.
-----------------------------------------------------------------------------*/

// 0x100b3650
NCPetInventoryWnd::NCPetInventoryWnd()
{
}
NW_HOOK_CTOR(0x100b3650, NCPetInventoryWnd)

// 0x100b36a0
NW_HOOK_DTOR(0x100b36a0, NCPetInventoryWnd)

// stock body shared with NControl's (0x10004e90)
INT NCPetInventoryWnd::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (Msg != WM_PAINT && !IsEnabled())
        return 0;
    return NCWnd::WindowProc(Msg, wParam, lParam) != 0;
}

// 0x100b6300 (vslot 36) - the grid, 6 x 2 icons of 32 pixels, 160 at most
INT NCPetInventoryWnd::OnCreate()
{
    NCWnd::OnCreate();
    BackTex = FindTexture(TEXT("L2UI.PetWnd.pet03"));
    Items = new NCPetInvenItemWnd(6, 2, 160, 32);
    Items->SetWindowRect(16, 29, 223, 69);
    CreateChild(Items, NWS_VISIBLE | 0x20);   // 0x20: meaning unknown
    return 1;
}
NW_HOOK_METHOD(0x100b6300, NCPetInventoryWnd, OnCreate)

// 0x100b36f0 (vslot 37) - the grid goes, then the base
INT NCPetInventoryWnd::OnDestroy()
{
    guard(NCPetInventoryWnd::OnDestroy);
    DestroyChild(this, Items);
    NCWnd::OnDestroy();
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x100b36f0, NCPetInventoryWnd, OnDestroy)

// stock body shared with NCPetActionWnd's (0x10091680)
INT NCPetInventoryWnd::OnPaint(NCGDevice* Device)
{
    Device->BeginPaint(ScreenX, ScreenY);
    Device->SetClip(ScreenX, ScreenY, Width, Height);
    if (BackTex)
        Device->DrawTexture(0, 0, Width, Height, 0, 0, Width, Height, BackTex);
    Device->EndPaint();
    Device->ReleaseClip();
    return 1;
}

// 0x100b6170 (C1 0x10094010; no C2 match) - the grid takes it
UBOOL NCPetInventoryWnd::AddItem(NItemInfo* Info)
{
    Items->AddItem(Info);
    return 1;
}
NW_HOOK_METHOD(0x100b6170, NCPetInventoryWnd, AddItem)

// 0x100b63c0 (C1 0x10094030; no C2 match) - the server's change of one item: 1 added, 2 changed (when here), 3
// removed (the stock code calls 0x100dbb90 for that, the body NCTradeItemWnd keeps: ours is the grid's own copy)
UBOOL NCPetInventoryWnd::UpdateItem(INT Op, NItemInfo* Info)
{
    if (Op == 1)
    {
        Items->AddItem(Info);
        return 1;
    }
    if (Op == 2)
    {
        if (Items->HasItem(Info->ObjectID))
            Items->UpdateItem(Info);
        return 1;
    }
    if (Op == 3)
        Items->RemoveItem(Info->ObjectID);
    return 1;
}
NW_HOOK_METHOD(0x100b63c0, NCPetInventoryWnd, UpdateItem)

// 0x100b37a0 - the pet's item with that object id (the shortcut bar and the console)
NItemEntry* NCPetInventoryWnd::GetItem(INT ObjectID)
{
    INT Index;
    if (Items->FindItem(ObjectID, &Index))
        return Items->Items(Index);
    return NULL;
}
NW_HOOK_METHOD(0x100b37a0, NCPetInventoryWnd, GetItem)

// 0x100b2b90 - the grid's Clear (vslot 70) as a tail jump; the stock body is shared with NCTrainListWnd's page
// (NCTrainWnd::Reset calls the same address for its list box at +0x9C)
void NCPetInventoryWnd::ClearItems()
{
    Items->Clear();
}
NW_HOOK_METHOD(0x100b2b90, NCPetInventoryWnd, ClearItems)

NW_CHECK_VTABLE(NCPetInventoryWnd, 0x10121af8, 67)
