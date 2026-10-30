/*=============================================================================
    NCPledgeWnd.cpp - the clan page. Stock: 0x10033da0 .. 0x10034053, 0x10034060 .. 0x10034203,
    0x10034210 .. 0x100342b9, 0x10034f50 .. 0x1003504f, 0x10035050 .. 0x100360d9,
    0x100360e0 .. 0x100364b5, 0x10036dc0 .. 0x10036e14 (the rest of the range is
    NCActionWnd's and NCActionItemWnd's).
=============================================================================*/
#include "nwindow/NCPledgeWnd.h"
#include "nwindow/NCListCtrl.h"
#include "nwindow/NCButton.h"
#include "nwindow/NCGDevice.h"
#include "nwindow/NConsoleWnd.h"
#include "nwindow/NDialogInfo.h"
#include "nwindow/NCDialogBox.h"
#include "nwindow/NCConsole.h"
#include "nwindow/NCEmpowerWnd.h"

#include <wchar.h>

// systemmsg lines
enum
{
    MSG_TargetNotFound = 50,          // "Target can not be found."
    MSG_ConfirmWithdraw = 68,         // "Withdraw from the $s1 Clan. Do you want to continue?"
    MSG_ConfirmExpel = 69,            // "Expel $s1 from the clan. Do you want to continue?"
    MSG_SelectInvitee = 186,          // "Select user to invite to your clan."
    MSG_EnterWarTarget = 206,         // "Enter name of clan to declare war on."
    MSG_CrestFormat = 211,            // "Can only register 16*12 sized bmp files of 256 colors."
    MSG_LeaderCannotWithdraw = 239,   // "The clan leader cannot withdraw."
    MSG_EnterSurrenderTarget = 252,   // "Enter the name of clan to surrender to."
    MSG_EnterEndWarTarget = 253,      // "Enter the name of clan to request end of war."
    MSG_EnterName = 256,              // "Enter name."
    MSG_SelectMember = 259,           // "Select clan members from list."
    MSG_NoPowerTarget = 145,          // (0x91; text not read) OnNewButton2's dialog for a selected member without an id
};

enum { DIALOG_SystemMessage = 0x1F };   // NDialogInfo::ID: a message, OK only

// sysstring lines
enum
{
    STR_Name = 50,
    STR_Lv = 88,
    STR_Leader = 342,
    STR_Base = 343,
    STR_HideoutExists = 344,
    STR_CastleExists = 345,
    STR_Status = 346,
    STR_On = 347,
    STR_Out = 348,
};

// dialogs of the buttons (NCDialogBox::OnOK / OnCancel act on the ID)
enum
{
    DIALOG_GiveTitle = 0xF,
    DIALOG_RegisterCrest = 0x10,
    DIALOG_DeclareWar = 0x12,
    DIALOG_EndWar = 0x14,
    DIALOG_Surrender = 0x17,
    DIALOG_Expel = 0x18,
    DIALOG_SurrenderPersonally = 0x19,
    DIALOG_Withdraw = 0x1A,
};

enum { CLAN_TEXT_COLOR = 0xFFB09B79, CLAN_LABEL_COLOR = 0xFFA3A3A3 };

// one button of the page (repeated inline in the stock OnCreate)
static NCButton* CreateButton(NCWnd* Parent, INT ID, INT X, INT Y, INT TextID)
{
    NCButton* Button = new NCButton(TEXT("L2UI.control.DefaultButton"),
                                    TEXT("L2UI.control.DefaultButton_click"), 0);
    Button->ID = ID;
    Button->SetWindowRect(X, Y, 67, 19);
    Parent->CreateChild(Button, NWS_VISIBLE);
    Button->SetTextID(TextID);
    return Button;
}

// one child of OnDestroy (repeated inline in the stock code)
template <class T> static void DestroyChild(NCWnd* Parent, T*& Child)
{
    if (Child)
    {
        Parent->RemoveChildWindow(Child->Entry);
        delete Child;
        Child = NULL;
    }
}

// the member list columns (repeated inline in the stock OnCreate)
static void AddMemberColumn(NCListCtrl* List, INT Width, INT TitleID, UBOOL bNumeric)
{
    NListColumnInfo Info;
    Info.Width = Width;
    Info.Height = 15;
    Info.bNumeric = bNumeric;
    Info.SetTitleID(TitleID);
    Info.SetNormalTex(TEXT("L2UI.Control.listbutton"));
    Info.SetPushedTex(TEXT("L2UI.Control.listbutton_click"));
    List->AddColumn(0, &Info);
}

// 0x10040f70
NCPledgeWnd::NCPledgeWnd()
{
    BackTex = NULL;
    CrestTex = NULL;
    Info.Name[0] = 0;
    Info.LeaderName[0] = 0;
    Info.StatusText[0] = 0;
    Info.PledgeID = 0;
    Info.CrestID = 0;
    Info.UnknownE4 = 0;
    Info.Level = 0;
    Info.bHasCastle = 0;
    Info.bHasHideout = 0;
    Info.Unknown94 = 0;
    Info.Unknown98 = 0;
    Info.Status = 0;
    Info.UnknownE0 = 0;
    MemberList = NULL;
}
NW_HOOK_CTOR(0x10040f70, NCPledgeWnd)

// 0x10041020
NW_HOOK_DTOR(0x10041020, NCPledgeWnd)

// 0x10040260 (vslot 25) - C2 has no IsEnabled check of its own, and two more buttons
INT NCPledgeWnd::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (NCWnd::WindowProc(Msg, wParam, lParam))
        return 1;
    if (Msg == NWM_COMMAND)
    {
        if ((INT)wParam == NID_PLEDGE_INVITE)
            return OnInvite() != 0;
        if ((INT)wParam == NID_PLEDGE_TITLE)
            return OnGiveTitle() != 0;
        if ((INT)wParam == NID_PLEDGE_CREST)
            return OnSetCrest() != 0;
        if ((INT)wParam == NID_PLEDGE_WAR)
            return OnDeclareWar() != 0;
        if ((INT)wParam == NID_PLEDGE_TRUCE)
            return OnEndWar() != 0;
        if ((INT)wParam == NID_PLEDGE_SURRENDER)
            return OnSurrender() != 0;
        if ((INT)wParam == NID_PLEDGE_EXPEL)
            return OnExpel() != 0;
        if ((INT)wParam == NID_PLEDGE_YIELD)
            return OnSurrenderPersonally() != 0;
        if ((INT)wParam == NID_PLEDGE_LEAVE)
            return OnWithdraw() != 0;
        if ((INT)wParam == NID_PLEDGE_NEW1)
            return OnNewButton1() != 0;
        if ((INT)wParam == NID_PLEDGE_NEW2)
            return OnNewButton2() != 0;
    }
    else if (Msg == (UINT)NWM_DIALOG_RESULT)
        return OnDialogResult(wParam, lParam) != 0;
    return 0;
}
NW_HOOK_METHOD(0x10040260, NCPledgeWnd, WindowProc)

// 0x10041070 (vslot 36) - the member list (name, level, on / out) and eleven buttons; the page is
// 17 pixels shorter than C1's (the buttons moved up) and two buttons are new
INT NCPledgeWnd::OnCreate()
{
    NCWnd::OnCreate();
    BackTex = FindTexture(TEXT("L2UI.ActionWnd.PledgeBack"));

    MemberList = new NCListCtrl(1000, 7, 3, 12);
    MemberList->SetWindowRect(15, 82, 221, 123);
    CreateChild(MemberList, NWS_VISIBLE | 0x20);   // 0x20: meaning unknown
    MemberList->SetTextOffset(0, 3);
    AddMemberColumn(MemberList, 128, STR_Name, 0);
    AddMemberColumn(MemberList, 28, STR_Lv, 1);
    AddMemberColumn(MemberList, 53, STR_Status, 0);

    InviteButton = CreateButton(this, NID_PLEDGE_INVITE, 15, 198, 330);        // "Invite"
    TitleButton = CreateButton(this, NID_PLEDGE_TITLE, 15, 217, 331);          // "Title"
    CrestButton = CreateButton(this, NID_PLEDGE_CREST, 15, 236, 332);          // "Set Crest"
    WarButton = CreateButton(this, NID_PLEDGE_WAR, 86, 198, 333);              // "Battle"
    TruceButton = CreateButton(this, NID_PLEDGE_TRUCE, 86, 217, 334);          // "Truce"
    SurrenderButton = CreateButton(this, NID_PLEDGE_SURRENDER, 86, 236, 335);  // "Surrender"
    ExpelButton = CreateButton(this, NID_PLEDGE_EXPEL, 157, 198, 338);         // "Dismiss"
    YieldButton = CreateButton(this, NID_PLEDGE_YIELD, 157, 217, 336);         // "Yield"
    LeaveButton = CreateButton(this, NID_PLEDGE_LEAVE, 157, 236, 337);         // "Leave"
    NewButton6F4 = CreateButton(this, NID_PLEDGE_NEW1, 15, 255, 666);          // new in C2
    NewButton6F8 = CreateButton(this, NID_PLEDGE_NEW2, 86, 255, 667);          // new in C2
    return 1;
}
NW_HOOK_METHOD(0x10041070, NCPledgeWnd, OnCreate)

// 0x10041d90 (vslot 37) - C2 guards it; the two new buttons go last
INT NCPledgeWnd::OnDestroy()
{
    guard(NCPledgeWnd::OnDestroy);
    DestroyChild(this, MemberList);
    DestroyChild(this, InviteButton);
    DestroyChild(this, TitleButton);
    DestroyChild(this, CrestButton);
    DestroyChild(this, WarButton);
    DestroyChild(this, TruceButton);
    DestroyChild(this, SurrenderButton);
    DestroyChild(this, ExpelButton);
    DestroyChild(this, YieldButton);
    DestroyChild(this, LeaveButton);
    DestroyChild(this, NewButton6F4);
    DestroyChild(this, NewButton6F8);
    NCWnd::OnDestroy();
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x10041d90, NCPledgeWnd, OnDestroy)

// a clan privilege byte of the player (C2: three bytes at User +0x1EE, +0x1EF, +0x1F0, new; the
// engine header only has them in a filler): invite, title, crest
static FORCEINLINE UBOOL PledgeRight(User* Me, INT Offset)
{
    return *((const BYTE*)Me + Offset) != 0;
}

// 0x10040420 (vslot 43) - leader: everything but Leave; members: only Leave (and Yield,
// which is never touched); C2 also opens invite / title / crest to a member holding that right
void NCPledgeWnd::ShowWindow()
{
    NCWnd::ShowWindow();
    User* Me = GConsole->GetMyUser();
    if (!Me)
        return;
    UBOOL bLeader = (Me->StatusFlags >> 6) & 1;
    if (bLeader)
    {
        InviteButton->EnableWindow(1);
        TitleButton->EnableWindow(1);
        CrestButton->EnableWindow(1);
        WarButton->EnableWindow(1);
        TruceButton->EnableWindow(1);
        SurrenderButton->EnableWindow(1);
        ExpelButton->EnableWindow(1);
    }
    else
    {
        InviteButton->EnableWindow(0);
        TitleButton->EnableWindow(0);
        CrestButton->EnableWindow(0);
        WarButton->EnableWindow(0);
        TruceButton->EnableWindow(0);
        SurrenderButton->EnableWindow(0);
        ExpelButton->EnableWindow(0);
    }
    LeaveButton->EnableWindow(!bLeader);
    if (PledgeRight(Me, 0x1EE))
        InviteButton->EnableWindow(1);
    if (PledgeRight(Me, 0x1EF))
        TitleButton->EnableWindow(1);
    if (PledgeRight(Me, 0x1F0))
        CrestButton->EnableWindow(1);
}
NW_HOOK_METHOD(0x10040420, NCPledgeWnd, ShowWindow)

// 0x100419e0 (vslot 50) - the clip opens 17 pixels above the window (the tab row), before
// BeginPaint, and closes after EndPaint, as in the stock code. StatusText is drawn after the
// leader's name, measured from the leader's name, but nothing sets it.
INT NCPledgeWnd::OnPaint(NCGDevice* Device)
{
    Device->SetClip(ScreenX, ScreenY - 17, Width, Height + 17);
    Device->BeginPaint(ScreenX, ScreenY);
    if (BackTex)
        Device->DrawTexture(0, -17, 240, 301, 0, 0, 240, 301, BackTex);

    INT NameX = 18;
    if (Info.CrestID > 0)
    {
        UTexture* Crest = GConsole->GetPledgeCrestTexture(Info.PledgeID);
        if (Crest)
        {
            Device->DrawTexture(18, 18, 16, 12, 0, 4, 16, 12, Crest);
            NameX = 38;
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
        Device->DrawText(17, 33, CLAN_LABEL_COLOR, *SysString(STR_Leader), 0, 0, 0,
                         L2FONT_Normal, 0, 0, 0, 0, 0, 0);
    const TCHAR* Leader = Info.LeaderName;
    if (Leader && *Leader)
        Device->DrawText(60, 33, CLAN_TEXT_COLOR, Leader, 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
    const TCHAR* Status = Info.StatusText;
    if (Status && *Status)
    {
        NCGDevice::GetTextExtent(&Size, Leader, 0, 0, 0, 0, 0);
        Device->DrawText(Size.cx + 48, 33, CLAN_TEXT_COLOR, Status, 0, 0, 0, L2FONT_Normal, 0, 0, 0,
                         0, 0, 0);
    }
    if (*SysString(STR_Lv))
        Device->DrawText(169, 33, CLAN_LABEL_COLOR, *SysString(STR_Lv), 0, 0, 0,
                         L2FONT_Normal, 0, 0, 0, 0, 0, 0);   // C2: 30 pixels right of C1's
    TCHAR Number[10];
    appSprintf(Number, TEXT("%d"), Info.Level);
    Device->DrawText(188, 33, CLAN_TEXT_COLOR, Number, 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);

    if (*SysString(STR_Base))
        Device->DrawText(17, 47, CLAN_LABEL_COLOR, *SysString(STR_Base), 0, 0, 0,
                         L2FONT_Normal, 0, 0, 0, 0, 0, 0);
    if (Info.bHasCastle)
    {
        Device->DrawText(60, 47, CLAN_TEXT_COLOR, *SysString(STR_CastleExists), 0, 0,
                         0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
        if (Info.bHasHideout)
        {
            NCGDevice::GetTextExtent(&Size, *SysString(STR_CastleExists), 0, 0, 0, 0,
                                     0);
            Device->DrawText(Size.cx + 62, 47, CLAN_TEXT_COLOR,
                             *SysString(STR_HideoutExists), 0, 0, 0, L2FONT_Normal, 0,
                             0, 0, 0, 0, 0);
        }
    }
    else if (Info.bHasHideout)
        Device->DrawText(60, 47, CLAN_TEXT_COLOR, *SysString(STR_HideoutExists), 0, 0,
                         0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
    Device->EndPaint();
    Device->ReleaseClip();
    return 1;
}
NW_HOOK_METHOD(0x100419e0, NCPledgeWnd, OnPaint)

// 0x10040750 (vslot 62) - the target, when it is a player
INT NCPledgeWnd::OnInvite()
{
    User* Target = GConsole->GetSelectedActor();
    if (Target && !Target->Unknown8)
    {
        L2ParamStack Params(10);
        Params.PushBack((void*)Target->Pawn->CreatureID);
        GConsole->RequestJoinPledge(Params);
    }
    else
        GConsole->ShowSystemMessage(MSG_SelectInvitee, CLAN_TEXT_COLOR);
    return 1;
}
NW_HOOK_METHOD(0x10040750, NCPledgeWnd, OnInvite)

// 0x10042430 (vslot 63) - the file name is typed in the box
INT NCPledgeWnd::OnSetCrest()
{
    NDialogInfo Dialog;
    Dialog.ID = DIALOG_RegisterCrest;
    Dialog.bMainBox = 1;
    Dialog.bInput = 0;
    Dialog.Type = 0;
    Dialog.SetSystemMessage(MSG_CrestFormat);
    GConsole->ShowDialog(&Dialog);
    return 1;
}
NW_HOOK_METHOD(0x10042430, NCPledgeWnd, OnSetCrest)

// 0x100424d0 (vslot 64)
INT NCPledgeWnd::OnDeclareWar()
{
    NDialogInfo Dialog;
    Dialog.ID = DIALOG_DeclareWar;
    Dialog.bMainBox = 1;
    Dialog.bInput = 1;
    Dialog.Type = 0;
    Dialog.SetSystemMessage(MSG_EnterWarTarget);
    GConsole->ShowDialog(&Dialog);
    return 1;
}
NW_HOOK_METHOD(0x100424d0, NCPledgeWnd, OnDeclareWar)

// 0x10042570 (vslot 65)
INT NCPledgeWnd::OnEndWar()
{
    NDialogInfo Dialog;
    Dialog.ID = DIALOG_EndWar;
    Dialog.bMainBox = 1;
    Dialog.bInput = 1;
    Dialog.Type = 0;
    Dialog.SetSystemMessage(MSG_EnterEndWarTarget);
    GConsole->ShowDialog(&Dialog);
    return 1;
}
NW_HOOK_METHOD(0x10042570, NCPledgeWnd, OnEndWar)

// 0x10042610 (vslot 66)
INT NCPledgeWnd::OnSurrender()
{
    NDialogInfo Dialog;
    Dialog.ID = DIALOG_Surrender;
    Dialog.bMainBox = 1;
    Dialog.bInput = 1;
    Dialog.Type = 0;
    Dialog.SetSystemMessage(MSG_EnterSurrenderTarget);
    GConsole->ShowDialog(&Dialog);
    return 1;
}
NW_HOOK_METHOD(0x10042610, NCPledgeWnd, OnSurrender)

// 0x100426b0 (vslot 67) - the selected member's name goes in the first button's text as well
// as in the parameter, as in the stock code
INT NCPledgeWnd::OnExpel()
{
    NListRow* Row = MemberList->GetSelectedRow();
    if (!Row)
    {
        GConsole->ShowSystemMessage(MSG_SelectMember, CLAN_TEXT_COLOR);
        return 1;
    }
    const TCHAR* Member = Row->Cells->Text;
    if (Member && *Member)
    {
        NDialogInfo Dialog;
        Dialog.ID = DIALOG_Expel;
        Dialog.bMainBox = 1;
        Dialog.bInput = 0;
        Dialog.SetSystemMessage(MSG_ConfirmExpel);
        Dialog.SetButtons(Row->Cells->Text, NULL);
        Dialog.SetParamText(Row->Cells->Text);
        GConsole->ShowDialog(&Dialog);
    }
    return 1;
}
NW_HOOK_METHOD(0x100426b0, NCPledgeWnd, OnExpel)

// 0x100427a0 (vslot 68) - the same message as OnSurrender
INT NCPledgeWnd::OnSurrenderPersonally()
{
    NDialogInfo Dialog;
    Dialog.ID = DIALOG_SurrenderPersonally;
    Dialog.bMainBox = 1;
    Dialog.bInput = 1;
    Dialog.Type = 0;
    Dialog.SetSystemMessage(MSG_EnterSurrenderTarget);
    GConsole->ShowDialog(&Dialog);
    return 1;
}
NW_HOOK_METHOD(0x100427a0, NCPledgeWnd, OnSurrenderPersonally)

// 0x10042840 (vslot 69) - the clan's name goes in the first button's text
INT NCPledgeWnd::OnWithdraw()
{
    User* Me = GConsole->GetMyUser();
    if (Me && !((Me->StatusFlags >> 6) & 1))
    {
        NDialogInfo Dialog;
        Dialog.ID = DIALOG_Withdraw;
        Dialog.bMainBox = 1;
        Dialog.bInput = 0;
        Dialog.SetSystemMessage(MSG_ConfirmWithdraw);
        Dialog.SetButtons(Info.Name, NULL);
        GConsole->ShowDialog(&Dialog);
        return 1;
    }
    GConsole->ShowSystemMessage(MSG_LeaderCannotWithdraw, CLAN_TEXT_COLOR);
    return 1;
}
NW_HOOK_METHOD(0x10042840, NCPledgeWnd, OnWithdraw)

// 0x10042360 (vslot 70) - the title of the target, when it is a player
INT NCPledgeWnd::OnGiveTitle()
{
    User* Target = GConsole->GetSelectedActor();
    if (Target && !Target->Unknown8)
    {
        NDialogInfo Dialog;
        Dialog.ID = DIALOG_GiveTitle;
        Dialog.bMainBox = 1;
        Dialog.bInput = 1;
        Dialog.Type = 0;
        Dialog.SetSystemMessage(MSG_EnterName);
        GConsole->ShowDialog(&Dialog);
        return 1;
    }
    GConsole->ShowSystemMessage(MSG_TargetNotFound, CLAN_TEXT_COLOR);
    return 1;
}
NW_HOOK_METHOD(0x10042360, NCPledgeWnd, OnGiveTitle)

// 0x10040590 - the status text, UnknownE0 and UnknownE4 are left out; the names only when
// given
void NCPledgeWnd::SetPledgeInfo(NPledgeInfo* InInfo)
{
    Info.PledgeID = InInfo->PledgeID;
    if (*InInfo->Name)
        appStrcpy(Info.Name, InInfo->Name);
    const TCHAR* Leader = InInfo->LeaderName;
    if (Leader && *Leader)
        appStrcpy(Info.LeaderName, Leader);
    Info.CrestID = InInfo->CrestID;
    Info.Level = InInfo->Level;
    Info.bHasCastle = InInfo->bHasCastle;
    Info.bHasHideout = InInfo->bHasHideout;
    Info.Unknown94 = InInfo->Unknown94;
    Info.Unknown98 = InInfo->Unknown98;
    Info.Status = InInfo->Status;
    Info.UnknownE8 = InInfo->UnknownE8;
}
NW_HOOK_METHOD(0x10040590, NCPledgeWnd, SetPledgeInfo)

// 0x10040650 - the same without the ID and the names
void NCPledgeWnd::UpdatePledgeInfo(NPledgeInfo* InInfo)
{
    Info.CrestID = InInfo->CrestID;
    Info.Level = InInfo->Level;
    Info.bHasCastle = InInfo->bHasCastle;
    Info.bHasHideout = InInfo->bHasHideout;
    Info.Unknown94 = InInfo->Unknown94;
    Info.Unknown98 = InInfo->Unknown98;
    Info.Status = InInfo->Status;
    Info.UnknownE8 = InInfo->UnknownE8;
}
NW_HOOK_METHOD(0x10040650, NCPledgeWnd, UpdatePledgeInfo)

// 0x100406c0
const TCHAR* NCPledgeWnd::GetPledgeName()
{
    const TCHAR* Name = Info.Name;
    if (!Name || !*Name)
        return NULL;
    return Name;
}
NW_HOOK_METHOD(0x100406c0, NCPledgeWnd, GetPledgeName)

// 0x100406e0
void NCPledgeWnd::ClearMembers()
{
    MemberList->ClearRows();
}
NW_HOOK_METHOD(0x100406e0, NCPledgeWnd, ClearMembers)

// 0x100406f0 - UnknownE8 is kept
void NCPledgeWnd::ClearInfo()
{
    Info.Name[0] = 0;
    Info.LeaderName[0] = 0;
    Info.StatusText[0] = 0;
    Info.PledgeID = 0;
    Info.CrestID = 0;
    Info.UnknownE4 = 0;
    Info.Level = 0;
    Info.bHasCastle = 0;
    Info.bHasHideout = 0;
    Info.Unknown94 = 0;
    Info.Unknown98 = 0;
    Info.Status = 0;
    Info.UnknownE0 = 0;
}
NW_HOOK_METHOD(0x100406f0, NCPledgeWnd, ClearInfo)

// 0x10042020
void NCPledgeWnd::Reset()
{
    ClearInfo();
    ClearMembers();
}
NW_HOOK_METHOD(0x10042020, NCPledgeWnd, Reset)

// 0x10042030 - a member: the row with that name takes the level and the on / off text; a new row is made of the name,
// the level and that text, with the row's params (bOnline, 0, -1) (C2: three params; online means above 0)
void NCPledgeWnd::AddMember(NPledgeMember* Member)
{
    TArray<NListRow*>* Rows = MemberList->GetRows();
    for (INT i = 0; i < Rows->Num(); i++)
    {
        const TCHAR* Name = Member->Name;
        const TCHAR* RowName = (*Rows)(i)->Cells->Text;
        if (Name && *Name && RowName && *RowName && !_wcsicmp(RowName, Name))
        {
            (*Rows)(i)->Cells[1].SetNumber(Member->Level);
            if (Member->bOnline < 1)
            {
                if (*NCObject::SysString(STR_Out))
                    (*Rows)(i)->Cells[2].SetText(*NCObject::SysString(STR_Out));
            }
            else if (*NCObject::SysString(STR_On))
                (*Rows)(i)->Cells[2].SetText(*NCObject::SysString(STR_On));
            return;
        }
    }
    NListRow* Row = new NListRow(3);
    Row->Cells[0].SetText(Member->Name);
    Row->Cells[1].SetNumber(Member->Level);
    if (Member->bOnline < 1)
    {
        if (*NCObject::SysString(STR_Out))
            Row->Cells[2].SetText(*NCObject::SysString(STR_Out));
    }
    else if (*NCObject::SysString(STR_On))
        Row->Cells[2].SetText(*NCObject::SysString(STR_On));
    Row->SetParams(Member->bOnline, 0, -1);
    MemberList->AddRow(Row);
}
NW_HOOK_METHOD(0x10042030, NCPledgeWnd, AddMember)

// 0x10042240 - members not listed are ignored; an online member's row also gets the params (bOnline, 0, -1), an
// offline one's row keeps the old ones
void NCPledgeWnd::UpdateMember(NPledgeMember* Member)
{
    TArray<NListRow*>* Rows = MemberList->GetRows();
    for (INT i = 0; i < Rows->Num(); i++)
    {
        const TCHAR* Name = Member->Name;
        const TCHAR* RowName = (*Rows)(i)->Cells->Text;
        if (Name && *Name && RowName && *RowName && !_wcsicmp(RowName, Name))
        {
            (*Rows)(i)->Cells[1].SetNumber(Member->Level);
            if (Member->bOnline > 0)
            {
                if (*NCObject::SysString(STR_On))
                    (*Rows)(i)->Cells[2].SetText(*NCObject::SysString(STR_On));
                (*Rows)(i)->SetParams(Member->bOnline, 0, -1);
            }
            else if (*NCObject::SysString(STR_Out))
                (*Rows)(i)->Cells[2].SetText(*NCObject::SysString(STR_Out));
            return;
        }
    }
}
NW_HOOK_METHOD(0x10042240, NCPledgeWnd, UpdateMember)

// 0x10043340 - the row leaves the list but is not deleted (the stock code does not free it either)
void NCPledgeWnd::DeleteMember(const TCHAR* Name)
{
    TArray<NListRow*>* Rows = MemberList->GetRows();
    for (INT i = 0; i < Rows->Num(); i++)
    {
        if (Name && !_wcsicmp((*Rows)(i)->Cells->Text, Name))
        {
            Rows->Remove(i, 1);
            return;
        }
    }
}
NW_HOOK_METHOD(0x10043340, NCPledgeWnd, DeleteMember)

// 0x10042920 (vslot 76, new in C2; name ours) - the button 0xF019 (text 666): the player's own privileges in the clan.
// The server is asked for them (RequestPledgePower: the player's object id, mode 1 = view) and the empower window
// (NCConsole +0xA0) is emptied, told whose they are (the player's id and name) and opened; its SetPower fills the
// boxes when the server answers. The same stores are the stock helper 0x10040930 (NCEmpowerWnd, name ours:
// SetMember(MemberID, Name, Mode)) inlined here.
INT NCPledgeWnd::OnNewButton1()
{
    User* Me = GConsole->GetMyUser();
    if (Me)
    {
        GConsole->RequestPledgePower(Me->ObjectID, 1, NULL);
        NCEmpowerWnd* Empower = static_cast<NCEmpowerWnd*>(GDesktop->EmpowerWnd);
        Empower->Reset();
        Empower->MemberID = Me->ObjectID;
        Empower->Mode = 1;
        const TCHAR* MyName = (const TCHAR*)((BYTE*)Me + 0x1C);   // User's name: TCHAR[24] at +0x1C
        if (MyName)
            Empower->Name = MyName;
        static_cast<NCEmpowerWnd*>(GDesktop->EmpowerWnd)->ShowWindow();
    }
    return 1;
}
NW_HOOK_METHOD(0x10042920, NCPledgeWnd, OnNewButton1)

// 0x100429c0 (vslot 77, new in C2; name ours) - the button 0xF018 (text 667): the privileges of the member selected in
// the list, for the clan leader only (StatusFlags bit 6): an online member's row (its first param, the member's id:
// 0 = offline) asks the server (mode 2 = edit), empties the empower window, tells it whose they are (the id and the
// row's name) and opens it; with no id, an OK-only dialog box with the system message 0x91. Nothing when no row with
// a name is selected.
INT NCPledgeWnd::OnNewButton2()
{
    if ((GConsole->GetMyUser()->StatusFlags >> 6) & 1)
    {
        NListRow* Row = MemberList->GetSelectedRow();
        if (Row && Row->Cells && Row->Cells->Text[0])
        {
            if (Row->Param1 > 0)
            {
                GConsole->RequestPledgePower(Row->Param1, 2, NULL);
                NCEmpowerWnd* Empower = static_cast<NCEmpowerWnd*>(GDesktop->EmpowerWnd);
                Empower->Reset();
                Empower->MemberID = Row->Param1;   // 0x10040930 (SetMember) inlined, as in OnNewButton1
                Empower->Mode = 2;
                if (Row->Cells->Text)
                    Empower->Name = Row->Cells->Text;
                static_cast<NCEmpowerWnd*>(GDesktop->EmpowerWnd)->ShowWindow();
            }
            else
            {
                NDialogInfo Info;
                Info.ID = DIALOG_SystemMessage;
                Info.bMainBox = 1;
                Info.bInput = 0;
                Info.SetSystemMessage(MSG_NoPowerTarget);
                GConsole->ShowDialog(&Info);
            }
        }
    }
    return 1;
}
NW_HOOK_METHOD(0x100429c0, NCPledgeWnd, OnNewButton2)

NW_CHECK_VTABLE(NCPledgeWnd, 0x10111c08, 79)
