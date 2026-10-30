/*=============================================================================
    NCGMWnd.cpp - the GM's windows. Stock: 0x1005bc40 .. 0x1005bd7c, 0x1005be70 .. 0x1005bf22,
    0x1005bfc0 .. 0x1005c5a4, 0x1005c730 .. 0x1005c974, 0x1005c980 .. 0x1005ca42,
    0x1005e310 .. 0x1005e9a3, 0x1005eac0 .. 0x1005f1fd, 0x1005f250 .. 0x1005fc4c,
    0x1005ff10 .. 0x1006004f, 0x10060050 .. 0x100603f1 (NCGMStoreWnd's functions sit in
    between; NCGMActionWnd's OnDestroy is NCPetActionWnd's body, its OnPaint NCActionWnd's).
=============================================================================*/
#include "nwindow/NCGMWnd.h"
#include "nwindow/NCButton.h"
#include "nwindow/NCConsole.h"
#include "nwindow/NCDetailStatusWnd.h"
#include "nwindow/NCEditBox.h"
#include "nwindow/NCFrameCtrl.h"
#include "nwindow/NCGDevice.h"
#include "nwindow/NCListBox.h"
#include "nwindow/NCMagicSkillWnd.h"
#include "nwindow/NCQuestInfoWnd.h"
#include "nwindow/NCQuestItemWnd.h"
#include "nwindow/NCStoreWnd.h"
#include "nwindow/NCTabControl.h"
#include "nwindow/NConsoleWnd.h"
#include "nwindow/NDialogInfo.h"
#include "nwindow/NGameData.h"
#include "nwindow/NItemInfo.h"

// sysstring
enum
{
    STR_Item = 2,
    STR_Name = 50,
    STR_Lv = 88,
    STR_Quest = 118,
    STR_Skills = 119,
    STR_Warehouse = 131,
    STR_Inventory = 138,
    STR_AcceptedQuest = 324,
    STR_Description = 325,
    STR_QuestItem = 326,
    STR_Cancel = 327,
    STR_Leader = 342,
    STR_Base = 343,
    STR_HideoutExists = 344,
    STR_CastleExists = 345,
    STR_Status = 433,
    STR_Clan = 439,
};

enum { MSG_EnterUserName = 364 };      // systemmsg "Enter user's name to search."

enum { DIALOG_SystemMessage = 0x1F };  // NDialogInfo::ID: a message, OK only

enum { CLAN_TEXT_COLOR = 0xFFB09B79, CLAN_LABEL_COLOR = 0xFFA3A3A3 };

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
    NCGMWnd
-----------------------------------------------------------------------------*/

// 0x10076fd0 - CloseButton is left as allocated
NCGMWnd::NCGMWnd()
{
    TopTex = NULL;
    MidTex = NULL;
    BottomTex = NULL;
    NameEdit = NULL;
    InventoryButton = NULL;
    SkillsButton = NULL;
    StatusButton = NULL;
    QuestButton = NULL;
    WarehouseButton = NULL;
    ClanButton = NULL;
}
NW_HOOK_CTOR(0x10076fd0, NCGMWnd)

// 0x10077050
NW_HOOK_DTOR(0x10077050, NCGMWnd)

// 0x100732f0 (vslot 25) - C2 has no IsEnabled check of its own, and six more buttons
INT NCGMWnd::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (NCWnd::WindowProc(Msg, wParam, lParam))
        return 1;
    if (Msg == NWM_COMMAND)
    {
        if (wParam == NID_GM_INVENTORY)
            return OnInventory() != 0;
        if (wParam == NID_GM_SKILLS)
            return OnSkills() != 0;
        if (wParam == NID_GM_STATUS)
            return OnStatus() != 0;
        if (wParam == NID_GM_QUEST)
            return OnQuest() != 0;
        if (wParam == NID_GM_WAREHOUSE)
            return OnWarehouse() != 0;
        if (wParam == NID_GM_CLAN)
            return OnClan() != 0;
        if (wParam == NID_GM_CLOSE)
            return OnClose() != 0;
        if (wParam == NID_GM_NEW1)
            return OnNewButton1() != 0;
        if (wParam == NID_GM_NEW2)
            return OnNewButton2() != 0;
        if (wParam == NID_GM_NEW3)
            return OnNewButton3() != 0;
        if (wParam == NID_GM_NEW4)
            return OnNewButton4() != 0;
        if (wParam == NID_GM_NEW5)
            return OnNewButton5() != 0;
        if (wParam == NID_GM_NEW6)
            return OnNewButton6() != 0;
    }
    return 0;
}
NW_HOOK_METHOD(0x100732f0, NCGMWnd, WindowProc)

// the six view buttons: the desktop's toggle with the name typed, or a message without one
// (repeated in the stock code; the text is fetched twice)
#define NCGMWND_VIEW_BUTTON(Method, Toggle)                                                   \
    INT NCGMWnd::Method()                                                                     \
    {                                                                                         \
        if (NameEdit->CommitText())                                                           \
            GDesktop->Toggle((TCHAR*)NameEdit->CommitText());                                 \
        else                                                                                  \
            GConsole->ShowSystemMessage(MSG_EnterUserName, 0xFFFFFFFF);                       \
        return 1;                                                                             \
    }

// 0x100734c0 (vslot 62)
NCGMWND_VIEW_BUTTON(OnInventory, ToggleGMInventory)
NW_HOOK_METHOD(0x100734c0, NCGMWnd, OnInventory)
// 0x10073520 (vslot 63)
NCGMWND_VIEW_BUTTON(OnSkills, ToggleGMMagicSkill)
NW_HOOK_METHOD(0x10073520, NCGMWnd, OnSkills)
// 0x10073580 (vslot 64)
NCGMWND_VIEW_BUTTON(OnStatus, ToggleGMDetailStatus)
NW_HOOK_METHOD(0x10073580, NCGMWnd, OnStatus)
// 0x100735e0 (vslot 65)
NCGMWND_VIEW_BUTTON(OnQuest, ToggleGMQuest)
NW_HOOK_METHOD(0x100735e0, NCGMWnd, OnQuest)
// 0x10073640 (vslot 66)
NCGMWND_VIEW_BUTTON(OnWarehouse, ToggleGMStore)
NW_HOOK_METHOD(0x10073640, NCGMWnd, OnWarehouse)
// 0x100736a0 (vslot 67)
NCGMWND_VIEW_BUTTON(OnClan, ToggleGMAction)
NW_HOOK_METHOD(0x100736a0, NCGMWnd, OnClan)

// 0x10073700 (vslot 73) - the view windows shown are hidden, then this one. The first two
// tests are crossed in the stock code: the inventory being shown hides the skill window and
// the skill window being shown hides the inventory, so the inventory never closes here and
// the skill window only along with it.
INT NCGMWnd::OnClose()
{
    if (GDesktop->GMInventoryWnd->IsShowWindow())
        GDesktop->GMMagicSkillWnd->HideWindow();
    if (GDesktop->GMMagicSkillWnd->IsShowWindow())
        GDesktop->GMInventoryWnd->HideWindow();
    if (GDesktop->GMDetailStatusWnd->IsShowWindow())
        GDesktop->GMDetailStatusWnd->HideWindow();
    if (GDesktop->GMQuestWnd->IsShowWindow())
        GDesktop->GMQuestWnd->HideWindow();
    if (GDesktop->GMStoreWnd->IsShowWindow())
        GDesktop->GMStoreWnd->HideWindow();
    if (GDesktop->GMActionWnd->IsShowWindow())
        GDesktop->GMActionWnd->HideWindow();
    NCWnd::HideWindow();
    return 1;
}
NW_HOOK_METHOD(0x10073700, NCGMWnd, OnClose)

// a view button (repeated in the stock OnCreate): 54 x 18, labelled with a sysstring
static FORCEINLINE NCButton* CreateViewButton(NCWnd* Parent, INT ID, INT X, INT Y, INT TextID)
{
    NCButton* Button = new NCButton(TEXT("sek.cbui91"), TEXT("sek.cbui93"), 0);
    Button->ID = ID;
    Button->SetWindowRect(X, Y, 54, 18);
    Parent->CreateChild(Button, NWS_VISIBLE | 0x20);   // 0x20: meaning unknown
    Button->SetText(*NCObject::SysString(TextID));
    return Button;
}

// a button of the C2 columns (repeated in the stock OnCreate): 54 x 18, its text a sysstring id given to SetTextID
static FORCEINLINE NCButton* CreateNewButton(NCWnd* Parent, INT ID, INT X, INT Y, INT TextID)
{
    NCButton* Button = new NCButton(TEXT("sek.cbui91"), TEXT("sek.cbui93"), 0);
    Button->ID = ID;
    Button->SetWindowRect(X, Y, 54, 18);
    Parent->CreateChild(Button, NWS_VISIBLE | 0x20);   // 0x20: meaning unknown
    Button->SetTextID(TextID);
    return Button;
}

// 0x100770a0 (vslot 36) - a plain title bar 25 pixels high, 19 below the top, the name box and the buttons in three
// columns (x 10, 74, 138; rows 75, 103 and 131, 159): the six view buttons of C1 in the first two, the new ones (texts
// 687.. 692, the sysstrings of OnNewButton1..6) in the third column and in the last row. The third new button
// (NID_GM_NEW3, NewChild[2]; its text would be 689 by the sequence) is not made: the gap at (138, 103) stays and its slot
// stays as allocated, OnNewButton3 (the "snoop" command) has no button.
INT NCGMWnd::OnCreate()
{
    NCWnd::OnCreate();
    TopTex = FindTexture(TEXT("sek.ui02"));
    MidTex = FindTexture(TEXT("sek.cbui157"));
    BottomTex = FindTexture(TEXT("sek.ui03"));

    FrameCtrl = new NCFrameCtrl(0);
    FrameCtrl->ID = NID_GM_FRAME;
    FrameCtrl->SetWindowRect(0, 19, Width, 25);
    CreateChild(FrameCtrl, NWS_VISIBLE);

    NameEdit = new NCEditBox(0, TEXT("NWindow.ChatBack"), 1, 128, 0);
    NameEdit->ID = NID_GM_NAME;
    NameEdit->SetWindowRect(35, 55, 112, 14);
    CreateChild(NameEdit, NWS_VISIBLE);

    StatusButton = CreateViewButton(this, NID_GM_STATUS, 10, 75, STR_Status);
    InventoryButton = CreateViewButton(this, NID_GM_INVENTORY, 74, 75, STR_Inventory);
    SkillsButton = CreateViewButton(this, NID_GM_SKILLS, 10, 103, STR_Skills);
    QuestButton = CreateViewButton(this, NID_GM_QUEST, 74, 103, STR_Quest);
    WarehouseButton = CreateViewButton(this, NID_GM_WAREHOUSE, 10, 131, STR_Warehouse);
    ClanButton = CreateViewButton(this, NID_GM_CLAN, 74, 131, STR_Clan);

    NewChild[0] = CreateNewButton(this, NID_GM_NEW1, 138, 75, 687);
    NewChild[1] = CreateNewButton(this, NID_GM_NEW2, 138, 131, 688);
    NewChild[3] = CreateNewButton(this, NID_GM_NEW4, 10, 159, 690);
    NewChild[4] = CreateNewButton(this, NID_GM_NEW5, 74, 159, 691);
    NewChild[5] = CreateNewButton(this, NID_GM_NEW6, 138, 159, 692);

    CloseButton = new NCButton();
    CloseButton->ID = NID_GM_CLOSE;
    CloseButton->SetWindowRect(79, 194, 50, 16);
    CreateChild(CloseButton, NWS_VISIBLE);
    CloseButton->SetPushedTex(TEXT("sek.cbui27"));
    return 1;
}
NW_HOOK_METHOD(0x100770a0, NCGMWnd, OnCreate)

// 0x10077ac0 (vslot 37) - C2 guards it; the six new children go last
INT NCGMWnd::OnDestroy()
{
    guard(NCGMWnd::OnDestroy);
    DestroyChild(this, NameEdit);
    DestroyChild(this, InventoryButton);
    DestroyChild(this, SkillsButton);
    DestroyChild(this, StatusButton);
    DestroyChild(this, QuestButton);
    DestroyChild(this, WarehouseButton);
    DestroyChild(this, ClanButton);
    DestroyChild(this, CloseButton);
    DestroyChild(this, NewChild[0]);
    DestroyChild(this, NewChild[1]);
    DestroyChild(this, NewChild[2]);
    DestroyChild(this, NewChild[3]);
    DestroyChild(this, NewChild[4]);
    DestroyChild(this, NewChild[5]);
    NCWnd::OnDestroy();
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x10077ac0, NCGMWnd, OnDestroy)

// 0x10073860 (vslot 74, new in C2; name ours) - the player named in the box: the builder command "teleportto <name>"
INT NCGMWnd::OnNewButton1()
{
    const TCHAR* Name = NameEdit->CommitText();
    if (Name)
    {
        FString Command(TEXT("teleportto "));
        Command += Name;
        L2ParamStack Params(10);
        Params.PushBack((void*)*Command);
        GConsole->BuilderCommand(Params);
    }
    return 1;
}
NW_HOOK_METHOD(0x10073860, NCGMWnd, OnNewButton1)

// 0x10073920 (vslot 75, new in C2; name ours) - "recall <name>"
INT NCGMWnd::OnNewButton2()
{
    const TCHAR* Name = NameEdit->CommitText();
    if (Name)
    {
        FString Command(TEXT("recall "));
        Command += Name;
        L2ParamStack Params(10);
        Params.PushBack((void*)*Command);
        GConsole->BuilderCommand(Params);
    }
    return 1;
}
NW_HOOK_METHOD(0x10073920, NCGMWnd, OnNewButton2)

// 0x100739e0 (vslot 76, new in C2; name ours) - "snoop <name> on"
INT NCGMWnd::OnNewButton3()
{
    const TCHAR* Name = NameEdit->CommitText();
    if (Name)
    {
        FString Command(TEXT("snoop "));
        Command += Name;
        Command += TEXT(" on");
        L2ParamStack Params(10);
        Params.PushBack((void*)*Command);
        GConsole->BuilderCommand(Params);
    }
    return 1;
}
NW_HOOK_METHOD(0x100739e0, NCGMWnd, OnNewButton3)

// the match of a name search (the three buttons below): "ClassID:<id> Name:<name>" in an OK-only dialog box (no owner)
static FORCEINLINE void ShowClassID(INT ClassID, const TCHAR* Name)
{
    TCHAR Text[256];
    appSprintf(Text, TEXT("ClassID:%d Name:%s"), ClassID, Name);
    NDialogInfo Info;
    Info.ID = DIALOG_SystemMessage;
    Info.bMainBox = 1;
    Info.bInput = 0;
    Info.SetText(Text);
    GConsole->ShowDialog(&Info);
}

// 0x10078bf0 (vslot 77, new in C2; name ours) - the first NPC of the NPC table (GL2GameData +0x27E8, by id) whose
// name is the one typed: its class id (the NPC id plus 1,000,000) and name in a dialog box. The records are the
// table's walk by TMap's iterator, from the start; none found: nothing.
INT NCGMWnd::OnNewButton4()
{
    const TCHAR* Search = NameEdit->CommitText();
    if (Search)
    {
        NDataMap<NViewNpcData>* Map = GL2GameData.NpcTable;
        for (INT i = 0; i < Map->Pairs.Num(); i++)
        {
            NViewNpcData* Npc = Map->Pairs(i).Value;
            if (Npc && *Npc->Name && !appStrcmp(*Npc->Name, Search))
            {
                ShowClassID(Npc->ID + 1000000, *Npc->Name);
                return 1;
            }
        }
    }
    return 1;
}
NW_HOOK_METHOD(0x10078bf0, NCGMWnd, OnNewButton4)

// 0x10078d60 (vslot 78, new in C2; name ours) - the same for the item table (+0x27E4): the name is an L2FName, the id
// shown the record's +0xC
INT NCGMWnd::OnNewButton5()
{
    const TCHAR* Search = NameEdit->CommitText();
    if (Search)
    {
        NDataMap<NViewItemData>* Map = GL2GameData.ItemTable;
        for (INT i = 0; i < Map->Pairs.Num(); i++)
        {
            NViewItemData* Item = Map->Pairs(i).Value;
            if (Item && *Item->Name && !appStrcmp(*Item->Name, Search))
            {
                ShowClassID(Item->ItemID, *Item->Name);
                return 1;
            }
        }
    }
    return 1;
}
NW_HOOK_METHOD(0x10078d60, NCGMWnd, OnNewButton5)

// 0x10078ec0 (vslot 79, new in C2; name ours) - the same for the skill table (+0x4CB8: one record per level, the
// first level with the name is found): the skill id (+4) and the name (an FString at +0x28)
INT NCGMWnd::OnNewButton6()
{
    const TCHAR* Search = NameEdit->CommitText();
    if (Search)
    {
        NDataMap<FL2MagicSkillData>* Map = GL2GameData.SkillTable;
        for (INT i = 0; i < Map->Pairs.Num(); i++)
        {
            FL2MagicSkillData* Skill = Map->Pairs(i).Value;
            if (Skill && *Skill->Name && !appStrcmp(*Skill->Name, Search))
            {
                ShowClassID(Skill->SkillID, *Skill->Name);
                return 1;
            }
        }
    }
    return 1;
}
NW_HOOK_METHOD(0x10078ec0, NCGMWnd, OnNewButton6)

// 0x10077da0 (vslot 50) - the middle first, the top and bottom over its edges, "Name" before
// the box
INT NCGMWnd::OnPaint(NCGDevice* Device)
{
    Device->SetClip(ScreenX, ScreenY, Width, Height);
    Device->BeginPaint(ScreenX, ScreenY);
    if (MidTex)
        Device->DrawTexture(0, 43, 208, 144, 0, 0, 208, 144, MidTex);
    if (TopTex)
        Device->DrawTexture(0, 0, 208, 47, 0, 0, 208, 47, TopTex);
    if (BottomTex)
        Device->DrawTexture(0, 184, 208, 48, 0, 0, 208, 48, BottomTex);
    Device->DrawText(7, 55, 0xFFFFFFFF, *NCObject::SysString(STR_Name), 0, 0, 0, L2FONT_Normal,
                     0, 0, 0, 0, 0, 0);
    Device->EndPaint();
    Device->ReleaseClip();
    return 1;
}
NW_HOOK_METHOD(0x10077da0, NCGMWnd, OnPaint)

NW_CHECK_VTABLE(NCGMWnd, 0x10117788, 80)

/*-----------------------------------------------------------------------------
    NCGMInvenItemWnd, NCGMEquipItemWnd
-----------------------------------------------------------------------------*/

// 0x100784a0
NCGMInvenItemWnd::NCGMInvenItemWnd()
{
}
NW_HOOK_CTOR(0x100784a0, NCGMInvenItemWnd)

// 0x10078540
NCGMInvenItemWnd::NCGMInvenItemWnd(INT InColumns, INT InVisibleRows, INT InMaxItems, INT InCellSize)
    : NCInvenItemWnd(InColumns, InVisibleRows, InMaxItems, InCellSize)
{
}
NW_HOOK_CTOR_ARGS(0x10078540, NCGMInvenItemWnd, (INT A, INT B, INT C, INT D), (A, B, C, D))

// 0x100784f0
NW_HOOK_DTOR(0x100784f0, NCGMInvenItemWnd)

// 0x10072f70 (vslot 25) - the base's answer as a plain 0 / 1
INT NCGMInvenItemWnd::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    return NCInvenItemWnd::WindowProc(Msg, wParam, lParam) != 0;
}
NW_HOOK_METHOD(0x10072f70, NCGMInvenItemWnd, WindowProc)

// stock body shared with NCGMSkillItemWnd's OnLButtonUp (0x1005bdc0): the carried item is only
// put down
INT NCGMInvenItemWnd::OnLButtonUp(WPARAM wParam, LPARAM lParam)
{
    bDragging = 0;
    return 1;
}

// stock body shared with NCGMSkillItemWnd's OnLButtonUp (0x1005bdc0)
INT NCGMInvenItemWnd::OnRButtonDown(WPARAM wParam, LPARAM lParam)
{
    bDragging = 0;
    return 1;
}

// stock body shared with NCGMSkillItemWnd's OnLButtonUp (0x1005bdc0)
INT NCGMInvenItemWnd::OnLButtonDblClick(WPARAM wParam, LPARAM lParam)
{
    bDragging = 0;
    return 1;
}

NW_CHECK_VTABLE(NCGMInvenItemWnd, 0x10117dc8, 74)

// 0x100783f0
NCGMEquipItemWnd::NCGMEquipItemWnd(INT InCellSize) : NCEquipItemWnd(InCellSize)
{
    Unknown250 = 0;
    Unknown251 = 0;
}
NW_HOOK_CTOR_ARGS(0x100783f0, NCGMEquipItemWnd, (INT A), (A))

// 0x10078450
NW_HOOK_DTOR(0x10078450, NCGMEquipItemWnd)

// 0x10072e40 (vslot 25) - the base's answer as a plain 0 / 1
INT NCGMEquipItemWnd::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    return NCEquipItemWnd::WindowProc(Msg, wParam, lParam) != 0;
}
NW_HOOK_METHOD(0x10072e40, NCGMEquipItemWnd, WindowProc)

// 0x10072e60 (vslot 62) - the carried item is only put down; also slots 58 and 62
INT NCGMEquipItemWnd::OnLButtonUp(WPARAM wParam, LPARAM lParam)
{
    bDragging = 0;
    return 1;
}
NW_HOOK_METHOD(0x10072e60, NCGMEquipItemWnd, OnLButtonUp)

// stock body shared with OnLButtonUp (0x1005bc80)
INT NCGMEquipItemWnd::OnRButtonDown(WPARAM wParam, LPARAM lParam)
{
    bDragging = 0;
    return 1;
}

// stock body shared with OnLButtonUp (0x1005bc80)
INT NCGMEquipItemWnd::OnLButtonDblClick(WPARAM wParam, LPARAM lParam)
{
    bDragging = 0;
    return 1;
}

// 0x10072e80 (vslot 67) - the paperdoll slot of a body part, by its bits alone: NConsoleWnd::GetEquipSlot's table
// without the part that asks what the player wears for the two rings and the two earrings (the GM sees somebody else's
// items: the first of the bits 2 / 4 and 0x10 / 0x20 found wins, AddItem has kept only one of each pair)
INT NCGMEquipItemWnd::GetEquipSlot(INT BodyPart, INT ObjectID)
{
    if (BodyPart & 2)
        return 3;
    if (BodyPart & 4)
        return 4;
    if (BodyPart & 0x10)
        return 12;
    if (BodyPart & 0x20)
        return 13;
    switch (BodyPart)
    {
    case 0x1: return 0;
    case 0x8: return 8;
    case 0x40: return 1;
    case 0x80: return 5;
    case 0x100: return 7;
    case 0x200: return 9;
    case 0x400: return 6;
    case 0x800: return 10;
    case 0x1000: return 11;
    case 0x2000: return 2;
    case 0x4000: return 15;
    case 0x8000: return 14;
    }
    return -1;
}
NW_HOOK_METHOD(0x10072e80, NCGMEquipItemWnd, GetEquipSlot)

// the cell of each single slot (the 5 x 3 grid of the paperdoll, 35 pixels apart; NCEquipItemWnd's SlotX / SlotY, which
// the stock GM paint spells as a switch)
static const INT GMSlotX[14] = { 0, 35, 70, 140, 175, 0, 35, 70, 175, 0, 35, 70, 140, 175 };
static const INT GMSlotY[14] = { 0, 0, 0, 0, 0, 35, 35, 35, 35, 70, 70, 70, 70, 70 };

// 0x10073cd0 (vslot 55) - NCEquipItemWnd's paint (0x1007b5c0) as code of its own: the icons in their slots (the two-slot
// items in both, the second under the "dual" cap; a bow in the right hand alone while an item with Unknown1C98 is
// equipped), and the tattoo boxes for three slots whatever the character may have (the console is not asked)
INT NCGMEquipItemWnd::OnPaint(NCGDevice* Device)
{
    guard(NCGMEquipItemWnd::OnPaint);
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
            X = GMSlotX[Slot];
            Y = GMSlotY[Slot];
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
        for (INT i = 0; i < 3; i++)
        {
            Device->DrawTexture(0x6C, i * 27, 26, 26, 0, 0, 26, 26, HennaBackTex);
            if (i < Hennas.Num() && Hennas(i)->GetIcon())
            {
                UTexture* Icon = Hennas(i)->GetIcon();
                INT TopY = i * 27 + 1;
                Device->DrawStretchedTex(0x6D, TopY, 24, 24, Icon);
                if (Hennas(i)->UnknownC34 == 0)
                    Device->DrawTexture(0x6D, TopY, 24, 24, 0, 0, 24, 24, FindTexture(TEXT("NWindow.ChatBack")));
            }
        }
    }
    Device->EndPaint();
    Device->ReleaseClip();
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x10073cd0, NCGMEquipItemWnd, OnPaint)

NW_CHECK_VTABLE(NCGMEquipItemWnd, 0x10117cb0, 69)

/*-----------------------------------------------------------------------------
    NCGMInventoryWnd
-----------------------------------------------------------------------------*/

// 0x100741a0 - the textures other than BackTex and the paperdoll are left as allocated
NCGMInventoryWnd::NCGMInventoryWnd()
{
    BackTex = NULL;
    TabCtrl = NULL;
    ItemWnd[0] = NULL;
    ItemWnd[1] = NULL;
}
NW_HOOK_CTOR(0x100741a0, NCGMInventoryWnd)

// 0x10074200
NW_HOOK_DTOR(0x10074200, NCGMInventoryWnd)

// stock body shared with NControl's (0x10004e90)
INT NCGMInventoryWnd::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (Msg != WM_PAINT && !IsEnabled())
        return 0;
    return NCWnd::WindowProc(Msg, wParam, lParam) != 0;
}

// 0x10072fb0 - the paperdoll and both pages empty
void NCGMInventoryWnd::ClearItems()
{
    EquipWnd->Clear();
    ItemWnd[0]->Clear();
    ItemWnd[1]->Clear();
    EquipWnd->Unknown250 = 0;
    EquipWnd->Unknown251 = 0;
}
NW_HOOK_METHOD(0x10072fb0, NCGMInventoryWnd, ClearItems)

// 0x10073000 (vslot 43)
void NCGMInventoryWnd::ShowWindow()
{
    NCWnd::ShowWindow();
    TabCtrl->ShowCurrentPage();
}
NW_HOOK_METHOD(0x10073000, NCGMInventoryWnd, ShowWindow)

// 0x10078160 - a worn item goes to the paperdoll, the others to the page of their type (3: the quest items, else the
// items)
UBOOL NCGMInventoryWnd::AddItem(NItemInfo* Info)
{
    if (Info->UnknownC8C)
    {
        EquipWnd->AddItem(Info);
        return 1;
    }
    NCGMInvenItemWnd* Page = ItemWnd[Info->Param == 3];
    if (Page)
        Page->AddItem(Info);
    return 1;
}
NW_HOOK_METHOD(0x10078160, NCGMInventoryWnd, AddItem)

// 0x10077f50 - NCEquipItemWnd's AddItem; then on the new entry's slot mask: the first of two bits 2 / 4 (0x30: 0x10 /
// 0x20, two earring or ring slots) found keeps its low bit and clears the other, the second one takes the other (the
// two flags remember which was given out)
UBOOL NCGMEquipItemWnd::AddItem(NItemInfo* Info)
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
    if (Item->BodyPart & 6)
    {
        if (Unknown250 == 0)
        {
            Item->BodyPart &= ~2;
            Unknown250 = 1;
        }
        else
            Item->BodyPart &= ~4;
    }
    if (Item->BodyPart & 0x30)
    {
        if (Unknown251 == 0)
        {
            Item->BodyPart &= ~0x10;
            Unknown251 = 1;
        }
        else
            Item->BodyPart &= ~0x20;
    }
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
NW_HOOK_METHOD(0x10077f50, NCGMEquipItemWnd, AddItem)

// 0x10074250 (vslot 55) - the background and the "Item" page's adena (thousands separators, right aligned to 184)
INT NCGMInventoryWnd::OnPaint(NCGDevice* Device)
{
    guard(NCGMInventoryWnd::OnPaint);
    Device->BeginPaint(ScreenX, ScreenY);
    Device->SetClip(ScreenX, ScreenY, Width, Height);
    if (BackTex)
        Device->DrawTexture(0, 21, 240, 344, 0, 0, 240, 344, BackTex);

    TCHAR Buf[128];
    TCHAR Amount[128];
    SIZE Size;
    appSprintf(Buf, TEXT("%d"), ItemWnd[0]->GetAdena());
    FormatNumber(Amount, Buf);
    appSprintf(Buf, TEXT("%s"), Amount);
    NCGDevice::GetTextExtent(&Size, Buf, 0, 0, 0, 0, 0);
    Device->DrawText(184 - Size.cx, 317, 0xFFFFFFFF, Buf, 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
    if (AdenaTex)
        Device->DrawTexture(79, 317, 16, 12, 0, 0, 16, 12, AdenaTex);
    Device->EndPaint();
    Device->ReleaseClip();
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x10074250, NCGMInventoryWnd, OnPaint)

// 0x100743f0 (vslot 37)
INT NCGMInventoryWnd::OnDestroy()
{
    DestroyChild(this, ItemWnd[0]);
    DestroyChild(this, ItemWnd[1]);
    DestroyChild(this, EquipWnd);
    DestroyChild(this, TabCtrl);
    NCWnd::OnDestroy();
    return 1;
}
NW_HOOK_METHOD(0x100743f0, NCGMInventoryWnd, OnDestroy)

// 0x10078570 (vslot 36) - the inventory's layout: the title bar, the paperdoll (32-pixel cells), the two pages (6 x 4
// icons of 32 pixels, 160 items at most) and their tabs
INT NCGMInventoryWnd::OnCreate()
{
    NCWnd::OnCreate();
    BackTex = FindTexture(TEXT("L2UI.InventoryWnd.InventoryBack"));
    AdenaTex = FindTexture(TEXT("L2UI.WindowIcon.WindowIconAdena"));

    FrameCtrl = new NCFrameCtrl(1);
    FrameCtrl->ID = NID_GMINVENTORY_FRAME;
    FrameCtrl->SetWindowRect(0, 0, 240, 21);
    CreateChild(FrameCtrl, NWS_VISIBLE);

    EquipWnd = new NCGMEquipItemWnd(32);
    EquipWnd->SetWindowRect(16, 29, 207, 102);
    CreateChild(EquipWnd, NWS_VISIBLE | 0x20);

    ItemWnd[0] = new NCGMInvenItemWnd(6, 4, 160, 32);
    ItemWnd[0]->SetWindowRect(16, 166, 222, 137);
    CreateChild(ItemWnd[0], NWS_VISIBLE | 0x20);

    ItemWnd[1] = new NCGMInvenItemWnd(6, 4, 160, 32);
    ItemWnd[1]->SetWindowRect(16, 166, 222, 137);
    CreateChild(ItemWnd[1], NWS_VISIBLE | 0x20);

    TabCtrl = new NCTabControl();
    TabCtrl->ID = NID_GMINVENTORY_TABS;
    TabCtrl->SetWindowRect(15, 141, 138, 16);
    CreateChild(TabCtrl, NWS_VISIBLE);
    TabCtrl->AddTab(STR_Item, ItemWnd[0], 69, 16, TEXT("L2UI.NWindow.Tab2"), TEXT("L2UI.NWindow.Tab1"), 0, -1);
    TabCtrl->AddTab(STR_Quest, ItemWnd[1], 69, 16, TEXT("L2UI.NWindow.Tab2"), TEXT("L2UI.NWindow.Tab1"), 0, -1);
    return 1;
}
NW_HOOK_METHOD(0x10078570, NCGMInventoryWnd, OnCreate)

// 0x10073020 (name ours) - the console's slot 143 (GMAddHennaInfo): the paperdoll's tattoo (a tail jump to NCEquipItemWnd::AddHenna)
void NCGMInventoryWnd::AddHenna(INT HennaID, INT bUsable)
{
    EquipWnd->AddHenna(HennaID, bUsable);
}
NW_HOOK_METHOD(0x10073020, NCGMInventoryWnd, AddHenna)

// 0x10073030 (name ours) - the console's slot 144 (GMReceiveHennaInfo): the paperdoll's tattoos gone (a tail jump to NCEquipItemWnd::ClearHennas)
void NCGMInventoryWnd::ClearHennas()
{
    EquipWnd->ClearHennas();
}
NW_HOOK_METHOD(0x10073030, NCGMInventoryWnd, ClearHennas)

NW_CHECK_VTABLE(NCGMInventoryWnd, 0x10116c98, 67)

/*-----------------------------------------------------------------------------
    NCGMQuestWnd
-----------------------------------------------------------------------------*/

// 0x10076370
NCGMQuestWnd::NCGMQuestWnd()
{
}
NW_HOOK_CTOR(0x10076370, NCGMQuestWnd)

// 0x100763c0
NW_HOOK_DTOR(0x100763c0, NCGMQuestWnd)

// 0x100730f0 (vslot 25) - the base's answer as a plain 0 / 1
INT NCGMQuestWnd::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    return NCQuestWnd::WindowProc(Msg, wParam, lParam) != 0;
}
NW_HOOK_METHOD(0x100730f0, NCGMQuestWnd, WindowProc)

// 0x10076410 (vslot 36) - a title bar first (so the quest window's own OnCreate puts everything 21 pixels lower), then
// the quest window's, and no Cancel button for the GM
INT NCGMQuestWnd::OnCreate()
{
    FrameCtrl = new NCFrameCtrl(1);
    FrameCtrl->ID = NID_GMQUEST_FRAME;
    FrameCtrl->SetWindowRect(0, 0, 240, 21);
    CreateChild(FrameCtrl, NWS_VISIBLE);
    NCQuestWnd::OnCreate();
    CancelButton->EnableWindow(0);
    return 1;
}
NW_HOOK_METHOD(0x10076410, NCGMQuestWnd, OnCreate)

// 0x100764d0 (vslot 55) - the quest window's pictures 21 pixels lower under the title bar; the clip is 11 pixels taller
// than the window; the title is the accepted quests' (no count of them)
INT NCGMQuestWnd::OnPaint(NCGDevice* Device)
{
    Device->SetClip(ScreenX, ScreenY, Width, Height + 11);
    Device->BeginPaint(ScreenX, ScreenY);
    if (TopTex)
        Device->DrawTexture(0, 21, 240, 115, 0, 0, 240, 115, TopTex);
    UTexture* Back = CurTab == 0 ? InfoBackTex : ItemBackTex;
    if (Back)
        Device->DrawTexture(0, 135, 240, 185, 0, 0, 240, 185, Back);
    if (ItemPageIconTex && CurTab)
        Device->DrawTexture(220, 136, 14, 14, 0, 0, 32, 32, ItemPageIconTex);
    SIZE Size;
    NCGDevice::GetTextExtent(&Size, *SysString(324), 0, 0, 0, 0, 0);
    Device->DrawText((105 - Size.cx) / 2 + 22, 27, 0xFFFFFFFF, *SysString(324), 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0,
                     0, 0);
    Device->EndPaint();
    Device->ReleaseClip();
    return 1;
}
NW_HOOK_METHOD(0x100764d0, NCGMQuestWnd, OnPaint)

NW_CHECK_VTABLE(NCGMQuestWnd, 0x10117268, 74)

/*-----------------------------------------------------------------------------
    NCGMPledgeWnd
-----------------------------------------------------------------------------*/

// 0x10076760
NCGMPledgeWnd::NCGMPledgeWnd()
{
}
NW_HOOK_CTOR(0x10076760, NCGMPledgeWnd)

// 0x100767b0
NW_HOOK_DTOR(0x100767b0, NCGMPledgeWnd)

// 0x10073180 (vslot 25) - the base's answer as a plain 0 / 1
INT NCGMPledgeWnd::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    return NCPledgeWnd::WindowProc(Msg, wParam, lParam) != 0;
}
NW_HOOK_METHOD(0x10073180, NCGMPledgeWnd, WindowProc)

// 0x100731a0 (vslot 37)
INT NCGMPledgeWnd::OnDestroy()
{
    NCPledgeWnd::OnDestroy();
    return 1;
}
NW_HOOK_METHOD(0x100731a0, NCGMPledgeWnd, OnDestroy)

// 0x10076800 (vslot 55) - the player's page (0x100419e0) one pixel to the left: the clip starts at the top of the window
// (the background drawn 17 pixels above it loses its first 17 rows, 300 of them drawn instead of 301) and runs 17 past
// the bottom; the crest by its texture name ("crest.<server>_<id>", found by each paint, kept in CrestTex) with no
// question to the network handler (C1 asked whether it had imported it)
INT NCGMPledgeWnd::OnPaint(NCGDevice* Device)
{
    Device->SetClip(ScreenX, ScreenY, Width, Height + 17);
    Device->BeginPaint(ScreenX, ScreenY);
    if (BackTex)
        Device->DrawTexture(0, -17, 240, 300, 0, 0, 240, 300, BackTex);

    INT NameX = 17;
    if (Info.CrestID > 0)
    {
        TCHAR CrestName[64];
        appSprintf(CrestName, TEXT("crest.%d_%d"), GConsole->GetServerID(), Info.CrestID);
        CrestTex = FindTexture(CrestName);
        if (CrestTex)
        {
            Device->DrawTexture(17, 18, 16, 12, 0, 4, 16, 12, CrestTex);
            NameX = 37;
        }
    }
    SIZE Size;
    const TCHAR* Name = Info.Name;
    if (Name && *Name)
    {
        NCGDevice::GetTextExtent(&Size, Name, 0, 0, 0, 0, 0);  // not used
        Device->DrawText(NameX, 19, CLAN_TEXT_COLOR, Name, 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
    }

    if (*SysString(STR_Leader))
        Device->DrawText(17, 33, CLAN_LABEL_COLOR, *SysString(STR_Leader), 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
    const TCHAR* Leader = Info.LeaderName;
    if (Leader && *Leader)
        Device->DrawText(60, 33, CLAN_TEXT_COLOR, Leader, 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
    const TCHAR* Status = Info.StatusText;
    if (Status && *Status)
    {
        NCGDevice::GetTextExtent(&Size, Leader, 0, 0, 0, 0, 0);
        Device->DrawText(Size.cx + 48, 33, CLAN_TEXT_COLOR, Status, 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
    }
    if (*SysString(STR_Lv))
        Device->DrawText(139, 33, CLAN_LABEL_COLOR, *SysString(STR_Lv), 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
    TCHAR Number[10];
    appSprintf(Number, TEXT("%d"), Info.Level);
    Device->DrawText(158, 33, CLAN_TEXT_COLOR, Number, 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);

    if (*SysString(STR_Base))
        Device->DrawText(17, 47, CLAN_LABEL_COLOR, *SysString(STR_Base), 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
    if (Info.bHasCastle)
    {
        Device->DrawText(60, 47, CLAN_TEXT_COLOR, *SysString(STR_CastleExists), 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0,
                         0);
        if (Info.bHasHideout)
        {
            NCGDevice::GetTextExtent(&Size, *SysString(STR_CastleExists), 0, 0, 0, 0, 0);
            Device->DrawText(Size.cx + 62, 47, CLAN_TEXT_COLOR, *SysString(STR_HideoutExists), 0, 0, 0, L2FONT_Normal,
                             0, 0, 0, 0, 0, 0);
        }
    }
    else if (Info.bHasHideout)
        Device->DrawText(60, 47, CLAN_TEXT_COLOR, *SysString(STR_HideoutExists), 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0,
                         0);
    Device->EndPaint();
    Device->ReleaseClip();
    return 1;
}
NW_HOOK_METHOD(0x10076800, NCGMPledgeWnd, OnPaint)

NW_CHECK_VTABLE(NCGMPledgeWnd, 0x101174b0, 79)

/*-----------------------------------------------------------------------------
    NCGMActionWnd
-----------------------------------------------------------------------------*/

// the clan page's nine buttons disabled (repeated in OnCreate and ShowWindow)
static FORCEINLINE void DisablePledgeButtons(NCPledgeWnd* Pledge)
{
    Pledge->TitleButton->EnableWindow(0);
    Pledge->CrestButton->EnableWindow(0);
    Pledge->WarButton->EnableWindow(0);
    Pledge->TruceButton->EnableWindow(0);
    Pledge->SurrenderButton->EnableWindow(0);
    Pledge->ExpelButton->EnableWindow(0);
    Pledge->YieldButton->EnableWindow(0);
    Pledge->LeaveButton->EnableWindow(0);
    Pledge->InviteButton->EnableWindow(0);
}

// 0x10076bd0 - PledgeWnd is left as allocated
NCGMActionWnd::NCGMActionWnd()
{
    BackTex = NULL;
}
NW_HOOK_CTOR(0x10076bd0, NCGMActionWnd)

// 0x10076c20
NW_HOOK_DTOR(0x10076c20, NCGMActionWnd)

// stock body shared with NControl's (0x10004e90)
INT NCGMActionWnd::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (Msg != WM_PAINT && !IsEnabled())
        return 0;
    return NCWnd::WindowProc(Msg, wParam, lParam) != 0;
}

// the label the GM's clan page gives its second new button: a Korean text in the stock code (U+AD8C U+D55C U+BCF4 U+AE30,
// "gwonhan bogi", view privileges; 0x10117714), spelled by code points
static const TCHAR GMPledgeButtonText[] = { 0xAD8C, 0xD55C, 0xBCF4, 0xAE30, 0 };

// 0x10076c70 (vslot 36) - the title bar and the clan page, hidden, its buttons disabled; C2: the page's two new buttons
// as well, the second (NCPledgeWnd::OnNewButton2, the selected member's privileges) enabled again and labelled with a
// text of its own
INT NCGMActionWnd::OnCreate()
{
    NCWnd::OnCreate();
    FrameCtrl = new NCFrameCtrl(1);
    FrameCtrl->ID = NID_GMACTION_FRAME;
    FrameCtrl->SetWindowRect(0, 0, 240, 21);
    CreateChild(FrameCtrl, NWS_VISIBLE);
    BackTex = FindTexture(TEXT("L2UI.AcitonWnd.PledgeBack"));

    PledgeWnd = new NCGMPledgeWnd();
    PledgeWnd->SetWindowRect(0, 21, 240, 300);
    CreateChild(PledgeWnd, NWS_VISIBLE);
    PledgeWnd->HideWindow();
    DisablePledgeButtons(PledgeWnd);
    PledgeWnd->NewButton6F4->EnableWindow(0);
    PledgeWnd->NewButton6F8->EnableWindow(1);
    PledgeWnd->NewButton6F8->SetText(GMPledgeButtonText);
    return 1;
}
NW_HOOK_METHOD(0x10076c70, NCGMActionWnd, OnCreate)

// 0x10076f20 (vslot 37)
INT NCGMActionWnd::OnDestroy()
{
    DestroyChild(this, PledgeWnd);
    NCWnd::OnDestroy();
    return 1;
}
NW_HOOK_METHOD(0x10076f20, NCGMActionWnd, OnDestroy)

// 0x10073220 (vslot 43) - the clan page's buttons disabled again (its own ShowWindow enables
// them by rank)
void NCGMActionWnd::ShowWindow()
{
    NCWnd::ShowWindow();
    DisablePledgeButtons(PledgeWnd);
}
NW_HOOK_METHOD(0x10073220, NCGMActionWnd, ShowWindow)

// stock body shared with NCActionWnd's (0x1005f200): draws nothing
INT NCGMActionWnd::OnPaint(NCGDevice* Device)
{
    Device->SetClip(ScreenX, ScreenY, Width, Height);
    Device->BeginPaint(ScreenX, ScreenY);
    Device->EndPaint();
    Device->ReleaseClip();
    return 1;
}

NW_CHECK_VTABLE(NCGMActionWnd, 0x10117608, 67)

/*-----------------------------------------------------------------------------
    The data tables' hash maps: the stock instance of TMapBase::Rehash sits among this window's code
    (the linker folded every NDataMap<T>'s copy into one)
-----------------------------------------------------------------------------*/

typedef NDataMap<NViewItemData> TViewItemMap;

// 0x10073ac0 - the hash rebuilt after the map grew or lost pairs: a new table of HashCount slots ("HashMapHash") filled
// with -1, each pair chained into the slot of its key's low bits, the old table freed. The iterator's destructor jumps
// to it (0x1006b660) and the NPC / item / pawn viewers' AddAll* functions call it.
NW_HOOK_METHOD(0x10073ac0, TViewItemMap, Rehash)
