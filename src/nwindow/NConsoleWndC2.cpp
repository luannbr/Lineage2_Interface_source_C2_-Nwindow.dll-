/*=============================================================================
    NConsoleWndC2.cpp - the virtuals of the console that only C2 has (slots of the C2 vtable the
    C1 console did not have): the manor lists, the recipes, the henna, the macros, the radar's
    and the shortcut bar's, the town map, the observer, the Christmas seal, the screen resize.
    Names are the ones of include/ue2/gen/UL2ConsoleWnd.vt.inc (a UGameEngine handler's, a guard
    string's, or ours: the note of tools/ue2_data/l2consolewnd_slots_c2.csv says which).

    The windows these slots feed are classes new in C2 (recipes, henna, the macro window...): they are
    reached from the desktop's pointers (NCConsole.h keeps them as NCWnd*, the Desk* helpers below give
    them back as the classes they are) and called through their own methods; nw::Field is left for the
    fields no header names yet.
=============================================================================*/
#include "nwindow/NConsoleWnd.h"
#include "nwindow/NCConsole.h"
#include "nwindow/NCAuthWnd.h"
#include "nwindow/NCXMasSealWnd.h"
#include "nwindow/NMacroEntry.h"
#include "nwindow/NCWnd.h"
#include "nwindow/NItemInfo.h"
#include "nwindow/NGameData.h"
#include "nwindow/NGameGuard.h"
#include "nwindow/NDialogInfo.h"
#include "nwindow/NCShortCutWnd.h"
#include "nwindow/NCGMSnoopManager.h"
#include "nwindow/NCEmpowerWnd.h"
#include "nwindow/NCGMWnd.h"
#include "nwindow/NCHennaInfoWnd.h"
#include "nwindow/NCHennaListBox.h"
#include "nwindow/NCHennaListWnd.h"
#include "nwindow/NCInventoryWnd.h"
#include "nwindow/NCMacroListWnd.h"
#include "nwindow/NCRecipeBookWnd.h"
#include "nwindow/NCRecipeBuyListWnd.h"
#include "nwindow/NCRecipeBuyManufactureWnd.h"
#include "nwindow/NCRecipeListBox.h"
#include "nwindow/NCRecipeManufactureWnd.h"
#include "nwindow/NCRecipeShopWnd.h"
#include "nwindow/NCWarningDialogBox.h"
#include "nwindow/NTownMapWnd.h"
#include "nwindow/NRecipeData.h"
#include <stdio.h>

// the desktop's window at an offset (NCConsole.h declares the ones that are known; the new ones are not)
static FORCEINLINE NCWnd* DeskWnd(NConsoleWnd* Console, DWORD Offset)
{
    return nw::Field<NCWnd*>(Console->Desktop, Offset);
}

// the windows of the desktop that the C2 slots below feed, as the classes they are (NCConsole.h keeps the C2
// ones as NCWnd*: the offsets are the stock's)
static FORCEINLINE NCRecipeBookWnd* DeskRecipeBook(NConsoleWnd* Console)       { return static_cast<NCRecipeBookWnd*>(Console->GetDesktop()->NewWnd134); }
static FORCEINLINE NCRecipeManufactureWnd* DeskRecipeManufacture(NConsoleWnd* Console) { return static_cast<NCRecipeManufactureWnd*>(Console->GetDesktop()->NewWnd13C); }
static FORCEINLINE NCRecipeShopWnd* DeskRecipeShop(NConsoleWnd* Console)       { return static_cast<NCRecipeShopWnd*>(Console->GetDesktop()->RecipeShopWnd); }
static FORCEINLINE NCRecipeBuyListWnd* DeskRecipeBuyList(NConsoleWnd* Console) { return static_cast<NCRecipeBuyListWnd*>(Console->GetDesktop()->NewWnd144); }
static FORCEINLINE NCRecipeBuyManufactureWnd* DeskRecipeBuyManufacture(NConsoleWnd* Console) { return static_cast<NCRecipeBuyManufactureWnd*>(Console->GetDesktop()->RecipeBuyManufactureWnd); }
static FORCEINLINE NCMacroListWnd* DeskMacroList(NConsoleWnd* Console)         { return static_cast<NCMacroListWnd*>(Console->GetDesktop()->NewWndF8); }
static FORCEINLINE NCHennaListWnd* DeskHennaList(NConsoleWnd* Console)         { return static_cast<NCHennaListWnd*>(Console->GetDesktop()->NewWnd14C); }
static FORCEINLINE NCHennaInfoWnd* DeskHennaInfo(NConsoleWnd* Console)         { return static_cast<NCHennaInfoWnd*>(Console->GetDesktop()->NewWnd150); }

// Window A takes the place of window B and keeps its own size, then is shown and focused: the dance of
// every list window of the recipes and the henna (SetWindowRect(B's screen X and Y, A's local W and H))
static FORCEINLINE void PlaceAtAndShow(NCWnd* A, NCWnd* B)
{
    A->SetWindowRect(B->ScreenX, B->ScreenY, A->LocalW, A->LocalH);
    A->ShowWindow();
    A->SetFocus();
}

/*-----------------------------------------------------------------------------
    Records of the stock code (names ours)
-----------------------------------------------------------------------------*/

// What NCRecipeShopWnd's list takes for a recipe on sale (0xA20 bytes, built on the stack by
// AddRecipeShopSellItem and handed to the list window by address)
struct NRecipeSellEntry
{
    TCHAR Name[128];           // +0x000 the recipe item's name
    TCHAR IconName[128];       // +0x100 the product's icon
    TCHAR Description[1024];   // +0x200 the product's description
    INT Unknown00;             // +0xA00 (0)
    INT RecipeID;              // +0xA04
    INT ItemID;                // +0xA08 FL2RecipeData +0x20
    INT Unknown0C;             // +0xA0C FL2RecipeData +0x34
    INT P3;                    // +0xA10
    INT CrystalGrade;          // +0xA14 of the product (item data +0xBC for a weapon, +0x214 for an armor)
    INT P2;                    // +0xA18
    INT Unknown1C;             // +0xA1C (-1)
};
NW_ASSERT_SIZE(NRecipeSellEntry, 0xA20);

// What the henna list windows take for a dye (0x141C bytes, by address): the dye item's names and icon
// and the numbers of the HennaInfo
struct NHennaSendEntry
{
    TCHAR Name[256];           // +0x000 item data +0x40
    TCHAR IconName[256];       // +0x200 item data +0x34
    TCHAR Description[2048];   // +0x400 item data +0x44 (its additional name), or henna data +0x2C
    INT Unknown1400;           // +0x1400 (0)
    INT Info0;                 // +0x1404 HennaInfo +0x00
    INT ItemID;                // +0x1408 HennaInfo +0x04
    INT Info8;                 // +0x140C
    INT InfoC;                 // +0x1410
    INT Info10;                // +0x1414
    INT Unknown1418;           // +0x1418 (0)
};
NW_ASSERT_SIZE(NHennaSendEntry, 0x141C);

/*-----------------------------------------------------------------------------
    C2-only virtuals: the slots C2 added to the console's vtable. Those not ported yet are stubs that
    jump to the stock function (tools/verify_hooks.py wants every slot to reach its stock function).
-----------------------------------------------------------------------------*/

// 0x10061c80 (vslot 28) - a message on the warning dialog box (WarningDialogBox, +0x3554): P2 locks it
// (+0x1B8), P1 sets its message (0x10006e00) and shows it modal (0x10006dc0)
void NConsoleWnd::ShowWarningDialog(INT P1, INT P2)
{
    if (P2)
        nw::Field<INT>(WarningDialogBox, 0x1B8) = 1;
    if (P1)
    {
        static_cast<NCWarningDialogBox*>(WarningDialogBox)->SetText((const TCHAR*)P1);
        static_cast<NCWarningDialogBox*>(WarningDialogBox)->ShowModal();
    }
}
NW_HOOK_METHOD(0x10061c80, NConsoleWnd, ShowWarningDialog)

// 0x10057aa0 (vslot 29) - the nProtect query (four values, by address) goes to NPGameLibApi::SendAuthQuery
// (0x100e0130, cdecl; NGameGuard.h: in hook mode the library stays the original, the standalone build has none and the
// query goes unanswered)
void NConsoleWnd::GameGuardQuery(INT P1, INT P2, INT P3, INT P4)
{
    guard(NConsoleWnd::GameGuardQuery);
    INT Query[4];
    Query[0] = P1;
    Query[1] = P2;
    Query[2] = P3;
    Query[3] = P4;
    NGameGuard::SendAuthQuery(Query);
    unguard;
}
NW_HOOK_METHOD(0x10057aa0, NConsoleWnd, GameGuardQuery)

// 0x10058760 (vslot 31) - the security card check starts (UGameEngine::OnSCCheckStart): the login
// screen's first window (+0x98) takes the two values (NCAuthWnd::SetCardKey, 0x10057c80: it asks for the card's cell)
void NConsoleWnd::SCCheckStart(INT P1, INT P2)
{
    if (LoginWnd)
        nw::Field<NCAuthWnd*>(LoginWnd, 0x98)->SetCardKey(P1, P2);
}
NW_HOOK_METHOD(0x10058760, NConsoleWnd, SCCheckStart)

// 0x10053600 (vslot 60) - the inventory window (+0xCC) does its geo check (0x10080440; it takes nothing: the
// stock pops nothing, so UGameEngine::OnCheckGeo's vector is not passed on)
void NConsoleWnd::CheckGeo()
{
    GetDesktop()->InventoryWnd->ApplyItemOrder();
}
NW_HOOK_METHOD(0x10053600, NConsoleWnd, CheckGeo)

// 0x10054530 (vslot 141) - C2 only: NetHandler slot 215 (the henna list)
void NConsoleWnd::RequestHennaItemList()
{
    Engine->NetHandler->RequestHennaItemList();
}
NW_HOOK_METHOD(0x10054530, NConsoleWnd, RequestHennaItemList)

// 0x10054540 (vslot 142) - C2 only: NetHandler slot 216
void NConsoleWnd::RequestHennaUnequipList()
{
    Engine->NetHandler->RequestHennaUnequipList();
}
NW_HOOK_METHOD(0x10054540, NConsoleWnd, RequestHennaUnequipList)

// 0x10057850 (vslot 333) - C2 only: NetHandler's +0x3f0
void NConsoleWnd::RequestObserverEndPacket(L2ParamStack& Params)
{
    Engine->NetHandler->RequestObserverEndPacket(Params);
}
NW_HOOK_METHOD(0x10057850, NConsoleWnd, RequestObserverEndPacket)

// 0x10057860 (vslot 334) - the pledge power request: the network handler's slot 253 (+0x3F4), when
// there is an engine and a handler (the stock tail-jumps)
void NConsoleWnd::RequestPledgePower(INT P1, INT P2, BYTE* P3)
{
    if (Engine && Engine->NetHandler)
        Engine->NetHandler->RequestPledgePower(P1, P2, P3);
}
NW_HOOK_METHOD(0x10057860, NConsoleWnd, RequestPledgePower)

/*-----------------------------------------------------------------------------
    Recipes: the recipe book (+0x134, the list at +0x13C), the shops of recipes (+0x140, +0x144, +0x148).
-----------------------------------------------------------------------------*/

// The item info of a recipe, as the recipe lists want it: the recipe's item (FL2RecipeData +0x20: name and
// description) and its product (+0x28: icon, weight, crystal grade)
static FORCEINLINE void FillRecipeItemInfo(NItemInfo& Info, BYTE* Recipe)
{
    Info.ID = *(INT*)(Recipe + 0x20);
    Info.UnknownC20 = *(INT*)(Recipe + 0x24);
    Info.ObjectID = *(INT*)(Recipe + 0x1C);
    NViewItemData* Item = GL2GameData.ItemTable->FindRef(*(INT*)(Recipe + 0x20));
    if (Item)
    {
        appStrcpy(Info.Name, *Item->Name);
        appStrcpy(Info.Description, **(FString*)((BYTE*)Item + 0x48));
        Info.UnknownC28 = Item->Weight;
    }
    NViewItemData* Product = GL2GameData.ItemTable->FindRef(*(INT*)(Recipe + 0x28));
    if (Product)
    {
        appStrcpy(Info.IconName, *Product->IconName);
        if (Product->Class == ITEMCLASS_Weapon)
            Info.UnknownC34[7] = Product->CrystalGrade;
        else if (Product->Class == ITEMCLASS_Armor)
            Info.UnknownC34[7] = Product->ArmorCrystalGrade;
    }
}

// 0x100590c0 (vslot 123) - the recipe book starts over (P1: 0x10098e70) and takes the place of the list
void NConsoleWnd::ReceiveRecipeBookItemList(INT P1)
{
    NCRecipeBookWnd* Book = DeskRecipeBook(this);
    Book->Clear();
    Book->Unknown0BC = P1;   // the body 0x10098e70 (NCMinimapWnd::SetMapID's, folded): a setter of the dword at +0xBC
    PlaceAtAndShow(DeskWnd(this, 0x134), DeskWnd(this, 0x13C));
}
NW_HOOK_METHOD(0x100590c0, NConsoleWnd, ReceiveRecipeBookItemList)

// 0x10066870 (vslot 124) - a recipe of the book (the recipe by its index)
void NConsoleWnd::AddRecipeBookItem(INT RecipeID)
{
    BYTE* Recipe = (BYTE*)GL2GameData.GetRecipeDataByIndex(RecipeID);
    if (!Recipe)
        return;
    NItemInfo Info;
    FillRecipeItemInfo(Info, Recipe);
    DeskRecipeBook(this)->AddRecipe(&Info);
}
NW_HOOK_METHOD(0x10066870, NConsoleWnd, AddRecipeBookItem)

// 0x10059140 (vslot 125) - the make info of a recipe: the list window (+0x13C) shows it (0x100c7de0) with
// the three numbers kept in it (+0x1AE0..), over the book (0x100c47b0) when there is the recipe
void NConsoleWnd::ReceiveRecipeItemMakeInfo(INT P1, INT P2, INT P3, INT P4)
{
    BYTE* Recipe = (BYTE*)GL2GameData.GetRecipeDataByIndex(P1);
    if (!Recipe)
        return;
    DeskWnd(this, 0x134)->HideWindow();
    DeskRecipeManufacture(this)->Reset();
    DeskRecipeManufacture(this)->SetRecipe((NRecipeData*)Recipe);
    nw::Field<INT>(DeskWnd(this, 0x13C), 0x1AE4) = P3;
    nw::Field<INT>(DeskWnd(this, 0x13C), 0x1AE0) = P2;
    nw::Field<INT>(DeskWnd(this, 0x13C), 0x1AE8) = P4;
    PlaceAtAndShow(DeskWnd(this, 0x13C), DeskWnd(this, 0x134));
}
NW_HOOK_METHOD(0x10059140, NConsoleWnd, ReceiveRecipeItemMakeInfo)

// 0x10059220 (vslot 126) - the shop's manage list starts over with its two numbers
void NConsoleWnd::ReceiveRecipeShopManageList(INT P1, INT P2)
{
    DeskRecipeShop(this)->ClearLists();
    nw::Field<INT>(DeskWnd(this, 0x140), 0xC4) = P1;
    nw::Field<INT>(DeskWnd(this, 0x140), 0xC8) = P2;
    DeskWnd(this, 0x140)->ShowWindow();
}
NW_HOOK_METHOD(0x10059220, NConsoleWnd, ReceiveRecipeShopManageList)

// 0x100669a0 (vslot 127) - a recipe the shop can sell (the second number is kept in the entry)
void NConsoleWnd::AddRecipeShopManageItem(INT RecipeID, INT P2)
{
    BYTE* Recipe = (BYTE*)GL2GameData.GetRecipeDataByIndex(RecipeID);
    if (!Recipe)
        return;
    NItemInfo Info;
    FillRecipeItemInfo(Info, Recipe);
    Info.UnknownC90 = P2;
    DeskRecipeShop(this)->AddRecipeItem(&Info);
}
NW_HOOK_METHOD(0x100669a0, NConsoleWnd, AddRecipeShopManageItem)

// 0x10066ae0 (vslot 128) - a recipe the shop has registered for sale
void NConsoleWnd::AddRecipeShopRegisterdItem(INT RecipeID, INT P2, INT P3)
{
    BYTE* Recipe = (BYTE*)GL2GameData.GetRecipeDataByIndex(RecipeID);
    if (!Recipe)
        return;
    NItemInfo Info;
    FillRecipeItemInfo(Info, Recipe);
    Info.UnknownC1C = P3;
    Info.UnknownC90 = P2;
    DeskRecipeShop(this)->AddProductionItem(&Info);
}
NW_HOOK_METHOD(0x10066ae0, NConsoleWnd, AddRecipeShopRegisterdItem)

// 0x10059270 (vslot 129) - the list of recipes a shop sells starts over: the window at +0x144 (0x100c11b0,
// 0x100c11c0), its two numbers (+0xAC, +0xB0) and the third (0x100c11d0), over the window at +0x148
void NConsoleWnd::ReceiveRecipeShopSellList(INT P1, INT P2, INT P3, INT P4)
{
    DeskRecipeBuyList(this)->ClearList();
    DeskRecipeBuyList(this)->SetShopOwner(P1);
    DeskRecipeBuyList(this)->MP = P2;
    DeskRecipeBuyList(this)->MaxMP = P3;
    DeskRecipeBuyList(this)->SetAdena(P4);
    PlaceAtAndShow(DeskWnd(this, 0x144), DeskWnd(this, 0x148));
}
NW_HOOK_METHOD(0x10059270, NConsoleWnd, ReceiveRecipeShopSellList)

// 0x10066c30 (vslot 130) - a recipe the shop sells, with its price and the player's count
void NConsoleWnd::AddRecipeShopSellItem(INT RecipeID, INT P2, INT P3)
{
    BYTE* Recipe = (BYTE*)GL2GameData.GetRecipeDataByIndex(RecipeID);
    if (!Recipe)
        return;
    NRecipeSellEntry Entry;
    Entry.ItemID = *(INT*)(Recipe + 0x20);
    Entry.P3 = P3;
    Entry.Unknown0C = *(INT*)(Recipe + 0x34);
    Entry.Name[0] = 0;
    Entry.IconName[0] = 0;
    Entry.Description[0] = 0;
    Entry.Unknown00 = 0;
    Entry.CrystalGrade = 0;
    Entry.Unknown1C = -1;
    Entry.RecipeID = RecipeID;
    Entry.P2 = P2;
    NViewItemData* Product = GL2GameData.ItemTable->FindRef(*(INT*)(Recipe + 0x28));
    if (Product)
    {
        appStrcpy(Entry.Name, *Product->Name);
        appStrcpy(Entry.Description, **(FString*)((BYTE*)Product + 0x48));
        appStrcpy(Entry.IconName, *Product->IconName);
        if (Product->Class == ITEMCLASS_Weapon)
            Entry.CrystalGrade = Product->CrystalGrade;
        else if (Product->Class == ITEMCLASS_Armor)
            Entry.CrystalGrade = Product->ArmorCrystalGrade;
    }
    DeskRecipeBuyList(this)->AddRecipe((const NRecipeRow*)&Entry);   // NRecipeSellEntry has the layout of NRecipeRow
}
NW_HOOK_METHOD(0x10066c30, NConsoleWnd, AddRecipeShopSellItem)

// 0x10059330 (vslot 131) - the info of a recipe in a shop (the recipe is the second value, by its index): the
// window at +0x148 shows it (0x100c3520) with the other four values (+0x1AE8, +0x1AE4, +0x1AE0, +0x1AF0) over
// the one at +0x144, which hides
void NConsoleWnd::ReceiveRecipeShopItemInfo(INT P1, INT P2, INT P3, INT P4, INT P5)
{
    BYTE* Recipe = (BYTE*)GL2GameData.GetRecipeDataByIndex(P2);
    if (!Recipe)
        return;
    DeskRecipeBuyList(this)->ClearList();
    DeskWnd(this, 0x144)->HideWindow();
    DeskRecipeBuyManufacture(this)->Reset();
    nw::Field<INT>(DeskWnd(this, 0x148), 0x1AE8) = P1;
    nw::Field<INT>(DeskWnd(this, 0x148), 0x1AE4) = P4;
    nw::Field<INT>(DeskWnd(this, 0x148), 0x1AE0) = P3;
    nw::Field<INT>(DeskWnd(this, 0x148), 0x1AF0) = P5;
    DeskRecipeBuyManufacture(this)->SetRecipe((NRecipeData*)Recipe);
    PlaceAtAndShow(DeskWnd(this, 0x148), DeskWnd(this, 0x144));
}
NW_HOOK_METHOD(0x10059330, NConsoleWnd, ReceiveRecipeShopItemInfo)

/*-----------------------------------------------------------------------------
    Macros: the macro window (+0xF8, the list of macros at its +0xC4 and the 24 records it keeps from +0xD4,
    0x3FC bytes each) and what feeds it.
-----------------------------------------------------------------------------*/

// 0x10066d70 (vslot 132) - one macro of the list the server sends (P1 macros in all: C1 had none of this).
// While the list is being received (Unknown357C is set) the first macro empties the window's list (every macro
// object deleted, the 24 records cleared); each macro is copied to the window's record of its number and
// handed to a new macro object (0x1008f840, filled by 0x1008b380 and added by AddMacro); Unknown3580 counts
// them, and the last one (or P1 = P2 = 0) ends with UpdateMacroShortCut(P5, P4)
void NConsoleWnd::ReceiveMacroListInfo(char P1, char P2, MacroInfo Macro, char P4, INT P5)
{
    if (Unknown357C)
    {
        Unknown3580 = 0;
        if (Desktop && nw::Field<void*>(Desktop, 0xF8))
        {
            INT Count = GetMacroCount();
            for (; Count > 0; Count--)
            {
                INT Last = GetMacroCount() - 1;
                if (GetMacro(Last))
                {
                    NMacroEntry* Old = GetMacro(Last);
                    if (Old)
                    {
                        Old->~NMacroEntry();                                         // its destructor (0x1008F910)
                        GMalloc->Free(Old);
                    }
                }
                DeskMacroList(this)->Macros.Remove(Last, 1);                         // TPtrArray::Remove (0x1001D1B0)
            }
            for (INT i = 0; i < 0x5FA0; i += 0x3FC)
            {
                appMemset(nw::Field<BYTE*>(Desktop, 0xF8) + 0x134 + i, 0, 10);
                appMemset(nw::Field<BYTE*>(Desktop, 0xF8) + 0xD8 + i, 0, 0x1A);
                appMemset(nw::Field<BYTE*>(Desktop, 0xF8) + 0xF2 + i, 0, 0x42);
                for (INT k = 0; k < 0x390; k += 0x4C)
                    appMemset(nw::Field<BYTE*>(Desktop, 0xF8) + i + 0x14A + k, 0, 0x42);
            }
        }
        if (!P1 && !P2)
        {
            UpdateMacroShortCut(P5, P4);
            return;
        }
        Unknown357C = 0;
    }
    void* Mem = GMalloc->Malloc(0xA54, TEXT("new"));
    NMacroEntry* Object = Mem ? ::new (Mem) NMacroEntry : NULL;                      // the macro object's constructor (0x1008F840)
    if (Object)
    {
        appMemcpy(nw::Field<BYTE*>(Desktop, 0xF8) + GetMacroCount() * 0x3FC + 0xD4,
                  &Macro, 0x3FC);
        NMacroSlot* Slot = (NMacroSlot*)&Macro;                                      // the slot by value: the id, then the data
        Object->Set(Slot->ID, Slot->Info);
        AddMacro(Object);
    }
    Unknown3580++;
    if (Unknown3580 < (INT)P1)
        return;
    Unknown357C = 1;
    UpdateMacroShortCut(P5, P4);
}
NW_HOOK_METHOD(0x10066d70, NConsoleWnd, ReceiveMacroListInfo)

// 0x10054610 (vslot 159) - the end of the macro list (and of its shortcuts): the shortcut bar's window takes it
// (0x100cc8d0, NCShortCutWnd::UpdateMacro)
void NConsoleWnd::UpdateMacroShortCut(INT P1, INT P2)
{
    NCWnd* ShortCut = DeskWnd(this, 0x104);
    if (ShortCut)
        ((NCShortCutWnd*)ShortCut)->UpdateMacro(P1, P2);
}
NW_HOOK_METHOD(0x10054610, NConsoleWnd, UpdateMacroShortCut)

// The macro runner (the macro window, +0xF8) is told what happens: its running macros (the list at +0xC4) that
// wait for the event (type 3 at their step) are let go, by a delay in the step (+0x90) of one second or a
// thousandth
// 0x10054780 (vslot 217) - a skill was used (1, 0) or cancelled (0, 1): 0x1008c450
void NConsoleWnd::MacroSkillUse(INT P1, INT P2)
{
    DeskMacroList(this)->OnSkillCoolTime(P1, P2);
}
NW_HOOK_METHOD(0x10054780, NConsoleWnd, MacroSkillUse)

// 0x100547c0 (vslot 218) - the target changed: 0x1008c3c0
void NConsoleWnd::MacroTargetSelected()
{
    DeskMacroList(this)->OnTargetDone();
}
NW_HOOK_METHOD(0x100547c0, NConsoleWnd, MacroTargetSelected)

// 0x100547a0 (vslot 219) - an item was picked up: 0x1008c330
void NConsoleWnd::MacroItemPickedUp()
{
    DeskMacroList(this)->OnPickUpDone();
}
NW_HOOK_METHOD(0x100547a0, NConsoleWnd, MacroItemPickedUp)

/*-----------------------------------------------------------------------------
    Henna: the dye lists (+0x14C the list, +0x150 the info of one), the inventory (+0xCC), the GM's views.
-----------------------------------------------------------------------------*/

// 0x10059440 (vslot 133) - the list of dyes the player can engrave starts over (P1 of them; the engrave
// flag +0xA4 is 0): the list window (+0x14C) takes the place of the info window (+0x150)
void NConsoleWnd::ReceiveHennaEquipList(INT P1, INT P2)
{
    DeskHennaList(this)->Clear();
    nw::Field<INT>(DeskWnd(this, 0x14C), 0xA8) = P1;
    nw::Field<INT>(DeskWnd(this, 0x14C), 0xA4) = 0;
    INT X = DeskWnd(this, 0x150)->ScreenX;
    INT Y = DeskWnd(this, 0x150)->ScreenY;
    DeskWnd(this, 0x150)->HideWindow();
    DeskWnd(this, 0x14C)->SetWindowRect(X, Y, DeskWnd(this, 0x14C)->LocalW, DeskWnd(this, 0x14C)->LocalH);
    DeskWnd(this, 0x14C)->ShowWindow();
    DeskWnd(this, 0x14C)->SetFocus();
}
NW_HOOK_METHOD(0x10059440, NConsoleWnd, ReceiveHennaEquipList)

// 0x10059720 (vslot 138) - the same for the dyes to remove (+0xA4 is 1)
void NConsoleWnd::ReceiveHennaUnequipList(INT P1, INT P2)
{
    DeskHennaList(this)->Clear();
    nw::Field<INT>(DeskWnd(this, 0x14C), 0xA8) = P1;
    nw::Field<INT>(DeskWnd(this, 0x14C), 0xA4) = 1;
    INT X = DeskWnd(this, 0x150)->ScreenX;
    INT Y = DeskWnd(this, 0x150)->ScreenY;
    DeskWnd(this, 0x150)->HideWindow();
    DeskWnd(this, 0x14C)->SetWindowRect(X, Y, DeskWnd(this, 0x14C)->LocalW, DeskWnd(this, 0x14C)->LocalH);
    DeskWnd(this, 0x14C)->ShowWindow();
    DeskWnd(this, 0x14C)->SetFocus();
}
NW_HOOK_METHOD(0x10059720, NConsoleWnd, ReceiveHennaUnequipList)

// What both lists take for a dye: the names and the icon (from the item data, or the henna data) and the
// five numbers of the HennaInfo
static FORCEINLINE void ClearHennaEntry(NHennaSendEntry& Entry)
{
    Entry.Name[0] = 0;
    Entry.IconName[0] = 0;
    Entry.Description[0] = 0;
    Entry.Unknown1400 = 0;
    Entry.Info0 = 0;
    Entry.ItemID = 0;
    Entry.Info8 = 0;
    Entry.InfoC = 0;
    Entry.Info10 = 0;
    Entry.Unknown1418 = 0;
}

static FORCEINLINE void SetHennaEntryNumbers(NHennaSendEntry& Entry, const HennaInfo& Info)
{
    Entry.Info0 = Info.HennaID;
    Entry.Info8 = Info.Param08;
    Entry.InfoC = Info.Param0C;
    Entry.ItemID = Info.ItemID;
    Entry.Info10 = Info.Param10;
}

// 0x100594f0 (vslot 134) - a dye to engrave (the list window +0x14C adds it, 0x1007aef0): its item's names
void NConsoleWnd::AddHennaEquipInfo(HennaInfo Info)
{
    BYTE* Item = (BYTE*)GL2GameData.GetItemData(Info.ItemID);
    if (!Item)
        return;
    NHennaSendEntry Entry;
    ClearHennaEntry(Entry);
    appStrcpy(Entry.Name, *nw::Field<L2FName>(Item, 0x40));
    appStrcpy(Entry.Description, *nw::Field<L2FName>(Item, 0x44));
    appStrcpy(Entry.IconName, *nw::Field<L2FName>(Item, 0x34));
    SetHennaEntryNumbers(Entry, Info);
    DeskHennaList(this)->AddEntry((const NHennaListEntry*)&Entry);   // NHennaSendEntry has the layout of NHennaListEntry
}
NW_HOOK_METHOD(0x100594f0, NConsoleWnd, AddHennaEquipInfo)

// 0x100597d0 (vslot 139) - a dye to remove: its henna data's texts (FStrings at +8, +0x14 and +0x2C)
void NConsoleWnd::AddHennaUnequipInfo(HennaInfo Info)
{
    BYTE* Henna = (BYTE*)GL2GameData.GetHennaData(Info.HennaID);
    if (!Henna)
        return;
    NHennaSendEntry Entry;
    ClearHennaEntry(Entry);
    appStrcpy(Entry.Name, **(FString*)(Henna + 0x8));
    appStrcpy(Entry.Description, **(FString*)(Henna + 0x2C));
    appStrcpy(Entry.IconName, **(FString*)(Henna + 0x14));
    SetHennaEntryNumbers(Entry, Info);
    DeskHennaList(this)->AddEntry((const NHennaListEntry*)&Entry);   // NHennaSendEntry has the layout of NHennaListEntry
}
NW_HOOK_METHOD(0x100597d0, NConsoleWnd, AddHennaUnequipInfo)

// 0x10059610 (vslot 135) - the info of a dye to engrave: the info window (+0x150) shows it (0x10079240) with
// P2 (+0x3524) and the engrave flag (+0x3520 is 0), in the place of the list window
void NConsoleWnd::ReceiveHennaItemInfo(HennaInfo Info, INT P2)
{
    DeskHennaList(this)->Clear();
    DeskWnd(this, 0x14C)->HideWindow();
    DeskHennaInfo(this)->ClearParams();
    nw::Field<INT>(DeskWnd(this, 0x150), 0x3524) = P2;
    nw::Field<INT>(DeskWnd(this, 0x150), 0x3520) = 0;
    DeskHennaInfo(this)->SetHenna((const NHennaParam*)&Info);   // HennaInfo has the layout of NHennaParam
    PlaceAtAndShow(DeskWnd(this, 0x150), DeskWnd(this, 0x14C));
}
NW_HOOK_METHOD(0x10059610, NConsoleWnd, ReceiveHennaItemInfo)

// 0x10059930 (vslot 140) - the same for a dye to remove (+0x3520 is 1)
void NConsoleWnd::ReceiveHennaUnequipItemInfo(HennaInfo Info, INT P2)
{
    DeskHennaList(this)->Clear();
    DeskWnd(this, 0x14C)->HideWindow();
    DeskHennaInfo(this)->ClearParams();
    nw::Field<INT>(DeskWnd(this, 0x150), 0x3524) = P2;
    nw::Field<INT>(DeskWnd(this, 0x150), 0x3520) = 1;
    DeskHennaInfo(this)->SetHenna((const NHennaParam*)&Info);   // HennaInfo has the layout of NHennaParam
    PlaceAtAndShow(DeskWnd(this, 0x150), DeskWnd(this, 0x14C));
}
NW_HOOK_METHOD(0x10059930, NConsoleWnd, ReceiveHennaUnequipItemInfo)

// 0x100596e0 (vslot 136) - the player's dyes: the inventory window (+0xCC) takes them over (0x100807a0) and the
// character window's child at +0x98 keeps the record (+0xCC)
void NConsoleWnd::ReceiveHennaInfo(HennaInfo Info, INT P2)
{
    GetDesktop()->InventoryWnd->ClearHennas();
    nw::Field<HennaInfo>(nw::Field<NCWnd*>(DeskWnd(this, 0x98), 0x98), 0xCC) = Info;
}
NW_HOOK_METHOD(0x100596e0, NConsoleWnd, ReceiveHennaInfo)

// 0x100544b0 (vslot 137) - the inventory window's call (0x10080250)
void NConsoleWnd::AddHennaInfo(INT P1, INT P2)
{
    GetDesktop()->InventoryWnd->AddHenna(P1, P2);
}
NW_HOOK_METHOD(0x100544b0, NConsoleWnd, AddHennaInfo)

// 0x100544d0 (vslot 143) - the GM's inventory window (+0x158): 0x10073020
void NConsoleWnd::GMAddHennaInfo(INT P1, INT P2)
{
    GetDesktop()->GMInventoryWnd->AddHenna(P1, P2);
}
NW_HOOK_METHOD(0x100544d0, NConsoleWnd, GMAddHennaInfo)

// 0x100598f0 (vslot 144) - the GM's view of the player's dyes: the inventory window takes them over
// (0x10073030) and the GM's detail window (+0x160) keeps the record (+0xCC)
void NConsoleWnd::GMReceiveHennaInfo(HennaInfo Info, INT P2)
{
    GetDesktop()->GMInventoryWnd->ClearHennas();
    nw::Field<HennaInfo>(DeskWnd(this, 0x160), 0xCC) = Info;
}
NW_HOOK_METHOD(0x100598f0, NConsoleWnd, GMReceiveHennaInfo)

/*-----------------------------------------------------------------------------
    The calculator, the party, the shortcut bar's pad, the snoop, the town map, the observer, the Christmas
    seal, the quit dialog.
-----------------------------------------------------------------------------*/

// 0x100559e0 (vslot 261) - the calculator: code 0x1129 shows its window (+0x9C) and the one inside it
void NConsoleWnd::ShowCalc(INT Code)
{
    if (Code == 0x1129)
    {
        DeskWnd(this, 0x9C)->ShowWindow();
        nw::Field<NCWnd*>(DeskWnd(this, 0x9C), 0x9C)->ShowWindow();
    }
}
NW_HOOK_METHOD(0x100559e0, NConsoleWnd, ShowCalc)

// 0x10052c20 (vslot 303) - a number the party window keeps (+0xA8), 0 without a desktop or a party window
INT NConsoleWnd::GetPartyMemberCount()
{
    if (Desktop)
    {
        NCWnd* Party = DeskWnd(this, 0xC0);
        if (Party)
            return nw::Field<INT>(Party, 0xA8);
    }
    return 0;
}
NW_HOOK_METHOD(0x10052c20, NConsoleWnd, GetPartyMemberCount)

// 0x10057770 (vslot 307) - the shortcut bar's pad, left half held or not (C1's SetPadShift1)
void NConsoleWnd::SetPadHoldL(BYTE bHold)
{
    GetDesktop()->ShortCutWnd->SetPadHoldL(bHold != 0);
}
NW_HOOK_METHOD(0x10057770, NConsoleWnd, SetPadHoldL)

// 0x10057790 (vslot 308) - the right half (C1's SetPadShift2)
void NConsoleWnd::SetPadHoldR(BYTE bHold)
{
    GetDesktop()->ShortCutWnd->SetPadHoldR(bHold != 0);
}
NW_HOOK_METHOD(0x10057790, NConsoleWnd, SetPadHoldR)

// 0x10055c20 (vslot 306) - a line a GM snoops (a player's chat): the snoop id and the name of the snooped player go
// to the snoop manager (+0x190, OpenSnoop 0x100b7550), and the line "name: text" in the color of its chat type
// (AddLine 0x100b74f0: the snoop id comes first, five arguments), unless the type is none of the known ones
void NConsoleWnd::Snoop(L2ParamStack& Params)
{
    INT Id = (INT)Params.Top();
    TCHAR* TargetName = (TCHAR*)Params.Top();
    Params.Top();
    INT Type = (INT)Params.Top();
    TCHAR* Name = (TCHAR*)Params.Top();
    TCHAR* Text = (TCHAR*)Params.Top();
    if (!Text)
        return;
    NCGMSnoopManager* Snoops = (NCGMSnoopManager*)DeskWnd(this, 0x190);
    Snoops->OpenSnoop(Id, TargetName);
    TCHAR Line[1024];
    _swprintf(Line, TEXT("%s: %s"), Name, Text);
    DWORD Color;
    if (Type == 1)
        Color = 0xFFFF7200;
    else if (Type == 3)
        Color = 0xFF00FF00;
    else if (Type == 4)
        Color = 0xFF7D77FF;
    else if (Type == 2)
        Color = 0xFFFF00FF;
    else if (Type == 8)
        Color = 0xFFEAA5F5;
    else if (Type == 9)
        Color = 0xFF77FF99;
    else if (Type == 0)
        Color = 0xFFFFFFFF;
    else
        return;
    Snoops->AddLine(Id, Line, Color, Type, 0);
}
NW_HOOK_METHOD(0x10055c20, NConsoleWnd, Snoop)

// 0x100577b0 (vslot 310) - the town map: three values, taken even when there is no window (+0x194) or the first
// is 0; the window takes them (0x10098f00) and shows
void NConsoleWnd::ShowTownMap(L2ParamStack& Params)
{
    void* P1 = Params.Top();
    void* P2 = Params.Top();
    void* P3 = Params.Top();
    NCWnd* Map = DeskWnd(this, 0x194);
    if (Map && P1)
    {
        static_cast<NTownMapWnd*>(Map)->SetMap((const TCHAR*)P1, (INT)P2, (INT)P3);
        DeskWnd(this, 0x194)->ShowWindow();
    }
}
NW_HOOK_METHOD(0x100577b0, NConsoleWnd, ShowTownMap)

// 0x10057810 (vslot 311) - the observer's window (+0x198) shows
void NConsoleWnd::ObserverStart()
{
    NCWnd* Observer = DeskWnd(this, 0x198);
    if (Observer)
        Observer->ShowWindow();
}
NW_HOOK_METHOD(0x10057810, NConsoleWnd, ObserverStart)

// 0x10057830 (vslot 312) - and hides
void NConsoleWnd::ObserverEnd()
{
    NCWnd* Observer = DeskWnd(this, 0x198);
    if (Observer)
        Observer->HideWindow();
}
NW_HOOK_METHOD(0x10057830, NConsoleWnd, ObserverEnd)

// 0x10057880 (vslot 313) - the pledge's power: the window at +0xA0 takes the three values (0x100721f0)
void NConsoleWnd::PledgePower(INT P1, INT P2, BYTE* P3)
{
    if (Desktop && DeskWnd(this, 0xA0))
        static_cast<NCEmpowerWnd*>(DeskWnd(this, 0xA0))->SetPower(P1, P2, P3);
}
NW_HOOK_METHOD(0x10057880, NConsoleWnd, PledgePower)

// 0x10061be0 (vslot 316) - the window was closed in the game: the dialog 0x1C asks (system message 0x7D) to
// quit, answered to the system menu; the global console's ShowDialog opens it (the stock calls it by the
// global, not by this)
void NConsoleWnd::ShowQuitDialog()
{
    NDialogInfo Info;
    Info.ID = 0x1C;
    Info.bMainBox = 1;
    Info.bInput = 0;
    Info.Owner = DeskWnd(this, 0xF0)->Entry;
    Info.SetSystemMessage(0x7D);
    GConsole->ShowDialog(&Info);
}
NW_HOOK_METHOD(0x10061be0, NConsoleWnd, ShowQuitDialog)

// 0x10001ad0 (vslot 318) - empty (the shared empty function)
// (declared inline in the header)

// 0x10057910 (vslot 319) - the Christmas seal: the window at +0x19C takes the value (0x100dfb40) and the desktop
// shows or hides it (0x1004c440)
void NConsoleWnd::ShowXMasSeal(INT P1)
{
    static_cast<NCXMasSealWnd*>(DeskWnd(this, 0x19C))->SetItem(P1);
    static_cast<NCConsole*>(Desktop)->ToggleXMasSeal();
}
NW_HOOK_METHOD(0x10057910, NConsoleWnd, ShowXMasSeal)
