/*=============================================================================
    NCConsole.cpp - the desktop. Stock: 0x1003d740 .. 0x1003d7d1, 0x1003d7e0 .. 0x1003daa0,
    0x1003dac0 .. 0x1003e716, 0x1003e950 .. 0x10040f1e, 0x10041800 .. 0x10041b19,
    0x10041bc0 .. 0x100424d1, 0x10042530 .. 0x100427ba (HideWindow is NCChatWnd's body; the
    TArray<NChatBalloon> code at 0x10041b20 and 0x100424e0 is the template's).
=============================================================================*/
#include "nwindow/NCConsole.h"
#include "nwindow/NCAbnormalStatusWnd.h"
#include "nwindow/NCActionWnd.h"
#include "nwindow/NCBoardWnd.h"
#include "nwindow/NCChatWnd.h"
#include "nwindow/NCDeliverWnd.h"
#include "nwindow/NCDetailStatusWnd.h"
#include "nwindow/NCDialogBox.h"
#include "nwindow/NCEditBox.h"
#include "nwindow/NCGDevice.h"
#include "nwindow/NCGMWnd.h"
#include "nwindow/NCHtmlViewer.h"
#include "nwindow/NCInventoryWnd.h"
#include "nwindow/NCMagicSkillWnd.h"
#include "nwindow/NCMainWnd.h"
#include "nwindow/NCMatchOptionWnd.h"
#include "nwindow/NCMatchWnd.h"
#include "nwindow/NCMenuWnd.h"
#include "nwindow/NCMinimapWnd.h"
#include "nwindow/NCNPCHtmlViewer.h"
#include "nwindow/NCOptionWnd.h"
#include "nwindow/NCPartyWnd.h"
#include "nwindow/NCPetitionWnd.h"
#include "nwindow/NCPetWnd.h"
#include "nwindow/NCPrivateBuyWnd.h"
#include "nwindow/NCPrivateShopWnd.h"
#include "nwindow/NCRadarWnd.h"
#include "nwindow/NCShopWnd.h"
#include "nwindow/NCShortCutWnd.h"
#include "nwindow/NCSiegeInfoMainWnd.h"
#include "nwindow/NCStatusWnd.h"
#include "nwindow/NCStoreWnd.h"
#include "nwindow/NCSummonedWnd.h"
#include "nwindow/NCSystemMenuWnd.h"
#include "nwindow/NCTestHtmlViewer.h"
#include "nwindow/NCTradeWnd.h"
#include "nwindow/NCTrainWnd.h"
#include "nwindow/NCTutorialViewerWnd.h"
#include "nwindow/NCVIPShopWnd.h"
#include "nwindow/NCCalcWnd.h"
#include "nwindow/NCEmpowerWnd.h"
#include "nwindow/NCGMSnoopManager.h"
#include "nwindow/NCGaraDialogBox.h"
#include "nwindow/NCHennaInfoWnd.h"
#include "nwindow/NCHennaListWnd.h"
#include "nwindow/NCMacroEditWnd.h"
#include "nwindow/NCMacroInfoWnd.h"
#include "nwindow/NCMacroListWnd.h"
#include "nwindow/NCManorShopWnd.h"
#include "nwindow/NCObserverWnd.h"
#include "nwindow/NCPetitionBeginWnd.h"
#include "nwindow/NCRecipeBookWnd.h"
#include "nwindow/NCRecipeBuyListWnd.h"
#include "nwindow/NCRecipeBuyManufactureWnd.h"
#include "nwindow/NCRecipeManufactureWnd.h"
#include "nwindow/NCRecipeShopWnd.h"
#include "nwindow/NCRecipeTreeWnd.h"
#include "nwindow/NCXMasSealWnd.h"
#include "nwindow/NTownMapWnd.h"
#include "nwindow/NConsoleWnd.h"
#include "nwindow/NClasses.gen.h"

#include <stdio.h>
#include <wchar.h>

// OnCreate numbers the windows it makes, in order (NCWnd +0x78; name ours)
#if NW_HOOK_MODE
#define GNextWndNumber NW_GLOBAL(INT, 0x101a72b8)
#else
extern INT GNextWndNumber;
#endif

// interface sounds of the toggles (names ours)
enum { SOUND_WindowOpen = 5, SOUND_WindowClose = 6, SOUND_Minimap = 8, SOUND_Petition = 10 };

// the player's petition window (NConsoleWnd::PetitionWnd is declared as an NCWnd)
static FORCEINLINE NCPetitionWnd* PlayerPetitionWnd()
{
    return static_cast<NCPetitionWnd*>(GConsole->PetitionWnd);
}

// the console's options window
static FORCEINLINE NCWnd* OptionWnd()
{
    return GConsole->OptionWnd;
}

// NCStoreWnd::GetMode; the stock code calls it on the NPC shop too, whose mode is at the same
// place (+0xBC)
static FORCEINLINE INT StoreWndMode(NCWnd* Store)
{
    return static_cast<NCStoreWnd*>(Store)->GetMode();
}

/*-----------------------------------------------------------------------------
    Construction.
-----------------------------------------------------------------------------*/

// 0x10051820 - most window fields are left as allocated (C2: more of them are cleared, for the windows it added)
NCConsole::NCConsole()
{
    ChatWnd = NULL;
    NpcHtmlWnd = NULL;
    HelpWnd = NULL;
    Owner = NULL;
    InventoryWnd = NULL;
    TargetStatusWnd = NULL;
    PlayerStatusWnd = NULL;
    AbnormalStatusWnd = NULL;
    NewWndE4 = NULL;
    MenuWnd = NULL;
    SystemMenuWnd = NULL;
    NewWndF8 = NULL;
    NewWndFC = NULL;
    NewWnd100 = NULL;
    ShortCutWnd = NULL;
    TrainWnd = NULL;
    MatchWnd = NULL;
    MatchOptionWnd = NULL;
    MinimapWnd = NULL;
    BoardWnd = NULL;
    PetWnd = NULL;
    SummonedWnd = NULL;
    NewWnd134 = NULL;
    NewWnd138 = NULL;
    NewWnd13C = NULL;
    RecipeShopWnd = NULL;
    NewWnd144 = NULL;
    NewWnd14C = NULL;
    NewWnd150 = NULL;
    GMWnd = NULL;
    GMInventoryWnd = NULL;
    GMMagicSkillWnd = NULL;
    GMDetailStatusWnd = NULL;
    GMQuestWnd = NULL;
    GMStoreWnd = NULL;
    NewWnd194 = NULL;
    NewWnd198 = NULL;
    NewWnd19C = NULL;
    BlackTex = NULL;
    BalloonTex = NULL;
    UnknownA8 = 1;
    Unknown150 = 1;
    BalloonTimer = 0;
    Ticks = 0;
    Unknown170[1] = 0;
    Unknown170[0] = 0;
    Unknown170[2] = 0;
    Unknown170[3] = 0;
}
NW_HOOK_CTOR(0x10051820, NCConsole)

// 0x10051a00 - NConsoleWnd::Initialize's; also clears TestNpcHtmlWnd, and leaves RecipeShopWnd and the
// windows at +0x144, +0x194 and +0x198 as allocated
NCConsole::NCConsole(NConsoleWnd* InOwner)
{
    ChatWnd = NULL;
    NpcHtmlWnd = NULL;
    HelpWnd = NULL;
    InventoryWnd = NULL;
    TargetStatusWnd = NULL;
    PlayerStatusWnd = NULL;
    AbnormalStatusWnd = NULL;
    NewWndE4 = NULL;
    MenuWnd = NULL;
    SystemMenuWnd = NULL;
    NewWndF8 = NULL;
    NewWndFC = NULL;
    NewWnd100 = NULL;
    ShortCutWnd = NULL;
    TrainWnd = NULL;
    MatchWnd = NULL;
    MatchOptionWnd = NULL;
    MinimapWnd = NULL;
    BoardWnd = NULL;
    PetWnd = NULL;
    SummonedWnd = NULL;
    NewWnd134 = NULL;
    NewWnd138 = NULL;
    NewWnd13C = NULL;
    NewWnd14C = NULL;
    NewWnd150 = NULL;
    GMWnd = NULL;
    GMInventoryWnd = NULL;
    GMMagicSkillWnd = NULL;
    GMDetailStatusWnd = NULL;
    GMQuestWnd = NULL;
    GMStoreWnd = NULL;
    TestNpcHtmlWnd = NULL;
    NewWnd19C = NULL;
    BlackTex = NULL;
    BalloonTex = NULL;
    UnknownA8 = 1;
    Unknown150 = 1;
    BalloonTimer = 0;
    Ticks = 0;
    Unknown170[1] = 0;
    Unknown170[0] = 0;
    Unknown170[2] = 0;
    Unknown170[3] = 0;
    Owner = InOwner;
}
NW_HOOK_CTOR_ARGS(0x10051a00, NCConsole, (NConsoleWnd* A), (A))

// 0x100519a0 - Balloons' destructor is TArray<NChatBalloon>'s (0x100424e0)
NW_HOOK_DTOR(0x100519a0, NCConsole)

/*-----------------------------------------------------------------------------
    Messages.
-----------------------------------------------------------------------------*/

// 0x1004b820 - NID_CONSOLE_QUIT (C2: out of line; C1 had it inside WindowProc): the game ends
INT NCConsole::OnQuit()
{
    appRequestExit(0);
    return 1;
}
NW_HOOK_METHOD(0x1004b820, NCConsole, OnQuit)

// 0x1004c880 (vslot 25) - NID_CONSOLE_QUIT quits, NWM_MOVETO walks (C2: no IsEnabled test first, as C1 had)
INT NCConsole::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (NCWnd::WindowProc(Msg, wParam, lParam))
        return 1;
    if (Msg == NWM_COMMAND)
    {
        if (wParam == NID_CONSOLE_QUIT)
            return OnQuit() != 0;
    }
    else if (Msg == (UINT)NWM_MOVETO)
        return OnMoveTo(wParam, lParam) != 0;
    return 0;
}
NW_HOOK_METHOD(0x1004c880, NCConsole, WindowProc)

// 0x1004b7f0 - the player walks to the FVector wParam points at
INT NCConsole::OnMoveTo(WPARAM wParam, LPARAM lParam)
{
    Owner->MoveToLocation(*(FVector*)wParam, 0);
    return 1;
}
NW_HOOK_METHOD(0x1004b7f0, NCConsole, OnMoveTo)

// 0x100516f0 (vslot 35) - each second the balloons older than 5 seconds go (and off their
// pawns)
INT NCConsole::OnTimer(INT TimerID)
{
    if (TimerID == NTIMER_BALLOONS)
    {
        Ticks++;
        for (INT i = 0; i < Balloons.Num();)
        {
            NChatBalloon& Balloon = Balloons(i);
            if (Balloon.Tick + 5 < Ticks)
            {
                User* Speaker = GConsole->FindUserByID(Balloon.ObjectID);
                if (Speaker && Speaker->Pawn)
                {
                    Speaker->Pawn->ChatBalloon = NULL;
                    Speaker->Pawn->Unknown5C++;
                }
                if (Balloon.Text)
                {
                    delete[] Balloon.Text;
                    Balloon.Text = NULL;
                }
                Balloons.Remove(i, 1);
            }
            else
                i++;
        }
    }
    return 1;
}
NW_HOOK_METHOD(0x100516f0, NCConsole, OnTimer)

// 0x1004b750 (vslot 55) - draws nothing
INT NCConsole::OnPaint(NCGDevice* Device)
{
    guard(NCConsole::OnPaint);
    Device->SetClip(ScreenX, ScreenY, Width, Height);
    Device->ReleaseClip();
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x1004b750, NCConsole, OnPaint)

/*-----------------------------------------------------------------------------
    Creation.
-----------------------------------------------------------------------------*/

// A child hidden before its creation message (inlined in the stock OnCreate for eight
// windows: the shortcut bar, the trainer, the party matching and its options, the minimap, the
// GM window and the two test pages; name ours): registered, hidden, then attached (entry,
// screen position, WM_CREATE)
static FORCEINLINE void CreateHiddenChild(NCWnd* Parent, NCWnd* Child, DWORD ChildStyle)
{
    NWndEntry* E = Parent->AddChildWindow(Child, ChildStyle);
    Child->HideWindow();
    Child->AttachEntry(E);
}

// A child hidden before its creation message, the attach inline (the shortcut bar, the trainer, the matching
// windows, the minimap, the town map and the observer window: name ours): registered, hidden, then attached
static FORCEINLINE void CreateChildHidden(NCWnd* Parent, NCWnd* Child, DWORD ChildStyle)
{
    NWndEntry* E = Parent->AddChildWindow(Child, ChildStyle);
    Child->HideWindow();
    Child->Entry = E;
    if (E->Parent)
    {
        NCWnd* Owner = E->Parent->Wnd;
        Child->ScreenX += Owner->ScreenX;
        Child->ScreenY += Owner->ScreenY;
    }
    Child->WindowProc(WM_CREATE, 0, 0);
}

// A child attached by the out-of-line AttachEntry (where the stock stopped inlining it: the manor shop, the GM
// windows and the last ones; name ours)
static FORCEINLINE void CreateChildCall(NCWnd* Parent, NCWnd* Child, DWORD ChildStyle)
{
    NWndEntry* E = Parent->AddChildWindow(Child, ChildStyle);
    Child->AttachEntry(E);
}

// 0x1004c900 (vslot 36) - every in-game window, in the order the stock makes them; the C2 desktop has the calculator
// (+0x9C), the GM snoop manager, the petition begin and empower windows, the macro, recipe, henna, manor, town
// map, observer and X-mas seal windows besides the C1 ones, and the two dialog boxes are NCGaraDialogBoxes.
// Most are numbered (NCWnd +0x7C) in creation order; the shops, the freight, the dialogs and a few more are not.
// Positions come from the size of the desktop: LocalW / LocalH for some, Width / Height for others, halved unsigned.
INT NCConsole::OnCreate()
{
    DWORD ScreenW = LocalW;
    DWORD ScreenH = LocalH;

    MainWnd = new NCMainWnd;
    MainWnd->SetWindowRect(207, 50, 240, 365);
    CreateChild(MainWnd, 0x3002);
    MainWnd->Unknown78 = ++GNextWndNumber;

    CalcWnd = new NCCalcWnd;
    CalcWnd->SetWindowRect(300, 200, 240, 215);
    CreateChild(CalcWnd, 0x1002);
    CalcWnd->SetTitle(*GL2GameData.SysString[669]);

    GMSnoopManager = new NCGMSnoopManager;
    CreateChild(GMSnoopManager, 6);

    GMPetitionWnd = new NCGMPetitionWnd;
    GMPetitionWnd->SetWindowRect(200, 200, 300, 301);
    CreateChild(GMPetitionWnd, 2);
    GMPetitionWnd->SetTitle(*GL2GameData.SysString[472]);
    GMPetitionWnd->Unknown78 = ++GNextWndNumber;

    PetitionBeginWnd = new NCPetitionBeginWnd;
    PetitionBeginWnd->SetWindowRect(300, 200, 240, 365);
    CreateChild(PetitionBeginWnd, 2);
    PetitionBeginWnd->SetTitle(*GL2GameData.SysString[470]);

    EmpowerWnd = new NCEmpowerWnd;
    EmpowerWnd->SetWindowRect(447, 50, 240, 352);
    CreateChild(EmpowerWnd, 0x1002);
    EmpowerWnd->SetTitle(*GL2GameData.SysString[668]);

    NpcHtmlWnd = new NCNPCHtmlViewer;
    NpcHtmlWnd->SetWindowRect(300, 250, 300, 277);
    CreateChild(NpcHtmlWnd, 0x1002);
    NpcHtmlWnd->HideWindow();
    NpcHtmlWnd->Unknown78 = ++GNextWndNumber;
    NpcHtmlWnd->SetTitle(*GL2GameData.SysString[444]);

    BoardWnd = new NCBoardWnd;
    BoardWnd->SetWindowRect(60, 20, 646, 483);
    CreateChild(BoardWnd, 0x3002);
    BoardWnd->HideWindow();
    BoardWnd->Unknown78 = ++GNextWndNumber;
    BoardWnd->SetTitle(*GL2GameData.SysString[387]);

    InventoryWnd = new NCInventoryWnd;
    InventoryWnd->SetWindowRect(ScreenW - 290, 205, 240, 365);
    CreateChild(InventoryWnd, 0x3002);
    InventoryWnd->Unknown78 = ++GNextWndNumber;
    InventoryWnd->SetTitle(*GL2GameData.SysString[138]);

    PlayerStatusWnd = new NCPlayerStatusWnd(100);
    PlayerStatusWnd->SetWindowRect(0, 0, 152, 71);
    CreateChild(PlayerStatusWnd, 0x1002);
    PlayerStatusWnd->Unknown78 = ++GNextWndNumber;

    DWORD HalfW = ScreenW / 2;
    TargetStatusWnd = new NCTargetStatusWnd(130);
    TargetStatusWnd->SetWindowRect(HalfW - 76, 0, 152, 36);
    CreateChild(TargetStatusWnd, 0x1002);
    TargetStatusWnd->Unknown78 = ++GNextWndNumber;

    AbnormalStatusWnd = new NCAbnormalStatusWnd(24);
    AbnormalStatusWnd->SetWindowRect(154, 2, 252, 50);
    CreateChild(AbnormalStatusWnd, 0x1022);
    AbnormalStatusWnd->Unknown78 = ++GNextWndNumber;

    PartyWnd = new NCPartyWnd;
    PartyWnd->SetWindowRect(0, 72, 152, 36);
    CreateChild(PartyWnd, 0x1002);
    PartyWnd->Unknown78 = ++GNextWndNumber;

    PetStatusWnd = new NCPetStatusWnd(100);
    PetStatusWnd->SetWindowRect(152, 0, 152, 43);
    CreateChild(PetStatusWnd, 0x1002);
    PetStatusWnd->Unknown78 = ++GNextWndNumber;

    SummonedStatusWnd = new NCSummonedStatusWnd(100);
    SummonedStatusWnd->SetWindowRect(152, 0, 152, 43);
    CreateChild(SummonedStatusWnd, 0x1002);
    SummonedStatusWnd->Unknown78 = ++GNextWndNumber;

    StoreWnd = new NCStoreWnd;
    StoreWnd->SetWindowRect(ScreenW - 290, 205, 240, 365);
    CreateChild(StoreWnd, 0x1002);
    StoreWnd->Unknown78 = ++GNextWndNumber;
    StoreWnd->SetTitle(*GL2GameData.SysString[131]);

    DeliverWnd = new NCDeliverWnd;
    DeliverWnd->SetWindowRect(ScreenW - 290, 205, 240, 365);
    CreateChild(DeliverWnd, 0x1002);
    DeliverWnd->SetTitle(*GL2GameData.SysString[557]);

    SelectDeliverWnd = new NCSelectDeliverWnd;
    SelectDeliverWnd->SetWindowRect(300, 250, 300, 277);
    CreateChild(SelectDeliverWnd, 0x1002);
    SelectDeliverWnd->SetTitle(*GL2GameData.SysString[558]);

    ShopWnd = new NCShopWnd;
    ShopWnd->SetWindowRect(ScreenW - 290, 205, 240, 365);
    CreateChild(ShopWnd, 0x1002);
    ShopWnd->Unknown78 = ++GNextWndNumber;

    VIPShopWnd = new NCVIPShopWnd;
    VIPShopWnd->SetWindowRect(ScreenW - 290, 205, 240, 365);
    CreateChild(VIPShopWnd, 0x1002);
    VIPShopWnd->SetTitle(*GL2GameData.SysString[136]);

    TradeWnd = new NCTradeWnd;
    TradeWnd->SetWindowRect(ScreenW - 290, 205, 240, 365);
    CreateChild(TradeWnd, 0x1002);
    TradeWnd->Unknown78 = ++GNextWndNumber;
    TradeWnd->SetTitle(*GL2GameData.SysString[445]);

    MenuWnd = new NCMenuWnd;
    MenuWnd->SetWindowRect(Width - 166, Height - 44, 167, 44);
    CreateChild(MenuWnd, 0x1002);
    MenuWnd->Unknown78 = ++GNextWndNumber;

    SystemMenuWnd = new NCSystemMenuWnd;
    SystemMenuWnd->SetWindowRect(Width - 290, 250, 240, 192);
    CreateChild(SystemMenuWnd, 0x3002);
    SystemMenuWnd->SetTitle(*GL2GameData.SysString[446]);

    DWORD HalfH = ScreenH / 2;
    HelpWnd = new NCNPCHtmlViewer;
    HelpWnd->SetWindowRect(HalfW - 162, HalfH - 139, 300, 277);
    CreateChild(HelpWnd, 0x1002);
    HelpWnd->HideWindow();
    HelpWnd->Unknown78 = ++GNextWndNumber;
    HelpWnd->SetTitle(*GL2GameData.SysString[145]);

    MainDialogBox = (NCDialogBox*)new NCGaraDialogBox;
    MainDialogBox->SetWindowRect((DWORD)Width / 2 - 101, (DWORD)Height / 2 - 43, 203, 87);
    CreateChild(MainDialogBox, 0x22);
    MainDialogBox->SetButtonTexts(140, 141);

    SecondDialogBox = (NCDialogBox*)new NCGaraDialogBox;
    SecondDialogBox->SetWindowRect(347, Height - 89, 203, 87);
    CreateChild(SecondDialogBox, 0x22);
    SecondDialogBox->SetButtonTexts(184, 185);

    NewWndF8 = new NCMacroListWnd;
    NewWndF8->SetWindowRect(60, 20, 240, 365);
    CreateChild(NewWndF8, 0x3002);
    NewWndF8->HideWindow();
    NewWndF8->Unknown78 = ++GNextWndNumber;
    NewWndF8->SetTitle(*GL2GameData.SysString[711]);

    NewWnd100 = new NCMacroInfoWnd;
    NewWnd100->SetWindowRect(100, 100, 240, 182);
    CreateChild(NewWnd100, 2);
    NewWnd100->HideWindow();
    NewWnd100->Unknown78 = ++GNextWndNumber;
    NewWnd100->SetTitle(*GL2GameData.SysString[657]);

    NewWndFC = new NCMacroEditWnd;
    NewWndFC->SetWindowRect(350, 20, 240, 365);
    CreateChild(NewWndFC, 0x3002);
    NewWndFC->HideWindow();
    NewWndFC->Unknown78 = ++GNextWndNumber;
    NewWndFC->SetTitle(*GL2GameData.SysString[711]);

    ShortCutWnd = new NCShortCutWnd(10, 10, 32);
    ShortCutWnd->SetWindowRect(Width - 45, (DWORD)Height / 2 - 218, 44, 437);
    CreateChildHidden(this, ShortCutWnd, 0x1022);
    ShortCutWnd->Unknown78 = ++GNextWndNumber;

    TrainWnd = new NCTrainWnd;
    TrainWnd->SetWindowRect(ScreenW - 258, 64, 240, 365);
    CreateChildHidden(this, TrainWnd, 0x1002);
    TrainWnd->Unknown78 = ++GNextWndNumber;

    MatchWnd = new NCMatchWnd;
    MatchWnd->SetWindowRect(ScreenW - 350, 215, 300, 277);
    CreateChildHidden(this, MatchWnd, 0x3002);
    MatchWnd->Unknown78 = ++GNextWndNumber;
    MatchWnd->SetTitle(*GL2GameData.SysString[389]);

    MatchOptionWnd = new NCMatchOptionWnd;
    MatchOptionWnd->SetWindowRect(HalfW - 120, HalfH - 80, 240, 161);
    CreateChildHidden(this, MatchOptionWnd, 2);

    MinimapWnd = new NCMinimapWnd;
    MinimapWnd->SetWindowRect(HalfW - 167, HalfH - 178, 334, 357);
    CreateChildHidden(this, MinimapWnd, 0x3002);
    MinimapWnd->Unknown78 = ++GNextWndNumber;
    MinimapWnd->SetTitle(*GL2GameData.SysString[447]);

    NewWnd194 = new NTownMapWnd;
    NewWnd194->SetWindowRect(HalfW - 167, HalfH - 178, 334, 357);
    CreateChildHidden(this, NewWnd194, 0x1002);
    NewWnd194->Unknown78 = ++GNextWndNumber;
    NewWnd194->SetTitle(*GL2GameData.SysString[447]);

    NewWnd198 = new NCObserverWnd;
    NewWnd198->SetWindowRect(HalfW - 60, 0, 120, 55);
    CreateChildHidden(this, NewWnd198, 0x5002);
    NewWnd198->Unknown78 = ++GNextWndNumber;
    NewWnd198->SetTitle(*GL2GameData.SysString[664]);

    PrivateShopWnd = new NCPrivateShopWnd;
    PrivateShopWnd->SetWindowRect(ScreenW - 290, 205, 240, 365);
    CreateChild(PrivateShopWnd, 0x1002);
    PrivateShopWnd->HideWindow();
    PrivateShopWnd->Unknown78 = ++GNextWndNumber;
    PrivateShopWnd->SetTitle(*GL2GameData.SysString[596]);

    PrivateBuyWnd = new NCPrivateBuyWnd;
    PrivateBuyWnd->SetWindowRect(ScreenW - 290, 205, 240, 365);
    CreateChild(PrivateBuyWnd, 0x1002);
    PrivateBuyWnd->HideWindow();
    PrivateBuyWnd->SetTitle(*GL2GameData.SysString[597]);

    TutorialWnd = new NCTutorialViewerWnd;
    TutorialWnd->SetWindowRect(ScreenW - 300, 0, 300, 533);
    CreateChild(TutorialWnd, 0x5002);
    TutorialWnd->HideWindow();
    TutorialWnd->SetTitle(*GL2GameData.SysString[448]);

    TutorialQuestionWnd = new NCTutorialQuestionWnd;
    TutorialQuestionWnd->SetWindowRect(5, ScreenH - 218, 32, 32);
    CreateChild(TutorialQuestionWnd, 0x1002);
    TutorialQuestionWnd->HideWindow();

    RadarWnd = new NCRadarWnd;
    RadarWnd->SetWindowRect(ScreenW - 102, 0, 102, 102);
    CreateChild(RadarWnd, 0xa);

    PetWnd = new NCPetWnd;
    PetWnd->SetWindowRect(207, 50, 240, 213);
    CreateChild(PetWnd, 0x3002);
    PetWnd->HideWindow();
    PetWnd->SetTitle(*GL2GameData.SysString[494]);

    SummonedWnd = new NCSummonedWnd;
    SummonedWnd->SetWindowRect(207, 50, 240, 213);
    CreateChild(SummonedWnd, 0x3002);
    SummonedWnd->HideWindow();
    SummonedWnd->SetTitle(*GL2GameData.SysString[495]);

    ChatWnd = new NCChatWnd;
    ChatWnd->SetWindowRect(0, Height - 173, 345, 174);
    CreateChild(ChatWnd, 0x100a);
    ChatWnd->Unknown78 = ++GNextWndNumber;

    NewWnd134 = new NCRecipeBookWnd;
    NewWnd134->SetWindowRect(ScreenW - 290, 205, 240, 365);
    CreateChild(NewWnd134, 0x1002);
    NewWnd134->SetTitle(*GL2GameData.SysString[663]);

    NewWnd138 = new NCRecipeTreeWnd;
    NewWnd138->SetWindowRect(ScreenW - 260, 150, 240, 365);
    CreateChild(NewWnd138, 0x1002);
    NewWnd138->SetTitle(*GL2GameData.SysString[662]);

    NewWnd13C = new NCRecipeManufactureWnd;
    NewWnd13C->SetWindowRect(ScreenW - 290, 205, 240, 365);
    CreateChild(NewWnd13C, 0x1002);
    NewWnd13C->SetTitle(*GL2GameData.SysString[663]);

    RecipeShopWnd = new NCRecipeShopWnd;
    RecipeShopWnd->SetWindowRect(ScreenW - 290, 205, 240, 365);
    CreateChild(RecipeShopWnd, 0x1002);
    RecipeShopWnd->SetTitle(*GL2GameData.SysString[663]);

    NewWnd144 = new NCRecipeBuyListWnd;
    NewWnd144->SetWindowRect(ScreenW - 290, 205, 240, 365);
    CreateChild(NewWnd144, 0x1002);
    NewWnd144->SetTitle(*GL2GameData.SysString[663]);

    RecipeBuyManufactureWnd = new NCRecipeBuyManufactureWnd;
    RecipeBuyManufactureWnd->SetWindowRect(ScreenW - 290, 205, 240, 365);
    CreateChild(RecipeBuyManufactureWnd, 0x1002);
    RecipeBuyManufactureWnd->SetTitle(*GL2GameData.SysString[663]);

    NewWnd14C = new NCHennaListWnd;
    NewWnd14C->SetWindowRect(ScreenW - 290, 205, 240, 365);
    CreateChild(NewWnd14C, 0x1002);

    NewWnd150 = new NCHennaInfoWnd;
    NewWnd150->SetWindowRect(ScreenW - 290, 205, 240, 365);
    CreateChild(NewWnd150, 0x1002);

    NewWndE4 = new NCManorShopWnd;
    NewWndE4->SetWindowRect(ScreenW - 290, 205, 240, 365);
    CreateChildCall(this, NewWndE4, 0x1002);
    NewWndE4->Unknown78 = ++GNextWndNumber;
    NewWndE4->SetTitle(*GL2GameData.SysString[738]);

    GMWnd = new NCGMWnd;
    GMWnd->SetWindowRect(Width - 209, Height - 327, 208, 229);
    CreateHiddenChild(this, GMWnd, 2);
    GMWnd->Unknown78 = ++GNextWndNumber;

    GMInventoryWnd = new NCGMInventoryWnd;
    GMInventoryWnd->SetWindowRect(ScreenW - 209, 100, 240, 506);
    CreateChildCall(this, GMInventoryWnd, 2);
    GMInventoryWnd->Unknown78 = ++GNextWndNumber;

    GMMagicSkillWnd = new NCGMMagicSkillWnd;
    GMMagicSkillWnd->SetWindowRect(ScreenW - 209, 100, 240, 313);
    CreateChildCall(this, GMMagicSkillWnd, 2);
    GMMagicSkillWnd->Unknown78 = ++GNextWndNumber;

    GMDetailStatusWnd = new NCGMDetailStatusWnd;
    GMDetailStatusWnd->SetWindowRect(0, 100, 240, 321);
    CreateChildCall(this, GMDetailStatusWnd, 2);
    GMDetailStatusWnd->Unknown78 = ++GNextWndNumber;

    GMQuestWnd = new NCGMQuestWnd;
    GMQuestWnd->SetWindowRect(ScreenW - 209, 100, 240, 313);
    CreateChildCall(this, GMQuestWnd, 0x22);
    GMQuestWnd->Unknown78 = ++GNextWndNumber;

    GMStoreWnd = new NCGMStoreWnd;
    GMStoreWnd->SetWindowRect(ScreenW - 209, 100, 240, 506);
    CreateChildCall(this, GMStoreWnd, 0x22);
    GMStoreWnd->Unknown78 = ++GNextWndNumber;

    GMActionWnd = new NCGMActionWnd;
    GMActionWnd->SetWindowRect(ScreenW - 209, 100, 240, 313);
    CreateChildCall(this, GMActionWnd, 2);
    GMActionWnd->Unknown78 = ++GNextWndNumber;

    SiegeInfoWnd = new NCSiegeInfoMainWnd;
    SiegeInfoWnd->SetWindowRect(200, 100, 240, 365);
    CreateChildCall(this, SiegeInfoWnd, 0x1002);
    SiegeInfoWnd->SetTitle(*GL2GameData.SysString[593]);

    NewWnd19C = new NCXMasSealWnd;
    NewWnd19C->SetWindowRect(HalfW - 167, HalfH - 178, 334, 354);
    CreateHiddenChild(this, NewWnd19C, 0x3002);
    NewWnd19C->Unknown78 = ++GNextWndNumber;
    NewWnd19C->SetTitle(*GL2GameData.SysString[2]);
    if (GIsOnlyL2Console)
    {
        TestNpcHtmlWnd = new NCNPCHtmlViewer;
        TestNpcHtmlWnd->SetWindowRect(60, 20, 646, 483);
        CreateHiddenChild(this, TestNpcHtmlWnd, 0x1022);
        TestNpcHtmlWnd->HtmlViewer->LoadHtmlFile((TCHAR*)TEXT("test.html"));
    }
    if (GIsTestHtml)
    {
        TestHtmlWnd = new NCTestHtmlViewer;
        TestHtmlWnd->SetWindowRect(HalfW, HalfH, 208, 412);
        CreateHiddenChild(this, TestHtmlWnd, 0x22);
    }

    BlackTex = FindTexture(TEXT("NWindow.BlackTexture"));
    BalloonTex = FindTexture(TEXT("NWindow.chat"));
    BalloonTimer = SetTimer(NTIMER_BALLOONS, 1000, NULL);
    return 1;
}
NW_HOOK_METHOD(0x1004c900, NCConsole, OnCreate)

// a child window goes (repeated inline in the stock OnDestroy)
template <class T> static FORCEINLINE void DestroyChild(NCWnd* Parent, T*& Child)
{
    if (Child)
    {
        Parent->RemoveChildWindow(Child->Entry);
        delete Child;
        Child = NULL;
    }
}

// 0x10050b60 (vslot 37) - the balloons and every window but the pet and summon status ones
// (never destroyed), then the timer (C2: the order of the stock, with the new windows)
INT NCConsole::OnDestroy()
{
    for (INT i = 0; i < Balloons.Num(); i++)
        if (Balloons(i).Text)
            delete[] Balloons(i).Text;
    Balloons.Empty();
    DestroyChild(this, ChatWnd);
    DestroyChild(this, NpcHtmlWnd);
    DestroyChild(this, HelpWnd);
    DestroyChild(this, GMPetitionWnd);
    DestroyChild(this, BoardWnd);
    DestroyChild(this, NewWndF8);
    DestroyChild(this, NewWnd100);
    DestroyChild(this, NewWndFC);
    DestroyChild(this, ShortCutWnd);
    DestroyChild(this, InventoryWnd);
    DestroyChild(this, TargetStatusWnd);
    DestroyChild(this, PlayerStatusWnd);
    DestroyChild(this, AbnormalStatusWnd);
    DestroyChild(this, StoreWnd);
    DestroyChild(this, ShopWnd);
    DestroyChild(this, VIPShopWnd);
    DestroyChild(this, MenuWnd);
    DestroyChild(this, SystemMenuWnd);
    DestroyChild(this, TradeWnd);
    DestroyChild(this, MainDialogBox);
    DestroyChild(this, SecondDialogBox);
    DestroyChild(this, TrainWnd);
    DestroyChild(this, MatchWnd);
    DestroyChild(this, MatchOptionWnd);
    DestroyChild(this, MinimapWnd);
    DestroyChild(this, NewWnd194);
    DestroyChild(this, PetWnd);
    DestroyChild(this, SummonedWnd);
    DestroyChild(this, NewWnd134);
    DestroyChild(this, NewWnd138);
    DestroyChild(this, NewWnd13C);
    DestroyChild(this, RecipeShopWnd);
    DestroyChild(this, NewWnd144);
    DestroyChild(this, RecipeBuyManufactureWnd);
    DestroyChild(this, NewWnd14C);
    DestroyChild(this, NewWnd150);
    DestroyChild(this, GMWnd);
    DestroyChild(this, GMInventoryWnd);
    DestroyChild(this, GMMagicSkillWnd);
    DestroyChild(this, GMDetailStatusWnd);
    DestroyChild(this, GMQuestWnd);
    DestroyChild(this, GMStoreWnd);
    DestroyChild(this, GMActionWnd);
    DestroyChild(this, PrivateShopWnd);
    DestroyChild(this, PrivateBuyWnd);
    DestroyChild(this, MainWnd);
    DestroyChild(this, PartyWnd);
    DestroyChild(this, TutorialWnd);
    DestroyChild(this, TutorialQuestionWnd);
    DestroyChild(this, RadarWnd);
    DestroyChild(this, DeliverWnd);
    DestroyChild(this, SelectDeliverWnd);
    DestroyChild(this, SiegeInfoWnd);
    DestroyChild(this, GMSnoopManager);
    DestroyChild(this, PetitionBeginWnd);
    DestroyChild(this, CalcWnd);
    DestroyChild(this, EmpowerWnd);
    DestroyChild(this, NewWndE4);
    DestroyChild(this, NewWnd19C);
    if (GIsOnlyL2Console)
        DestroyChild(this, TestNpcHtmlWnd);
    if (GIsTestHtml)
        DestroyChild(this, TestHtmlWnd);
    if (BalloonTimer)
    {
        KillTimer(BalloonTimer);
        BalloonTimer = 0;
    }
    NCWnd::OnDestroy();
    return 1;
}
NW_HOOK_METHOD(0x10050b60, NCConsole, OnDestroy)

// 0x1004f6b0 (vslot 43) - entering the game: the bars, the chat, the radar and the status
// windows show, the rest hides (C2: the party window is hidden when it has no members, the pet and summon
// status windows are still left; the observer window goes unless the player's controller has the flag at
// +0xA00 set; the new windows hide with the others)
void NCConsole::ShowWindow()
{
    NCWnd::ShowWindow();
    SiegeInfoWnd->HideWindow();
    DeliverWnd->HideWindow();
    SelectDeliverWnd->HideWindow();
    GMPetitionWnd->HideWindow();
    GConsole->PetitionWnd->HideWindow();
    MainWnd->HideWindow();
    BoardWnd->HideWindow();
    NewWndF8->HideWindow();
    NewWnd100->HideWindow();
    NewWndFC->HideWindow();
    HelpWnd->HideWindow();
    ShortCutWnd->ShowWindow();
    ChatWnd->ShowWindow();
    InventoryWnd->HideWindow();
    TargetStatusWnd->ShowWindow();
    AbnormalStatusWnd->ShowWindow();
    PlayerStatusWnd->ShowWindow();
    MainDialogBox->HideWindow();
    SecondDialogBox->HideWindow();
    MenuWnd->ShowWindow();
    SystemMenuWnd->HideWindow();
    StoreWnd->HideWindow();
    ShopWnd->HideWindow();
    VIPShopWnd->HideWindow();
    TradeWnd->HideWindow();
    NpcHtmlWnd->HideWindow();
    TrainWnd->HideWindow();
    MatchWnd->HideWindow();
    MatchOptionWnd->HideWindow();
    MinimapWnd->HideWindow();
    TutorialWnd->HideWindow();
    TutorialQuestionWnd->HideWindow();
    RadarWnd->ShowWindow();
    PetWnd->HideWindow();
    SummonedWnd->HideWindow();
    NewWnd134->HideWindow();
    NewWnd138->HideWindow();
    NewWnd13C->HideWindow();
    RecipeShopWnd->HideWindow();
    NewWnd144->HideWindow();
    RecipeBuyManufactureWnd->HideWindow();
    NewWnd14C->HideWindow();
    NewWnd150->HideWindow();
    GMWnd->HideWindow();
    GMInventoryWnd->HideWindow();
    GMMagicSkillWnd->HideWindow();
    GMDetailStatusWnd->HideWindow();
    GMQuestWnd->HideWindow();
    GMStoreWnd->HideWindow();
    GMActionWnd->HideWindow();
    PrivateShopWnd->HideWindow();
    PrivateBuyWnd->HideWindow();
    OptionWnd()->HideWindow();
    PetitionBeginWnd->HideWindow();
    CalcWnd->HideWindow();
    EmpowerWnd->HideWindow();
    NewWnd194->HideWindow();
    NewWndE4->HideWindow();
    NewWnd19C->HideWindow();
    if (PartyWnd && !PartyWnd->NumMembers)
        PartyWnd->HideWindow();
    if (NewWnd198)
    {
        APlayerController* Actor = GConsole->Engine->Client->Viewports(0)->Actor;
        ALineagePlayerController* Controller = (Actor && Actor->IsA(ALineagePlayerController::StaticClass()))
                                                   ? (ALineagePlayerController*)Actor : NULL;
        if (Controller && !(nw::Field<BYTE>(Controller, 0xA00) & 1))
            NewWnd198->HideWindow();
    }
    if (GIsOnlyL2Console)
    {
        TestNpcHtmlWnd->ShowWindow();
        NewWnd194->ShowWindow();
        NewWnd198->ShowWindow();
    }
    if (GIsTestHtml)
        TestHtmlWnd->ShowWindow();
}
NW_HOOK_METHOD(0x1004f6b0, NCConsole, ShowWindow)

// stock body shared with NCChatWnd's (0x1003dab0)
void NCConsole::HideWindow()
{
    NCWnd::HideWindow();
}

/*-----------------------------------------------------------------------------
    Toggles and resets (the menus, the shortcut keys, the console).
-----------------------------------------------------------------------------*/

// (repeated inline in the stock CloseWindows)
static FORCEINLINE void HideIfShown(NCWnd* Wnd)
{
    if (Wnd->IsShowWindow())
        Wnd->HideWindow();
}

// 0x1004b840 - the private stores are left (and closed on the server when set up), the recipe
// shop and the trade cancelled, the party matching left, the options and petition windows hidden,
// then the whole interface shown or hidden (C2: the recipe shop is new, and the trade test reads
// TradeWnd where the C1 one read TrainWnd)
void NCConsole::ToggleInterface()
{
    if (PrivateShopWnd->IsShowWindow())
    {
        if (PrivateShopWnd->Mode == 1)
            GConsole->RequestPrivateStoreQuitSell();
        PrivateShopWnd->Reset();
    }
    if (PrivateBuyWnd->IsShowWindow())
    {
        if (PrivateBuyWnd->Mode == 1)
            GConsole->RequestPrivateStoreQuitBuy();
        PrivateBuyWnd->Reset();  // the stock code calls the sell window's body for both
    }
    if (RecipeShopWnd->IsShowWindow())
    {
        GConsole->RequestRecipeShopManageQuit();
        static_cast<NCRecipeShopWnd*>(RecipeShopWnd)->ClearLists();   // its own reset
    }
    if (TradeWnd->IsShowWindow())
        GConsole->RequestTradeDone(0);
    if (MatchWnd->IsShowWindow())
        GConsole->RequestPartyMatchList(3);
    if (OptionWnd())
        OptionWnd()->HideWindow();
    if (GConsole->PetitionWnd)
        GConsole->PetitionWnd->HideWindow();
    if (IsShowWindow())
        HideWindow();
    else
        ShowWindow();
}
NW_HOOK_METHOD(0x1004b840, NCConsole, ToggleInterface)

// 0x1004b970 - the dialogs, shops, stores, menus and the radar close
void NCConsole::CloseWindows()
{
    HideIfShown(InventoryWnd);
    HideIfShown(ShopWnd);
    HideIfShown(VIPShopWnd);
    HideIfShown(StoreWnd);
    HideIfShown(TradeWnd);
    HideIfShown(SystemMenuWnd);
    HideIfShown(HelpWnd);
    HideIfShown(MainDialogBox);
    HideIfShown(SecondDialogBox);
    HideIfShown(RadarWnd);
    HideIfShown(PrivateShopWnd);
    HideIfShown(PrivateBuyWnd);
    HideIfShown(MainWnd);
    HideIfShown(GConsole->PetitionWnd);
    HideIfShown(GMPetitionWnd);
    HideIfShown(DeliverWnd);
    HideIfShown(SelectDeliverWnd);
    HideIfShown(SiegeInfoWnd);
}
NW_HOOK_METHOD(0x1004b970, NCConsole, CloseWindows)

// 0x1004bbc0 - leaving the game: the dialogs, the chat, the shortcuts, the buffs, the party
// matching list, the clan page, the radar and both petition windows empty
void NCConsole::Reset()
{
    MainDialogBox->Reset();
    SecondDialogBox->Reset();
    ChatWnd->ClearAll();
    ShortCutWnd->ClearShortCuts();
    AbnormalStatusWnd->ClearStatus();
    MatchWnd->ClearList();
    MainWnd->ActionWnd->ResetPledgeWnd();
    RadarWnd->ClearTargets();
    GMPetitionWnd->Clear();
    PlayerPetitionWnd()->Clear();
}
NW_HOOK_METHOD(0x1004bbc0, NCConsole, Reset)

// 0x1003de90  NOT PORTED (no C2 counterpart)
void NCConsole::TogglePetition()
{
    NW_UNPORTED_NORET();
}

// 0x1004bc40
void NCConsole::ToggleSystemMenu()
{
    if (SystemMenuWnd->IsShowWindow())
    {
        GConsole->PlayInterfaceSound(SOUND_WindowClose);
        SystemMenuWnd->HideWindow();
    }
    else
    {
        GConsole->PlayInterfaceSound(SOUND_WindowOpen);
        SystemMenuWnd->ShowWindow();
        SystemMenuWnd->SetFocus();
    }
}
NW_HOOK_METHOD(0x1004bc40, NCConsole, ToggleSystemMenu)

// 0x1004bca0 - not while a shop (mode 1), a warehouse (mode 0), a trade or the freight is
// open; opening asks the server for the item list (its answer shows the window)
void NCConsole::ToggleInventory()
{
    if (ShopWnd->IsShowWindow() && StoreWndMode(ShopWnd) == 1)
        return;
    if (StoreWnd->IsShowWindow() && !StoreWndMode(StoreWnd))
        return;
    if (TradeWnd->IsShowWindow() || DeliverWnd->IsShowWindow())
        return;
    if (InventoryWnd->IsShowWindow())
    {
        GConsole->PlayInterfaceSound(SOUND_WindowClose);
        InventoryWnd->HideWindow();
    }
    else
    {
        GConsole->RequestItemList();
        GConsole->PlayInterfaceSound(SOUND_WindowOpen);
    }
}
NW_HOOK_METHOD(0x1004bca0, NCConsole, ToggleInventory)

// 0x1004bd50
void NCConsole::ToggleChat()
{
    if (ChatWnd->IsShowWindow())
        ChatWnd->HideWindow();
    else
        ChatWnd->ShowWindow();
}
NW_HOOK_METHOD(0x1004bd50, NCConsole, ToggleChat)

// 0x1004bd80 - the help page; bMacro (C2, name ours) opens the macro help instead
void NCConsole::ToggleHelp(UBOOL bMacro)
{
    if (HelpWnd->IsShowWindow())
    {
        HelpWnd->HideWindow();
        return;
    }
    HelpWnd->LoadFile((TCHAR*)(bMacro ? TEXT("..\\L2text\\help_macro.htm") : TEXT("..\\L2text\\help.htm")));
    HelpWnd->ShowWindow();
    HelpWnd->SetFocus();
}
NW_HOOK_METHOD(0x1004bd80, NCConsole, ToggleHelp)

// 0x1004bde0 - closing leaves the matching room (3); opening sends the player's saved
// options (MatchOption.ini, section = the character's name) and joins as registered or not
void NCConsole::TogglePartyMatch()
{
    if (MatchWnd->IsShowWindow())
    {
        GConsole->RequestPartyMatchList(3);
        MatchWnd->HideWindow();
        return;
    }
    if (!GConsole->GetMyUser())
        return;
    const TCHAR* Section = GConsole->GetMyUser()->GetName();
    UBOOL bAutoRegister, bLevelOpen, bClassOpen;
    TCHAR Memo[256];
    if (!GConfig->GetBool(Section, TEXT("autoregister"), bAutoRegister, TEXT("MatchOption.ini")))
        bAutoRegister = 1;
    if (!GConfig->GetBool(Section, TEXT("levelopen"), bLevelOpen, TEXT("MatchOption.ini")))
        bLevelOpen = 0;
    if (!GConfig->GetBool(Section, TEXT("classopen"), bClassOpen, TEXT("MatchOption.ini")))
        bClassOpen = 0;
    if (!GConfig->GetString(Section, TEXT("memo"), Memo, 256, TEXT("MatchOption.ini")))
        Memo[0] = 0;
    L2ParamStack Params(10);
    Params.PushBack((void*)(INT_PTR)bAutoRegister);
    Params.PushBack((void*)(INT_PTR)bLevelOpen);
    Params.PushBack((void*)(INT_PTR)bClassOpen);
    Params.PushBack(Memo);
    GConsole->RequestPartyMatchConfig(Params);
    GConsole->RequestPartyMatchList(bAutoRegister ? 1 : 0);
    MatchOptionWnd->LoadOptions();
    MatchWnd->ShowWindow();
}
NW_HOOK_METHOD(0x1004bde0, NCConsole, TogglePartyMatch)

// 0x1004bfc0 - shown centered on the player
void NCConsole::ToggleMinimap()
{
    if (MinimapWnd->IsShowWindow())
    {
        GConsole->PlayInterfaceSound(SOUND_Minimap);
        MinimapWnd->HideWindow();
    }
    else
    {
        MinimapWnd->ShowWindow();
        MinimapWnd->CenterOnPlayer();
        GConsole->PlayInterfaceSound(SOUND_Minimap);
    }
}
NW_HOOK_METHOD(0x1004bfc0, NCConsole, ToggleMinimap)

// 0x1004c020
void NCConsole::ToggleMatchOption()
{
    if (MatchOptionWnd->IsShowWindow())
        MatchOptionWnd->HideWindow();
    else
        MatchOptionWnd->ShowWindow();
}
NW_HOOK_METHOD(0x1004c020, NCConsole, ToggleMatchOption)

// 0x1004c050 - opening asks the server for the board (its answer shows the window)
void NCConsole::ToggleBoard()
{
    if (BoardWnd->IsShowWindow())
    {
        GConsole->PlayInterfaceSound(SOUND_WindowClose);
        BoardWnd->HideWindow();
        return;
    }
    L2ParamStack Params(10);
    GConsole->RequestShowBoard(Params);
    GConsole->PlayInterfaceSound(SOUND_WindowOpen);
}
NW_HOOK_METHOD(0x1004c050, NCConsole, ToggleBoard)

// 0x1004c100 - the macro window (+0xF8): shown and focused with the sound of opening a window (5), or hidden with the
// sound of closing it (6)
void NCConsole::ToggleMacroList()
{
    if (NewWndF8->IsShowWindow())
    {
        GConsole->PlayInterfaceSound(6);
        NewWndF8->HideWindow();
        return;
    }
    NewWndF8->ShowWindow();
    NewWndF8->SetFocus();
    GConsole->PlayInterfaceSound(5);
}
NW_HOOK_METHOD(0x1004c100, NCConsole, ToggleMacroList)

// 0x1004c160 - the macro editor (+0xFC), the same way
void NCConsole::ToggleMacroEdit()
{
    if (NewWndFC->IsShowWindow())
    {
        GConsole->PlayInterfaceSound(6);
        NewWndFC->HideWindow();
        return;
    }
    NewWndFC->ShowWindow();
    NewWndFC->SetFocus();
    GConsole->PlayInterfaceSound(5);
}
NW_HOOK_METHOD(0x1004c160, NCConsole, ToggleMacroEdit)

// 0x1004c440 - the Christmas seal window (+0x19C): hidden when shown, shown when hidden (tail calls)
void NCConsole::ToggleXMasSeal()
{
    if (NewWnd19C->IsShowWindow())
        NewWnd19C->HideWindow();
    else
        NewWnd19C->ShowWindow();
}
NW_HOOK_METHOD(0x1004c440, NCConsole, ToggleXMasSeal)

// 0x1004c1c0 - the GM window's name box takes the focus
void NCConsole::ToggleGMWnd()
{
    if (GMWnd->IsShowWindow())
    {
        GMWnd->HideWindow();
        return;
    }
    GMWnd->ShowWindow();
    GMWnd->NameEdit->SetFocus();
}
NW_HOOK_METHOD(0x1004c1c0, NCConsole, ToggleGMWnd)

// the GM view windows: hidden, or the server asked about the player Name and the window
// shown (repeated in the six stock toggles)
#define NCCONSOLE_GM_TOGGLE(Method, Wnd, Request)                                             \
    void NCConsole::Method(TCHAR* Name)                                                       \
    {                                                                                         \
        if (Wnd->IsShowWindow())                                                              \
        {                                                                                     \
            Wnd->HideWindow();                                                                \
            return;                                                                           \
        }                                                                                     \
        GConsole->Request(Name);                                                              \
        Wnd->ShowWindow();                                                                    \
        Wnd->SetFocus();                                                                      \
    }

// 0x1004c200
NCCONSOLE_GM_TOGGLE(ToggleGMInventory, GMInventoryWnd, RequestGMInventory)
NW_HOOK_METHOD(0x1004c200, NCConsole, ToggleGMInventory)
// 0x1004c260
NCCONSOLE_GM_TOGGLE(ToggleGMMagicSkill, GMMagicSkillWnd, RequestGMSkills)
NW_HOOK_METHOD(0x1004c260, NCConsole, ToggleGMMagicSkill)
// 0x1004c2c0
NCCONSOLE_GM_TOGGLE(ToggleGMDetailStatus, GMDetailStatusWnd, RequestGMStatus)
NW_HOOK_METHOD(0x1004c2c0, NCConsole, ToggleGMDetailStatus)
// 0x1004c320
NCCONSOLE_GM_TOGGLE(ToggleGMQuest, GMQuestWnd, RequestGMQuests)
NW_HOOK_METHOD(0x1004c320, NCConsole, ToggleGMQuest)
// 0x1004c380
NCCONSOLE_GM_TOGGLE(ToggleGMStore, GMStoreWnd, RequestGMWarehouse)
NW_HOOK_METHOD(0x1004c380, NCConsole, ToggleGMStore)
// 0x1004c3e0
NCCONSOLE_GM_TOGGLE(ToggleGMAction, GMActionWnd, RequestGMPledge)
NW_HOOK_METHOD(0x1004c3e0, NCConsole, ToggleGMAction)

// 0x100507b0 - a tab of the character window (NCMainWnd::ShowTab), or with -1 the window
// shown or hidden
void NCConsole::ToggleCharacter(INT Tab)
{
    if (Tab != -1)
    {
        MainWnd->ShowTab(Tab);
        return;
    }
    if (MainWnd->IsShowWindow())
    {
        GConsole->PlayInterfaceSound(SOUND_WindowClose);
        MainWnd->HideWindow();
    }
    else
    {
        MainWnd->ShowWindow();
        GConsole->PlayInterfaceSound(SOUND_WindowOpen);
    }
}
NW_HOOK_METHOD(0x100507b0, NCConsole, ToggleCharacter)

/*-----------------------------------------------------------------------------
    Chat.
-----------------------------------------------------------------------------*/

// 0x1004c470 - a systemmsg line (type 5), when the message has a text
void NCConsole::AddSystemMessage(INT MsgID, DWORD Color)
{
    if (*GL2GameData.SystemMsg[MsgID].Message)
        ChatWnd->AddChatMessage(*GL2GameData.SystemMsg[MsgID].Message, Color, 5, 0);
}
NW_HOOK_METHOD(0x1004c470, NCConsole, AddSystemMessage)

// 0x1004c4c0 - a system line (type 5), when not empty
void NCConsole::AddSystemText(const TCHAR* Text, INT SubType, DWORD Color)
{
    if (Text && *Text)
        ChatWnd->AddChatMessage(Text, Color, 5, SubType);
}
NW_HOOK_METHOD(0x1004c4c0, NCConsole, AddSystemText)

// 0x10050820 - "Name: Text" of the chat types but the petition ones (6, 7: into the GM
// petition window, or the player's, shown, when the GM one is hidden). Then the speaker's
// balloon: its first 19 characters ("..." after a longer text) for 5 seconds, a newer one
// replacing it - not when the speaker's User +0x1F8 (16 bits) is set.
void NCConsole::Say(INT ObjectID, const TCHAR* Name, const TCHAR* Text, DWORD Color, INT Type)
{
    TCHAR Line[1024];
    _swprintf(Line, TEXT("%s: %s"), Name, Text);
    if (Type == 6 || Type == 7)
    {
        GMPetitionWnd->AddMessage(Line, Color, Type, 0);
        if (!GMPetitionWnd->IsShowWindow())
        {
            GConsole->PetitionWnd->ShowWindow();
            PlayerPetitionWnd()->AddMessage(Line, Color, Type, 0);
        }
        return;
    }
    ChatWnd->AddChatMessage(Line, Color, Type, 0);
    User* Speaker = GConsole->FindUserByID(ObjectID);
    if (Speaker && Speaker->PrivateStoreType)
        return;
    NChatBalloon Balloon;
    TCHAR* Short = new TCHAR[23];
    if (wcslen(Text) < 19)
        appStrcpy(Short, Text);
    else
    {
        wcsncpy(Short, Text, 19);
        Short[19] = '.';
        Short[20] = '.';
        Short[21] = '.';
        Short[22] = 0;
    }
    Balloon.Tick = Ticks;
    Balloon.ObjectID = ObjectID;
    Balloon.Text = Short;
    INT i;
    for (i = 0; i < Balloons.Num(); i++)
        if (Balloons(i).ObjectID == ObjectID)
            break;
    if (i < Balloons.Num())
    {
        if (Balloons(i).Text)
            delete[] Balloons(i).Text;
        Balloons(i) = Balloon;
    }
    else
        Balloons(Balloons.Add()) = Balloon;
    Speaker = GConsole->FindUserByID(ObjectID);
    if (Speaker && Speaker->Pawn)
    {
        if (!Speaker->Pawn->ChatBalloon)
            Speaker->Pawn->Unknown5C++;
        Speaker->Pawn->ChatBalloon = Short;
    }
}
NW_HOOK_METHOD(0x10050820, NCConsole, Say)

/*-----------------------------------------------------------------------------
    HTML page commands.
-----------------------------------------------------------------------------*/

// 0x1004fab0 (vslot 67) - "Link <bypass>": to the server (the tutorial pages' links go their
// own way). "Write <board> <a> <b> <control> [<control> [<control>]]": a board post, the texts
// of the page's controls named; boards 4, 5 and 7 take "-" for the second text, 4 and 7 for
// the third. A missing third text still sends the post, and returns 0.
INT NCConsole::OnHtmlCommand(const TCHAR* Command, NCHtmlViewer* Source)
{
    const TCHAR* Str = Command;
    if (ParseCommand(&Str, TEXT("Link")))
    {
        TCHAR Bypass[256] = { 0 };
        if (ParseToken(Str, Bypass, 256, 1))
        {
            L2ParamStack Params(10);
            Params.PushBack(Bypass);
            if (Source->IsA(NClass::NCTutorialHtmlViewer()))
                GConsole->RequestTutorialLinkHtml(Params);
            else
                GConsole->RequestLinkHtml(Params);
        }
        return 1;
    }
    if (!ParseCommand(&Str, TEXT("Write")))
        return 0;
    L2ParamStack Params(10);
    TCHAR Board[256], Arg1[256], Arg2[256], Control1[256], Control2[256], Control3[256];
    if (!ParseToken(Str, Board, 256, 1))
        return 0;
    Params.PushBack(Board);
    if (!ParseToken(Str, Arg1, 256, 1))
        return 0;
    Params.PushBack(Arg1);
    if (!ParseToken(Str, Arg2, 256, 1))
        return 0;
    Params.PushBack(Arg2);
    if (!ParseToken(Str, Control1, 256, 1) || !Source)
        return 0;
    TCHAR* Text = Source->GetControlValue(Control1);
    if (!Text)
        return 0;
    Params.PushBack(Text);
    if (appStrcmp(Board, TEXT("4")) && appStrcmp(Board, TEXT("5")) && appStrcmp(Board, TEXT("7")))
    {
        if (!ParseToken(Str, Control2, 256, 1))
            return 0;
        Text = Source->GetControlValue(Control2);
        if (!Text)
            return 0;
        Params.PushBack(Text);
    }
    else
    {
        appStrcpy(Control2, TEXT("-"));
        Params.PushBack(Control2);
    }
    if (appStrcmp(Board, TEXT("4")) && appStrcmp(Board, TEXT("7")))
    {
        if (ParseToken(Str, Control3, 256, 1))
        {
            Text = Source->GetControlValue(Control3);
            if (!Text)
            {
                GConsole->RequestBBSWrite(Params);
                return 0;
            }
            Params.PushBack(Text);
        }
    }
    else
    {
        appStrcpy(Control3, TEXT("-"));
        Params.PushBack(Control3);
    }
    GConsole->RequestBBSWrite(Params);
    return 1;
}
NW_HOOK_METHOD(0x1004fab0, NCConsole, OnHtmlCommand)

// 0x1004c5f0 - a balloon's destructor (the text is freed by hand; Say's unwind code)
NW_HOOK_DTOR(0x1004c5f0, NChatBalloon)

NW_CHECK_VTABLE(NCConsole, 0x10113898, 68)
