/*=============================================================================
    NConsoleWndItems.cpp - what the engine tells the console about items (UGameEngine::On*
    handlers, one slot each): the inventory, the trade, the NPC shops, the warehouse, the
    freight, the private stores, the VIP shop, the quest items, the enchant scrolls. Each item
    the server lists becomes an NItemInfo for its window: the texts and the icon from the game
    data, the values of the item from ItemInfo and those of its kind (weapon, armor or etc
    item) from the item table. Every list takes its own part of those.
=============================================================================*/
#include "nwindow/NConsoleWnd.h"
#include "nwindow/NCConsole.h"
#include "nwindow/NDialogInfo.h"
#include "nwindow/NCItemWnd.h"
#include "nwindow/NCInventoryWnd.h"
#include "nwindow/NCTradeWnd.h"
#include "nwindow/NCShopWnd.h"
#include "nwindow/NCStoreWnd.h"
#include "nwindow/NCDeliverWnd.h"
#include "nwindow/NCVIPShopWnd.h"
#include "nwindow/NCPrivateShopWnd.h"
#include "nwindow/NCPrivateBuyWnd.h"
#include "nwindow/NCPetWnd.h"
#include "nwindow/NCMainWnd.h"
#include "nwindow/NCQuestWnd.h"
#include "nwindow/NCGMWnd.h"
#include "nwindow/NCNPCHtmlViewer.h"
#include "nwindow/NCTrainWnd.h"
#include "nwindow/NCManorShopWnd.h"

// system messages (systemmsg ids)
enum
{
    MSG_AskTrade = 100,                // "$s1 has requested a trade. Do you wish to continue?"
    MSG_WarehouseFailed = 307,         // "You have failed to trade with the warehouse."
};

// the dialog boxes opened here (NDialogInfo::ID; NCDialogBox sends the answer)
enum { DIALOG_TRADE_REQUEST = 7 };

// the texts of the item and its icon's name
static FORCEINLINE void SetItemTexts(NItemInfo& Info, INT ItemID, NViewItemData* Data)
{
    appStrcpy(Info.Name, GL2GameData.GetItemName(ItemID));
    appStrcpy(Info.IconName, *Data->IconName);
    appStrcpy(Info.Description, GL2GameData.GetItemDescription(ItemID));
}

// the values of a weapon, an armor or an etc item from the item table (the NItemInfo fields
// the item windows copy to NItemEntry's P.Atk, M.Atk, P.Def, M.Def, the shield's values, the
// crystal grade, the attack speed, the armor type, the MP bonus and the shots used)
static FORCEINLINE void SetWeaponValues(NItemInfo& Info, NViewItemData* Data)
{
    Info.UnknownC28 = Data->Weight;
    Info.UnknownC2C = Data->Unknown48;
    Info.UnknownC30 = Data->WeaponType;
    Info.UnknownC34[0] = Data->PAtk;
    Info.UnknownC34[1] = Data->MAtk;
    Info.UnknownC34[4] = Data->ShieldPDef;
    Info.UnknownC34[5] = Data->ShieldRate;
    Info.UnknownC34[6] = Data->Unknown40;
    Info.UnknownC34[7] = Data->CrystalGrade;
    Info.UnknownC34[8] = Data->Unknown90;
    Info.UnknownC34[9] = Data->UnknownA4;
    Info.UnknownC34[10] = Data->UnknownA8;
    Info.UnknownC34[11] = Data->AttackSpeed;
    Info.UnknownC34[12] = Data->UnknownBC;
    Info.UnknownC6C = Data->Evasion;
    Info.UnknownC7C = Data->SoulShots;
    Info.UnknownC80 = Data->SpiritShots;
}
static FORCEINLINE void SetArmorValues(NItemInfo& Info, NViewItemData* Data)
{
    Info.UnknownC28 = Data->Weight;
    Info.UnknownC2C = Data->Unknown48;
    Info.UnknownC34[6] = Data->Unknown40;
    Info.UnknownC34[7] = Data->ArmorCrystalGrade;
    Info.SubType = Data->ArmorType;
    Info.UnknownC6C = Data->ArmorEvasion;
    Info.UnknownC34[2] = Data->PDef;
    Info.UnknownC34[3] = Data->MDef;
    Info.UnknownC78 = Data->MPBonus;
}
static FORCEINLINE void SetEtcValues(NItemInfo& Info, NViewItemData* Data)
{
    Info.UnknownC28 = Data->Weight;
    Info.UnknownC2C = Data->Unknown48;
    Info.UnknownC34[6] = Data->Unknown40;
    Info.bStackable = Data->bStackable;
}
static FORCEINLINE void SetClassValues(NItemInfo& Info, INT Class, NViewItemData* Data)
{
    if (Class == ITEMCLASS_Weapon)
        SetWeaponValues(Info, Data);
    else if (Class == ITEMCLASS_Armor)
        SetArmorValues(Info, Data);
    else if (Class == ITEMCLASS_Etc)
        SetEtcValues(Info, Data);
}

// The weapon's values without Unknown40 (the inventory and pet updates)
static FORCEINLINE void SetUpdatedWeaponValues(NItemInfo& Info, NViewItemData* Data)
{
    Info.UnknownC28 = Data->Weight;
    Info.UnknownC2C = Data->Unknown48;
    Info.UnknownC30 = Data->WeaponType;
    Info.UnknownC34[0] = Data->PAtk;
    Info.UnknownC34[1] = Data->MAtk;
    Info.UnknownC34[4] = Data->ShieldPDef;
    Info.UnknownC34[5] = Data->ShieldRate;
    Info.UnknownC34[7] = Data->CrystalGrade;
    Info.UnknownC34[8] = Data->Unknown90;
    Info.UnknownC34[9] = Data->UnknownA4;
    Info.UnknownC34[10] = Data->UnknownA8;
    Info.UnknownC34[11] = Data->AttackSpeed;
    Info.UnknownC34[12] = Data->UnknownBC;
    Info.UnknownC6C = Data->Evasion;
    Info.UnknownC7C = Data->SoulShots;
    Info.UnknownC80 = Data->SpiritShots;
}

// The weapon's values without the shots, the armor's without the MP bonus (the GM's warehouse,
// the trainer's required items)
static FORCEINLINE void SetWeaponValuesNoShots(NItemInfo& Info, NViewItemData* Data)
{
    Info.UnknownC28 = Data->Weight;
    Info.UnknownC2C = Data->Unknown48;
    Info.UnknownC30 = Data->WeaponType;
    Info.UnknownC34[0] = Data->PAtk;
    Info.UnknownC34[1] = Data->MAtk;
    Info.UnknownC34[4] = Data->ShieldPDef;
    Info.UnknownC34[5] = Data->ShieldRate;
    Info.UnknownC34[6] = Data->Unknown40;
    Info.UnknownC34[7] = Data->CrystalGrade;
    Info.UnknownC34[8] = Data->Unknown90;
    Info.UnknownC34[9] = Data->UnknownA4;
    Info.UnknownC34[10] = Data->UnknownA8;
    Info.UnknownC34[11] = Data->AttackSpeed;
    Info.UnknownC34[12] = Data->UnknownBC;
    Info.UnknownC6C = Data->Evasion;
}
static FORCEINLINE void SetArmorValuesNoMP(NItemInfo& Info, NViewItemData* Data)
{
    Info.UnknownC28 = Data->Weight;
    Info.UnknownC2C = Data->Unknown48;
    Info.UnknownC34[6] = Data->Unknown40;
    Info.UnknownC34[7] = Data->ArmorCrystalGrade;
    Info.SubType = Data->ArmorType;
    Info.UnknownC6C = Data->ArmorEvasion;
    Info.UnknownC34[2] = Data->PDef;
    Info.UnknownC34[3] = Data->MDef;
}

// C2: a field of the item record the header has not placed (+0x54, +0x8C), by offset
static FORCEINLINE INT ItemRecordInt(NViewItemData* Data, INT Offset)
{
    return *(INT*)((BYTE*)Data + Offset);
}

// the inventories' etc items (C2: also the value at +0x8C, the etc type for the entry's display name
// (Unknown1710), and the type 5 flag; type 2: NItemEntry +0x1C98, the paperdoll keeps them)
static FORCEINLINE void SetInventoryEtcValues(NItemInfo& Info, NViewItemData* Data)
{
    SetEtcValues(Info, Data);
    Info.UnknownC34[7] = ItemRecordInt(Data, 0x8C);
    Info.Unknown1710 = Data->EtcType;
    if (Data->EtcType == 2)
        Info.UnknownC94 = 1;
    else if (Data->EtcType == 5)
        Info.Unknown17A0 = 1;
}

// The etc values of the pet's and the GM's item lists (C2): the item record's +0x8C and the etc type (the
// inventory's own also flags type 5)
static FORCEINLINE void SetTypedEtcValues(NItemInfo& Info, NViewItemData* Data)
{
    SetEtcValues(Info, Data);
    Info.UnknownC34[7] = ItemRecordInt(Data, 0x8C);
    Info.Unknown1710 = Data->EtcType;
    if (Data->EtcType == 2)
        Info.UnknownC94 = 1;
}

// the class values with the typed etc values (C2: the trade lists)
static FORCEINLINE void SetClassValuesTyped(NItemInfo& Info, INT Class, NViewItemData* Data)
{
    if (Class == ITEMCLASS_Weapon)
        SetWeaponValues(Info, Data);
    else if (Class == ITEMCLASS_Armor)
        SetArmorValues(Info, Data);
    else if (Class == ITEMCLASS_Etc)
        SetTypedEtcValues(Info, Data);
}

/*-----------------------------------------------------------------------------
    The inventory.
-----------------------------------------------------------------------------*/

// 0x100534d0 - a new list follows (AddInventoryItem); shown when the server says so
void NConsoleWnd::ReceiveItemList(INT bShow)
{
    guard(NConsoleWnd::ReceiveItemList);
    GetDesktop()->InventoryWnd->ClearItems();
    if (bShow)
    {
        GetDesktop()->InventoryWnd->ShowWindow();
        GetDesktop()->InventoryWnd->SetFocus();
    }
    unguard;
}
NW_HOOK_METHOD(0x100534d0, NConsoleWnd, ReceiveItemList)

// 0x10053580 - (no handler)
void NConsoleWnd::ReceiveUpdateItemList()
{
    guard(NConsoleWnd::ReceiveUpdateItemList);
    GetDesktop()->InventoryWnd->ClearItems();
    unguard;
}
NW_HOOK_METHOD(0x10053580, NConsoleWnd, ReceiveUpdateItemList)

// 0x10063e00 - with the two-slot items' other icons. C2: the name comes from the item record (+0x40), with its
// additional name (+0x44) and the record's +0x54 (Unknown1788)
void NConsoleWnd::AddInventoryItem(ItemInfo& Item)
{
    if (GetDesktop()->InventoryWnd)
    {
        NItemInfo Info;
        NViewItemData* Data = NItemTable::Find(&Item.ItemID);
        if (Item.ItemID >= 0 && Data)
        {
            INT Class = Data->Class;
            appStrcpy(Info.Name, *Data->Name);
            appStrcpy(Info.AdditionalName, *Data->AdditionalName);
            appStrcpy(Info.IconName, *Data->IconName);
            appStrcpy(Info.Unknown200, *Data->IconName2);
            appStrcpy(Info.Unknown300, *Data->IconName3);
            appStrcpy(Info.Description, GL2GameData.GetItemDescription(Item.ItemID));
            Info.ID = Item.ItemID;
            Info.Param = Item.Type;
            Info.ObjectID = Item.ObjectID;
            Info.Enchant = Item.Enchant;
            Info.Count = Item.Count;
            Info.BodyPart = Item.BodyPart;
            Info.UnknownC8C = Item.bEquipped;
            Info.UnknownC84 = Item.Unknown1C;
            Info.UnknownC70 = Item.Unknown24;
            Info.Unknown1788 = ItemRecordInt(Data, 0x54);
            if (Class == ITEMCLASS_Weapon)
                SetWeaponValues(Info, Data);
            else if (Class == ITEMCLASS_Armor)
                SetArmorValues(Info, Data);
            else if (Class == ITEMCLASS_Etc)
                SetInventoryEtcValues(Info, Data);
            GetDesktop()->InventoryWnd->AddItem(&Info);
        }
    }
}
NW_HOOK_METHOD(0x10063e00, NConsoleWnd, AddInventoryItem)

// 0x10064120 - an item added (1), changed (2) or removed (3). C2: the name from the game data, the additional
// name and the two other icons from the item record, and its +0x54 (Unknown1788)
void NConsoleWnd::UpdateInventoryItem(INT Change, ItemInfo& Item)
{
    if (!GetDesktop()->InventoryWnd)
        return;
    NItemInfo Info;
    NViewItemData* Data = NItemTable::Find(&Item.ItemID);
    if (Item.ItemID < 0 || !Data)
        return;
    INT Class = Data->Class;
    appStrcpy(Info.Name, GL2GameData.GetItemName(Item.ItemID));
    appStrcpy(Info.AdditionalName, *Data->AdditionalName);
    appStrcpy(Info.IconName, *Data->IconName);
    appStrcpy(Info.Unknown200, *Data->IconName2);
    appStrcpy(Info.Unknown300, *Data->IconName3);
    appStrcpy(Info.Description, GL2GameData.GetItemDescription(Item.ItemID));
    Info.Param = Item.Type;
    Info.ObjectID = Item.ObjectID;
    Info.ID = Item.ItemID;
    Info.Count = Item.Count;
    Info.BodyPart = Item.BodyPart;
    Info.Enchant = Item.Enchant;
    Info.UnknownC84 = Item.Unknown1C;
    Info.UnknownC8C = Item.bEquipped;
    Info.UnknownC70 = Item.Unknown24;
    Info.Unknown1788 = ItemRecordInt(Data, 0x54);
    if (Class == ITEMCLASS_Weapon)
        SetUpdatedWeaponValues(Info, Data);
    else if (Class == ITEMCLASS_Armor)
        SetArmorValues(Info, Data);
    else if (Class == ITEMCLASS_Etc)
        SetInventoryEtcValues(Info, Data);
    GetDesktop()->InventoryWnd->UpdateItem(&Info, Change);
}
NW_HOOK_METHOD(0x10064120, NConsoleWnd, UpdateInventoryItem)

// 0x10053620 - an enchant scroll was used: the inventory waits for the item to enchant (C2: and it also
// calls its virtual 51, SetItemSelect)
void NConsoleWnd::SelectItemToEnchant()
{
    GetDesktop()->InventoryWnd->SetEnchantMode(1);
    GetDesktop()->InventoryWnd->SetItemSelect();
}
NW_HOOK_METHOD(0x10053620, NConsoleWnd, SelectItemToEnchant)

// 0x100536d0 - the enchant mode ends whatever the result (the stock switches on 0, 1 and 2,
// the same in each case)
void NConsoleWnd::ReceiveEnchantResult(INT Result)
{
    GetDesktop()->InventoryWnd->SetEnchantMode(0);
}
NW_HOOK_METHOD(0x100536d0, NConsoleWnd, ReceiveEnchantResult)

// 0x10053720 - its three values are not used
void NConsoleWnd::UpdateEquipItem(L2ParamStack& Params)
{
    Params.Top();
    Params.Top();
    Params.Top();
}
NW_HOOK_METHOD(0x10053720, NConsoleWnd, UpdateEquipItem)

// 0x10054550 - (no handler) the inventory shown or hidden; shown, it asks for the item list
// first, which shows it (ReceiveItemList)
void NConsoleWnd::ToggleInventory()
{
    if (GetDesktop()->InventoryWnd->IsShowWindow())
        GetDesktop()->InventoryWnd->HideWindow();
    else
        RequestItemList();
}
NW_HOOK_METHOD(0x10054550, NConsoleWnd, ToggleInventory)

/*-----------------------------------------------------------------------------
    The trade.
-----------------------------------------------------------------------------*/

// 0x10064470 - someone asks for a trade: refused at once when not known
void NConsoleWnd::ReceiveTradeDemand(L2ParamStack& Params)
{
    User* Trader = FindUserByID((INT)Params.Top());
    if (Trader)
    {
        NDialogInfo Info;
        Info.ID = DIALOG_TRADE_REQUEST;
        Info.bMainBox = 0;
        Info.bInput = 0;
        Info.SetSystemMessage(MSG_AskTrade);
        Info.SetButtons(Trader->GetName(), NULL);
        Info.bProgress = 1;
        ShowDialog(&Info);
    }
    else
    {
        L2ParamStack Answer(10);
        Answer.PushBack(NULL);
        AnswerTradeRequest(Answer);
    }
}
NW_HOOK_METHOD(0x10064470, NConsoleWnd, ReceiveTradeDemand)

// 0x10064580 - the trade window opens (instead of the inventory) with both names
void NConsoleWnd::ReceiveTradeStart(INT ObjectID)
{
    guard(NConsoleWnd::ReceiveTradeStart);
    if (GetDesktop()->InventoryWnd->IsShowWindow())
        GetDesktop()->InventoryWnd->HideWindow();
    GetDesktop()->TradeWnd->Reset();
    GetDesktop()->TradeWnd->SetMyName(GetMyUser()->GetName());
    GetDesktop()->TradeWnd->SetPartnerName(FindUserByID(ObjectID)->GetName());
    GetDesktop()->TradeWnd->ShowWindow();
    unguard;
}
NW_HOOK_METHOD(0x10064580, NConsoleWnd, ReceiveTradeStart)

// The three trade lists, from an item by value: the inventory's, the player's offer and the
// partner's. C2: the additional name from the item record (+0x44) and the typed etc values
static FORCEINLINE UBOOL MakeTradeItem(NItemInfo& Info, NItemRequest& Item)
{
    NViewItemData* Data = NItemTable::Find(&Item.ItemID);
    if (Item.ItemID < 0 || !Data)
        return 0;
    INT Class = Data->Class;
    appStrcpy(Info.Name, GL2GameData.GetItemName(Item.ItemID));
    appStrcpy(Info.AdditionalName, *Data->AdditionalName);
    appStrcpy(Info.IconName, *Data->IconName);
    appStrcpy(Info.Description, GL2GameData.GetItemDescription(Item.ItemID));
    Info.ID = Item.ItemID;
    Info.Param = Item.Type;
    Info.ObjectID = Item.ObjectID;
    Info.Enchant = Item.Enchant;
    Info.Count = Item.Count;
    Info.BodyPart = Item.BodyPart;
    Info.UnknownC8C = Item.bEquipped;
    Info.UnknownC84 = Item.Unknown1C;
    Info.UnknownC70 = Item.Unknown24;
    SetClassValuesTyped(Info, Class, Data);
    return 1;
}

// 0x10064690
void NConsoleWnd::ReceiveAddTradeInventoryItem(NItemRequest Item)
{
    NItemInfo Info;
    if (MakeTradeItem(Info, Item))
        GetDesktop()->TradeWnd->AddInventoryItem(&Info);
}
NW_HOOK_METHOD(0x10064690, NConsoleWnd, ReceiveAddTradeInventoryItem)

// 0x10064990
void NConsoleWnd::ReceiveAddTradeOwnItem(NItemRequest Item)
{
    NItemInfo Info;
    if (MakeTradeItem(Info, Item))
        GetDesktop()->TradeWnd->AddMyItem(&Info);
}
NW_HOOK_METHOD(0x10064990, NConsoleWnd, ReceiveAddTradeOwnItem)

// 0x10064c90
void NConsoleWnd::ReceiveAddTradeOtherItem(NItemRequest Item)
{
    NItemInfo Info;
    if (MakeTradeItem(Info, Item))
        GetDesktop()->TradeWnd->AddPartnerItem(&Info);
}
NW_HOOK_METHOD(0x10064c90, NConsoleWnd, ReceiveAddTradeOtherItem)

// 0x10064f90 - the inventory list of the trade changed (1 add, 3 change...)
void NConsoleWnd::ReceiveTradeUpdate(INT Change, ItemInfo& Item)
{
    if (!GetDesktop()->TradeWnd)
        return;
    NItemInfo Info;
    NViewItemData* Data = NItemTable::Find(&Item.ItemID);
    if (Item.ItemID < 0 || !Data)
        return;
    INT Class = Data->Class;
    appStrcpy(Info.Name, GL2GameData.GetItemName(Item.ItemID));
    appStrcpy(Info.AdditionalName, *Data->AdditionalName);
    appStrcpy(Info.IconName, *Data->IconName);
    appStrcpy(Info.Description, GL2GameData.GetItemDescription(Item.ItemID));
    Info.ID = Item.ItemID;
    Info.Param = Item.Type;
    Info.ObjectID = Item.ObjectID;
    Info.Enchant = Item.Enchant;
    Info.UnknownC8C = Item.bEquipped;
    Info.Count = Item.Count;
    Info.BodyPart = Item.BodyPart;
    Info.UnknownC84 = Item.Unknown1C;
    Info.UnknownC70 = Item.Unknown24;
    SetClassValuesTyped(Info, Class, Data);
    GetDesktop()->TradeWnd->UpdateInventoryItem(Change, &Info);
}
NW_HOOK_METHOD(0x10064f90, NConsoleWnd, ReceiveTradeUpdate)

// 0x10053980 - the trade ended (the result is not read)
void NConsoleWnd::ReceiveTradeDoneMsg(INT Result)
{
    guard(NConsoleWnd::ReceiveTradeDoneMsg);
    GetDesktop()->TradeWnd->Reset();
    GetDesktop()->TradeWnd->HideWindow();
    unguard;
}
NW_HOOK_METHOD(0x10053980, NConsoleWnd, ReceiveTradeDoneMsg)

/*-----------------------------------------------------------------------------
    The NPC shops and the warehouse: the window opens in its mode, the lists follow.
-----------------------------------------------------------------------------*/

// the shops' items (the NPC's and the player's). C2: the additional name from the item record (+0x44) and the
// typed etc values
static FORCEINLINE UBOOL MakeShopItem(NItemInfo& Info, ItemInfo& Item)
{
    NViewItemData* Data = NItemTable::Find(&Item.ItemID);
    if (Item.ItemID < 0 || !Data)
        return 0;
    INT Class = Data->Class;
    appStrcpy(Info.Name, GL2GameData.GetItemName(Item.ItemID));
    appStrcpy(Info.AdditionalName, *Data->AdditionalName);
    appStrcpy(Info.IconName, *Data->IconName);
    appStrcpy(Info.Description, GL2GameData.GetItemDescription(Item.ItemID));
    Info.Param = Item.Type;
    Info.ID = Item.ItemID;
    Info.ObjectID = Item.ObjectID;
    Info.Count = Item.Count;
    Info.BodyPart = Item.BodyPart;
    Info.Enchant = Item.Enchant;
    Info.UnknownC70 = Item.Unknown24;
    Info.UnknownC1C = Item.Unknown10;
    Info.UnknownC8C = Item.bEquipped;
    Info.UnknownC84 = Item.Unknown1C;
    SetClassValuesTyped(Info, Class, Data);
    return 1;
}

// the warehouse's items (and the freight's). C2: as the shops' with Unknown2C instead of Unknown10
static FORCEINLINE UBOOL MakeStoreItem(NItemInfo& Info, ItemInfo& Item)
{
    NViewItemData* Data = NItemTable::Find(&Item.ItemID);
    if (Item.ItemID < 0 || !Data)
        return 0;
    INT Class = Data->Class;
    appStrcpy(Info.Name, GL2GameData.GetItemName(Item.ItemID));
    appStrcpy(Info.AdditionalName, *Data->AdditionalName);
    appStrcpy(Info.IconName, *Data->IconName);
    appStrcpy(Info.Description, GL2GameData.GetItemDescription(Item.ItemID));
    Info.Param = Item.Type;
    Info.ID = Item.ItemID;
    Info.ObjectID = Item.ObjectID;
    Info.Count = Item.Count;
    Info.BodyPart = Item.BodyPart;
    Info.Enchant = Item.Enchant;
    Info.UnknownC70 = Item.Unknown24;
    Info.UnknownC8C = Item.bEquipped;
    Info.UnknownC84 = Item.Unknown1C;
    Info.UnknownC90 = Item.Unknown2C;
    SetClassValuesTyped(Info, Class, Data);
    return 1;
}

// 0x10053d50 - the NPC shop in its buy mode: the list's id and the player's adena
void NConsoleWnd::StartBuyList(INT Adena, INT ListID)
{
    GetDesktop()->NpcHtmlWnd->HideWindow();
    GetDesktop()->ShopWnd->Clear();
    GetDesktop()->ShopWnd->SetMode(SHOPMODE_Buy);
    GetDesktop()->ShopWnd->SetListID(ListID);
    GetDesktop()->ShopWnd->SetAdena(Adena);
    GetDesktop()->ShopWnd->ShowWindow();
}
NW_HOOK_METHOD(0x10053d50, NConsoleWnd, StartBuyList)

// 0x10065870 (vslot 62)
void NConsoleWnd::AddBuyList(ItemInfo& Item)
{
    NItemInfo Info;
    if (MakeShopItem(Info, Item))
        GetDesktop()->ShopWnd->AddShopItem(&Info);
}
NW_HOOK_METHOD(0x10065870, NConsoleWnd, AddBuyList)

// 0x10053dd0 (vslot 63) - the sell mode, instead of the inventory
void NConsoleWnd::StartSellList(INT Adena, INT ListID)
{
    GetDesktop()->NpcHtmlWnd->HideWindow();
    if (GetDesktop()->InventoryWnd->IsShowWindow())
        GetDesktop()->InventoryWnd->HideWindow();
    GetDesktop()->ShopWnd->Clear();
    GetDesktop()->ShopWnd->SetMode(SHOPMODE_Sell);
    GetDesktop()->ShopWnd->SetListID(ListID);
    GetDesktop()->ShopWnd->SetAdena(Adena);
    GetDesktop()->ShopWnd->ShowWindow();
}
NW_HOOK_METHOD(0x10053dd0, NConsoleWnd, StartSellList)

// 0x10065b60 (vslot 64)
void NConsoleWnd::AddSellList(ItemInfo& Item)
{
    NItemInfo Info;
    if (MakeShopItem(Info, Item))
        GetDesktop()->ShopWnd->AddPlayerItem(&Info);
}
NW_HOOK_METHOD(0x10065b60, NConsoleWnd, AddSellList)

// 0x10053b20 (vslot 65) - C2 only: the manor's shop window (the desktop's +0xE4) in its buy mode: the same as
// StartBuyList, on another window (its Clear 0x10092300, SetMode 0x10093290, SetListID 0x100922e0, SetAdena 0x100922f0)
void NConsoleWnd::StartManorBuyList(INT Adena, INT ListID)
{
    GetDesktop()->NpcHtmlWnd->HideWindow();
    NCManorShopWnd* Manor = static_cast<NCManorShopWnd*>(GetDesktop()->NewWndE4);
    Manor->Clear();
    Manor->SetMode(0);
    Manor->SetListID(ListID);
    Manor->SetAdena(Adena);
    nw::Field<NCWnd*>(Desktop, 0xE4)->ShowWindow();
}
NW_HOOK_METHOD(0x10053b20, NConsoleWnd, StartManorBuyList)

// 0x10065290 (vslot 66) - C2 only: the seeds the manor buys (AddBuyList's twin: the window's AddShopItem is
// 0x100932d0)
void NConsoleWnd::AddManorBuyList(ItemInfo& Item)
{
    NItemInfo Info;
    if (MakeShopItem(Info, Item))
        static_cast<NCManorShopWnd*>(GetDesktop()->NewWndE4)->AddShopItem(&Info);
}
NW_HOOK_METHOD(0x10065290, NConsoleWnd, AddManorBuyList)

// 0x10053ba0 (vslot 67) - C2 only: the sell mode (instead of the inventory), as StartSellList
void NConsoleWnd::StartManorSellList(INT Adena, INT ListID)
{
    GetDesktop()->NpcHtmlWnd->HideWindow();
    if (GetDesktop()->InventoryWnd->IsShowWindow())
        GetDesktop()->InventoryWnd->HideWindow();
    NCManorShopWnd* Manor = static_cast<NCManorShopWnd*>(GetDesktop()->NewWndE4);
    Manor->Clear();
    Manor->SetMode(1);
    Manor->SetListID(ListID);
    Manor->SetAdena(Adena);
    nw::Field<NCWnd*>(Desktop, 0xE4)->ShowWindow();
}
NW_HOOK_METHOD(0x10053ba0, NConsoleWnd, StartManorSellList)

// 0x10065580 (vslot 68) - C2 only: what the player can sell the manor (AddSellList's twin: 0x100932e0)
void NConsoleWnd::AddManorSellList(ItemInfo& Item)
{
    NItemInfo Info;
    if (MakeShopItem(Info, Item))
        static_cast<NCManorShopWnd*>(GetDesktop()->NewWndE4)->AddPlayerItem(&Info);
}
NW_HOOK_METHOD(0x10065580, NConsoleWnd, AddManorSellList)

// 0x10053ea0 - the warehouse in its deposit mode, instead of the inventory
void NConsoleWnd::StartDepositList(INT Adena)
{
    GetDesktop()->NpcHtmlWnd->HideWindow();
    if (GetDesktop()->InventoryWnd->IsShowWindow())
        GetDesktop()->InventoryWnd->HideWindow();
    GetDesktop()->StoreWnd->Clear();
    GetDesktop()->StoreWnd->SetMode(STOREMODE_Deposit);
    GetDesktop()->StoreWnd->SetAdena(Adena);
    GetDesktop()->StoreWnd->ShowWindow();
}
NW_HOOK_METHOD(0x10053ea0, NConsoleWnd, StartDepositList)

// 0x10065e50
void NConsoleWnd::AddDepositList(ItemInfo& Item)
{
    NItemInfo Info;
    if (MakeStoreItem(Info, Item))
        GetDesktop()->StoreWnd->AddInventoryItem(&Info);
}
NW_HOOK_METHOD(0x10065e50, NConsoleWnd, AddDepositList)

// 0x10053f40 - the withdraw mode
void NConsoleWnd::StartWithdrawList(INT Adena)
{
    GetDesktop()->NpcHtmlWnd->HideWindow();
    GetDesktop()->StoreWnd->Clear();
    GetDesktop()->StoreWnd->SetMode(STOREMODE_Withdraw);
    GetDesktop()->StoreWnd->SetAdena(Adena);
    GetDesktop()->StoreWnd->ShowWindow();
}
NW_HOOK_METHOD(0x10053f40, NConsoleWnd, StartWithdrawList)

// 0x10066140
void NConsoleWnd::AddWithdrawList(ItemInfo& Item)
{
    NItemInfo Info;
    if (MakeStoreItem(Info, Item))
        GetDesktop()->StoreWnd->AddStoredItem(&Info);
}
NW_HOOK_METHOD(0x10066140, NConsoleWnd, AddWithdrawList)

// 0x1006c1e0 (vslot 73) - said only when it failed
void NConsoleWnd::ReceiveWareHouseDone(L2ParamStack& Params)
{
    if (!Params.Top())
        ShowSystemMessage(MSG_WarehouseFailed, 0xFFB09B79);
}
NW_HOOK_METHOD(0x1006c1e0, NConsoleWnd, ReceiveWareHouseDone)

/*-----------------------------------------------------------------------------
    The freight.
-----------------------------------------------------------------------------*/

// 0x10054c30 - a character the freight can go to (its object id and name); the first one
// (index 0) opens the window
void NConsoleWnd::AddPackageToList(L2ParamStack& Params)
{
    if (!Params.Top())
        GetDesktop()->SelectDeliverWnd->ShowWindow();
    INT ID = (INT)Params.Top();
    const TCHAR* Name = (const TCHAR*)Params.Top();
    GetDesktop()->SelectDeliverWnd->AddTarget(ID, Name);
}
NW_HOOK_METHOD(0x10054c30, NConsoleWnd, AddPackageToList)

// 0x10067c20 - the items of the freight window (C2: the additional name from the game data, the typed etc
// values); Index 0 opens the window with the adena and the target
void NConsoleWnd::AddSendableItemList(INT Index, INT TargetID, INT Adena, ItemInfo& Item)
{
    if (!Index)
    {
        GetDesktop()->DeliverWnd->ShowWindow();
        GetDesktop()->DeliverWnd->SetAdena(Adena);
        GetDesktop()->DeliverWnd->SetTargetID(TargetID);
    }
    NItemInfo Info;
    NViewItemData* Data = NItemTable::Find(&Item.ItemID);
    if (Item.ItemID >= 0 && Data)
    {
        INT Class = Data->Class;
        appStrcpy(Info.Name, GL2GameData.GetItemName(Item.ItemID));
        appStrcpy(Info.AdditionalName, GL2GameData.GetItemAdditionalName(Item.ItemID));
        appStrcpy(Info.IconName, *Data->IconName);
        appStrcpy(Info.Description, GL2GameData.GetItemDescription(Item.ItemID));
        Info.Param = Item.Type;
        Info.ObjectID = Item.ObjectID;
        Info.ID = Item.ItemID;
        Info.BodyPart = Item.BodyPart;
        Info.Count = Item.Count;
        Info.Enchant = Item.Enchant;
        Info.UnknownC70 = Item.Unknown24;
        Info.UnknownC84 = Item.Unknown1C;
        Info.UnknownC8C = Item.bEquipped;
        Info.UnknownC90 = Item.Unknown2C;
        SetClassValuesTyped(Info, Class, Data);
        GetDesktop()->DeliverWnd->AddInventoryItem(&Info);
    }
}
NW_HOOK_METHOD(0x10067c20, NConsoleWnd, AddSendableItemList)

/*-----------------------------------------------------------------------------
    The private stores: the window opens in its mode (1 one's own, 0 someone's) when not
    shown yet, then takes each item.
-----------------------------------------------------------------------------*/

// a store's items as the selling window shows them (C2: the additional name from the game data, the typed
// etc values)
static FORCEINLINE UBOOL MakeSellItem(NItemInfo& Info, ItemInfo& Item)
{
    NViewItemData* Data = NItemTable::Find(&Item.ItemID);
    if (Item.ItemID < 0 || !Data)
        return 0;
    INT Class = Data->Class;
    appStrcpy(Info.Name, GL2GameData.GetItemName(Item.ItemID));
    appStrcpy(Info.AdditionalName, GL2GameData.GetItemAdditionalName(Item.ItemID));
    appStrcpy(Info.IconName, *Data->IconName);
    appStrcpy(Info.Description, GL2GameData.GetItemDescription(Item.ItemID));
    Info.ObjectID = Item.ObjectID;
    Info.ID = Item.ItemID;
    Info.Count = Item.Count;
    Info.Enchant = Item.Enchant;
    Info.UnknownC98 = Item.Unknown28;
    Info.UnknownC84 = Item.Unknown1C;
    Info.UnknownC70 = Item.Unknown24;
    Info.Param = Item.Type;
    Info.BodyPart = Item.BodyPart;
    Info.UnknownC1C = Item.Unknown10;
    SetClassValuesTyped(Info, Class, Data);
    return 1;
}

// 0x10069190 - one's own selling store: an item of the inventory and one on sale (C2: the texts and the
// typed etc values as the other lists)
void NConsoleWnd::PrivateStoreManageList(ItemInfo& InventoryItem, ItemInfo& StoreItem, INT OwnerID, INT Adena)
{
    if (!GetDesktop()->PrivateShopWnd->IsShowWindow())
    {
        GetDesktop()->PrivateShopWnd->Reset();
        GetDesktop()->PrivateShopWnd->SetMode(1);
        GetDesktop()->PrivateShopWnd->ShowWindow();
    }
    GetDesktop()->PrivateShopWnd->SetOwnerID(OwnerID);
    GetDesktop()->PrivateShopWnd->SetAdena(Adena);
    NItemInfo PlayerInfo;
    NItemInfo StoreInfo;
    NViewItemData* Data = NItemTable::Find(&InventoryItem.ItemID);
    if (InventoryItem.ItemID >= 0 && Data)
    {
        INT Class = Data->Class;
        appStrcpy(PlayerInfo.Name, GL2GameData.GetItemName(InventoryItem.ItemID));
        appStrcpy(PlayerInfo.AdditionalName, GL2GameData.GetItemAdditionalName(InventoryItem.ItemID));
        appStrcpy(PlayerInfo.IconName, *Data->IconName);
        appStrcpy(PlayerInfo.Description, GL2GameData.GetItemDescription(InventoryItem.ItemID));
        PlayerInfo.Count = InventoryItem.Count;
        PlayerInfo.ID = InventoryItem.ItemID;
        PlayerInfo.ObjectID = InventoryItem.ObjectID;
        PlayerInfo.UnknownC70 = InventoryItem.Unknown24;
        PlayerInfo.Enchant = InventoryItem.Enchant;
        PlayerInfo.UnknownC84 = InventoryItem.Unknown1C;
        PlayerInfo.Param = InventoryItem.Type;
        PlayerInfo.UnknownC98 = InventoryItem.Unknown28;
        PlayerInfo.BodyPart = InventoryItem.BodyPart;
        SetClassValuesTyped(PlayerInfo, Class, Data);
        GetDesktop()->PrivateShopWnd->AddPlayerItem(&PlayerInfo);
    }
    if (MakeSellItem(StoreInfo, StoreItem))
        GetDesktop()->PrivateShopWnd->AddStoreItem(&StoreInfo);
}
NW_HOOK_METHOD(0x10069190, NConsoleWnd, PrivateStoreManageList)

// 0x100697c0 - someone's selling store: the window opens in its buy mode when not shown yet, then takes the
// owner, the adena and each item
void NConsoleWnd::PrivateStoreList(ItemInfo& Item, INT OwnerID, INT Adena)
{
    if (!GetDesktop()->PrivateShopWnd->IsShowWindow())
    {
        GetDesktop()->PrivateShopWnd->Reset();
        GetDesktop()->PrivateShopWnd->SetMode(0);
        GetDesktop()->PrivateShopWnd->ShowWindow();
    }
    GetDesktop()->PrivateShopWnd->SetOwnerID(OwnerID);
    GetDesktop()->PrivateShopWnd->SetAdena(Adena);
    NItemInfo Info;
    if (MakeSellItem(Info, Item))
        GetDesktop()->PrivateShopWnd->AddStoreItem(&Info);
}
NW_HOOK_METHOD(0x100697c0, NConsoleWnd, PrivateStoreList)

// 0x10069b30 - one's own buying store: an item of the inventory (by its item id) and one bought (C2: the
// texts and the typed etc values as the other lists)
void NConsoleWnd::PrivateStoreBuyManageList(ItemInfo& InventoryItem, ItemInfo& StoreItem, INT OwnerID, INT Adena)
{
    if (!GetDesktop()->PrivateBuyWnd->IsShowWindow())
    {
        GetDesktop()->PrivateBuyWnd->Reset();
        GetDesktop()->PrivateBuyWnd->SetMode(1);
        GetDesktop()->PrivateBuyWnd->ShowWindow();
    }
    GetDesktop()->PrivateBuyWnd->SetOwnerID(OwnerID);
    GetDesktop()->PrivateBuyWnd->SetAdena(Adena);
    NItemInfo PlayerInfo;
    NItemInfo StoreInfo;
    NViewItemData* Data = NItemTable::Find(&InventoryItem.ItemID);
    if (InventoryItem.ItemID >= 0 && Data)
    {
        INT Class = Data->Class;
        appStrcpy(PlayerInfo.Name, GL2GameData.GetItemName(InventoryItem.ItemID));
        appStrcpy(PlayerInfo.AdditionalName, GL2GameData.GetItemAdditionalName(InventoryItem.ItemID));
        appStrcpy(PlayerInfo.IconName, *Data->IconName);
        appStrcpy(PlayerInfo.Description, GL2GameData.GetItemDescription(InventoryItem.ItemID));
        PlayerInfo.ID = InventoryItem.ItemID;
        PlayerInfo.Count = InventoryItem.Count;
        PlayerInfo.Enchant = InventoryItem.Enchant;
        PlayerInfo.UnknownC70 = InventoryItem.Unknown24;
        PlayerInfo.Param = InventoryItem.Type;
        PlayerInfo.UnknownC98 = InventoryItem.Unknown28;
        PlayerInfo.BodyPart = InventoryItem.BodyPart;
        SetClassValuesTyped(PlayerInfo, Class, Data);
        GetDesktop()->PrivateBuyWnd->AddPlayerItem(&PlayerInfo);
    }
    Data = NItemTable::Find(&StoreItem.ItemID);
    if (StoreItem.ItemID >= 0 && Data)
    {
        INT Class = Data->Class;
        appStrcpy(StoreInfo.Name, GL2GameData.GetItemName(StoreItem.ItemID));
        appStrcpy(StoreInfo.AdditionalName, GL2GameData.GetItemAdditionalName(StoreItem.ItemID));
        appStrcpy(StoreInfo.IconName, *Data->IconName);
        appStrcpy(StoreInfo.Description, GL2GameData.GetItemDescription(StoreItem.ItemID));
        StoreInfo.Count = StoreItem.Count;
        StoreInfo.ID = StoreItem.ItemID;
        StoreInfo.Enchant = StoreItem.Enchant;
        StoreInfo.UnknownC98 = StoreItem.Unknown28;
        StoreInfo.UnknownC70 = StoreItem.Unknown24;
        StoreInfo.BodyPart = StoreItem.BodyPart;
        StoreInfo.Param = StoreItem.Type;
        StoreInfo.UnknownC1C = StoreItem.Unknown10;
        StoreInfo.UnknownC90 = StoreItem.Unknown2C;
        SetClassValuesTyped(StoreInfo, Class, Data);
        GetDesktop()->PrivateBuyWnd->AddStoreItem(&StoreInfo);
    }
}
NW_HOOK_METHOD(0x10069b30, NConsoleWnd, PrivateStoreBuyManageList)

// 0x1006a140 - someone's buying store (C2: the same item values as the selling one, plus Unknown2C)
void NConsoleWnd::PrivateStoreBuyList(ItemInfo& Item, INT OwnerID, INT Adena)
{
    if (!GetDesktop()->PrivateBuyWnd->IsShowWindow())
    {
        GetDesktop()->PrivateBuyWnd->Reset();
        GetDesktop()->PrivateBuyWnd->SetMode(0);
        GetDesktop()->PrivateBuyWnd->ShowWindow();
    }
    GetDesktop()->PrivateBuyWnd->SetOwnerID(OwnerID);
    GetDesktop()->PrivateBuyWnd->SetAdena(Adena);
    NItemInfo Info;
    NViewItemData* Data = NItemTable::Find(&Item.ItemID);
    if (Item.ItemID >= 0 && Data)
    {
        INT Class = Data->Class;
        appStrcpy(Info.Name, GL2GameData.GetItemName(Item.ItemID));
        appStrcpy(Info.AdditionalName, GL2GameData.GetItemAdditionalName(Item.ItemID));
        appStrcpy(Info.IconName, *Data->IconName);
        appStrcpy(Info.Description, GL2GameData.GetItemDescription(Item.ItemID));
        Info.ObjectID = Item.ObjectID;
        Info.ID = Item.ItemID;
        Info.Count = Item.Count;
        Info.Enchant = Item.Enchant;
        Info.UnknownC84 = Item.Unknown1C;
        Info.UnknownC98 = Item.Unknown28;
        Info.UnknownC70 = Item.Unknown24;
        Info.BodyPart = Item.BodyPart;
        Info.Param = Item.Type;
        Info.UnknownC1C = Item.Unknown10;
        Info.UnknownC90 = Item.Unknown2C;
        SetClassValuesTyped(Info, Class, Data);
        GetDesktop()->PrivateBuyWnd->AddStoreItem(&Info);
    }
}
NW_HOOK_METHOD(0x1006a140, NConsoleWnd, PrivateStoreBuyList)

// 0x10055a80 - a selling store's lists follow (PrivateStoreManageList): the window is emptied
// and hidden until then
void NConsoleWnd::StartPrivateStoreManageList()
{
    GetDesktop()->PrivateShopWnd->Reset();
    GetDesktop()->PrivateShopWnd->HideWindow();
}
NW_HOOK_METHOD(0x10055a80, NConsoleWnd, StartPrivateStoreManageList)

// the stock body is StartPrivateStoreManageList's (folded)
void NConsoleWnd::StartPrivateStoreList()
{
    GetDesktop()->PrivateShopWnd->Reset();
    GetDesktop()->PrivateShopWnd->HideWindow();
}

// 0x10055b20 - the same for a buying store (PrivateStoreBuyManageList)
void NConsoleWnd::StartPrivateStoreBuyManageList()
{
    GetDesktop()->PrivateBuyWnd->Reset();
    GetDesktop()->PrivateBuyWnd->HideWindow();
}
NW_HOOK_METHOD(0x10055b20, NConsoleWnd, StartPrivateStoreBuyManageList)

// the stock body is StartPrivateStoreBuyManageList's (folded)
void NConsoleWnd::StartPrivateStoreBuyList()
{
    GetDesktop()->PrivateBuyWnd->Reset();
    GetDesktop()->PrivateBuyWnd->HideWindow();
}

/*-----------------------------------------------------------------------------
    The VIP shop.
-----------------------------------------------------------------------------*/

// 0x10055b50 - its list follows (AddVIPItemList, AddVIPNeedItemList)
void NConsoleWnd::SetVIPShopID(INT ShopID)
{
    GetDesktop()->VIPShopWnd->Open(ShopID);
}
NW_HOOK_METHOD(0x10055b50, NConsoleWnd, SetVIPShopID)

// 0x1006a4c0 - a product: the item and its slot line (a weapon's type before it); the item
// table's entry is not tested
void NConsoleWnd::AddVIPItemList(INT ProductID, INT ItemID, INT BodyPart, INT Type, INT Count)
{
    NVIPShopEntry Entry;
    NViewItemData* Data = NItemTable::Find(&ItemID);
    appStrcpy(Entry.Name, GL2GameData.GetItemName(ItemID));
    Entry.ItemID = ItemID;
    Entry.ProductID = ProductID;
    Entry.Count = Count;
    Entry.BodyPart = BodyPart;
    Entry.Type = Type;
    appStrcpy(Entry.IconName, *Data->IconName);
    FString SlotText;
    if (Data->Class == ITEMCLASS_Armor || Type == 1)
    {
        GetItemSlotText(Type, BodyPart, SlotText, Data->ArmorType);
        appStrcpy(Entry.Desc, *SlotText);
    }
    else if (Data->Class == ITEMCLASS_Weapon)
    {
        GetItemSlotText(Type, BodyPart, SlotText, -1);
        appSprintf(Entry.Desc, TEXT("%s / %s"), GetWeaponTypeName(Data->WeaponType), *SlotText);
    }
    else
    {
        GetItemSlotText(Type, BodyPart, SlotText, -1);
        appStrcpy(Entry.Desc, *SlotText);
    }
    GetDesktop()->VIPShopWnd->AddRecord(&Entry);
}
NW_HOOK_METHOD(0x1006a4c0, NConsoleWnd, AddVIPItemList)

// 0x1006a6d0 - an item a product costs (C2: NVIPNeededItem is 0x30C bytes; the window's AddNeededItem is 0x100873f0)
void NConsoleWnd::AddVIPNeedItemList(INT ProductID, INT ItemID, INT Count)
{
    NVIPNeededItem Needed;
    NViewItemData* Data = GL2GameData.ItemTable->FindRef(ItemID);
    appStrcpy(Needed.Name, GL2GameData.GetItemName(ItemID));
    Needed.ItemID = ItemID;
    appStrcpy(Needed.IconName, *Data->IconName);
    Needed.Count = Count;
    GetDesktop()->VIPShopWnd->AddNeededItem(ProductID, &Needed);
}
NW_HOOK_METHOD(0x1006a6d0, NConsoleWnd, AddVIPNeedItemList)

// 0x10055b70 (vslot 201)
void NConsoleWnd::VIPItemListEnd()
{
    GetDesktop()->VIPShopWnd->ShowWindow();
    GetDesktop()->VIPShopWnd->ShowList();
}
NW_HOOK_METHOD(0x10055b70, NConsoleWnd, VIPItemListEnd)

/*-----------------------------------------------------------------------------
    The pet's inventory, the quest items.
-----------------------------------------------------------------------------*/

// 0x100683d0 - (no handler) a pet's item. C2: the additional name comes from the game data
void NConsoleWnd::AddPetInventoryItem(ItemInfo& Item)
{
    NCPetWnd* PetWnd = GetDesktop()->PetWnd;
    if (PetWnd && nw::Field<void*>(PetWnd, 0x170))
    {
        NItemInfo Info;
        NViewItemData* Data = NItemTable::Find(&Item.ItemID);
        if (Item.ItemID >= 0 && Data)
        {
            INT Class = Data->Class;
            appStrcpy(Info.Name, GL2GameData.GetItemName(Item.ItemID));
            appStrcpy(Info.AdditionalName, GL2GameData.GetItemAdditionalName(Item.ItemID));
            appStrcpy(Info.IconName, *Data->IconName);
            appStrcpy(Info.Description, GL2GameData.GetItemDescription(Item.ItemID));
            Info.Param = Item.Type;
            Info.ID = Item.ItemID;
            Info.ObjectID = Item.ObjectID;
            Info.Count = Item.Count;
            Info.Enchant = Item.Enchant;
            Info.UnknownC84 = Item.Unknown1C;
            Info.BodyPart = Item.BodyPart;
            Info.UnknownC8C = Item.bEquipped;
            Info.UnknownC70 = Item.Unknown24;
            if (Class == ITEMCLASS_Weapon)
                SetWeaponValues(Info, Data);
            else if (Class == ITEMCLASS_Armor)
                SetArmorValues(Info, Data);
            else if (Class == ITEMCLASS_Etc)
                SetTypedEtcValues(Info, Data);
            GetDesktop()->PetWnd->AddInventoryItem(&Info);
        }
    }
}
NW_HOOK_METHOD(0x100683d0, NConsoleWnd, AddPetInventoryItem)

// 0x100686e0 - as UpdateInventoryItem, for the pet's (C2: the same text and value changes as AddPetInventoryItem)
void NConsoleWnd::UpdatePetInventoryItem(INT Change, ItemInfo& Item)
{
    NCPetWnd* PetWnd = GetDesktop()->PetWnd;
    if (!PetWnd || !nw::Field<void*>(PetWnd, 0x170))
        return;
    NItemInfo Info;
    NViewItemData* Data = NItemTable::Find(&Item.ItemID);
    if (Item.ItemID < 0 || !Data)
        return;
    INT Class = Data->Class;
    appStrcpy(Info.Name, GL2GameData.GetItemName(Item.ItemID));
    appStrcpy(Info.AdditionalName, GL2GameData.GetItemAdditionalName(Item.ItemID));
    appStrcpy(Info.IconName, *Data->IconName);
    appStrcpy(Info.Description, GL2GameData.GetItemDescription(Item.ItemID));
    Info.ObjectID = Item.ObjectID;
    Info.Param = Item.Type;
    Info.ID = Item.ItemID;
    Info.BodyPart = Item.BodyPart;
    Info.Enchant = Item.Enchant;
    Info.UnknownC70 = Item.Unknown24;
    Info.Count = Item.Count;
    Info.UnknownC8C = Item.bEquipped;
    Info.UnknownC84 = Item.Unknown1C;
    if (Class == ITEMCLASS_Weapon)
        SetUpdatedWeaponValues(Info, Data);
    else if (Class == ITEMCLASS_Armor)
        SetArmorValues(Info, Data);
    else if (Class == ITEMCLASS_Etc)
        SetTypedEtcValues(Info, Data);
    GetDesktop()->PetWnd->UpdateInventoryItem(Change, &Info);
}
NW_HOOK_METHOD(0x100686e0, NConsoleWnd, UpdatePetInventoryItem)

// 0x100665d0 - an item a skill of the trainer needs: its type, item id, count and one more
// value (as the body part). C2: the additional name from the item record, the typed etc values
void NConsoleWnd::AddAcquireSkillExtendedInfo(L2ParamStack& Params)
{
    INT Type = (INT)Params.Top();
    INT ItemID = (INT)Params.Top();
    INT Count = (INT)Params.Top();
    INT BodyPart = (INT)Params.Top();
    NItemInfo Info;
    NViewItemData* Data = NItemTable::Find(&ItemID);
    if (ItemID < 0 || !Data)
        return;
    INT Class = Data->Class;
    appStrcpy(Info.Name, GL2GameData.GetItemName(ItemID));
    appStrcpy(Info.AdditionalName, *Data->AdditionalName);
    appStrcpy(Info.IconName, *Data->IconName);
    appStrcpy(Info.Description, GL2GameData.GetItemDescription(ItemID));
    if (Class == ITEMCLASS_Weapon)
        SetWeaponValuesNoShots(Info, Data);
    else if (Class == ITEMCLASS_Armor)
        SetArmorValuesNoMP(Info, Data);
    else if (Class == ITEMCLASS_Etc)
        SetTypedEtcValues(Info, Data);
    Info.Param = Type;
    Info.ID = ItemID;
    Info.Count = Count;
    Info.BodyPart = BodyPart;
    GetDesktop()->TrainWnd->AddRequiredItem(&Info);
}
NW_HOOK_METHOD(0x100665d0, NConsoleWnd, AddAcquireSkillExtendedInfo)

// the quest items (type 3), with the etc item's values
static FORCEINLINE void MakeQuestItem(NItemInfo& Info, ItemInfo& Item, NViewItemData* Data)
{
    SetItemTexts(Info, Item.ItemID, Data);
    Info.ObjectID = Item.ObjectID;
    Info.ID = Item.ItemID;
    Info.Count = Item.Count;
    Info.BodyPart = Item.BodyPart;
    Info.Param = 3;
    SetEtcValues(Info, Data);
}

// 0x10066430 - only etc items (a quest item: type 3)
void NConsoleWnd::AddQuestItem(ItemInfo& Item)
{
    if (GetDesktop()->MainWnd->QuestWnd)
    {
        NItemInfo Info;
        NViewItemData* Data = NItemTable::Find(&Item.ItemID);
        if (Item.ItemID >= 0 && Data && Data->Class == ITEMCLASS_Etc)
        {
            MakeQuestItem(Info, Item, Data);
            static_cast<NCQuestWnd*>(GetDesktop()->MainWnd->QuestWnd)->AddQuestItem(&Info);
        }
    }
}
NW_HOOK_METHOD(0x10066430, NConsoleWnd, AddQuestItem)

/*-----------------------------------------------------------------------------
    A GM's views of a character's items.
-----------------------------------------------------------------------------*/

// 0x100556b0
void NConsoleWnd::GMViewReceiveItemList(INT bShow)
{
    guard(NConsoleWnd::ReceiveItemList);
    GetDesktop()->GMInventoryWnd->ClearItems();
    if (bShow)
    {
        GetDesktop()->GMInventoryWnd->ShowWindow();
        GetDesktop()->GMInventoryWnd->SetFocus();
    }
    unguard;
}
NW_HOOK_METHOD(0x100556b0, NConsoleWnd, GMViewReceiveItemList)

// 0x10068b90 - as AddInventoryItem, for the GM's inventory window (C2: the additional name from the game
// data, no Unknown1788, the typed etc values)
void NConsoleWnd::GMViewAddInventoryItem(ItemInfo& Item)
{
    if (GetDesktop()->GMInventoryWnd)
    {
        NItemInfo Info;
        NViewItemData* Data = NItemTable::Find(&Item.ItemID);
        if (Item.ItemID >= 0 && Data)
        {
            INT Class = Data->Class;
            appStrcpy(Info.Name, GL2GameData.GetItemName(Item.ItemID));
            appStrcpy(Info.AdditionalName, GL2GameData.GetItemAdditionalName(Item.ItemID));
            appStrcpy(Info.IconName, *Data->IconName);
            appStrcpy(Info.Unknown200, *Data->IconName2);
            appStrcpy(Info.Unknown300, *Data->IconName3);
            appStrcpy(Info.Description, GL2GameData.GetItemDescription(Item.ItemID));
            Info.ObjectID = Item.ObjectID;
            Info.Param = Item.Type;
            Info.ID = Item.ItemID;
            Info.BodyPart = Item.BodyPart;
            Info.Enchant = Item.Enchant;
            Info.UnknownC70 = Item.Unknown24;
            Info.Count = Item.Count;
            Info.UnknownC8C = Item.bEquipped;
            Info.UnknownC84 = Item.Unknown1C;
            if (Class == ITEMCLASS_Weapon)
                SetWeaponValues(Info, Data);
            else if (Class == ITEMCLASS_Armor)
                SetArmorValues(Info, Data);
            else if (Class == ITEMCLASS_Etc)
                SetTypedEtcValues(Info, Data);
            GetDesktop()->GMInventoryWnd->AddItem(&Info);
        }
    }
}
NW_HOOK_METHOD(0x10068b90, NConsoleWnd, GMViewAddInventoryItem)

// 0x10055760 - the GM's warehouse window, in the withdraw mode
void NConsoleWnd::GMViewStartWithdrawList(INT Adena)
{
    GetDesktop()->GMStoreWnd->Clear();
    GetDesktop()->GMStoreWnd->SetMode(STOREMODE_Withdraw);
    GetDesktop()->GMStoreWnd->SetAdena(Adena);
    GetDesktop()->GMStoreWnd->ShowWindow();
}
NW_HOOK_METHOD(0x10055760, NConsoleWnd, GMViewStartWithdrawList)

// 0x10068ec0 - as AddWithdrawList, without the shots and the MP bonus (and the additional name from the game
// data)
void NConsoleWnd::GMViewAddWithdrawList(ItemInfo& Item)
{
    NItemInfo Info;
    NViewItemData* Data = NItemTable::Find(&Item.ItemID);
    if (Item.ItemID < 0 || !Data)
        return;
    INT Class = Data->Class;
    appStrcpy(Info.Name, GL2GameData.GetItemName(Item.ItemID));
    appStrcpy(Info.AdditionalName, GL2GameData.GetItemAdditionalName(Item.ItemID));
    appStrcpy(Info.IconName, *Data->IconName);
    appStrcpy(Info.Description, GL2GameData.GetItemDescription(Item.ItemID));
    Info.Param = Item.Type;
    Info.ID = Item.ItemID;
    Info.ObjectID = Item.ObjectID;
    Info.Count = Item.Count;
    Info.BodyPart = Item.BodyPart;
    Info.Enchant = Item.Enchant;
    Info.UnknownC70 = Item.Unknown24;
    Info.UnknownC8C = Item.bEquipped;
    Info.UnknownC84 = Item.Unknown1C;
    Info.UnknownC90 = Item.Unknown2C;
    if (Class == ITEMCLASS_Weapon)
        SetWeaponValuesNoShots(Info, Data);
    else if (Class == ITEMCLASS_Armor)
        SetArmorValuesNoMP(Info, Data);
    else if (Class == ITEMCLASS_Etc)
        SetTypedEtcValues(Info, Data);
    GetDesktop()->GMStoreWnd->AddStoredItem(&Info);
}
NW_HOOK_METHOD(0x10068ec0, NConsoleWnd, GMViewAddWithdrawList)

// 0x10068a00 - as AddQuestItem, for the GM's quest window and any class of item
void NConsoleWnd::GMViewAddQuestItem(ItemInfo& Item)
{
    if (GetDesktop()->GMQuestWnd)
    {
        NItemInfo Info;
        NViewItemData* Data = NItemTable::Find(&Item.ItemID);
        if (Item.ItemID >= 0 && Data)
        {
            MakeQuestItem(Info, Item, Data);
            GetDesktop()->GMQuestWnd->AddQuestItem(&Info);
        }
    }
}
NW_HOOK_METHOD(0x10068a00, NConsoleWnd, GMViewAddQuestItem)
