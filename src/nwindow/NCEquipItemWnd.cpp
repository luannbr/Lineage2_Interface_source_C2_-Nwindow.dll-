/*=============================================================================
    NCEquipItemWnd.cpp - the inventory's paperdoll. Stock: 0x10060730 .. 0x10062357 (up to
    the tool tip's jump table), 0x100642d0 .. 0x10065056, 0x100661a0 .. 0x10067684 (the item
    pages' and the inventory's functions sit in between), plus the constructor (0x10068380)
    and the destructor (0x1005fd70).
=============================================================================*/
#include "nwindow/NCEquipItemWnd.h"
#include "nwindow/NCConsole.h"
#include "nwindow/NCDialogBox.h"
#include "nwindow/NCGDevice.h"
#include "nwindow/NCInvenItemWnd.h"
#include "nwindow/NCInventoryWnd.h"
#include "nwindow/NCShortCutWnd.h"
#include "nwindow/NMacroEntry.h"
#include "nwindow/NClasses.gen.h"
#include "nwindow/NDialogInfo.h"
#include "nwindow/NDropDialogInfo.h"
#include "nwindow/NHennaData.h"
#if NW_FEATURES
// phase 6: ItemHoverTip (not in NWindow_C1: the hover tip of the item grids in the recent style)
#include "features/ItemHoverTip.h"
#endif

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
    MSG_DestroyHowMany = 73,    // "How many $s1(s) do you want to destroy?"
    MSG_Destroy = 74,           // "Destroy $s1. Do you want to continue?"
    MSG_CannotDropHere = 179,   // "You cannot discard items here."
    MSG_Crystallize = 336,      // "$s1 is crystallized. Do you want to continue?"
    MSG_Drop = 400,             // "Discard $s1. Do you wish to continue?"
    MSG_PetAdenaHowMuch = 575,  // "How much adena do you wish to transfer to your pet?"
    MSG_PetHowMuch = 576,       // "How much do you wish to transfer?"
};

// the slots' places: a grid of 35 pixels, six columns by three rows without the fourth
// column and the fifth of the middle row; 14 and 15 (the full armor over 6 and 10, the
// two-handed weapon over 5 and 7) are drawn apart
static const INT SlotX[14] = { 0, 35, 70, 140, 175, 0, 35, 70, 175, 0, 35, 70, 140, 175 };
static const INT SlotY[14] = { 0, 0, 0, 0, 0, 35, 35, 35, 35, 70, 70, 70, 70, 70 };

static FORCEINLINE NCDialogBox* MainDialogBox()
{
    return GDesktop->MainDialogBox;
}

// The shortcut bars forget a pending selection (C2: the bars' list sits behind the desktop, +0xF8; 0x10056a20 counts
// them, 0x10060ce0 gets one, 0x1008b4b0 clears its slots and the flag at +0x9D4).
static FORCEINLINE void ClearShortCutSelections()
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

// takes Item off when it is the one of Slot (OnRButtonDown, OnLButtonDblClick): by its body part bits, but an earring
// or a ring by the bit of the ear or the finger that wears it; the right button then clears the shortcut bars'
// selections (C2)
static FORCEINLINE UBOOL UnEquipIfInSlot(NCEquipItemWnd* Wnd, NItemEntry* Item, INT Slot, UBOOL bClearShortCuts)
{
    INT ItemSlot = Wnd->GetEquipSlot(Item->BodyPart, Item->ObjectID);
    if (ItemSlot == 15 || ItemSlot == 14)
    {
        if (Slot != (ItemSlot == 15 ? 5 : 6))
            return 0;
        L2ParamStack Params(10);
        Params.PushBack((void*)(INT_PTR)Item->BodyPart);
        GConsole->RequestUnEquipItem(Params);
        if (bClearShortCuts)
            ClearShortCutSelections();
        return 1;
    }
    if (Slot != ItemSlot)
        return 0;
    L2ParamStack Params(10);
    INT BodyPart = Item->BodyPart;
    if (BodyPart & 6)
    {
        User* Me = GConsole->GetMyUser();
        if (Me->Equipped[1] == Item->ObjectID)
            Params.PushBack((void*)2);
        if (Me->Equipped[2] == Item->ObjectID)
            Params.PushBack((void*)4);
    }
    else if (Item->BodyPart & 0x30)
    {
        User* Me = GConsole->GetMyUser();
        if (Me->Equipped[4] == Item->ObjectID)
            Params.PushBack((void*)0x10);
        if (Me->Equipped[5] == Item->ObjectID)
            Params.PushBack((void*)0x20);
    }
    else
        Params.PushBack((void*)(INT_PTR)BodyPart);
    GConsole->RequestUnEquipItem(Params);
    if (bClearShortCuts)
        ClearShortCutSelections();
    return 1;
}

// 0x10078240 - Items and Hennas (their destructors are TArray<T*>'s: the entries are not freed), then the base
NW_HOOK_DTOR(0x10078240, NCEquipItemWnd)

// 0x1007f730 - the inventory makes it with 32-pixel cells; DualCapTex, HennaBackTex and the rest are left as allocated
NCEquipItemWnd::NCEquipItemWnd(INT InCellSize)
{
    CellSize = InCellSize;
    SelectTex = NULL;
    ToolTipTex = NULL;
    bDragging = 0;
    bEnchantMode = 0;
    bNoDrag = 0;
    bHasUnknown1C98 = 0;
    Unknown104 = 5;
    Unknown108 = 3;
    NumSlots = 14;
    Selected = -1;
    HoverSlot = -1;
    HoverHenna = -1;
    SelectedIndex = -1;
}
NW_HOOK_CTOR_ARGS(0x1007f730, NCEquipItemWnd, (INT A), (A))


// 0x1007b3a0 (vslot 25)
INT NCEquipItemWnd::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (NCWnd::WindowProc(Msg, wParam, lParam))
        return 1;
    if (Msg == WM_LBUTTONDBLCLK)
        return OnLButtonDblClick(wParam, lParam) != 0;
    return 0;
}
NW_HOOK_METHOD(0x1007b3a0, NCEquipItemWnd, WindowProc)

// 0x1007b400 (vslot 36)
INT NCEquipItemWnd::OnCreate()
{
    NCWnd::OnCreate();
    ToolTipTex = FindTexture(TEXT("L2UI.tooltip.mini-tool01"));
    SelectTex = FindTexture(TEXT("l2ui.nwindow.item_click"));
    DualCapTex = FindTexture(TEXT("L2UI.InventoryWnd.Icon_dualcap"));
    HennaBackTex = FindTexture(TEXT("L2UI.InventoryWnd.Henna_Icon_Back"));
    return 1;
}
NW_HOOK_METHOD(0x1007b400, NCEquipItemWnd, OnCreate)

// 0x1007b480
void NCEquipItemWnd::SetEnchantMode(UBOOL bOn)
{
    bEnchantMode = bOn;
}
NW_HOOK_METHOD(0x1007b480, NCEquipItemWnd, SetEnchantMode)

// 0x1007b490 (vslot 67) - the console's answer (the slot an item with this body part sits in)
INT NCEquipItemWnd::GetEquipSlot(INT BodyPart, INT ObjectID)
{
    return GConsole->GetEquipSlot(BodyPart, ObjectID);
}
NW_HOOK_METHOD(0x1007b490, NCEquipItemWnd, GetEquipSlot)


// 0x1007b5c0 (vslot 55) - the icons in their slots, the two-slot items' in both (the second under the "dual" cap),
// with the second and third icons when the item has them; a bow is drawn in the right hand alone while an item with
// Unknown1C98 is equipped. C2: then the tattoos, a column of boxes of 26 pixels (HennaBackTex) for as many slots as the
// character may have (the console's 0x10060E90), each with its icon in 24 and, when UnknownC34 is 0, dimmed by
// NWindow.ChatBack
INT NCEquipItemWnd::OnPaint(NCGDevice* Device)
{
    guard(NCEquipItemWnd::OnPaint);
    Device->SetClip(ScreenX - 1, ScreenY - 1, Width + 2, Height + 2);
    Device->BeginPaint(ScreenX, ScreenY);
    INT X = 0;
    INT Y = 0;
    for (INT i = 0; i < Items.Num(); i++)
    {
        NItemEntry* Item = Items(i);
        if (!Item)
            continue;
        INT Slot = GetEquipSlot(Item->BodyPart, Item->ObjectID);
        if ((DWORD)Slot > 15)
            continue;
        UBOOL bTwoSlots = 0;
        if (Slot < 14)
        {
            X = SlotX[Slot];
            Y = SlotY[Slot];
        }
        else
            bTwoSlots = 1;
        if (Item->UnknownC30 == 6 && bHasUnknown1C98)
        {
            X = 0;
            Y = 35;
        }
        else if (bTwoSlots)
        {
            UTexture* First;
            UTexture* Second;
            if (Item->Unknown30C[0])
            {
                First = Item->GetIcon2();
                Second = Item->GetIcon3();
            }
            else if (Item->Unknown20C[0])
            {
                First = Item->GetIcon2();
                Second = Item->GetIcon2();
            }
            else
            {
                First = Item->GetIcon();
                Second = Item->GetIcon();
            }
            if (Slot == 15)
            {
                Device->DrawTexture(0, 35, CellSize, CellSize, 0, 0, CellSize, CellSize, First);
                Device->DrawTexture(70, 35, CellSize, CellSize, 0, 0, CellSize, CellSize, Second);
                Device->DrawTexture(70, 35, CellSize, CellSize, 0, 0, CellSize, CellSize, DualCapTex);
                if (Item->IsSelected())
                    Device->DrawTexture(-1, 34, 34, 34, 0, 0, 34, 34, SelectTex);
            }
            else if (Slot == 14)
            {
                Device->DrawTexture(35, 35, CellSize, CellSize, 0, 0, CellSize, CellSize, First);
                Device->DrawTexture(35, 70, CellSize, CellSize, 0, 0, CellSize, CellSize, Second);
                Device->DrawTexture(35, 70, CellSize, CellSize, 0, 0, CellSize, CellSize, DualCapTex);
                if (Item->IsSelected())
                    Device->DrawTexture(34, 34, 34, 34, 0, 0, 34, 34, SelectTex);
            }
            continue;
        }
        Device->DrawTexture(X, Y, CellSize, CellSize, 0, 0, CellSize, CellSize, Item->GetIcon());
        if (Item->IsSelected())
            Device->DrawTexture(X - 1, Y - 1, 34, 34, 0, 0, 34, 34, SelectTex);
    }
    if (HennaBackTex)
    {
        // 0x10060E90: how many tattoo slots the character has (1..3 by its class, -1 none)
        INT NumHennaSlots = GConsole->GetHennaSlotCount();
        for (INT i = 0; i < NumHennaSlots; i++)
        {
            Device->DrawTexture(0x6C, i * 27, 26, 26, 0, 0, 26, 26, HennaBackTex);
            if (i < Hennas.Num() && Hennas(i)->GetIcon())
            {
                UTexture* Icon = Hennas(i)->GetIcon();
                INT TopY = i * 27 + 1;
                Device->DrawStretchedTex(0x6D, TopY, 24, 24, Icon);
                if (Hennas(i)->UnknownC34 == 0)
                    Device->DrawTexture(0x6D, TopY, 24, 24, 0, 0, 24, 24,
                                        FindTexture(TEXT("NWindow.ChatBack")));
            }
        }
    }
    Device->EndPaint();
    Device->ReleaseClip();
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x1007b5c0, NCEquipItemWnd, OnPaint)

// 0x10080480 (vslot 56) - the tool tip of the slot under the mouse (C1 had a box of text): the position of the
// slot's cell (the 5 x 3 grid, cells CellSize + 4 apart; slot 8 sits 0x13 further and slot 9 on its own row) is
// where the skin puts the box (it moves it to stay on the screen): the small box (title) for an item that is
// not selected, the full one for the selected item (and for the tattoo under the mouse). The two-slot items
// (14, 15) answer for the second slot they cover (6, 5).
INT NCEquipItemWnd::OnDrawToolTip(NCGDevice* Device)
{
    Device->BeginPaint(ScreenX, ScreenY);
    Device->SetClip(ScreenX, ScreenY, Width, Height);
    INT X = 0;
    INT Y = 0;
    INT Slot = HoverSlot;
    if (Slot >= 0 && Slot < 5)
    {
        X = (CellSize + 4) * Slot;
        Y = 4;
    }
    else if (Slot == 5)
    {
        X = 0;
        Y = CellSize + 4;
    }
    else if (Slot > 5 && Slot < 8)
    {
        X = (Slot - 5) * (CellSize + 4);
        Y = CellSize + 4;
    }
    else if (Slot == 8)
    {
        X = (CellSize + 4) * 3 + 0x13;
        Y = CellSize + 4;
    }
    else if (Slot == 9)
    {
        X = 0;
        Y = CellSize * 2 + 8;
    }
    else if (Slot > 9 && Slot <= 13)
    {
        X = (Slot - 9) * (CellSize + 4);
        Y = (CellSize + 4) * 2;
    }
    for (INT i = 0; i < Items.Num(); i++)
    {
        INT EquipSlot = GetEquipSlot(Items(i)->BodyPart, Items(i)->ObjectID);
        if (HoverSlot == EquipSlot || (EquipSlot == 0xF && HoverSlot == 5) || (EquipSlot == 0xE && HoverSlot == 6))
        {
            NToolTipRect Wnd;
            Wnd.X = ScreenX;
            Wnd.Y = ScreenY;
            Wnd.W = Width;
            Wnd.H = Height;
            if (!Items(i)->IsSelected())
            {
                if (ToolTipSkin)
                    ToolTipSkin->Draw(Device, Items(i)->ToolTip, &X, &Y, Wnd);
            }
            else if (ToolTipSkin)
                ToolTipSkin->DrawFull(Device, Items(i)->ToolTip, &X, &Y, Wnd);
            break;
        }
    }
    if (HoverHenna >= 0 && HoverHenna < Hennas.Num() && Hennas(HoverHenna) != NULL)
    {
        Y = HoverHenna * 26;
        X = 0x6C;
        if (ToolTipSkin)
        {
            NToolTipRect Wnd;
            Wnd.X = ScreenX;
            Wnd.Y = ScreenY;
            Wnd.W = Width;
            Wnd.H = Height;
            ToolTipSkin->DrawFull(Device, Hennas(HoverHenna)->ToolTip, &X, &Y, Wnd);
        }
    }
    Device->ReleaseClip();
    Device->EndPaint();
    return 1;
}
NW_HOOK_METHOD(0x10080480, NCEquipItemWnd, OnDrawToolTip)

// 0x1007baa0
void NCEquipItemWnd::UnselectAll()
{
    for (INT i = 0; i < Items.Num(); i++)
        Items(i)->Unselect();
}
NW_HOOK_METHOD(0x1007baa0, NCEquipItemWnd, UnselectAll)

// 0x1007bad0 - the slot's 32 x 32 pixels (whatever CellSize); out of every slot the answer is -1 (C1: 0, the first
// slot)
INT NCEquipItemWnd::HitTest(INT X, INT Y)
{
    for (INT Slot = 0; Slot < 14; Slot++)
        if (PtInRect(X, Y, ScreenX + SlotX[Slot], ScreenY + SlotY[Slot], 32, 32))
            return Slot;
    return -1;
}
NW_HOOK_METHOD(0x1007bad0, NCEquipItemWnd, HitTest)


// 0x1007bc60 - the item with Info's object id takes Info's fields and a new tool tip; the entry built for that is
// never freed, as in the stock code
UBOOL NCEquipItemWnd::UpdateItem(NItemInfo* Info)
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
NW_HOOK_METHOD(0x1007bc60, NCEquipItemWnd, UpdateItem)

// 0x1007bd60
UBOOL NCEquipItemWnd::HasItem(INT ObjectID)
{
    for (INT i = 0; i < Items.Num(); i++)
        if (ObjectID == Items(i)->ObjectID)
            return 1;
    return 0;
}
NW_HOOK_METHOD(0x1007bd60, NCEquipItemWnd, HasItem)

// 0x1007bdb0
UBOOL NCEquipItemWnd::FindItem(INT ObjectID, INT* Index)
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
NW_HOOK_METHOD(0x1007bdb0, NCEquipItemWnd, FindItem)

// 0x1007d0c0 (vslot 61) - selects the item of the slot under the mouse and picks it up (unless bNoDrag, which lasts
// one click); a two-slot item met on the way unselects all. C2: the questions pending are not forgotten here, the
// desktop's box is left alone.
INT NCEquipItemWnd::OnLButtonDown(WPARAM wParam, LPARAM lParam)
{
    guard(NCEquipItemWnd::OnLButtonDown);
    INT Slot = HitTest(LOWORD(lParam), HIWORD(lParam));
    HoverSlot = Slot;
    Selected = Slot;
    UBOOL bFound = 0;
    for (INT i = 0; i < Items.Num(); i++)
    {
        INT ItemSlot = GetEquipSlot(Items(i)->BodyPart, Items(i)->ObjectID);
        if (ItemSlot == 15)
        {
            UnselectAll();
            if (Selected != 5)
                continue;
        }
        else if (ItemSlot == 14)
        {
            UnselectAll();
            if (Selected != 6)
                continue;
        }
        else if (Selected == ItemSlot)
            UnselectAll();
        else
            continue;
        Items(i)->Select();
        SelectedIndex = i;
        bFound = 1;
        break;
    }
    if (!bFound)
        SelectedIndex = -1;
    if (!bNoDrag)
    {
        bDragging = 1;
        PlayInterfaceSound(SOUND_PickUp);
    }
    else
        bNoDrag = 0;
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x1007d0c0, NCEquipItemWnd, OnLButtonDown)

// 0x1007bbd0 (name ours) - the tattoo under the mouse (its box, relative to the window), -1 outside
INT NCEquipItemWnd::HitTestHenna(INT X, INT Y)
{
    INT RX = X - ScreenX;
    INT RY = Y - ScreenY;
    for (INT i = 0; i < Hennas.Num(); i++)
    {
        if (PtInRect(RX, RY, Hennas(i)->BoxX, Hennas(i)->BoxY, Hennas(i)->BoxW, Hennas(i)->BoxH))
            return i;
    }
    return -1;
}
NW_HOOK_METHOD(0x1007bbd0, NCEquipItemWnd, HitTestHenna)

// 0x1007d250 (vslot 60) - NCInvenItemWnd::OnMouseMove for the paperdoll, with the tattoo under the mouse too
INT NCEquipItemWnd::OnMouseMove(WPARAM wParam, LPARAM lParam)
{
    INT X = LOWORD(lParam);
    INT Y = HIWORD(lParam);
    HoverSlot = HitTest(X, Y);
    HoverHenna = HitTestHenna(X, Y);
    if (IsFocused())
    {
        if (!bDragging)
        {
            if (!GConsole->IsCtrlDown() && !bEnchantMode)
            {
                UnlockCursor();
                SetCursor(0, NULL);
            }
        }
        else if (Selected >= 0 && Selected < NumSlots && SelectedIndex < Items.Num() && SelectedIndex >= 0)
        {
            SetCursor(4, Items(SelectedIndex)->GetIcon());
            LockCursor();
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
NW_HOOK_METHOD(0x1007d250, NCEquipItemWnd, OnMouseMove)

// 0x1007d3e0 (vslot 62) - where the carried item is put down: on an item page it is taken off (the ring from one finger
// only), on a shortcut bar it is registered, on the pet's inventory it is given (a stack asks how many first), on the
// trash it is destroyed after a question (the full name of the item in it), on the crystallize icon it is crystallized
// after one, and on the game it is dropped after one (unless a pawn is under the mouse). C2: the questions carry the
// item's object id (SetParams) and the count (+0x74) instead of the window remembering them; the drop's question also
// carries the ground point (+0x78, NDialogInfo's trailing vector: NCDialogBox::SetInfo copies it to the box's +0x22C).
INT NCEquipItemWnd::OnLButtonUp(WPARAM wParam, LPARAM lParam)
{
    if (!GConsole->IsCtrlDown() && !bEnchantMode)
    {
        UnlockCursor();
        SetCursor(0, NULL);
    }
    INT Index = 0;
    if (Selected < 0 || Selected >= NumSlots)
    {
        bDragging = 0;
        return 1;
    }
    if (0 < Items.Num())
    {
        UBOOL bMatch;
        do
        {
            INT ItemSlot = GetEquipSlot(Items(Index)->BodyPart, Items(Index)->ObjectID);
            if (ItemSlot == 15)
                bMatch = Selected == 5;
            else if (ItemSlot == 14)
                bMatch = Selected == 6;
            else
                bMatch = Selected == ItemSlot;
        } while (!bMatch && ++Index < Items.Num());
    }
    if (Index == Items.Num())
    {
        bDragging = 0;
        return 1;
    }
    if (!bDragging || Items(Index) == NULL)
    {
        bDragging = 0;
        return 1;
    }
    NItemEntry* Item = Items(Index);
    if (bEnchantMode)
    {
        GConsole->RequestEnchantItem(Item->ObjectID);
        bDragging = 0;
        return 1;
    }
    INT X = LOWORD(lParam);
    INT Y = HIWORD(lParam);
    NWndEntry* Found = FindWindowAt(X, Y);
    NCWnd* Wnd = Found ? Found->Wnd : NULL;
    if (Wnd)
    {
        if (Wnd->IsA(NClass::NCInvenItemWnd()))
        {
            // taken off (not an item with Unknown1C98)
            if (Item->Unknown1C98 == 0)
            {
                L2ParamStack Params(10);
                INT BodyPart = Items(Index)->BodyPart;
                if (BodyPart & 6)
                {
                    User* Me = GConsole->GetMyUser();
                    if (Me->Equipped[1] == Items(Index)->ObjectID)
                        Params.PushBack((void*)2);
                    if (Me->Equipped[2] == Items(Index)->ObjectID)
                        Params.PushBack((void*)4);
                }
                else if (BodyPart & 0x30)
                {
                    User* Me = GConsole->GetMyUser();
                    if (Me->Equipped[4] == Items(Index)->ObjectID)
                        Params.PushBack((void*)0x10);
                    else if (Me->Equipped[5] == Items(Index)->ObjectID)
                        Params.PushBack((void*)0x20);
                }
                else
                    Params.PushBack((void*)(INT_PTR)BodyPart);
                GConsole->RequestUnEquipItem(Params);
                UnselectAll();
                Selected = -1;
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
                Params.PushBack((void*)(INT_PTR)Items(Index)->ObjectID);
                Params.PushBack((void*)1);
                GConsole->RequestShortCutReg(Params);
            }
            UnselectAll();
            Selected = -1;
        }
        else if (Wnd->IsA(NClass::NCPetInvenItemWnd()))
        {
            if (!Item->IsStackable() || Item->Count < 2)
            {
                // the item of the index of the slot, as in the stock code
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
                if (Item->ID == ITEM_ADENA)
                {
                    Info.SetSystemMessage(MSG_PetAdenaHowMuch);
                    Info.SetButtons(Name, NULL);
                    Info.Unknown74 = Item->Count;
                    Info.bLargeAmount = 1;
                }
                else
                {
                    Info.SetSystemMessage(MSG_PetHowMuch);
                    Info.SetButtons(Name, NULL);
                    Info.Unknown74 = Item->Count;
                    Info.bLargeAmount = 0;
                }
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
                    Info.bInput = 0;
                    Info.bMainBox = 1;
                    Info.Owner = Entry;
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
                    Info.Unknown74 = Item->Count;
                    Info.bLargeAmount = Item->ID == ITEM_ADENA;
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
                    Info.bInput = 0;
                    Info.bMainBox = 1;
                    Info.Owner = Entry;
                    Info.SetSystemMessage(MSG_Crystallize);
                    Info.SetButtons(Name, NULL);
                    Info.bLargeAmount = 0;
                    Info.SetParams(Item->ObjectID, -1);
                    GConsole->ShowDialog(&Info);
                }
            }
        }
        else if (Wnd->IsA(NClass::NCConsole()))
        {
            // on the ground, unless a pawn is under the mouse
            if (!GConsole->GetTargetedActor(1))
            {
                TCHAR Name[256];
                appSprintf(Name, TEXT("%s"), Item->GetName());
                DropLocation = static_cast<UGameEngine*>(GConsole->Engine)->MouseHitLocation;
                if (!Item->IsStackable() || Item->Count < 2)
                {
                    NDropDialogInfo Info;
                    Info.ID = DIALOG_DROP_ITEM;
                    Info.bMainBox = 1;
                    Info.bInput = 0;
                    Info.Owner = Entry;
                    Info.SetSystemMessage(MSG_Drop);
                    Info.SetButtons(Name, NULL);
                    Info.bLargeAmount = 0;
                    Info.SetParams(Item->ObjectID, -1);
                    Info.Location = static_cast<UGameEngine*>(GConsole->Engine)->MouseHitLocation;
                    GConsole->ShowDialog(&Info);
                }
                else
                {
                    NDropDialogInfo Info;
                    Info.ID = DIALOG_DROP_ITEM;
                    Info.Type = 3;
                    Info.bMainBox = 1;
                    Info.bInput = 1;
                    Info.Owner = Entry;
                    Info.SetSystemMessage(MSG_DropHowMany);
                    Info.SetButtons(Name, NULL);
                    Info.Unknown74 = Item->Count;
                    Info.bLargeAmount = Item->ID == ITEM_ADENA;
                    Info.SetParams(Item->ObjectID, -1);
                    Info.Location = static_cast<UGameEngine*>(GConsole->Engine)->MouseHitLocation;
                    GConsole->ShowDialog(&Info);
                }
            }
        }
    }
    bDragging = 0;
    return 1;
}
NW_HOOK_METHOD(0x1007d3e0, NCEquipItemWnd, OnLButtonUp)

// 0x1007def0 (vslot 63) - takes off the item of the slot under the mouse (nothing once an item with Unknown1C98 comes
// first in the list), then the shortcut bars forget their selections. C2: no dialog to close, no pending flags.
INT NCEquipItemWnd::OnRButtonDown(WPARAM wParam, LPARAM lParam)
{
    Selected = HitTest(LOWORD(lParam), HIWORD(lParam));
    if (Selected >= 0 && Selected < NumSlots)
    {
        for (INT i = 0; i < Items.Num(); i++)
        {
            NItemEntry* Item = Items(i);
            if (Item && Item->Unknown1C98)
                break;
            if (UnEquipIfInSlot(this, Item, Selected, 1))
                break;
        }
    }
    bDragging = 0;
    return 1;
}
NW_HOOK_METHOD(0x1007def0, NCEquipItemWnd, OnRButtonDown)

// 0x1007e380 (vslot 64) - takes off the item of the slot under the mouse; the release that follows moves nothing
INT NCEquipItemWnd::OnLButtonDblClick(WPARAM wParam, LPARAM lParam)
{
    bNoDrag = 1;
    Selected = HitTest(LOWORD(lParam), HIWORD(lParam));
    if (Selected >= 0 && Selected < NumSlots)
    {
        for (INT i = 0; i < Items.Num(); i++)
            if (UnEquipIfInSlot(this, Items(i), Selected, 0))
                break;
    }
    UnselectAll();
    Selected = -1;
    bDragging = 0;
    return 1;
}
NW_HOOK_METHOD(0x1007e380, NCEquipItemWnd, OnLButtonDblClick)

// 0x1007f7e0 (vslot 37)
INT NCEquipItemWnd::OnDestroy()
{
    guard(NCEquipItemWnd::OnDestroy);
    for (INT i = 0; i < Items.Num(); i++)
        if (Items(i))
            delete Items(i);
    Items.Empty();
    NCWnd::OnDestroy();
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x1007f7e0, NCEquipItemWnd, OnDestroy)

// 0x1007e5e0 - the three icons come from their names ("NWindow.BlackTexture" for those missing), written back into
// Info; 0 when the fourteen slots are taken. Every add checks again whether an item with Unknown1C98 is equipped.
UBOOL NCEquipItemWnd::AddItem(NItemInfo* Info)
{
    if (Items.Num() >= NumSlots)
        return 0;
    if (Info->IconName[0])
        Info->Icon = FindTexture(Info->IconName);
    if (Info->Unknown200[0])
        Info->UnknownC04 = (INT)(INT_PTR)FindTexture(Info->Unknown200);
    if (Info->Unknown300[0])
        Info->UnknownC08 = (INT)(INT_PTR)FindTexture(Info->Unknown300);
    if (!Info->Icon)
        Info->Icon = FindTexture(TEXT("NWindow.BlackTexture"));
    if (!Info->UnknownC04)
        Info->UnknownC04 = (INT)(INT_PTR)FindTexture(TEXT("NWindow.BlackTexture"));
    if (!Info->UnknownC08)
        Info->UnknownC08 = (INT)(INT_PTR)FindTexture(TEXT("NWindow.BlackTexture"));
    NItemEntry* Item = new NItemEntry(Info);
    Item->FillToolTip(2, 0);
    Items.AddItem(Item);
    for (INT i = 0; i < Items.Num(); i++)
    {
        if (Items(i)->Unknown1C98)
        {
            bHasUnknown1C98 = 1;
            break;
        }
        bHasUnknown1C98 = 0;
    }
    return 1;
}
NW_HOOK_METHOD(0x1007e5e0, NCEquipItemWnd, AddItem)

// 0x1007f8b0 (C1 0x100675c0) - the item is deleted; bHasUnknown1C98 stays as it was
UBOOL NCEquipItemWnd::RemoveItem(INT ObjectID)
{
    for (INT i = 0; i < Items.Num(); i++)
    {
        if (Items(i)->ObjectID == ObjectID)
        {
            NItemEntry* Item = Items(i);
            Items.Remove(i);
            if (Item)
                delete Item;
            return 1;
        }
    }
    return 0;
}
NW_HOOK_METHOD(0x1007f8b0, NCEquipItemWnd, RemoveItem)

// 0x1007f980 (name ours) - a tattoo of the game data (nothing when the id is not in it): the entry takes the tattoo's
// name (+8 of the data), description (+0x20) and icon name (+0x14), the icon (the black texture when it is not found),
// bUsable in UnknownC34 (the paint dims the icon when it is 0), the box of the next slot, and its tool tip
void NCEquipItemWnd::AddHenna(INT HennaID, INT bUsable)
{
    FL2HennaData* Data = GL2GameData.GetHennaData(HennaID);
    if (!Data)
        return;
    NHennaEntry* Henna = new NHennaEntry;
    appStrcpy(Henna->Name, *Data->Name);
    appStrcpy(Henna->Description, *Data->Description);
    appStrcpy(Henna->IconName, *Data->IconName);
    Henna->UnknownC34 = bUsable;
    if (Henna->IconName)
        Henna->Icon = FindTexture(Henna->IconName);
    if (!Henna->Icon)
        Henna->Icon = FindTexture(TEXT("NWindow.BlackTexture"));
    Henna->BoxY = Hennas.Num() * 27;
    Henna->BoxX = 0x6D;
    Henna->BoxW = 24;
    Henna->BoxH = 24;
    Henna->FillToolTip(0x80, 0);
    Hennas.AddItem(Henna);
}
NW_HOOK_METHOD(0x1007f980, NCEquipItemWnd, AddHenna)

// 0x10080740 (name ours) - every tattoo deleted
void NCEquipItemWnd::ClearHennas()
{
    for (INT i = 0; i < Hennas.Num(); i++)
        if (Hennas(i))
            delete Hennas(i);
    Hennas.Empty();
}
NW_HOOK_METHOD(0x10080740, NCEquipItemWnd, ClearHennas)

// 0x1007f920
void NCEquipItemWnd::Clear()
{
    for (INT i = 0; i < Items.Num(); i++)
        if (Items(i))
            delete Items(i);
    Items.Empty();
}
NW_HOOK_METHOD(0x1007f920, NCEquipItemWnd, Clear)

// stock body shared with NCEditBox's (0x100a5d70)
void NCEquipItemWnd::ShowWindow()
{
    NCWnd::ShowWindow();
}

// stock body shared with NCChatWnd's (0x1003dab0)
void NCEquipItemWnd::HideWindow()
{
    NCWnd::HideWindow();
}

NW_CHECK_VTABLE(NCEquipItemWnd, 0x10117948, 69)
