#pragma once
// NCConsole - the desktop (UClass "CConsole", 0x180; NConsoleWnd +0x36DC, built by the
// console's Initialize): the screen-sized parent of every in-game window. OnCreate makes them
// all, numbered in order (NCWnd +0x78); ShowWindow (entering the game) hides the dialogs and
// shows the bars, the chat and the status windows. It owns the toggles the menus and the
// shortcut keys use, the HTML page commands ("Link", "Write") and the chat balloons: the
// first 19 characters a character says are drawn over it (its pawn's +0x142C) for 5 seconds.

#include "nwindow/NCWnd.h"
#include "nwindow/NConsoleWnd.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NConsoleWnd;
class NCMainWnd;
class NCChatWnd;
class NCNPCHtmlViewer;
class NCPartyWnd;
class NCPetStatusWnd;
class NCSummonedStatusWnd;
class NCTargetStatusWnd;
class NCPlayerStatusWnd;
class NCAbnormalStatusWnd;
class NCMenuWnd;
class NCSystemMenuWnd;
class NCShortCutWnd;
class NCTrainWnd;
class NCMatchWnd;
class NCMatchOptionWnd;
class NCMinimapWnd;
class NCTutorialViewerWnd;
class NCTutorialQuestionWnd;
class NCPetWnd;
class NCSummonedWnd;
class NCRadarWnd;
class NCGMMagicSkillWnd;
class NCGMDetailStatusWnd;
class NCDialogBox;
class NCHtmlViewer;
class NCGMPetitionWnd;
class NCInventoryWnd;
class NCTradeWnd;
class NCGMWnd;
class NCGMInventoryWnd;
class NCGMQuestWnd;
class NCGMActionWnd;
class NCVIPShopWnd;
class NCStoreWnd;
class NCShopWnd;
class NCGMStoreWnd;
class NCDeliverWnd;
class NCSelectDeliverWnd;
class NCPrivateShopWnd;
class NCPrivateBuyWnd;

// NWM_COMMAND id the desktop answers: quit the game (name ours)
enum { NID_CONSOLE_QUIT = 0x1002 };

// the desktop's own messages (names ours): wParam an FVector, the player moves there
enum { NWM_MOVETO = (INT)0xF0000001 };

// the balloon timer (SetTimer id, once a second)
enum { NTIMER_BALLOONS = 0xFA0D05 };

// A chat balloon (name ours; 12 bytes, NCConsole::Balloons).
struct NChatBalloon
{
    INT ObjectID;              // +0x0 who said it
    TCHAR* Text;               // +0x4 new TCHAR[23]: at most 19 characters, then "..."; the
                               //   pawn's balloon (APawn +0x142C) points at it
    INT Tick;                  // +0x8 NCConsole::Ticks when said

    NChatBalloon() : ObjectID(0), Text(NULL), Tick(0) {}
    ~NChatBalloon() { Text = NULL; }   // the text is freed by hand
};
NW_ASSERT_SIZE(NChatBalloon, 0xC);

class NCConsole : public NCWnd
{
    NW_DECLARE_CLASS(NCConsole, NCWnd, 0, 0x101B1248)

    NCMainWnd* MainWnd;                    // +0x98 character window
    NCWnd* CalcWnd;                        // +0x9C NCCalcWnd (new in C2; name ours), made by OnCreate
    NCWnd* EmpowerWnd;                     // +0xA0 NCEmpowerWnd (new in C2; name ours)
    NCChatWnd* ChatWnd;                    // +0xA4
    NCNPCHtmlViewer* NpcHtmlWnd;           // +0xA8 NPC dialogs
    NCNPCHtmlViewer* HelpWnd;              // +0xAC "..\L2text\help.htm"
    NConsoleWnd* Owner;                    // +0xB0 the console (the second constructor)
    INT UnknownA8;                         // +0xB4 (1)
    NCGMPetitionWnd* GMPetitionWnd;        // +0xB8
    NCWnd* PetitionBeginWnd;               // +0xBC NCPetitionBeginWnd (new in C2; name ours)
    NCPartyWnd* PartyWnd;                  // +0xC0
    NCPetStatusWnd* PetStatusWnd;          // +0xC4
    NCSummonedStatusWnd* SummonedStatusWnd;// +0xC8
    NCInventoryWnd* InventoryWnd;          // +0xCC
    NCTargetStatusWnd* TargetStatusWnd;    // +0xD0
    NCPlayerStatusWnd* PlayerStatusWnd;    // +0xD4
    NCAbnormalStatusWnd* AbnormalStatusWnd;// +0xD8 buffs
    NCStoreWnd* StoreWnd;                  // +0xDC warehouse
    NCShopWnd* ShopWnd;                    // +0xE0 NPC shop
    NCWnd* NewWndE4;                       // +0xE4 (name ours; new in C2: zeroed by both constructors) NCManorShopWnd (OnCreate; text 738)
    NCVIPShopWnd* VIPShopWnd;              // +0xE8
    NCMenuWnd* MenuWnd;                    // +0xEC the menu bar
    NCSystemMenuWnd* SystemMenuWnd;        // +0xF0
    NCTradeWnd* TradeWnd;                  // +0xF4
    NCWnd* NewWndF8;                       // +0xF8 (name ours; new in C2: zeroed by both constructors) NCMacroListWnd (OnCreate)
    NCWnd* NewWndFC;                       // +0xFC NCMacroEditWnd (its +0x9C holds the edit lines)
    NCWnd* NewWnd100;                      // +0x100 NCMacroInfoWnd
    NCShortCutWnd* ShortCutWnd;            // +0x104
    NCTrainWnd* TrainWnd;                  // +0x108 skill trainer
    NCMatchWnd* MatchWnd;                  // +0x10C party matching
    NCMatchOptionWnd* MatchOptionWnd;      // +0x110
    NCMinimapWnd* MinimapWnd;              // +0x114
    NCWnd* BoardWnd;                       // +0x118 NCBoardWnd (community)
    NCTutorialViewerWnd* TutorialWnd;      // +0x11C
    NCTutorialQuestionWnd* TutorialQuestionWnd; // +0x120 the question mark
    NCPetWnd* PetWnd;                      // +0x124
    NCSummonedWnd* SummonedWnd;            // +0x128
    NCRadarWnd* RadarWnd;                  // +0x12C
    NCWnd* SiegeInfoWnd;                   // +0x130 NCSiegeInfoMainWnd
    NCWnd* NewWnd134;                      // +0x134 (name ours; new in C2, zeroed by both constructors) NCRecipeBookWnd
    NCWnd* NewWnd138;                      // +0x138 NCRecipeTreeWnd
    NCWnd* NewWnd13C;                      // +0x13C NCRecipeManufactureWnd
    NCWnd* RecipeShopWnd;                  // +0x140 (name ours; new in C2) NCRecipeShopWnd; ToggleInterface: the recipe shop manage window
    NCWnd* NewWnd144;                      // +0x144 (name ours; new in C2: only the first constructor zeroes it) NCRecipeBuyListWnd
    NCWnd* RecipeBuyManufactureWnd;        // +0x148 NCRecipeBuyManufactureWnd (new in C2; name ours)
    NCWnd* NewWnd14C;                      // +0x14C (name ours; new in C2, zeroed by both constructors) NCHennaListWnd
    NCWnd* NewWnd150;                      // +0x150 NCHennaInfoWnd
    NCGMWnd* GMWnd;                        // +0x154
    NCGMInventoryWnd* GMInventoryWnd;      // +0x158
    NCGMMagicSkillWnd* GMMagicSkillWnd;    // +0x15C
    NCGMDetailStatusWnd* GMDetailStatusWnd;// +0x160
    NCGMQuestWnd* GMQuestWnd;              // +0x164
    NCGMStoreWnd* GMStoreWnd;              // +0x168
    NCGMActionWnd* GMActionWnd;            // +0x16C
    NCPrivateShopWnd* PrivateShopWnd;      // +0x170 selling
    NCPrivateBuyWnd* PrivateBuyWnd;        // +0x174 buying
    NCDeliverWnd* DeliverWnd;              // +0x178 freight
    NCSelectDeliverWnd* SelectDeliverWnd;  // +0x17C
    NCNPCHtmlViewer* TestNpcHtmlWnd;       // +0x180 "test.html" (GIsOnlyL2Console only)
    NCWnd* TestHtmlWnd;                    // +0x184 NCTestHtmlViewer (GIsTestHtml only)
    NCDialogBox* MainDialogBox;            // +0x188 NDialogInfo::bMainBox
    NCDialogBox* SecondDialogBox;          // +0x18C
    NCWnd* GMSnoopManager;                 // +0x190 NCGMSnoopManager (new in C2; name ours): a child with no rectangle
    NCWnd* NewWnd194;                      // +0x194 (name ours; new in C2: only the first constructor zeroes it) NTownMapWnd
    NCWnd* NewWnd198;                      // +0x198 NCObserverWnd
    NCWnd* NewWnd19C;                      // +0x19C (zeroed by both constructors) NCXMasSealWnd
    INT Unknown150;                        // +0x1A0 (1)
    UTexture* BlackTex;                    // +0x1A4 "NWindow.BlackTexture"
    UTexture* BalloonTex;                  // +0x1A8 "NWindow.chat"
    TArray<NChatBalloon> Balloons;         // +0x1AC
    INT BalloonTimer;                      // +0x1B8 SetTimer result
    INT Ticks;                             // +0x1BC seconds (the balloon timer)
    INT Unknown170[4];                     // +0x1C0

    NCConsole();                                                     // 0x10051820
    NCConsole(NConsoleWnd* InOwner);                                 // 0x10051a00 (Initialize)
    // ~NCConsole                                                    // 0x100519a0

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x1004c880
    virtual INT OnTimer(INT TimerID);                                // 35 0x100516f0
    virtual INT OnCreate();                                          // 36 0x1004c900
    virtual INT OnDestroy();                                         // 37 0x10050b60
    virtual void ShowWindow();                                       // 43 0x1004f6b0
    virtual void HideWindow();                                       // 44 0x1007b470 (folded)
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x1004b750
    // name ours: an HTML page's "Link <bypass>" or "Write <board> ..." command
    virtual INT OnHtmlCommand(const TCHAR* Command, NCHtmlViewer* Source); // 67 0x1004fab0

    // names ours
    __declspec(noinline) INT OnMoveTo(WPARAM wParam, LPARAM lParam);                      // 0x1004b7f0 NWM_MOVETO
    __declspec(noinline) INT OnQuit();                                                    // 0x1004b820 NID_CONSOLE_QUIT: appRequestExit(0)
    void ToggleInterface();                                          // 0x1004b840 (a shortcut key)
    void CloseWindows();                                             // 0x1004b970 (a shortcut key)
    void Reset();                                                    // 0x1004bbc0 (the console)
    void TogglePetition();                                           // 0x1003de90 (C1 only: no C2 match) (the chat line)
    void ToggleSystemMenu();                                         // 0x1004bc40
    void ToggleInventory();                                          // 0x1004bca0
    void ToggleChat();                                               // 0x1004bd50
    void ToggleHelp(UBOOL bMacro);                                   // 0x1004bd80 (C2: bMacro, name ours)
    void TogglePartyMatch();                                         // 0x1004bde0
    void ToggleMinimap();                                            // 0x1004bfc0
    void ToggleMatchOption();                                        // 0x1004c020
    void ToggleBoard();                                              // 0x1004c050
    __declspec(noinline) void ToggleMacroList();                                          // 0x1004c100 (C2 only; name ours) the macro window, with its sound
    __declspec(noinline) void ToggleMacroEdit();                                          // 0x1004c160 (C2 only; name ours) the macro editor, the same way
    __declspec(noinline) void ToggleXMasSeal();                                           // 0x1004c440 (C2 only; name ours) the Christmas seal window
    void ToggleGMWnd();                                              // 0x1004c1c0
    void ToggleGMInventory(TCHAR* Name);                             // 0x1004c200 (the GM window)
    void ToggleGMMagicSkill(TCHAR* Name);                            // 0x1004c260
    void ToggleGMDetailStatus(TCHAR* Name);                          // 0x1004c2c0
    void ToggleGMQuest(TCHAR* Name);                                 // 0x1004c320
    void ToggleGMStore(TCHAR* Name);                                 // 0x1004c380
    void ToggleGMAction(TCHAR* Name);                                // 0x1004c3e0
    void AddSystemMessage(INT MsgID, DWORD Color);                   // 0x1004c470 into the chat
    void AddSystemText(const TCHAR* Text, INT SubType, DWORD Color); // 0x1004c4c0 into the chat
    void ToggleCharacter(INT Tab);                                   // 0x100507b0 -1: show / hide
    void Say(INT ObjectID, const TCHAR* Name, const TCHAR* Text, DWORD Color,
             INT Type);                                              // 0x10041880 the console
};
NW_ASSERT_OFFSET(NCConsole, MainWnd, 0x98);
NW_ASSERT_OFFSET(NCConsole, CalcWnd, 0x9C);
NW_ASSERT_OFFSET(NCConsole, EmpowerWnd, 0xA0);
NW_ASSERT_OFFSET(NCConsole, Owner, 0xB0);
NW_ASSERT_OFFSET(NCConsole, PetitionBeginWnd, 0xBC);
NW_ASSERT_OFFSET(NCConsole, RecipeBuyManufactureWnd, 0x148);
NW_ASSERT_OFFSET(NCConsole, GMSnoopManager, 0x190);
NW_ASSERT_OFFSET(NCConsole, InventoryWnd, 0xCC);
NW_ASSERT_OFFSET(NCConsole, ShortCutWnd, 0x104);
NW_ASSERT_OFFSET(NCConsole, RadarWnd, 0x12C);
NW_ASSERT_OFFSET(NCConsole, RecipeShopWnd, 0x140);
NW_ASSERT_OFFSET(NCConsole, MainDialogBox, 0x188);
NW_ASSERT_OFFSET(NCConsole, Balloons, 0x1AC);
NW_ASSERT_OFFSET(NCConsole, Ticks, 0x1BC);
NW_ASSERT_SIZE(NCConsole, 0x1D0);

// the desktop (NConsoleWnd::Desktop is declared as an NCWnd)
#define GDesktop (static_cast<NCConsole*>(GConsole->Desktop))
inline NCConsole* NConsoleWnd::GetDesktop() const
{
    return static_cast<NCConsole*>(Desktop);
}

#pragma pack(pop)