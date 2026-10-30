/*=============================================================================
    NConsoleWndNet.cpp - the console's requests to the server, and what they act on (the
    player, its target, what the network handler knows). Almost all of them only pass their
    arguments on to the engine's network handler (Engine->NetHandler, a UL2NetHandler whose 228
    virtuals are pure in Engine.dll and have no names yet: the slot is given at each call).
    Within a family the slots follow the protocol's packet order (e.g. 195..201 are the private
    store packets 0x73..0x79), which names the requests no chat command names.
=============================================================================*/
#include "nwindow/NConsoleWnd.h"
#include "nwindow/NCConsole.h"
#include "nwindow/NCChatWnd.h"
#include "nwindow/NCLobbyWnd.h"
#include "nwindow/NCLoginWnd.h"
#include "nwindow/NCLoadingWnd.h"
#include "nwindow/NCMessageWnd.h"
#include "nwindow/NCRestartMenuWnd.h"
#include "nwindow/NCItemWnd.h"

// interface sounds (FL2NetNotify::OnInterfacePlaySound; names ours)
enum { SOUND_UseShortcut = 1, SOUND_PickUp = 3 };

// the pawn the player's controller drives, and the actor under the mouse
static FORCEINLINE APawn* PlayerPawn(UEngine* Engine)
{
    return Engine->Client->Viewports(0)->Actor->Pawn;
}
static FORCEINLINE AActor* HoveredActor(UEngine* Engine)
{
    return static_cast<UGameEngine*>(Engine)->MouseHitActor;
}

// (GetMyLocation, 0x10057dd0 in C2, is in NConsoleWndMisc.cpp)

// 0x10051f90 - a chat line (NetHandler slot 146: type, text, whisper target); with
// GIsOnlyL2Console (no server) it is only shown in the chat window. Returns 0.
UBOOL NConsoleWnd::SendChatMsgToServer(INT Type, TCHAR* Text, TCHAR* Target)
{
    guard(NConsoleWnd::SendChatMsgToServer);
    UL2NetHandler* NetHandler = Engine->NetHandler;
    if (GIsOnlyL2Console)
        static_cast<NCConsole*>(Desktop)->ChatWnd->AddChatMessage(Text, 0xFFFFFFFF, 0, 0);
    else
    {
        L2ParamStack Params(10);
        Params.PushBack((void*)(INT_PTR)Type);
        Params.PushBack(Text);
        Params.PushBack(Target);
        NetHandler->Say2(Params);
    }
    return 0;
    unguard;
}
NW_HOOK_METHOD(0x10051f90, NConsoleWnd, SendChatMsgToServer)

// 0x10052080 - NetHandler slot 68
UBOOL NConsoleWnd::RequestAuthLogin(const TCHAR* Account, const TCHAR* Password)
{
    guard(NConsoleWnd::RequestAuthLogin);
    return Engine->NetHandler->RequestAuthLogin((TCHAR*)Account, (TCHAR*)Password) != 0;
    unguard;
}
NW_HOOK_METHOD(0x10052080, NConsoleWnd, RequestAuthLogin)

// 0x100521a0 - NetHandler slot 147
void NConsoleWnd::RequestLogout()
{
    Engine->NetHandler->SendLogOutPacket();
}
NW_HOOK_METHOD(0x100521a0, NConsoleWnd, RequestLogout)

// 0x100521b0 - NetHandler slot 71; when sent, the loading screen replaces the character
// selection until the server answers (AcceptGameStart)
UBOOL NConsoleWnd::RequestGameStart(INT Index)
{
    guard(NConsoleWnd::RequestGameStart);
    if (!Engine->NetHandler->RequestGameStart(Index))
        return 0;
    LobbyWnd->HideWindow();
    MessageWnd->HideWindow();
    LoadingWnd->SetLoadingTexture(LoadingScreenID);
    LoadingWnd->ShowWindow();
    SetState(L2CS_Waiting);
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x100521b0, NConsoleWnd, RequestGameStart)

// 0x100522a0 - NetHandler slot 72
UBOOL NConsoleWnd::RequestCharacterDelete(INT Index)
{
    guard(NConsoleWnd::RequestCharacterDelete);
    if (!Engine->NetHandler->RequestCharacterDelete(Index))
        return 0;
    SetState(L2CS_Waiting);
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x100522a0, NConsoleWnd, RequestCharacterDelete)

// 0x10052350 - NetHandler slot 73
UBOOL NConsoleWnd::RequestCharacterRestore(INT Index)
{
    guard(NConsoleWnd::RequestCharacterRestore);
    if (!Engine->NetHandler->RequestCharacterRestore(Index))
        return 0;
    SetState(L2CS_Waiting);
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x10052350, NConsoleWnd, RequestCharacterRestore)

// 0x10052400 - NetHandler slot 74
void NConsoleWnd::RequestNewCharacter()
{
    guard(NConsoleWnd::RequestNewCharacter);
    Engine->NetHandler->RequestNewCharacter();
    unguard;
}
NW_HOOK_METHOD(0x10052400, NConsoleWnd, RequestNewCharacter)

// 0x10052480 - NetHandler slot 75 (the creation's choices, by value: C2's record is 0x60 bytes, ret 0x60)
UBOOL NConsoleWnd::RequestCharacterCreate(CharacterInfo Info)
{
    guard(NConsoleWnd::RequestCharacterCreate);
    if (!Engine->NetHandler->RequestCharacterCreate(Info))
        return 0;
    SetState(L2CS_Waiting);
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x10052480, NConsoleWnd, RequestCharacterCreate)

// 0x10052540 - NetHandler slot 159
void NConsoleWnd::RequestRestart(L2ParamStack& Params)
{
    Engine->NetHandler->RequestRestart(Params);
}
NW_HOOK_METHOD(0x10052540, NConsoleWnd, RequestRestart)

// 0x10052550 - NetHandler slot 160
void NConsoleWnd::RequestRestartPoint(L2ParamStack& Params)
{
    Engine->NetHandler->RequestRestartPoint(Params);
}
NW_HOOK_METHOD(0x10052550, NConsoleWnd, RequestRestartPoint)

// 0x100526c0 - NetHandler slot 70
void NConsoleWnd::RequestServerLogin(L2ParamStack& Params)
{
    guard(NConsoleWnd::RequestServerLogin);
    Engine->NetHandler->RequestServerLogin(Params);
    unguard;
}
NW_HOOK_METHOD(0x100526c0, NConsoleWnd, RequestServerLogin)

// 0x100533c0 - NetHandler slot 77
void NConsoleWnd::RequestItemList()
{
    guard(NConsoleWnd::RequestItemList);
    Engine->NetHandler->RequestItemList();
    unguard;
}
NW_HOOK_METHOD(0x100533c0, NConsoleWnd, RequestItemList)

// 0x10053440 - NetHandler slot 86
void NConsoleWnd::RequestUseItem(INT ObjectID)
{
    guard(NConsoleWnd::RequestUseItem);
    Engine->NetHandler->RequestUseItem(ObjectID);
    unguard;
}
NW_HOOK_METHOD(0x10053440, NConsoleWnd, RequestUseItem)

// 0x100534c0 - NetHandler slot 153
void NConsoleWnd::RequestUnEquipItem(L2ParamStack& Params)
{
    Engine->NetHandler->RequestUnEquipItem(Params);
}
NW_HOOK_METHOD(0x100534c0, NConsoleWnd, RequestUnEquipItem)

// 0x100537e0 - NetHandler slot 82
void NConsoleWnd::RequestTrade(INT CreatureID)
{
    guard(NConsoleWnd::RequestTrade);
    Engine->NetHandler->RequestTrade(CreatureID);
    unguard;
}
NW_HOOK_METHOD(0x100537e0, NConsoleWnd, RequestTrade)

// 0x10053860 - NetHandler slot 83 (by value)
void NConsoleWnd::RequestAddTradeItem(NItemRequest Request)
{
    guard(NConsoleWnd::RequestAddTradeItem);
    Engine->NetHandler->RequestAddTradeItem(Request);
    unguard;
}
NW_HOOK_METHOD(0x10053860, NConsoleWnd, RequestAddTradeItem)

// 0x100538f0 - NetHandler slot 84
void NConsoleWnd::RequestTradeDone(INT bConfirm)
{
    guard(NConsoleWnd::RequestTradeDone);
    Engine->NetHandler->RequestTradeDone(bConfirm);
    unguard;
}
NW_HOOK_METHOD(0x100538f0, NConsoleWnd, RequestTradeDone)

// 0x10053a20 - NetHandler slot 90 (C2 only: the manor's seed purchase; the fmap paired the C1 RequestBuyItem with it)
void NConsoleWnd::RequestBuySeed(L2ParamStack& Params)
{
    guard(NConsoleWnd::RequestBuySeed);
    Engine->NetHandler->RequestBuySeed(Params);
    unguard;
}
NW_HOOK_METHOD(0x10053a20, NConsoleWnd, RequestBuySeed)

// 0x10053aa0 - NetHandler slot 91 (C2 only: the manor's crop procurement)
void NConsoleWnd::RequestProcureCrop(L2ParamStack& Params)
{
    guard(NConsoleWnd::RequestProcureCrop);
    Engine->NetHandler->RequestProcureCrop(Params);
    unguard;
}
NW_HOOK_METHOD(0x10053aa0, NConsoleWnd, RequestProcureCrop)

// 0x10053c50 - NetHandler slot 89
void NConsoleWnd::RequestBuyItem(L2ParamStack& Params)
{
    guard(NConsoleWnd::RequestBuyItem);
    Engine->NetHandler->RequestBuyItem(Params);
    unguard;
}
NW_HOOK_METHOD(0x10053c50, NConsoleWnd, RequestBuyItem)

// 0x10053cd0 - NetHandler slot 88
void NConsoleWnd::RequestSellItem(L2ParamStack& Params)
{
    guard(NConsoleWnd::RequestSellItem);
    Engine->NetHandler->RequestSellItem(Params);
    unguard;
}
NW_HOOK_METHOD(0x10053cd0, NConsoleWnd, RequestSellItem)

// 0x10053e80 - NetHandler slot 138: the warehouse window's list in its deposit mode
void NConsoleWnd::RequestWarehouseDeposit(L2ParamStack& Params)
{
    Engine->NetHandler->SendWareHouseDepositList(Params);
}
NW_HOOK_METHOD(0x10053e80, NConsoleWnd, RequestWarehouseDeposit)

// 0x10053e90 - NetHandler slot 139: the same in its withdraw mode
void NConsoleWnd::RequestWarehouseWithdraw(L2ParamStack& Params)
{
    Engine->NetHandler->SendWareHouseWithdrawList(Params);
}
NW_HOOK_METHOD(0x10053e90, NConsoleWnd, RequestWarehouseWithdraw)

// 0x10054380 - NetHandler slot 145 (C2 only: macros; names ours, after the slot's)
void NConsoleWnd::RequestMakeMacro(L2ParamStack& Params)
{
    Engine->NetHandler->RequestMakeMacro(Params);
}
NW_HOOK_METHOD(0x10054380, NConsoleWnd, RequestMakeMacro)

// 0x10054390 - NetHandler slot 146 (C2 only: macros)
void NConsoleWnd::RequestDeleteMacro(L2ParamStack& Params)
{
    Engine->NetHandler->RequestDeleteMacro(Params);
}
NW_HOOK_METHOD(0x10054390, NConsoleWnd, RequestDeleteMacro)

// 0x100543a0 - NetHandler slot 149
void NConsoleWnd::RequestMagicSkillUse(L2ParamStack& Params)
{
    Engine->NetHandler->RequestMagicSkillUse(Params);
}
NW_HOOK_METHOD(0x100543a0, NConsoleWnd, RequestMagicSkillUse)

// 0x100543b0 - NetHandler slot 158
void NConsoleWnd::RequestSkillList(L2ParamStack& Params)
{
    Engine->NetHandler->RequestSkillItem(Params);
}
NW_HOOK_METHOD(0x100543b0, NConsoleWnd, RequestSkillList)

// 0x100543f0 - NetHandler slot 199 (C2 only; name ours, after the slot's)
void NConsoleWnd::RequestRecipeBookOpen()
{
    Engine->NetHandler->RequestRecipeBookOpen();
}
NW_HOOK_METHOD(0x100543f0, NConsoleWnd, RequestRecipeBookOpen)

// 0x10054400 - NetHandler slot 200 (C2 only; name ours, after the slot's)
void NConsoleWnd::RequestRecipeItemDelete(INT P1)
{
    Engine->NetHandler->RequestRecipeItemDelete(P1);
}
NW_HOOK_METHOD(0x10054400, NConsoleWnd, RequestRecipeItemDelete)

// 0x10054410 - NetHandler slot 201 (C2 only; name ours, after the slot's)
void NConsoleWnd::RequestRecipeItemMakeInfo(INT P1)
{
    Engine->NetHandler->RequestRecipeItemMakeInfo(P1);
}
NW_HOOK_METHOD(0x10054410, NConsoleWnd, RequestRecipeItemMakeInfo)

// 0x10054420 - NetHandler slot 202 (C2 only; name ours, after the slot's)
void NConsoleWnd::RequestRecipeItemMakeSelf(INT P1)
{
    Engine->NetHandler->RequestRecipeItemMakeSelf(P1);
}
NW_HOOK_METHOD(0x10054420, NConsoleWnd, RequestRecipeItemMakeSelf)

// 0x10054430 - NetHandler slot 203 (C2 only; name ours, after the slot's)
void NConsoleWnd::RequestRecipeShopManageList()
{
    Engine->NetHandler->RequestRecipeShopManageList();
}
NW_HOOK_METHOD(0x10054430, NConsoleWnd, RequestRecipeShopManageList)

// 0x10054440 - NetHandler slot 204 (C2 only; name ours, after the slot's)
void NConsoleWnd::RequestRecipeShopMessageSet(TCHAR* P1)
{
    Engine->NetHandler->RequestRecipeShopMessageSet(P1);
}
NW_HOOK_METHOD(0x10054440, NConsoleWnd, RequestRecipeShopMessageSet)

// 0x10054450 - NetHandler slot 205 (C2 only; name ours, after the slot's)
void NConsoleWnd::RequestRecipeShopListSet(L2ParamStack& P1)
{
    Engine->NetHandler->RequestRecipeShopListSet(P1);
}
NW_HOOK_METHOD(0x10054450, NConsoleWnd, RequestRecipeShopListSet)

// 0x10054460 - NetHandler slot 206 (C2 only; name ours, after the slot's)
void NConsoleWnd::RequestRecipeShopManageQuit()
{
    Engine->NetHandler->RequestRecipeShopManageQuit();
}
NW_HOOK_METHOD(0x10054460, NConsoleWnd, RequestRecipeShopManageQuit)

// 0x10054470 - NetHandler slot 207 (C2 only; name ours, after the slot's)
void NConsoleWnd::RequestRecipeShopManageCancel()
{
    Engine->NetHandler->RequestRecipeShopManageCancel();
}
NW_HOOK_METHOD(0x10054470, NConsoleWnd, RequestRecipeShopManageCancel)

// 0x10054480 - NetHandler slot 208 (C2 only; name ours, after the slot's)
void NConsoleWnd::RequestRecipeShopSellList(INT P1)
{
    Engine->NetHandler->RequestRecipeShopSellList(P1);
}
NW_HOOK_METHOD(0x10054480, NConsoleWnd, RequestRecipeShopSellList)

// 0x10054490 - NetHandler slot 209 (C2 only; name ours, after the slot's)
void NConsoleWnd::RequestRecipeShopMakeInfo(INT P1, INT P2)
{
    Engine->NetHandler->RequestRecipeShopMakeInfo(P1, P2);
}
NW_HOOK_METHOD(0x10054490, NConsoleWnd, RequestRecipeShopMakeInfo)

// 0x100544a0 - NetHandler slot 210 (C2 only; name ours, after the slot's)
void NConsoleWnd::RequestRecipeShopMakeDo(INT P1, INT P2)
{
    Engine->NetHandler->RequestRecipeShopMakeDo(P1, P2);
}
NW_HOOK_METHOD(0x100544a0, NConsoleWnd, RequestRecipeShopMakeDo)

// 0x100544f0 - NetHandler slot 211 (C2 only; name ours, after the slot's)
void NConsoleWnd::RequestHennaItemInfo(INT P1)
{
    Engine->NetHandler->RequestHennaItemInfo(P1);
}
NW_HOOK_METHOD(0x100544f0, NConsoleWnd, RequestHennaItemInfo)

// 0x10054500 - NetHandler slot 212 (C2 only; name ours, after the slot's)
void NConsoleWnd::RequestHennaEquip(INT P1)
{
    Engine->NetHandler->RequestHennaEquip(P1);
}
NW_HOOK_METHOD(0x10054500, NConsoleWnd, RequestHennaEquip)

// 0x10054510 - NetHandler slot 213 (C2 only; name ours, after the slot's)
void NConsoleWnd::RequestHennaUnequipInfo(INT P1)
{
    Engine->NetHandler->RequestHennaUnequipInfo(P1);
}
NW_HOOK_METHOD(0x10054510, NConsoleWnd, RequestHennaUnequipInfo)

// 0x10054520 - NetHandler slot 214 (C2 only; name ours, after the slot's)
void NConsoleWnd::RequestHennaUnequip(INT P1)
{
    Engine->NetHandler->RequestHennaUnequip(P1);
}
NW_HOOK_METHOD(0x10054520, NConsoleWnd, RequestHennaUnequip)

// 0x10067080 - NetHandler slot 142, then the pickup sound (the icon was dropped on the bar)
void NConsoleWnd::RequestShortCutReg(L2ParamStack& Params)
{
    Engine->NetHandler->RequestShortCutReg(Params);
    PlayInterfaceSound(SOUND_PickUp);
}
NW_HOOK_METHOD(0x10067080, NConsoleWnd, RequestShortCutReg)

// 0x10044860  NOT PORTED (no C2 counterpart)
void NConsoleWnd::UseShortcut(L2ParamStack& Params)
{
    NW_UNPORTED_NORET();
}

// 0x100545e0 - NetHandler slot 142
void NConsoleWnd::RequestShortCutDel(L2ParamStack& Params)
{
    Engine->NetHandler->RequestShortCutDel(Params);
}
NW_HOOK_METHOD(0x100545e0, NConsoleWnd, RequestShortCutDel)

// 0x100546b0 - NetHandler slot 143
void NConsoleWnd::RequestTargetCancel(L2ParamStack& Params)
{
    Engine->NetHandler->RequestTargetCancel(Params);
}
NW_HOOK_METHOD(0x100546b0, NConsoleWnd, RequestTargetCancel)

// 0x100546c0 - NetHandler slot 157
void NConsoleWnd::RequestActionUse(L2ParamStack& Params)
{
    Engine->NetHandler->RequestActionUse(Params);
}
NW_HOOK_METHOD(0x100546c0, NConsoleWnd, RequestActionUse)

// 0x100546d0 - NetHandler slot 52 (target, origin, stand still): 0
void NConsoleWnd::RequestAttackForce(INT ObjectID, FVector Origin)
{
    Engine->NetHandler->Attack(ObjectID, Origin, 0);
}
NW_HOOK_METHOD(0x100546d0, NConsoleWnd, RequestAttackForce)

// 0x10054710 - the same, standing still
void NConsoleWnd::RequestAttackStand(INT ObjectID, FVector Origin)
{
    Engine->NetHandler->Attack(ObjectID, Origin, 1);
}
NW_HOOK_METHOD(0x10054710, NConsoleWnd, RequestAttackStand)

// 0x10054750 - NetHandler slot 57
void NConsoleWnd::RequestSocialAction(INT Action)
{
    Engine->NetHandler->SocialAction(Action);
}
NW_HOOK_METHOD(0x10054750, NConsoleWnd, RequestSocialAction)

// 0x10054760 - NetHandler slot 59
void NConsoleWnd::RequestChangeWaitType(UBOOL bStand)
{
    Engine->NetHandler->ChangeWaitType(bStand);
}
NW_HOOK_METHOD(0x10054760, NConsoleWnd, RequestChangeWaitType)

// 0x10054770 - NetHandler slot 58
void NConsoleWnd::RequestChangeMoveType(UBOOL bRun)
{
    Engine->NetHandler->ChangeMoveType(bRun);
}
NW_HOOK_METHOD(0x10054770, NConsoleWnd, RequestChangeMoveType)

// 0x100547e0 - NetHandler slot 102
void NConsoleWnd::RequestPledgeMemberList()
{
    Engine->NetHandler->RequestPledgeMemberList();
}
NW_HOOK_METHOD(0x100547e0, NConsoleWnd, RequestPledgeMemberList)

// 0x100547f0 - NetHandler slot 103
void NConsoleWnd::RequestNet103(L2ParamStack& Params)
{
    Engine->NetHandler->RequestCreatePledge(Params);
}
NW_HOOK_METHOD(0x100547f0, NConsoleWnd, RequestNet103)

// 0x10054800 - NetHandler slot 104
void NConsoleWnd::RequestAnswerJoinPledge(L2ParamStack& Params)
{
    Engine->NetHandler->RequestAnswerJoinPledge(Params);
}
NW_HOOK_METHOD(0x10054800, NConsoleWnd, RequestAnswerJoinPledge)

// 0x10054810 - NetHandler slot 105
void NConsoleWnd::RequestOustPledgeMember(L2ParamStack& Params)
{
    Engine->NetHandler->RequestOustPledgeMember(Params);
}
NW_HOOK_METHOD(0x10054810, NConsoleWnd, RequestOustPledgeMember)

// 0x10054820 - NetHandler slot 106
void NConsoleWnd::RequestNet106()
{
    Engine->NetHandler->RequestDismissPledge();
}
NW_HOOK_METHOD(0x10054820, NConsoleWnd, RequestNet106)

// 0x10054830 - NetHandler slot 109
void NConsoleWnd::RequestStartPledgeWar(L2ParamStack& Params)
{
    Engine->NetHandler->RequestStartPledgeWar(Params);
}
NW_HOOK_METHOD(0x10054830, NConsoleWnd, RequestStartPledgeWar)

// 0x10054840 - NetHandler slot 110
void NConsoleWnd::RequestReplyStartPledgeWar(L2ParamStack& Params)
{
    Engine->NetHandler->RequestReplyStartPledgeWar(Params);
}
NW_HOOK_METHOD(0x10054840, NConsoleWnd, RequestReplyStartPledgeWar)

// 0x10054850 - NetHandler slot 111
void NConsoleWnd::RequestStopPledgeWar(L2ParamStack& Params)
{
    Engine->NetHandler->RequestStopPledgeWar(Params);
}
NW_HOOK_METHOD(0x10054850, NConsoleWnd, RequestStopPledgeWar)

// 0x10054860 - NetHandler slot 112
void NConsoleWnd::RequestReplyStopPledgeWar(L2ParamStack& Params)
{
    Engine->NetHandler->RequestReplyStopPledgeWar(Params);
}
NW_HOOK_METHOD(0x10054860, NConsoleWnd, RequestReplyStopPledgeWar)

// 0x10054870 - NetHandler slot 113
void NConsoleWnd::RequestSurrenderPledgeWar(L2ParamStack& Params)
{
    Engine->NetHandler->RequestSurrenderPledgeWar(Params);
}
NW_HOOK_METHOD(0x10054870, NConsoleWnd, RequestSurrenderPledgeWar)

// 0x10054880 - NetHandler slot 115
void NConsoleWnd::RequestSurrenderPersonally(L2ParamStack& Params)
{
    Engine->NetHandler->RequestSurrenderPersonally(Params);
}
NW_HOOK_METHOD(0x10054880, NConsoleWnd, RequestSurrenderPersonally)

// 0x10054890 - NetHandler slot 116
void NConsoleWnd::RequestGiveNickName(L2ParamStack& Params)
{
    Engine->NetHandler->RequestGiveNickName(Params);
}
NW_HOOK_METHOD(0x10054890, NConsoleWnd, RequestGiveNickName)

// 0x100548a0 - NetHandler slot 107
void NConsoleWnd::RequestJoinPledge(L2ParamStack& Params)
{
    Engine->NetHandler->RequestJoinPledge(Params);
}
NW_HOOK_METHOD(0x100548a0, NConsoleWnd, RequestJoinPledge)

// 0x100548b0 - NetHandler slot 108
void NConsoleWnd::RequestWithdrawalPledge()
{
    Engine->NetHandler->RequestWithDrawalPledge();
}
NW_HOOK_METHOD(0x100548b0, NConsoleWnd, RequestWithdrawalPledge)

// 0x10054aa0 - NetHandler slot 119
void NConsoleWnd::RequestAllyInvite(L2ParamStack& Params)
{
    Engine->NetHandler->RequestJoinAlly(Params);
}
NW_HOOK_METHOD(0x10054aa0, NConsoleWnd, RequestAllyInvite)

// 0x10054ab0 - NetHandler slot 120
void NConsoleWnd::RequestAnswerJoinAlly(L2ParamStack& Params)
{
    Engine->NetHandler->RequestAnswerJoinAlly(Params);
}
NW_HOOK_METHOD(0x10054ab0, NConsoleWnd, RequestAnswerJoinAlly)

// 0x10054ac0 - NetHandler slot 121
void NConsoleWnd::RequestOustAlly(L2ParamStack& Params)
{
    Engine->NetHandler->RequestOustAlly(Params);
}
NW_HOOK_METHOD(0x10054ac0, NConsoleWnd, RequestOustAlly)

// 0x10054ad0 - NetHandler slot 124
void NConsoleWnd::RequestAllyInfo()
{
    Engine->NetHandler->RequestAllyInfo();
}
NW_HOOK_METHOD(0x10054ad0, NConsoleWnd, RequestAllyInfo)

// 0x10054ae0 - NetHandler slot 122
void NConsoleWnd::RequestWithdrawAlly()
{
    Engine->NetHandler->RequestWithdrawAlly();
}
NW_HOOK_METHOD(0x10054ae0, NConsoleWnd, RequestWithdrawAlly)

// 0x10054af0 - NetHandler slot 123
void NConsoleWnd::RequestDismissAlly()
{
    Engine->NetHandler->RequestDismissAlly();
}
NW_HOOK_METHOD(0x10054af0, NConsoleWnd, RequestDismissAlly)

// 0x10054c80 - NetHandler slot 125
void NConsoleWnd::RequestFriendInvite(L2ParamStack& Params)
{
    Engine->NetHandler->RequestFriendInvite(Params);
}
NW_HOOK_METHOD(0x10054c80, NConsoleWnd, RequestFriendInvite)

// 0x10054c90 - NetHandler slot 126
void NConsoleWnd::RequestFriendDel(L2ParamStack& Params)
{
    Engine->NetHandler->RequestFriendDel(Params);
}
NW_HOOK_METHOD(0x10054c90, NConsoleWnd, RequestFriendDel)

// 0x10054ca0 - NetHandler slot 128
void NConsoleWnd::RequestAnswerFriendInvite(L2ParamStack& Params)
{
    Engine->NetHandler->RequestFriendAddReply(Params);
}
NW_HOOK_METHOD(0x10054ca0, NConsoleWnd, RequestAnswerFriendInvite)

// 0x10054cb0 - NetHandler slot 127
void NConsoleWnd::RequestFriendList()
{
    Engine->NetHandler->RequestFriendList();
}
NW_HOOK_METHOD(0x10054cb0, NConsoleWnd, RequestFriendList)

// 0x10054cc0 - NetHandler slot 129
void NConsoleWnd::RequestBlock(L2ParamStack& Params)
{
    Engine->NetHandler->RequestBlock(Params);
}
NW_HOOK_METHOD(0x10054cc0, NConsoleWnd, RequestBlock)

// 0x10054d70 - NetHandler slot 130
void NConsoleWnd::RequestPartyInvite(L2ParamStack& Params)
{
    Engine->NetHandler->RequestJoinParty(Params);
}
NW_HOOK_METHOD(0x10054d70, NConsoleWnd, RequestPartyInvite)

// 0x10054d80 - NetHandler slot 131
void NConsoleWnd::RequestAnswerJoinParty(L2ParamStack& Params)
{
    Engine->NetHandler->RequestAnswerJoinParty(Params);
}
NW_HOOK_METHOD(0x10054d80, NConsoleWnd, RequestAnswerJoinParty)

// 0x10054d90 - NetHandler slot 132
void NConsoleWnd::RequestPartyLeave(L2ParamStack& Params)
{
    Engine->NetHandler->RequestWithDrawalParty(Params);
}
NW_HOOK_METHOD(0x10054d90, NConsoleWnd, RequestPartyLeave)

// 0x10054da0 - NetHandler slot 133
void NConsoleWnd::RequestPartyDismiss(L2ParamStack& Params)
{
    Engine->NetHandler->RequestOustPartyMember(Params);
}
NW_HOOK_METHOD(0x10054da0, NConsoleWnd, RequestPartyDismiss)

// 0x10054db0 - NetHandler slot 135
void NConsoleWnd::RequestPartyMatchConfig(L2ParamStack& Params)
{
    Engine->NetHandler->RequestPartyMatchConfig(Params);
}
NW_HOOK_METHOD(0x10054db0, NConsoleWnd, RequestPartyMatchConfig)

// 0x10054dc0 - NetHandler slot 136
void NConsoleWnd::RequestPartyMatchList(INT Status)
{
    Engine->NetHandler->RequestPartyMatchList(Status);
}
NW_HOOK_METHOD(0x10054dc0, NConsoleWnd, RequestPartyMatchList)

// 0x10054dd0 - NetHandler slot 137
void NConsoleWnd::RequestPartyMatchDetail(INT ObjectID)
{
    Engine->NetHandler->RequestPartyMatchDetail(ObjectID);
}
NW_HOOK_METHOD(0x10054dd0, NConsoleWnd, RequestPartyMatchDetail)

// 0x10055050 - NetHandler slot 91
void NConsoleWnd::RequestLinkHtml(L2ParamStack& Params)
{
    Engine->NetHandler->RequestLinkHtml(Params);
}
NW_HOOK_METHOD(0x10055050, NConsoleWnd, RequestLinkHtml)

// 0x10055060 - NetHandler slot 93
void NConsoleWnd::RequestBypassToServer(L2ParamStack& Params)
{
    Engine->NetHandler->RequestBypassToServer(Params);
}
NW_HOOK_METHOD(0x10055060, NConsoleWnd, RequestBypassToServer)

// 0x10055070 - NetHandler slot 92
void NConsoleWnd::RequestBBSWrite(L2ParamStack& Params)
{
    Engine->NetHandler->RequestBBSWrite(Params);
}
NW_HOOK_METHOD(0x10055070, NConsoleWnd, RequestBBSWrite)

// 0x10055080 - NetHandler slot 163
void NConsoleWnd::RequestShowBoard(L2ParamStack& Params)
{
    Engine->NetHandler->RequestShowboard(Params);
}
NW_HOOK_METHOD(0x10055080, NConsoleWnd, RequestShowBoard)

// 0x10055190 - NetHandler slot 94
void NConsoleWnd::RequestTutorialLinkHtml(L2ParamStack& Params)
{
    Engine->NetHandler->RequestTutorialLinkHtml(Params);
}
NW_HOOK_METHOD(0x10055190, NConsoleWnd, RequestTutorialLinkHtml)

// 0x100551a0 - NetHandler slot 95
void NConsoleWnd::RequestTutorialPassCmdToServer(L2ParamStack& Params)
{
    Engine->NetHandler->RequestTutorialPassCmdToServer(Params);
}
NW_HOOK_METHOD(0x100551a0, NConsoleWnd, RequestTutorialPassCmdToServer)

// 0x100551b0 - NetHandler slot 96
void NConsoleWnd::RequestTutorialQuestionMark(INT ID)
{
    Engine->NetHandler->RequestTutorialQuestionMarkPressed(ID);
}
NW_HOOK_METHOD(0x100551b0, NConsoleWnd, RequestTutorialQuestionMark)

// 0x10055220 - the item nearest to the pet within Range (NetHandler slot 210, a float) is
// picked up by it (slot 189 with the item's object id, its first field). The stock also tests
// the address of the item's field +0x10 for NULL, which it never is.
void NConsoleWnd::PetPickUp(INT Range, INT Param)
{
    Item* Found = Engine->NetHandler->GetPetNearestItem((FLOAT)Range, Param);
    if (Found)
        Engine->NetHandler->RequestPetGetItem(Found->ObjectID);
}
NW_HOOK_METHOD(0x10055220, NConsoleWnd, PetPickUp)

// 0x10055260 - NetHandler slot 190
void NConsoleWnd::RequestPetUseItem(INT ObjectID)
{
    Engine->NetHandler->RequestPetUseItem(ObjectID);
}
NW_HOOK_METHOD(0x10055260, NConsoleWnd, RequestPetUseItem)

// 0x10055270 - NetHandler slot 191
void NConsoleWnd::RequestGiveItemToPet(L2ParamStack& Params)
{
    Engine->NetHandler->RequestGiveItemToPet(Params);
}
NW_HOOK_METHOD(0x10055270, NConsoleWnd, RequestGiveItemToPet)

// 0x10055280 - NetHandler slot 192
void NConsoleWnd::RequestGetItemFromPet(L2ParamStack& Params)
{
    Engine->NetHandler->RequestGetItemFromPet(Params);
}
NW_HOOK_METHOD(0x10055280, NConsoleWnd, RequestGetItemFromPet)

// 0x10055290 - NetHandler slot 194
void NConsoleWnd::RequestChangePetName(TCHAR* Name)
{
    Engine->NetHandler->RequestChangePetName(Name);
}
NW_HOOK_METHOD(0x10055290, NConsoleWnd, RequestChangePetName)

// 0x10055390 - NetHandler slot 78 (C2: a second argument, passed on)
void NConsoleWnd::RequestPetition(TCHAR* Text, INT Type)
{
    Engine->NetHandler->RequestPetition(Text, Type);
}
NW_HOOK_METHOD(0x10055390, NConsoleWnd, RequestPetition)

// 0x100553a0 - NetHandler slot 79
void NConsoleWnd::RequestPetitionCancel()
{
    Engine->NetHandler->RequestPetitionCancel();
}
NW_HOOK_METHOD(0x100553a0, NConsoleWnd, RequestPetitionCancel)

// 0x100553b0 - NetHandler slot 80
void NConsoleWnd::RequestGMList()
{
    Engine->NetHandler->RequestGMList();
}
NW_HOOK_METHOD(0x100553b0, NConsoleWnd, RequestGMList)

// The GM window's queries about a character: NetHandler slot 81 (name, what).

// 0x100553c0
void NConsoleWnd::RequestGMInventory(TCHAR* Name)
{
    Engine->NetHandler->RequestGMCommand(Name, 5);
}
NW_HOOK_METHOD(0x100553c0, NConsoleWnd, RequestGMInventory)

// 0x100553e0
void NConsoleWnd::RequestGMSkills(TCHAR* Name)
{
    Engine->NetHandler->RequestGMCommand(Name, 3);
}
NW_HOOK_METHOD(0x100553e0, NConsoleWnd, RequestGMSkills)

// 0x10055400
void NConsoleWnd::RequestGMStatus(TCHAR* Name)
{
    Engine->NetHandler->RequestGMCommand(Name, 1);
}
NW_HOOK_METHOD(0x10055400, NConsoleWnd, RequestGMStatus)

// 0x10055420
void NConsoleWnd::RequestGMQuests(TCHAR* Name)
{
    Engine->NetHandler->RequestGMCommand(Name, 4);
}
NW_HOOK_METHOD(0x10055420, NConsoleWnd, RequestGMQuests)

// 0x10055440
void NConsoleWnd::RequestGMWarehouse(TCHAR* Name)
{
    Engine->NetHandler->RequestGMCommand(Name, 6);
}
NW_HOOK_METHOD(0x10055440, NConsoleWnd, RequestGMWarehouse)

// 0x10055460
void NConsoleWnd::RequestGMPledge(TCHAR* Name)
{
    Engine->NetHandler->RequestGMCommand(Name, 2);
}
NW_HOOK_METHOD(0x10055460, NConsoleWnd, RequestGMPledge)

// The private stores: selling (NetHandler slots 195..201, packets 0x73..0x79) and buying
// (202..208, packets 0x90..0x96).

// 0x10055a20 - "/vendor": manage the selling store
void NConsoleWnd::RequestVendor()
{
    Engine->NetHandler->RequestPrivateStoreManage();
}
NW_HOOK_METHOD(0x10055a20, NConsoleWnd, RequestVendor)

// 0x10055a30 - the managed selling list (item, amount, price...)
void NConsoleWnd::RequestSetPrivateStoreListSell(L2ParamStack& Params)
{
    Engine->NetHandler->SetPrivateStoreList(Params);
}
NW_HOOK_METHOD(0x10055a30, NConsoleWnd, RequestSetPrivateStoreListSell)

// 0x10055a40 - the managing window closed without opening the store
void NConsoleWnd::RequestPrivateStoreManageCancel()
{
    Engine->NetHandler->RequestPrivateStoreManageCancel();
}
NW_HOOK_METHOD(0x10055a40, NConsoleWnd, RequestPrivateStoreManageCancel)

// 0x10055a50
void NConsoleWnd::RequestPrivateStoreQuitSell()
{
    Engine->NetHandler->RequestPrivateStoreQuit();
}
NW_HOOK_METHOD(0x10055a50, NConsoleWnd, RequestPrivateStoreQuitSell)

// 0x10055a60
void NConsoleWnd::SetPrivateStoreMsg(TCHAR* Text)
{
    Engine->NetHandler->SetPrivateStoreMsg(Text);
}
NW_HOOK_METHOD(0x10055a60, NConsoleWnd, SetPrivateStoreMsg)

// 0x10055a70 - buying from another player's store
void NConsoleWnd::RequestPrivateStoreBuy(L2ParamStack& Params)
{
    Engine->NetHandler->SendPrivateStoreBuyList(Params);
}
NW_HOOK_METHOD(0x10055a70, NConsoleWnd, RequestPrivateStoreBuy)

// 0x10055ac0 - "/buy": manage the buying store
void NConsoleWnd::RequestBuy()
{
    Engine->NetHandler->RequestPrivateStoreBuyManage();
}
NW_HOOK_METHOD(0x10055ac0, NConsoleWnd, RequestBuy)

// 0x10055ad0
void NConsoleWnd::RequestSetPrivateStoreListBuy(L2ParamStack& Params)
{
    Engine->NetHandler->SetPrivateStoreBuyList(Params);
}
NW_HOOK_METHOD(0x10055ad0, NConsoleWnd, RequestSetPrivateStoreListBuy)

// 0x10055ae0
void NConsoleWnd::RequestPrivateStoreBuyManageCancel()
{
    Engine->NetHandler->RequestPrivateStoreBuyManageCancel();
}
NW_HOOK_METHOD(0x10055ae0, NConsoleWnd, RequestPrivateStoreBuyManageCancel)

// 0x10055af0
void NConsoleWnd::RequestPrivateStoreQuitBuy()
{
    Engine->NetHandler->RequestPrivateStoreBuyQuit();
}
NW_HOOK_METHOD(0x10055af0, NConsoleWnd, RequestPrivateStoreQuitBuy)

// 0x10055b00
void NConsoleWnd::SetPrivateStoreBuyMsg(TCHAR* Text)
{
    Engine->NetHandler->SetPrivateStoreBuyMsg(Text);
}
NW_HOOK_METHOD(0x10055b00, NConsoleWnd, SetPrivateStoreBuyMsg)

// 0x10055b10 - selling to another player's buying store
void NConsoleWnd::RequestPrivateStoreSell(L2ParamStack& Params)
{
    Engine->NetHandler->SendPrivateStoreBuyBuyList(Params);
}
NW_HOOK_METHOD(0x10055b10, NConsoleWnd, RequestPrivateStoreSell)

// 0x10056000 - NetHandler slot 248 (C2 only; name ours, after the slot's)
void NConsoleWnd::BypassUserCmd(L2ParamStack& Params)
{
    Engine->NetHandler->BypassUserCmd(Params);
}
NW_HOOK_METHOD(0x10056000, NConsoleWnd, BypassUserCmd)

// 0x10056010 - NetHandler slot 247
void NConsoleWnd::RequestDwarvenManufacture()
{
    Engine->NetHandler->RequestRemainTime();
}
NW_HOOK_METHOD(0x10056010, NConsoleWnd, RequestDwarvenManufacture)

// 0x10056030 - "Previous" on the character selection: back to the login screen (NetHandler
// slots 147, the logout, and 168)
void NConsoleWnd::ReturnToLogin()
{
    LobbyWnd->ClearCharacters();
    LobbyWnd->HideWindow();
    Engine->NetHandler->SendLogOutPacket();
    Engine->NetHandler->AuthReconnect();
    LoginWnd->ResetWindows();
    LoginWnd->ShowWindow();
    MessageWnd->SetText(TEXT("Copyright 2002 NCsoft Corporation. All Rights Reserved."));
    MoveCameraByState(L2CS_None, L2CS_Login);
    SetState(L2CS_Login);
}
NW_HOOK_METHOD(0x10056030, NConsoleWnd, ReturnToLogin)

// 0x10056680 - NetHandler slot 30
INT NConsoleWnd::GetServerID()
{
    return Engine->NetHandler->GetServerID();
}
NW_HOOK_METHOD(0x10056680, NConsoleWnd, GetServerID)

// 0x10056690 - NetHandler slot 35: an item lying on the ground
Item* NConsoleWnd::FindItemByID(INT ObjectID)
{
    return Engine->NetHandler->GetItem(ObjectID);
}
NW_HOOK_METHOD(0x10056690, NConsoleWnd, FindItemByID)

// 0x100566a0 - NetHandler slot 31
PetInfo NConsoleWnd::GetPetInfo()
{
    return Engine->NetHandler->GetPetInfo();
}
NW_HOOK_METHOD(0x100566a0, NConsoleWnd, GetPetInfo)

// 0x100566c0 - NetHandler slot 32
User* NConsoleWnd::FindUserByName(TCHAR* Name)
{
    return Engine->NetHandler->GetUser(Name);
}
NW_HOOK_METHOD(0x100566c0, NConsoleWnd, FindUserByName)

// 0x100566d0 - NetHandler slot 33 (not inlined: the stock calls it)
__declspec(noinline) User* NConsoleWnd::FindUserByID(INT ObjectID)
{
    return Engine->NetHandler->GetUser(ObjectID);
}
NW_HOOK_METHOD(0x100566d0, NConsoleWnd, FindUserByID)

// 0x100566e0 - NetHandler slot 34
FServerStaticObject* NConsoleWnd::FindStaticObjectInfo(INT CreatureID)
{
    return Engine->NetHandler->GetStaticObject(CreatureID);
}
NW_HOOK_METHOD(0x100566e0, NConsoleWnd, FindStaticObjectInfo)

// 0x100566f0 - the pawn under the mouse; with bCheck, only a player's (its User record,
// NetHandler slot 33, without Unknown8)
AActor* NConsoleWnd::GetTargetedActor(UBOOL bCheck)
{
    guard(NConsoleWnd::GetTargetedActor);
    if (!Engine || !HoveredActor(Engine) || !HoveredActor(Engine)->IsA(APawn::StaticClass()))
        return NULL;
    AActor* Actor = HoveredActor(Engine);
    if (bCheck)
    {
        User* Info = Engine->NetHandler->GetUser(Actor->CreatureID);
        if (!Info || Info->Unknown8)
            return NULL;
    }
    return Actor;
    unguard;
}
NW_HOOK_METHOD(0x100566f0, NConsoleWnd, GetTargetedActor)

// 0x10056820 - NetHandler slot 38
PledgeInfo* NConsoleWnd::GetPledgeInfo(INT PledgeID)
{
    return Engine->NetHandler->GetPledgeInfo(PledgeID);
}
NW_HOOK_METHOD(0x10056820, NConsoleWnd, GetPledgeInfo)

// 0x10056830 - NetHandler slot 39
UTexture* NConsoleWnd::GetPledgeCrestTexture(INT PledgeID)
{
    return Engine->NetHandler->GetPledgeCrestTex(PledgeID);
}
NW_HOOK_METHOD(0x10056830, NConsoleWnd, GetPledgeCrestTexture)

// 0x10056840 - NetHandler slot 40
UTexture* NConsoleWnd::GetAllyCrestTexture(INT AllyID)
{
    return Engine->NetHandler->GetAllyCrestTex(AllyID);
}
NW_HOOK_METHOD(0x10056840, NConsoleWnd, GetAllyCrestTexture)

// 0x10058d00 - NetHandler slot 87 (by value)
void NConsoleWnd::RequestDropItem(NItemRequest Request, FVector Location)
{
    guard(NConsoleWnd::RequestDropItem);
    Engine->NetHandler->RequestDropItem(Request, Location);
    unguard;
}
NW_HOOK_METHOD(0x10058d00, NConsoleWnd, RequestDropItem)

// The actions on a target: NetHandler slot 53 (object id, where the player stands, Shift held)
// selects it, slot 52 (same) attacks it.

// 0x10059b50 - a click on the player's status window
void NConsoleWnd::TargetMyself()
{
    APawn* Pawn = PlayerPawn(Engine);
    if (!Pawn)
        return;
    FVector Origin = Pawn->Location;
    Engine->NetHandler->Action(PlayerPawn(Engine)->CreatureID, Origin,
                               (UBOOL)Engine->Client->Viewports(0)->Input->KeyDown(IK_Shift));
}
NW_HOOK_METHOD(0x10059b50, NConsoleWnd, TargetMyself)

// 0x10059be0
void NConsoleWnd::RequestTarget(INT ObjectID)
{
    APawn* Pawn = PlayerPawn(Engine);
    if (!Pawn)
        return;
    FVector Origin = Pawn->Location;
    Engine->NetHandler->Action(ObjectID, Origin,
                               (UBOOL)Engine->Client->Viewports(0)->Input->KeyDown(IK_Shift));
}
NW_HOOK_METHOD(0x10059be0, NConsoleWnd, RequestTarget)

// 0x10059c60 - the item nearest to the player within Range (NetHandler slot 211, a float); the
// stock tests the address of its field +0x10 too, never NULL
void NConsoleWnd::RequestPickUp(INT Range, INT ObjectID)
{
    Item* Found = Engine->NetHandler->GetNearestItem((FLOAT)Range, ObjectID);
    if (!Found)
        return;
    Engine->NetHandler->Action(Found->ObjectID, PlayerPawn(Engine)->Location,
                               (UBOOL)Engine->Client->Viewports(0)->Input->KeyDown(IK_Shift));
}
NW_HOOK_METHOD(0x10059c60, NConsoleWnd, RequestPickUp)

// 0x10059cf0 - the next creature within Range after CurrentID (NetHandler slot 214)
void NConsoleWnd::RequestTargetNext(INT Range, INT CurrentID)
{
    User* Next = Engine->NetHandler->GetNextEnemy((FLOAT)Range, CurrentID);
    if (!Next || !Next->Pawn)
        return;
    Engine->NetHandler->Action(Next->ObjectID, PlayerPawn(Engine)->Location,
                               (UBOOL)Engine->Client->Viewports(0)->Input->KeyDown(IK_Shift));
}
NW_HOOK_METHOD(0x10059cf0, NConsoleWnd, RequestTargetNext)

// 0x10059d80 - what the party member targets (C2: the member's target may be a static object, found by the network
// handler's slot 35; the action starts from the player's own place: the Origin argument is not read)
void NConsoleWnd::AssistMember(INT ObjectID, FVector Origin)
{
    User* Member = FindUserByID(ObjectID);
    if (!Member || !Member->Pawn)
        return;
    AController* Controller = Member->Pawn->Controller;
    if (!Controller || !Controller->Target)
        return;
    User* Target = FindUserByID(Controller->Target->CreatureID);
    INT TargetObjectID;
    if (Target)
        TargetObjectID = Target->ObjectID;
    else
    {
        FServerStaticObject* StaticObject = Engine->NetHandler->GetStaticObject(Member->Pawn->Controller->Target->CreatureID);
        if (!StaticObject)
            return;
        TargetObjectID = StaticObject->Unknown8;
    }
    Engine->NetHandler->Action(TargetObjectID, PlayerPawn(Engine)->Location,
                               (UBOOL)Engine->Client->Viewports(0)->Input->KeyDown(IK_Shift));
}
NW_HOOK_METHOD(0x10059d80, NConsoleWnd, AssistMember)

// 0x10059ec0 - a click on the pet / summon status window
void NConsoleWnd::RequestAction(INT ObjectID, FVector Origin)
{
    Engine->NetHandler->Action(ObjectID, Origin,
                               (UBOOL)Engine->Client->Viewports(0)->Input->KeyDown(IK_Shift));
}
NW_HOOK_METHOD(0x10059ec0, NConsoleWnd, RequestAction)

// 0x10059f20 - attack with Ctrl held (NetHandler slot 51), else select (slot 52). C2 takes two more words: the
// callers push them after the position; either is replaced by the key's state when Ctrl / Shift is down
void NConsoleWnd::RequestAttack(INT ObjectID, FVector Origin, INT bForceAttack, INT bShift)
{
    if (IsCtrlDown())
        bForceAttack = IsCtrlDown();
    if (Engine->Client->Viewports(0)->Input->KeyDown(IK_Shift))
        bShift = Engine->Client->Viewports(0)->Input->KeyDown(IK_Shift);
    if (bForceAttack)
        Engine->NetHandler->Attack(ObjectID, Origin, bShift);
    else
        Engine->NetHandler->Action(ObjectID, Origin, bShift & 0xFF);
}
NW_HOOK_METHOD(0x10059f20, NConsoleWnd, RequestAttack)

// 0x10060ac0 - the player's User record (not inlined: the stock calls it)
__declspec(noinline) User* NConsoleWnd::GetMyUser()
{
    APawn* Pawn = PlayerPawn(Engine);
    if (!Pawn)
        return NULL;
    return FindUserByID(Pawn->CreatureID);
}
NW_HOOK_METHOD(0x10060ac0, NConsoleWnd, GetMyUser)

// 0x10060b10 - the User record of the player's target (not inlined: the stock calls it)
__declspec(noinline) User* NConsoleWnd::GetSelectedActor()
{
    guard(NConsoleWnd::GetSelectedActor);
    APawn* Pawn = PlayerPawn(Engine);
    if (Engine && Pawn && Pawn->Controller && Pawn->Controller->Target)
        return FindUserByID(Pawn->Controller->Target->CreatureID);
    return NULL;
    unguard;
}
NW_HOOK_METHOD(0x10060b10, NConsoleWnd, GetSelectedActor)

// 0x10060bc0 - the player's target as an actor (doors and the other static objects have no
// User record)
AActor* NConsoleWnd::GetSelectedStaticActor()
{
    APawn* Pawn = PlayerPawn(Engine);
    if (Engine && Pawn && Pawn->Controller && Pawn->Controller->Target)
        return Pawn->Controller->Target;
    return NULL;
}
NW_HOOK_METHOD(0x10060bc0, NConsoleWnd, GetSelectedStaticActor)

// 0x10060c00 - the player has a target (the shortcut keys)
UBOOL NConsoleWnd::HasSelectedTarget()
{
    APawn* Pawn = PlayerPawn(Engine);
    if (Engine && Pawn && Pawn->Controller && Pawn->Controller->Target)
        return 1;
    return 0;
}
NW_HOOK_METHOD(0x10060c00, NConsoleWnd, HasSelectedTarget)

// C2 CANDIDATE: the C2 function is not the same code as the C1 one; kept only while it matches the stock one (verify_data)
// 0x10061130 - move there (NetHandler slot 90: the player's id, 3 and the point in whole units,
// or 2 and no height)
UBOOL NConsoleWnd::MoveToLocation(FVector Loc, UBOOL bNoZ)
{
    APawn* Pawn = PlayerPawn(Engine);
    if (bNoZ)
        Engine->NetHandler->RequestTeleport(Pawn->CreatureID, 2, (INT)Loc.X, (INT)Loc.Y, 0);
    else
        Engine->NetHandler->RequestTeleport(Pawn->CreatureID, 3, (INT)Loc.X, (INT)Loc.Y,
                                            (INT)Loc.Z);
    return 1;
}
NW_HOOK_METHOD(0x10061130, NConsoleWnd, MoveToLocation)

// 0x10067420 - what the target targets (C2: a static object's too: slot 35 of the network handler)
void NConsoleWnd::RequestAssist()
{
    User* Target = GetSelectedActor();
    if (!Target || !Target->Pawn)
        return;
    AController* Controller = Target->Pawn->Controller;
    if (!Controller || !Controller->Target)
        return;
    User* AssistTarget = FindUserByID(Controller->Target->CreatureID);
    INT ObjectID;
    if (AssistTarget)
        ObjectID = AssistTarget->ObjectID;
    else
    {
        FServerStaticObject* StaticObject = Engine->NetHandler->GetStaticObject(Target->Pawn->Controller->Target->CreatureID);
        if (!StaticObject)
            return;
        ObjectID = StaticObject->Unknown8;
    }
    Engine->NetHandler->Action(ObjectID, PlayerPawn(Engine)->Location,
                               (UBOOL)Engine->Client->Viewports(0)->Input->KeyDown(IK_Shift));
}
NW_HOOK_METHOD(0x10067420, NConsoleWnd, RequestAssist)


/*-----------------------------------------------------------------------------
    The virtual requests: UL2ConsoleWnd slots the engine leaves pure and the console's own
    (272..284), which the windows call through the vtable.
-----------------------------------------------------------------------------*/

// 0x10052640 - NetHandler slot 69
void NConsoleWnd::RequestServerList()
{
    guard(NConsoleWnd::RequestServerList);
    Engine->NetHandler->RequestServerList();
    unguard;
}
NW_HOOK_METHOD(0x10052640, NConsoleWnd, RequestServerList)

// 0x100531f0 - the answer to a resurrection offer (NetHandler slot 76: the answer and the
// offer's two values); refused, the restart menu comes back
void NConsoleWnd::AnswerReviveRequest(UBOOL bYes, INT Param1, INT Param2)
{
    if (!bYes)
        RestartMenuWnd->ShowWindow();
    L2ParamStack Params(10);
    Params.PushBack((void*)(INT_PTR)bYes);
    Params.PushBack((void*)(INT_PTR)Param1);
    Params.PushBack((void*)(INT_PTR)Param2);
    Engine->NetHandler->ReviveReply(Params);
}
NW_HOOK_METHOD(0x100531f0, NConsoleWnd, AnswerReviveRequest)

// 0x10053650 - NetHandler slot 171: the item the enchant scroll goes on
void NConsoleWnd::RequestEnchantItem(INT ObjectID)
{
    guard(NConsoleWnd::RequestEnchantItem);
    Engine->NetHandler->RequestEnchantItem(ObjectID);
    unguard;
}
NW_HOOK_METHOD(0x10053650, NConsoleWnd, RequestEnchantItem)

// 0x10053740 - NetHandler slot 165
void NConsoleWnd::RequestDestroyItem(NItemRequest Request)
{
    guard(NConsoleWnd::RequestDestroyItem);
    Engine->NetHandler->RequestDestroyItem(Request);
    unguard;
}
NW_HOOK_METHOD(0x10053740, NConsoleWnd, RequestDestroyItem)

// 0x100537d0 - NetHandler slot 166
void NConsoleWnd::RequestCrystallizeItem(INT ObjectID, INT Count)
{
    Engine->NetHandler->RequestCrystallizeItem(ObjectID, Count);
}
NW_HOOK_METHOD(0x100537d0, NConsoleWnd, RequestCrystallizeItem)

// 0x10053970 - NetHandler slot 156: the answer to a trade offer
void NConsoleWnd::AnswerTradeRequest(L2ParamStack& Params)
{
    Engine->NetHandler->AnswerTradeRequest(Params);
}
NW_HOOK_METHOD(0x10053970, NConsoleWnd, AnswerTradeRequest)

// 0x10053fb0 - NetHandler slot 173 (C2: not in the Thai HTML mode either)
UBOOL NConsoleWnd::RequestQuestList()
{
    guard(NConsoleWnd::RequestQuestList);
    if (GIsOnlyL2Console || GL2ThaiHtmlMode)
        return 0;
    return Engine->NetHandler->RequestQuestList();
    unguard;
}
NW_HOOK_METHOD(0x10053fb0, NConsoleWnd, RequestQuestList)

// 0x10054050 - NetHandler slot 170
void NConsoleWnd::RequestDestroyQuest(INT QuestID)
{
    guard(NConsoleWnd::RequestDestroyQuest);
    Engine->NetHandler->RequestDestroyQuest(QuestID);
    unguard;
}
NW_HOOK_METHOD(0x10054050, NConsoleWnd, RequestDestroyQuest)

// 0x10054340 - NetHandler slot 172: what a skill of the trainer needs
void NConsoleWnd::RequestAcquireSkillInfo(L2ParamStack& Params)
{
    Engine->NetHandler->RequestAcquireSkillInfo(Params);
}
NW_HOOK_METHOD(0x10054340, NConsoleWnd, RequestAcquireSkillInfo)

// 0x10054350 - NetHandler slot 173
void NConsoleWnd::RequestAcquireSkill(L2ParamStack& Params)
{
    Engine->NetHandler->RequestAcquireSkill(Params);
}
NW_HOOK_METHOD(0x10054350, NConsoleWnd, RequestAcquireSkill)

// 0x10054bc0 - NetHandler slot 175
void NConsoleWnd::RequestStartAllianceWar(L2ParamStack& Params)
{
    Engine->NetHandler->RequestStartAllianceWar(Params);
}
NW_HOOK_METHOD(0x10054bc0, NConsoleWnd, RequestStartAllianceWar)

// 0x10054bd0 - NetHandler slot 176
void NConsoleWnd::RequestReplyStartAllianceWar(L2ParamStack& Params)
{
    Engine->NetHandler->ReplyStartAllianceWar(Params);
}
NW_HOOK_METHOD(0x10054bd0, NConsoleWnd, RequestReplyStartAllianceWar)

// 0x10054be0 - NetHandler slot 177
void NConsoleWnd::RequestStopAllianceWar(L2ParamStack& Params)
{
    Engine->NetHandler->RequestStopAllianceWar(Params);
}
NW_HOOK_METHOD(0x10054be0, NConsoleWnd, RequestStopAllianceWar)

// 0x10054bf0 - NetHandler slot 178
void NConsoleWnd::RequestReplyStopAllianceWar(L2ParamStack& Params)
{
    Engine->NetHandler->ReplyStopAllianceWar(Params);
}
NW_HOOK_METHOD(0x10054bf0, NConsoleWnd, RequestReplyStopAllianceWar)

// 0x10054c00 - NetHandler slot 179
void NConsoleWnd::RequestSurrenderAllianceWar(L2ParamStack& Params)
{
    Engine->NetHandler->RequestSurrenderAllianceWar(Params);
}
NW_HOOK_METHOD(0x10054c00, NConsoleWnd, RequestSurrenderAllianceWar)

// 0x10054c10 - NetHandler slot 187: the freight
void NConsoleWnd::RequestPackageSend(L2ParamStack& Params)
{
    Engine->NetHandler->RequestPackageSend(Params);
}
NW_HOOK_METHOD(0x10054c10, NConsoleWnd, RequestPackageSend)

// 0x10054c20 - NetHandler slot 188
void NConsoleWnd::RequestPackageSendableItemList(L2ParamStack& Params)
{
    Engine->NetHandler->RequestPackageSendableItemList(Params);
}
NW_HOOK_METHOD(0x10054c20, NConsoleWnd, RequestPackageSendableItemList)

// 0x10054cd0 - NetHandler slot 180 (no caller in NWindow)
void NConsoleWnd::vfn184(L2ParamStack& Params)
{
    Engine->NetHandler->RequestCastleSiegeInfo(Params);
}
NW_HOOK_METHOD(0x10054cd0, NConsoleWnd, vfn184)

// 0x10054ce0 - NetHandler slot 181
void NConsoleWnd::RequestSiegeAttackerList(L2ParamStack& Params)
{
    Engine->NetHandler->RequestCastleSiegeAttackerList(Params);
}
NW_HOOK_METHOD(0x10054ce0, NConsoleWnd, RequestSiegeAttackerList)

// 0x10054cf0 - NetHandler slot 182
void NConsoleWnd::RequestSiegeDefenderList(L2ParamStack& Params)
{
    Engine->NetHandler->RequestCastleSiegeDefenderList(Params);
}
NW_HOOK_METHOD(0x10054cf0, NConsoleWnd, RequestSiegeDefenderList)

// 0x10054d00 - NetHandler slot 183
void NConsoleWnd::RequestJoinSiege(L2ParamStack& Params)
{
    Engine->NetHandler->RequestJoinCastleSiege(Params);
}
NW_HOOK_METHOD(0x10054d00, NConsoleWnd, RequestJoinSiege)

// 0x10054d10 - NetHandler slot 184
void NConsoleWnd::RequestConfirmSiegeWaitingList(L2ParamStack& Params)
{
    Engine->NetHandler->RequestConfirmCastleSiegeWaitingList(Params);
}
NW_HOOK_METHOD(0x10054d10, NConsoleWnd, RequestConfirmSiegeWaitingList)

// 0x10054d20 - NetHandler slot 185: the siege date
void NConsoleWnd::RequestSetSiegeTime(L2ParamStack& Params)
{
    Engine->NetHandler->RequestSetCastleSiegeTime(Params);
}
NW_HOOK_METHOD(0x10054d20, NConsoleWnd, RequestSetSiegeTime)

// 0x100551c0 - the tutorial's client events: each pending one (TutorialEnableClientEvent) is
// sent once (NetHandler slot 97)
void NConsoleWnd::SendTutorialEvent(INT Event)
{
    if (TutorialEvents & Event)
    {
        Engine->NetHandler->RequestTutorialClientEvent(Event);
        TutorialEvents ^= Event;
    }
}
NW_HOOK_METHOD(0x100551c0, NConsoleWnd, SendTutorialEvent)

// 0x10055ba0 - NetHandler slot 186
void NConsoleWnd::RequestBuyVIPItem(L2ParamStack& Params)
{
    Engine->NetHandler->RequestMultiSellChoose(Params);
}
NW_HOOK_METHOD(0x10055ba0, NConsoleWnd, RequestBuyVIPItem)

// 0x10056020 - NetHandler slot 164: a GM command
void NConsoleWnd::BuilderCommand(L2ParamStack& Params)
{
    Engine->NetHandler->SendBypassBuildCmd(Params);
}
NW_HOOK_METHOD(0x10056020, NConsoleWnd, BuilderCommand)

// 0x10057310 (vslot 216) - NetHandler slot 178 (no caller in NWindow). The stock tail-jumps into the slot
// with Params still on the stack: the headers it was built with gave the slot one argument,
// but the client's UNetworkHandler::RequestSkillCoolTime takes none (and pops none). Called
// through the slot to keep that, not through the declaration.
void NConsoleWnd::vfn176(L2ParamStack& Params)
{
    nw::VirtualCall<void>(Engine->NetHandler, 178, &Params);
}
NW_HOOK_METHOD(0x10057310, NConsoleWnd, vfn176)
