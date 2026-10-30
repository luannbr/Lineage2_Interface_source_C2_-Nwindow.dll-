/*=============================================================================
    NConsoleWndLogin.cpp - what the engine tells the console before the game and around it
    (UGameEngine::On* handlers, one slot each): the login and its failures, the announcements,
    the server list, the character selection and creation, entering the world, the restart,
    the logout, a closed server, a blocked account, the protocol version.
=============================================================================*/
#include "nwindow/NConsoleWnd.h"
#include "nwindow/NCConsole.h"
#include "nwindow/NDialogInfo.h"
#include "nwindow/NCDialogBox.h"
#include "nwindow/NCLoginWnd.h"
#include "nwindow/NCAuthWnd.h"
#include "nwindow/NCEulaWnd.h"
#include "nwindow/NCNoticeWnd.h"
#include "nwindow/NCWarningWnd.h"
#include "nwindow/NCServerSelectWnd.h"
#include "nwindow/NCEditBox.h"
#include "nwindow/NCLobbyWnd.h"
#include "nwindow/NCPawnCreateWnd.h"
#include "nwindow/NCLoadingWnd.h"
#include "nwindow/NCMessageWnd.h"
#include "nwindow/NCOptionWnd.h"
#include "nwindow/NCRestartMenuWnd.h"

// system messages (systemmsg ids)
enum
{
    MSG_Disconnected = 0,              // "Your connection has been disconnected from the server."
    MSG_TooManyCharacters = 77,        // "You cannot create another character. Please delete the
                                       //   existing character and try again."
    MSG_NameExists = 79,               // "Name already exists."
    MSG_NameInvalid = 80,              // "Please insert 1~16 English characters."
    MSG_ProtocolQuit = 88,             // "Protocol version is different. Please quit the program."
    MSG_ProtocolContinue = 89,         // "Protocol version is different. Please continue."
    MSG_DisconnectedRetry = 127,       // "Your connection has been disconnected from the server.
                                       //   Please try again."
    MSG_CreateFailed = 128,            // "You have failed to create a character."
    MSG_DeleteFailed = 306,            // "You have failed to delete the character."
    MSG_AskRevive = 332,               // "Do you want to be restored?"
    MSG_TemporaryPassword = 396,       // "Please login after changing your temporary password."
    MSG_NoTimeLeft = 398,              // "You have no more time left on your account."
    MSG_SystemError = 399,             // "System error."
    MSG_BlockReason = 428,             // .. 443: a reason each, a bit of BlockedAccount's
    MSG_Withdrawn = 444,               // "The following account may not be accessed due to a
                                       //   withdrawal request that has been submitted." and
                                       //   445..447 after it
    MSG_LoginSystemError = 448,        // "System error, please log in again later."
    MSG_WrongPassword = 449,           // "Password does not match this acount."
    MSG_ConfirmAccount = 450,          // "Confirm your account information and log in again later."
    MSG_WrongAccount = 453,            // "Your account information is incorrect."
    MSG_ContactSupport = 454,          // "Please contact the Customer Support Center or the 1: 1
                                       //   inquiry service of the Lineage II official website."
    MSG_AccountInUse = 455,            // "The account is already in use. Access denied."
    MSG_AdultsOnly = 456,              // "Lineage II game services may only be used by adults 18
                                       //   years of age and older."
    MSG_Maintenance = 457,             // "Server under maintenance. Please try again later."
    MSG_TermExpired = 458,             // "Your usage term has been expired." 459, 460 after it
    MSG_AccessFailed = 461,            // "Access failed." 462, 463 after it
    MSG_ClanMemberDelete = 540,        // "Clan leaders cannot be deleted. DD Dissolve the clan and
                                       //   try again."
    MSG_ClanLeaderDelete = 541,        // "You cannot delete a clan member. Withdraw from the clan
                                       //   and try again."
    MSG_KoreaOnly = 621,               // "This server is reserved for players in Korea..."
    MSG_WeekTimeOver = 756,            // "This week's usage time has finished." (no reason of C2's AuthLoginFail has it)
    MSG_LoginReason31 = 1243,          // C2: the reasons 31 and 32 of AuthLoginFail (the security card's;
    MSG_LoginReason32 = 1242,          //   the texts are not known here)
};

// the dialog boxes opened here (NDialogInfo::ID; NCDialogBox sends the answer)
enum
{
    DIALOG_DISCONNECTED = 0x1D,        // OK only: the game ends
    DIALOG_REVIVE = 0x20,
};

// a system message's text
static FORCEINLINE const TCHAR* SysMsg(INT ID)
{
    return *GL2GameData.SystemMsg[ID].Message;
}

// the login box's mode, card and card key cleared, and the account line shown and given the focus
// (C2: stock 0x10057CD0, a function of its own that StartNotice calls and the rest of the login
// screen inlines)
static FORCEINLINE void ResetAuthWnd(NCAuthWnd* Auth)
{
    Auth->Mode = 0;
    Auth->CardID = 0;
    Auth->CardPos = 0;
    Auth->AccountEdit->ShowWindow();
    Auth->AccountEdit->SetFocus();
}

// back to the login box, the server list and the other boxes of the login screen hidden (after the
// login failures); bReset: the box starts again (account mode)
static FORCEINLINE void ShowAuthWnd(NConsoleWnd* Console, UBOOL bReset = 1)
{
    NLOGIN_SERVERLIST(Console->LoginWnd)->HideWindow();
    Console->LoginWnd->NoticeWnd->HideWindow();
    Console->LoginWnd->WarningWnd->HideWindow();
    Console->LoginWnd->EulaWnd->HideWindow();      // C2: the two boxes it added
    Console->LoginWnd->AnnounceWnd->HideWindow();
    Console->LoginWnd->AuthWnd->ShowWindow();
    if (bReset)
        ResetAuthWnd(Console->LoginWnd->AuthWnd);
    Console->SetState(L2CS_Login);
}

/*-----------------------------------------------------------------------------
    The login.
-----------------------------------------------------------------------------*/

// 0x10058410 (vslot 32) - the login server refused: the reason's message, then the login box again (for
// the reasons without a message the stock shows an uninitialized buffer; empty here). C2: the reasons
// 31 and 32 (the security card's: 31 leaves the box as it is) and no 30
void NConsoleWnd::AuthLoginFail(INT Reason)
{
    TCHAR Text[256];
    Text[0] = 0;
    switch (Reason)
    {
    case 1:
        appSprintf(Text, TEXT("%s"), SysMsg(MSG_LoginSystemError));
        break;
    case 2:
    case 3:
        appSprintf(Text, TEXT("%s %s"), SysMsg(MSG_WrongPassword), SysMsg(MSG_ConfirmAccount));
        break;
    case 5:
        appSprintf(Text, TEXT("%s %s"), SysMsg(MSG_WrongAccount), SysMsg(MSG_ContactSupport));
        break;
    case 7:
        appSprintf(Text, TEXT("%s"), SysMsg(MSG_AccountInUse));
        break;
    case 12:
        appSprintf(Text, TEXT("%s"), SysMsg(MSG_AdultsOnly));
        break;
    case 16:
        appSprintf(Text, TEXT("%s"), SysMsg(MSG_Maintenance));
        break;
    case 17:
        appSprintf(Text, TEXT("%s"), SysMsg(MSG_TemporaryPassword));
        break;
    case 18:
        appSprintf(Text, TEXT("%s %s %s"), SysMsg(MSG_TermExpired), SysMsg(MSG_TermExpired + 1),
                   SysMsg(MSG_TermExpired + 2));
        break;
    case 19:
        appSprintf(Text, TEXT("%s"), SysMsg(MSG_NoTimeLeft));
        break;
    case 20:
        appSprintf(Text, TEXT("%s"), SysMsg(MSG_SystemError));
        break;
    case 21:
        appSprintf(Text, TEXT("%s"), SysMsg(MSG_AccessFailed));
        break;
    case 22:
        appSprintf(Text, TEXT("%s"), SysMsg(MSG_KoreaOnly));
        break;
    case 31:
        appSprintf(Text, TEXT("%s"), SysMsg(MSG_LoginReason31));
        break;
    case 32:
        appSprintf(Text, TEXT("%s"), SysMsg(MSG_LoginReason32));
        break;
    case 4:
    case 6:
    case 8:
    case 9:
    case 10:
    case 11:
    case 13:
    case 14:
    case 15:
        appSprintf(Text, TEXT("%s %s %s"), SysMsg(MSG_AccessFailed), SysMsg(MSG_AccessFailed + 1),
                   SysMsg(MSG_AccessFailed + 2));
        break;
    }
    ShowMessageWnd(Text);
    ShowAuthWnd(this, Reason != 31);
}
NW_HOOK_METHOD(0x10058410, NConsoleWnd, AuthLoginFail)

// 0x10058780 (vslot 33) - after the login: the announcements, if there are any, else straight to the
// server list; the value after them goes to the box. C2 has three kinds, in this order: the text at
// GL2GameData +0x5D7A0 in the login screen's box at +0xA4 (the header's names for the text and the
// box, "NoticeText" and "EulaWnd", may be swapped: the offsets are the stock's), a Chinese client's box
// at +0xA8, and the text at +0x5D788 in the announcement box (its bit 0: the warning box follows). The
// two first leave the login box in account mode (ResetAuthWnd) and hide it and the server list.
void NConsoleWnd::StartNotice(L2ParamStack& Params)
{
    if (GL2GameData.NoticeText.Len() > 0)
    {
        ResetAuthWnd(LoginWnd->AuthWnd);
        LoginWnd->AuthWnd->HideWindow();
        NLOGIN_SERVERLIST(LoginWnd)->HideWindow();
        LoginWnd->EulaWnd->ShowWindow();
        LoginWnd->EulaWnd->SetFocus();
        static_cast<NCEulaWnd*>(LoginWnd->EulaWnd)->ClearText();                      // ClearText of its text box
        static_cast<NCEulaWnd*>(LoginWnd->EulaWnd)->SetText(*GL2GameData.NoticeText); // SetText
        nw::Field<void*>(LoginWnd->EulaWnd, 0xA8) = Params.Top();
        SetState(L2CS_Login);
        return;
    }
    if (GLanguageType == 4)
    {
        ResetAuthWnd(LoginWnd->AuthWnd);
        LoginWnd->AuthWnd->HideWindow();
        NLOGIN_SERVERLIST(LoginWnd)->HideWindow();
        LoginWnd->AnnounceWnd->ShowWindow();
        LoginWnd->AnnounceWnd->SetFocus();
        nw::Field<void*>(LoginWnd->EulaWnd, 0xA8) = Params.Top();   // the +0xA4 box's, as the stock has it
        SetState(L2CS_Login);
        return;
    }
    if (GL2GameData.EulaText.Len() > 0)
    {
        LoginWnd->AuthWnd->HideWindow();
        NLOGIN_SERVERLIST(LoginWnd)->HideWindow();
        LoginWnd->NoticeWnd->ShowWindow();
        LoginWnd->NoticeWnd->SetFocus();
        LoginWnd->NoticeWnd->ClearText();
        LoginWnd->NoticeWnd->SetText(*GL2GameData.EulaText);
        LoginWnd->NoticeWnd->Type = (INT)Params.Top();
        SetState(L2CS_Login);
        return;
    }
    ResetAuthWnd(LoginWnd->AuthWnd);
    RequestServerList();
}
NW_HOOK_METHOD(0x10058780, NConsoleWnd, StartNotice)

// 0x10052740 - the server list follows (AddServerList), with the server last played on
void NConsoleWnd::ReceiveServerList(INT LastServerID)
{
    guard(NConsoleWnd::ReceiveServerList);
    MessageWnd->ClearText();
    OptionWnd->HideWindow();
    LoginWnd->ResetWindows();
    LoginWnd->AuthWnd->HideWindow();
    LoginWnd->NoticeWnd->HideWindow();
    LoginWnd->WarningWnd->HideWindow();
    NLOGIN_WND_A4(LoginWnd)->HideWindow();     // C2: two more windows of the login screen
    NLOGIN_WND_A8(LoginWnd)->HideWindow();
    NLOGIN_SERVERLIST(LoginWnd)->SetLastServerID(LastServerID);
    NLOGIN_SERVERLIST(LoginWnd)->ShowWindow();
    NLOGIN_SERVERLIST(LoginWnd)->SetFocus();
    unguard;
}
NW_HOOK_METHOD(0x10052740, NConsoleWnd, ReceiveServerList)

// 0x10052870
void NConsoleWnd::AddServerList(L2ParamStack& Params)
{
    guard(NConsoleWnd::AddServerList);
    NLOGIN_SERVERLIST(LoginWnd)->AddServer(Params);
    unguard;
}
NW_HOOK_METHOD(0x10052870, NConsoleWnd, AddServerList)

// 0x10052900
void NConsoleWnd::EndServerList()
{
    guard(NConsoleWnd::EndServerList);
    NLOGIN_SERVERLIST(LoginWnd)->OnServerListEnd();
    unguard;
}
NW_HOOK_METHOD(0x10052900, NConsoleWnd, EndServerList)

// 0x100574c0 (vslot 305) - the server list window's SetPing (the server list is the login screen's
// box at +0xAC)
void NConsoleWnd::SetServerPing(INT ServerID, DWORD Ping)
{
    if (NLOGIN_SERVERLIST(LoginWnd))
        NLOGIN_SERVERLIST(LoginWnd)->SetPing(ServerID, Ping);
}
NW_HOOK_METHOD(0x100574c0, NConsoleWnd, SetServerPing)

// 0x10058070 (vslot 271) - the account is blocked, with the server's text: the message is built from
// four lines of systemmsg and takes the text
void NConsoleWnd::BlockedAccountWithMsg(TCHAR* Msg)
{
    TCHAR Format[256];
    TCHAR Text[256];
    if (Msg && *Msg)
    {
        appSprintf(Format, TEXT("%s %s %s %s"), SysMsg(MSG_Withdrawn), SysMsg(MSG_Withdrawn + 1),
                   SysMsg(MSG_Withdrawn + 2), SysMsg(MSG_Withdrawn + 3));
        appSprintf(Text, Format, Msg);
        ShowMessageWnd(Text);
    }
    ShowAuthWnd(this);
}
NW_HOOK_METHOD(0x10058070, NConsoleWnd, BlockedAccountWithMsg)

// 0x100581f0 (vslot 272) - the account is blocked: the message of the lowest reason bit set (0..15)
void NConsoleWnd::BlockedAccount(INT Reason)
{
    for (INT Bit = 0; Bit < 16; Bit++)
    {
        if (Reason & (1 << Bit))
        {
            ShowMessageWnd(SysMsg(MSG_BlockReason + Bit));
            break;
        }
    }
    ShowAuthWnd(this);
}
NW_HOOK_METHOD(0x100581f0, NConsoleWnd, BlockedAccount)

// 0x10052f80 - the server's protocol version: 0 when it differs. With [URL] L2VersionCheck
// (1 when not set) the client logs out.
void NConsoleWnd::ReceiveVersionCheck(L2ParamStack& Params)
{
    BYTE bSame = (BYTE)(INT)Params.Top();
    UBOOL bVersionCheck = 1;
    if (!GConfig->GetBool(TEXT("URL"), TEXT("L2VersionCheck"), bVersionCheck, NULL))
        bVersionCheck = 1;
    if (bVersionCheck)
    {
        if (!bSame)
        {
            Engine->NetHandler->SendLogOutPacket();  // RequestLogout's
            ShowMessageWnd(SysMsg(MSG_ProtocolQuit));
        }
    }
    else if (!bSame)
        ShowMessageWnd(SysMsg(MSG_ProtocolContinue));
}
NW_HOOK_METHOD(0x10052f80, NConsoleWnd, ReceiveVersionCheck)

/*-----------------------------------------------------------------------------
    The character selection and creation, entering the world.
-----------------------------------------------------------------------------*/

// 0x10052a80 - the character list follows (AddCharacterInfo): the selection replaces the
// login screen (not while in the world)
void NConsoleWnd::ReceiveCharacterInfo()
{
    guard(NConsoleWnd::ReceiveCharacterInfo);
    if (State != L2CS_InGame)
    {
        LoginWnd->ResetWindows();
        LoginWnd->HideWindow();
        OptionWnd->HideWindow();
        PawnCreateWnd->HideWindow();
        LobbyWnd->ClearCharacters();
        LobbyWnd->ShowWindow();
        MoveCameraByState(L2CS_None, L2CS_Lobby);
        SetState(L2CS_Lobby);
    }
    unguard;
}
NW_HOOK_METHOD(0x10052a80, NConsoleWnd, ReceiveCharacterInfo)

// 0x10052b60
void NConsoleWnd::AddCharacterInfo(User Info)
{
    guard(NConsoleWnd::AddCharacterInfo);
    check(LobbyWnd);
    if (State != L2CS_InGame)
        LobbyWnd->AddCharacter(Info);
    unguard;
}
NW_HOOK_METHOD(0x10052b60, NConsoleWnd, AddCharacterInfo)

// 0x10058a30 (vslot 37) - into the world: the desktop replaces the loading screen, and the screen takes
// the game's resolution if it differs (the result is not read). C2: the four checks are the stock's own
// (names of its members, lines of Src\NConsoleWindow.cpp), and the game's size follows what the
// viewport got when SetRes succeeded
void NConsoleWnd::AcceptGameStart(INT Result)
{
    guard(NConsoleWnd::AcceptGameStart);
    if (!Desktop)
        appFailAssert("m_pConsole", "Src\\NConsoleWindow.cpp", 0x76A);
    if (!LoadingWnd)
        appFailAssert("m_pLoadingWnd", "Src\\NConsoleWindow.cpp", 0x76C);
    if (!Engine->Client->Viewports(0))
        appFailAssert("m_Engine->Client->Viewports(0)", "Src\\NConsoleWindow.cpp", 0x76D);
    if (!Engine->GRenDev)
        appFailAssert("m_Engine->GRenDev", "Src\\NConsoleWindow.cpp", 0x76E);
    LoadingWnd->HideWindow();
    Desktop->ShowWindow();
    UViewport* Viewport = Engine->Client->Viewports(0);
    INT ScreenX = GSEKScreenX;
    if (Viewport->SizeX != ScreenX)
    {
        if (Engine->GRenDev->SetRes(Viewport, ScreenX, GSEKScreenY, GColorBits / 8, GRefreshRate,
                                    Unknown3720, 1))   // +0x3594: fullscreen
        {
            GSEKScreenX = Engine->Client->Viewports(0)->SizeX;
            GSEKScreenY = nw::Field<INT>(Engine->Client->Viewports(0), 0x8C);   // UViewport::SizeY
        }
    }
    SetState(L2CS_InGame);
    unguard;
}
NW_HOOK_METHOD(0x10058a30, NConsoleWnd, AcceptGameStart)

// 0x10052ca0 - only the failures are answered
void NConsoleWnd::AcceptCharacterDelete(INT Result)
{
    guard(NConsoleWnd::AcceptCharacterDelete);
    switch (Result)
    {
    case 1:
        ShowMessageWnd(SysMsg(MSG_DeleteFailed));
        break;
    case 2:
        ShowMessageWnd(SysMsg(MSG_ClanLeaderDelete));
        break;
    case 3:
        ShowMessageWnd(SysMsg(MSG_ClanMemberDelete));
        break;
    }
    unguard;
}
NW_HOOK_METHOD(0x10052ca0, NConsoleWnd, AcceptCharacterDelete)

// 0x10052d60 - the creation may start: its screen replaces the selection
void NConsoleWnd::AcceptNewCharacter(INT Result)
{
    guard(NConsoleWnd::AcceptNewCharacter);
    if (Result)
    {
        LobbyWnd->HideWindow();
        PawnCreateWnd->Reset();
        PawnCreateWnd->ShowWindow();
        MoveCameraByState(L2CS_None, L2CS_Create);
        SetState(L2CS_Create);
    }
    else
    {
        SetState(L2CS_Lobby);
        ShowMessageWnd(SysMsg(MSG_CreateFailed));
    }
    unguard;
}
NW_HOOK_METHOD(0x10052d60, NConsoleWnd, AcceptNewCharacter)

// 0x10052e50 - only a failure is answered (the list follows a success)
void NConsoleWnd::AcceptCharacterCreate(INT Result)
{
    guard(NConsoleWnd::AcceptCharacterCreate);
    if (!Result)
        ShowMessageWnd(SysMsg(MSG_CreateFailed));
    unguard;
}
NW_HOOK_METHOD(0x10052e50, NConsoleWnd, AcceptCharacterCreate)

// 0x10052980 - the creation was refused: 1 too many characters (the choices start over), 2 the
// name is taken, 3 the name is not valid (the name is typed again)
void NConsoleWnd::CharacterCreateFail(INT Reason)
{
    switch (Reason)
    {
    case 0:
        ShowMessageWnd(SysMsg(MSG_CreateFailed));
        break;
    case 1:
        ShowMessageWnd(SysMsg(MSG_TooManyCharacters));
        PawnCreateWnd->SetupWnd->Reset();
        break;
    case 2:
        ShowMessageWnd(SysMsg(MSG_NameExists));
        PawnCreateWnd->SetupWnd->NameEdit->ClearText();
        PawnCreateWnd->SetupWnd->NameEdit->SetFocus();
        break;
    case 3:
        ShowMessageWnd(SysMsg(MSG_NameInvalid));
        PawnCreateWnd->SetupWnd->NameEdit->ClearText();
        PawnCreateWnd->SetupWnd->NameEdit->SetFocus();
        break;
    }
    SetState(L2CS_Create);
}
NW_HOOK_METHOD(0x10052980, NConsoleWnd, CharacterCreateFail)

// 0x100532b0 - the template of a class for the creation
void NConsoleWnd::AddRecommandedData(RecommandedDataStr Data)
{
    guard(NConsoleWnd::AddRecommandedData);
    PawnCreateWnd->SetRecommendedData(Data);
    unguard;
}
NW_HOOK_METHOD(0x100532b0, NConsoleWnd, AddRecommandedData)

// 0x10053340
void NConsoleWnd::EndRecommandedData()
{
    guard(NConsoleWnd::EndRecommandedData);
    PawnCreateWnd->EndRecommendedData();
    unguard;
}
NW_HOOK_METHOD(0x10053340, NConsoleWnd, EndRecommandedData)

/*-----------------------------------------------------------------------------
    The restart, the revival, the logout, a closed server.
-----------------------------------------------------------------------------*/

// 0x10053010 - the restart was accepted (first): the game's windows give way to the loading
// screen
void NConsoleWnd::RestartResetWindows()
{
    GetDesktop()->Reset();
    Desktop->HideWindow();
    OptionWnd->HideWindow();
    PetitionWnd->HideWindow();
    LoadingWnd->ShowWindow();
    RestartMenuWnd->HideWindow();
}
NW_HOOK_METHOD(0x10053010, NConsoleWnd, RestartResetWindows)

// 0x10053070 - (then) back to the character selection: the engine travels to the lobby map
// ("Lobby?L2Restart", from the [DefaultPlayer] URL) and the selection is shown again
void NConsoleWnd::RestartToLobby()
{
    FString Error;
    FURL DefaultURL(NULL);
    DefaultURL.LoadURLConfig(TEXT("DefaultPlayer"), TEXT("User"));
    FURL URL(&DefaultURL, TEXT("Lobby"), TRAVEL_Partial);
    URL.AddOption(TEXT("L2Restart"));
    if (!static_cast<UGameEngine*>(Engine)->Browse(URL, NULL, Error))
        GError->Logf(LocalizeError(TEXT("FailedBrowse"), TEXT("Engine")), *Error);
    SpawnDefaultCharacter();
    LoadingWnd->HideWindow();
    LobbyWnd->ClearCharacters();
    LobbyWnd->ShowWindow();
    SetState(L2CS_Lobby);
    MoveCameraByState(L2CS_None, L2CS_Lobby);
}
NW_HOOK_METHOD(0x10053070, NConsoleWnd, RestartToLobby)

// 0x10063d40 - someone offers to revive the dead player (the offer's two values go back with
// the answer); only in the world
void NConsoleWnd::ReviveRequest(INT Param1, INT Param2)
{
    if (State != L2CS_InGame)
        return;
    RestartMenuWnd->HideWindow();
    NDialogInfo Info;
    Info.ID = DIALOG_REVIVE;
    Info.bMainBox = 1;
    Info.bInput = 0;
    Info.Owner = NULL;
    Info.SetParams(Param1, Param2);
    Info.SetSystemMessage(MSG_AskRevive);
    ShowDialog(&Info);
}
NW_HOOK_METHOD(0x10063d40, NConsoleWnd, ReviveRequest)

// 0x10063bf0 - the server let the player go: the window positions are saved and the game
// ends
void NConsoleWnd::AcceptLogOut()
{
    SaveWindowPositions();
    appRequestExit(0);
}
NW_HOOK_METHOD(0x10063bf0, NConsoleWnd, AcceptLogOut)

#if NW_FEATURES
// phase 6: ReturnToLogin (included here, below the check()s: their __LINE__ stays the faithful
// build's)
#include "features/ReturnToLogin.h"
#endif

// the stock body is AcceptLogOut's (folded)
void NConsoleWnd::QuitGame()
{
#if NW_FEATURES
    // phase 6: ReturnToLogin (NWindow_C1 src/inject/ReturnToLogin.cpp): OK on the
    // "disconnected" box (the only caller) goes back to the login screen from the next Tick
    // instead of ending the game
    if (nwf::ReturnToLogin_QuitGame())
        return;
#endif
    SaveWindowPositions();
    appRequestExit(0);
}

// 0x10063c00 (vslot 42) - the server closed the connection: in the world a dialog box whose OK ends the
// game, before it the login stages' own box. C2 sets the same system message for the reasons 0, 1 and 2
// (C1 marked reason 2 in the text)
void NConsoleWnd::ServerClose(INT Reason)
{
    if (State == L2CS_InGame)
    {
        NDialogInfo Info;
        Info.ID = DIALOG_DISCONNECTED;
        Info.bMainBox = 1;
        Info.bInput = 0;
        Info.Owner = NULL;
        if (Reason == 0 || Reason == 1 || Reason == 2)
            Info.SetSystemMessage(MSG_DisconnectedRetry);
        ShowDialog(&Info);
    }
    else
    {
        NDialogInfo Info;
        Info.ID = DIALOG_DISCONNECTED;
        Info.bInput = 0;
        Info.Owner = NULL;
        if (Reason == 0 || Reason == 1 || Reason == 2)
            Info.SetSystemMessage(MSG_DisconnectedRetry);
        DialogBox->SetInfo(&Info);
        DialogBox->ShowWindow();
        DialogBox->SetFocus();
    }
}
NW_HOOK_METHOD(0x10063c00, NConsoleWnd, ServerClose)

// 0x10052ef0 - the console's own: the connection is gone (on the message line)
void NConsoleWnd::ReceiveServerClose()
{
    guard(NConsoleWnd::ReceiveServerClose);
    ShowMessageWnd(SysMsg(MSG_Disconnected));
    unguard;
}
NW_HOOK_METHOD(0x10052ef0, NConsoleWnd, ReceiveServerClose)
