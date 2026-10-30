/*=============================================================================
    NCSiegeInfoMainWnd.cpp - the castle siege window, its tabs and its clan list.
    Stock: 0x100ab3a0 .. 0x100adf1c (the TArray<NSiegeClanInfo*> code at 0x100ad570 is the
    template's).
=============================================================================*/
#include "nwindow/NCSiegeInfoMainWnd.h"
#include "nwindow/NCButton.h"
#include "nwindow/NCComboBox.h"
#include "nwindow/NCFrameCtrl.h"
#include "nwindow/NCGDevice.h"
#include "nwindow/NCTabControl.h"
#include "nwindow/NConsoleWnd.h"
#include "nwindow/NDialogInfo.h"

// NWM_COMMAND ids (names ours)
enum
{
    NID_SIEGE_FRAME = 0xF22035,
    NID_SIEGE_TABS = 0xF22036,
    NID_SIEGE_APPLY = 0xF023A0,
    NID_SIEGE_CANCELAPPLY = 0xF023A1,
    NID_SIEGE_APPROVE = 0xF023A2,
    NID_SIEGE_REFUSE = 0xF023A3,
    NID_SIEGE_TIMECOMBO = 0xA0F311,
};

// what the date's dialog sends its owner on OK (NCDialogBox::OnOK, name ours)
enum { NWM_SIEGE_DATE_OK = (INT)0xF0000011 };

// NDialogInfo::ID (names ours): 0x3D answers the date tab, the others the server (NCDialogBox).
// C2: every id is 4 above C1's (0x39 .. 0x3F)
enum
{
    DIALOG_SIEGE_DATE = 0x3D,
    DIALOG_SIEGE_ATTACK = 0x3E,
    DIALOG_SIEGE_CANCEL_ATTACK = 0x3F,
    DIALOG_SIEGE_DEFEND = 0x40,
    DIALOG_SIEGE_CANCEL_DEFEND = 0x41,
    DIALOG_SIEGE_APPROVE = 0x42,
    DIALOG_SIEGE_REFUSE = 0x43,
};

// systemmsg lines
enum
{
    MSG_ConfirmSiegeTime = 663,      // "The siege time has been declared for $s. ..."
    MSG_ApplyAttack = 667,           // "You are registering on the attacking side of the $s1 siege. ..."
    MSG_ApplyDefense = 668,          // "... on the defending side of the $s1 siege. ..."
    MSG_CancelApplication = 669,
    MSG_RefuseDefender = 670,
    MSG_ApproveDefender = 671,
};

// sysstring lines
enum
{
    STR_Leader = 342,
    STR_Approved = 567,
    STR_Waiting = 568,
    STR_AllianceLeader = 569,
    STR_ApplyTime = 570,
    STR_AllianceName = 571,
    STR_Apply = 572,
    STR_CancelApply = 573,
    STR_Approve = 574,
    STR_Cancel = 575,                // the refuse button's
    STR_Refused = 579,               // a refused defender's status (C1 had a literal; "Refuse")
    STR_AttackingClans = 576,
    STR_DefendingClans = 577,
    STR_WaitingClans = 578,
    STR_ClanName = 580,
    STR_Status = 581,
    STR_CurrentTime = 582,
    STR_SiegeDate = 583,
    STR_Undecided = 584,
    STR_AutoSetting = 585,
    STR_SiegeInfo = 586,
    STR_DefenseInfo = 587,
    STR_Lord = 588,
    STR_Clan = 589,
    STR_Alliance = 590,
    STR_NoAlliance = 591,
    STR_NoCastleOwner = 595,
};

enum { SOUND_WindowOpen = 5 };  // interface sound (FL2NetNotify::OnInterfacePlaySound)

enum { SIEGE_TITLE_COLOR = 0xFFB09B79, SIEGE_LABEL_COLOR = 0xFFA3A3A3 };

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

// the siege window a tab belongs to
static FORCEINLINE NCSiegeInfoMainWnd* MainOf(NCWnd* Tab)
{
    return (NCSiegeInfoMainWnd*)Tab->Entry->Parent->Wnd;
}

// a cell's text, NULL when empty (inlined in the stock code)
static FORCEINLINE const TCHAR* CellText(NListCell& Cell)
{
    return (Cell.Text && Cell.Text[0]) ? Cell.Text : NULL;
}

/*-----------------------------------------------------------------------------
    NSiegeCastleInfo, NSiegeClanInfo.
-----------------------------------------------------------------------------*/

// 0x100d0290
__declspec(noinline) NSiegeCastleInfo& NSiegeCastleInfo::operator=(const NSiegeCastleInfo& Other)
{
    appStrcpy(AllianceName, Other.AllianceName);
    appStrcpy(LordName, Other.LordName);
    appStrcpy(ClanName, Other.ClanName);
    CastleID = Other.CastleID;
    bIsOwner = Other.bIsOwner;
    PledgeID = Other.PledgeID;
    AllianceID = Other.AllianceID;
    CurrentTime = Other.CurrentTime;
    SiegeTime = Other.SiegeTime;
    return *this;
}
NW_HOOK_METHOD(0x100d0290, NSiegeCastleInfo, operator=)

// 0x100d0320
__declspec(noinline) NSiegeClanInfo::NSiegeClanInfo(const NSiegeClanInfo& Other)
{
    appStrcpy(ClanName, Other.ClanName);
    appStrcpy(LeaderName, Other.LeaderName);
    appStrcpy(AllianceName, Other.AllianceName);
    appStrcpy(AllianceLeader, Other.AllianceLeader);
    PledgeID = Other.PledgeID;
    CrestID = Other.CrestID;
    ApplyTime = Other.ApplyTime;
    Type = Other.Type;
    AllianceID = Other.AllianceID;
}
NW_HOOK_CTOR_ARGS(0x100d0320, NSiegeClanInfo, (const NSiegeClanInfo& A), (A))

/*-----------------------------------------------------------------------------
    NCSiegeInfoListCtrl.
-----------------------------------------------------------------------------*/

// 0x100d23f0
NCSiegeInfoListCtrl::NCSiegeInfoListCtrl()
{
}
NW_HOOK_CTOR(0x100d23f0, NCSiegeInfoListCtrl)

// 0x100d2490
NCSiegeInfoListCtrl::NCSiegeInfoListCtrl(INT InMaxRows, INT InVisibleRows, INT InMaxColumns,
                                         INT Unused)
    : NCListCtrl(InMaxRows, InVisibleRows, InMaxColumns, Unused)
{
}
NW_HOOK_CTOR_ARGS(0x100d2490, NCSiegeInfoListCtrl, (INT A, INT B, INT C, INT D), (A, B, C, D))

// 0x100d2440
NW_HOOK_DTOR(0x100d2440, NCSiegeInfoListCtrl)

// 0x100d00e0 (vslot 25) - the base's answer as a plain 0 / 1
INT NCSiegeInfoListCtrl::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    return NCListCtrl::WindowProc(Msg, wParam, lParam) != 0;
}
NW_HOOK_METHOD(0x100d00e0, NCSiegeInfoListCtrl, WindowProc)

// 0x100d0480 (vslot 55) - NCListCtrl's paint (with its guard name), but the first column starts with the clan's
// crest and, on the attacking side, the second one with the alliance's; the cell icons are not drawn
INT NCSiegeInfoListCtrl::OnPaint(NCGDevice* Device)
{
    guard(NCListCtrl::OnPaint);
    Device->BeginPaint(ScreenX, ScreenY);
    Device->SetClip(ScreenX, ScreenY, Width, Height);
    if (BackTex)
        Device->DrawTexture(0, 0, Width, Height, 0, 0, Width, Height, BackTex);
    INT Line = 0;
    for (INT i = TopRow; i < TopRow + VisibleRows && i <= NumRows && i < Rows.Num(); i++, Line++)
    {
        Device->SetClip(ScreenX, ScreenY + HeaderHeight, Width, Height);
        if (i == Selected)
            Device->DrawRect(0, (LineGap + 12) * Line + TextY + HeaderHeight - 1, Width - 18,
                             LineGap + 12, 0xFF646464);
        Device->ReleaseClip();
        INT X = TextX;
        NListRow* Row = Rows(i);
        for (INT c = 0; c < Columns.Num(); c++)
        {
            NCHeaderCtrl* Column = Columns(c);
            INT Y = (LineGap + 12) * Line + TextY + HeaderHeight;
            Device->SetClip(ScreenX + X, ScreenY + Y - 1, Column->ColumnWidth, 14);
            if (c == 0)
            {
                UTexture* Crest = GConsole->GetPledgeCrestTexture(Row->Param1);
                if (Crest)
                    Device->DrawTexture(0, Y, 16, 12, 0, 4, 16, 12, Crest);
                Device->DrawText(17, Y, Row->Cells[0].Color, Row->Cells[0].Text, 0, 0, 0,
                                 L2FONT_Normal, 0, 0, 0, 0, 0, 0, 0);
            }
            else if (Row->Cells[0].Param2 == 0)
            {
                UTexture* Crest = GConsole->GetAllyCrestTexture(Row->Param1);
                if (Crest)
                    Device->DrawTexture(Row->Cells[c].IconW + X, Y, 8, 12, 0, 4, 8, 12, Crest);
                Device->DrawText(X + 9, Y, Row->Cells[c].Color, Row->Cells[c].Text, 0, 0, 0,
                                 L2FONT_Normal, 0, 0, 0, 0, 0, 0, 0);
            }
            else
            {
                Device->DrawText(X, Y, Row->Cells[c].Color, Row->Cells[c].Text, 0, 0, 0,
                                 L2FONT_Normal, 0, 0, 0, 0, 0, 0, 0);
            }
            X += Column->ColumnWidth;
            Device->ReleaseClip();
        }
    }
    Device->ReleaseClip();
    Device->EndPaint();
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x100d0480, NCSiegeInfoListCtrl, OnPaint)

// 0x100ab530  NOT PORTED (no C2 counterpart)
void NCSiegeInfoListCtrl::DrawRowToolTip(NCGDevice* Device, NListRow* Row, INT X, INT Y)
{
    NW_UNPORTED_NORET();
}

// 0x100d03b0 - the clan's id in the selected row's first parameter, 0 without a row
INT NCSiegeInfoListCtrl::GetSelectedPledgeID()
{
    if (Selected >= 0 && Selected < Rows.Num() && Rows(Selected))
        return Rows(Selected)->Param1;
    return 0;
}
NW_HOOK_METHOD(0x100d03b0, NCSiegeInfoListCtrl, GetSelectedPledgeID)

// 0x100d03e0 - the selected row's status text (its second cell): 3 when it is the system string 567, 2 for 568, else -1
INT NCSiegeInfoListCtrl::GetSelectedStatus()
{
    if (Selected >= 0 && Selected < Rows.Num() && Rows(Selected))
    {
        NListRow* Row = Rows(Selected);
        const TCHAR* Text = Row->Cells[1].Text;
        if (!Text || !*Text)
            Text = NULL;
        if (appStrcmp(*NCObject::SysString(567), Text) == 0)
            return 3;
        Text = Row->Cells[1].Text;
        if (!Text || !*Text)
            Text = NULL;
        if (appStrcmp(*NCObject::SysString(568), Text) == 0)
            return 2;
    }
    return -1;
}
NW_HOOK_METHOD(0x100d03e0, NCSiegeInfoListCtrl, GetSelectedStatus)

NW_CHECK_VTABLE(NCSiegeInfoListCtrl, 0x10125c28, 74)

/*-----------------------------------------------------------------------------
    NCSiegePartInfoWnd.
-----------------------------------------------------------------------------*/

// 0x100d24c0
NCSiegePartInfoWnd::NCSiegePartInfoWnd()
{
    List = NULL;
    Side = SIEGESIDE_Attacker;
    BackTex = FindTexture(TEXT("L2UI.SiegeWnd.siege_back2"));
}
NW_HOOK_CTOR(0x100d24c0, NCSiegePartInfoWnd)

// 0x100d2570
NW_HOOK_DTOR(0x100d2570, NCSiegePartInfoWnd)

// 0x100d0100 (vslot 25) - the four buttons; C2 does not test IsEnabled first
INT NCSiegePartInfoWnd::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (NCWnd::WindowProc(Msg, wParam, lParam))
        return 1;
    if (Msg == NWM_COMMAND)
    {
        if ((INT)wParam == NID_SIEGE_APPLY)
            return OnApply() != 0;
        if ((INT)wParam == NID_SIEGE_CANCELAPPLY)
            return OnCancelApply() != 0;
        if ((INT)wParam == NID_SIEGE_APPROVE)
            return OnApprove() != 0;
        if ((INT)wParam == NID_SIEGE_REFUSE)
            return OnRefuse() != 0;
    }
    return 0;
}
NW_HOOK_METHOD(0x100d0100, NCSiegePartInfoWnd, WindowProc)

// 0x100d25d0 (vslot 36) - the list (256 rows, 13 shown, 2 columns) and the buttons: apply and cancel share one
// place (UpdateButtons shows one of them), approve sits left of refuse. NCWnd's OnCreate is not called.
INT NCSiegePartInfoWnd::OnCreate()
{
    List = new NCSiegeInfoListCtrl(256, 13, 2, 12);
    List->SetWindowRect(15, 15, 223, 192);
    CreateChild(List, NWS_VISIBLE | NWS_UNKNOWN20);   // C2 gives the list the style 0x20 too
    List->SetTextOffset(0, 3);

    ApplyButton = new NCButton(TEXT("L2UI.Control.DefaultButton"),
                               TEXT("L2UI.Control.DefaultButton_click"), 0);
    ApplyButton->ID = NID_SIEGE_APPLY;
    ApplyButton->SetWindowRect(158, 204, 67, 19);
    CreateChild(ApplyButton, NWS_VISIBLE);
    ApplyButton->SetTextID(STR_Apply);

    CancelApplyButton = new NCButton(TEXT("L2UI.Control.DefaultButton"),
                                     TEXT("L2UI.Control.DefaultButton_click"), 0);
    CancelApplyButton->ID = NID_SIEGE_CANCELAPPLY;
    CancelApplyButton->SetWindowRect(158, 204, 67, 19);
    CreateChild(CancelApplyButton, NWS_VISIBLE);
    CancelApplyButton->SetTextID(STR_CancelApply);

    ApproveButton = new NCButton(TEXT("L2UI.Control.DefaultButton"),
                                 TEXT("L2UI.Control.DefaultButton_click"), 0);
    ApproveButton->ID = NID_SIEGE_APPROVE;
    ApproveButton->SetWindowRect(80, 204, 67, 19);
    CreateChild(ApproveButton, NWS_VISIBLE);
    ApproveButton->SetTextID(STR_Approve);

    RefuseButton = new NCButton(TEXT("L2UI.Control.DefaultButton"),
                                TEXT("L2UI.Control.DefaultButton_click"), 0);
    RefuseButton->ID = NID_SIEGE_REFUSE;
    RefuseButton->SetWindowRect(158, 204, 67, 19);
    CreateChild(RefuseButton, NWS_VISIBLE);
    RefuseButton->SetTextID(STR_Cancel);
    return 1;
}
NW_HOOK_METHOD(0x100d25d0, NCSiegePartInfoWnd, OnCreate)

// 0x100d1c20 (vslot 37) - NCWnd's first; the clans' copies are freed without a destructor, then the list and the
// four buttons go
INT NCSiegePartInfoWnd::OnDestroy()
{
    NCWnd::OnDestroy();
    for (INT i = 0; i < Clans.Num(); i++)
    {
        if (Clans(i))
            appFree(Clans(i));
    }
    Clans.Empty();
    DestroyChild(this, List);
    DestroyChild(this, ApplyButton);
    DestroyChild(this, CancelApplyButton);
    DestroyChild(this, ApproveButton);
    DestroyChild(this, RefuseButton);
    return 1;
}
NW_HOOK_METHOD(0x100d1c20, NCSiegePartInfoWnd, OnDestroy)

// 0x100d07c0 (vslot 55) - the picture and the count line over it: the attackers, or the defenders approved and waiting
INT NCSiegePartInfoWnd::OnPaint(NCGDevice* Device)
{
    TCHAR Text[128];
    Device->BeginPaint(ScreenX, ScreenY);
    Device->SetClip(ScreenX, ScreenY - 18, Width, Height);
    Device->DrawTexture(0, -18, 240, 246, 0, 0, 240, 246, BackTex);
    if (Side == SIEGESIDE_Attacker)
        appSprintf(Text, TEXT("%s : %d"), *NCObject::SysString(STR_AttackingClans), Clans.Num());
    else
        appSprintf(Text, TEXT("%s/%s : %d/%d"), *NCObject::SysString(STR_DefendingClans),
                   *NCObject::SysString(STR_WaitingClans), NumApproved, NumWaiting);
    Device->DrawText(14, 2, 0xFFFFFFFF, Text, 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
    Device->EndPaint();
    Device->ReleaseClip();
    return 1;
}
NW_HOOK_METHOD(0x100d07c0, NCSiegePartInfoWnd, OnPaint)

// 0x100d0a80 (vslot 67) - asks before applying: the dialog depends on the side (the message ids 0x29C / 0x29B, the
// dialog ids 0x40 / 0x3E), with the castle's name and id; the owner is not set
INT NCSiegePartInfoWnd::OnApply()
{
    NDialogInfo Info;
    INT MsgID;
    if (Side == SIEGESIDE_Defender)
    {
        Info.ID = 0x40;
        MsgID = 0x29C;
    }
    else
    {
        Info.ID = 0x3E;
        MsgID = 0x29B;
    }
    Info.SetSystemMessage(MsgID);
    Info.bMainBox = 1;
    Info.bInput = 0;
    Info.SetButtons(GConsole->GetCastleName(CastleID), NULL);
    Info.SetParams(CastleID, -1);
    GConsole->ShowDialog(&Info);
    return 1;
}
NW_HOOK_METHOD(0x100d0a80, NCSiegePartInfoWnd, OnApply)

// 0x100d0b60 (vslot 68) - asks before cancelling the application (dialog id 0x41 for the defenders, else 0x3F)
INT NCSiegePartInfoWnd::OnCancelApply()
{
    NDialogInfo Info;
    Info.bMainBox = 1;
    Info.bInput = 0;
    Info.ID = (Side == SIEGESIDE_Defender) * 2 + 0x3F;
    Info.SetSystemMessage(0x29D);
    Info.SetButtons(GConsole->GetCastleName(CastleID), NULL);
    Info.SetParams(CastleID, -1);
    GConsole->ShowDialog(&Info);
    return 1;
}
NW_HOOK_METHOD(0x100d0b60, NCSiegePartInfoWnd, OnCancelApply)

// 0x100d0c40 (vslot 69) - the owner approves the selected waiting clan (status 2): asks first (dialog id 0x42,
// message 0x29F), with the clan's name and the castle id with the clan id
INT NCSiegePartInfoWnd::OnApprove()
{
    if (List->GetSelectedPledgeID() > 0 && List->GetSelectedStatus() == 2)
    {
        PledgeInfo* Pledge = GConsole->GetPledgeInfo(List->GetSelectedPledgeID());
        if (Pledge)
        {
            NDialogInfo Info;
            Info.ID = 0x42;
            Info.bMainBox = 1;
            Info.bInput = 0;
            Info.SetSystemMessage(0x29F);
            const TCHAR* PledgeName = Pledge->Name;
            if (PledgeName && *PledgeName)
                Info.SetButtons(PledgeName, NULL);
            Info.SetParams(CastleID, List->GetSelectedPledgeID());
            GConsole->ShowDialog(&Info);
        }
    }
    return 1;
}
NW_HOOK_METHOD(0x100d0c40, NCSiegePartInfoWnd, OnApprove)

// 0x100d0d60 (vslot 70) - the same to refuse a waiting or approved clan (status 2 or 3: dialog id 0x43, message 0x29E)
INT NCSiegePartInfoWnd::OnRefuse()
{
    if (List->GetSelectedPledgeID() > 0)
    {
        if (List->GetSelectedStatus() != 2 && List->GetSelectedStatus() != 3)
            return 1;
        PledgeInfo* Pledge = GConsole->GetPledgeInfo(List->GetSelectedPledgeID());
        if (Pledge)
        {
            NDialogInfo Info;
            Info.ID = 0x43;
            Info.bMainBox = 1;
            Info.bInput = 0;
            Info.SetSystemMessage(0x29E);
            const TCHAR* PledgeName = Pledge->Name;
            if (PledgeName && *PledgeName)
                Info.SetButtons(PledgeName, NULL);
            Info.SetParams(CastleID, List->GetSelectedPledgeID());
            GConsole->ShowDialog(&Info);
        }
    }
    return 1;
}
NW_HOOK_METHOD(0x100d0d60, NCSiegePartInfoWnd, OnRefuse)

// 0x100d0920 - and the list's columns: the clan's name, then the alliance's name (attackers)
// or the status (defenders). The second column sorts as numbers on both sides.
__declspec(noinline) void NCSiegePartInfoWnd::SetSide(INT InSide)
{
    Side = InSide;

    NListColumnInfo NameColumn;
    NameColumn.Width = 109;
    NameColumn.Height = 15;
    NameColumn.SetTitleID(STR_ClanName);
    NameColumn.SetNormalTex(TEXT("L2UI.Control.listbutton"));
    NameColumn.SetPushedTex(TEXT("L2UI.Control.listbutton_click"));
    List->AddColumn(0, &NameColumn);

    NListColumnInfo SecondColumn;
    SecondColumn.Width = 100;
    SecondColumn.Height = 15;
    SecondColumn.bNumeric = 1;
    if (Side == SIEGESIDE_Attacker)
    {
        SecondColumn.SetTitleID(STR_AllianceName);
        SecondColumn.SetNormalTex(TEXT("L2UI.Control.listbutton"));
        SecondColumn.SetPushedTex(TEXT("L2UI.Control.listbutton_click"));
    }
    else
    {
        SecondColumn.SetTitleID(STR_Status);
        SecondColumn.SetNormalTex(TEXT("L2UI.Control.listbutton"));
        SecondColumn.SetPushedTex(TEXT("L2UI.Control.listbutton_click"));
    }
    List->AddColumn(0, &SecondColumn);
}
NW_HOOK_METHOD(0x100d0920, NCSiegePartInfoWnd, SetSide)

// 0x100d0e90 - apply or cancel by whether the player's clan is listed; the owner gets
// neither, and approves or refuses the defenders instead
__declspec(noinline) void NCSiegePartInfoWnd::UpdateButtons()
{
    bListed = 1;
    if (Side == SIEGESIDE_Attacker)
    {
        if (bRegistered)
        {
            ApplyButton->HideWindow();
            CancelApplyButton->ShowWindow();
        }
        else
        {
            ApplyButton->ShowWindow();
            CancelApplyButton->HideWindow();
        }
        if (MainOf(this)->Castle.bIsOwner)
        {
            ApplyButton->HideWindow();
            CancelApplyButton->HideWindow();
        }
        ApproveButton->HideWindow();
        RefuseButton->HideWindow();
    }
    else
    {
        if (bRegistered)
        {
            ApplyButton->HideWindow();
            CancelApplyButton->ShowWindow();
        }
        else
        {
            ApplyButton->ShowWindow();
            CancelApplyButton->HideWindow();
        }
        if (MainOf(this)->Castle.bIsOwner)
        {
            ApplyButton->HideWindow();
            CancelApplyButton->HideWindow();
            ApproveButton->ShowWindow();
            RefuseButton->ShowWindow();
        }
        else
        {
            ApproveButton->HideWindow();
            RefuseButton->HideWindow();
        }
    }
}
NW_HOOK_METHOD(0x100d0e90, NCSiegePartInfoWnd, UpdateButtons)

// 0x100d1dc0 - the window closing or showing another castle
__declspec(noinline) void NCSiegePartInfoWnd::Reset()
{
    for (INT i = 0; i < Clans.Num(); i++)
    {
        if (Clans(i))
            appFree(Clans(i));
    }
    Clans.Empty();
    List->ClearRows();
    bRegistered = 0;
    NumApproved = 0;
    NumWaiting = 0;
    bListed = 0;
    CastleID = 0;
}
NW_HOOK_METHOD(0x100d1dc0, NCSiegePartInfoWnd, Reset)

// 0x100d1e40 - the console, before a new list: the copies of the clans are dropped without
// being freed (they leak, as in the stock code)
void NCSiegePartInfoWnd::ClearList()
{
    Clans.Empty();
    List->ClearRows();
    bRegistered = 0;
    NumApproved = 0;
    NumWaiting = 0;
}
NW_HOOK_METHOD(0x100d1e40, NCSiegePartInfoWnd, ClearList)

// 0x100d1e80 - the clan's row (its data in the header) and a copy of the record. On the defending side the owner
// counts as waiting, and "refused" is the system string 579. The player's own clan turns apply into cancel. C2 no
// longer builds a tool tip text for the row (C1's BuildToolTip) and the cells' and the row's parameters take a third
// value, -1.
__declspec(noinline) void NCSiegePartInfoWnd::AddClan(NSiegeClanInfo* Info)
{
    NListRow* Row = new NListRow(2);
    if (Side == SIEGESIDE_Attacker)
    {
        Row->Cells[0].SetText(Info->ClanName);
        Row->Cells[0].SetParams(Info->ApplyTime, 0, -1);
        Row->Cells[1].SetText(Info->AllianceName);
    }
    else
    {
        Row->Cells[0].SetText(Info->ClanName);
        Row->Cells[0].SetParams(Info->ApplyTime, 1, -1);
        switch (Info->Type)
        {
        case SIEGECLAN_Owner:
            Row->Cells[1].SetText(*GL2GameData.SysString[STR_Lord]);
            NumWaiting++;
            break;
        case SIEGECLAN_Waiting:
            Row->Cells[1].SetText(*GL2GameData.SysString[STR_Waiting]);
            NumWaiting++;
            break;
        case SIEGECLAN_Approved:
            Row->Cells[1].SetText(*GL2GameData.SysString[STR_Approved]);
            NumApproved++;
            break;
        case SIEGECLAN_Refused:
            Row->Cells[1].SetText(*GL2GameData.SysString[STR_Refused]);
            break;
        }
    }
    Row->Cells[0].SetText2(Info->LeaderName);
    Row->Cells[1].SetText2(Info->AllianceLeader);
    Row->SetText(Info->AllianceName);
    Row->SetParams(Info->PledgeID, Info->CrestID, -1);
    List->AddRow(Row);
    Clans.AddItem(new NSiegeClanInfo(*Info));
    if (Info->PledgeID == GConsole->GetMyUser()->PledgeID)
        bRegistered = 1;
}
NW_HOOK_METHOD(0x100d1e80, NCSiegePartInfoWnd, AddClan)

NW_CHECK_VTABLE(NCSiegePartInfoWnd, 0x10125d88, 71)

/*-----------------------------------------------------------------------------
    NCSiegeDateInfoWnd.
-----------------------------------------------------------------------------*/

// 0x100d20c0
NCSiegeDateInfoWnd::NCSiegeDateInfoWnd()
{
    BackTex = FindTexture(TEXT("L2UI.SiegeWnd.siege_back3"));
}
NW_HOOK_CTOR(0x100d20c0, NCSiegeDateInfoWnd)

// 0x100d2160
NW_HOOK_DTOR(0x100d2160, NCSiegeDateInfoWnd)

// 0x100d01b0 (vslot 25) - the combo box picks a date (lParam = the item); the confirmation box answers with
// NWM_SIEGE_DATE_OK. C2 does not test IsEnabled first
INT NCSiegeDateInfoWnd::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (NCWnd::WindowProc(Msg, wParam, lParam))
        return 1;
    if (Msg == NWM_COMMAND)
    {
        if ((INT)wParam == NID_SIEGE_TIMECOMBO)
            return OnSelectTime(lParam) != 0;
    }
    else if (Msg == NWM_SIEGE_DATE_OK)
        return OnConfirmTime(wParam, lParam) != 0;
    return 0;
}
NW_HOOK_METHOD(0x100d01b0, NCSiegeDateInfoWnd, WindowProc)

// 0x100d0fd0 (vslot 36) - the combo box of the dates, next to "Siege Date" (NCWnd's
// OnCreate is not called)
INT NCSiegeDateInfoWnd::OnCreate()
{
    TimeCombo = new NCComboBox;
    TimeCombo->ID = NID_SIEGE_TIMECOMBO;
    TimeCombo->SetWindowRect(80, 60, 126, 15);
    CreateChild(TimeCombo, NWS_VISIBLE);
    return 1;
}
NW_HOOK_METHOD(0x100d0fd0, NCSiegeDateInfoWnd, OnCreate)

// 0x100d1080 (vslot 37) - guarded in C2; NCWnd's first, then the combo box goes
INT NCSiegeDateInfoWnd::OnDestroy()
{
    guard(NCSiegeDateInfoWnd::OnDestroy);
    NCWnd::OnDestroy();
    DestroyChild(this, TimeCombo);
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x100d1080, NCSiegeDateInfoWnd, OnDestroy)

// 0x100d1130 (vslot 50) - the current time and the siege's; without a date, "Undecided" for
// all but the owner, who has the combo box there. The clip goes on before the origin; the
// background texture is not checked.
INT NCSiegeDateInfoWnd::OnPaint(NCGDevice* Device)
{
    TCHAR Now[128];
    TCHAR When[128];
    Device->SetClip(ScreenX, ScreenY - 18, Width, Height);
    Device->BeginPaint(ScreenX, ScreenY);
    Device->DrawTexture(0, -18, 240, 246, 0, 0, 240, 246, BackTex);
    GConsole->FormatDateTime(CurrentTime, Now);
    Device->DrawText(10, 30, 0xFFFFFFFF, *GL2GameData.SysString[STR_CurrentTime], 0, 0, 0,
                     L2FONT_Normal, 0, 0, 0, 0, 0, 0);
    Device->DrawText(80, 30, 0xFFFFFFFF, Now, 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
    Device->DrawText(10, 60, 0xFFFFFFFF, *GL2GameData.SysString[STR_SiegeDate], 0, 0, 0,
                     L2FONT_Normal, 0, 0, 0, 0, 0, 0);
    if (SiegeTime > 0)
    {
        GConsole->FormatDateTime(SiegeTime, When);
        Device->DrawText(80, 60, 0xFFFFFFFF, When, 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
    }
    else if (SiegeTime == 0 && !MainOf(this)->Castle.bIsOwner)
    {
        Device->DrawText(80, 60, 0xFFFFFFFF, *GL2GameData.SysString[STR_Undecided], 0, 0, 0,
                         L2FONT_Normal, 0, 0, 0, 0, 0, 0);
    }
    Device->EndPaint();
    Device->ReleaseClip();
    return 1;
}
NW_HOOK_METHOD(0x100d1130, NCSiegeDateInfoWnd, OnPaint)

// 0x100d1370 (vslot 62) - a date picked (not the automatic setting) is confirmed first; OK
// comes back as NWM_SIEGE_DATE_OK
INT NCSiegeDateInfoWnd::OnSelectTime(INT Index)
{
    if (Index > 0)
    {
        SelectedIndex = Index;
        NDialogInfo Info;
        Info.ID = DIALOG_SIEGE_DATE;
        Info.Owner = Entry;
        Info.bMainBox = 1;
        Info.bInput = 0;
        Info.SetSystemMessage(MSG_ConfirmSiegeTime);
        Info.SetButtons(TimeCombo->GetItemText(Index), NULL);
        GConsole->ShowDialog(&Info);
    }
    return 1;
}
NW_HOOK_METHOD(0x100d1370, NCSiegeDateInfoWnd, OnSelectTime)

// 0x100d1440 (vslot 68) - the castle and the date chosen (0: the automatic setting)
INT NCSiegeDateInfoWnd::OnConfirmTime(WPARAM wParam, LPARAM lParam)
{
    L2ParamStack Params(10);
    Params.PushBack((void*)(INT_PTR)CastleID);
    Params.PushBack((void*)(INT_PTR)(SelectedIndex > 0 ? Times(SelectedIndex - 1) : 0));
    GConsole->RequestSetSiegeTime(Params);
    return 1;
}
NW_HOOK_METHOD(0x100d1440, NCSiegeDateInfoWnd, OnConfirmTime)

// 0x100d1300 - the combo box back to its one "Automatic setting" item
__declspec(noinline) void NCSiegeDateInfoWnd::Reset()
{
    Times.Empty();
    TimeCombo->ClearItems();
    TimeCombo->AddItem(*GL2GameData.SysString[STR_AutoSetting]);
    CastleID = 0;
    SelectedIndex = 0;
    CurrentTime = 0;
    SiegeTime = 0;
    TimeCombo->SetSelected(0);
}
NW_HOOK_METHOD(0x100d1300, NCSiegeDateInfoWnd, Reset)

// 0x100d14e0 - the combo box shows to the owner only, while no date is set
__declspec(noinline) void NCSiegeDateInfoWnd::SetTimes(INT InCurrentTime, INT InSiegeTime)
{
    CurrentTime = InCurrentTime;
    SiegeTime = InSiegeTime;
    if (InSiegeTime == 0)
    {
        if (MainOf(this)->Castle.bIsOwner)
            TimeCombo->ShowWindow();
        else
            TimeCombo->HideWindow();
    }
    else
        TimeCombo->HideWindow();
}
NW_HOOK_METHOD(0x100d14e0, NCSiegeDateInfoWnd, SetTimes)

// 0x100d21c0 - one more date the owner can choose (the console, from the server's offer)
void NCSiegeDateInfoWnd::AddTime(INT Time)
{
    TCHAR Text[128];
    GConsole->FormatDateTime(Time, Text);
    TimeCombo->AddItem(Text);
    Times.AddItem(Time);
}
NW_HOOK_METHOD(0x100d21c0, NCSiegeDateInfoWnd, AddTime)

NW_CHECK_VTABLE(NCSiegeDateInfoWnd, 0x10125b10, 69)

/*-----------------------------------------------------------------------------
    NCSiegeInfoMainWnd.
-----------------------------------------------------------------------------*/

// 0x100d1540
NCSiegeInfoMainWnd::NCSiegeInfoMainWnd()
{
    BackTex = FindTexture(TEXT("L2UI.SiegeWnd.Siege_Back1"));
}
NW_HOOK_CTOR(0x100d1540, NCSiegeInfoMainWnd)

// 0x100d1600
NW_HOOK_DTOR(0x100d1600, NCSiegeInfoMainWnd)

// 0x100d0230 (vslot 25) - C2 has no IsEnabled check of its own
INT NCSiegeInfoMainWnd::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (NCWnd::WindowProc(Msg, wParam, lParam))
        return 1;
    if (Msg == NWM_COMMAND && wParam == NID_SIEGE_TABS)
        return OnTab((INT)lParam) != 0;
    return 0;
}
NW_HOOK_METHOD(0x100d0230, NCSiegeInfoMainWnd, WindowProc)

// 0x100d2960 (vslot 36) - title bar, the tabs' windows (the sides hidden) and the tab
// buttons: date, attackers, defenders. NCWnd's OnCreate is not called.
INT NCSiegeInfoMainWnd::OnCreate()
{
    FrameCtrl = new NCFrameCtrl(1);
    FrameID = NID_SIEGE_FRAME;
    FrameCtrl->ID = NID_SIEGE_FRAME;
    FrameCtrl->SetWindowRect(0, 0, 240, 21);
    CreateChild(FrameCtrl, NWS_VISIBLE);

    DateInfo = new NCSiegeDateInfoWnd;
    DateInfo->SetWindowRect(0, 138, 240, 246);
    CreateChild(DateInfo, NWS_VISIBLE);

    AttackerInfo = new NCSiegePartInfoWnd;
    AttackerInfo->SetWindowRect(0, 138, 240, 246);
    CreateChild(AttackerInfo, NWS_VISIBLE);
    AttackerInfo->SetSide(SIEGESIDE_Attacker);
    AttackerInfo->HideWindow();

    DefenderInfo = new NCSiegePartInfoWnd;
    DefenderInfo->SetWindowRect(0, 138, 240, 246);
    CreateChild(DefenderInfo, NWS_VISIBLE);
    DefenderInfo->SetSide(SIEGESIDE_Defender);
    DefenderInfo->HideWindow();

    TabCtrl = new NCTabControl;
    TabCtrl->ID = NID_SIEGE_TABS;
    TabCtrl->SetWindowRect(15, 122, 207, 15);
    CreateChild(TabCtrl, NWS_VISIBLE);
    TabCtrl->AddTab(STR_SiegeDate, DateInfo, 69, 16, TEXT("L2UI.NWindow.Tab2"),
                    TEXT("L2UI.NWindow.Tab1"), 0, -1);
    TabCtrl->AddTab(STR_SiegeInfo, AttackerInfo, 69, 16, TEXT("L2UI.NWindow.Tab2"),
                    TEXT("L2UI.NWindow.Tab1"), 0, -1);
    TabCtrl->AddTab(STR_DefenseInfo, DefenderInfo, 69, 16, TEXT("L2UI.NWindow.Tab2"),
                    TEXT("L2UI.NWindow.Tab1"), 0, -1);
    return 1;
}
NW_HOOK_METHOD(0x100d2960, NCSiegeInfoMainWnd, OnCreate)

// 0x100d1650 (vslot 37) - C2 guards it
INT NCSiegeInfoMainWnd::OnDestroy()
{
    guard(NCSiegeInfoMainWnd::OnDestroy);
    NCWnd::OnDestroy();
    DestroyChild(this, TabCtrl);
    DestroyChild(this, DateInfo);
    DestroyChild(this, AttackerInfo);
    DestroyChild(this, DefenderInfo);
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x100d1650, NCSiegeInfoMainWnd, OnDestroy)

// 0x100d2320 (vslot 42) - opens emptied on the date tab (the console's SetCastleInfo
// follows)
void NCSiegeInfoMainWnd::ShowWindow()
{
    PlayInterfaceSound(SOUND_WindowOpen);
    NCWnd::ShowWindow();
    Reset();
    TabCtrl->SetTab(0);
}
NW_HOOK_METHOD(0x100d2320, NCSiegeInfoMainWnd, ShowWindow)

// 0x100d2310 (vslot 43)
void NCSiegeInfoMainWnd::HideWindow()
{
    NCWnd::HideWindow();
    Reset();
}
NW_HOOK_METHOD(0x100d2310, NCSiegeInfoMainWnd, HideWindow)

// 0x100d1780 (vslot 50) - the castle's name centered on the background's title, then lord,
// clan and alliance (the crests before the names), or "No castle owner". The clip goes on
// before the origin; the background texture is not checked. The alliance crest is looked up
// by the owner clan's id too.
INT NCSiegeInfoMainWnd::OnPaint(NCGDevice* Device)
{
    Device->SetClip(ScreenX, ScreenY, Width, Height);
    Device->BeginPaint(ScreenX, ScreenY);
    Device->DrawTexture(0, 21, 240, 99, 0, 0, 240, 99, BackTex);
    const TCHAR* Name = GConsole->GetCastleName(Castle.CastleID);
    if (Name)
    {
        SIZE Size;
        NCGDevice::GetTextExtent(&Size, Name, 0, 0, 0, 0, 0);
        Device->DrawText(120 - Size.cx / 2, 27, SIEGE_TITLE_COLOR, Name, 0, 0, 0, L2FONT_Normal, 0,
                         0, 0, 0, 0, 0);
    }
    Device->DrawText(20, 60, SIEGE_LABEL_COLOR, *SysString(STR_Lord), 0, 0, 0,
                     L2FONT_Normal, 0, 0, 0, 0, 0, 0);
    Device->DrawText(20, 75, SIEGE_LABEL_COLOR, *SysString(STR_Clan), 0, 0, 0,
                     L2FONT_Normal, 0, 0, 0, 0, 0, 0);
    Device->DrawText(20, 90, SIEGE_LABEL_COLOR, *SysString(STR_Alliance), 0, 0, 0,
                     L2FONT_Normal, 0, 0, 0, 0, 0, 0);
    if (Castle.LordName[0])
    {
        Device->DrawText(80, 60, 0xFFFFFFFF, Castle.LordName, 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0,
                         0, 0);
        if (Device->DrawPledgeCrest(80, 75, Castle.PledgeID))
            Device->DrawText(100, 75, 0xFFFFFFFF, Castle.ClanName, 0, 0, 0, L2FONT_Normal, 0, 0,
                             0, 0, 0, 0);
        else
            Device->DrawText(80, 75, 0xFFFFFFFF, Castle.ClanName, 0, 0, 0, L2FONT_Normal, 0, 0, 0,
                             0, 0, 0);
        if (Castle.AllianceName[0])
        {
            if (Device->DrawAllyCrest(88, 90, Castle.PledgeID))
                Device->DrawText(100, 90, 0xFFFFFFFF, Castle.AllianceName, 0, 0, 0,
                                 L2FONT_Normal, 0, 0, 0, 0, 0, 0);
            else
                Device->DrawText(80, 90, 0xFFFFFFFF, Castle.AllianceName, 0, 0, 0, L2FONT_Normal,
                                 0, 0, 0, 0, 0, 0);
        }
        else
        {
            Device->DrawText(80, 90, 0xFFFFFFFF, *SysString(STR_NoAlliance), 0, 0, 0,
                             L2FONT_Normal, 0, 0, 0, 0, 0, 0);
        }
    }
    else
    {
        Device->DrawText(80, 60, 0xFFFFFFFF, *SysString(STR_NoCastleOwner), 0, 0, 0,
                         L2FONT_Normal, 0, 0, 0, 0, 0, 0);
    }
    Device->EndPaint();
    Device->ReleaseClip();
    return 1;
}
NW_HOOK_METHOD(0x100d1780, NCSiegeInfoMainWnd, OnPaint)

// 0x100d1a50 (vslot 67) - a side's list is asked for once (until Reset)
INT NCSiegeInfoMainWnd::OnTab(INT Tab)
{
    switch (Tab)
    {
    case 0:
        DateInfo->ShowWindow();
        AttackerInfo->HideWindow();
        DefenderInfo->HideWindow();
        DateInfo->SetTimes(Castle.CurrentTime, Castle.SiegeTime);
        break;
    case 1:
        if (!AttackerInfo->bListed)
        {
            L2ParamStack Params(10);
            Params.PushBack((void*)(INT_PTR)Castle.CastleID);
            GConsole->RequestSiegeAttackerList(Params);
        }
        DateInfo->HideWindow();
        AttackerInfo->ShowWindow();
        DefenderInfo->HideWindow();
        AttackerInfo->UpdateButtons();
        break;
    case 2:
        if (!DefenderInfo->bListed)
        {
            L2ParamStack Params(10);
            Params.PushBack((void*)(INT_PTR)Castle.CastleID);
            GConsole->RequestSiegeDefenderList(Params);
        }
        DateInfo->HideWindow();
        AttackerInfo->HideWindow();
        DefenderInfo->ShowWindow();
        DefenderInfo->UpdateButtons();
        break;
    }
    return 1;
}
NW_HOOK_METHOD(0x100d1a50, NCSiegeInfoMainWnd, OnTab)

// 0x100d2220 - the castle's data and the three tabs emptied
__declspec(noinline) void NCSiegeInfoMainWnd::Reset()
{
    Castle.Clear();
    AttackerInfo->Reset();
    DefenderInfo->Reset();
    DateInfo->Reset();
}
NW_HOOK_METHOD(0x100d2220, NCSiegeInfoMainWnd, Reset)

// 0x100d2280 - the console, with the server's siege info (after ShowWindow)
void NCSiegeInfoMainWnd::SetCastleInfo(NSiegeCastleInfo* Info)
{
    Reset();
    Castle = *Info;
    DateInfo->CastleID = Info->CastleID;
    AttackerInfo->CastleID = Info->CastleID;
    DefenderInfo->CastleID = Info->CastleID;
    DateInfo->SetTimes(Info->CurrentTime, Info->SiegeTime);
}
NW_HOOK_METHOD(0x100d2280, NCSiegeInfoMainWnd, SetCastleInfo)

// 0x100d22f0
void NCSiegeInfoMainWnd::AddAttacker(NSiegeClanInfo* Info)
{
    AttackerInfo->AddClan(Info);
}
NW_HOOK_METHOD(0x100d22f0, NCSiegeInfoMainWnd, AddAttacker)

// 0x100d2300
void NCSiegeInfoMainWnd::AddDefender(NSiegeClanInfo* Info)
{
    DefenderInfo->AddClan(Info);
}
NW_HOOK_METHOD(0x100d2300, NCSiegeInfoMainWnd, AddDefender)

NW_CHECK_VTABLE(NCSiegeInfoMainWnd, 0x10125950, 68)
