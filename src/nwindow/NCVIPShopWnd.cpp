/*=============================================================================
    NCVIPShopWnd.cpp - the VIP shop and its pages. Stock: 0x10084760 .. 0x100847b9 (list page),
    0x100847c0 .. 0x10086110 (item page: DrawItemInfo 0x10084840), 0x10086110 .. 0x10086cbf,
    0x10086ec0 .. 0x10087590 (needed items, shop), 0x10087590 .. 0x100881f8, and 0x10087e40
    (ShowItem). NCVIPShopItemListBox's functions sit in between (NCVIPShopItemListBox.cpp).
=============================================================================*/
#include "nwindow/NCVIPShopWnd.h"
#include "nwindow/NCButton.h"
#include "nwindow/NCFrameCtrl.h"
#include "nwindow/NCGDevice.h"
#include "nwindow/NCInventoryWnd.h"
#include "nwindow/NConsoleWnd.h"
#include "nwindow/NItemToolTip.h"

// sysstring
enum
{
    STR_Store = 136,
    STR_OK = 140,
    STR_Cancel = 141,
    STR_SkillList = 364,
    STR_List = 562,
    STR_ItemInformation = 564,
    STR_ItemsRequired = 565,
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

/*-----------------------------------------------------------------------------
    NVIPShopRecord
-----------------------------------------------------------------------------*/

// 0x10087590 - the row's constructor is inlined; the needed items' array is emptied
NVIPShopRecord::NVIPShopRecord()
{
    NeededItems.Empty();
}
NW_HOOK_CTOR(0x10087590, NVIPShopRecord)

/*-----------------------------------------------------------------------------
    NCVIPShopItemListWnd
-----------------------------------------------------------------------------*/

// 0x10086540 - the list box and the texture are left as allocated
NCVIPShopItemListWnd::NCVIPShopItemListWnd()
{
}
NW_HOOK_CTOR(0x10086540, NCVIPShopItemListWnd)

// 0x10086590
NW_HOOK_DTOR(0x10086590, NCVIPShopItemListWnd)

// 0x10084760 (vslot 25) - C2: no IsEnabled test; the list box reports the clicked product
INT NCVIPShopItemListWnd::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (NCWnd::WindowProc(Msg, wParam, lParam))
        return 1;
    if (Msg == NWM_COMMAND && wParam == NID_VIPSHOP_LIST)
        return OnSelect((INT)lParam) != 0;
    return 0;
}
NW_HOOK_METHOD(0x10084760, NCVIPShopItemListWnd, WindowProc)

// 0x100c11b0 - called, not inlined, by the stock code (a tail jump to the list box's Clear, vslot 70;
// the code is shared with the other pages that keep a list box at +0x98)
void NCVIPShopItemListWnd::Clear()
{
    ListBox->Clear();
}
NW_HOOK_METHOD(0x100c11b0, NCVIPShopItemListWnd, Clear)

// 0x100865e0 (vslot 37) - the base first, then the list box
INT NCVIPShopItemListWnd::OnDestroy()
{
    guard(NCVIPShopItemListWnd::OnDestroy);
    NCWnd::OnDestroy();
    DestroyChild(this, ListBox);
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x100865e0, NCVIPShopItemListWnd, OnDestroy)

// 0x10086690 (vslot 55) - the background and "List" (drawn when sysstring 364, "Skill list", is
// there: the test reads another line than the one drawn)
INT NCVIPShopItemListWnd::OnPaint(NCGDevice* Device)
{
    guard(NCVIPShopItemListWnd::OnPaint);
    Device->BeginPaint(ScreenX, ScreenY);
    Device->SetClip(ScreenX, ScreenY, Width, Height);
    if (BackTex)
        Device->DrawTexture(0, 0, 240, 344, 0, 0, 240, 344, BackTex);
    if (*SysString(STR_SkillList))
        Device->DrawText(20, 5, 0xFFFFFFFF, *SysString(STR_List), 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0,
                         0, 0);
    Device->EndPaint();
    Device->ReleaseClip();
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x10086690, NCVIPShopItemListWnd, OnPaint)

// 0x10086eb0
void NCVIPShopItemListWnd::AddEntry(const NVIPShopEntry* Src)
{
    ListBox->AddEntry(Src);
}
NW_HOOK_METHOD(0x10086eb0, NCVIPShopItemListWnd, AddEntry)

// 0x10087720 (vslot 36) - 100 rows kept, 8 seen
INT NCVIPShopItemListWnd::OnCreate()
{
    NCWnd::OnCreate();
    BackTex = FindTexture(TEXT("L2UI.VipWnd.VipWndBack1"));
    ListBox = new NCVIPShopItemListBox(100, 8, 16);
    ListBox->ID = NID_VIPSHOP_LIST;
    ListBox->SetWindowRect(20, 24, 215, 284);
    CreateChild(ListBox, NWS_VISIBLE);
    return 1;
}
NW_HOOK_METHOD(0x10087720, NCVIPShopItemListWnd, OnCreate)

// 0x100881d0 (vslot 67; C1 slot 62) - a product clicked: the shop shows its item page
INT NCVIPShopItemListWnd::OnSelect(INT Index)
{
    NCVIPShopWnd* Shop = (NCVIPShopWnd*)Entry->Parent->Wnd;
    Shop->Selected = Index;
    Shop->ShowItem();
    return 1;
}
NW_HOOK_METHOD(0x100881d0, NCVIPShopItemListWnd, OnSelect)

NW_CHECK_VTABLE(NCVIPShopItemListWnd, 0x10119b98, 68)

/*-----------------------------------------------------------------------------
    NCVIPShopItemInfoWnd
-----------------------------------------------------------------------------*/

// 0x100878b0 - the item entry is constructed by NCVIPShopItemInfoWnd's member
NCVIPShopItemInfoWnd::NCVIPShopItemInfoWnd()
{
}
NW_HOOK_CTOR(0x100878b0, NCVIPShopItemInfoWnd)

// 0x10087930 - ~NItemEntry inlined (the tool tip data at +0x1A80)
NW_HOOK_DTOR(0x10087930, NCVIPShopItemInfoWnd)

// stock body shared with NCItemWnd's (0x10019810, hooked there): the base's answer as a plain 0 / 1
INT NCVIPShopItemInfoWnd::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    return NCScrollWnd::WindowProc(Msg, wParam, lParam) != 0;
}

// stock body shared with NCItemWnd's (0x1002ce20)
INT NCVIPShopItemInfoWnd::OnCreate()
{
    NCScrollWnd::OnCreate();
    return 1;
}

// 0x100847c0 (vslot 37)
INT NCVIPShopItemInfoWnd::OnDestroy()
{
    guard(NCVIPShopItemInfoWnd::OnDestroy);
    NCScrollWnd::OnDestroy();
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x100847c0, NCVIPShopItemInfoWnd, OnDestroy)

// 0x10086110 - a tail jump to NItemEntry::Reset
void NCVIPShopItemInfoWnd::Clear()
{
    Item.Reset();
}
NW_HOOK_METHOD(0x10086110, NCVIPShopItemInfoWnd, Clear)

// 0x10086920 (vslot 55) - the item's texts, scrolled; their height sizes the scroll range
INT NCVIPShopItemInfoWnd::OnPaint(NCGDevice* Device)
{
    Device->BeginPaint(ScreenX, ScreenY);
    Device->SetClip(ScreenX, ScreenY, Width, Height);
    Device->PushOrigin(ScrollX, ScrollY);
    SetContentHeight(DrawItemInfo(Device, &Item, 16, 15));
    Device->PopOrigin();
    Device->EndPaint();
    Device->ReleaseClip();
    return 1;
}
NW_HOOK_METHOD(0x10086920, NCVIPShopItemInfoWnd, OnPaint)

// 0x10087080 - Src copied, then completed from the item table (nothing more when the item is
// not in it): name, icon, description, and by the item's class the weapon's, the armor's or the
// etc item's values; the type and body part stay Src's
void NCVIPShopItemInfoWnd::SetItem(NItemEntry* Src)
{
    Item.Copy(Src);
    NViewItemData* Data = NItemTable::Find(&Item.ID);
    if (Item.ID < 0 || !Data)
        return;
    INT Class = Data->Class;
    appStrcpy(Item.Name, GL2GameData.GetItemName(Item.ID));
    appStrcpy(Item.IconName, *Data->IconName);
    appStrcpy(Item.Description, GL2GameData.GetItemDescription(Item.ID));
    Item.Type = Src->Type;
    Item.BodyPart = Src->BodyPart;
    if (Class == ITEMCLASS_Weapon)
    {
        Item.Weight = Data->Weight;
        Item.UnknownC2C = Data->Unknown48;
        Item.UnknownC30 = Data->WeaponType;
        Item.Unknown1C80 = Data->PAtk;
        Item.Unknown1C84 = Data->MAtk;
        Item.Unknown1C90 = Data->ShieldPDef;
        Item.Unknown1C94 = Data->ShieldRate;
        Item.Unknown1C58 = Data->Unknown40;
        Item.CrystalGrade = Data->CrystalGrade;
        Item.Unknown1C60 = Data->Unknown90;
        Item.Unknown1C64 = Data->UnknownA4;
        Item.Unknown1C68 = Data->UnknownA8;
        Item.Unknown1C6C = Data->AttackSpeed;
        Item.Unknown1C70 = Data->UnknownBC;
        Item.Unknown1C78 = Data->Evasion;
        Item.Unknown1CA0 = Data->SoulShots;
        Item.Unknown1CA4 = Data->SpiritShots;
    }
    else if (Class == ITEMCLASS_Armor)
    {
        Item.Weight = Data->Weight;
        Item.UnknownC2C = Data->Unknown48;
        Item.Unknown1C58 = Data->Unknown40;
        Item.CrystalGrade = Data->ArmorCrystalGrade;
        Item.SubType = Data->ArmorType;
        Item.Unknown1C78 = Data->ArmorEvasion;
        Item.Unknown1C88 = Data->PDef;
        Item.Unknown1C8C = Data->MDef;
        Item.Unknown1C9C = Data->MPBonus;
    }
    else if (Class == ITEMCLASS_Etc)
    {
        Item.Weight = Data->Weight;
        Item.UnknownC2C = Data->Unknown48;
        Item.Unknown1C58 = Data->Unknown40;
        Item.bStackable = Data->bStackable;
        if (Data->EtcType == 2)
            Item.Unknown1C98 = 1;
    }
}
NW_HOOK_METHOD(0x10087080, NCVIPShopItemInfoWnd, SetItem)

// 0x10084840 - the icon, then the item tool tip's lines (NItemToolTip.h) laid out for the
// page: the name (a stack's count without separators) and the slot beside the icon, 14 and 20
// pixels apart, the values under the icon 13 apart, the weight, 20 pixels, and the description
// wrapped at 200 pixels (C1: 168). A pet collar's level is not shown; the weight of type 5 items
// stays beside the icon. C2: a weapon with an enchant also shows "-" and its additional name
// after the name; the grade follows. Returns the Y under the description, whose height is taken
// from GetTextExtent with the same width.
INT NCVIPShopItemInfoWnd::DrawItemInfo(NCGDevice* Device, NItemEntry* Entry, INT X, INT Y)
{
    if (!Entry)
        return Y;
    Device->DrawTexture(X, Y - 1, 32, 32, 0, 0, 32, 32, Entry->Icon);
    INT TextX = X + 36;
    Y += 2;
    FString Desc;
    GConsole->ConvertNewLines((TCHAR*)Entry->GetDescription(), Desc);
    FString Slot;
    GConsole->GetItemSlotText(Entry->Type, Entry->BodyPart, Slot, Entry->SubType);
    TCHAR Text[2048];
    SIZE Size;

    // the name: after "+enchant " for weapons, armor, accessories and quest items; the grade
    // after it
    if (Entry->Enchant > 0
        && (Entry->Type == 0 || Entry->Type == 1 || Entry->Type == 3 || Entry->Type == 2))
    {
        appSprintf(Text, TEXT("+%d "), Entry->Enchant);
        ItemTip::DrawString(Device, TextX, Y, 0xFFB09B79, Text);
        NCGDevice::GetTextExtent(&Size, Text, 0, 0, 0, 0, 0);
        if (Entry->IsStackable())
            appSprintf(Text, TEXT("%s(%d)"), Entry->GetName(), Entry->Count);
        else
            appSprintf(Text, TEXT("%s"), Entry->GetName());
        TextX += Size.cx;
        ItemTip::DrawString(Device, TextX, Y, 0xFFFFFFFF, Text);
        if (Entry->Type == 0 && GConsole->HasItemAdditionalName(Entry->ID))
        {
            // 0x10057730: the item has an additional name ("-" gray, the name gold)
            NCGDevice::GetTextExtent(&Size, Text, 0, 0, 0, 0, 0);
            appSprintf(Text, TEXT("-"));
            TextX += Size.cx;
            ItemTip::DrawString(Device, TextX, Y, 0xFFA0A0A0, TEXT("-"));
            NCGDevice::GetTextExtent(&Size, Text, 0, 0, 0, 0, 0);
            appSprintf(Text, TEXT("%s"), Entry->AdditionalName);
            TextX += Size.cx;
            ItemTip::DrawString(Device, TextX, Y, 0xFFFFD969, Entry->AdditionalName);
        }
        if (ItemTip::GetGradeText(Entry->CrystalGrade))
        {
            NCGDevice::GetTextExtent(&Size, Text, 0, 0, 0, 0, 0);
            appSprintf(Text, TEXT("`%s`"), ItemTip::GetGradeText(Entry->CrystalGrade));
            Device->DrawText(Size.cx + 2 + TextX, Y, 0xFFFFFFFF, Text, 0, 0, 0, L2FONT_Normal, 1, 0,
                             0, 0, 0, 0);
        }
        TextX = X;
    }
    else
    {
        if (Entry->IsStackable())
            appSprintf(Text, TEXT("%s(%d)"), Entry->GetName(), Entry->Count);
        else
            appSprintf(Text, TEXT("%s"), Entry->GetName());
        ItemTip::DrawString(Device, TextX, Y, 0xFFFFFFFF, Text);
        if (ItemTip::GetGradeText(Entry->CrystalGrade))
        {
            NCGDevice::GetTextExtent(&Size, Text, 0, 0, 0, 0, 0);
            appSprintf(Text, TEXT("`%s`"), ItemTip::GetGradeText(Entry->CrystalGrade));
            Device->DrawText(Size.cx + 2 + TextX, Y, 0xFFFFFFFF, Text, 0, 0, 0, L2FONT_Normal, 1, 0,
                             0, 0, 0, 0);
        }
    }
    Y += 14;

    // the slot beside the icon, then the values of the type from X (quest items and the rest:
    // none)
    switch (Entry->Type)
    {
    case 0:  // weapons: type / slot, attacks, speed, shots
        appSprintf(Text, TEXT("%s / %s"), ItemTip::GetWeaponTypeName(Entry->UnknownC30), *Slot);
        ItemTip::DrawString(Device, TextX, Y, 0xFFB09B79, Text);
        TextX = X;
        Y += 20;
        ItemTip::DrawLabel(Device, Text, Size, X, Y, ItemTip::STR_PAtk);
        appSprintf(Text, TEXT("%d"), ItemTip::GetEnchantedAttack(Entry->CrystalGrade, Entry->Enchant,
                                                                 Entry->Unknown1C80));
        ItemTip::DrawValue(Device, Text, Size, X, Y);
        Y += 13;
        ItemTip::DrawLabel(Device, Text, Size, X, Y, ItemTip::STR_MAtk);
        appSprintf(Text, TEXT("%d"), ItemTip::GetEnchantedAttack(Entry->CrystalGrade, Entry->Enchant,
                                                                 Entry->Unknown1C84));
        ItemTip::DrawValue(Device, Text, Size, X, Y);
        Y += 13;
        ItemTip::DrawLabel(Device, Text, Size, X, Y, ItemTip::STR_AtkSpd);
        appSprintf(Text, TEXT("%s"), ItemTip::GetAttackSpeedText(Entry->Unknown1C6C));
        ItemTip::DrawValue(Device, Text, Size, X, Y);
        Y += 13;
        ItemTip::DrawLabel(Device, Text, Size, X, Y, ItemTip::STR_SoulShots);
        appSprintf(Text, TEXT("X %d"), Entry->Unknown1CA0);
        ItemTip::DrawValue(Device, Text, Size, X, Y);
        Y += 13;
        ItemTip::DrawLabel(Device, Text, Size, X, Y, ItemTip::STR_SpiritShots);
        appSprintf(Text, TEXT("X %d"), Entry->Unknown1CA4);
        ItemTip::DrawValue(Device, Text, Size, X, Y);
        Y += 13;
        break;
    case 1:  // armor: shields (body part 0x100 / 0x80) their defense, rate and evasion, robes
             // the MP bonus and P.Def., the rest P.Def.
        appSprintf(Text, TEXT("%s"), *Slot);
        ItemTip::DrawString(Device, TextX, Y, 0xFFB09B79, Text);
        Y += 20;
        TextX = X;
        if ((Entry->BodyPart & 0x100) || (Entry->BodyPart & 0x80))
        {
            ItemTip::DrawLabel(Device, Text, Size, X, Y, ItemTip::STR_PDef);
            appSprintf(Text, TEXT("%d"), ItemTip::GetEnchantedValue(Entry->CrystalGrade,
                                                                    Entry->Enchant, Entry->Unknown1C90));
            ItemTip::DrawValue(Device, Text, Size, X, Y);
            Y += 13;
            ItemTip::DrawLabel(Device, Text, Size, X, Y, ItemTip::STR_ShieldRate);
            appSprintf(Text, TEXT("%d"), Entry->Unknown1C94);
            ItemTip::DrawValue(Device, Text, Size, X, Y);
            Y += 13;
            ItemTip::DrawLabel(Device, Text, Size, X, Y, ItemTip::STR_Evasion);
            appSprintf(Text, TEXT("%d"), Entry->Unknown1C78);
            ItemTip::DrawValue(Device, Text, Size, X, Y);
            Y += 13;
        }
        else if (ItemTip::IsMagicArmor(Entry->ID))
        {
            ItemTip::DrawLabel(Device, Text, Size, X, Y, ItemTip::STR_MPBonus);
            appSprintf(Text, TEXT("%d"), Entry->Unknown1C9C);
            ItemTip::DrawValue(Device, Text, Size, X, Y);
            Y += 13;
            ItemTip::DrawLabel(Device, Text, Size, X, Y, ItemTip::STR_PDef);
            appSprintf(Text, TEXT("%d"), ItemTip::GetEnchantedValue(Entry->CrystalGrade,
                                                                    Entry->Enchant, Entry->Unknown1C88));
            ItemTip::DrawValue(Device, Text, Size, X, Y);
            Y += 13;
        }
        else
        {
            ItemTip::DrawLabel(Device, Text, Size, X, Y, ItemTip::STR_PDef);
            appSprintf(Text, TEXT("%d"), ItemTip::GetEnchantedValue(Entry->CrystalGrade,
                                                                    Entry->Enchant, Entry->Unknown1C88));
            ItemTip::DrawValue(Device, Text, Size, X, Y);
            Y += 13;
        }
        break;
    case 2:  // accessories: M.Def.
        appSprintf(Text, TEXT("%s"), *Slot);
        ItemTip::DrawString(Device, TextX, Y, 0xFFB09B79, Text);
        TextX = X;
        Y += 20;
        ItemTip::DrawLabel(Device, Text, Size, X, Y, ItemTip::STR_MDef);
        appSprintf(Text, TEXT("%d"), ItemTip::GetEnchantedValue(Entry->CrystalGrade, Entry->Enchant,
                                                                Entry->Unknown1C8C));
        ItemTip::DrawValue(Device, Text, Size, X, Y);
        Y += 13;
        break;
    }

    // the weight (quest items: the slot beside the icon; type 4: nothing), then the description
    switch (Entry->Type)
    {
    case 0:
    case 1:
    case 2:
    case 5:
        ItemTip::DrawLabel(Device, Text, Size, TextX, Y, ItemTip::STR_Weight);
        appSprintf(Text, TEXT("%d"), Entry->Weight);
        ItemTip::DrawValue(Device, Text, Size, TextX, Y);
        break;
    case 3:
        appSprintf(Text, TEXT("%s"), *Slot);
        ItemTip::DrawString(Device, TextX, Y, 0xFFB09B79, Text);
        break;
    }
    Y += 20;
    appSprintf(Text, TEXT("%s"), *Desc);
    Device->DrawText(X, Y, 0xFFB2BECF, Text, 200, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
    NCGDevice::GetTextExtent(&Size, Text, 200, 0, 0, 0, 0);
    Y += Size.cy;
    return Y;
}
NW_HOOK_METHOD(0x10084840, NCVIPShopItemInfoWnd, DrawItemInfo)

NW_CHECK_VTABLE(NCVIPShopItemInfoWnd, 0x1011a058, 70)

/*-----------------------------------------------------------------------------
    NCVIPShopNeededItemWnd
-----------------------------------------------------------------------------*/

// 0x100877f0
NCVIPShopNeededItemWnd::NCVIPShopNeededItemWnd()
{
}
NW_HOOK_CTOR(0x100877f0, NCVIPShopNeededItemWnd)

// 0x10087850 - Items' destructor is TArray<T*>'s (0x1006e700)
NW_HOOK_DTOR(0x10087850, NCVIPShopNeededItemWnd)

// stock body shared with NCItemWnd's (0x10019810, hooked there): the base's answer as a plain 0 / 1
INT NCVIPShopNeededItemWnd::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    return NCScrollWnd::WindowProc(Msg, wParam, lParam) != 0;
}

// stock body shared with NCItemWnd's (0x1002ce20)
INT NCVIPShopNeededItemWnd::OnCreate()
{
    NCScrollWnd::OnCreate();
    return 1;
}

// 0x100867b0 (vslot 55) - a row of 34 pixels per item: the icon, the name and "X count"
INT NCVIPShopNeededItemWnd::OnPaint(NCGDevice* Device)
{
    Device->BeginPaint(ScreenX, ScreenY);
    Device->SetClip(ScreenX, ScreenY, Width, Height);
    if (Items.Num() > 0)
    {
        Device->PushOrigin(ScrollX, ScrollY);
        INT i;
        INT Y = 7;
        for (i = 0; i < Items.Num(); i++, Y += 34)
        {
            Device->DrawTexture(15, Y - 2, 32, 32, 0, 0, 32, 32, Items(i)->Icon);
            Device->DrawText(49, Y, 0xFFFFFFFF, Items(i)->Name, 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0,
                             0);
            TCHAR Count[2048];
            appSprintf(Count, TEXT("X %d"), Items(i)->Count);
            Device->DrawText(49, Y + 14, 0xFFFFFFFF, Count, 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
        }
        SetContentHeight(i * 34 + 5);
        Device->PopOrigin();
    }
    Device->EndPaint();
    Device->ReleaseClip();
    return 1;
}
NW_HOOK_METHOD(0x100867b0, NCVIPShopNeededItemWnd, OnPaint)

// 0x10086ec0 (vslot 37) - the loop runs while the array is not empty, which it never changes:
// with items left it frees on past the end of the array (Clear, which the shop calls before
// each page, leaves it empty)
INT NCVIPShopNeededItemWnd::OnDestroy()
{
    guard(NCVIPShopNeededItemWnd::OnDestroy);
    for (INT i = 0; Items.Num(); i++)
    {
        if (Items(i))
            appFree(Items(i));
    }
    Items.Empty();
    NCScrollWnd::OnDestroy();
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x10086ec0, NCVIPShopNeededItemWnd, OnDestroy)

// 0x10086f90 - keeps a copy (the icon's name is not copied)
void NCVIPShopNeededItemWnd::AddItem(const NVIPNeededItem* Src)
{
    NVIPNeededItem* Item = new NVIPNeededItem;
    Item->ItemID = Src->ItemID;
    appStrcpy(Item->Name, Src->Name);
    Item->Icon = Src->Icon;
    Item->Count = Src->Count;
    Items.AddItem(Item);
}
NW_HOOK_METHOD(0x10086f90, NCVIPShopNeededItemWnd, AddItem)

// 0x10087020
void NCVIPShopNeededItemWnd::Clear()
{
    for (INT i = 0; i < Items.Num(); i++)
    {
        if (Items(i))
            appFree(Items(i));
    }
    Items.Empty();
    UnknownDC = 0;
    UnknownE0 = 0;
}
NW_HOOK_METHOD(0x10087020, NCVIPShopNeededItemWnd, Clear)

NW_CHECK_VTABLE(NCVIPShopNeededItemWnd, 0x10119f40, 70)

/*-----------------------------------------------------------------------------
    NCVIPShopWnd
-----------------------------------------------------------------------------*/

// 0x10087990 - only Records is set up; the rest is left as allocated
NCVIPShopWnd::NCVIPShopWnd()
{
}
NW_HOOK_CTOR(0x10087990, NCVIPShopWnd)

// 0x100879f0 - Records' destructor is TArray<T*>'s (0x1006e700): the records are not freed
NW_HOOK_DTOR(0x100879f0, NCVIPShopWnd)

// 0x10086120 (vslot 25) - C2: no IsEnabled test
INT NCVIPShopWnd::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (NCWnd::WindowProc(Msg, wParam, lParam))
        return 1;
    if (Msg == NWM_COMMAND)
    {
        if (wParam == NID_VIPSHOP_OK)
            return OnOK() != 0;
        if (wParam == NID_VIPSHOP_CANCEL)
            return OnCancel() != 0;
    }
    return 0;
}
NW_HOOK_METHOD(0x10086120, NCVIPShopWnd, WindowProc)

// 0x100869c0 (vslot 37) - the base first, then the pages and the buttons
INT NCVIPShopWnd::OnDestroy()
{
    guard(NCVIPShopWnd::OnDestroy);
    NCWnd::OnDestroy();
    DestroyChild(this, ListWnd);
    DestroyChild(this, InfoWnd);
    DestroyChild(this, NeededWnd);
    DestroyChild(this, OKButton);
    DestroyChild(this, CancelButton);
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x100869c0, NCVIPShopWnd, OnDestroy)

// 0x10086b10 (vslot 55) - the item page's background (the window's size, below the title bar)
// and its two headings; the list page draws itself
INT NCVIPShopWnd::OnPaint(NCGDevice* Device)
{
    Device->BeginPaint(ScreenX, ScreenY);
    Device->SetClip(ScreenX, ScreenY, Width, Height);
    if (Page == VIPSHOPPAGE_Item)
    {
        Device->DrawTexture(0, 21, Width, Height, 0, 0, Width, Height, BackTex);
        Device->DrawText(15, 25, 0xFFFFFFFF, *SysString(STR_ItemInformation), 0, 0, 0,
                         L2FONT_Normal, 0, 0, 0, 0, 0, 0);
        Device->DrawText(15, 179, 0xFFFFFFFF, *SysString(STR_ItemsRequired), 0, 0, 0,
                         L2FONT_Normal, 0, 0, 0, 0, 0, 0);
    }
    Device->EndPaint();
    Device->ReleaseClip();
    return 1;
}
NW_HOOK_METHOD(0x10086b10, NCVIPShopWnd, OnPaint)

// 0x10086c10 (vslot 67) - buys the product shown
INT NCVIPShopWnd::OnOK()
{
    L2ParamStack Params(10);
    Params.PushBack((void*)(INT_PTR)ListID);
    Params.PushBack((void*)(INT_PTR)Records(Selected)->ProductID);
    GConsole->RequestBuyVIPItem(Params);  // the list id and the product id
    return 1;
}
NW_HOOK_METHOD(0x10086c10, NCVIPShopWnd, OnOK)

// 0x100872d0 - the list page: a row per product
void NCVIPShopWnd::ShowList()
{
    ListWnd->Clear();
    InfoWnd->Clear();
    NeededWnd->Clear();
    for (INT i = 0; i < Records.Num(); i++)
    {
        NVIPShopEntry Row = *Records(i);
        ListWnd->AddEntry(&Row);
    }
    InfoWnd->HideWindow();
    NeededWnd->HideWindow();
    ListWnd->ShowWindow();
    OKButton->HideWindow();
    CancelButton->HideWindow();
    Page = VIPSHOPPAGE_List;
}
NW_HOOK_METHOD(0x100872d0, NCVIPShopWnd, ShowList)

// 0x10087e40 (C2: a function of its own, the list page's OnSelect calls it) - the item page of
// Records(Selected); with no product selected the pages are only emptied
void NCVIPShopWnd::ShowItem()
{
    ListWnd->Clear();
    InfoWnd->Clear();
    NeededWnd->Clear();
    if (Selected < 0)
        return;
    for (INT i = 0; i < Records(Selected)->NeededItems.Num(); i++)
    {
        NVIPNeededItem Needed;
        appStrcpy(Needed.Name, Records(Selected)->NeededItems(i)->Name);
        Needed.ItemID = Records(Selected)->NeededItems(i)->ItemID;
        Needed.Count = Records(Selected)->NeededItems(i)->Count;
        Needed.Icon = Records(Selected)->NeededItems(i)->Icon;
        NeededWnd->AddItem(&Needed);
    }
    NItemEntry Item;
    Item.ID = Records(Selected)->ItemID;
    Item.Type = Records(Selected)->Type;
    Item.BodyPart = Records(Selected)->BodyPart;
    Item.Icon = Records(Selected)->Icon;
    Item.Count = Records(Selected)->Count;
    InfoWnd->SetItem(&Item);
    ListWnd->HideWindow();
    InfoWnd->ShowWindow();
    NeededWnd->ShowWindow();
    OKButton->ShowWindow();
    CancelButton->ShowWindow();
    Page = VIPSHOPPAGE_Item;
}
NW_HOOK_METHOD(0x10087e40, NCVIPShopWnd, ShowItem)

// 0x100873a0 - everything emptied; the records are dropped, not freed
void NCVIPShopWnd::Reset()
{
    Selected = -1;
    ListID = 0;
    Records.Empty();
    ListWnd->Clear();
    InfoWnd->Clear();
    NeededWnd->Clear();
}
NW_HOOK_METHOD(0x100873a0, NCVIPShopWnd, Reset)

// 0x100873f0 - to every product with that id; the icon found by its name
void NCVIPShopWnd::AddNeededItem(INT ProductID, const NVIPNeededItem* Src)
{
    for (INT i = 0; i < Records.Num(); i++)
    {
        NVIPShopRecord* Record = Records(i);
        if (Record->ProductID == ProductID)
        {
            NVIPNeededItem* Item = new NVIPNeededItem;
            Item->ItemID = Src->ItemID;
            appStrcpy(Item->Name, Src->Name);
            Item->Count = Src->Count;
            Item->Icon = FindTexture(Src->IconName);
            Record->NeededItems.AddItem(Item);
        }
    }
}
NW_HOOK_METHOD(0x100873f0, NCVIPShopWnd, AddNeededItem)

// 0x100874d0 (vslot 68) - back to the list
INT NCVIPShopWnd::OnCancel()
{
    ShowList();
    return 1;
}
NW_HOOK_METHOD(0x100874d0, NCVIPShopWnd, OnCancel)

// 0x100874e0
void NCVIPShopWnd::Open(INT InListID)
{
    Reset();
    ListID = InListID;
}
NW_HOOK_METHOD(0x100874e0, NCVIPShopWnd, Open)

// 0x10087a50 (vslot 36) - the title bar ("Store"), the three pages over one another and the
// two buttons
INT NCVIPShopWnd::OnCreate()
{
    FrameCtrl = new NCFrameCtrl(1);
    FrameID = NID_VIPSHOP_FRAME;
    FrameCtrl->ID = NID_VIPSHOP_FRAME;
    FrameCtrl->SetWindowRect(0, 0, 240, 21);
    CreateChild(FrameCtrl, NWS_VISIBLE);
    FrameCtrl->SetTitleText(*SysString(STR_Store));

    ListWnd = new NCVIPShopItemListWnd;
    ListWnd->SetWindowRect(0, 21, 240, 344);
    CreateChild(ListWnd, NWS_VISIBLE);

    InfoWnd = new NCVIPShopItemInfoWnd;
    InfoWnd->SetWindowRect(0, 40, 238, 136);
    CreateChild(InfoWnd, NWS_VISIBLE);

    NeededWnd = new NCVIPShopNeededItemWnd;
    NeededWnd->SetWindowRect(0, 193, 238, 144);
    CreateChild(NeededWnd, NWS_VISIBLE);

    OKButton = new NCButton(TEXT("L2UI.control.DefaultButton"), TEXT("L2UI.control.DefaultButton_click"), 0);
    OKButton->ID = NID_VIPSHOP_OK;
    OKButton->SetWindowRect(85, 341, 67, 19);
    CreateChild(OKButton, NWS_VISIBLE);
    OKButton->SetTextID(STR_OK);

    CancelButton = new NCButton(TEXT("L2UI.control.DefaultButton"), TEXT("L2UI.control.DefaultButton_click"), 0);
    CancelButton->ID = NID_VIPSHOP_CANCEL;
    CancelButton->SetWindowRect(160, 341, 67, 19);
    CreateChild(CancelButton, NWS_VISIBLE);
    CancelButton->SetTextID(STR_Cancel);

    BackTex = FindTexture(TEXT("L2UI.VipWnd.VipWndBack2"));
    return 1;
}
NW_HOOK_METHOD(0x10087a50, NCVIPShopWnd, OnCreate)

// 0x10088090 (vslot 44) - hidden, the window is emptied
void NCVIPShopWnd::HideWindow()
{
    NCWnd::HideWindow();
    Reset();
}
NW_HOOK_METHOD(0x10088090, NCVIPShopWnd, HideWindow)

// 0x100880a0 - keeps a copy; the icon found by its name
void NCVIPShopWnd::AddRecord(const NVIPShopEntry* Src)
{
    NVIPShopRecord* Record = new NVIPShopRecord;
    Record->ProductID = Src->ProductID;
    Record->Type = Src->Type;
    Record->ItemID = Src->ItemID;
    appStrcpy(Record->Name, Src->Name);
    Record->Icon = FindTexture(Src->IconName);
    appStrcpy(Record->Desc, Src->Desc);
    Record->Count = Src->Count;
    Record->BodyPart = Src->BodyPart;
    Records.AddItem(Record);
}
NW_HOOK_METHOD(0x100880a0, NCVIPShopWnd, AddRecord)

NW_CHECK_VTABLE(NCVIPShopWnd, 0x1011a170, 69)
