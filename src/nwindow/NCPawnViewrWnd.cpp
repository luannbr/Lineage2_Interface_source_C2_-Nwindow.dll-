/*=============================================================================
    NCPawnViewrWnd.cpp - the pawn viewer's main window and its item pages. Stock:
    0x10087420 .. 0x10088108, 0x10088830 .. 0x10089102, 0x10089380 .. 0x100895e8,
    0x1008c2d0 .. 0x1008c3b8, 0x1008c750 .. 0x1008cd1d, 0x1008e230 .. 0x1008ee32 (the other
    PawnView classes sit in between); the item grid's window procedure is a body shared with
    the MSView / NpcView grids (hooked in NCNpcViewrWnd.cpp).
=============================================================================*/
#include "nwindow/NCPawnViewrWnd.h"
#include "nwindow/NCButton.h"
#include "nwindow/NCCheckBox.h"
#include "nwindow/NCComboBox.h"
#include "nwindow/NCEditBox.h"
#include "nwindow/NCFrameCtrl.h"
#include "nwindow/NCGDevice.h"
#include "nwindow/NCInventoryWnd.h"
#include "nwindow/NCTabControl.h"
#include "nwindow/NConsoleWnd.h"
#include "nwindow/NItemInfo.h"

#include <string.h>
#include <wchar.h>

// NWM_COMMAND ids (names ours)
enum
{
    NID_PAWNVIEW_FRAME = 0xF2,
    NID_PAWNVIEW_F3 = 0xF3,          // handled (vfn062), no control sends it
    NID_PAWNVIEW_GUI = 0xF4,
    NID_PAWNVIEW_TEXTLIST = 0xF5,    // each text page's list
    NID_PAWNVIEW_SEARCH = 0x102,
    NID_PAWNVIEW_VIEWLIST = 0x103,
    NID_PAWNVIEW_WEAPONTYPE = 0x114,
    NID_PAWNVIEW_USE = 0x115,
    NID_PAWNVIEW_SEARCHEDIT = 0x116,
    NID_PAWNVIEW_SPAWN = 0x117,      // third row, new in C2
    NID_PAWNVIEW_FLAG = 0x118,
    NID_PAWNVIEW_GOTO = 0x119,
    NID_PAWNVIEW_RANGEEDITA = 0x11A,
    NID_PAWNVIEW_RANGEEDITB = 0x11B,
    NID_PAWNVIEW_CATEGORY = 0xFE001A,
    NID_PAWNVIEW_PAGES = 0xFE002A,
};

// sysstring lines
enum
{
    STR_Gloves = 37,
    STR_Chest = 38,
    STR_Legs = 39,
    STR_Feet = 40,
    STR_Etc = 49,
    STR_Human = 170,
    STR_Elf = 171,
    STR_DarkElf = 172,
    STR_Orc = 173,
    STR_Dwarf = 174,
    STR_Warrior = 175,
    STR_Spellcaster = 176,
    STR_Male = 177,
    STR_Female = 178,
    STR_Headgear = 230,
    STR_Search = 420,
    STR_ViewList = 422,
    STR_Use = 443,
};

// item categories: the pages of CategoryTabs (names ours)
enum
{
    PAWNVIEW_Chest,
    PAWNVIEW_Legs,
    PAWNVIEW_Gloves,
    PAWNVIEW_Feet,
    PAWNVIEW_Headgear,
    PAWNVIEW_Etc,
    PAWNVIEW_NUM_CATEGORIES,
};

// the 18 default characters of the creation screen (0x101133b8, only the viewer reads it)
struct NDefaultCharacter
{
    INT Race;
    INT Sex;
    INT ClassID;
};
#if NW_HOOK_MODE
#define GDefaultCharacters (reinterpret_cast<const NDefaultCharacter*>(::nw::Addr(0x1017f378)))
#else
extern const NDefaultCharacter GDefaultCharacters[];
#endif

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

// The character chosen in the character window (0..19: the check lets one past the 19
// records through) puts the item on, or takes it off when it wears it already; a two-handed
// item frees both hands, a one-handed one the two-handed slot. The player's pawn is dressed
// as that character. Inline in both stock callers (name ours).
static FORCEINLINE void ToggleItem(NItemEntry* Item)
{
    NCPawnViewrWnd* Viewer = NPawnView::Viewer();
    INT Index = Viewer->CharSelectWnd->Selected;
    if (Index >= 0 && Index <= 19)
    {
        User& Character = Viewer->Characters[Index];
        if (Item->BodyPart == BODYSLOT_LRHand)
        {
            Character.Equipped[BODYSLOT_RHand] = 0;
            Character.Equipped[BODYSLOT_LHand] = 0;
        }
        else if (Item->BodyPart == BODYSLOT_RHand || Item->BodyPart == BODYSLOT_LHand)
        {
            Character.Equipped[BODYSLOT_LRHand] = 0;
        }
        INT& Worn = Character.Equipped[Item->BodyPart];
        Worn = (Worn == Item->ID) ? 0 : Item->ID;
        NPawnView::Viewer()->Characters[Index].Pawn = NPawnView::PlayerPawn();
        NPawnView::Viewer()->Characters[Index].SetPawnResource();
    }
}

// an item of the table as the pages show it: icon (black when the icon is missing) and name
static FORCEINLINE void FillItemInfo(NCObject* Owner, NItemInfo& Info, INT ItemID)
{
    if (GConsole->GetIconName(ItemID))
    {
        wcscpy(Info.IconName, GConsole->GetIconName(ItemID));
        Info.Icon = Owner->FindTexture(GConsole->GetIconName(ItemID));
    }
    if (GConsole->GetIconName(ItemID))
        Info.Icon = Owner->FindTexture(GConsole->GetIconName(ItemID));
    if (!Info.Icon)
        Info.Icon = Owner->FindTexture(TEXT("NWindow.BlackTexture"));
    if (GConsole->GetItemName(ItemID))
        wcscpy(Info.Name, GConsole->GetItemName(ItemID));
}

/*-----------------------------------------------------------------------------
    NCPawnView_InvenItemWnd.
-----------------------------------------------------------------------------*/

// 0x100affe0
NCPawnView_InvenItemWnd::NCPawnView_InvenItemWnd()
{
}
NW_HOOK_CTOR(0x100affe0, NCPawnView_InvenItemWnd)

// 0x100b0080
NCPawnView_InvenItemWnd::NCPawnView_InvenItemWnd(INT InColumns, INT InVisibleRows, INT InMaxItems, INT InCellSize)
    : NCInvenItemWnd(InColumns, InVisibleRows, InMaxItems, InCellSize)
{
}
NW_HOOK_CTOR_ARGS(0x100b0080, NCPawnView_InvenItemWnd, (INT A, INT B, INT C, INT D), (A, B, C, D))

// 0x100b0030
NW_HOOK_DTOR(0x100b0030, NCPawnView_InvenItemWnd)

// stock body shared with the skill viewer's and the NPC viewer's grids (0x100aa450, hooked as NCNpcView_InvenItemWnd's);
// C2 no longer refuses the messages of a disabled window here
INT NCPawnView_InvenItemWnd::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (NCInvenItemWnd::WindowProc(Msg, wParam, lParam))
        return 1;
    if (Msg == WM_LBUTTONDBLCLK)
        return OnLButtonDblClick(wParam, lParam) != 0;
    return 0;
}

// 0x100abbc0 (vslot 60) - the item under the mouse; with the mouse in the window the cursor is the carried item's icon
// while dragging, else the normal one (C2: new, the grids' mouse code used to be the inventory's)
INT NCPawnView_InvenItemWnd::OnMouseMove(WPARAM wParam, LPARAM lParam)
{
    guard(NCPawnView_InvenItemWnd::OnMouseMove);
    HoverIndex = HitTest(LOWORD(lParam), HIWORD(lParam));
    if (IsFocused())
    {
        if (!bDragging)
        {
            UnlockCursor();
            SetCursor(0, NULL);
        }
        else if (Selected >= 0 && Selected < Items.Num())
        {
            SetCursor(4, Items(Selected)->GetIcon());
            LockCursor();
        }
    }
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x100abbc0, NCPawnView_InvenItemWnd, OnMouseMove)

// The item let go outside the window (C2: new) is dropped into the world where the mouse points: the character's
// engine event OnDropItem gets a one-item record (name of the guard: the stock copies OnRButtonDown's).
// 0x100abcb0 (vslot 62)
INT NCPawnView_InvenItemWnd::OnLButtonUp(WPARAM wParam, LPARAM lParam)
{
    guard(NCPawnView_InvenItemWnd::OnRButtonDown);
    if (Selected < 0 || Selected >= Items.Num())
    {
        bDragging = 0;
        return 1;
    }
    if (bDragging && !bDblClicked)
    {
        NItemEntry* Entry = Items(Selected);
        if (!Entry)
        {
            bDragging = 0;
            return 1;
        }
        if (!IsInWindow(LOWORD(lParam), HIWORD(lParam)))
        {
            DropLocation = static_cast<UGameEngine*>(GConsole->Engine)->MouseHitLocation;
            // the engine's Item, 0x18 bytes here: no object id, the item id, not stackable, one of it
            struct { INT ObjectID; INT ItemID; UBOOL bStackable; INT Count; INT Unknown10; INT Unknown14; } Dropped;
            Dropped.ObjectID = 0;
            Dropped.ItemID = Entry->ID;
            Dropped.bStackable = 0;
            Dropped.Count = 1;
            Dropped.Unknown10 = 0;
            Dropped.Unknown14 = 0;
            NCPawnViewrWnd* Viewer = NPawnView::Viewer();
            static_cast<FL2NetNotify*>(GConsole->Engine)->OnDropItem(
                &Viewer->Characters[Viewer->CharSelectWnd->Selected], (Item*)&Dropped, DropLocation,
                FRotator(0, 0, 0));
        }
    }
    bDblClicked = 0;
    bDragging = 0;
    UnlockCursor();
    SetCursor(0, NULL);
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x100abcb0, NCPawnView_InvenItemWnd, OnLButtonUp)

// 0x100abe60 (vslot 63) - the item under the mouse goes on the chosen character, or off
INT NCPawnView_InvenItemWnd::OnRButtonDown(WPARAM wParam, LPARAM lParam)
{
    guard(NCPawnView_InvenItemWnd::OnRButtonDown);
    Selected = HitTest(LOWORD(lParam), HIWORD(lParam));
    if (Selected >= 0 && Selected < Items.Num())
        ToggleItem(Items(Selected));
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x100abe60, NCPawnView_InvenItemWnd, OnRButtonDown)

// 0x100aa4b0 (vslot 73) - as the right button
INT NCPawnView_InvenItemWnd::OnLButtonDblClick(WPARAM wParam, LPARAM lParam)
{
    guard(NCPawnView_InvenItemWnd::OnLButtonDblClick);
    OnRButtonDown(wParam, lParam);
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x100aa4b0, NCPawnView_InvenItemWnd, OnLButtonDblClick)

NW_CHECK_VTABLE(NCPawnView_InvenItemWnd, 0x10121060, 74)

/*-----------------------------------------------------------------------------
    NCPawnView_InvenTextWnd.
-----------------------------------------------------------------------------*/

// 0x100abb20
NCPawnView_InvenTextWnd::NCPawnView_InvenTextWnd()
{
}
NW_HOOK_CTOR(0x100abb20, NCPawnView_InvenTextWnd)

// 0x100abb70
NW_HOOK_DTOR(0x100abb70, NCPawnView_InvenTextWnd)

// 0x100ab580 (vslot 25) - a click on the list
INT NCPawnView_InvenTextWnd::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (NCWnd::WindowProc(Msg, wParam, lParam))
        return 1;
    if (Msg == NWM_COMMAND && wParam == NID_PAWNVIEW_TEXTLIST)
        return OnInvenItemList((INT)lParam) != 0;
    return 0;
}
NW_HOOK_METHOD(0x100ab580, NCPawnView_InvenTextWnd, WindowProc)

// 0x100ad0c0 (vslot 36) - the list (3000 lines, 14 shown), created without the visible bit
// (0x21: bits of unknown meaning); NCWnd's OnCreate is not called
INT NCPawnView_InvenTextWnd::OnCreate()
{
    guard(NCPawnView_InvenTextWnd::OnCreate);
    List = new NCListBox(3000, 14, 16);
    List->ID = NID_PAWNVIEW_TEXTLIST;
    List->SetWindowRect(20, 20, 150, 180);
    CreateChild(List, 0x21);
    List->TextX = 1;
    List->TextY = 1;
    List->LineGap = 1;
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x100ad0c0, NCPawnView_InvenTextWnd, OnCreate)

// 0x100acf60 (vslot 67) - the line clicked (its param is its item) goes on the chosen character, or off; the line is
// read from the viewer's list of the page shown (not this window's own)
INT NCPawnView_InvenTextWnd::OnInvenItemList(INT Index)
{
    guard(NCPawnView_InvenTextWnd::OnInvenItemList);
    NCPawnViewrWnd* Viewer = NPawnView::Viewer();
    ToggleItem((NItemEntry*)Viewer->TextWnds[Viewer->CurrentPage]->List->GetItemParam(Index));
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x100acf60, NCPawnView_InvenTextWnd, OnInvenItemList)

NW_CHECK_VTABLE(NCPawnView_InvenTextWnd, 0x101204d8, 68)

/*-----------------------------------------------------------------------------
    NCPawnViewrWnd.
-----------------------------------------------------------------------------*/

// 0x100abfe0 - the nineteen records are built by their constructors; the grid texture, the first seven
// grids and the level are cleared
NCPawnViewrWnd::NCPawnViewrWnd() : CategoryTabs(NULL), GridBackTex(NULL)
{
    for (INT i = 0; i < 7; i++)
        ItemWnds[i] = NULL;
    Level = NULL;
}
NW_HOOK_CTOR(0x100abfe0, NCPawnViewrWnd)

// 0x100ac0b0
NW_HOOK_DTOR(0x100ac0b0, NCPawnViewrWnd)

// 0x100aa540 (vslot 25) - C2: no IsEnabled test; the third row's buttons (0x117..0x119) and the Enter keys of
// the two new edit lines (0x116, 0x11B) are handled
INT NCPawnViewrWnd::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (NCWnd::WindowProc(Msg, wParam, lParam))
        return 1;
    if (Msg == NWM_COMMAND)
    {
        if (wParam == NID_PAWNVIEW_CATEGORY)
            return OnCategoryTab((INT)lParam) != 0;
        if (wParam == NID_PAWNVIEW_F3)
            return vfn062() != 0;
        if (wParam == NID_PAWNVIEW_GUI)
            return OnGUICheck() != 0;
        if (wParam == NID_PAWNVIEW_SEARCH)
            return OnSearch() != 0;
        if (wParam == NID_PAWNVIEW_VIEWLIST)
            return OnViewList() != 0;
        if (wParam == NID_PAWNVIEW_USE)
            return OnUseWeapon() != 0;
        if (wParam == NID_PAWNVIEW_SPAWN)
            return OnSpawnCharacter() != 0;
        if (wParam == NID_PAWNVIEW_FLAG)
            return OnToggleFlag() != 0;
        if (wParam == NID_PAWNVIEW_GOTO)
            return OnGoToMap() != 0;
        if (wParam == NID_PAWNVIEW_SEARCHEDIT)
            return OnSearchEdit((INT)lParam) != 0;
        if (wParam == NID_PAWNVIEW_RANGEEDITB)
            return OnRangeEdit((INT)lParam) != 0;
    }
    return 0;
}
NW_HOOK_METHOD(0x100aa540, NCPawnViewrWnd, WindowProc)

// 0x100b19a0 (vslot 36) - title bar, the three pages and their tabs, a grid and a text list per category (C2: a seventh
// pair for the search results) and the category tabs (a seventh "search"), "GUI", the character window (registered at
// the top level), the weapon type and "Use", the search line, "Search" and "View List", and C2's third row (the
// "summon" and "attack mode" buttons, the two map number lines and "change map"); then the items, the characters
// and the hair window's values. NCWnd's OnCreate is not called.
INT NCPawnViewrWnd::OnCreate()
{
    guard(NCPawnViewrWnd::OnCreate);
    for (INT i = 0; i < 8; i++)
        DefaultWeapons[i] = -1;

    FrameCtrl = new NCFrameCtrl(1);
    FrameID = NID_PAWNVIEW_FRAME;
    FrameCtrl->ID = NID_PAWNVIEW_FRAME;
    FrameCtrl->SetWindowRect(0, 19, Width, 25);
    CreateChild(FrameCtrl, NWS_VISIBLE);
    GridBackTex = FindTexture(TEXT("L2UI.QuestWnd.questBack3"));
    BackTex = FindTexture(TEXT("L2UI.etcWnd.OptionBack"));

    MixAnimWnd = new NCPawnViewr_MixAnimWnd;
    MixAnimWnd->SetWindowRect(0, 53, 240, 236);
    CreateChild(MixAnimWnd, NWS_VISIBLE);
    HairChangeWnd = new NCPawnViewr_HairChangeWnd;
    HairChangeWnd->SetWindowRect(0, 53, 240, 236);
    CreateChild(HairChangeWnd, NWS_VISIBLE);
    Simulator = new NCPawnViewr_Simulator;
    Simulator->SetWindowRect(0, 53, 240, 236);
    CreateChild(Simulator, NWS_VISIBLE);

    // "combined skin", "hair mesh", "skin test"
    PageTabs = new NCTabControl;
    PageTabs->ID = NID_PAWNVIEW_PAGES;
    PageTabs->SetWindowRect(15, 42, 207, 16);
    CreateChild(PageTabs, NWS_VISIBLE);
    PageTabs->AddTab(TEXT("\xC870\xD569\xC2A4\xD0AC"), MixAnimWnd, 69, 16,
                     TEXT("L2UI.NWindow.Tab2"), TEXT("L2UI.NWindow.Tab1"), 0, -1);
    PageTabs->AddTab(TEXT("\xD5E4\xC5B4\xBA54\xC26C"), HairChangeWnd, 69, 16,
                     TEXT("L2UI.NWindow.Tab2"), TEXT("L2UI.NWindow.Tab1"), 0, -1);
    PageTabs->AddTab(TEXT("\xC2A4\xD0AC\xD14C\xC2A4\xD2B8"), Simulator, 69, 16,
                     TEXT("L2UI.NWindow.Tab2"), TEXT("L2UI.NWindow.Tab1"), 0, -1);

    for (INT i = 0; i < PAWNVIEW_NUM_CATEGORIES; i++)
    {
        ItemWnds[i] = new NCPawnView_InvenItemWnd(6, 4, 80, 32);
        ItemWnds[i]->SetWindowRect(16, 319, 218, 150);
        CreateChild(ItemWnds[i], 0x31);   // 0x31: bits of unknown meaning, not visible
        TextWnds[i] = new NCPawnView_InvenTextWnd;
        TextWnds[i]->SetWindowRect(10, 293, 230, 190);
        CreateChild(TextWnds[i], 0x11);
    }
    // the seventh pair (the search results), written out in the stock code
    ItemWnds[6] = new NCPawnView_InvenItemWnd(6, 4, 80, 32);
    ItemWnds[6]->SetWindowRect(16, 319, 218, 150);
    CreateChild(ItemWnds[6], 0x31);
    TextWnds[6] = new NCPawnView_InvenTextWnd;
    TextWnds[6]->SetWindowRect(10, 293, 230, 190);
    CreateChild(TextWnds[6], 0x11);

    CategoryTabs = new NCTabControl;
    CategoryTabs->ID = NID_PAWNVIEW_CATEGORY;
    CategoryTabs->SetWindowRect(5, 290, 193, 20);
    CreateChild(CategoryTabs, NWS_VISIBLE);
    CategoryTabs->AddTab(STR_Chest, NULL, 27, 20, TEXT("L2UI.NWindow.Tab2"),
                         TEXT("L2UI.NWindow.Tab1"), 0, -1);
    CategoryTabs->AddTab(STR_Legs, NULL, 27, 20, TEXT("L2UI.NWindow.Tab2"),
                         TEXT("L2UI.NWindow.Tab1"), 0, -1);
    CategoryTabs->AddTab(STR_Gloves, NULL, 27, 20, TEXT("L2UI.NWindow.Tab2"),
                         TEXT("L2UI.NWindow.Tab1"), 0, -1);
    CategoryTabs->AddTab(STR_Feet, NULL, 27, 20, TEXT("L2UI.NWindow.Tab2"),
                         TEXT("L2UI.NWindow.Tab1"), 0, -1);
    CategoryTabs->AddTab(STR_Headgear, NULL, 27, 20, TEXT("L2UI.NWindow.Tab2"),
                         TEXT("L2UI.NWindow.Tab1"), 0, -1);
    CategoryTabs->AddTab(STR_Etc, NULL, 27, 20, TEXT("L2UI.NWindow.Tab2"),
                         TEXT("L2UI.NWindow.Tab1"), 0, -1);
    CategoryTabs->AddTab(TEXT("\xAC80\xC0C9"), NULL, 27, 20, TEXT("L2UI.NWindow.Tab2"),
                         TEXT("L2UI.NWindow.Tab1"), 0, -1);

    GUICheck = new NCCheckBox(TEXT("GUI"), 0, 0);
    GUICheck->ID = NID_PAWNVIEW_GUI;
    GUICheck->SetWindowRect(195, 294, 39, 16);
    CreateChild(GUICheck, NWS_VISIBLE);
    GUICheck->bChecked = 1;

    for (INT i = 0; i < 19; i++)
        Characters[i].Pawn = NULL;

    CharSelectWnd = new NCPawnView_CharacterSelectWnd;
    CharSelectWnd->SetWindowRect(34, 34, 234, 503);
    CreateTopWindow(CharSelectWnd, NWS_VISIBLE | NWS_NOHITTEST);

    WeaponTypeCombo = new NCComboBox;
    WeaponTypeCombo->ID = NID_PAWNVIEW_WEAPONTYPE;
    WeaponTypeCombo->SetWindowRect(25, 540, 70, 16);
    CreateChild(WeaponTypeCombo, NWS_VISIBLE);
    WeaponTypeCombo->AddItem(TEXT("0(HAND)"));
    WeaponTypeCombo->AddItem(TEXT("1(1HS)"));
    WeaponTypeCombo->AddItem(TEXT("2(2HS)"));
    WeaponTypeCombo->AddItem(TEXT("3(DUAL)"));
    WeaponTypeCombo->AddItem(TEXT("4(POLE)"));
    WeaponTypeCombo->AddItem(TEXT("5(BOW)"));
    WeaponTypeCombo->AddItem(TEXT("7(DUALFIST)"));
    UseButton = new NCButton(TEXT("L2UI.Control.DefaultButton"),
                             TEXT("L2UI.Control.DefaultButton_click"), 0);
    UseButton->ID = NID_PAWNVIEW_USE;
    UseButton->SetWindowRect(100, 540, 28, 16);
    CreateChild(UseButton, NWS_VISIBLE);
    UseButton->SetTextID(STR_Use);

    SearchEdit = new NCEditBox(1, NULL, 1, 64, 0);
    SearchEdit->ID = NID_PAWNVIEW_SEARCHEDIT;
    SearchEdit->SetWindowRect(25, 520, 60, 16);
    CreateChild(SearchEdit, NWS_VISIBLE);
    SearchButton = new NCButton(TEXT("L2UI.Control.DefaultButton"),
                                TEXT("L2UI.Control.DefaultButton_click"), 0);
    SearchButton->ID = NID_PAWNVIEW_SEARCH;
    SearchButton->SetWindowRect(95, 520, 28, 16);
    CreateChild(SearchButton, NWS_VISIBLE);
    SearchButton->SetText(TEXT("\xCC3E\xAE30"));       // "find"
    ListButton = new NCButton(TEXT("L2UI.Control.DefaultButton"),
                              TEXT("L2UI.Control.DefaultButton_click"), 0);
    ListButton->ID = NID_PAWNVIEW_VIEWLIST;
    ListButton->SetWindowRect(130, 520, 50, 16);
    CreateChild(ListButton, NWS_VISIBLE);
    ListButton->SetTextID(STR_ViewList);

    // the third row (C2)
    Button108 = new NCButton(TEXT("L2UI.Control.DefaultButton"),
                             TEXT("L2UI.Control.DefaultButton_click"), 0);
    Button108->ID = NID_PAWNVIEW_SPAWN;
    Button108->SetWindowRect(25, 500, 28, 16);
    CreateChild(Button108, NWS_VISIBLE);
    Button108->SetText(TEXT("\xC18C\xD658"));           // "summon"
    Button10C = new NCButton(TEXT("L2UI.Control.DefaultButton"),
                             TEXT("L2UI.Control.DefaultButton_click"), 0);
    Button10C->ID = NID_PAWNVIEW_FLAG;
    Button10C->SetWindowRect(70, 500, 50, 16);
    CreateChild(Button10C, NWS_VISIBLE);
    Button10C->SetText(TEXT("\xACF5\xACA9\xBAA8\xB4DC"));  // "attack mode"
    RangeEditA = new NCEditBox(3, NULL, 1, 64, 0);
    RangeEditA->ID = NID_PAWNVIEW_RANGEEDITA;
    RangeEditA->SetWindowRect(25, 560, 23, 16);
    CreateChild(RangeEditA, NWS_VISIBLE);
    RangeEditB = new NCEditBox(3, NULL, 1, 64, 0);
    RangeEditB->ID = NID_PAWNVIEW_RANGEEDITB;
    RangeEditB->SetWindowRect(60, 560, 23, 16);
    CreateChild(RangeEditB, NWS_VISIBLE);
    Button110 = new NCButton(TEXT("L2UI.Control.DefaultButton"),
                             TEXT("L2UI.Control.DefaultButton_click"), 0);
    Button110->ID = NID_PAWNVIEW_GOTO;
    Button110->SetWindowRect(110, 560, 42, 16);
    CreateChild(Button110, NWS_VISIBLE);
    Button110->SetText(TEXT("\xB9F5\xBCC0\xACBD"));     // "change map"
    RangeEditA->SetTabOrder(RangeEditB->Entry, NULL);

    AddAllItem();
    AddAllCharacter();
    CharSelectWnd->OnNameComboBox(0);
    OnCategoryTab(0);
    PageTabs->SetTab(0);
    HairChangeWnd->SyncFromCharacter();
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x100b19a0, NCPawnViewrWnd, OnCreate)

// 0x100ac9b0 (vslot 36) - the character window and two pages are deleted without leaving
// the window manager; the text pages, the simulator, the page tabs, the search line, its
// buttons, the weapon combo and "Use" are left alone
INT NCPawnViewrWnd::OnDestroy()
{
    guard(NCPawnViewrWnd::OnDestroy);
    for (INT i = 0; i < PAWNVIEW_NUM_CATEGORIES; i++)
        DestroyChild(this, ItemWnds[i]);
    DestroyChild(this, GUICheck);
    DestroyChild(this, CategoryTabs);
    DestroyChild(this, UnknownF8);
    DestroyChild(this, FrameCtrl);
    if (CharSelectWnd)
        delete CharSelectWnd;
    if (MixAnimWnd)
        delete MixAnimWnd;
    if (HairChangeWnd)
        delete HairChangeWnd;
    NCWnd::OnDestroy();
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x100ac9b0, NCPawnViewrWnd, OnDestroy)

// 0x100acc10 (vslot 43) - the character window shows too; only the current page's grid (or
// list) stays up
void NCPawnViewrWnd::ShowWindow()
{
    guard(NCPawnViewrWnd::ShowWindow);
    if (CharSelectWnd)
        CharSelectWnd->ShowWindow();
    OnGUICheck();
    NCWnd::ShowWindow();
    for (INT i = 0; i < PAWNVIEW_NUM_CATEGORIES; i++)
    {
        if (CurrentPage != i || !GUICheck->bChecked)
            ItemWnds[i]->HideWindow();
        if (CurrentPage != i || GUICheck->bChecked)
            TextWnds[i]->HideWindow();
    }
    CategoryTabs->ShowCurrentPage();
    PageTabs->ShowCurrentPage();
    unguard;
}
NW_HOOK_METHOD(0x100acc10, NCPawnViewrWnd, ShowWindow)

// 0x100ab320 (vslot 44)
void NCPawnViewrWnd::HideWindow()
{
    guard(NCPawnViewrWnd::HideWindow);
    if (CharSelectWnd)
        CharSelectWnd->HideWindow();
    NCWnd::HideWindow();
    unguard;
}
NW_HOOK_METHOD(0x100ab320, NCPawnViewrWnd, HideWindow)

// 0x100ac760 (vslot 55) - the background of the item part (the grids' or the lists'), then the upper part and the
// bottom of the option background; C2 adds the "_" and ".unr" between the two map edit lines (y 560); the origin
// goes on before the clip
INT NCPawnViewrWnd::OnPaint(NCGDevice* Device)
{
    guard(NCPawnViewrWnd::OnPaint);
    SIZE Size;
    Device->BeginPaint(ScreenX, ScreenY);
    Device->SetClip(ScreenX, ScreenY, Width, Height);
    UTexture* ItemBack = GUICheck->bChecked ? GridBackTex : BackTex;
    if (ItemBack)
        Device->DrawTexture(0, 289, 240, 185, 0, 0, 240, 185, ItemBack);
    if (BackTex)
    {
        Device->DrawTexture(0, 40, 240, 249, 0, 0, 240, 249, BackTex);
        Device->DrawTexture(0, 474, 240, 125, 0, 219, 240, 125, BackTex);
    }
    NCGDevice::GetTextExtent(&Size, TEXT("_"), 0, 0, 0, 0, 0);
    Device->DrawText(50, (16 - Size.cy) / 2 + 560, 0xFFFFFFFF, TEXT("_"), 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
    NCGDevice::GetTextExtent(&Size, TEXT(".unr"), 0, 0, 0, 0, 0);
    Device->DrawText(84, (16 - Size.cy) / 2 + 560, 0xFFFFFFFF, TEXT(".unr"), 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
    Device->EndPaint();
    Device->ReleaseClip();
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x100ac760, NCPawnViewrWnd, OnPaint)

// C2 CANDIDATE: the C2 function is not the same code as the C1 one; kept only while it matches the stock one (verify_data)
// 0x100ac100 (vslot 68) - a category: its grid, or its list (the text page itself does not
// show: its ShowWindow is empty)
INT NCPawnViewrWnd::OnCategoryTab(INT Index)
{
    HidePages();
    if (GUICheck->bChecked)
    {
        if (Index >= 0 && Index <= 6 && ItemWnds[Index])
        {
            CurrentPage = Index;
            ItemWnds[Index]->ShowWindow();
        }
    }
    else if (Index >= 0 && Index <= 6 && ItemWnds[Index])
    {
        CurrentPage = Index;
        TextWnds[Index]->List->ShowWindow();
        TextWnds[CurrentPage]->ShowWindow();
    }
    return 1;
}
NW_HOOK_METHOD(0x100ac100, NCPawnViewrWnd, OnCategoryTab)

// 0x100ac940 (vslot 69) - the current category again, as a grid or as a list
INT NCPawnViewrWnd::OnGUICheck()
{
    HidePages();
    if (GUICheck->bChecked)
    {
        ItemWnds[CurrentPage]->ShowWindow();
        return 1;
    }
    TextWnds[CurrentPage]->List->ShowWindow();
    TextWnds[CurrentPage]->ShowWindow();
    return 1;
}
NW_HOOK_METHOD(0x100ac940, NCPawnViewrWnd, OnGUICheck)

// the item id of a record (+0x0C; the header leaves it as filler)
static FORCEINLINE INT RecordItemID(NViewItemData* Data)
{
    return *(INT*)((BYTE*)Data + 0x0C);
}

// 0x100b01d0 (vslot 70) - C2: the matches go to the extra page 6 (its grid and its list), and the current category
// shown gets them again; the page of the category in view, the extra one and the lists are emptied first. The
// walk stops at the first empty record
INT NCPawnViewrWnd::OnSearch()
{
    const TCHAR* Text = SearchEdit->CommitText();
    HidePages();
    if (!GUICheck->bChecked)
    {
        TextWnds[CurrentPage]->List->ShowWindow();
        TextWnds[CurrentPage]->ShowWindow();
    }
    else
        ItemWnds[CurrentPage]->ShowWindow();
    ItemWnds[CurrentPage]->Clear();
    TextWnds[CurrentPage]->List->Clear();
    ItemWnds[6]->Clear();
    TextWnds[6]->List->Clear();
    if (Text)
    {
        NDataMap<NViewItemData>* Map = GL2GameData.ItemTable;
        for (INT i = 0; i < Map->Pairs.Num(); i++)
        {
            NViewItemData* Data = Map->Pairs(i).Value;
            NItemInfo Info;
            if (!Data)
                return 1;
            if (appStrstr(*Data->Name, Text))
            {
                FillItemInfo(Data, &Info);
                AddItem(6, &Info);
                ItemWnds[6]->MaxItems++;
                INT Page = GetItemCategory(Data);
                if (CurrentPage == Page)
                {
                    AddItem(Page, &Info);
                    ItemWnds[Page]->MaxItems++;
                }
            }
        }
    }
    return 1;
}
NW_HOOK_METHOD(0x100b01d0, NCPawnViewrWnd, OnSearch)

// 0x100ab490 (vslot 73) - Enter in the search line searches
INT NCPawnViewrWnd::OnSearchEdit(INT Code)
{
    if (Code == 1)
        OnSearch();
    return 1;
}
NW_HOOK_METHOD(0x100ab490, NCPawnViewrWnd, OnSearchEdit)

// 0x100ab4b0 (vslot 74) - Enter in the second map line goes
INT NCPawnViewrWnd::OnRangeEdit(INT Code)
{
    if (Code == 1)
        OnGoToMap();
    return 1;
}
NW_HOOK_METHOD(0x100ab4b0, NCPawnViewrWnd, OnRangeEdit)

// 0x100ac1a0 (vslot 75) - "summon": the chosen default character (the character window's name combo) spawned where the
// player stands, with the player's rotation and a player controller of its own, the record's resources set; then moved
// down onto the ground by the box trace of NCNpcView_CharacterSelectWnd::OnSummon (30 tries; a hit raises a very close
// one to the middle of 1.9 .. 2.4). Nothing checks that the pawn was spawned; its spawn event is not run
INT NCPawnViewrWnd::OnSpawnCharacter()
{
    APlayerController* Viewer = ViewActor();
    if (!Viewer || !Viewer->Pawn || !Viewer->XLevel)
        return 1;
    INT Index = NPawnView::Viewer()->CharSelectWnd->NameCombo->Selected;
    FVector Location = ViewActor()->Pawn->Location;
    ULevel* Level = ViewActor()->XLevel;
    AController* Controller = (AController*)Level->SpawnActor(
        UObject::StaticLoadClass(AController::StaticClass(), NULL, TEXT("Engine.PlayerController"), NULL, 1, NULL),
        NAME_None, Location, FRotator(0, 0, 0), NULL, 1, 1);
    UClass* PawnClass = UObject::StaticLoadClass(APawn::StaticClass(), NULL,
                                                 NPawnView::Viewer()->Characters[Index].GetClassNamePointer(), NULL, 0, NULL);
    if (!PawnClass)
        return 1;
    APawn* Pawn = (APawn*)Level->SpawnActor(PawnClass, NAME_None, Location, ViewActor()->Pawn->Rotation, NULL, 1, 0,
                                            Controller);
    Controller->Pawn = Pawn;
    Pawn->Controller = Controller;
    Controller->Role = ROLE_Authority;
    Controller->Pawn->Role = ROLE_Authority;
    Controller->Pawn->setPhysics(1, NULL, FVector(0.f, 0.f, 1.f));
    FVector Start = Location;
    FVector End = Location;
    Start.Z += 50.0f;
    User Record;
    appMemcpy(&Record, &NPawnView::Viewer()->Characters[Index], sizeof(User));
    Record.Pawn = Controller->Pawn;
    Record.SetPawnResource();
    for (INT i = 0; i < 30; i++)
    {
        Start.Z += 30.0f;
        End.Z -= 30.0f;
        FCheckResult Hit(1.0f);
        APawn* Summoned = Controller->Pawn;
        Level->SingleLineCheck(Hit, Summoned, End, Start, 0x86,
                               FVector(Summoned->CollisionRadius, Summoned->CollisionRadius,
                                       Summoned->CollisionHeight));
        if (Hit.Location.Z != 0.0f)
        {
            DOUBLE Step = Hit.Time * 12.0;
            if (Step < 1.9f)
                Hit.Location.Z = (FLOAT)(((DOUBLE)2.4f + 1.9f) * 0.5f - Step + Hit.Location.Z);
            Level->FarMoveActor(Controller->Pawn, Hit.Location, 0, 0, 0);
            break;
        }
    }
    return 1;
}
NW_HOOK_METHOD(0x100ac1a0, NCPawnViewrWnd, OnSpawnCharacter)

// 0x100ac590 (vslot 76) - flips bit 2 of the player controller's flags (+0x3F0)
INT NCPawnViewrWnd::OnToggleFlag()
{
    ViewActor()->Unknown638 ^= 2;
    return 1;
}
NW_HOOK_METHOD(0x100ac590, NCPawnViewrWnd, OnToggleFlag)

// 0x100ac5c0 (vslot 77) - the two edit lines as map coordinates (each under 100): a seamless world teleports the
// player to the middle of that map block, the other opens the map "<x>_<y>.unr"
INT NCPawnViewrWnd::OnGoToMap()
{
    TCHAR Command[128];
    if (RangeEditA->CommitText() && RangeEditB->CommitText())
    {
        APlayerController* Controller = ViewActor();
        if (Controller && Controller->Pawn && Controller->XLevel)
        {
            INT X = appAtoi(RangeEditA->CommitText());
            INT Y = appAtoi(RangeEditB->CommitText());
            if (X < 100 && Y < 100)
            {
                if (GIsL2Seamless)
                {
                    static_cast<UGameEngine*>(GConsole->Engine)->L2_Teleport(
                        ViewActor()->Pawn,
                        FVector(((FLOAT)(X - 9) * 32768.0f - 360448.0f) + 16384.0f,
                                ((FLOAT)(Y - 9) * 32768.0f - 294912.0f) + 16384.0f, -1000.0f),
                        0, 1, 0);
                    return 1;
                }
                appSprintf(Command, TEXT("open %02d_%02d.unr"), X, Y);
                static_cast<FExec*>(GConsole->Engine)->Exec(Command, *GLog);
            }
        }
    }
    return 1;
}
NW_HOOK_METHOD(0x100ac5c0, NCPawnViewrWnd, OnGoToMap)

// 0x100b03b0 (vslot 71) - every category refilled from the item table (the items of the
// lists' lines are not freed)
INT NCPawnViewrWnd::OnViewList()
{
    Clear();
    for (INT i = 0; i < PAWNVIEW_NUM_CATEGORIES; i++)
        TextWnds[i]->List->Clear();
    AddAllItem();
    return 1;
}
NW_HOOK_METHOD(0x100b03b0, NCPawnViewrWnd, OnViewList)

// 0x100afa30 (vslot 67) - the chosen character takes the first weapon found of the type
// picked ("7(DUALFIST)", the seventh line, is type 7); bare hands for type 0 or none found
INT NCPawnViewrWnd::OnUseWeapon()
{
    INT Type = WeaponTypeCombo->Selected;
    INT Index = CharSelectWnd->Selected;
    if (Index < 0 || Index > 19)
        return 1;
    if (Type == 6)
        Type = 7;
    User& Character = Characters[Index];
    NViewItemData* Data = NItemTable::Find(&DefaultWeapons[Type]);
    if (Data && Type != 0)
    {
        if (Data->BodyPart == BODYSLOT_LRHand)
        {
            Character.Equipped[BODYSLOT_RHand] = 0;
            Character.Equipped[BODYSLOT_LHand] = 0;
            Character.Equipped[Data->BodyPart] = DefaultWeapons[Type];
        }
        else
        {
            if (Data->BodyPart == BODYSLOT_RHand || Data->BodyPart == BODYSLOT_LHand)
                Character.Equipped[BODYSLOT_LRHand] = 0;
            Character.Equipped[Data->BodyPart] = DefaultWeapons[Type];
        }
    }
    else
    {
        Character.Equipped[BODYSLOT_RHand] = 0;
        Character.Equipped[BODYSLOT_LHand] = 0;
        Character.Equipped[BODYSLOT_LRHand] = 0;
    }
    Character.Pawn = NPawnView::PlayerPawn();
    Character.SetPawnResource();
    return 1;
}
NW_HOOK_METHOD(0x100afa30, NCPawnViewrWnd, OnUseWeapon)

// 0x100aa6e0 - every category's grid and list hidden
__declspec(noinline) void NCPawnViewrWnd::HidePages()
{
    for (INT i = 0; i < PAWNVIEW_NUM_CATEGORIES; i++)
    {
        ItemWnds[i]->HideWindow();
        TextWnds[i]->HideWindow();
        TextWnds[i]->List->HideWindow();
    }
}
NW_HOOK_METHOD(0x100aa6e0, NCPawnViewrWnd, HidePages)

// "<race><gender><class type>" in the character window's combo box
static FORCEINLINE void AddCharacterName(NCPawnViewrWnd* Viewer, TCHAR* Text, INT Race,
                                         INT Sex, INT Type)
{
    appSprintf(Text, TEXT("%s%s%s"), *GL2GameData.SysString[Race], *GL2GameData.SysString[Sex],
               *GL2GameData.SysString[Type]);
    Viewer->CharSelectWnd->NameCombo->AddItem(Text);
}

// 0x100aa720 - the records emptied and given the creation screen's race, gender and class
// (the 19th reads past the 18 of the table), and the 18 names in the character window
__declspec(noinline) void NCPawnViewrWnd::AddAllCharacter()
{
    guard(NCPawnViewrWnd::AddAllCharacter);
    TCHAR Text[100];
    for (INT i = 0; i < 19; i++)
    {
        memset(&Characters[i], 0, sizeof(User));
        Characters[i].Race = GDefaultCharacters[i].Race;
        Characters[i].Sex = GDefaultCharacters[i].Sex;
        Characters[i].ClassID = GDefaultCharacters[i].ClassID;
    }
    AddCharacterName(this, Text, STR_Human, STR_Male, STR_Warrior);
    AddCharacterName(this, Text, STR_Human, STR_Female, STR_Warrior);
    AddCharacterName(this, Text, STR_Human, STR_Male, STR_Spellcaster);
    AddCharacterName(this, Text, STR_Human, STR_Female, STR_Spellcaster);
    AddCharacterName(this, Text, STR_Elf, STR_Male, STR_Warrior);
    AddCharacterName(this, Text, STR_Elf, STR_Female, STR_Warrior);
    AddCharacterName(this, Text, STR_Elf, STR_Male, STR_Spellcaster);
    AddCharacterName(this, Text, STR_Elf, STR_Female, STR_Spellcaster);
    AddCharacterName(this, Text, STR_DarkElf, STR_Male, STR_Warrior);
    AddCharacterName(this, Text, STR_DarkElf, STR_Female, STR_Warrior);
    AddCharacterName(this, Text, STR_DarkElf, STR_Male, STR_Spellcaster);
    AddCharacterName(this, Text, STR_DarkElf, STR_Female, STR_Spellcaster);
    AddCharacterName(this, Text, STR_Orc, STR_Male, STR_Warrior);
    AddCharacterName(this, Text, STR_Orc, STR_Female, STR_Warrior);
    AddCharacterName(this, Text, STR_Orc, STR_Male, STR_Spellcaster);
    AddCharacterName(this, Text, STR_Orc, STR_Female, STR_Spellcaster);
    AddCharacterName(this, Text, STR_Dwarf, STR_Male, STR_Warrior);
    AddCharacterName(this, Text, STR_Dwarf, STR_Female, STR_Warrior);
    unguard;
}
NW_HOOK_METHOD(0x100aa720, NCPawnViewrWnd, AddAllCharacter)

// 0x100aaf50 - the page of an item record: armor by its body part, the rest under etc; the first weapon of each
// type met is kept for OnUseWeapon (a dual only when two-handed). C2: one argument (the record's id is read from it)
__declspec(noinline) INT NCPawnViewrWnd::GetItemCategory(NViewItemData* Data)
{
    if (Data->Class == 1)
    {
        switch (Data->BodyPart)
        {
        case 6:
            return PAWNVIEW_Headgear;
        case 9:
            return PAWNVIEW_Gloves;
        case 10:
            return PAWNVIEW_Chest;
        case 11:
            return PAWNVIEW_Legs;
        case 12:
            return PAWNVIEW_Feet;
        }
    }
    else if (Data->Class == 0)
    {
        INT Type = Data->PawnWeaponType;
        if (DefaultWeapons[Type] == -1 && (Type != 3 || Data->BodyPart == BODYSLOT_LRHand))
            DefaultWeapons[Type] = RecordItemID(Data);
    }
    return PAWNVIEW_Etc;
}
NW_HOOK_METHOD(0x100aaf50, NCPawnViewrWnd, GetItemCategory)

// 0x100aafe0 (name ours; AddAllItem and OnSearch call it) - the record into an NItemInfo: the names and icons, the
// numbers by item class (0 weapon, 1 armor, 2 etc item) and a description of three parts, one to a line: the name at
// (0, 0) and the extra name at (0, 0) (two virtuals of the record), then the record's description. A record of
// another class stops after the icon (no description). The icon comes from the first icon's name,
// "NWindow.BlackTexture" when missing.
void NCPawnViewrWnd::FillItemInfo(NViewItemData* Data, NItemInfo* Info)
{
    INT Class = Data->Class;
    appStrcpy(Info->Name, *Data->Name);
    appStrcpy(Info->AdditionalName, *Data->AdditionalName);
    appStrcpy(Info->IconName, *Data->IconName);
    appStrcpy(Info->Unknown200, *Data->IconName2);
    appStrcpy(Info->Unknown300, *Data->IconName3);
    Info->Param = Class;
    Info->ID = Data->ItemID;
    Info->Icon = FindTexture(*Data->IconName);
    if (!Info->Icon)
        Info->Icon = FindTexture(TEXT("NWindow.BlackTexture"));
    if (Class == ITEMCLASS_Weapon)
    {
        Info->UnknownC28 = Data->Weight;
        Info->UnknownC30 = Data->WeaponType;
        Info->UnknownC34[0] = Data->PAtk;
        Info->UnknownC34[1] = Data->MAtk;
        Info->UnknownC34[4] = Data->ShieldPDef;
        Info->UnknownC34[7] = Data->CrystalGrade;
        Info->UnknownC7C = Data->SoulShots;
        Info->UnknownC80 = Data->SpiritShots;
        Info->BodyPart = Data->BodyPart;
    }
    else if (Class == ITEMCLASS_Armor)
    {
        Info->UnknownC28 = Data->Weight;
        Info->UnknownC34[7] = Data->ArmorCrystalGrade;
        Info->SubType = Data->ArmorType;
        Info->UnknownC34[2] = Data->PDef;
        Info->UnknownC34[3] = Data->MDef;
        Info->BodyPart = Data->BodyPart;
    }
    else if (Class == ITEMCLASS_Etc)
    {
        Info->UnknownC28 = Data->Weight;
        Info->UnknownC34[7] = Data->Unknown8C;
        Info->bStackable = Data->bStackable;
        Info->Unknown1710 = Data->EtcType;
        if (Data->EtcType == 2)
            Info->UnknownC94 = 1;
        else if (Data->EtcType == 5)
            Info->Unknown17A0 = 1;
    }
    else
        return;
    appSprintf(Info->Description, TEXT("%s\n%s\n%s"), *Data->GetNameAt(0, 0), *Data->GetExtraNameAt(0, 0),
               *Data->Description ? *Data->Description : TEXT(""));
}
NW_HOOK_METHOD(0x100aafe0, NCPawnViewrWnd, FillItemInfo)

// 0x100ab260 - the grids emptied (not the lists)
__declspec(noinline) void NCPawnViewrWnd::Clear()
{
    guard(NCPawnViewrWnd::Clear);
    ItemWnds[PAWNVIEW_Chest]->Clear();
    ItemWnds[PAWNVIEW_Legs]->Clear();
    ItemWnds[PAWNVIEW_Gloves]->Clear();
    ItemWnds[PAWNVIEW_Feet]->Clear();
    ItemWnds[PAWNVIEW_Headgear]->Clear();
    ItemWnds[PAWNVIEW_Etc]->Clear();
    unguard;
}
NW_HOOK_METHOD(0x100ab260, NCPawnViewrWnd, Clear)

// 0x100ab3b0 - an item into a category's grid and list (the list line gets an NItemEntry of
// its own, never freed); the page check lets 7 (the unused eighth pointer) through
__declspec(noinline) INT NCPawnViewrWnd::AddItem(INT Page, NItemInfo* Info)
{
    guard(NCPawnViewrWnd::AddItem);
    if (Page >= 0 && Page < 8 && ItemWnds[Page])
    {
        ItemWnds[Page]->AddItem(Info);
        NItemEntry* Entry = new NItemEntry(Info);
        TextWnds[Page]->List->AddString(Info->Name, 0xFFFFFFFF, (INT)Entry);
    }
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x100ab3b0, NCPawnViewrWnd, AddItem)

// 0x100b00b0 - every record of the item table into its category; each grid grows by one for every item added, so
// none ever fills
__declspec(noinline) void NCPawnViewrWnd::AddAllItem()
{
    guard(NCPawnViewrWnd::AddAllItem);
    NDataMap<NViewItemData>* Map = GL2GameData.ItemTable;
    for (INT i = 0; i < Map->Pairs.Num(); i++)
    {
        NViewItemData* Data = Map->Pairs(i).Value;
        if (Data)
        {
            NItemInfo Info;
            FillItemInfo(Data, &Info);
            INT Page = GetItemCategory(Data);
            AddItem(Page, &Info);
            ItemWnds[Page]->MaxItems++;
        }
    }
    unguard;
}
NW_HOOK_METHOD(0x100b00b0, NCPawnViewrWnd, AddAllItem)

NW_CHECK_VTABLE(NCPawnViewrWnd, 0x101206d8, 78)
