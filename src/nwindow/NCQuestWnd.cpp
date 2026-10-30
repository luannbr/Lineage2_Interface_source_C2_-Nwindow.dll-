/*=============================================================================
    NCQuestWnd.cpp - the quest window. Stock: 0x100a0370 .. 0x100a04e8, 0x100a0fd0 ..
    0x100a13b6, 0x100a1580 .. 0x100a1656 and OnCreate (0x100a1890), plus the destructor
    (0x1005c6b0, among NCGMQuestWnd's code).
=============================================================================*/
#include "nwindow/NCQuestWnd.h"
#include "nwindow/NCButton.h"
#include "nwindow/NCDialogBox.h"
#include "nwindow/NCGDevice.h"
#include "nwindow/NCListBox.h"
#include "nwindow/NCQuestInfoWnd.h"
#include "nwindow/NCQuestItemWnd.h"
#include "nwindow/NCTabControl.h"
#include "nwindow/NConsoleWnd.h"
#include "nwindow/NDialogInfo.h"

// ids (names ours)
enum
{
    NID_QUEST_TAB = 0xF6,
    NID_QUEST_CANCEL = 0xF7,
    NID_QUEST_LIST = 0x1A03,
    NID_QUEST_NEW1 = 0xF8,            // new in C2
    NID_QUEST_NEW2 = 0xF9,
};
enum { DIALOG_CANCEL_QUEST = 30 };      // NDialogInfo::ID (name ours)
enum { TUTORIAL_QUEST_ITEMS = 0x2000 }; // the item page opened (name ours)

// sysstring
enum
{
    STR_AcceptedQuest = 324,
    STR_Description = 325,
    STR_Item = 326,
    STR_Cancel = 327,
};

// 0x100bf930 - the textures and the Cancel button are left as allocated
NCQuestWnd::NCQuestWnd()
{
    QuestList = NULL;
    InfoWnd = NULL;
    ItemWnd = NULL;
    TabCtrl = NULL;
    CurTab = 0;
    DropW = 27;
    DropH = 27;
    DropX = 160;
    DropY = 420;
}
NW_HOOK_CTOR(0x100bf930, NCQuestWnd)

// 0x10073c50
NW_HOOK_DTOR(0x10073c50, NCQuestWnd)

// 0x100becb0 (vslot 25) - C2 has no IsEnabled check of its own, and two more commands
INT NCQuestWnd::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (NCWnd::WindowProc(Msg, wParam, lParam))
        return 1;
    if (Msg == NWM_DIALOG_RESULT)
        return OnDialogResult(wParam, lParam) != 0;
    if (Msg == NWM_COMMAND)
    {
        if (wParam == NID_QUEST_CANCEL)
            return OnCancel() != 0;
        if (wParam == NID_QUEST_LIST)
            return OnSelectQuest((INT)lParam) != 0;
        if (wParam == NID_QUEST_TAB)
            return OnTab((INT)lParam) != 0;
        if (wParam == NID_QUEST_NEW1)
            return OnNewButton1() != 0;
        if (wParam == NID_QUEST_NEW2)
            return OnNewButton2() != 0;
    }
    return 0;
}
NW_HOOK_METHOD(0x100becb0, NCQuestWnd, WindowProc)

// 0x100bedb0 (vslot 71) - Cancel (and the two new children) only on the description page; the item page
// counts as a tutorial event
INT NCQuestWnd::OnTab(INT Page)
{
    if (Page == 0)
    {
        CurTab = 0;
        CancelButton->ShowWindow();
        NewChild[1]->ShowWindow();
        NewChild[0]->ShowWindow();
        return 1;
    }
    if (Page == 1)
    {
        CurTab = 1;
        CancelButton->HideWindow();
        NewChild[1]->HideWindow();
        NewChild[0]->HideWindow();
        GConsole->SendTutorialEvent(TUTORIAL_QUEST_ITEMS);
    }
    return 1;
}
NW_HOOK_METHOD(0x100bedb0, NCQuestWnd, OnTab)

// 0x100bee50 (vslot 67) - edges included
UBOOL NCQuestWnd::IsInDropZone(INT X, INT Y)
{
    X -= ScreenX + DropX;
    Y -= ScreenY + DropY;
    return X >= 0 && X <= DropW && Y >= 0 && Y <= DropH;
}
NW_HOOK_METHOD(0x100bee50, NCQuestWnd, IsInDropZone)

// 0x100bf9a0 (vslot 37) - C2 guards it
INT NCQuestWnd::OnDestroy()
{
    guard(NCQuestWnd::OnDestroy);
    if (QuestList)
    {
        RemoveChildWindow(QuestList->Entry);
        delete QuestList;
        QuestList = NULL;
    }
    if (InfoWnd)
    {
        RemoveChildWindow(InfoWnd->Entry);
        delete InfoWnd;
        InfoWnd = NULL;
    }
    if (ItemWnd)
    {
        RemoveChildWindow(ItemWnd->Entry);
        delete ItemWnd;
        ItemWnd = NULL;
    }
    if (TabCtrl)
    {
        RemoveChildWindow(TabCtrl->Entry);
        delete TabCtrl;
        TabCtrl = NULL;
    }
    if (CancelButton)
    {
        RemoveChildWindow(CancelButton->Entry);
        delete CancelButton;
        CancelButton = NULL;
    }
    if (NewChild[0])
    {
        RemoveChildWindow(NewChild[0]->Entry);
        delete NewChild[0];
        NewChild[0] = NULL;
    }
    if (NewChild[1])
    {
        RemoveChildWindow(NewChild[1]->Entry);
        delete NewChild[1];
        NewChild[1] = NULL;
    }
    NCWnd::OnDestroy();
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x100bf9a0, NCQuestWnd, OnDestroy)

// 0x100bfe90 (vslot 72, new in C2; name ours) - the command 0xF8: the arrow forward, the next step
INT NCQuestWnd::OnNewButton1()
{
    InfoWnd->NextStep();
    return 1;
}
NW_HOOK_METHOD(0x100bfe90, NCQuestWnd, OnNewButton1)

// 0x100bfeb0 (vslot 73, new in C2; name ours) - the command 0xF9: the arrow back, the step before
INT NCQuestWnd::OnNewButton2()
{
    InfoWnd->PrevStep();
    return 1;
}
NW_HOOK_METHOD(0x100bfeb0, NCQuestWnd, OnNewButton2)

// 0x100bfb50 (vslot 50) - BeginPaint comes before the clip here; the title is centered in
// the 105 pixels from x 22
INT NCQuestWnd::OnPaint(NCGDevice* Device)
{
    guard(NCQuestWnd::OnPaint);
    Device->BeginPaint(ScreenX, ScreenY);
    Device->SetClip(ScreenX, ScreenY, Width, Height);
    if (TopTex)
        Device->DrawTexture(0, 0, 240, 115, 0, 0, 240, 115, TopTex);
    if (CurTab == 0 && InfoBackTex)
        Device->DrawTexture(0, 115, 240, 185, 0, 0, 240, 185, InfoBackTex);
    else if (CurTab != 0 && ItemBackTex)
        Device->DrawTexture(0, 115, 240, 185, 0, 0, 240, 185, ItemBackTex);
    if (ItemPageIconTex && CurTab)
        Device->DrawTexture(220, 127, 0, 14, 0, 0, 32, 32, ItemPageIconTex);   // C2: the width is 0 (C1: 14)
    SIZE Size;
    NCGDevice::GetTextExtent(&Size, *SysString(STR_AcceptedQuest), 0, 0, 0, 0, 0);
    Device->DrawText((105 - Size.cx) / 2 + 22, 3, 0xFFFFFFFF, *SysString(STR_AcceptedQuest), 0,
                     0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
    if (QuestList)
    {
        // C2: how many quests are accepted, of 15
        TCHAR CountText[16];
        appSprintf(CountText, TEXT("(%d/%d)"), QuestList->NumLines, 15);
        Device->DrawText(190, 3, 0xFFB09B79, CountText, 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
    }
    Device->EndPaint();
    Device->ReleaseClip();
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x100bfb50, NCQuestWnd, OnPaint)

// 0x100bfd40 (vslot 68) - whatever the answer: the Cancel question only reports OK
INT NCQuestWnd::OnDialogResult(WPARAM wParam, LPARAM lParam)
{
    GConsole->RequestDestroyQuest(QuestList->GetItemParam(QuestList->Selected));
    return 1;
}
NW_HOOK_METHOD(0x100bfd40, NCQuestWnd, OnDialogResult)

// 0x100bfd70 (vslot 69) - asks before giving up the selected quest (systemmsg 182)
INT NCQuestWnd::OnCancel()
{
    guard(NCQuestWnd::OnCancel);
    if (QuestList->Selected >= 0 && QuestList->Selected < QuestList->NumLines
        && QuestList->GetItemParam(QuestList->Selected) >= 0)
    {
        NDialogInfo Info;
        Info.ID = DIALOG_CANCEL_QUEST;
        Info.bMainBox = 1;
        Info.bInput = 0;
        Info.Owner = Entry;
        Info.SetSystemMessage(182);  // "Do you want to quit the current quest?"
        GConsole->ShowDialog(&Info);
    }
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x100bfd70, NCQuestWnd, OnCancel)

// 0x100bfe70 (vslot 70) - the description page shows the quest picked in the list
INT NCQuestWnd::OnSelectQuest(INT Index)
{
    InfoWnd->ShowQuest(Index);
    return 1;
}
NW_HOOK_METHOD(0x100bfe70, NCQuestWnd, OnSelectQuest)

// 0x100c0aa0 - the list, the descriptions and the items go
void NCQuestWnd::Clear()
{
    QuestList->Clear();
    InfoWnd->ClearQuests();
    ItemWnd->Clear();
}
NW_HOOK_METHOD(0x100c0aa0, NCQuestWnd, Clear)

// 0x100c0ad0 (vslot 43) - emptied, then filled again by the server's answer; the arrows come and go with the
// description page
void NCQuestWnd::ShowWindow()
{
    Clear();
    GConsole->RequestQuestList();
    NCWnd::ShowWindow();
    TabCtrl->ShowCurrentPage();
    if (CurTab)
    {
        CancelButton->HideWindow();
        NewChild[1]->HideWindow();
        NewChild[0]->HideWindow();
    }
    else
    {
        CancelButton->ShowWindow();
        NewChild[1]->ShowWindow();
        NewChild[0]->ShowWindow();
    }
}
NW_HOOK_METHOD(0x100c0ad0, NCQuestWnd, ShowWindow)

// 0x100c01a0 (C1 0x100a1600) - a quest: its name goes to the list with the quest id as the parameter, its
// steps (Step: the server's bit set or count) to the description page
INT NCQuestWnd::AddQuest(INT QuestID, const TCHAR* Name, INT Step)
{
    QuestList->AddString(Name, 0xFFFFFFFF, QuestID);
    InfoWnd->AddQuest(QuestID, Step);
    return 1;
}
NW_HOOK_METHOD(0x100c01a0, NCQuestWnd, AddQuest)

// 0x100c01e0 - a quest item goes to the item page (C1 0x100a1640)
INT NCQuestWnd::AddQuestItem(NItemInfo* Info)
{
    ItemWnd->AddItem(Info);
    return 1;
}
NW_HOOK_METHOD(0x100c01e0, NCQuestWnd, AddQuestItem)

// 0x100c0480 (vslot 36) - NCWnd::OnCreate is not called (no title bar here); the list and both pages are
// created with the style bit 0x20. C2: everything is moved down by the title bar's height when the window has
// one (FrameCtrl), and the two arrows of the description page are new
INT NCQuestWnd::OnCreate()
{
    INT Y = FrameCtrl ? 0x15 : 0;
    TopTex = FindTexture(TEXT("L2UI.QuestWnd.questBack1"));
    InfoBackTex = FindTexture(TEXT("L2UI.QuestWnd.questBack2"));
    ItemBackTex = FindTexture(TEXT("L2UI.QuestWnd.questBack3"));
    ItemPageIconTex = FindTexture(TEXT("sek.cbui303"));

    QuestList = new NCListBox(165, 5, 16);
    QuestList->ID = NID_QUEST_LIST;
    QuestList->SetWindowRect(17, Y + 31, 220, 74);
    CreateChild(QuestList, NWS_VISIBLE | NWS_UNKNOWN20);   // 0x22
    QuestList->TextX = 1;
    QuestList->TextY = 1;
    QuestList->LineGap = 1;

    InfoWnd = new NCQuestInfoWnd();
    InfoWnd->SetWindowRect(18, Y + 150, 220, 122);
    CreateChild(InfoWnd, NWS_VISIBLE | NWS_UNKNOWN20);

    ItemWnd = new NCQuestItemWnd(6, 4, 80, 32);
    ItemWnd->SetWindowRect(16, Y + 144, 220, 139);
    CreateChild(ItemWnd, NWS_VISIBLE | NWS_UNKNOWN20);

    TabCtrl = new NCTabControl();
    TabCtrl->ID = NID_QUEST_TAB;
    TabCtrl->SetWindowRect(15, Y + 117, 138, 15);
    CreateChild(TabCtrl, NWS_VISIBLE);
    TabCtrl->AddTab(STR_Description, InfoWnd, 69, 16, TEXT("L2UI.NWindow.Tab2"), TEXT("L2UI.NWindow.Tab1"),
                    0, -1);
    TabCtrl->AddTab(STR_Item, ItemWnd, 69, 16, TEXT("L2UI.NWindow.Tab2"), TEXT("L2UI.NWindow.Tab1"), 0,
                    -1);

    CancelButton = new NCButton(TEXT("L2UI.Control.Defaultbutton"), TEXT("L2UI.Control.Defaultbutton_click"), 0);
    CancelButton->ID = NID_QUEST_CANCEL;
    CancelButton->SetWindowRect(87, Y + 277, 67, 19);
    CreateChild(CancelButton, NWS_VISIBLE);
    CancelButton->SetTextID(STR_Cancel);

    // the arrows under the description: forward (command 0xF8) and back (0xF9)
    NCPushButton* Next = new NCPushButton(TEXT("L2UI.ShortcutWnd.RightBtn1"), TEXT("L2UI.ShortcutWnd.RightBtn2"));
    NewChild[1] = Next;
    Next->ID = NID_QUEST_NEW1;
    Next->SetWindowRect(183, Y + 279, 13, 13);
    CreateChild(Next, NWS_VISIBLE);
    Next->RepeatDelay = 180;

    NCPushButton* Prev = new NCPushButton(TEXT("L2UI.ShortcutWnd.LeftBtn1"), TEXT("L2UI.ShortcutWnd.LeftBtn2"));
    NewChild[0] = Prev;
    Prev->ID = NID_QUEST_NEW2;
    Prev->SetWindowRect(45, Y + 279, 13, 13);
    CreateChild(Prev, NWS_VISIBLE);
    Prev->RepeatDelay = 180;
    return 1;
}
NW_HOOK_METHOD(0x100c0480, NCQuestWnd, OnCreate)

NW_CHECK_VTABLE(NCQuestWnd, 0x10116b38, 74)
