#pragma once
// NConsoleWnd - the game console (UClass "ConsoleWnd", CLASS_Config, sizeof 0x35B8): implements
// UL2ConsoleWnd for the engine, owns the window manager and every window, runs the timers.
// The whole vtable is declared in the order of the C2 one (335 slots, 0x10115438): the 275 slots the
// engine leaves pure (all named: the 39 that only C2 has from the UGameEngine handlers that call them and
// from what they do, NConsoleWndC2.cpp), the engine's own, and 15 of the console's (slots 320..334); docs/notas/NConsoleWnd.md
// maps the C1 ones. Every virtual below sits at its C2 slot (the number in its comment).

#include "nwindow/NCObject.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCWnd;
class NCConsole;
class NCVirtualWndMain;
class NCDebugWnd;
struct NDialogInfo;
struct CharacterInfo;
class NCHtmlViewer;
class NCRestartMenuWnd;
class NCOptionWnd;
class NCLoginWnd;
class NCLobbyWnd;
class NCPawnCreateWnd;
class NCLoadingWnd;
class NCMessageWnd;
class NCDialogBox;
class NKeyCrypt;
struct NItemRequest;
struct NMacroEntry;

// one SetTimer record (NConsoleWnd::NSetTimer)
struct NTimer
{
    UINT ID;              // +0x00
    UINT Elapse;          // +0x04 milliseconds
    UBOOL bKilled;        // +0x08 set by NKillTimer, dropped by the next Tick
    FLOAT Elapsed;        // +0x0C seconds since the last WM_TIMER
    NCObject* Owner;      // +0x10 gets WindowProc(WM_TIMER, ID, 0)
};
NW_ASSERT_SIZE(NTimer, 0x14);

// a parameter of the system message being received (names ours; AddSystemMessageParam): its
// text, made from a string or a number
struct NSysMsgParam
{
    TCHAR* Text;          // +0x00 GMalloc'd copy
    INT Len;              // +0x04 appStrlen(Text)

    void Set(const TCHAR* InText);    // 0x10047380 a copy of InText; nothing for NULL
    void SetNumber(INT Value);        // 0x100473d0 "%d"
};
NW_ASSERT_SIZE(NSysMsgParam, 0x8);

// The Korean particles of the system messages ("%" and a particle after a "$sN"): whether
// a Hangul syllable ends without a final consonant (any other character counts so; stock
// 0x100427d0, which takes the character in AX: not hooked, its own calling convention),
// and the particle's form after it: i / ga, eul / reul, eun / neun, gwa / wa (0 for
// another character: nothing is put). MakeSystemMessage and EndSystemMessageParam.
FORCEINLINE UBOOL HasNoFinalConsonant(TCHAR C)
{
    if (C < 0xAC00 || C > 0xD7A3)
        return 1;
    return (C - 0xAC00) % (21 * 28) % 28 == 0;
}
FORCEINLINE TCHAR ParticleForm(TCHAR Particle, UBOOL bNoFinal)
{
    switch (Particle)
    {
    case 0xC774: return bNoFinal ? 0xAC00 : 0xC774;
    case 0xC744: return bNoFinal ? 0xB97C : 0xC744;
    case 0xC740: return bNoFinal ? 0xB294 : 0xC740;
    case 0xC640: return bNoFinal ? 0xC640 : 0xACFC;
    }
    return 0;
}

// One line of questname-*.dat in the C2 game data (name ours): 1000 of them at FL2GameData +0x4CF4
// (0x24 bytes each: C1's FL2QuestNameData had 0x14, the two texts are FStrings now). The Engine
// header does not place the table yet; NConsoleWnd reads it through GQuestNames.
struct NQuestNameLine
{
    INT bValid;               // +0x00 0: no quest here
    INT QuestID;              // +0x04
    INT Step;                 // +0x08
    FString Name;             // +0x0C the quest's (AddQuestID)
    FString Text;             // +0x18 the step's (GetQuestText)
};
NW_ASSERT_SIZE(NQuestNameLine, 0x24);
#define GQuestNames ((NQuestNameLine*)((BYTE*)&GL2GameData + 0x4CF4))

// A dye of the henna lists (names ours; 0x44 bytes in C2: UGameEngine::ReceiveHennaInfo and its kind pass it by value
// to the console, NConsoleWndC2.cpp reads these fields: the henna's id for the henna data, the dye's item id for the
// item data)
struct HennaInfo
{
    INT HennaID;               // +0x00 FL2GameData::GetHennaData's key
    INT ItemID;                // +0x04 the dye's item (FL2GameData::GetItemData)
    INT Param08;               // +0x08
    INT Param0C;               // +0x0C
    INT Param10;               // +0x10
    BYTE _Unknown14[0x44 - 0x14]; // +0x14 not read by NWindow
};
NW_ASSERT_SIZE(HennaInfo, 0x44);

// A macro of the list the server sends (UGameEngine::ReceiveMacroListInfo; 0x3FC bytes in C2: the console copies it
// into the macro window's records and its macro object; the fields are the macro window's business)
struct MacroInfo
{
    BYTE Data[0x3FC];
};
NW_ASSERT_SIZE(MacroInfo, 0x3FC);

// a window's place as NConsoleWnd::ScaleRect takes it (names ours): the screen place and size
struct NResizeRect
{
    INT X;
    INT Y;
    INT W;
    INT H;
};
NW_ASSERT_SIZE(NResizeRect, 0x10);

class NConsoleWnd : public UL2ConsoleWnd
{
    NW_DECLARE_CLASS(NConsoleWnd, UL2ConsoleWnd, CLASS_Config, 0x101B1778)

    UBOOL bInitialized;               // +0x3518 (name ours) set at the end of Initialize
    NCVirtualWndMain* WndMain;        // +0x351C (NConsoleWnd::SetCursor)
    // the windows of the login and character stages (Initialize; names ours)
    NCRestartMenuWnd* RestartMenuWnd; // +0x3520 after dying
    NCOptionWnd* OptionWnd;           // +0x3524
    NCLoginWnd* LoginWnd;             // +0x3528
    NCWnd* CreditWnd;                 // +0x352C (name ours; new in C2) NCCreditWnd, a top window made by Initialize
    NCLobbyWnd* LobbyWnd;             // +0x3530 the character selection
    NCPawnCreateWnd* PawnCreateWnd;   // +0x3534 the character creation
    NCLoadingWnd* LoadingWnd;         // +0x3538
    NCMessageWnd* MessageWnd;         // +0x353C the line under the login screens
    NCWnd* Desktop;                   // +0x3540 (name ours) the screen-sized NCConsole window,
                                      //   root of the in-game UI (Initialize)
    NCDebugWnd* DebugWnd;             // +0x3544 (NCObject::Debugf)
    NCWnd* UnusedWnd;                 // +0x3548 (name ours) a top window never created
                                      //   (Finalize destroys it)
    NCDialogBox* DialogBox;           // +0x354C (name ours) the login stages' dialog box
    NCWnd* PetitionWnd;               // +0x3550 (name ours) top-level NCPetitionWnd, the GM
                                      //   chat window (Initialize)
    NCWnd* WarningDialogBox;          // +0x3554 (name ours; new in C2) NCWarningDialogBox, a top window made by Initialize
    TArray<NSysMsgParam> SysMsgParams;// +0x3558 (name ours) of the system message being received (the constructor
                                      //   clears +0x3558, +0x355C, +0x3560; Add/EndSystemMessageParam, MakeSystemMessage)
    NCWnd* WordWrapperWnd;            // +0x3564 (name ours; new in C2) L2WordWrapperWnd, a top window made by Initialize
    TArray<NTimer> Timers;            // +0x3568
    INT MouseWheelDelta;              // +0x3574 (name ours) of the last wheel event
    FLOAT DeltaTime;                  // +0x3578 seconds of the last Tick
    INT Unknown357C;                  // +0x357C (new in C2; the constructor sets 1)
    INT Unknown3580;                  // +0x3580 (new in C2; the constructor clears it)
    UBOOL bCtrlDown;                  // +0x3584 (name ours) PressCtrl / ReleaseCtrl
    UBOOL bCtrlDownAlt;               // +0x3588 (name ours) IsCtrlDown's other half, never set
    UBOOL bSuspended;                 // +0x358C (name ours) no painting, no input; never set
    UBOOL bWheelHandled;              // +0x3590 (name ours) the last wheel step went to a window:
                                      //   its release is eaten (MasterConsoleEventProcess)
    INT Unknown3720;                  // +0x3594
    INT TutorialEvents;               // +0x3598 (name ours) the pending tutorial event bits
    NKeyCrypt* KeyCrypt;              // +0x359C (new in C2; the constructor clears it) the keyboard protection helper
                                      //   (NKeyCrypt.h), made by Initialize when GL2UseKeyCrypt is set
    INT LoadingScreenID;              // +0x35A0 (name ours) shown when entering the game (10001)
    INT PartyItemDistribution;        // +0x35A4 (name ours) sent with party invitations (NCGameOptionWnd::Apply writes it,
                                      //   NCMatchWnd::OnInvite and UseAction read it)
    UBOOL bNoChatOnEnter;             // +0x35A8 (name ours) Enter leaves the chat line alone (the constructor clears it,
                                      //   HookShortCutKey reads it); never set
    NCWnd* PawnViewerWnd[3];          // +0x35AC (name ours) the pawn viewer's windows

    NConsoleWnd();                                                   // 0x1006b6a0
    NCConsole* GetDesktop() const;                                   // Desktop as what it is
                                                                     //   (defined in NCConsole.h)

    // Decompiled in NConsoleWndNet.cpp / NConsoleWndData.cpp, not in the groups of the windows
    // below (names ours: the requests after the protocol's packets)
    void RequestBypassToServer(L2ParamStack& Params);                // 0x10055060 HTML "bypass"
    void RequestTutorialPassCmdToServer(L2ParamStack& Params);       // 0x100551a0 the tutorial's
    Item* FindItemByID(INT ObjectID);                                // 0x10056690 on the ground
    UTexture* GetAllyCrestTexture(INT AllyID);                       // 0x10056840 8x12 crest
    void RequestAddTradeItem(NItemRequest Request);                  // 0x10053860 (guard name)
    void RequestDropItem(NItemRequest Request, FVector Location);    // 0x10058d00 (guard name)
    const TCHAR* GetSkillIconName(INT SkillID, INT Level);           // 0x10056850
    const TCHAR* GetActionIconName(INT ActionID);                    // 0x10060c40 (C2: heap entries, FString at +0x28)
    const TCHAR* GetCastleName(INT CastleID);                        // 0x10056e80 castlename-*.dat
    const TCHAR* GetCastleLocation(INT CastleID);                    // 0x10056ed0 its 2nd text
    const TCHAR* GetCastleDescription(INT CastleID);                 // 0x10056f20 its 3rd text
    INT GetWeaponEnchantSteps(INT Enchant);                          // 0x100572f0 past +3: x2
    INT GetArmorEnchantSteps(INT Enchant);                           // 0x10057300 past +3: x3
    const TCHAR* GetIconName(INT ItemID);                            // 0x1006aa90 (guard name)
    UBOOL HasSelectedTarget();                                       // 0x10060c00 (shortcut keys)
    void FormatDateTime(INT Time, TCHAR* Out);                       // 0x1006dd10 "yyyy/mm/dd hh:mm"
                                                                     //   (the siege windows)
    void MakeSystemMessage(L2ParamStack& Params, TCHAR* Out);        // 0x1006d4c0 (NConsoleWndText.cpp)
                                                                     //   systemmsg id, count, then
                                                                     //   (type, value) each

    // Declared by the windows that use them. Decompiled: the requests to the server
    // (NConsoleWndNet.cpp) and the data getters (NConsoleWndData.cpp); the rest in the other
    // NConsoleWnd*.cpp files. Names ours unless a guard string gave them; the
    // Request* ones pass the call on to the engine's network handler (UL2NetHandler at Engine
    // +0x5C) and are named after the chat command that uses them.
    INT GetIMEType();                 // 0x10051bb0 kind of the keyboard layout's IME: 1 closed
                                      //   or none, 2..12 by layout (9, 10: Chinese), -1 other
    UBOOL SendChatMsgToServer(INT Type, TCHAR* Text, TCHAR* Target); // 0x10051f90 (guard name) 0
    void RequestTrade(INT CreatureID);                               // 0x100537e0 (guard name)
    User* GetSelectedActor();                                        // 0x10060b10 (guard name) target
    User* GetMyUser();                                               // 0x10060ac0 the player
    User* FindUserByName(TCHAR* Name);                               // 0x100566c0
    FVector GetMyLocation();                                         // 0x10057dd0 (C2: anonymous, the C1 twin was 0x10038540) where the player's pawn stands
    // the player's pawn and the macros (C2 only; names ours): the macro list is the macro window's (NCConsole +0xF8)
    APawn* GetMyPawn();                                              // 0x10060af0 the first viewport's controller's pawn (no checks)
    __declspec(noinline) INT GetMacroCount();                                             // 0x10056a20 0 without the macro window
    __declspec(noinline) NMacroEntry* GetMacro(INT Index);                                // 0x10060ce0 NULL out of 0..GetMacroCount
    UBOOL IsAnyMacroRunning();                                       // 0x10060d10 a macro with bRunning set
    TCHAR* GetMacroName(INT MacroID);                                // 0x10060d70 the name of the macro with that id, NULL when none
    INT GetMacroIconType(INT MacroID);                               // 0x10060dd0 its icon number, -1 when none
    void StartMacro(INT MacroID);                                    // 0x10061cc0 sets bRunning of the macro with that id
    UBOOL AddMacro(NMacroEntry* Macro);                              // 0x100569e0 into the macro window's list; 0 when there is none
    INT GetHennaSlotCount();                                         // 0x10060e90 1..3 by the profession tier of the class (-1: no player)
    UBOOL HasItemAdditionalName(INT ItemID);                         // 0x10057730 the item's additional name is not "NAME_None"
    const TCHAR* GetItemAdditionalName(INT ItemID);                  // 0x100578a0
    TCHAR* GetPartyMemberName(INT Index);                            // 0x10052c50 the party window's member, NULL out of range
    UBOOL GetMousePos(INT* X, INT* Y);                               // 0x10056130 the window manager's (tail call)
    void RequestGMSnoopEnd(INT SnoopID);                             // 0x10055c10 NetHandler slot 251 (a tail call)
    void StartGameGuard(INT hWnd);                                   // 0x100574e0 (name ours) the game's side of the nProtect GameGuard start-up
    void ScaleRect(INT W0, INT H0, INT W1, INT H1, NResizeRect* Rect); // 0x10056500 the place a window keeps in proportion when the desktop goes from W0 x H0 to W1 x H1
    void ShowChatText(const TCHAR* Text, DWORD Color);               // 0x100560f0 the desktop's AddSystemText with sub type 5
    void GetNumberMessage(INT MsgID, INT Number, TCHAR* Out);        // 0x10055e20 the text of a system message with one number
    void ShowSystemMessage(INT ID, DWORD Color);                     // 0x1006a8c0 systemmsg line (Color is not read in C2)
    __declspec(noinline) void PlaySoundName(const TCHAR* Name);                           // 0x100578d0 (name ours) the interface sound of a name
    void PlayInterfaceSound(INT SoundID);                            // 0x10061d20 (name ours) the sound of a number: the table
                                                                     //   of names at 0x1016a0a0 (0 click_1 ... 10 petition)
    void ShowDialog(NDialogInfo* Info);                              // 0x1005b310
    void ExecCommand(TCHAR* Command);                                // 0x1005b2e0 viewport Exec
    void RequestPetition(TCHAR* Text, INT Type);                     // 0x10055390 (C2: Type is new, passed on: name ours)
    void RequestPetitionCancel();                                    // 0x100553a0
    void RequestGMList();                                            // 0x100553b0
    void RequestSocialAction(INT Action);                            // 0x10054750
    void RequestChangeMoveType(UBOOL bRun);                          // 0x10054770
    void RequestChangeWaitType(UBOOL bStand);                        // 0x10054760
    void RequestAttack(INT ObjectID, FVector Origin, INT bForceAttack = 0, INT bShift = 0); // 0x10059f20 (C2: two more words, ret 0x18)
    void RequestAttackForce(INT ObjectID, FVector Origin);           // 0x100546d0
    void RequestAttackStand(INT ObjectID, FVector Origin);           // 0x10054710
    void RequestTarget(INT ObjectID);                                // 0x10059be0
    void RequestTargetNext(INT Range, INT CurrentID);                // 0x10059cf0
    void RequestPickUp(INT Range, INT ObjectID);                     // 0x10059c60
    void RequestAssist();                                            // 0x10067420
    void RequestPartyInvite(L2ParamStack& Params);                   // 0x10054d70
    void RequestPartyLeave(L2ParamStack& Params);                    // 0x10054d90
    void RequestPartyDismiss(L2ParamStack& Params);                  // 0x10054da0
    void RequestVendor();                                            // 0x10055a20
    void RequestMagicSkillUse(L2ParamStack& Params);                 // 0x100543a0
    void RequestMakeMacro(L2ParamStack& Params);                     // 0x10054380 (C2 only; name ours) NetHandler slot 145
    void RequestDeleteMacro(L2ParamStack& Params);                   // 0x10054390 (C2 only; name ours) NetHandler slot 146
    void BypassUserCmd(L2ParamStack& Params);                        // 0x10056000 (C2 only; name ours) NetHandler slot 248
    void RequestRecipeBookOpen();                                     // 0x100543f0 (C2 only; name ours) NetHandler slot 199
    void RequestRecipeItemDelete(INT P1);                             // 0x10054400 (C2 only; name ours) NetHandler slot 200
    void RequestRecipeItemMakeInfo(INT P1);                           // 0x10054410 (C2 only; name ours) NetHandler slot 201
    void RequestRecipeItemMakeSelf(INT P1);                           // 0x10054420 (C2 only; name ours) NetHandler slot 202
    void RequestRecipeShopManageList();                               // 0x10054430 (C2 only; name ours) NetHandler slot 203
    void RequestRecipeShopMessageSet(TCHAR* P1);                      // 0x10054440 (C2 only; name ours) NetHandler slot 204
    void RequestRecipeShopListSet(L2ParamStack& P1);                  // 0x10054450 (C2 only; name ours) NetHandler slot 205
    void RequestRecipeShopManageQuit();                               // 0x10054460 (C2 only; name ours) NetHandler slot 206
    void RequestRecipeShopManageCancel();                             // 0x10054470 (C2 only; name ours) NetHandler slot 207
    void RequestRecipeShopSellList(INT P1);                           // 0x10054480 (C2 only; name ours) NetHandler slot 208
    void RequestRecipeShopMakeInfo(INT P1, INT P2);                   // 0x10054490 (C2 only; name ours) NetHandler slot 209
    void RequestRecipeShopMakeDo(INT P1, INT P2);                     // 0x100544a0 (C2 only; name ours) NetHandler slot 210
    void RequestHennaItemInfo(INT P1);                                // 0x100544f0 (C2 only; name ours) NetHandler slot 211
    void RequestHennaEquip(INT P1);                                   // 0x10054500 (C2 only; name ours) NetHandler slot 212
    void RequestHennaUnequipInfo(INT P1);                             // 0x10054510 (C2 only; name ours) NetHandler slot 213
    void RequestHennaUnequip(INT P1);                                 // 0x10054520 (C2 only; name ours) NetHandler slot 214
    void UseShortcut(L2ParamStack& Params);                          // 0x10044860 (C1 only: no C2 match)
    void UseShortCutEquip(INT Index);                                // 0x10066fd0 (C2 only; name ours) the item of a shortcut slot, a weapon or armor not worn yet: put on
    void RequestAllyInvite(L2ParamStack& Params);                    // 0x10054aa0
    void RequestAllyInfo();                                          // 0x10054ad0
    void RequestFriendInvite(L2ParamStack& Params);                  // 0x10054c80
    void RequestFriendDel(L2ParamStack& Params);                     // 0x10054c90
    void RequestFriendList();                                        // 0x10054cb0
    void RequestBlock(L2ParamStack& Params);                         // 0x10054cc0
    void RequestBuy();                                               // 0x10055ac0
    void RequestDwarvenManufacture();                                // 0x10056010
    // used by the dialog box (NCDialogBox::OnOK / OnCancel)
    void ConvertNewLines(TCHAR* Text, FString& Result);              // 0x10056f70 escaped newlines
    UBOOL HasInvalidChars(TCHAR* Text);                              // 0x100563d0 empty, or a
                                                                     //   character the font lacks
    UBOOL HasBadWord(TCHAR* Text);                                   // 0x1004a190 (C1 only: no C2 match) word filter
    UBOOL IsValidName(TCHAR* Text);                                  // 0x1006d3f0
    void RegisterPledgeCrest(L2ParamStack& Params);                  // 0x1006c580 bmp file path
    void RegisterAllyCrest(L2ParamStack& Params);                    // 0x1006c830
    void SetPrivateStoreMsg(TCHAR* Text);                            // 0x10055a60
    void SetPrivateStoreBuyMsg(TCHAR* Text);                         // 0x10055b00
    void RequestChangePetName(TCHAR* Name);                          // 0x10055290
    void RequestAnswerJoinParty(L2ParamStack& Params);               // 0x10054d80
    void RequestAnswerJoinPledge(L2ParamStack& Params);              // 0x10054800
    void RequestAnswerJoinAlly(L2ParamStack& Params);                // 0x10054ab0
    void RequestAnswerFriendInvite(L2ParamStack& Params);            // 0x10054ca0
    void RequestGiveNickName(L2ParamStack& Params);                  // 0x10054890 member title
    void RequestOustPledgeMember(L2ParamStack& Params);              // 0x10054810
    void RequestWithdrawalPledge();                                  // 0x100548b0
    void RequestStartPledgeWar(L2ParamStack& Params);                // 0x10054830
    void RequestReplyStartPledgeWar(L2ParamStack& Params);           // 0x10054840
    void RequestStopPledgeWar(L2ParamStack& Params);                 // 0x10054850
    void RequestReplyStopPledgeWar(L2ParamStack& Params);            // 0x10054860
    void RequestSurrenderPledgeWar(L2ParamStack& Params);            // 0x10054870
    void RequestSurrenderPersonally(L2ParamStack& Params);           // 0x10054880
    void RequestOustAlly(L2ParamStack& Params);                      // 0x10054ac0
    void RequestWithdrawAlly();                                      // 0x10054ae0
    void RequestDismissAlly();                                       // 0x10054af0
    void RequestActionUse(L2ParamStack& Params);                     // 0x100546c0
    void RequestNet103(L2ParamStack& Params);                        // 0x100547f0 (dialog 0xB,
    void RequestNet106();                                            // 0x10054820  0xC: opened
                                                                     //   nowhere in NWindow)
    // used by the login screens (NCAuthWnd, NCLoginWnd, NCServerSelectWnd)
    UBOOL RequestAuthLogin(const TCHAR* Account, const TCHAR* Password); // 0x10052080 (guard name)
    UBOOL RequestAuthCardKeyLogin(INT CardID, INT Key);              // 0x10052110 (guard name; C2 only) NetHandler slot 68 (RequestSCCheck)
    void RequestServerLogin(L2ParamStack& Params);                   // 0x100526c0 (guard name)
    // ShowMessageWnd (the message window) is a virtual in C2: slot 317
    void ToggleOptionWnd();                                          // 0x100460c0 (C1 only: no C2 match) +0x36C4
    // used by the clan and party matching windows
    UTexture* GetPledgeCrestTexture(INT PledgeID);                   // 0x10056830 16x12 crest
    void RequestJoinPledge(L2ParamStack& Params);                    // 0x100548a0 invitation
    void RequestPartyMatchConfig(L2ParamStack& Params);              // 0x10054db0
    void RequestPartyMatchDetail(INT ObjectID);                      // 0x10054dd0
    const TCHAR* GetClassNameString(INT ClassID);                    // 0x10057090 SysString[247 + ID]
    // used by the menus and the option windows
    void RequestRestartPoint(L2ParamStack& Params);                  // 0x10052550 restart menu
                                                                     //   choice (0 village .. 4 fixed)
    void RequestRestart(L2ParamStack& Params);                       // 0x10052540 back to the
                                                                     //   character selection
    // RequestLogout (quit the game) is a virtual in C2: slot 315
    // C1 virtuals (slots 269 and 270) with no function in C2 (the pad of the shortcut bar is gone)
    void SetPadShift1(BYTE bShift);                                  // 0x10047b50 (C1 only: no C2 match) ShortCutWnd->bPadShift1
    void SetPadShift2(BYTE bShift);                                  // 0x10047b70 (C1 only: no C2 match) ShortCutWnd->bPadShift2
    void ResetWindowPositions();                                     // 0x1005f9b0 every window back
                                                                     //   to its default place
    void ReloadGameTexts();                                          // 0x100565f0 after the item /
                                                                     //   skill texts were reloaded
    // used by the status windows (player, target, party, buffs)
    void TargetMyself();                                             // 0x10059b50 click on the
                                                                     //   player window
    AActor* GetSelectedStaticActor();                                // 0x10060bc0 door or other
                                                                     //   static target
    FServerStaticObject* FindStaticObjectInfo(INT CreatureID);       // 0x100566e0
    PetInfo GetPetInfo();                                            // 0x100566a0
    BYTE IsSiegeNpc(INT NpcID);                                      // 0x10057320 castle guards,
                                                                     //   teleporters, mercenaries
    PledgeInfo* GetPledgeInfo(INT PledgeID);                         // 0x10056820
    void RequestTargetCancel(L2ParamStack& Params);                  // 0x100546b0
    void AssistMember(INT ObjectID, FVector Origin);                 // 0x10059d80 targets what
                                                                     //   the member targets
    INT GetSkillRange(INT SkillID, INT Level);                       // 0x100569b0 skill data +0x2C
    TCHAR* GetSkillName(INT SkillID, INT Level);                     // 0x10056880
    TCHAR* GetSkillDesc(INT SkillID, INT Level);                     // 0x100568b0
    INT GetServerID();                                               // 0x10056680 NetHandler slot
                                                                     //   30 (GM status: crest names)
    // used by the pet, summon and minimap windows
    void RequestAction(INT ObjectID, FVector Origin);                // 0x10059ec0 net handler
                                                                     //   slot 53 (click on it)
    void LockCursor();                                               // 0x10056140
    void UnlockCursor();                                             // 0x10056150
    // used by the action window
    void RequestPledgeMemberList();                                  // 0x100547e0 NetHandler slot
                                                                     //   102 (shown with a clan)
    void RequestShortCutReg(L2ParamStack& Params);                   // 0x10044830 (C1 only: no C2 match) (type, slot,
                                                                     //   id...) + the drop sound
    void PetPickUp(INT Range, INT Param);                            // 0x10055220 nearest item
                                                                     //   (slot 210) to slot 189
    // used by the character selection and creation (NCLobbyWnd, NCPawnSelectWnd,
    // NCPawnCreateWnd) and the announcements (NCNoticeWnd)
    UBOOL RequestGameStart(INT Index);                               // 0x100521b0 (guard name)
    UBOOL RequestCharacterDelete(INT Index);                         // 0x100522a0 (guard name)
    UBOOL RequestCharacterRestore(INT Index);                        // 0x10052350 (guard name)
    void RequestNewCharacter();                                      // 0x10052400 (guard name)
    void ReturnToLogin();                                            // 0x10056030 "Previous" on
                                                                     //   the character selection
    void ReturnToCharacterSelect();                                  // 0x100560b0 creation cancelled
    void ShowWarningWnd();                                           // 0x10052560 after the
                                                                     //   announcements, when any
    void FormatSystemMessage(TCHAR* Out, const TCHAR* Format, const TCHAR* Param1,
                             const TCHAR* Param2);                   // 0x10046120 $s1 / $s2
    UBOOL RequestCharacterCreate(CharacterInfo Info);                // 0x10052480 (guard name)
    // used by the quest and item windows
    const TCHAR* GetQuestText(INT QuestID, INT Step);                // 0x10054230 (name ours; it
                                                                     //   uses AddQuestID's guard)
    void GetItemSlotText(INT Type, INT BodyPart, FString& Result, INT SubType); // 0x10056b30 the
                                                                     //   slot line of a tool tip
    // used by the shortcut bar and the skill windows
    void RequestUseItem(INT ObjectID);                               // 0x10053440 (guard name)
    void RequestPetUseItem(INT ObjectID);                            // 0x10055260 NetHandler slot 190
    void RequestShortCutDel(L2ParamStack& Params);                   // 0x100545e0 (slot)
    void RequestSkillList(L2ParamStack& Params);                     // 0x100543b0 NetHandler slot 158
    const TCHAR* GetItemName(INT ItemID);                            // 0x1006ab40 (guard name)
    const TCHAR* GetActionName(INT ActionID);                        // 0x10060c90 (C2: FString at +0x1C) actionname-*.dat
    UBOOL IsToggleSkill(INT SkillID, INT Level);                     // 0x100568e0 skill data +0x14 == 3
    UBOOL HasAbnormal(INT SkillID);                                  // 0x100611c0 the skill's effect
                                                                     //   is on the player (buffs)
    UBOOL IsShotItem(INT ItemID);                                    // 0x100573d0 soul / spirit shots
    INT GetSkillMpCost(INT SkillID, INT Level);                      // 0x10056980 skill data +0x1C
    const TCHAR* GetSkillTypeText(INT SkillID, INT Level);           // 0x10056910 "Active Skill",
                                                                     //   "Passive Skill" or "Magic"
    // used by the HTML viewer
    UBOOL HasNoAmpersand(TCHAR* Text);                               // 0x10056430 0 when Text
                                                                     //   holds a '&'
    void RequestTutorialQuestionMark(INT ID);                        // 0x100551b0 the tutorial
                                                                     //   question mark clicked
    // used by the desktop (NCConsole)
    UBOOL MoveToLocation(FVector Loc, UBOOL bNoZ);                   // 0x10061130 NetHandler slot 90
    User* FindUserByID(INT ObjectID);                                // 0x100566d0 NetHandler slot 33
    void RequestItemList();                                          // 0x100533c0 (guard name)
    void RequestPartyMatchList(INT Status);                          // 0x10054dc0 3: leaves the room
    void RequestLinkHtml(L2ParamStack& Params);                      // 0x10055050 HTML "Link"
    void RequestBBSWrite(L2ParamStack& Params);                      // 0x10055070 HTML "Write"
    void RequestShowBoard(L2ParamStack& Params);                     // 0x10055080 community board
    void RequestTutorialLinkHtml(L2ParamStack& Params);              // 0x10055190
    void RequestPrivateStoreQuitSell();                              // 0x10055a50
    void RequestPrivateStoreQuitBuy();                               // 0x10055af0
    void RequestGMInventory(TCHAR* Name);                            // 0x100553c0 GM command 5
    void RequestGMSkills(TCHAR* Name);                               // 0x100553e0 GM command 3
    void RequestGMStatus(TCHAR* Name);                               // 0x10055400 GM command 1
    void RequestGMQuests(TCHAR* Name);                               // 0x10055420 GM command 4
    void RequestGMWarehouse(TCHAR* Name);                            // 0x10055440 GM command 6
    void RequestGMPledge(TCHAR* Name);                               // 0x10055460 GM command 2
    // used by the trade window and the pet's inventory
    void RequestTradeDone(INT bConfirm);                             // 0x100538f0 (guard name) OK 1,
                                                                     //   cancel 0
    void RequestGetItemFromPet(L2ParamStack& Params);                // 0x10055280 NetHandler slot
                                                                     //   192 (object id, amount, 0)
    // used by the inventory (NCInventoryWnd, NCInvenItemWnd, NCEquipItemWnd)
    void RequestUnEquipItem(L2ParamStack& Params);                   // 0x100534c0 body part bits
    void RequestGiveItemToPet(L2ParamStack& Params);                 // 0x10055270 object id, amount
    AActor* GetTargetedActor(UBOOL bCheck);                          // 0x100566f0 (guard name) the
                                                                     //   pawn under the mouse
    INT GetEquipSlot(INT BodyPart, INT ObjectID);                    // 0x10060f10 paperdoll slot,
                                                                     //   14 / 15: two-slot items
    const TCHAR* GetGradeText(INT Grade);                            // 0x100570b0 crystal grade
    UBOOL IsPetCollar(INT ItemID);                                   // 0x10046880 (C1 only: no C2 match) 2375, 3500..3502
    UBOOL IsMagicArmor(INT ItemID);                                  // 0x1006abf0 armor type 3 (MP)
    const TCHAR* GetWeaponTypeName(INT Type);                        // 0x10056a40 sysstring 43..504
    const TCHAR* GetAttackSpeedText(INT Speed);                      // 0x10056e00 sysstring 306..310
    INT GetEnchantedDefense(INT Grade, INT Enchant, INT Base);       // 0x10060e60 base + bonus
    INT GetEnchantedAttack(INT Grade, INT Enchant, INT Base);        // 0x10060e30 (weapons)
    // used by the shop and warehouse windows (NCShopWnd, NCStoreWnd): (id, count) lists
    void RequestBuyItem(L2ParamStack& Params);                       // 0x10053c50 (guard name)
    void RequestBuySeed(L2ParamStack& Params);                       // 0x10053a20 (guard name; C2 only) NetHandler slot 90
    void RequestProcureCrop(L2ParamStack& Params);                   // 0x10053aa0 (guard name; C2 only) NetHandler slot 91
    void RequestSellItem(L2ParamStack& Params);                      // 0x10053cd0 (guard name)
    void RequestWarehouseDeposit(L2ParamStack& Params);              // 0x10053e80 NetHandler slot 138
    void RequestWarehouseWithdraw(L2ParamStack& Params);             // 0x10053e90 NetHandler slot 139
    // used by the private stores (names ours, after the packets; the quit requests, NetHandler
    // slots 198 and 205, are with the desktop's above)
    void RequestSetPrivateStoreListSell(L2ParamStack& Params);       // 0x10055a30 NetHandler slot
                                                                     //   196: count, then object id,
                                                                     //   count and price of each
    void RequestPrivateStoreManageCancel();                          // 0x10055a40 NetHandler slot 197
    void RequestPrivateStoreBuy(L2ParamStack& Params);               // 0x10055a70 NetHandler slot
                                                                     //   201: the owner's object id,
                                                                     //   then as above
    void RequestSetPrivateStoreListBuy(L2ParamStack& Params);        // 0x10055ad0 NetHandler slot
                                                                     //   203: count, then item id,
                                                                     //   enchant, count and price
    void RequestPrivateStoreBuyManageCancel();                       // 0x10055ae0 NetHandler slot 204
    void RequestPrivateStoreSell(L2ParamStack& Params);              // 0x10055b10 NetHandler slot
                                                                     //   208: the owner's object id,
                                                                     //   count, then object id and
                                                                     //   the above for each
    // the console's own window management (names ours unless a guard string gave them)
    UBOOL Initialize(HWND hWnd);                                     // 0x10061e10 every window
    void LoadWindowPositions();                                      // 0x1005c800 WindowsInfo.ini
    void SaveWindowPositions();                                      // 0x1005b7c0 WindowsInfo.ini
    void Finalize();                                                 // 0x10062d30
    UBOOL InitializePawnViewer(HWND hWnd);                           // 0x100613a0
    INT NSetTimer(UINT ID, UINT Elapse, NCObject* Owner);            // 0x10063220 (NCObject::SetTimer)
    UBOOL NKillTimer(UINT ID, NCObject* Owner);                      // 0x10058020
    void SetCursor(INT Type, UTexture* Texture);                     // 0x10056110 unless locked
    void UpdateCursor();                                             // 0x1006ad10 every frame
    void UpdateTargetWindow();                                       // 0x1006a9d0 every tick
    INT DispatchWndMsgX(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam); // 0x1006b840
    UBOOL HookShortCutKey(UINT Msg, WPARAM wParam, LPARAM lParam);   // 0x1004f050 (C1 only: no C2 match)
    // the console's own text services and actions (names ours)
    void RemoveInvalidChars(TCHAR* Text);                            // 0x10056470 the characters
                                                                     //   the font lacks
    AActor* GetPickupUnderMouse();                                   // 0x100567e0 an AL2Pickup
    void UseAction(INT ActionID, INT bForce = 0, INT bShift = 0);    // 0x1006b0b0 an action slot (C2: ret 0xC: Ctrl and Shift)
    void UseShortCut(INT Slot, INT bForce = 0, INT bShift = 0);      // 0x1006c210 (C2: ret 0xC) a shortcut slot
                                                                     //   (the keys, slot 263)

    // UObject's and FExec's
    virtual void Destroy();                                             // 10 0x1006b090
    virtual UBOOL Exec(const TCHAR* Cmd, FOutputDevice& Ar) { return 0; } // FExec 0x100016f0

    // UL2ConsoleWnd's, implemented here (called by the engine), in the order of the C2 vtable: each slot has the
    // name of include/ue2/gen/UL2ConsoleWnd.vt.inc (tools/ue2_data/l2consolewnd_slots_c2.csv says how it was
    // found). The UGameEngine::On* handler of FL2NetNotify that calls a slot names it (without "On") unless a
    // guard string does; names ours otherwise (vfnNNN: not known). The slots only C2 has are in NConsoleWndC2.cpp
    // (the manor's in NConsoleWndItems.cpp, 314 in NConsoleWndResize.cpp).
    virtual void Tick(FLOAT DeltaTime);                                 // 25 0x1006bc50
    // WndMain->IsOverWindow
    virtual UBOOL IsOverWindow(INT X, INT Y);                           // 27 0x10051da0
    virtual void ShowWarningDialog(INT P1, INT P2);                                         // 28 0x10061c80 (name ours)
    virtual void GameGuardQuery(INT P1, INT P2, INT P3, INT P4);        // 29 0x10057aa0
    virtual void AuthLoginSuccess() {}                                  // 30 0x100ab5e0
    virtual void SCCheckStart(INT P1, INT P2);                                         // 31 0x10058760 (name ours)
    virtual void AuthLoginFail(INT Reason);                             // 32 0x10058410
    // the announcements (NCNoticeWnd)
    virtual void StartNotice(L2ParamStack& Params);                     // 33 0x10058780
    virtual void CharacterCreateFail(INT Reason);                       // 34 0x10052980
    virtual void ReceiveCharacterInfo();                                // 35 0x10052a80
    virtual void AddCharacterInfo(User Info);                           // 36 0x10052b60
    virtual void AcceptGameStart(INT Result);                           // 37 0x10058a30
    virtual void AcceptCharacterDelete(INT Result);                     // 38 0x10052ca0
    virtual void AcceptNewCharacter(INT Result);                        // 39 0x10052d60
    virtual void AcceptCharacterCreate(INT Result);                     // 40 0x10052e50
    // the same body as QuitGame (321)
    virtual void AcceptLogOut();                                        // 41 0x10063bf0
    virtual void ServerClose(INT Reason);                               // 42 0x10063c00
    virtual void ReviveRequest(INT Param1, INT Param2);                 // 43 0x10063d40
    // OnStartServerList
    virtual void ReceiveServerList(INT LastServerID);                   // 44 0x10052740
    virtual void AddServerList(L2ParamStack& Params);                   // 45 0x10052870
    virtual void EndServerList();                                       // 46 0x10052900

    // the target, the inventory, the trade, the NPC shops and the warehouse
    virtual void MyTargetSelected(INT LevelDiff);                       // 47 0x10059ff0
    virtual void ReceiveItemList(INT bShow);                            // 48 0x100534d0
    virtual void ReceiveUpdateItemList();                               // 49 0x10053580 no handler
    virtual void ReceiveTradeStart(INT ObjectID);                       // 50 0x10064580
    // the engine's ItemInfo, by value
    virtual void ReceiveAddTradeInventoryItem(NItemRequest Item);       // 51 0x10064690
    virtual void ReceiveAddTradeOwnItem(NItemRequest Item);             // 52 0x10064990
    virtual void ReceiveAddTradeOtherItem(NItemRequest Item);           // 53 0x10064c90
    virtual void ReceiveTradeUpdate(INT Change, ItemInfo& Item);        // 54 0x10064f90
    virtual void ReceiveTradeDoneMsg(INT Result);                       // 55 0x10053980
    virtual void AddRecommandedData(RecommandedDataStr Data);           // 56 0x100532b0
    virtual void EndRecommandedData();                                  // 57 0x10053340
    virtual void AddInventoryItem(ItemInfo& Item);                      // 58 0x10063e00
    // OnReceiveUpdateItemList
    virtual void UpdateInventoryItem(INT Change, ItemInfo& Item);       // 59 0x10064120
    virtual void CheckGeo();                                            // 60 0x10053600 (no argument: the stock pops nothing)
    virtual void StartBuyList(INT Adena, INT ListID);                   // 61 0x10053d50 NPC shop
    virtual void AddBuyList(ItemInfo& Item);                            // 62 0x10065870
    virtual void StartSellList(INT Adena, INT ListID);                  // 63 0x10053dd0
    virtual void AddSellList(ItemInfo& Item);                           // 64 0x10065b60
    virtual void StartManorBuyList(INT Adena, INT ListID);              // 65 0x10053b20
    virtual void AddManorBuyList(ItemInfo& Item);                       // 66 0x10065290
    virtual void StartManorSellList(INT Adena, INT ListID);             // 67 0x10053ba0
    virtual void AddManorSellList(ItemInfo& Item);                      // 68 0x10065580
    virtual void StartDepositList(INT Adena);                           // 69 0x10053ea0 warehouse
    virtual void AddDepositList(ItemInfo& Item);                        // 70 0x10065e50
    virtual void StartWithdrawList(INT Adena);                          // 71 0x10053f40
    virtual void AddWithdrawList(ItemInfo& Item);                       // 72 0x10066140
    // OnReceiveWareHouseDonePacket
    virtual void ReceiveWareHouseDone(L2ParamStack& Params);            // 73 0x1006c1e0
    virtual void UpdatePlayerInfo(L2ParamStack& Params);                // 74 0x10055fa0

    // HTML pages, the tutorial, the radar
    virtual void NpcHtmlMessage(L2ParamStack& Params);                  // 75 0x100681f0
    virtual void TutorialShowHtml(TCHAR* Html);                         // 76 0x10055090
    virtual void TutorialShowQuestionMark(INT ID);                      // 77 0x1005acd0
    virtual void TutorialEnableClientEvent(INT Events);                 // 78 0x100551f0
    virtual void TutorialCloseHtml();                                   // 79 0x10055200
    // CheckTutorialClientEvent; once per pending bit (+0x3724)
    virtual void SendTutorialEvent(INT Event);                          // 80 0x100551c0
    virtual void ShowRadar(INT Param1, INT Param2, INT X, INT Y, INT Z); // 81 0x10055800
    virtual void DeleteRadar(INT X, INT Y, INT Z);                      // 82 0x10055990

    // the clan, the alliance, the friends, the party
    // empty, folded
    virtual void ReceivePledgeExtendedInfo(PledgeInfo& Info);           // 83 0x10001ad0 (empty; traced in the standalone)
    virtual void ReceivePledgeInfo(PledgeInfo& Info);                   // 84 0x1005a0b0
    virtual void ReceivePledgeInfoUpdate(PledgeInfo& Info);             // 85 0x1005a320
    virtual void ReceivePledgeMemberList();                             // 86 0x100548c0
    virtual void AddPledgeMember(L2ParamStack& Params);                 // 87 0x1005a530
    virtual void UpdatePledgeMember(L2ParamStack& Params);              // 88 0x1005a630
    virtual void DeletePledgeMember(TCHAR* Name);                       // 89 0x10054950
    virtual void DeleteAllPledgeMember();                               // 90 0x100549e0
    virtual void ReceiveCreatePledge(L2ParamStack& Params);             // 91 0x1006cb20
    virtual void ReceiveAskJoinPledge(L2ParamStack& Params);            // 92 0x10067570
    virtual void ReceiveOustPledgeMember(L2ParamStack& Params);         // 93 0x1006cb60
    virtual void ReceiveDismissPledge(L2ParamStack& Params);            // 94 0x1006cba0
    virtual void ReceiveJoinPledge(L2ParamStack& Params);               // 95 0x1006caf0
    // OnReceiveWidthdrawalPledge
    virtual void ReceiveWithdrawalPledge(L2ParamStack& Params);         // 96 0x1006cbe0
    virtual void ReceiveOustPledgeMessage(L2ParamStack& Params);        // 97 0x1006cc20
    virtual void ReceiveDismissPledgeMessage(L2ParamStack& Params);     // 98 0x1006cc40
    virtual void ReceiveStartPledgeWar(L2ParamStack& Params);           // 99 0x10067680
    virtual void ReceiveReplyStartPledgeWar(L2ParamStack& Params);      // 100 0x1006cc80
    virtual void ReceiveStopPledgeWar(L2ParamStack& Params);            // 101 0x100677a0
    virtual void ReceiveReplyStopPledgeWar(L2ParamStack& Params);       // 102 0x1006ccc0
    virtual void ReceiveSurrenderPledgeWar(L2ParamStack& Params);       // 103 0x1006cd00
    virtual void ReceiveSurrenderPersonally(L2ParamStack& Params);      // 104 0x1006cd30
    virtual void ReceiveAskJoinParty(L2ParamStack& Params);             // 105 0x10068020
    virtual void ReceiveJoinParty(L2ParamStack& Params);                // 106 0x1006cde0
    virtual void ReceiveWithdrawalParty(L2ParamStack& Params);          // 107 0x10054de0
    virtual void ReceiveOustPartyMember(L2ParamStack& Params);          // 108 0x1006ce10
    virtual void ReceiveOustPartyMessage(L2ParamStack& Params);         // 109 0x1006ce40
    virtual void ReceiveDismissParty(L2ParamStack& Params);             // 110 0x10054e20
    // OnRecevieDismissPartyMessage
    virtual void ReceiveDismissPartyMessage(L2ParamStack& Params);      // 111 0x1006ce60
    virtual void ReceiveAskJoinAlly(L2ParamStack& Params);              // 112 0x100678c0
    virtual void ReceiveDismissAlly(L2ParamStack& Params);              // 113 0x10054b80
    virtual void ReceiveJoinAlly(L2ParamStack& Params);                 // 114 0x1006cd60
    virtual void ReceiveOustAlly(L2ParamStack& Params);                 // 115 0x10054b40
    virtual void ReceiveWithdrawAlly(L2ParamStack& Params);             // 116 0x10054b00
    virtual void ReceiveFriendAddRequestResult(L2ParamStack& Params);   // 117 0x1006cda0
    virtual void ReceiveFriendAddRequest(L2ParamStack& Params);         // 118 0x10067f60
    // empty, folded
    virtual void ReceiveFriendAdd(L2ParamStack& Params) {}              // 119 0x10001ad0
    // empty, folded
    virtual void ReceiveFriendRemove(L2ParamStack& Params) {}           // 120 0x10001ad0
    // empty, folded
    virtual void ReceiveFriendList(L2ParamStack& Params) {}             // 121 0x10001ad0
    // empty, folded
    virtual void ReceiveFriendStatus(L2ParamStack& Params) {}           // 122 0x10001ad0

    // new in C2 (recipes, hennas, the minimap, ...): nobody has named them yet, except the two UL2NetHandler forwarders
    virtual void ReceiveRecipeBookItemList(INT P1);                                         // 123 0x100590c0 (name ours)
    virtual void AddRecipeBookItem(INT RecipeID);                                         // 124 0x10066870 (name ours)
    virtual void ReceiveRecipeItemMakeInfo(INT P1, INT P2, INT P3, INT P4);                                         // 125 0x10059140 (name ours)
    virtual void ReceiveRecipeShopManageList(INT P1, INT P2);                                         // 126 0x10059220 (name ours)
    virtual void AddRecipeShopManageItem(INT RecipeID, INT P2);                                         // 127 0x100669a0 (name ours)
    virtual void AddRecipeShopRegisterdItem(INT RecipeID, INT P2, INT P3);                                         // 128 0x10066ae0 (name ours)
    virtual void ReceiveRecipeShopSellList(INT P1, INT P2, INT P3, INT P4);                                         // 129 0x10059270 (name ours)
    virtual void AddRecipeShopSellItem(INT RecipeID, INT P2, INT P3);                                         // 130 0x10066c30 (name ours)
    virtual void ReceiveRecipeShopItemInfo(INT P1, INT P2, INT P3, INT P4, INT P5);                                         // 131 0x10059330 (name ours)
    virtual void ReceiveMacroListInfo(char P1, char P2, MacroInfo Macro, char P4, INT P5);                                         // 132 0x10066d70 (name ours)
    virtual void ReceiveHennaEquipList(INT P1, INT P2);                                         // 133 0x10059440 (name ours)
    virtual void AddHennaEquipInfo(HennaInfo Info);                                         // 134 0x100594f0 (name ours)
    virtual void ReceiveHennaItemInfo(HennaInfo Info, INT P2);                                         // 135 0x10059610 (name ours)
    virtual void ReceiveHennaInfo(HennaInfo Info, INT P2);                                         // 136 0x100596e0 (name ours)
    virtual void AddHennaInfo(INT P1, INT P2);                                         // 137 0x100544b0 (name ours)
    virtual void ReceiveHennaUnequipList(INT P1, INT P2);                                         // 138 0x10059720 (name ours)
    virtual void AddHennaUnequipInfo(HennaInfo Info);                                         // 139 0x100597d0 (name ours)
    virtual void ReceiveHennaUnequipItemInfo(HennaInfo Info, INT P2);                                         // 140 0x10059930 (name ours)
    virtual void RequestHennaItemList();                                // 141 0x10054530
    virtual void RequestHennaUnequipList();                             // 142 0x10054540
    virtual void GMAddHennaInfo(INT P1, INT P2);                                         // 143 0x100544d0 (name ours)
    virtual void GMReceiveHennaInfo(HennaInfo Info, INT P2);                                         // 144 0x100598f0 (name ours)
    virtual void ReceiveBlockList(L2ParamStack& Params) {}              // 145 0x10001ad0

    // the inventory and character toggles, the shortcut keys and the Ctrl cursor (C1 slots 261-270 moved here)
    virtual void ToggleInventory();                                     // 146 0x10054550
    // no handler; Desktop->ToggleCharacter(0)
    virtual void ShowCharacterStatus();                                 // 147 0x10054590
    virtual void UseShortCutSlot(INT Index);                            // 148 0x1006dce0
    virtual void ChangeShortCutPage(INT bNext);                         // 149 0x100545a0
    // no handler; the pawn after (or before) CreatureID among the pawns in an area (ULevel slot 57 in C2)
    virtual User* FindNextPawnInArea(FLOAT Radius, INT CreatureID, INT bNext, INT P4, INT P5, INT P6, INT P7); // 150 0x10061a00
    // the desktop's view rectangle (NCConsole +0x1C0..0x1CC in C2: left, top, width, height)
    virtual void SetViewRect(INT Left, INT Top, INT Right, INT Bottom); // 151 0x10061ba0
    // bCtrlDown = 1
    virtual void PressCtrl();                                           // 152 0x10056160
    // bCtrlDown = 0
    virtual void ReleaseCtrl();                                         // 153 0x10056190

    // shortcuts, buffs, quests, chat, party, skills, system messages
    virtual void ReceiveShortCutList();                                 // 154 0x100545f0
    virtual void AddShortCutItem(L2ParamStack& Params);                 // 155 0x100670b0
    virtual void UpdateShortCutItem(INT ObjectID);                      // 156 0x10054630
    virtual void UpdatePetShortCutItem(INT ObjectID);                   // 157 0x10054650
    virtual void ReceiveShortCutDelete(INT Slot);                       // 158 0x10054670
    virtual void UpdateMacroShortCut(INT P1, INT P2);                                         // 159 0x10054610 (name ours)
    virtual void ReceiveMagicSkillList(L2ParamStack& Params);           // 160 0x100543c0
    virtual void ReceiveAbnormalStatus();                               // 161 0x10054690
    virtual void AddAbnormalStatus(L2ParamStack& Params);               // 162 0x10059a00
    virtual void ReceiveQuestList();                                    // 163 0x100540d0
    virtual void AddQuestID(INT QuestID, INT Step);                     // 164 0x10054160
    virtual void AddQuestItem(ItemInfo& Item);                          // 165 0x10066430
    // a chat line
    virtual UBOOL Say2(L2ParamStack& Params);                           // 166 0x1006a790
    // no handler; the debug window
    virtual void VARARGS DebugLogf(INT Category, const TCHAR* Fmt, ...); // 167 0x10055dc0
    virtual void ReceiveVersionCheck(L2ParamStack& Params);             // 168 0x10052f80
    virtual void UpdateEquipItem(L2ParamStack& Params);                 // 169 0x10053720
    // OnPartyAddMemberPacket
    virtual void PartyAddMember(L2ParamStack& Params);                  // 170 0x10054e60
    virtual void PartyUpdateMember(L2ParamStack& Params);               // 171 0x10054f00
    virtual void PartyDeleteMember(L2ParamStack& Params);               // 172 0x10054f80
    virtual void PartyDeleteAll(L2ParamStack& Params);                  // 173 0x10054fc0
    virtual void ReceivePartyMatchList();                               // 174 0x10054ff0
    virtual void AddPartyMatchMember(L2ParamStack& Params);             // 175 0x10055010
    virtual void ReceivePartyMatchDetail(L2ParamStack& Params);         // 176 0x10055030
    // empty, folded
    virtual void ShowMagicWnd() {}                                      // 177 0x100ab5e0
    virtual void ShowSkillWnd();                                        // 178 0x100543d0
    // OnSkillListPacket
    virtual void ReceiveSkillList(L2ParamStack& Params);                // 179 0x10058f80
    // OnMagicListPacket; empty, folded
    virtual void ReceiveMagicList(L2ParamStack& Params) {}              // 180 0x10001ad0
    virtual void ReceiveTradeDemand(L2ParamStack& Params);              // 181 0x10064470
    // OnRestartResponse, first
    virtual void RestartResetWindows();                                 // 182 0x10053010
    // OnRestartResponse, then
    virtual void RestartToLobby();                                      // 183 0x10053070
    virtual void AddSystemMessageParam(L2ParamStack& Params);           // 184 0x1006ce80
    virtual void EndSystemMessageParam(L2ParamStack& Params, INT P2, INT P3); // 185 0x1006cf70

    // board, enchant, skill trainer, restart, VIP shop, chat channels, pets
    // OnShowboardPacket
    virtual void ShowBoard(L2ParamStack& Params);                       // 186 0x100550d0
    // NetHandler 164
    virtual void BuilderCommand(L2ParamStack& Params);                  // 187 0x10056020
    virtual void SelectItemToEnchant();                                 // 188 0x10053620
    virtual void ReceiveEnchantResult(INT Result);                      // 189 0x100536d0
    virtual void ReceiveAction(INT Action);                             // 190 0x10067500
    virtual void ShowTrainListWnd();                                    // 191 0x10054300
    virtual void ShowTrainInfoWnd(L2ParamStack& Params);                // 192 0x1006da70
    virtual void AddAcquireSkillInfo(L2ParamStack& Params);             // 193 0x10058db0
    virtual void AddAcquireSkillExtendedInfo(L2ParamStack& Params);     // 194 0x100665d0
    // no handler; the restart points after dying
    virtual void ShowRestartMenu(L2ParamStack& Params);                 // 195 0x10058be0
    virtual void TeleportToLocation();                                  // 196 0x100532a0
    virtual void HideTrainWnd();                                        // 197 0x10054360
    virtual void SetVIPShopID(INT ShopID);                              // 198 0x10055b50
    virtual void AddVIPItemList(INT ProductID, INT ItemID, INT BodyPart, INT Type, INT Count); // 199 0x1006a4c0
    virtual void AddVIPNeedItemList(INT ProductID, INT ItemID, INT Count); // 200 0x1006a6d0
    virtual void VIPItemListEnd();                                      // 201 0x10055b70
    // NetHandler 186
    virtual void RequestBuyVIPItem(L2ParamStack& Params);               // 202 0x10055ba0
    virtual void EnablePartyChat(bool bEnable);                         // 203 0x10055bb0
    virtual void EnablePledgeChat(bool bEnable);                        // 204 0x10055bd0
    virtual void EnableAllyChat(bool bEnable);                          // 205 0x10055bf0
    virtual void ShowPetStatusWnd(INT Type);                            // 206 0x10068240
    virtual void ShowPetPartyWnd(INT Type);                             // 207 0x100552a0
    virtual void HideSummonWnd();                                       // 208 0x100682b0 no handler
    virtual void HidePetWnd();                                          // 209 0x10068340
    virtual void ShowPetNameBtn(INT bShow);                             // 210 0x10055330
    virtual void ReceivePetStatusInfo(INT Type, L2ParamStack& Params);  // 211 0x10055350
    virtual void ReceivePetItemList();                                  // 212 0x10055370
    virtual void AddPetInventoryItem(ItemInfo& Item);                   // 213 0x100683d0
    virtual void UpdatePetInventoryItem(INT Change, ItemInfo& Item);    // 214 0x100686e0
    // OnSkillCoolTimePacket
    virtual void SkillCoolTime(INT Param1, INT Param2, INT Param3, INT Param4); // 215 0x10061210

    // alliance wars and sieges (the Request* pass the call on to the network handler)
    // NetHandler 174; no caller in NWindow
    virtual void vfn176(L2ParamStack& Params);                          // 216 0x10057310
    virtual void MacroSkillUse(INT P1, INT P2);                                         // 217 0x10054780 (name ours)
    virtual void MacroTargetSelected();                                         // 218 0x100547c0 (name ours)
    virtual void MacroItemPickedUp();                                         // 219 0x100547a0 (name ours)
    virtual void RequestStartAllianceWar(L2ParamStack& Params);         // 220 0x10054bc0
    // NetHandler 176
    virtual void RequestReplyStartAllianceWar(L2ParamStack& Params);    // 221 0x10054bd0
    // NetHandler 177
    virtual void RequestStopAllianceWar(L2ParamStack& Params);          // 222 0x10054be0
    // NetHandler 178
    virtual void RequestReplyStopAllianceWar(L2ParamStack& Params);     // 223 0x10054bf0
    // NetHandler 179
    virtual void RequestSurrenderAllianceWar(L2ParamStack& Params);     // 224 0x10054c00
    virtual void StartAllianceWar(TCHAR* AllianceName, TCHAR* Name);    // 225 0x100679e0
    virtual void StopAllianceWar(TCHAR* AllianceName, TCHAR* Name);     // 226 0x10067b00
    // NetHandler 180; no caller in NWindow
    virtual void vfn184(L2ParamStack& Params);                          // 227 0x10054cd0
    // NetHandler 181
    virtual void RequestSiegeAttackerList(L2ParamStack& Params);        // 228 0x10054ce0
    // NetHandler 182
    virtual void RequestSiegeDefenderList(L2ParamStack& Params);        // 229 0x10054cf0
    // NetHandler 183
    virtual void RequestJoinSiege(L2ParamStack& Params);                // 230 0x10054d00
    // NetHandler 184
    virtual void RequestConfirmSiegeWaitingList(L2ParamStack& Params);  // 231 0x10054d10
    // NetHandler 185
    virtual void RequestSetSiegeTime(L2ParamStack& Params);             // 232 0x10054d20
    virtual void ReceiveSiegeInfo(L2ParamStack& Params);                // 233 0x1005a730
    virtual void ReceiveSiegeAttackerList(L2ParamStack& Params);        // 234 0x1005a840
    virtual void ReceiveSiegeDefenderList(L2ParamStack& Params);        // 235 0x1005a930
    virtual void AddSelectableTime(INT Time);                           // 236 0x1005aa30
    virtual void EndSiegeAttackerList();                                // 237 0x1005aa50
    virtual void EndSiegeDefenderList();                                // 238 0x1005aa70
    virtual void ClearAttackerList();                                   // 239 0x10054d30
    virtual void ClearDefenderList();                                   // 240 0x10054d50

    // packages, GM views, minimap, private stores
    // NetHandler 187
    virtual void RequestPackageSend(L2ParamStack& Params);              // 241 0x10054c10
    // NetHandler 188
    virtual void RequestPackageSendableItemList(L2ParamStack& Params);  // 242 0x10054c20
    virtual void AddPackageToList(L2ParamStack& Params);                // 243 0x10054c30
    virtual void AddSendableItemList(INT Index, INT TargetID, INT Adena, ItemInfo& Item); // 244 0x10067c20
    virtual void GMViewCharacterInfo(User& Info);                       // 245 0x10055480
    virtual void GMViewReceivePledgeInfo(PledgeInfo& Info);             // 246 0x1005ad30
    // guard ReceivePledgeMemberList
    virtual void GMViewReceivePledgeMemberList();                       // 247 0x100554b0
    // guard AddPledgeMember
    virtual void GMViewAddPledgeMember(L2ParamStack& Params);           // 248 0x1005af80
    virtual void GMViewShowSkillWnd();                                  // 249 0x10055540
    // OnGMViewSkillListPacket
    virtual void GMViewSkillList(L2ParamStack& Params);                 // 250 0x1005b070
    // empty, folded
    virtual void GMViewShowMagicWnd() {}                                // 251 0x100ab5e0
    // OnGMViewMagicListPacket
    virtual void GMViewMagicList(L2ParamStack& Params);                 // 252 0x1005b1b0
    // guard ReceiveQuestList
    virtual void GMViewReceiveQuestList();                              // 253 0x10055560
    // guard AddQuestID
    virtual void GMViewAddQuestID(INT QuestID, INT Step);               // 254 0x100555e0
    // guard AddQuestItem
    virtual void GMViewAddQuestItem(ItemInfo& Item);                    // 255 0x10068a00
    // guard ReceiveItemList
    virtual void GMViewReceiveItemList(INT bShow);                      // 256 0x100556b0
    // guard AddInventoryItem
    virtual void GMViewAddInventoryItem(ItemInfo& Item);                // 257 0x10068b90
    virtual void GMViewStartWithdrawList(INT Adena);                    // 258 0x10055760
    virtual void GMViewAddWithdrawList(ItemInfo& Item);                 // 259 0x10068ec0
    // OnShowMinimap
    virtual void ShowMapItem(INT ItemID);                               // 260 0x100557c0
    virtual void ShowCalc(INT Code);                                         // 261 0x100559e0 (name ours)
    // the same body as StartPrivateStoreList (263)
    virtual void StartPrivateStoreManageList();                         // 262 0x10055a80
    virtual void StartPrivateStoreList();                               // 263 0x10055a80
    virtual void PrivateStoreManageList(ItemInfo& InventoryItem, ItemInfo& StoreItem, INT OwnerID, INT Adena); // 264 0x10069190
    virtual void PrivateStoreList(ItemInfo& Item, INT OwnerID, INT Adena); // 265 0x100697c0
    // also OnPrivateStoreBuyMsg; empty
    virtual void PrivateStoreMsg(TCHAR* Text, INT Param) {}             // 266 0x10055ab0
    // the same body as StartPrivateStoreBuyList (268)
    virtual void StartPrivateStoreBuyManageList();                      // 267 0x10055b20
    virtual void StartPrivateStoreBuyList();                            // 268 0x10055b20
    virtual void PrivateStoreBuyManageList(ItemInfo& InventoryItem, ItemInfo& StoreItem, INT OwnerID, INT Adena); // 269 0x10069b30
    virtual void PrivateStoreBuyList(ItemInfo& Item, INT OwnerID, INT Adena); // 270 0x1006a140

    // the rest of the interface
    virtual void BlockedAccountWithMsg(TCHAR* Msg);                     // 271 0x10058070
    virtual void BlockedAccount(INT Reason);                            // 272 0x100581f0
    virtual void SetSummonRemainTime(INT Remaining, INT Total);         // 273 0x1005ad00
    // bCtrlDown || bCtrlDownAlt
    virtual UBOOL IsCtrlDown();                                         // 274 0x100561c0
    virtual void VARARGS MessageBoxf(const TCHAR* Caption, const TCHAR* Fmt, ...); // 275 0x10055d70 no handler
    virtual UBOOL Init(UEngine* InEngine);                              // 280 0x10063170
    virtual INT ConsolePostRender(UCanvas* Canvas);                     // 281 0x1006be70
    virtual INT DispatchWndMsg(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam); // 282 0x1006d9e0
    virtual UBOOL NpcSay(L2ParamStack& Params);                         // 283 0x10051dc0
    virtual UBOOL MasterConsoleEventProcess(INT Key, INT Action, FLOAT Delta); // 284 0x1006bf40
    // "L2Debug", "L2DebugWindow"
    virtual UBOOL DebugCommand(const TCHAR* Cmd);                       // 285 0x10057420
    virtual UBOOL InitForPawnViewer(UEngine* InEngine);                 // 297 0x1006afe0
    virtual void PawnViewerFunction(INT Function);                      // 298 0x10057200
    // GMalloc'd array of the item ids
    virtual INT* GetInventoryItemIDs(INT& Count);                       // 301 0x10061070

    // the last pure slots: C1's Free and SetServerPing, the rest C2 additions (RequestLogout and ShowMessageWnd, non-virtual in C1, are here)
    // folded with operator delete
    virtual void Free(void* Ptr);                                       // 302 0x10026550
    virtual INT GetPartyMemberCount();                                         // 303 0x10052c20 (name ours)
    virtual void SetServerPing(INT ServerID, DWORD Ping);               // 305 0x100574c0
    virtual void Snoop(L2ParamStack& Params);                                         // 306 0x10055c20 (name ours)
    virtual void SetPadHoldL(BYTE bHold);                                         // 307 0x10057770 (name ours)
    virtual void SetPadHoldR(BYTE bHold);                                         // 308 0x10057790 (name ours)
    virtual void vfn271(INT Param) {}                                   // 309 0x10001ad0
    virtual void ShowTownMap(L2ParamStack& Params);                                         // 310 0x100577b0 (name ours)
    virtual void ObserverStart();                                         // 311 0x10057810 (name ours)
    virtual void ObserverEnd();                                         // 312 0x10057830 (name ours)
    virtual void PledgePower(INT P1, INT P2, BYTE* P3);                                         // 313 0x10057880 (name ours)
    virtual void ResizeWindows();                                         // 314 0x1005ddb0 (name ours)
    virtual void RequestLogout();                                       // 315 0x100521a0
    virtual void ShowQuitDialog();                                         // 316 0x10061be0 (name ours)
    virtual void ShowMessageWnd(const TCHAR* Text);                     // 317 0x100561e0
    virtual void vfn318(INT P1) {}                                         // 318 0x10001ad0 (name ours)
    virtual void ShowXMasSeal(INT P1);                                         // 319 0x10057910 (name ours)

    // NConsoleWnd's own
    virtual void RequestServerList();                                   // 320 0x10052640
    // the same body as AcceptLogOut (41)
    virtual void QuitGame();                                            // 321 0x10063bf0
    virtual void ReceiveServerClose();                                  // 322 0x10052ef0
    // NetHandler 76
    virtual void AnswerReviveRequest(UBOOL bYes, INT Param1, INT Param2); // 323 0x100531f0
    // NetHandler 171
    virtual void RequestEnchantItem(INT ObjectID);                      // 324 0x10053650
    // NetHandler 165
    virtual void RequestDestroyItem(NItemRequest Request);              // 325 0x10053740
    // NetHandler 166
    virtual void RequestCrystallizeItem(INT ObjectID, INT Count);       // 326 0x100537d0
    // NetHandler 156
    virtual void AnswerTradeRequest(L2ParamStack& Params);              // 327 0x10053970
    // NetHandler 169; 0 without a server
    virtual UBOOL RequestQuestList();                                   // 328 0x10053fb0
    // NetHandler 170
    virtual void RequestDestroyQuest(INT QuestID);                      // 329 0x10054050
    // NetHandler 172
    virtual void RequestAcquireSkillInfo(L2ParamStack& Params);         // 330 0x10054340
    // NetHandler 173
    virtual void RequestAcquireSkill(L2ParamStack& Params);             // 331 0x10054350
    // "bypass [-h] ..."; 2: close the page
    virtual INT RunHtmlCommand(TCHAR* Command, NCHtmlViewer* Viewer);   // 332 0x1005aa90
    virtual void RequestObserverEndPacket(L2ParamStack& Params);        // 333 0x10057850 (C2 only: a tail call of UL2NetHandler's slot of the same name)
    virtual void RequestPledgePower(INT P1, INT P2, BYTE* P3);          // 334 0x10057860 (C2 only: a tail call of UL2NetHandler's slot of the same name)
};
NW_ASSERT_OFFSET(NConsoleWnd, WndMain, 0x351C);
NW_ASSERT_OFFSET(NConsoleWnd, Desktop, 0x3540);
NW_ASSERT_OFFSET(NConsoleWnd, DebugWnd, 0x3544);
NW_ASSERT_OFFSET(NConsoleWnd, PetitionWnd, 0x3550);
NW_ASSERT_OFFSET(NConsoleWnd, Timers, 0x3568);
NW_ASSERT_OFFSET(NConsoleWnd, DeltaTime, 0x3578);
NW_ASSERT_OFFSET(NConsoleWnd, PartyItemDistribution, 0x35A4);
NW_ASSERT_OFFSET(NConsoleWnd, RestartMenuWnd, 0x3520);
NW_ASSERT_OFFSET(NConsoleWnd, MessageWnd, 0x353C);
NW_ASSERT_OFFSET(NConsoleWnd, LoadingScreenID, 0x35A0);
NW_ASSERT_OFFSET(NConsoleWnd, bInitialized, 0x3518);
NW_ASSERT_OFFSET(NConsoleWnd, CreditWnd, 0x352C);
NW_ASSERT_OFFSET(NConsoleWnd, WarningDialogBox, 0x3554);
NW_ASSERT_OFFSET(NConsoleWnd, SysMsgParams, 0x3558);
NW_ASSERT_OFFSET(NConsoleWnd, WordWrapperWnd, 0x3564);
NW_ASSERT_OFFSET(NConsoleWnd, TutorialEvents, 0x3598);
NW_ASSERT_OFFSET(NConsoleWnd, PawnViewerWnd, 0x35AC);
NW_ASSERT_SIZE(NConsoleWnd, 0x35B8);

// The children of the login screen in C2, by offset (the stock code reads them at these places; NCLoginWnd.h, the
// pawn group's, still has C1's order): +0x98 AuthWnd, +0x9C NoticeWnd, +0xA0 WarningWnd, +0xA4 and +0xA8 two
// windows C2 added (names ours: LOGIN_Wnd_A4, LOGIN_Wnd_A8), +0xAC the server list
class NCServerSelectWnd;
#define NLOGIN_SERVERLIST(Login) (nw::Field<NCServerSelectWnd*>((Login), 0xAC))
#define NLOGIN_WND_A4(Login) (nw::Field<NCWnd*>((Login), 0xA4))
#define NLOGIN_WND_A8(Login) (nw::Field<NCWnd*>((Login), 0xA8))

// Cast<APawn> out of line (0x10047230, name ours): NULL unless Object is a pawn
APawn* CastToPawn(UObject* Object);

// Cast<ALineagePlayerController> out of line (0x1004c5c0, name ours): NULL unless Object is the L2 player's controller
ALineagePlayerController* CastToLineagePlayerController(UObject* Object);

#pragma pack(pop)