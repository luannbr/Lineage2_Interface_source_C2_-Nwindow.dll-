/*=============================================================================
    NConsoleWndEvents.cpp - what the engine tells the console in the game: each
    UGameEngine::On* handler of a server packet calls one of its UL2ConsoleWnd slots (named
    after the handler). Here: the target, the HTML pages, the tutorial, the radar, the clan,
    the alliance, the friends, the party and its matching, the chat channels, the shortcuts,
    the buffs, the skills and the trainer, the restart menu, the quests, the system messages,
    the pet and the summon, the siege, a GM's views. Most pass the news on to a window of the
    desktop; the answers to requests show a system message, the offers a dialog box. The login
    stages' are in NConsoleWndLogin.cpp, the items' in NConsoleWndItems.cpp.
=============================================================================*/
#include "nwindow/NConsoleWnd.h"
#include "nwindow/NCConsole.h"
#include "nwindow/NDialogInfo.h"
#include "nwindow/NCDialogBox.h"
#include "nwindow/NCGaraDialogBox.h"
#include "nwindow/NCEditBox.h"
#include "nwindow/NCStatusWnd.h"
#include "nwindow/NCAbnormalStatusWnd.h"
#include "nwindow/NCTutorialViewerWnd.h"
#include "nwindow/NCNPCHtmlViewer.h"
#include "nwindow/NCBoardWnd.h"
#include "nwindow/NCRadarWnd.h"
#include "nwindow/NCShortCutWnd.h"
#include "nwindow/NCPartyWnd.h"
#include "nwindow/NCChatWnd.h"
#include "nwindow/NCMatchWnd.h"
#include "nwindow/NCMainWnd.h"
#include "nwindow/NCActionWnd.h"
#include "nwindow/NCPledgeWnd.h"
#include "nwindow/NCQuestWnd.h"
#include "nwindow/NCGMWnd.h"
#include "nwindow/NCMagicSkillWnd.h"
#include "nwindow/NCSkillWnd.h"
#include "nwindow/NCTrainWnd.h"
#include "nwindow/NCRestartMenuWnd.h"
#include "nwindow/NCPetWnd.h"
#include "nwindow/NCSummonedWnd.h"
#include "nwindow/NCProgressCtrl.h"
#include "nwindow/NCSiegeInfoMainWnd.h"
#include "nwindow/NCDetailStatusWnd.h"
#include "nwindow/NCMinimapWnd.h"
#include "nwindow/NCItemWnd.h"
#include "nwindow/NCSkillItemWnd.h"
#include "nwindow/NCInventoryWnd.h"
#include "nwindow/NCPetInventoryWnd.h"
#include "nwindow/NCTrainListBox.h"

#if NW_FEATURES
// phase 6: AutoShot (NWindow_C1 src/inject/AutoShot.cpp), CraftUi (NWindow_C1
// src/inject/craft/CraftUi.cpp)
#include "features/AutoShot.h"
#include "features/craft/CraftUi.h"
// phase 6: ChatColors (not in NWindow_C1: the Interlude colors of the chat types C1 draws white)
#include "features/ChatColors.h"
#endif

// system messages (systemmsg ids)
enum
{
    MSG_AskJoinPledge = 67,            // "$s1 has invited you to the $s2 Clan. Do you want to join?"
    MSG_ClanCreated = 189,             // "Clan has been created."
    MSG_ClanCreateFailed = 190,        // "You have failed to create a clan."
    MSG_ClanDispersed = 193,           // "Clan has dispersed."
    MSG_ClanDisperseFailed = 194,      // "You have failed to disperse the Clan."
    MSG_ClanWithdrawn = 197,           // "Withdrawn from the clan."
    MSG_ClanWithdrawFailed = 1120,     // "You have failed to withdraw from the $s1 Clan." (C1: 198)
    MSG_ClanMembershipEnded = 199,     // "Membership to this clan has been terminated."
    MSG_PartyWithdrawn = 200,          // "You have withdrawn from the party."
    MSG_PartyExpelled = 202,           // "You have been expelled from the party."
    MSG_PartyDispersed = 203,          // "The party has dispersed."
    MSG_AskPledgeWar = 207,            // "$s2 of the $s1 Clan requests declaration of war. Do
                                       //   you accept?"
    MSG_PledgeTruceDenied = 228,       // "Request to end war has been denied."
    MSG_AskPledgeTruce = 255,          // "The $s1 Clan has requested to end war. Do you agree?"
    MSG_PartyDeclined = 305,           // "The player declined to join your party."
    MSG_ClanJoinFailed = 308,          // "Failed to join the clan"
    MSG_ClanExpelDone = 309,           // "Succeeded in expelling a clan member."
    MSG_ClanExpelFailed = 310,         // "Failed to expel a clan member."
    MSG_PledgeWarAccepted = 311,       // "Clan war has been accepted."
    MSG_PledgeWarRefused = 312,        // "Clan war has been refused."
    MSG_PledgeTruceAccepted = 313,     // "The cease war request has been accepted."
    MSG_SurrenderFailed = 314,         // "Failed to surrender."
    MSG_PersonalSurrenderFailed = 315, // "Ffailed to personally surrender."
    MSG_PartyWithdrawFailed = 316,     // "Failed to withdraw from the party."
    MSG_PartyExpelFailed = 317,        // "Failed to expel a party member."
    MSG_PartyDisperseFailed = 318,     // "Failed to disperse the party."
    MSG_AskFriend = 516,               // "$s1 has invited you as a Friend."
    MSG_AllyAccepted = 517,            // "You have accepted the alliance."
    MSG_AllyInviteFailed = 518,        // "You have failed to invite a clan into the alliance."
    MSG_AllyWithdrawn = 519,           // "You have withdrawn from the alliance."
    MSG_AllyWithdrawFailed = 520,      // "You have failed to withdraw from the alliance."
    MSG_AllyExpelDone = 521,           // "You have succeeded in expelling a clan."
    MSG_AllyExpelFailed = 522,         // "You have failed to expel a clan."
    MSG_AllyDissolved = 523,           // "The alliance has dissolved."
    MSG_AllyDissolveFailed = 524,      // "You have failed to dissolve the alliance."
    MSG_FriendInvited = 525,           // "You have succeeded in inviting a Friend."
    MSG_FriendInviteFailed = 526,      // "You have failed to invite a Friend."
    MSG_AskJoinAlly = 527,             // "$s2, the alliance leader of $s1 has requested an
                                       //   alliance."
    MSG_AskJoinPartyLooter = 572,      // "Do you wish to consent to $s1's party invitation?
                                       //   (Item distribution: possession of the looter)"
    MSG_AskJoinPartyRandom = 573,      // "... (Item distribution: random distribution to party
                                       //   members)"
    MSG_AskJoinPartyRandomSpoil = 967, // C2: three more ways to share (random including spoil, by turn,
    MSG_AskJoinPartyByTurn = 968,      //   by turn including spoil)
    MSG_AskJoinPartyByTurnSpoil = 969,
    MSG_AskAllyWar = 665,              // "$s2% of $s1 alliance is applying for alliance war. Do
                                       //   you want to accept the challenge?"
    MSG_AskAllyTruce = 666,            // "A request for ceasefire has been received from $s1
                                       //   alliance. Do you want to agree to the cease-fire?"
};

// the dialog boxes opened here (NDialogInfo::ID; NCDialogBox sends the answer)
enum
{
    DIALOG_JOIN_PARTY = 0x0A,
    DIALOG_JOIN_PLEDGE = 0x0E,
    DIALOG_START_PLEDGE_WAR = 0x13,
    DIALOG_STOP_PLEDGE_WAR = 0x15,
    DIALOG_SYSTEM_MESSAGE = 0x1F,      // OK only
    DIALOG_JOIN_ALLY = 0x33,           // C2: C1's 0x2F..0x37 moved by 4 (new kinds before them)
    DIALOG_START_ALLY_WAR = 0x34,
    DIALOG_STOP_ALLY_WAR = 0x35,
    DIALOG_FRIEND_INVITE = 0x3B,
};

// sysstrings: the clan's status texts, the announcements' speaker
enum
{
    STR_Announcement = 328,
    STR_PledgeStatus1 = 339,
    STR_PledgeStatus4 = 340,
    STR_PledgeStatus3 = 341,
    STR_PledgeStatus2 = 349,
};

// Say2 types, the chat channels (names ours, after the protocol's)
enum
{
    SAY2_Shout = 1,
    SAY2_Tell = 2,
    SAY2_Party = 3,
    SAY2_Pledge = 4,
    SAY2_PetitionPlayer = 6,
    SAY2_PetitionGM = 7,
    SAY2_Market = 8,
    SAY2_Ally = 9,
    SAY2_Announcement = 10,
};

// interface sounds (FL2NetNotify::OnInterfacePlaySound; after the engine's list)
enum { SOUND_WindowOpen = 5, SOUND_WindowClose = 6 };

// the pawn the player's controller drives (as NConsoleWndNet.cpp's)
static FORCEINLINE APawn* PlayerPawn(UEngine* Engine)
{
    return Engine->Client->Viewports(0)->Actor->Pawn;
}

/*-----------------------------------------------------------------------------
    The target, the HTML pages, the tutorial, the radar.
-----------------------------------------------------------------------------*/

// 0x10059ff0 - a target was selected: its level against the player's. C2: with the GM window open, a selected
// player (not a creature: User +0x08 is 0) also goes into the GM window's name line
void NConsoleWnd::MyTargetSelected(INT LevelDiff)
{
    GetDesktop()->TargetStatusWnd->SetLevelDiff(LevelDiff);
    APawn* Pawn = PlayerPawn(Engine);
    if (Engine && Pawn && Pawn->Controller && Pawn->Controller->Target)
    {
        User* Target = FindUserByID(Pawn->Controller->Target->CreatureID);
        if (Target && !Target->Unknown8 && GetDesktop()->GMWnd->IsShowWindow())
        {
            GetDesktop()->GMWnd->NameEdit->ClearText();
            GetDesktop()->GMWnd->NameEdit->InsertText((TCHAR*)((BYTE*)Target + 0x1C), 1);
        }
    }
}
NW_HOOK_METHOD(0x10059ff0, NConsoleWnd, MyTargetSelected)

// 0x10067500 - what an action's shortcut asks of the client: 4 the next target, 5 pick up,
// 6 assist
void NConsoleWnd::ReceiveAction(INT Action)
{
    switch (Action)
    {
    case 4:
    {
        User* Target = GetSelectedActor();
        if (Target && Target->Pawn)
            RequestTargetNext(200, Target->Pawn->CreatureID);
        else
            RequestTargetNext(200, -1);
        break;
    }
    case 5:
        RequestPickUp(200, -1);
        break;
    case 6:
        RequestAssist();
        break;
    }
}
NW_HOOK_METHOD(0x10067500, NConsoleWnd, ReceiveAction)

// 0x100681f0 - an NPC's page
void NConsoleWnd::NpcHtmlMessage(L2ParamStack& Params)
{
    TCHAR* Html = (TCHAR*)Params.Top();
#if NW_FEATURES
    // phase 6: CraftUi (NWindow_C1 src/inject/craft/CraftUi.cpp, its OnNpcHtmlMessage hook): the
    // server's "L2W mfg" data messages are the craft windows', not a page
    if (CraftUi_OnNpcHtml(Html))
        return;
    // phase 6: AutoShot (NWindow_C1 src/inject/craft/CraftUi.cpp, its OnNpcHtmlMessage hook):
    // the server's "L2W shot" data messages are the automatic shots' state, not a page
    if (nwf::AutoShot_OnNpcHtml(Html))
        return;
#endif
    GConsole->PlayInterfaceSound(SOUND_WindowOpen);
    GetDesktop()->NpcHtmlWnd->ShowPage(Html);
    GetDesktop()->NpcHtmlWnd->ShowWindow();
}
NW_HOOK_METHOD(0x100681f0, NConsoleWnd, NpcHtmlMessage)

// 0x100550d0 - the community board: the bypass commands of its six buttons, then the page
void NConsoleWnd::ShowBoard(L2ParamStack& Params)
{
    TCHAR* Command1 = (TCHAR*)Params.Top();
    TCHAR* Command2 = (TCHAR*)Params.Top();
    TCHAR* Command3 = (TCHAR*)Params.Top();
    TCHAR* Command4 = (TCHAR*)Params.Top();
    TCHAR* Command5 = (TCHAR*)Params.Top();
    TCHAR* Command6 = (TCHAR*)Params.Top();
    TCHAR* Html = (TCHAR*)Params.Top();
    if (Html)
    {
        static_cast<NCBoardWnd*>(GetDesktop()->BoardWnd)->LoadPage(Html);
        static_cast<NCBoardWnd*>(GetDesktop()->BoardWnd)->SetCommands(Command1, Command2, Command3,
                                                                     Command4, Command5, Command6);
        GetDesktop()->BoardWnd->ShowWindow();
        GetDesktop()->BoardWnd->SetFocus();
    }
}
NW_HOOK_METHOD(0x100550d0, NConsoleWnd, ShowBoard)

// 0x10055090 - a page of the tutorial
void NConsoleWnd::TutorialShowHtml(TCHAR* Html)
{
    GetDesktop()->TutorialWnd->ShowPage(Html);
    GetDesktop()->TutorialWnd->ShowWindow();
}
NW_HOOK_METHOD(0x10055090, NConsoleWnd, TutorialShowHtml)

// 0x1005acd0 - the tutorial's question mark, sent back when clicked
void NConsoleWnd::TutorialShowQuestionMark(INT ID)
{
    GetDesktop()->TutorialQuestionWnd->QuestionID = ID;
    GetDesktop()->TutorialQuestionWnd->ShowWindow();
}
NW_HOOK_METHOD(0x1005acd0, NConsoleWnd, TutorialShowQuestionMark)

// 0x100551f0 - the client events the tutorial waits for (SendTutorialEvent)
void NConsoleWnd::TutorialEnableClientEvent(INT Events)
{
    TutorialEvents = Events;
}
NW_HOOK_METHOD(0x100551f0, NConsoleWnd, TutorialEnableClientEvent)

// 0x10055200
void NConsoleWnd::TutorialCloseHtml()
{
    GetDesktop()->TutorialWnd->HideWindow();
}
NW_HOOK_METHOD(0x10055200, NConsoleWnd, TutorialCloseHtml)

// 0x10055800 (vslot 81) - what to do with a point on the radar (the desktop's +0x12C) and the minimap (+0x114): Param1
// 0 adds it (0x100c0d30), 1 removes it (0x1009a4d0), 2 clears all (0x1009a540, no point); Param2 chooses the radar (0),
// the minimap (1) or both (2). C2: UGameEngine::OnShowRadar passes five values, C1's passed the point only
void NConsoleWnd::ShowRadar(INT Param1, INT Param2, INT X, INT Y, INT Z)
{
    if (Param1 == 0)
    {
        if (Param2 == 0)
            GetDesktop()->RadarWnd->AddTarget(X, Y, Z);
        else if (Param2 == 1)
            GetDesktop()->MinimapWnd->AddTarget(X, Y, Z);
        else if (Param2 == 2)
        {
            GetDesktop()->RadarWnd->AddTarget(X, Y, Z);
            GetDesktop()->MinimapWnd->AddTarget(X, Y, Z);
        }
    }
    else if (Param1 == 1)
    {
        if (Param2 == 0)
            GetDesktop()->RadarWnd->RemoveTarget(X, Y, Z);
        else if (Param2 == 1)
            GetDesktop()->MinimapWnd->RemoveTarget(X, Y, Z);
        else if (Param2 == 2)
        {
            GetDesktop()->RadarWnd->RemoveTarget(X, Y, Z);
            GetDesktop()->MinimapWnd->RemoveTarget(X, Y, Z);
        }
    }
    else if (Param1 == 2)
    {
        GetDesktop()->RadarWnd->ClearTargets();
        GetDesktop()->MinimapWnd->ClearTargets();   // the stock body (0x1009a540) serves both windows
    }
}
NW_HOOK_METHOD(0x10055800, NConsoleWnd, ShowRadar)

// 0x10055990 (vslot 82) - a point off the radar and the map (the fmap paired the C1 ShowRadar with this: the C2 call
// is NCRadarWnd::RemoveTarget's 0x1009a4d0 twice, C1's DeleteRadar)
void NConsoleWnd::DeleteRadar(INT X, INT Y, INT Z)
{
    GetDesktop()->RadarWnd->RemoveTarget(X, Y, Z);
    GetDesktop()->MinimapWnd->RemoveTarget(X, Y, Z);
}
NW_HOOK_METHOD(0x10055990, NConsoleWnd, DeleteRadar)

/*-----------------------------------------------------------------------------
    The clan: its members, the server's answers, the offers (a dialog box; refused at once
    when the other side is not known).
-----------------------------------------------------------------------------*/

// 0x100548c0 - a new member list follows (AddPledgeMember)
void NConsoleWnd::ReceivePledgeMemberList()
{
    guard(NConsoleWnd::ReceivePledgeMemberList);
    NW_TRACE("ReceivePledgeMemberList (a new member list follows)");
    GetDesktop()->MainWnd->ActionWnd->PledgeWnd->ClearMembers();
    unguard;
}
NW_HOOK_METHOD(0x100548c0, NConsoleWnd, ReceivePledgeMemberList)

// 0x1005a530 (vslot 87) - name, level, class, two values and whether on line
void NConsoleWnd::AddPledgeMember(L2ParamStack& Params)
{
    guard(NConsoleWnd::AddPledgeMember);
    NPledgeMember Member;
    TCHAR* Name = (TCHAR*)Params.Top();
    if (Name)
        appStrcpy(Member.Name, Name);
    Member.Level = (INT)Params.Top();
    Member.ClassID = (INT)Params.Top();
    Member.Unknown208 = (INT)Params.Top();
    Member.Unknown20C = (INT)Params.Top();
    Member.bOnline = (INT)Params.Top();
    NW_TRACE("AddPledgeMember name=%ls level=%d class=%d v1=%d v2=%d online=%d", Member.Name, Member.Level, Member.ClassID,
             Member.Unknown208, Member.Unknown20C, Member.bOnline);
    GetDesktop()->MainWnd->ActionWnd->PledgeWnd->AddMember(&Member);
    unguard;
}
NW_HOOK_METHOD(0x1005a530, NConsoleWnd, AddPledgeMember)

// 0x1005a630 (vslot 88) - as AddPledgeMember
void NConsoleWnd::UpdatePledgeMember(L2ParamStack& Params)
{
    guard(NConsoleWnd::UpdatePledgeMember);
    NPledgeMember Member;
    TCHAR* Name = (TCHAR*)Params.Top();
    if (Name)
        appStrcpy(Member.Name, Name);
    Member.Level = (INT)Params.Top();
    Member.ClassID = (INT)Params.Top();
    Member.Unknown208 = (INT)Params.Top();
    Member.Unknown20C = (INT)Params.Top();
    Member.bOnline = (INT)Params.Top();
    GetDesktop()->MainWnd->ActionWnd->PledgeWnd->UpdateMember(&Member);
    unguard;
}
NW_HOOK_METHOD(0x1005a630, NConsoleWnd, UpdatePledgeMember)

// 0x10054950
void NConsoleWnd::DeletePledgeMember(TCHAR* Name)
{
    guard(NConsoleWnd::DeletePledgeMember);
    GetDesktop()->MainWnd->ActionWnd->PledgeWnd->DeleteMember(Name);
    unguard;
}
NW_HOOK_METHOD(0x10054950, NConsoleWnd, DeletePledgeMember)

// 0x100549e0 - the player has no clan any more
void NConsoleWnd::DeleteAllPledgeMember()
{
    guard(NConsoleWnd::DeleteAllPledgeMember);
    GetDesktop()->MainWnd->ActionWnd->PledgeWnd->ClearMembers();
    GetDesktop()->MainWnd->ActionWnd->PledgeWnd->ClearInfo();
    GetDesktop()->MainWnd->ActionWnd->UpdatePledgeTab();
    unguard;
}
NW_HOOK_METHOD(0x100549e0, NConsoleWnd, DeleteAllPledgeMember)

// The clan's status text (sysstring) by its status, into an NPledgeInfo
static FORCEINLINE void SetPledgeStatusText(NPledgeInfo& Pledge, INT Status)
{
    switch (Status)
    {
    case 0:
        Pledge.StatusText[0] = 0;
        break;
    case 1:
        if (*GL2GameData.SysString[STR_PledgeStatus1])
            appStrcpy(Pledge.StatusText, *GL2GameData.SysString[STR_PledgeStatus1]);
        break;
    case 2:
        if (*GL2GameData.SysString[STR_PledgeStatus2])
            appStrcpy(Pledge.StatusText, *GL2GameData.SysString[STR_PledgeStatus2]);
        break;
    case 3:
        if (*GL2GameData.SysString[STR_PledgeStatus3])
            appStrcpy(Pledge.StatusText, *GL2GameData.SysString[STR_PledgeStatus3]);
        break;
    case 4:
        if (*GL2GameData.SysString[STR_PledgeStatus4])
            appStrcpy(Pledge.StatusText, *GL2GameData.SysString[STR_PledgeStatus4]);
        break;
    }
}

// the clan's record as the clan windows take it
static FORCEINLINE void MakePledgeInfo(NPledgeInfo& Pledge, PledgeInfo& Info)
{
    Pledge.PledgeID = Info.PledgeID;
    if (Info.Name)
        appStrcpy(Pledge.Name, Info.Name);
    if (Info.LeaderName)
        appStrcpy(Pledge.LeaderName, Info.LeaderName);
    Pledge.CrestID = Info.CrestID;
    Pledge.Level = Info.Level;
    Pledge.bHasCastle = Info.bHasCastle;
    Pledge.bHasHideout = Info.bHasHideout;
    Pledge.Unknown94 = Info.Unknown90;
    Pledge.Unknown98 = Info.Unknown94;
    Pledge.Status = Info.Status;
    Pledge.UnknownE8 = Info.Unknown9C;
    SetPledgeStatusText(Pledge, Info.Status);
}

// 0x10001ad0 (vslot 83) - the stock body is the empty `ret 4` the linker folded for dozens of slots
void NConsoleWnd::ReceivePledgeExtendedInfo(PledgeInfo& Info)
{
    NW_TRACE("ReceivePledgeExtendedInfo id=%d name=%ls leader=%ls", Info.PledgeID, Info.Name, Info.LeaderName);
}

// 0x1005a0b0 - the player's clan (or its new state): the clan page starts over with it
void NConsoleWnd::ReceivePledgeInfo(PledgeInfo& Info)
{
    guard(NConsoleWnd::ReceivePledgeInfo);
    NW_TRACE("ReceivePledgeInfo id=%d name=%ls leader=%ls ally=%ls level=%d crest=%d castle=%d hideout=%d status=%d", Info.PledgeID,
             Info.Name, Info.LeaderName, Info.AllyName, Info.Level, Info.CrestID, Info.bHasCastle, Info.bHasHideout, Info.Status);
    NPledgeInfo Pledge;
    MakePledgeInfo(Pledge, Info);
    GetDesktop()->MainWnd->ActionWnd->ResetPledgeWnd();
    GetDesktop()->MainWnd->ActionWnd->PledgeWnd->SetPledgeInfo(&Pledge);
    GetDesktop()->MainWnd->ActionWnd->UpdatePledgeTab();
    unguard;
}
NW_HOOK_METHOD(0x1005a0b0, NConsoleWnd, ReceivePledgeInfo)

// 0x1005a320 - the clan's changing values only (no names, no id)
void NConsoleWnd::ReceivePledgeInfoUpdate(PledgeInfo& Info)
{
    guard(NConsoleWnd::ReceivePledgeInfoUpdate);
    NW_TRACE("ReceivePledgeInfoUpdate level=%d crest=%d castle=%d hideout=%d status=%d", Info.Level, Info.CrestID, Info.bHasCastle,
             Info.bHasHideout, Info.Status);
    NPledgeInfo Pledge;
    Pledge.CrestID = Info.CrestID;
    Pledge.Level = Info.Level;
    Pledge.bHasCastle = Info.bHasCastle;
    Pledge.bHasHideout = Info.bHasHideout;
    Pledge.Unknown94 = Info.Unknown90;
    Pledge.Unknown98 = Info.Unknown94;
    Pledge.Status = Info.Status;
    Pledge.UnknownE8 = Info.Unknown9C;
    SetPledgeStatusText(Pledge, Info.Status);
    GetDesktop()->MainWnd->ActionWnd->PledgeWnd->UpdatePledgeInfo(&Pledge);
    GetDesktop()->MainWnd->ActionWnd->UpdatePledgeTab();
    unguard;
}
NW_HOOK_METHOD(0x1005a320, NConsoleWnd, ReceivePledgeInfoUpdate)

// 0x1006cb20 (vslot 91)
void NConsoleWnd::ReceiveCreatePledge(L2ParamStack& Params)
{
    if (Params.Top())
        ShowSystemMessage(MSG_ClanCreated, 0xFFB09B79);
    else
        ShowSystemMessage(MSG_ClanCreateFailed, 0xFFB09B79);
}
NW_HOOK_METHOD(0x1006cb20, NConsoleWnd, ReceiveCreatePledge)

// 0x10067570 - an invitation: the inviter's object id and the clan's name
void NConsoleWnd::ReceiveAskJoinPledge(L2ParamStack& Params)
{
    INT InviterID = (INT)Params.Top();
    TCHAR* PledgeName = (TCHAR*)Params.Top();
    User* Inviter = FindUserByID(InviterID);
    if (Inviter)
    {
        NDialogInfo Info;
        Info.ID = DIALOG_JOIN_PLEDGE;
        Info.bMainBox = 0;
        Info.bInput = 0;
        Info.SetSystemMessage(MSG_AskJoinPledge);
        Info.SetButtons(Inviter->GetName(), PledgeName);
        Info.bProgress = 1;
        ShowDialog(&Info);
    }
    else
    {
        L2ParamStack Answer(10);
        Answer.PushBack(NULL);
        RequestAnswerJoinPledge(Answer);
    }
}
NW_HOOK_METHOD(0x10067570, NConsoleWnd, ReceiveAskJoinPledge)

// 0x1006cb60 (vslot 93)
void NConsoleWnd::ReceiveOustPledgeMember(L2ParamStack& Params)
{
    if (Params.Top())
        ShowSystemMessage(MSG_ClanExpelDone, 0xFFB09B79);
    else
        ShowSystemMessage(MSG_ClanExpelFailed, 0xFFB09B79);
}
NW_HOOK_METHOD(0x1006cb60, NConsoleWnd, ReceiveOustPledgeMember)

// 0x1006cba0 (vslot 94)
void NConsoleWnd::ReceiveDismissPledge(L2ParamStack& Params)
{
    if (Params.Top())
        ShowSystemMessage(MSG_ClanDispersed, 0xFFB09B79);
    else
        ShowSystemMessage(MSG_ClanDisperseFailed, 0xFFB09B79);
}
NW_HOOK_METHOD(0x1006cba0, NConsoleWnd, ReceiveDismissPledge)

// 0x1006caf0 (vslot 95) - said only when it failed
void NConsoleWnd::ReceiveJoinPledge(L2ParamStack& Params)
{
    if (!Params.Top())
        ShowSystemMessage(MSG_ClanJoinFailed, 0xFFB09B79);
}
NW_HOOK_METHOD(0x1006caf0, NConsoleWnd, ReceiveJoinPledge)

// 0x1006cbe0
void NConsoleWnd::ReceiveWithdrawalPledge(L2ParamStack& Params)
{
    if (Params.Top())
        ShowSystemMessage(MSG_ClanWithdrawn, 0xFFB09B79);
    else
        ShowSystemMessage(MSG_ClanWithdrawFailed, 0xFFB09B79);
}
NW_HOOK_METHOD(0x1006cbe0, NConsoleWnd, ReceiveWithdrawalPledge)

// 0x1006cc20 (vslot 97) - the player was expelled
void NConsoleWnd::ReceiveOustPledgeMessage(L2ParamStack& Params)
{
    ShowSystemMessage(MSG_ClanMembershipEnded, 0xFFB09B79);
}
NW_HOOK_METHOD(0x1006cc20, NConsoleWnd, ReceiveOustPledgeMessage)

// 0x1006cc40 (vslot 98) - the player's clan was dispersed (in red)
void NConsoleWnd::ReceiveDismissPledgeMessage(L2ParamStack& Params)
{
    if (Params.Top())
        ShowSystemMessage(MSG_ClanDispersed, 0xFFFF0000);
    else
        ShowSystemMessage(MSG_ClanDisperseFailed, 0xFFB09B79);
}
NW_HOOK_METHOD(0x1006cc40, NConsoleWnd, ReceiveDismissPledgeMessage)

// 0x10067680 - a clan declares war: its name and the name of who asks, sent back with the
// answer
void NConsoleWnd::ReceiveStartPledgeWar(L2ParamStack& Params)
{
    TCHAR* PledgeName = (TCHAR*)Params.Top();
    TCHAR* Name = (TCHAR*)Params.Top();
    if (PledgeName && Name)
    {
        NDialogInfo Info;
        Info.ID = DIALOG_START_PLEDGE_WAR;
        Info.bMainBox = 0;
        Info.bInput = 0;
        Info.bProgress = 1;
        Info.SetSystemMessage(MSG_AskPledgeWar);
        Info.SetButtons(PledgeName, Name);
        Info.SetParamText(Name);
        ShowDialog(&Info);
    }
    else
    {
        L2ParamStack Answer(10);
        Answer.PushBack(Name);
        Answer.PushBack(NULL);
        RequestReplyStartPledgeWar(Answer);
    }
}
NW_HOOK_METHOD(0x10067680, NConsoleWnd, ReceiveStartPledgeWar)

// 0x1006cc80 (vslot 100) - the other clan's answer: 1 accepted, 0 refused
void NConsoleWnd::ReceiveReplyStartPledgeWar(L2ParamStack& Params)
{
    INT Result = (INT)Params.Top();
    if (Result > 0)
        ShowSystemMessage(MSG_PledgeWarAccepted, 0xFFB09B79);
    else if (Result == 0)
        ShowSystemMessage(MSG_PledgeWarRefused, 0xFFB09B79);
}
NW_HOOK_METHOD(0x1006cc80, NConsoleWnd, ReceiveReplyStartPledgeWar)

// 0x100677a0 - a clan asks to end the war: as ReceiveStartPledgeWar
void NConsoleWnd::ReceiveStopPledgeWar(L2ParamStack& Params)
{
    TCHAR* PledgeName = (TCHAR*)Params.Top();
    TCHAR* Name = (TCHAR*)Params.Top();
    if (PledgeName && Name)
    {
        NDialogInfo Info;
        Info.ID = DIALOG_STOP_PLEDGE_WAR;
        Info.bMainBox = 0;
        Info.bInput = 0;
        Info.bProgress = 1;
        Info.SetSystemMessage(MSG_AskPledgeTruce);
        Info.SetButtons(PledgeName, NULL);
        Info.SetParamText(Name);
        ShowDialog(&Info);
    }
    else
    {
        L2ParamStack Answer(10);
        Answer.PushBack(Name);
        Answer.PushBack(NULL);
        RequestReplyStopPledgeWar(Answer);
    }
}
NW_HOOK_METHOD(0x100677a0, NConsoleWnd, ReceiveStopPledgeWar)

// 0x1006ccc0 (vslot 102) - the other clan's answer: 1 accepted, 0 refused
void NConsoleWnd::ReceiveReplyStopPledgeWar(L2ParamStack& Params)
{
    INT Result = (INT)Params.Top();
    if (Result > 0)
        ShowSystemMessage(MSG_PledgeTruceAccepted, 0xFFB09B79);
    else if (Result == 0)
        ShowSystemMessage(MSG_PledgeTruceDenied, 0xFFB09B79);
}
NW_HOOK_METHOD(0x1006ccc0, NConsoleWnd, ReceiveReplyStopPledgeWar)

// 0x1006cd00 (vslot 103) - said only when it failed
void NConsoleWnd::ReceiveSurrenderPledgeWar(L2ParamStack& Params)
{
    if (!Params.Top())
        ShowSystemMessage(MSG_SurrenderFailed, 0xFFB09B79);
}
NW_HOOK_METHOD(0x1006cd00, NConsoleWnd, ReceiveSurrenderPledgeWar)

// 0x1006cd30 (vslot 104) - said only when it failed
void NConsoleWnd::ReceiveSurrenderPersonally(L2ParamStack& Params)
{
    if (!Params.Top())
        ShowSystemMessage(MSG_PersonalSurrenderFailed, 0xFFB09B79);
}
NW_HOOK_METHOD(0x1006cd30, NConsoleWnd, ReceiveSurrenderPersonally)

/*-----------------------------------------------------------------------------
    The alliance, the friends.
-----------------------------------------------------------------------------*/

// 0x100678c0 - an invitation: the inviter's object id, the alliance's name, a value not used
// and its leader's name
void NConsoleWnd::ReceiveAskJoinAlly(L2ParamStack& Params)
{
    INT InviterID = (INT)Params.Top();
    TCHAR* AllyName = (TCHAR*)Params.Top();
    Params.Top();
    TCHAR* LeaderName = (TCHAR*)Params.Top();
    if (FindUserByID(InviterID))
    {
        NDialogInfo Info;
        Info.ID = DIALOG_JOIN_ALLY;
        Info.bMainBox = 0;
        Info.bInput = 0;
        Info.bProgress = 1;
        Info.SetSystemMessage(MSG_AskJoinAlly);
        Info.SetButtons(AllyName, LeaderName);
        ShowDialog(&Info);
    }
    else
    {
        L2ParamStack Answer(10);
        Answer.PushBack(NULL);
        RequestAnswerJoinAlly(Answer);
    }
}
NW_HOOK_METHOD(0x100678c0, NConsoleWnd, ReceiveAskJoinAlly)

// 0x10054b80 - into the chat window
void NConsoleWnd::ReceiveDismissAlly(L2ParamStack& Params)
{
    if (Params.Top())
        GetDesktop()->AddSystemMessage(MSG_AllyDissolved, 0xFFB09B79);
    else
        GetDesktop()->AddSystemMessage(MSG_AllyDissolveFailed, 0xFFB09B79);
}
NW_HOOK_METHOD(0x10054b80, NConsoleWnd, ReceiveDismissAlly)

// 0x1006cd60 (vslot 114)
void NConsoleWnd::ReceiveJoinAlly(L2ParamStack& Params)
{
    if (Params.Top())
        ShowSystemMessage(MSG_AllyAccepted, 0xFFB09B79);
    else
        ShowSystemMessage(MSG_AllyInviteFailed, 0xFFB09B79);
}
NW_HOOK_METHOD(0x1006cd60, NConsoleWnd, ReceiveJoinAlly)

// 0x10054b40 - into the chat window
void NConsoleWnd::ReceiveOustAlly(L2ParamStack& Params)
{
    if (Params.Top())
        GetDesktop()->AddSystemMessage(MSG_AllyExpelDone, 0xFFB09B79);
    else
        GetDesktop()->AddSystemMessage(MSG_AllyExpelFailed, 0xFFB09B79);
}
NW_HOOK_METHOD(0x10054b40, NConsoleWnd, ReceiveOustAlly)

// 0x10054b00 - into the chat window
void NConsoleWnd::ReceiveWithdrawAlly(L2ParamStack& Params)
{
    if (Params.Top())
        GetDesktop()->AddSystemMessage(MSG_AllyWithdrawn, 0xFFB09B79);
    else
        GetDesktop()->AddSystemMessage(MSG_AllyWithdrawFailed, 0xFFB09B79);
}
NW_HOOK_METHOD(0x10054b00, NConsoleWnd, ReceiveWithdrawAlly)

// 0x100679e0 - an alliance challenges the player's: its name and the name of who asks, sent
// back with the answer; without them the answer is -1
void NConsoleWnd::StartAllianceWar(TCHAR* AllianceName, TCHAR* Name)
{
    if (AllianceName && Name)
    {
        NDialogInfo Info;
        Info.ID = DIALOG_START_ALLY_WAR;
        Info.bMainBox = 0;
        Info.bInput = 0;
        Info.bProgress = 1;
        Info.SetSystemMessage(MSG_AskAllyWar);
        Info.SetButtons(AllianceName, Name);
        Info.SetParamText(Name);
        ShowDialog(&Info);
    }
    else
    {
        L2ParamStack Answer(10);
        Answer.PushBack(Name);
        Answer.PushBack((void*)-1);
        RequestReplyStartAllianceWar(Answer);
    }
}
NW_HOOK_METHOD(0x100679e0, NConsoleWnd, StartAllianceWar)

// 0x10067b00 - an alliance asks for a cease-fire: as StartAllianceWar, without the progress bar
void NConsoleWnd::StopAllianceWar(TCHAR* AllianceName, TCHAR* Name)
{
    if (AllianceName && Name)
    {
        NDialogInfo Info;
        Info.ID = DIALOG_STOP_ALLY_WAR;
        Info.bMainBox = 0;
        Info.bInput = 0;
        Info.SetSystemMessage(MSG_AskAllyTruce);
        Info.SetButtons(AllianceName, Name);
        Info.SetParamText(Name);
        ShowDialog(&Info);
    }
    else
    {
        L2ParamStack Answer(10);
        Answer.PushBack(Name);
        Answer.PushBack((void*)-1);
        RequestReplyStopAllianceWar(Answer);
    }
}
NW_HOOK_METHOD(0x10067b00, NConsoleWnd, StopAllianceWar)

// 0x1006cda0 (vslot 117)
void NConsoleWnd::ReceiveFriendAddRequestResult(L2ParamStack& Params)
{
    if (Params.Top())
        ShowSystemMessage(MSG_FriendInvited, 0xFFB09B79);
    else
        ShowSystemMessage(MSG_FriendInviteFailed, 0xFFB09B79);
}
NW_HOOK_METHOD(0x1006cda0, NConsoleWnd, ReceiveFriendAddRequestResult)

// 0x10067f60 - who asks
void NConsoleWnd::ReceiveFriendAddRequest(L2ParamStack& Params)
{
    TCHAR* Name = (TCHAR*)Params.Top();
    NDialogInfo Info;
    Info.ID = DIALOG_FRIEND_INVITE;
    Info.bMainBox = 0;
    Info.bInput = 0;
    Info.bProgress = 1;
    Info.SetSystemMessage(MSG_AskFriend);
    Info.SetButtons(Name, NULL);
    ShowDialog(&Info);
}
NW_HOOK_METHOD(0x10067f60, NConsoleWnd, ReceiveFriendAddRequest)

/*-----------------------------------------------------------------------------
    The party, its matching, the chat channels.
-----------------------------------------------------------------------------*/

// 0x10068020 - an invitation: the inviter's object id and how the items are shared (0 who
// picks them up, 1 at random, C2: 2 at random with the spoil, 3 by turn, 4 by turn with the spoil; for
// another value the stock leaves the text uninitialized, empty here)
void NConsoleWnd::ReceiveAskJoinParty(L2ParamStack& Params)
{
    TCHAR Text[128];
    Text[0] = 0;
    INT InviterID = (INT)Params.Top();
    INT Distribution = (INT)Params.Top();
    User* Inviter = FindUserByID(InviterID);
    switch (Distribution)
    {
    case 0:
        appSprintf(Text, TEXT("%s"), *GL2GameData.SystemMsg[MSG_AskJoinPartyLooter].Message);
        break;
    case 1:
        appSprintf(Text, TEXT("%s"), *GL2GameData.SystemMsg[MSG_AskJoinPartyRandom].Message);
        break;
    case 2:
        appSprintf(Text, TEXT("%s"), *GL2GameData.SystemMsg[MSG_AskJoinPartyRandomSpoil].Message);
        break;
    case 3:
        appSprintf(Text, TEXT("%s"), *GL2GameData.SystemMsg[MSG_AskJoinPartyByTurn].Message);
        break;
    case 4:
        appSprintf(Text, TEXT("%s"), *GL2GameData.SystemMsg[MSG_AskJoinPartyByTurnSpoil].Message);
        break;
    }
    if (Inviter)
    {
        NDialogInfo Info;
        Info.ID = DIALOG_JOIN_PARTY;
        Info.bMainBox = 0;
        Info.bInput = 0;
        Info.SetText(Text);
        Info.SetButtons(Inviter->GetName(), NULL);
        Info.bProgress = 1;
        ShowDialog(&Info);
    }
    else
    {
        L2ParamStack Answer(10);
        Answer.PushBack(NULL);
        RequestAnswerJoinParty(Answer);
    }
}
NW_HOOK_METHOD(0x10068020, NConsoleWnd, ReceiveAskJoinParty)

// 0x1006cde0 (vslot 106) - said only when refused
void NConsoleWnd::ReceiveJoinParty(L2ParamStack& Params)
{
    if (!Params.Top())
        ShowSystemMessage(MSG_PartyDeclined, 0xFFB09B79);
}
NW_HOOK_METHOD(0x1006cde0, NConsoleWnd, ReceiveJoinParty)

// 0x10054de0 - into the chat window
void NConsoleWnd::ReceiveWithdrawalParty(L2ParamStack& Params)
{
    if (Params.Top())
        GetDesktop()->AddSystemMessage(MSG_PartyWithdrawn, 0xFFB09B79);
    else
        GetDesktop()->AddSystemMessage(MSG_PartyWithdrawFailed, 0xFFB09B79);
}
NW_HOOK_METHOD(0x10054de0, NConsoleWnd, ReceiveWithdrawalParty)

// 0x1006ce10 (vslot 108) - said only when it failed
void NConsoleWnd::ReceiveOustPartyMember(L2ParamStack& Params)
{
    if (!Params.Top())
        ShowSystemMessage(MSG_PartyExpelFailed, 0xFFB09B79);
}
NW_HOOK_METHOD(0x1006ce10, NConsoleWnd, ReceiveOustPartyMember)

// 0x1006ce40 (vslot 109) - the player was expelled
void NConsoleWnd::ReceiveOustPartyMessage(L2ParamStack& Params)
{
    ShowSystemMessage(MSG_PartyExpelled, 0xFFB09B79);
}
NW_HOOK_METHOD(0x1006ce40, NConsoleWnd, ReceiveOustPartyMessage)

// 0x10054e20 - into the chat window
void NConsoleWnd::ReceiveDismissParty(L2ParamStack& Params)
{
    if (Params.Top())
        GetDesktop()->AddSystemMessage(MSG_PartyDispersed, 0xFFB09B79);
    else
        GetDesktop()->AddSystemMessage(MSG_PartyDisperseFailed, 0xFFB09B79);
}
NW_HOOK_METHOD(0x10054e20, NConsoleWnd, ReceiveDismissParty)

// 0x1006ce60 (vslot 111) - the player's party was dispersed
void NConsoleWnd::ReceiveDismissPartyMessage(L2ParamStack& Params)
{
    ShowSystemMessage(MSG_PartyDispersed, 0xFFB09B79);
}
NW_HOOK_METHOD(0x1006ce60, NConsoleWnd, ReceiveDismissPartyMessage)

// 0x10054e60 - a member joined: object id, name, HP, max HP, MP, max MP, then four values
// not used (class, level...); the party's chat tab comes with it
void NConsoleWnd::PartyAddMember(L2ParamStack& Params)
{
    INT MemberID = (INT)Params.Top();
    const TCHAR* Name = (const TCHAR*)Params.Top();
    INT HP = (INT)Params.Top();
    INT MaxHP = (INT)Params.Top();
    INT MP = (INT)Params.Top();
    INT MaxMP = (INT)Params.Top();
    Params.Top();
    Params.Top();
    Params.Top();
    Params.Top();
    GetDesktop()->PartyWnd->AddMember(MemberID, Name, HP, MaxHP, MP, MaxMP);
    GetDesktop()->ChatWnd->EnablePartyTab(true);
}
NW_HOOK_METHOD(0x10054e60, NConsoleWnd, PartyAddMember)

// 0x10054f00 - as PartyAddMember, two values not used
void NConsoleWnd::PartyUpdateMember(L2ParamStack& Params)
{
    INT MemberID = (INT)Params.Top();
    const TCHAR* Name = (const TCHAR*)Params.Top();
    INT HP = (INT)Params.Top();
    INT MaxHP = (INT)Params.Top();
    INT MP = (INT)Params.Top();
    INT MaxMP = (INT)Params.Top();
    Params.Top();
    Params.Top();
    GetDesktop()->PartyWnd->UpdateMember(MemberID, Name, HP, MaxHP, MP, MaxMP);
}
NW_HOOK_METHOD(0x10054f00, NConsoleWnd, PartyUpdateMember)

// 0x10054f80 - the member's id and name
void NConsoleWnd::PartyDeleteMember(L2ParamStack& Params)
{
    INT MemberID = (INT)Params.Top();
    const TCHAR* Name = (const TCHAR*)Params.Top();
    GetDesktop()->PartyWnd->DeleteMember(MemberID, Name);
}
NW_HOOK_METHOD(0x10054f80, NConsoleWnd, PartyDeleteMember)

// 0x10054fc0 - the party is gone, and its chat tab with it
void NConsoleWnd::PartyDeleteAll(L2ParamStack& Params)
{
    GetDesktop()->PartyWnd->DeleteAll();
    GetDesktop()->ChatWnd->EnablePartyTab(false);
}
NW_HOOK_METHOD(0x10054fc0, NConsoleWnd, PartyDeleteAll)

// 0x10054ff0 (vslot 174) - a new list of the party matching room follows (AddPartyMatchMember)
void NConsoleWnd::ReceivePartyMatchList()
{
    GetDesktop()->MatchWnd->ClearList();
}
NW_HOOK_METHOD(0x10054ff0, NConsoleWnd, ReceivePartyMatchList)

// 0x10055010 (vslot 175)
void NConsoleWnd::AddPartyMatchMember(L2ParamStack& Params)
{
    GetDesktop()->MatchWnd->AddPlayer(Params);
}
NW_HOOK_METHOD(0x10055010, NConsoleWnd, AddPartyMatchMember)

// 0x10055030
void NConsoleWnd::ReceivePartyMatchDetail(L2ParamStack& Params)
{
    GetDesktop()->MatchWnd->UpdatePlayerInfo(Params);
}
NW_HOOK_METHOD(0x10055030, NConsoleWnd, ReceivePartyMatchDetail)

// 0x10055bb0
void NConsoleWnd::EnablePartyChat(bool bEnable)
{
    GetDesktop()->ChatWnd->EnablePartyTab(bEnable);
}
NW_HOOK_METHOD(0x10055bb0, NConsoleWnd, EnablePartyChat)

// 0x10055bd0
void NConsoleWnd::EnablePledgeChat(bool bEnable)
{
    GetDesktop()->ChatWnd->EnablePledgeTab(bEnable);
}
NW_HOOK_METHOD(0x10055bd0, NConsoleWnd, EnablePledgeChat)

// 0x10055bf0
void NConsoleWnd::EnableAllyChat(bool bEnable)
{
    GetDesktop()->ChatWnd->EnableAllyTab(bEnable);
}
NW_HOOK_METHOD(0x10055bf0, NConsoleWnd, EnableAllyChat)

// 0x1006a790 - a chat line: who says it (object id), its channel, the name and the text (run
// through the word filter first); coloured by the channel. 0 without a text.
UBOOL NConsoleWnd::Say2(L2ParamStack& Params)
{
    INT ObjectID = (INT)Params.Top();
    INT Type = (INT)Params.Top();
    TCHAR* Name = (TCHAR*)Params.Top();
    TCHAR* Text = (TCHAR*)Params.Top();
    HasBadWord(Text);
    if (!Text)
        return 0;
    if (Type == SAY2_Shout)
        GetDesktop()->Say(ObjectID, Name, Text, 0xFFFF7200, Type);
    else if (Type == SAY2_Party)
        GetDesktop()->Say(ObjectID, Name, Text, 0xFF00FF00, Type);
    else if (Type == SAY2_Pledge)
        GetDesktop()->Say(ObjectID, Name, Text, 0xFF7D77FF, Type);
    else if (Type == SAY2_Tell)
    {
        GetDesktop()->Say(ObjectID, Name, Text, 0xFFFF00FF, Type);
        GetDesktop()->ChatWnd->AddTellName(Name);
    }
    else if (Type == SAY2_Market)
        GetDesktop()->Say(ObjectID, Name, Text, 0xFFEAA5F5, Type);
    else if (Type == SAY2_Ally)
        GetDesktop()->Say(ObjectID, Name, Text, 0xFF77FF99, Type);
    else if (Type == SAY2_PetitionPlayer)
        GetDesktop()->Say(ObjectID, Name, Text, 0xFF80FFFF, Type);
    else if (Type == SAY2_PetitionGM)
        GetDesktop()->Say(ObjectID, Name, Text, 0xFF80FFFF, Type);
    else if (Type == SAY2_Announcement)
        GetDesktop()->Say(ObjectID, *GL2GameData.SysString[STR_Announcement], Text, 0xFF80FFFF,
                          Type);
    else
    {
#if NW_FEATURES
        // phase 6: ChatColors: command channel and hero voice in their Interlude colors (the VIP
        // chat is sent as one of these types); still a normal line
        DWORD Color;
        if (nwf::ChatColors_Find(Type, &Color))
        {
            GetDesktop()->Say(ObjectID, Name, Text, Color, 0);
            return 1;
        }
#endif
        GetDesktop()->Say(ObjectID, Name, Text, 0xFFFFFFFF, 0);
    }
    return 1;
}
NW_HOOK_METHOD(0x1006a790, NConsoleWnd, Say2)

// 0x10051dc0 - an NPC speaks: its object id, the channel, its npc id (+ 1000000) and the text;
// as Say2, with the NPC's name (announcements go to the petition channel's type)
UBOOL NConsoleWnd::NpcSay(L2ParamStack& Params)
{
    guard(NConsoleWnd::NpcSay);
    INT ObjectID = (INT)Params.Top();
    INT Type = (INT)Params.Top();
    INT NpcID = (INT)Params.Top();
    TCHAR* Text = (TCHAR*)Params.Top();
    if (!Text || !*Text)
        return 0;
    TCHAR* Name = GL2GameData.GetNpcName(NpcID - 1000000);
    if (Type == SAY2_Shout)
        GetDesktop()->Say(ObjectID, Name, Text, 0xFFFF7200, Type);
    else if (Type == SAY2_Party)
        GetDesktop()->Say(ObjectID, Name, Text, 0xFF00FF00, Type);
    else if (Type == SAY2_Pledge)
        GetDesktop()->Say(ObjectID, Name, Text, 0xFF7D77FF, Type);
    else if (Type == SAY2_Tell)
    {
        GetDesktop()->Say(ObjectID, Name, Text, 0xFFFF00FF, Type);
        GetDesktop()->ChatWnd->AddTellName(Name);
    }
    else if (Type == SAY2_Market)
        GetDesktop()->Say(ObjectID, Name, Text, 0xFFEAA5F5, Type);
    else if (Type == SAY2_Ally)
        GetDesktop()->Say(ObjectID, Name, Text, 0xFF77FF99, Type);
    else if (Type == SAY2_PetitionPlayer)
        GetDesktop()->Say(ObjectID, Name, Text, 0xFF80FFFF, Type);
    else if (Type == SAY2_PetitionGM)
        GetDesktop()->Say(ObjectID, Name, Text, 0xFF80FFFF, Type);
    else if (Type == SAY2_Announcement)
        GetDesktop()->Say(ObjectID, *GL2GameData.SysString[STR_Announcement], Text, 0xFF80FFFF,
                          SAY2_PetitionGM);
    else
        GetDesktop()->Say(ObjectID, Name, Text, 0xFFFFFFFF, 0);
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x10051dc0, NConsoleWnd, NpcSay)

/*-----------------------------------------------------------------------------
    The shortcuts, the buffs, the skills, the trainer, the restart menu, the quests.
-----------------------------------------------------------------------------*/

// 0x100545f0 - a new list follows (AddShortCutItem)
void NConsoleWnd::ReceiveShortCutList()
{
    GetDesktop()->ShortCutWnd->ClearShortCuts();
}
NW_HOOK_METHOD(0x100545f0, NConsoleWnd, ReceiveShortCutList)

// 0x100670b0 (vslot 155) - a shortcut: its type (1 item, 2 skill, 3 action, 4 macro: new in C2), its slot, then an
// item's object id and owner (1 the inventory, 2 the pet's), a skill's id and level, an action's or a macro's id,
// and whose shortcut it is. An item not found is not set; for another type the stock passes an uninitialized owner
// (0 here). C2: the item's crystal grade and the two new numbers go along; the macro's icon is "L2UI.MacroWnd.
// Macro_Icon<n + 1>" when it has one (the window's SetShortCut is 0x100cfe90, NCShortCutWnd::SetShortCut)
void NConsoleWnd::AddShortCutItem(L2ParamStack& Params)
{
    if (!nw::Field<NCWnd*>(Desktop, 0x104))
        return;
    INT Owner = 0;
    NItemInfo Info;
    Info.Param = (INT)Params.Top();
    Info.UnknownC90 = (INT)Params.Top();
    switch (Info.Param)
    {
    case 1:
    {
        Info.ObjectID = (INT)Params.Top();
        Owner = (INT)Params.Top();
        NItemEntry* Item;
        if (Owner == 1)
            Item = GetDesktop()->InventoryWnd->FindItem(Info.ObjectID);
        else if (Owner == 2)
            Item = GetDesktop()->PetWnd->InventoryWnd->GetItem(Info.ObjectID);
        else
            return;
        if (!Item)
            return;
        Info.ID = Item->ID;
        if (Item->Name && Item->Name[0])
            appStrcpy(Info.Name, Item->Name);
        if (Item->AdditionalName && Item->AdditionalName[0])
            appStrcpy(Info.AdditionalName, Item->AdditionalName);
        if (Item->IconName && Item->IconName[0])
            appStrcpy(Info.IconName, Item->IconName);
        Info.Enchant = Item->Enchant;
        Info.bStackable = Item->bStackable;
        Info.Count = Item->Count;
        Info.UnknownC34[7] = Item->CrystalGrade;
        Info.Unknown17A0 = Item->Unknown1998;
        Info.Unknown1788 = Item->Unknown19AC;
        break;
    }
    case 2:
        Info.ID = (INT)Params.Top();
        Info.UnknownC20 = (INT)Params.Top();
        Owner = (INT)Params.Top();
        if (GetSkillIconName(Info.ID, Info.UnknownC20))
            appStrcpy(Info.IconName, GetSkillIconName(Info.ID, Info.UnknownC20));
        if (GetSkillName(Info.ID, Info.UnknownC20))
            appStrcpy(Info.Name, GetSkillName(Info.ID, Info.UnknownC20));
        Info.UnknownC34[12] = GetSkillMpCost(Info.ID, Info.UnknownC20);
        break;
    case 3:
        Info.ID = (INT)Params.Top();
        Owner = (INT)Params.Top();
        if (GetActionIconName(Info.ID))
            appStrcpy(Info.IconName, GetActionIconName(Info.ID));
        if (GetActionName(Info.ID))
            appStrcpy(Info.Name, GetActionName(Info.ID));
        break;
    case 4:
    {
        Info.ID = (INT)Params.Top();
        Owner = (INT)Params.Top();
        if (GetMacroIconType(Info.ID) >= 0)                                     // the macro's icon index
            appSprintf(Info.IconName, TEXT("%s%d"), TEXT("L2UI.MacroWnd.Macro_Icon"),
                       GetMacroIconType(Info.ID) + 1);
        if (GetMacroName(Info.ID))                                              // the macro's name
            appStrcpy(Info.Name, GetMacroName(Info.ID));
        break;
    }
    }
    GetDesktop()->ShortCutWnd->SetShortCut(&Info, Owner);
}
NW_HOOK_METHOD(0x100670b0, NConsoleWnd, AddShortCutItem)

// 0x10054630 (vslot 156) - the shortcuts of an inventory item (C2: a slot of its own, C1 had only the pet's)
void NConsoleWnd::UpdateShortCutItem(INT ObjectID)
{
    if (GetDesktop()->ShortCutWnd)
        GetDesktop()->ShortCutWnd->UpdateItem(ObjectID);
}
NW_HOOK_METHOD(0x10054630, NConsoleWnd, UpdateShortCutItem)

// 0x10054650 (vslot 157) - the shortcuts of a pet inventory item
void NConsoleWnd::UpdatePetShortCutItem(INT ObjectID)
{
    if (GetDesktop()->ShortCutWnd)
        GetDesktop()->ShortCutWnd->UpdatePetItem(ObjectID);
}
NW_HOOK_METHOD(0x10054650, NConsoleWnd, UpdatePetShortCutItem)

// 0x10054670 (vslot 158)
void NConsoleWnd::ReceiveShortCutDelete(INT Slot)
{
    if (GetDesktop()->ShortCutWnd)
        GetDesktop()->ShortCutWnd->DeleteShortCut(Slot);
}
NW_HOOK_METHOD(0x10054670, NConsoleWnd, ReceiveShortCutDelete)

// 0x100543c0 - its value is not used
void NConsoleWnd::ReceiveMagicSkillList(L2ParamStack& Params)
{
    Params.Top();
}
NW_HOOK_METHOD(0x100543c0, NConsoleWnd, ReceiveMagicSkillList)

// 0x10054690 (vslot 161) - a new list of the buffs follows (AddAbnormalStatus)
void NConsoleWnd::ReceiveAbnormalStatus()
{
    GetDesktop()->AbnormalStatusWnd->ClearStatus();
}
NW_HOOK_METHOD(0x10054690, NConsoleWnd, ReceiveAbnormalStatus)

// 0x10059a00 - a buff: the skill's id, its level and the seconds left (C2: bigger texts in the record, and the
// window's AddStatus is 0x100d8650)
void NConsoleWnd::AddAbnormalStatus(L2ParamStack& Params)
{
    FAbnormalStatus Status;
    Status.SkillID = (INT)Params.Top();
    Status.Level = (INT)Params.Top();
    Status.RemainTime = (INT)Params.Top();
    if (GetSkillIconName(Status.SkillID, Status.Level))
        appStrcpy(Status.IconName, GetSkillIconName(Status.SkillID, Status.Level));
    if (GetSkillName(Status.SkillID, Status.Level))
        appStrcpy(Status.Name, GetSkillName(Status.SkillID, Status.Level));
    if (GetSkillDesc(Status.SkillID, Status.Level))
        appStrcpy(Status.Desc, GetSkillDesc(Status.SkillID, Status.Level));
    GetDesktop()->AbnormalStatusWnd->AddStatus(&Status);
}
NW_HOOK_METHOD(0x10059a00, NConsoleWnd, AddAbnormalStatus)

// A skill's icon, name and description for the skill windows (each getter is asked twice)
static FORCEINLINE void SetSkillTexts(NConsoleWnd* Console, NItemInfo& Info)
{
    if (Console->GetSkillIconName(Info.ID, Info.UnknownC20))
        appStrcpy(Info.IconName, Console->GetSkillIconName(Info.ID, Info.UnknownC20));
    if (Console->GetSkillName(Info.ID, Info.UnknownC20))
        appStrcpy(Info.Name, Console->GetSkillName(Info.ID, Info.UnknownC20));
    if (Console->GetSkillDesc(Info.ID, Info.UnknownC20))
        appStrcpy(Info.Description, Console->GetSkillDesc(Info.ID, Info.UnknownC20));
}

// 0x10058f80 - a skill of the player: passive or not, its level and id (level 0: none)
void NConsoleWnd::ReceiveSkillList(L2ParamStack& Params)
{
    NItemInfo Info;
    Info.Param = (INT)Params.Top();
    Info.UnknownC20 = (INT)Params.Top();
    Info.ID = (INT)Params.Top();
    if (Info.UnknownC20 > 0)
    {
        SetSkillTexts(this, Info);
        static_cast<NCMagicSkillWnd*>(GetDesktop()->MainWnd->SkillWnd)->SkillWnd->AddSkill(&Info);
    }
}
NW_HOOK_METHOD(0x10058f80, NConsoleWnd, ReceiveSkillList)

// 0x10061210 - a skill's cool time (its id and level, the whole time and the time left, in seconds) starts on its
// shortcuts and on the skill window's icon, when shown. C2: a shortcut slot is 0x1A7C bytes and starts with an
// NItemEntry (its ID and level at +0x1918 / +0x191C), the cool time at +0x1A28
void NConsoleWnd::SkillCoolTime(INT SkillID, INT Level, INT Total, INT Remaining)
{
    FLOAT TotalTime = (FLOAT)Total;
    if (!(TotalTime > 0.f) || Total < Remaining)
        return;
    if (GetDesktop()->ShortCutWnd->IsShowWindow())
    {
        INT Num = GetDesktop()->ShortCutWnd->Pages * GetDesktop()->ShortCutWnd->Cols;
        for (INT i = 0; i < Num; i++)
        {
            NItemEntry* Item = (NItemEntry*)GetDesktop()->ShortCutWnd->ItemAt(i);
            if (Item->ID == SkillID && Item->UnknownC18 == Level)
                GetDesktop()->ShortCutWnd->ItemCoolTime(i)->Start(TotalTime, Remaining);
        }
    }
    if (static_cast<NCMagicSkillWnd*>(GetDesktop()->MainWnd->SkillWnd)->SkillWnd->IsShowWindow())
    {
        INT Count = static_cast<NCMagicSkillWnd*>(GetDesktop()->MainWnd->SkillWnd)->SkillWnd
                        ->ActiveList->GetItemCount();
        for (INT i = 0; i < Count; i++)
        {
            NCSkillItemWnd* List =
                static_cast<NCMagicSkillWnd*>(GetDesktop()->MainWnd->SkillWnd)->SkillWnd->ActiveList;
            NItemEntry* Item = List->Items(i);
            if (Item->ID == SkillID && Item->UnknownC18 == Level)
                List->CoolTimes(i)->Start(TotalTime, Remaining);
        }
    }
}
NW_HOOK_METHOD(0x10061210, NConsoleWnd, SkillCoolTime)

// 0x100543d0 (vslot 178) - the skill list follows (ReceiveSkillList)
void NConsoleWnd::ShowSkillWnd()
{
    static_cast<NCMagicSkillWnd*>(GetDesktop()->MainWnd->SkillWnd)->SkillWnd->ClearLists();
}
NW_HOOK_METHOD(0x100543d0, NConsoleWnd, ShowSkillWnd)

// 0x10054300 - the trainer's list follows (AddAcquireSkillInfo)
void NConsoleWnd::ShowTrainListWnd()
{
    GetDesktop()->TrainWnd->Reset();
    GetDesktop()->TrainWnd->ShowWindow();
    GetDesktop()->TrainWnd->ShowList();
}
NW_HOOK_METHOD(0x10054300, NConsoleWnd, ShowTrainListWnd)

// 0x10054360
void NConsoleWnd::HideTrainWnd()
{
    GetDesktop()->TrainWnd->HideWindow();
}
NW_HOOK_METHOD(0x10054360, NConsoleWnd, HideTrainWnd)

// 0x1006da70 - the trainer's details of a skill: its id, level and SP, with the skill data's
// texts, MP cost and range
void NConsoleWnd::ShowTrainInfoWnd(L2ParamStack& Params)
{
    NTrainSkillInfo Info;
    Info.SkillID = (INT)Params.Top();
    Info.Level = (INT)Params.Top();
    Info.SP = (INT)Params.Top();
    if (GetSkillIconName(Info.SkillID, Info.Level))
        appStrcpy(Info.IconName, GetSkillIconName(Info.SkillID, Info.Level));
    if (GetSkillName(Info.SkillID, Info.Level))
        appStrcpy(Info.Name, GetSkillName(Info.SkillID, Info.Level));
    if (GetSkillDesc(Info.SkillID, Info.Level))
        ConvertNewLines(GetSkillDesc(Info.SkillID, Info.Level), Info.Desc);
    if (GetSkillTypeText(Info.SkillID, Info.Level))
        appStrcpy(Info.Unknown20C, GetSkillTypeText(Info.SkillID, Info.Level));
    if (GConsole->GetSkillMpCost(Info.SkillID, Info.Level))
        Info.MpCost = GConsole->GetSkillMpCost(Info.SkillID, Info.Level);
    if (GConsole->GetSkillRange(Info.SkillID, Info.Level))
        Info.Range = GConsole->GetSkillRange(Info.SkillID, Info.Level);
    GetDesktop()->TrainWnd->InfoWnd->Reset();
    GetDesktop()->TrainWnd->SetSkillInfo(&Info);
    GetDesktop()->TrainWnd->ShowWindow();
    GetDesktop()->TrainWnd->ShowInfo();
}
NW_HOOK_METHOD(0x1006da70, NConsoleWnd, ShowTrainInfoWnd)

// 0x10058db0 - a skill the trainer teaches: its id, the next level, the highest (not used), the SP it costs and one
// more value (C2: the record is 0x149C bytes; the window's AddEntry is 0x100dde50)
void NConsoleWnd::AddAcquireSkillInfo(L2ParamStack& Params)
{
    NTrainEntry Entry;
    Entry.SkillID = (INT)Params.Top();
    Entry.Level = (INT)Params.Top();
    Params.Top();
    Entry.SP = (INT)Params.Top();
    Entry.UnknownA94 = (INT)Params.Top();
    if (GetSkillName(Entry.SkillID, Entry.Level))
        appStrcpy(Entry.Name, GetSkillName(Entry.SkillID, Entry.Level));
    if (GetSkillDesc(Entry.SkillID, Entry.Level))
        appStrcpy(Entry.Desc, GetSkillDesc(Entry.SkillID, Entry.Level));
    if (GetSkillTypeText(Entry.SkillID, Entry.Level))
        appStrcpy(Entry.UnknownA00, GetSkillTypeText(Entry.SkillID, Entry.Level));
    if (GetSkillIconName(Entry.SkillID, Entry.Level))
        appStrcpy(Entry.IconName, GetSkillIconName(Entry.SkillID, Entry.Level));
    GetDesktop()->TrainWnd->AddEntry(&Entry);
}
NW_HOOK_METHOD(0x10058db0, NConsoleWnd, AddAcquireSkillInfo)

// 0x10058be0 (vslot 195) - the player died: the restart menu with its places. C2: the second
// dialog box is not Reset; the stock calls NControl::NotifyParent(0) (0x10005820) on its member at +0x1EC
// (the NCGaraDialogBox's progress bar; the console holds the box as an NCDialogBox*)
void NConsoleWnd::ShowRestartMenu(L2ParamStack& Params)
{
    L2ParamStack Cancel(10);
    RequestTargetCancel(Cancel);
    GetDesktop()->MainDialogBox->Reset();
    ((NCGaraDialogBox*)GetDesktop()->SecondDialogBox)->Progress->NotifyParent(0);
    GetDesktop()->AbnormalStatusWnd->ClearStatus();
    INT bVillage = (INT)Params.Top();
    INT bHideaway = (INT)Params.Top();
    INT bCastle = (INT)Params.Top();
    INT bSiegeHQ = (INT)Params.Top();
    INT bFixed = (INT)Params.Top();
    if (bVillage || bHideaway || bCastle || bSiegeHQ || bFixed)
    {
        RestartMenuWnd->SetRestartPoints(bVillage, bHideaway, bCastle, bSiegeHQ, bFixed);
        RestartMenuWnd->ShowWindow();
    }
}
NW_HOOK_METHOD(0x10058be0, NConsoleWnd, ShowRestartMenu)

// 0x100532a0 - the player was teleported: no restart menu any more
void NConsoleWnd::TeleportToLocation()
{
    RestartMenuWnd->HideWindow();
}
NW_HOOK_METHOD(0x100532a0, NConsoleWnd, TeleportToLocation)

// 0x10055fa0 - copied and dropped
void NConsoleWnd::UpdatePlayerInfo(L2ParamStack& Params)
{
    L2ParamStack Copy(10);
    Copy = Params;
}
NW_HOOK_METHOD(0x10055fa0, NConsoleWnd, UpdatePlayerInfo)

// 0x100540d0 - a new list follows (AddQuestID)
void NConsoleWnd::ReceiveQuestList()
{
    guard(NConsoleWnd::ReceiveQuestList);
    static_cast<NCQuestWnd*>(GetDesktop()->MainWnd->QuestWnd)->Clear();
    unguard;
}
NW_HOOK_METHOD(0x100540d0, NConsoleWnd, ReceiveQuestList)

// 0x10054160 (vslot 124) - a quest of the player at its step, listed under its name (the first line of
// questname-*.dat with its id, whatever the step)
void NConsoleWnd::AddQuestID(INT QuestID, INT Step)
{
    guard(NConsoleWnd::AddQuestID);
    for (INT i = 0; i < 1000; i++)
    {
        NQuestNameLine& Data = GQuestNames[i];
        if (Data.bValid && Data.QuestID == QuestID)
        {
            static_cast<NCQuestWnd*>(GetDesktop()->MainWnd->QuestWnd)->AddQuest(QuestID, *Data.Name,
                                                                               Step);
            break;
        }
    }
    unguard;
}
NW_HOOK_METHOD(0x10054160, NConsoleWnd, AddQuestID)

/*-----------------------------------------------------------------------------
    The system messages with parameters: each parameter comes as a text (AddSystemMessageParam)
    before the message's id (EndSystemMessageParam).
-----------------------------------------------------------------------------*/

// 0x1006ce80 (vslot 184) - a parameter by its type: 0 text, 1 number, 2 NPC (id + 1000000), 3 item,
// 4 skill and level, 5 castle
void NConsoleWnd::AddSystemMessageParam(L2ParamStack& Params)
{
    NSysMsgParam Param;
    Param.Text = NULL;
    Param.Len = 0;
    switch ((INT)Params.Top())
    {
    case 0:
        Param.Set((TCHAR*)Params.Top());
        break;
    case 1:
        Param.SetNumber((INT)Params.Top());
        break;
    case 2:
        Param.Set(GL2GameData.GetNpcName((INT)Params.Top() - 1000000));
        break;
    case 3:
        Param.Set(GetItemName((INT)Params.Top()));
        break;
    case 4:
    {
        INT SkillID = (INT)Params.Top();
        INT Level = (INT)Params.Top();
        Param.Set(GetSkillName(SkillID, Level));
        break;
    }
    case 5:
        Param.Set(GetCastleName((INT)Params.Top()));
        break;
    }
    SysMsgParams.AddItem(Param);
}
NW_HOOK_METHOD(0x1006ce80, NConsoleWnd, AddSystemMessageParam)

// 0x1006cf70 - the message: "$sN" takes the text of parameter N; "%" and a Korean particle take the particle's form
// that fits the last parameter's end. Shown in the chat window (the message's type and color as the chat's), or in a
// dialog box for the ones of type 4, with the sound of its line; then the parameters are freed. C2: the text can be
// asked for instead (P2 set: copied to the buffer P3, nothing shown); UGameEngine::OnEndSystemMessageParam passes 0, 0.
// An id the table does not have is a line of the chat in an error message (the stock's call has the sub type and the
// color the other way round: kept)
void NConsoleWnd::EndSystemMessageParam(L2ParamStack& Params, INT P2, INT P3)
{
    INT ID = (INT)Params.Top();
    TCHAR Text[1024] = TEXT("");
    INT NumParams = SysMsgParams.Num();
    INT Last = -1;
    INT Len = 0;
    if (!GL2GameData.SystemMsg[ID].Unknown0 || ID < 0 || ID >= 3000)
    {
        TCHAR Error[254];
        appSprintf(Error, TEXT("\uC54C \uC218 \uC5C6\uB294 System Message format : %d"), ID);
        GetDesktop()->AddSystemText(Error, (INT)0xFFB09B79, (DWORD)-1);
    }
    else
    {
        FL2SystemMsgData Msg = GL2GameData.SystemMsg[ID];
        const TCHAR* Format = *Msg.Message;
        for (INT i = 0; i < Msg.Message.Len(); i++)
        {
            if (Format[i] == '$')
            {
                Last = Format[i + 2] - '1';
                if (Last < NumParams && SysMsgParams(Last).Text)
                {
                    appStrcpy(Text + Len, SysMsgParams(Last).Text);
                    Len += SysMsgParams(Last).Len;
                }
                i += 2;
            }
            else if (Format[i] == '%')
            {
                // without the parameter's text: i and wa stay, eul and eun take their other form
                NSysMsgParam& Param = SysMsgParams(Last);
                TCHAR Particle = Format[i + 1];
                TCHAR Form;
                if (Param.Text)
                    Form = ParticleForm(Particle, HasNoFinalConsonant(Param.Text[Param.Len - 1]));
                else
                    Form = (Particle == 0xC774 || Particle == 0xC640) ? Particle : ParticleForm(Particle, 1);
                if (Form)
                    Text[Len++] = Form;
                i++;
            }
            else
                Text[Len++] = Format[i];
        }
        Text[Len] = 0;
        if ((char)P2 == 0)
        {
            if (GL2GameData.SystemMsg[ID].Type == 4)
            {
                NDialogInfo Info;
                Info.ID = DIALOG_SYSTEM_MESSAGE;
                Info.bMainBox = 1;
                Info.bInput = 0;
                Info.Owner = NULL;
                Info.SetText(Text);
                ShowDialog(&Info);
                PlaySoundName(*GL2GameData.SystemMsg[ID].Message2);
            }
            else
            {
                GetDesktop()->AddSystemText(Text, GL2GameData.SystemMsg[ID].Type, GL2GameData.SystemMsg[ID].Unknown14);
                PlaySoundName(*GL2GameData.SystemMsg[ID].Message2);
            }
        }
        else if (P3)
            appStrcpy((TCHAR*)P3, Text);
    }
    for (INT i = 0; i < NumParams; i++)
    {
        NSysMsgParam& Param = SysMsgParams(i);
        if (Param.Text)
        {
            GMalloc->Free(Param.Text);
            Param.Text = NULL;
        }
        Param.Len = 0;
    }
    SysMsgParams.Empty();
}
NW_HOOK_METHOD(0x1006cf70, NConsoleWnd, EndSystemMessageParam)

/*-----------------------------------------------------------------------------
    The pet and the summon.
-----------------------------------------------------------------------------*/

// 0x10068240 - the window of the summon (1) or of the pet
void NConsoleWnd::ShowPetStatusWnd(INT Type)
{
    if (Type == 1)
    {
        GetDesktop()->SummonedWnd->ShowWindow();
        PlayInterfaceSound(SOUND_WindowOpen);
        GetDesktop()->SummonedWnd->SetFocus();
    }
    else
    {
        GetDesktop()->PetWnd->ShowWindow();
        PlayInterfaceSound(SOUND_WindowOpen);
        GetDesktop()->PetWnd->SetFocus();
    }
}
NW_HOOK_METHOD(0x10068240, NConsoleWnd, ShowPetStatusWnd)

// 0x100552a0 - its bars beside the party's: the summon's (1) or the pet's (C2: the window takes the focus too)
void NConsoleWnd::ShowPetPartyWnd(INT Type)
{
    if (Type == 1)
    {
        if (!GetDesktop()->SummonedStatusWnd->IsShowWindow())
        {
            GetDesktop()->SummonedStatusWnd->ShowWindow();
            GetDesktop()->SummonedStatusWnd->SetFocus();
        }
    }
    else
    {
        if (!GetDesktop()->PetStatusWnd->IsShowWindow())
        {
            GetDesktop()->PetStatusWnd->ShowWindow();
            GetDesktop()->PetStatusWnd->SetFocus();
        }
    }
}
NW_HOOK_METHOD(0x100552a0, NConsoleWnd, ShowPetPartyWnd)

// 0x100682b0 - (no handler) all the pet's and the summon's windows hidden, the summon's
// timer stopped
void NConsoleWnd::HideSummonWnd()
{
    GetDesktop()->PetWnd->HideWindow();
    GetDesktop()->SummonedWnd->HideWindow();
    GetDesktop()->PetStatusWnd->HideWindow();
    GetDesktop()->SummonedStatusWnd->HideWindow();
    NCSummonedStatusWnd* Summoned = GetDesktop()->SummonedStatusWnd;
    Summoned->TimeBar->Stop();
    Summoned->TimeBar->Reset();
    Summoned->bTimerStarted = 0;
    PlayInterfaceSound(SOUND_WindowClose);
}
NW_HOOK_METHOD(0x100682b0, NConsoleWnd, HideSummonWnd)

// 0x10068340 - the same, in another order
void NConsoleWnd::HidePetWnd()
{
    GetDesktop()->PetStatusWnd->HideWindow();
    GetDesktop()->SummonedStatusWnd->HideWindow();
    GetDesktop()->PetWnd->HideWindow();
    GetDesktop()->SummonedWnd->HideWindow();
    NCSummonedStatusWnd* Summoned = GetDesktop()->SummonedStatusWnd;
    Summoned->TimeBar->Stop();
    Summoned->TimeBar->Reset();
    Summoned->bTimerStarted = 0;
    PlayInterfaceSound(SOUND_WindowClose);
}
NW_HOOK_METHOD(0x10068340, NConsoleWnd, HidePetWnd)

// 0x10055330
void NConsoleWnd::ShowPetNameBtn(INT bShow)
{
    GetDesktop()->PetWnd->SetCanName(bShow);
}
NW_HOOK_METHOD(0x10055330, NConsoleWnd, ShowPetNameBtn)

// 0x10055350 (vslot 211) - the pet's (the type is not used)
void NConsoleWnd::ReceivePetStatusInfo(INT Type, L2ParamStack& Params)
{
    GetDesktop()->PetStatusWnd->SetInfo(Params);
}
NW_HOOK_METHOD(0x10055350, NConsoleWnd, ReceivePetStatusInfo)

// 0x10055370 - a new list of the pet's items follows (AddPetInventoryItem)
void NConsoleWnd::ReceivePetItemList()
{
    GetDesktop()->PetWnd->ClearInventory();
}
NW_HOOK_METHOD(0x10055370, NConsoleWnd, ReceivePetItemList)

// 0x1005ad00 - the summon's time left; the first time, its whole time
void NConsoleWnd::SetSummonRemainTime(INT Remaining, INT Total)
{
    NCSummonedStatusWnd* Wnd = GetDesktop()->SummonedStatusWnd;
    if (Wnd->bTimerStarted)
        Wnd->SetRemaining(Remaining);
    else
        Wnd->StartTimer(Total);
}
NW_HOOK_METHOD(0x1005ad00, NConsoleWnd, SetSummonRemainTime)

/*-----------------------------------------------------------------------------
    The siege, the minimap, a GM's views.
-----------------------------------------------------------------------------*/

// C2 CANDIDATE: the C2 function is not the same code as the C1 one; kept only while it matches the stock one (verify_data)
// 0x1005a730 - a castle's siege: its id, whether the player's clan owns it, the owner clan's
// id, name and leader, its alliance's id and name, the server's time and the siege's
void NConsoleWnd::ReceiveSiegeInfo(L2ParamStack& Params)
{
    NSiegeCastleInfo Info;
    Info.CastleID = (INT)Params.Top();
    Info.bIsOwner = (INT)Params.Top();
    Info.PledgeID = (INT)Params.Top();
    appStrcpy(Info.ClanName, (TCHAR*)Params.Top());
    appStrcpy(Info.LordName, (TCHAR*)Params.Top());
    Info.AllianceID = (INT)Params.Top();
    appStrcpy(Info.AllianceName, (TCHAR*)Params.Top());
    Info.CurrentTime = (INT)Params.Top();
    Info.SiegeTime = (INT)Params.Top();
    GetDesktop()->SiegeInfoWnd->ShowWindow();
    static_cast<NCSiegeInfoMainWnd*>(GetDesktop()->SiegeInfoWnd)->SetCastleInfo(&Info);
}
NW_HOOK_METHOD(0x1005a730, NConsoleWnd, ReceiveSiegeInfo)

// 0x1005a840 - a clan of the attacking side: its id, name and leader, crest, the time it
// applied, its alliance's id, name and leader
void NConsoleWnd::ReceiveSiegeAttackerList(L2ParamStack& Params)
{
    NSiegeClanInfo Info;
    Info.PledgeID = (INT)Params.Top();
    appStrcpy(Info.ClanName, (TCHAR*)Params.Top());
    appStrcpy(Info.LeaderName, (TCHAR*)Params.Top());
    Info.CrestID = (INT)Params.Top();
    Info.ApplyTime = (INT)Params.Top();
    Info.AllianceID = (INT)Params.Top();
    appStrcpy(Info.AllianceName, (TCHAR*)Params.Top());
    appStrcpy(Info.AllianceLeader, (TCHAR*)Params.Top());
    static_cast<NCSiegeInfoMainWnd*>(GetDesktop()->SiegeInfoWnd)->AddAttacker(&Info);
}
NW_HOOK_METHOD(0x1005a840, NConsoleWnd, ReceiveSiegeAttackerList)

// 0x1005a930 - a clan of the defending side: as an attacker's, with its status after the time
void NConsoleWnd::ReceiveSiegeDefenderList(L2ParamStack& Params)
{
    NSiegeClanInfo Info;
    Info.PledgeID = (INT)Params.Top();
    appStrcpy(Info.ClanName, (TCHAR*)Params.Top());
    appStrcpy(Info.LeaderName, (TCHAR*)Params.Top());
    Info.CrestID = (INT)Params.Top();
    Info.ApplyTime = (INT)Params.Top();
    Info.Type = (INT)Params.Top();
    Info.AllianceID = (INT)Params.Top();
    appStrcpy(Info.AllianceName, (TCHAR*)Params.Top());
    appStrcpy(Info.AllianceLeader, (TCHAR*)Params.Top());
    static_cast<NCSiegeInfoMainWnd*>(GetDesktop()->SiegeInfoWnd)->AddDefender(&Info);
}
NW_HOOK_METHOD(0x1005a930, NConsoleWnd, ReceiveSiegeDefenderList)

// 0x1005aa30 (vslot 236) - a date the castle's lord can choose
void NConsoleWnd::AddSelectableTime(INT Time)
{
    static_cast<NCSiegeInfoMainWnd*>(GetDesktop()->SiegeInfoWnd)->DateInfo->AddTime(Time);
}
NW_HOOK_METHOD(0x1005aa30, NConsoleWnd, AddSelectableTime)

// 0x1005aa50
void NConsoleWnd::EndSiegeAttackerList()
{
    static_cast<NCSiegeInfoMainWnd*>(GetDesktop()->SiegeInfoWnd)->AttackerInfo->UpdateButtons();
}
NW_HOOK_METHOD(0x1005aa50, NConsoleWnd, EndSiegeAttackerList)

// 0x1005aa70
void NConsoleWnd::EndSiegeDefenderList()
{
    static_cast<NCSiegeInfoMainWnd*>(GetDesktop()->SiegeInfoWnd)->DefenderInfo->UpdateButtons();
}
NW_HOOK_METHOD(0x1005aa70, NConsoleWnd, EndSiegeDefenderList)

// 0x10054d30
void NConsoleWnd::ClearAttackerList()
{
    static_cast<NCSiegeInfoMainWnd*>(GetDesktop()->SiegeInfoWnd)->AttackerInfo->ClearList();
}
NW_HOOK_METHOD(0x10054d30, NConsoleWnd, ClearAttackerList)

// 0x10054d50
void NConsoleWnd::ClearDefenderList()
{
    static_cast<NCSiegeInfoMainWnd*>(GetDesktop()->SiegeInfoWnd)->DefenderInfo->ClearList();
}
NW_HOOK_METHOD(0x10054d50, NConsoleWnd, ClearDefenderList)

// 0x100557c0 - a map item was used: the minimap opens on its map, at the player
void NConsoleWnd::ShowMapItem(INT ItemID)
{
    GetDesktop()->MinimapWnd->SetMapID(ItemID);
    GetDesktop()->MinimapWnd->CenterOnPlayer();
    GetDesktop()->ToggleMinimap();
}
NW_HOOK_METHOD(0x100557c0, NConsoleWnd, ShowMapItem)

// C2 CANDIDATE: the C2 function is not the same code as the C1 one; kept only while it matches the stock one (verify_data)
// 0x10055480 - the character a GM looks at
void NConsoleWnd::GMViewCharacterInfo(User& Info)
{
    GetDesktop()->GMDetailStatusWnd->SetTarget(Info);
}
NW_HOOK_METHOD(0x10055480, NConsoleWnd, GMViewCharacterInfo)

// 0x1005ad30 - the clan of the character a GM looks at
void NConsoleWnd::GMViewReceivePledgeInfo(PledgeInfo& Info)
{
    guard(NConsoleWnd::GMViewReceivePledgeInfo);
    NPledgeInfo Pledge;
    MakePledgeInfo(Pledge, Info);
    GetDesktop()->GMActionWnd->PledgeWnd->SetPledgeInfo(&Pledge);
    unguard;
}
NW_HOOK_METHOD(0x1005ad30, NConsoleWnd, GMViewReceivePledgeInfo)

// 0x100554b0 - its clan's member list follows (GMViewAddPledgeMember)
void NConsoleWnd::GMViewReceivePledgeMemberList()
{
    guard(NConsoleWnd::ReceivePledgeMemberList);
    GetDesktop()->GMActionWnd->PledgeWnd->ClearMembers();
    unguard;
}
NW_HOOK_METHOD(0x100554b0, NConsoleWnd, GMViewReceivePledgeMemberList)

// 0x1005af80 (vslot 248) - as AddPledgeMember, into the GM's clan window
void NConsoleWnd::GMViewAddPledgeMember(L2ParamStack& Params)
{
    guard(NConsoleWnd::AddPledgeMember);
    NPledgeMember Member;
    TCHAR* Name = (TCHAR*)Params.Top();
    if (Name)
        appStrcpy(Member.Name, Name);
    Member.Level = (INT)Params.Top();
    Member.ClassID = (INT)Params.Top();
    Member.Unknown208 = (INT)Params.Top();
    Member.Unknown20C = (INT)Params.Top();
    Member.bOnline = (INT)Params.Top();
    GetDesktop()->GMActionWnd->PledgeWnd->AddMember(&Member);
    unguard;
}
NW_HOOK_METHOD(0x1005af80, NConsoleWnd, GMViewAddPledgeMember)

// 0x10055540 - its skill list follows (GMViewSkillList)
void NConsoleWnd::GMViewShowSkillWnd()
{
    GetDesktop()->GMMagicSkillWnd->SkillWnd->ClearLists();
}
NW_HOOK_METHOD(0x10055540, NConsoleWnd, GMViewShowSkillWnd)

// 0x1005b070 - as ReceiveSkillList, into the GM's skill window
void NConsoleWnd::GMViewSkillList(L2ParamStack& Params)
{
    NItemInfo Info;
    Info.Param = (INT)Params.Top();
    Info.UnknownC20 = (INT)Params.Top();
    Info.ID = (INT)Params.Top();
    if (Info.UnknownC20 > 0)
    {
        SetSkillTexts(this, Info);
        GetDesktop()->GMMagicSkillWnd->SkillWnd->AddSkill(&Info);
    }
}
NW_HOOK_METHOD(0x1005b070, NConsoleWnd, GMViewSkillList)

// 0x1005b1b0 - a magic of the list: its level and id
void NConsoleWnd::GMViewMagicList(L2ParamStack& Params)
{
    NItemInfo Info;
    Info.UnknownC20 = (INT)Params.Top();
    Info.ID = (INT)Params.Top();
    if (Info.UnknownC20 > 0)
    {
        SetSkillTexts(this, Info);
        GetDesktop()->GMMagicSkillWnd->SkillWnd->AddSkill(&Info);
    }
}
NW_HOOK_METHOD(0x1005b1b0, NConsoleWnd, GMViewMagicList)

// 0x10055560 - its quest list follows (GMViewAddQuestID)
void NConsoleWnd::GMViewReceiveQuestList()
{
    guard(NConsoleWnd::ReceiveQuestList);
    GetDesktop()->GMQuestWnd->Clear();
    unguard;
}
NW_HOOK_METHOD(0x10055560, NConsoleWnd, GMViewReceiveQuestList)

// 0x100555e0 - as AddQuestID (C2: the line need not be of the step either; C1 asked for the step too)
void NConsoleWnd::GMViewAddQuestID(INT QuestID, INT Step)
{
    guard(NConsoleWnd::AddQuestID);
    for (INT i = 0; i < 1000; i++)
    {
        NQuestNameLine& Data = GQuestNames[i];
        if (Data.bValid && Data.QuestID == QuestID)
        {
            GetDesktop()->GMQuestWnd->AddQuest(QuestID, *Data.Name, Step);
            break;
        }
    }
    unguard;
}
NW_HOOK_METHOD(0x100555e0, NConsoleWnd, GMViewAddQuestID)
