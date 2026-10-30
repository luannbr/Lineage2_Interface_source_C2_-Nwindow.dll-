/*=============================================================================
    NCInventoryWnd.cpp - the inventory. Stock: 0x100641f0 .. 0x10064289, 0x10065920 ..
    0x10066182, 0x100681a0 .. 0x10068373 and OnCreate (0x100684d0); the item pages' and the
    paperdoll's functions sit in between.
=============================================================================*/
#include "nwindow/NCInventoryWnd.h"
#include "nwindow/NCEquipItemWnd.h"
#include "nwindow/NCButton.h"
#include "nwindow/NCFrameCtrl.h"
#include "nwindow/NCGDevice.h"
#include "nwindow/NCIcon.h"
#include "nwindow/NCInvenItemWnd.h"
#include "nwindow/NCTabControl.h"

#include <stdio.h>
#include <wchar.h>

// ids (names ours)
enum
{
    NID_INVENTORY_FRAME = 0xF2,
    NID_INVENTORY_TAB = 0xFE001A,
    NID_INVENTORY_MANUFACTURE = 0xFE001C,
};

// sysstring
enum
{
    STR_Item = 2,
    STR_Quest = 118,
};

// 0x1007f470 - the paperdoll, the crystallize child and most textures are left as allocated
NCInventoryWnd::NCInventoryWnd()
{
    BackTex = NULL;
    TabCtrl = NULL;
    ItemWnd[0] = NULL;
    ItemWnd[1] = NULL;
    bOverCrystallize = 0;
    bOverTrash = 0;
    TrashW = 32;
    TrashH = 32;
    TrashX = 190;
    TrashY = 315;
    CrystallizeW = 32;
    CrystallizeH = 32;
    CrystallizeX = 10;
    CrystallizeY = 315;
}
NW_HOOK_CTOR(0x1007f470, NCInventoryWnd)


// 0x10080250 / 0x1007f560 are below. 0x1007f560 (the destructor) - ItemOrder's destructor is TArray<INT>'s, then the base
NW_HOOK_DTOR(0x1007f560, NCInventoryWnd)

// 0x1007b4a0 (vslot 25) - C2: no IsEnabled test; the manufacture button
INT NCInventoryWnd::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (NCWnd::WindowProc(Msg, wParam, lParam))
        return 1;
    if (Msg == NWM_COMMAND && wParam == NID_INVENTORY_MANUFACTURE)
        return OnManufacture() != 0;
    return 0;
}
NW_HOOK_METHOD(0x1007b4a0, NCInventoryWnd, WindowProc)

// 0x1007b5a0 (vslot 67; name ours) - the manufacture button: the recipe book is asked for
INT NCInventoryWnd::OnManufacture()
{
    GConsole->RequestRecipeBookOpen();
    return 1;
}
NW_HOOK_METHOD(0x1007b5a0, NCInventoryWnd, OnManufacture)

// 0x1007cd20 (vslot 60) - with an enchant scroll in use (the paperdoll's mode) the mouse leaving the window ends it: the
// item windows are told (-1: no item) and the selection cleared
INT NCInventoryWnd::OnMouseMove(WPARAM wParam, LPARAM lParam)
{
    if (EquipWnd->bEnchantMode)
    {
        if (!PtInRect((SWORD)LOWORD(lParam), (SWORD)HIWORD(lParam), LocalX, LocalY, LocalW, LocalH))
        {
            GConsole->RequestEnchantItem(-1);
            ClearItemSelect();
        }
    }
    return 1;
}
NW_HOOK_METHOD(0x1007cd20, NCInventoryWnd, OnMouseMove)

// 0x1007f610 (name ours) - the ids of the "Item" page's items, in their order, remembered
void NCInventoryWnd::SaveItemOrder()
{
    ItemOrder.Empty();
    INT Count = ItemWnd[0]->Items.Num();
    for (INT i = 0; i < Count; i++)
        ItemOrder.AddItem(ItemWnd[0]->Items(i)->ID);
}
NW_HOOK_METHOD(0x1007f610, NCInventoryWnd, SaveItemOrder)

// 0x1007f690 (name ours) - the page's last item moves to the front (a new item shows first)
void NCInventoryWnd::MoveLastToFront()
{
    NCInvenItemWnd* Page = ItemWnd[0];
    INT Count = Page->Items.Num();
    if (Count > -1)
    {
        NItemEntry* Last = Page->Items(Count - 1);
        Page->Items.Remove(Count - 1, 1);
        Page->Items.Insert(0, 1);
        Page->Items(0) = Last;
    }
}
NW_HOOK_METHOD(0x1007f690, NCInventoryWnd, MoveLastToFront)

// 0x1007f6f0 (vslot 44) - the page's order is remembered when it has items
void NCInventoryWnd::HideWindow()
{
    NCWnd::HideWindow();
    if (ItemWnd[0]->Items.Num() > 0)
        SaveItemOrder();
}
NW_HOOK_METHOD(0x1007f6f0, NCInventoryWnd, HideWindow)

// 0x10080260 (name ours) - the "Item" page back in the remembered order: the leading items that are not in the list stay
// first; then, for each remembered id in turn, the first item of that id at or after the same index (a place per id found)
// swaps its contents (through a temporary entry) with the next place
void NCInventoryWnd::RestoreItemOrder()
{
    NCInvenItemWnd* Page = ItemWnd[0];
    INT Skip = 0;
    for (INT i = 0; i < Page->Items.Num(); i++)
    {
        INT j;
        for (j = 0; j < ItemOrder.Num(); j++)
            if (Page->Items(i)->ID == ItemOrder(j))
                break;
        if (j < ItemOrder.Num())
            break;
        Skip++;
    }
    for (INT k = 0; k < ItemOrder.Num(); k++)
    {
        INT Count = ItemWnd[0]->Items.Num();
        for (INT i = k; i < Count; i++)
        {
            if (ItemOrder(k) == ItemWnd[0]->Items(i)->ID)
            {
                NItemEntry Temp;
                Temp.Copy(ItemWnd[0]->Items(Skip));
                ItemWnd[0]->Items(Skip)->Copy(ItemWnd[0]->Items(i));
                ItemWnd[0]->Items(i)->Copy(&Temp);
                Skip++;
                break;
            }
        }
    }
}
NW_HOOK_METHOD(0x10080260, NCInventoryWnd, RestoreItemOrder)

// 0x10080440 (name ours) - the console calls it: only when the page has items
void NCInventoryWnd::ApplyItemOrder()
{
    if (ItemWnd[0]->Items.Num() > 0)
        RestoreItemOrder();
}
NW_HOOK_METHOD(0x10080440, NCInventoryWnd, ApplyItemOrder)

// 0x1007b500 - edges included, as IsInCrystallize. (Left a stub while the two stock callers that keep ecx across the
// call, NCEquipItemWnd::OnLButtonUp 1007d3e0 and NCInvenItemWnd::OnLButtonUp 1007e790, were not ours.)
UBOOL NCInventoryWnd::IsInTrash(INT X, INT Y)
{
    X -= ScreenX + TrashX;
    Y -= ScreenY + TrashY;
    return X >= 0 && X <= TrashW && Y >= 0 && Y <= TrashH;
}
NW_HOOK_METHOD(0x1007b500, NCInventoryWnd, IsInTrash)

// 0x10080250 (name ours) - the paperdoll's AddHenna (a tail jump in the stock code; the console's slot 137 jumps here)
void NCInventoryWnd::AddHenna(INT HennaID, INT bUsable)
{
    EquipWnd->AddHenna(HennaID, bUsable);
}
NW_HOOK_METHOD(0x10080250, NCInventoryWnd, AddHenna)

// 0x100807a0 (name ours) - the paperdoll's ClearHennas (a tail jump; the console's slot 136 jumps here)
void NCInventoryWnd::ClearHennas()
{
    EquipWnd->ClearHennas();
}
NW_HOOK_METHOD(0x100807a0, NCInventoryWnd, ClearHennas)

// 0x1007b550 - edges included
UBOOL NCInventoryWnd::IsInCrystallize(INT X, INT Y)
{
    X -= ScreenX + CrystallizeX;
    Y -= ScreenY + CrystallizeY;
    return X >= 0 && X <= CrystallizeW && Y >= 0 && Y <= CrystallizeH;
}
NW_HOOK_METHOD(0x1007b550, NCInventoryWnd, IsInCrystallize)

// 0x1007c7a0 (vslot 55) - the background, the "Item" page's adena (thousands separators, right aligned to 184), the
// weight bar (87 pixels at full load; its texture by the load: up to 50%, 66.66%, 80%, above; the first texture sampled
// 10 rows high, the others 12) with the percent centered on it, the crystallize (with the skill) and trash icons, lit
// while an item is carried over them, and (C2) the number of items out of the most the player may carry, in brown at
// the window's right edge. The buffers are the stock frame's 128 characters (64 for the count).
INT NCInventoryWnd::OnPaint(NCGDevice* Device)
{
    guard(NCInventoryWnd::OnPaint);
    Device->BeginPaint(ScreenX, ScreenY);
    Device->SetClip(ScreenX, ScreenY, Width, Height);
    if (BackTex)
        Device->DrawTexture(0, 21, 240, 344, 0, 0, 240, 344, BackTex);

    TCHAR Buf[128];
    TCHAR Amount[128];
    TCHAR CountText[64];
    SIZE Size;
    appSprintf(Buf, TEXT("%d"), ItemWnd[0]->GetAdena());
    FormatNumber(Amount, Buf);
    appSprintf(Buf, TEXT("%s"), Amount);
    NCGDevice::GetTextExtent(&Size, Buf, 0, 0, 0, 0, 0);
    Device->DrawText(184 - Size.cx, 317, 0xFFFFFFFF, Buf, 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
    if (AdenaTex)
        Device->DrawTexture(79, 317, 16, 12, 0, 0, 16, 12, AdenaTex);
    if (WeightIconTex)
        Device->DrawTexture(79, 333, 16, 12, 0, 0, 16, 12, WeightIconTex);

    User* Me = GConsole->GetMyUser();
    if (Me)
    {
        FLOAT Percent = 0.0f;
        if (Me->MaxLoad)
        {
            INT W = Me->Load * 87 / Me->MaxLoad;
            if (W > 0)
            {
                // the first test uses the unrounded quotient (fst, then fcomp); the others reload the stored float, as
                // the stock code does
                DOUBLE Pct = (DOUBLE)Me->Load / Me->MaxLoad * 100.0;
                Percent = (FLOAT)Pct;
                if (Pct <= 50.0)
                    Device->DrawTexture(100, 333, W, 12, 0, 0, W, 10, WeightTex[0]);
                else if (Percent <= 66.66)
                    Device->DrawTexture(100, 333, W, 12, 0, 0, W, 12, WeightTex[1]);
                else if (Percent >= 66.66 && Percent <= 80.0f)
                    Device->DrawTexture(100, 333, W, 12, 0, 0, W, 12, WeightTex[2]);
                else if (Percent > 80.0f)
                    Device->DrawTexture(100, 333, W, 12, 0, 0, W, 12, WeightTex[3]);
            }
        }
        TCHAR PercentText[128];
        swprintf(PercentText, ARRAY_COUNT(PercentText), TEXT("%.2f%s"), (DOUBLE)Percent, TEXT("%"));
        NCGDevice::GetTextExtent(&Size, PercentText, 0, 0, 0, 0, 0);
        Device->DrawSpecialDigit(1, 141 - Size.cx / 2, 334, 0xFFFFFFFF, PercentText);
        if (Me->bCanCrystallize)
            Device->DrawTexture(CrystallizeX, CrystallizeY, 32, 32, 0, 0, 32, 32,
                                bOverCrystallize ? CrystallizeActiveTex : CrystallizeTex);
    }
    Device->DrawTexture(TrashX, TrashY, 32, 32, 0, 0, 32, 32, bOverTrash ? TrashActiveTex : TrashTex);
    if (Me)
        appSprintf(CountText, TEXT("(%d/%d)"), EquipWnd->Items.Num() + ItemWnd[1]->Items.Num() + ItemWnd[0]->Items.Num(),
                   (INT)nw::Field<SWORD>(Me, 0x1F6));
    NCGDevice::GetTextExtent(&Size, CountText, 0, 0, 0, 0, 0);
    Device->DrawText(Width - Size.cx - 5, 143, 0xFFB09B79, CountText, 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
    Device->EndPaint();
    Device->ReleaseClip();
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x1007c7a0, NCInventoryWnd, OnPaint)

// 0x1007cbd0 (vslot 37) - the children go in the order of the fields, the manufacture button last
INT NCInventoryWnd::OnDestroy()
{
    guard(NCInventoryWnd::OnDestroy);
    if (ItemWnd[0])
    {
        RemoveChildWindow(ItemWnd[0]->Entry);
        delete ItemWnd[0];
        ItemWnd[0] = NULL;
    }
    if (ItemWnd[1])
    {
        RemoveChildWindow(ItemWnd[1]->Entry);
        delete ItemWnd[1];
        ItemWnd[1] = NULL;
    }
    if (EquipWnd)
    {
        RemoveChildWindow(EquipWnd->Entry);
        delete EquipWnd;
        EquipWnd = NULL;
    }
    if (TabCtrl)
    {
        RemoveChildWindow(TabCtrl->Entry);
        delete TabCtrl;
        TabCtrl = NULL;
    }
    if (ManufactureButton)
    {
        RemoveChildWindow(ManufactureButton->Entry);
        delete ManufactureButton;
        ManufactureButton = NULL;
    }
    NCWnd::OnDestroy();
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x1007cbd0, NCInventoryWnd, OnDestroy)

// 0x1007cda0 (vslot 43) - the tab's page shows again; coming back from the icon the window opens where the icon
// was; the manufacture button shows with the crystallize icon
void NCInventoryWnd::ShowWindow()
{
    NCWnd::ShowWindow();
    TabCtrl->ShowCurrentPage();
    if (Icon)
    {
        SetWindowRect(Icon->ScreenX, Icon->ScreenY, Width, Height);
        Icon->SetWindowRect(0, 0, 0, 0);
        Icon->HideWindow();
    }
    if (GConsole->GetMyUser() && GConsole->GetMyUser()->bCanCrystallize)
        ManufactureButton->ShowWindow();
    else
        ManufactureButton->HideWindow();
}
NW_HOOK_METHOD(0x1007cda0, NCInventoryWnd, ShowWindow)

// 0x1007ce40 - an enchant scroll in use (the console): the item windows keep the cursor, which shows the icon of the
// item picked up
void NCInventoryWnd::SetEnchantMode(UBOOL bOn)
{
    EquipWnd->SetEnchantMode(bOn);
    ItemWnd[0]->SetEnchantMode(bOn);
    if (bOn)
    {
        NCInvenItemWnd* Page = ItemWnd[0];
        if (Page && Page->Selected >= 0 && Page->Selected < Page->Items.Num())
        {
            SetCursor(4, Page->Items(Page->Selected)->GetIcon());
            LockCursor();
        }
    }
    else
    {
        UnlockCursor();
        SetCursor(0, NULL);
    }
}
NW_HOOK_METHOD(0x1007ce40, NCInventoryWnd, SetEnchantMode)

// 0x1007cec0 - names and descriptions again from the item tables (the language changed), then the tool tips
void NCInventoryWnd::ReloadTexts()
{
    for (INT i = 0; i < EquipWnd->Items.Num(); i++)
    {
        NItemEntry* Item = EquipWnd->Items(i);
        if (!Item)
            continue;
        appStrcpy(Item->Name, GL2GameData.GetItemName(Item->ID));
        appStrcpy(Item->Description, GL2GameData.GetItemDescription(Item->ID));
        Item->FillToolTip(2, 0);
    }
    for (INT i = 0; i < ItemWnd[0]->Items.Num(); i++)
    {
        NItemEntry* Item = ItemWnd[0]->Items(i);
        if (!Item)
            continue;
        appStrcpy(Item->Name, GL2GameData.GetItemName(Item->ID));
        appStrcpy(Item->Description, GL2GameData.GetItemDescription(Item->ID));
        Item->FillToolTip(2, 0);
    }
    for (INT i = 0; i < ItemWnd[1]->Items.Num(); i++)
    {
        NItemEntry* Item = ItemWnd[1]->Items(i);
        if (!Item)
            continue;
        appStrcpy(Item->Name, GL2GameData.GetItemName(Item->ID));
        appStrcpy(Item->Description, GL2GameData.GetItemDescription(Item->ID));
        Item->FillToolTip(2, 0);
    }
}
NW_HOOK_METHOD(0x1007cec0, NCInventoryWnd, ReloadTexts)

// 0x1007d050 - the quest page is not searched
NItemEntry* NCInventoryWnd::FindItem(INT ObjectID)
{
    INT Index;
    if (EquipWnd->FindItem(ObjectID, &Index))
        return EquipWnd->Items(Index);
    if (ItemWnd[0]->FindItem(ObjectID, &Index))
        return ItemWnd[0]->Items(Index);
    return NULL;
}
NW_HOOK_METHOD(0x1007d050, NCInventoryWnd, FindItem)

// 0x100800b0
void NCInventoryWnd::ClearItems()
{
    EquipWnd->Clear();
    ItemWnd[0]->Clear();
    ItemWnd[1]->Clear();
}
NW_HOOK_METHOD(0x100800b0, NCInventoryWnd, ClearItems)

// 0x1007f5c0 - equipped items (NItemInfo::UnknownC8C) to the paperdoll, quest items (type 3) to the "Quest" page,
// the rest to the "Item" page
UBOOL NCInventoryWnd::AddItem(NItemInfo* Info)
{
    INT Page = Info->Param == 3;
    if (Info->UnknownC8C)
    {
        EquipWnd->AddItem(Info);
        return 1;
    }
    if (ItemWnd[Page])
        ItemWnd[Page]->AddItem(Info);
    return 1;
}
NW_HOOK_METHOD(0x1007f5c0, NCInventoryWnd, AddItem)

// 0x100800e0 - an item added (1), changed (2) or removed (3): an equipped item (NItemInfo::UnknownC8C) goes to the
// paperdoll, a quest item (type 3) to the "Quest" page, the rest to the "Item" page, where a new item shows first
// (MoveLastToFront); a changed item moves between the paperdoll and its page when it was put on or taken off. C2: the
// console calls it (C1 did it inline).
UBOOL NCInventoryWnd::UpdateItem(NItemInfo* Info, INT Change)
{
    INT Page = Info->Param == 3;
    if (Change == 1)
    {
        if (Info->UnknownC8C)
        {
            EquipWnd->AddItem(Info);
            return 1;
        }
    }
    else if (Change == 2)
    {
        if (Info->UnknownC8C)
        {
            INT ObjectID = Info->ObjectID;
            if (EquipWnd->HasItem(ObjectID))
            {
                EquipWnd->UpdateItem(Info);
                return 1;
            }
            ItemWnd[0]->RemoveItem(ObjectID);
            EquipWnd->AddItem(Info);
            return 1;
        }
        INT ObjectID = Info->ObjectID;
        NCInvenItemWnd* Wnd = ItemWnd[Page];
        if (Wnd->HasItem(ObjectID))
        {
            Wnd->UpdateItem(Info);
            return 1;
        }
        EquipWnd->RemoveItem(ObjectID);
    }
    else
    {
        if (Change == 3)
        {
            if (Info->UnknownC8C)
            {
                if (EquipWnd)
                    EquipWnd->RemoveItem(Info->ObjectID);
            }
            else
                ItemWnd[Page]->RemoveItem(Info->ObjectID);
        }
        return 1;
    }
    ItemWnd[Page]->AddItem(Info);
    if (Page == 0)
        MoveLastToFront();
    return 1;
}
NW_HOOK_METHOD(0x100800e0, NCInventoryWnd, UpdateItem)

// 0x1007fbf0 (vslot 36) - the title bar, the paperdoll (32-pixel cells), the two pages (6 x 4 icons of 32 pixels,
// 250 items at most) and the tabs that switch them, then the manufacture button
INT NCInventoryWnd::OnCreate()
{
    NCWnd::OnCreate();
    BackTex = FindTexture(TEXT("L2UI.InventoryWnd.InventoryBack"));
    CrystallizeTex = FindTexture(TEXT("L2UI.WindowIcon.Crystalize"));
    CrystallizeActiveTex = FindTexture(TEXT("L2UI.WindowIcon.Crystalize_active"));
    TrashTex = FindTexture(TEXT("L2UI.WindowIcon.DestroyItem"));
    TrashActiveTex = FindTexture(TEXT("L2UI.WindowIcon.DestroyItem_active"));
    WeightTex[0] = FindTexture(TEXT("L2UI.InventoryWnd.InventoryWT1"));
    WeightTex[1] = FindTexture(TEXT("L2UI.InventoryWnd.InventoryWT2"));
    WeightTex[2] = FindTexture(TEXT("L2UI.InventoryWnd.InventoryWT3"));
    WeightTex[3] = FindTexture(TEXT("L2UI.InventoryWnd.InventoryWT4"));
    AdenaTex = FindTexture(TEXT("L2UI.WindowIcon.WindowIconAdena"));
    WeightIconTex = FindTexture(TEXT("L2UI.WindowIcon.Weight"));

    FrameCtrl = new NCFrameCtrl(1);
    FrameCtrl->ID = NID_INVENTORY_FRAME;
    FrameCtrl->SetWindowRect(0, 0, 240, 21);
    CreateChild(FrameCtrl, NWS_VISIBLE);

    EquipWnd = new NCEquipItemWnd(32);
    EquipWnd->SetWindowRect(16, 29, 207, 102);
    CreateChild(EquipWnd, NWS_VISIBLE | 0x20);   // 0x20: meaning unknown

    ItemWnd[0] = new NCInvenItemWnd(6, 4, 250, 32);
    ItemWnd[0]->SetWindowRect(16, 166, 222, 137);
    CreateChild(ItemWnd[0], NWS_VISIBLE | 0x20);

    ItemWnd[1] = new NCInvenItemWnd(6, 4, 250, 32);
    ItemWnd[1]->SetWindowRect(16, 166, 222, 137);
    CreateChild(ItemWnd[1], NWS_VISIBLE | 0x20);

    TabCtrl = new NCTabControl();
    TabCtrl->ID = NID_INVENTORY_TAB;
    TabCtrl->SetWindowRect(15, 141, 138, 16);
    CreateChild(TabCtrl, NWS_VISIBLE);
    TabCtrl->AddTab(STR_Item, ItemWnd[0], 69, 16, TEXT("L2UI.NWindow.Tab2"), TEXT("L2UI.NWindow.Tab1"), 0, -1);
    TabCtrl->AddTab(STR_Quest, ItemWnd[1], 69, 16, TEXT("L2UI.NWindow.Tab2"), TEXT("L2UI.NWindow.Tab1"), 0, -1);

    ManufactureButton = new NCButton(TEXT("L2UI.WindowIcon.Manufacture"), TEXT("L2UI.WindowIcon.Manufacture_click"), 0);
    ManufactureButton->ID = NID_INVENTORY_MANUFACTURE;
    ManufactureButton->SetWindowRect(44, 315, 32, 32);
    CreateChild(ManufactureButton, NWS_VISIBLE);
    return 1;
}
NW_HOOK_METHOD(0x1007fbf0, NCInventoryWnd, OnCreate)

NW_CHECK_VTABLE(NCInventoryWnd, 0x10118928, 68)
