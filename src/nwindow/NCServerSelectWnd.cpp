/*=============================================================================
    NCServerSelectWnd.cpp - the server list of the login screen and its list control.
    Stock: 0x100a1e60 .. 0x100a24c7, 0x100a2500 .. 0x100a39cf (OnDestroy is a body the
    linker folded with other classes).
=============================================================================*/
#include "nwindow/NCServerSelectWnd.h"
#include "nwindow/NCLoginWnd.h"
#include "nwindow/NCAuthWnd.h"
#include "nwindow/NCButton.h"
#include "nwindow/NCEditBox.h"
#include "nwindow/NCFrameCtrl.h"
#include "nwindow/NCGDevice.h"
#include "nwindow/NConsoleWnd.h"
#include "nwindow/NCrt.h"

#include <wchar.h>

// NWM_COMMAND ids, timer id (names ours)
enum
{
    NID_SERVER_OK = 0xF007,
    NID_SERVER_FRAME = 0xF008,
    NID_SERVER_CANCEL = 0xF017,
    NID_SERVER_LIST = 0xF018,
    TIMER_SERVER_RETRY = 0xF019,    // 3 s: the next OK may log in again
    NID_SERVER_QUALITY = 0xF020,
    NID_SERVERLIST_SORT = 0xF021,   // no control of the list sends it
};

// interface sounds (FL2NetNotify::OnInterfacePlaySound)
enum { SOUND_Click = 0, SOUND_ClickDisabled = 2 };

// ping of a server that did not answer, or was not asked
enum { PING_None = 9999 };

// NConsoleWnd::SetLoadingTexture's screens, from the server's age rating (names ours)
enum
{
    LOADING_AllAges = 10001,
    LOADING_Over15 = 10002,
    LOADING_Over18 = 10003,
};

// names of NCsoft's servers 0..14, shown instead of sysstring 506 + id when the login server
// is theirs
#if NW_HOOK_MODE
#define GServerNames (reinterpret_cast<const TCHAR* const*>(::nw::Addr(0x10186b00)))
#else
extern const TCHAR* const GServerNames[];
#endif

// The cells of a row (C2: five columns; C1 had four, with the ping in 2 and the type in 3): the number, the name,
// the load (the PvP text in Text2, the server flags in Param1, users in Cells[1]), the ping (the age limit in
// Text2) and the icon column
enum
{
    CELL_Number = 0,
    CELL_Name = 1,
    CELL_Load = 2,
    CELL_Ping = 3,
    CELL_Icon = 4,
};

// the server flags (the login server's last field)
enum { SERVERFLAG_Sandglass = 2, SERVERFLAG_Special = 8 };

// the tables of 0x1C-byte records at +0x48314 (server names, by server index) and +0x498F4 (server type names),
// an FString at +4 in each (FL2GameData in include/ue2 does not place them)
static FORCEINLINE const TCHAR* GameDataString(INT Offset)
{
    return **(FString*)((BYTE*)&GL2GameData + Offset);
}

// the console's login window and loading screen
static NCLoginWnd* ConsoleLoginWnd() { return GConsole->LoginWnd; }
static INT& ConsoleLoadingScreen() { return GConsole->LoadingScreenID; }

/*-----------------------------------------------------------------------------
    NCServerSelectListCtrl.
-----------------------------------------------------------------------------*/

// 0x100c97f0
NCServerSelectListCtrl::NCServerSelectListCtrl()
{
}
NW_HOOK_CTOR(0x100c97f0, NCServerSelectListCtrl)

// 0x100c9890
NCServerSelectListCtrl::NCServerSelectListCtrl(INT InMaxRows, INT InVisibleRows, INT InMaxColumns,
                                               INT Unused)
    : NCListCtrl(InMaxRows, InVisibleRows, InMaxColumns, Unused)
{
}
NW_HOOK_CTOR_ARGS(0x100c9890, NCServerSelectListCtrl, (INT A, INT B, INT C, INT D), (A, B, C, D))

// 0x100c9840
NW_HOOK_DTOR(0x100c9840, NCServerSelectListCtrl)

// 0x100c8080 (vslot 25)
INT NCServerSelectListCtrl::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (NCListCtrl::WindowProc(Msg, wParam, lParam))
        return 1;
    if (Msg == NWM_COMMAND && wParam == NID_SERVERLIST_SORT)
        return SortByColumn((INT)lParam) != 0;
    return 0;
}
NW_HOOK_METHOD(0x100c8080, NCServerSelectListCtrl, WindowProc)

// 0x100c9550 (vslot 73) - NCListCtrl::SortByColumn's code (C2: five-column rows), then the row flagged "special"
// (bit 8 of the load cell's Param1) goes last, and the window's remembered row follows its server too
INT NCServerSelectListCtrl::SortByColumn(INT Column)
{
    NListRow* SelRow = GetSelectedRow();
    NCServerSelectWnd* Owner = (NCServerSelectWnd*)Entry->Parent->Wnd;
    NListRow* LastRow = NULL;
    if (Owner && Owner->LastServerRow >= 0 && Owner->LastServerRow < Rows.Num())
        LastRow = Rows(Owner->LastServerRow);
    for (INT i = 0; i < Rows.Num(); i++)
    {
        for (INT j = 0; j < Rows.Num() - i && j < Rows.Num() - 1; j++)
        {
            const TCHAR* A = Rows(j)->Cells[Column].Text;
            if (!*A)
                continue;
            const TCHAR* B = Rows(j + 1)->Cells[Column].Text;
            if (!*B)
                continue;
            NCHeaderCtrl* Header = Columns(Column);
            UBOOL bSwap;
            if (Header->bNumeric)
            {
                if (Header->bAscending)
                    bSwap = NWtol(A) > NWtol(B);
                else
                    bSwap = NWtol(A) < NWtol(B);
            }
            else
            {
                if (Header->bAscending)
                    bSwap = _wcsicmp(A, B) > 0;
                else
                    bSwap = _wcsicmp(A, B) < 0;
            }
            if (bSwap)
            {
                NListRow* Temp = Rows(j);
                Rows(j) = Rows(j + 1);
                Rows(j + 1) = Temp;
            }
        }
    }
    INT NumRows = Rows.Num();
    for (INT i = 0; i < NumRows; i++)
    {
        if (Rows(i)->Cells[CELL_Load].Param1 & SERVERFLAG_Special)
        {
            NListRow* Temp = Rows(i);
            Rows(i) = Rows(NumRows - 1);
            Rows(Rows.Num() - 1) = Temp;
            break;
        }
    }
    Columns(Column)->bAscending = !Columns(Column)->bAscending;
    if (SelRow)
    {
        for (Selected = 0; Selected < Rows.Num(); Selected++)
        {
            if (Rows(Selected) == SelRow)
                break;
        }
    }
    if (Owner && LastRow)
    {
        for (Owner->LastServerRow = 0; Owner->LastServerRow < Rows.Num(); Owner->LastServerRow++)
        {
            if (Rows(Owner->LastServerRow) == LastRow)
                break;
        }
    }
    return 1;
}
NW_HOOK_METHOD(0x100c9550, NCServerSelectListCtrl, SortByColumn)

NW_CHECK_VTABLE(NCServerSelectListCtrl, 0x10124e60, 74)

/*-----------------------------------------------------------------------------
    NCServerSelectWnd.
-----------------------------------------------------------------------------*/

// 0x100c86d0
NCServerSelectWnd::NCServerSelectWnd()
{
    BackTex = NULL;
    LastServerID = 0;
    bRequested = 0;
    PendingPings = 0;
    LastServerRow = -1;
}
NW_HOOK_CTOR(0x100c86d0, NCServerSelectWnd)

// 0x100c8740
NW_HOOK_DTOR(0x100c8740, NCServerSelectWnd)

// 0x100c80e0 (vslot 25) - C2 no longer refuses the messages of a disabled window here
INT NCServerSelectWnd::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (NCWnd::WindowProc(Msg, wParam, lParam))
        return 1;
    if (Msg == NWM_COMMAND)
    {
        if (wParam == NID_SERVER_OK)
            return OnOK() != 0;
        if (wParam == NID_SERVER_CANCEL)
            return OnCancel() != 0;
        if (wParam == NID_SERVER_QUALITY)
            return SortByQuality() != 0;
        if (wParam == NID_SERVER_LIST)
            return OnListNotify((INT)lParam) != 0;
    }
    return 0;
}
NW_HOOK_METHOD(0x100c80e0, NCServerSelectWnd, WindowProc)

// 0x100c83f0 (vslot 35)
INT NCServerSelectWnd::OnTimer(INT TimerID)
{
    if (TimerID == TIMER_SERVER_RETRY)
        bRequested = 0;
    return 1;
}
NW_HOOK_METHOD(0x100c83f0, NCServerSelectWnd, OnTimer)

// 0x100c98c0 (vslot 36) - the title bar gets its id but FrameID is left alone. C2: five columns (a number column of
// 30 pixels that language 2 gives to the name instead, name, status, ping, type) and 35 visible rows
INT NCServerSelectWnd::OnCreate()
{
    NCWnd::OnCreate();
    BackTex = FindTexture(TEXT("L2UI.lobbywnd.serverback"));
    TimerResult = SetTimer(TIMER_SERVER_RETRY, 3000, NULL);

    FrameCtrl = new NCFrameCtrl(1);
    FrameCtrl->ID = NID_SERVER_FRAME;
    FrameCtrl->SetWindowRect(0, 0, 300, 21);
    CreateChild(FrameCtrl, NWS_VISIBLE);

    ServerList = new NCServerSelectListCtrl(100, 35, 5, 12);
    ServerList->ID = NID_SERVER_LIST;
    ServerList->SetWindowRect(15, 26, 281, 472);
    CreateChild(ServerList, NWS_VISIBLE);
    ServerList->SetTextOffset(0, 3);

    NListColumnInfo NumberColumn;
    NumberColumn.Width = (GLanguageType == 2) ? 0 : 30;
    NumberColumn.Height = 15;
    if (GLanguageType != 2)
        NumberColumn.SetTitle(*GL2GameData.SysString[416]);
    NumberColumn.SetNormalTex(TEXT("L2UI.control.listButton"));
    NumberColumn.SetPushedTex(TEXT("L2UI.control.listButton_click"));
    ServerList->AddColumn(0, &NumberColumn);

    NListColumnInfo NameColumn;
    NameColumn.Width = (GLanguageType != 2) ? 108 : 138;
    NameColumn.Height = 15;
    NameColumn.SetTitle(*GL2GameData.SysString[451]);          // "Name"
    NameColumn.SetNormalTex(TEXT("L2UI.control.listButton"));
    NameColumn.SetPushedTex(TEXT("L2UI.control.listButton_click"));
    ServerList->AddColumn(1, &NameColumn);

    NListColumnInfo StatusColumn;
    StatusColumn.Width = 50;
    StatusColumn.Height = 15;
    StatusColumn.SetTitle(*GL2GameData.SysString[452]);        // "Status"
    StatusColumn.SetNormalTex(TEXT("L2UI.control.listButton"));
    StatusColumn.SetPushedTex(TEXT("L2UI.control.listButton_click"));
    ServerList->AddColumn(2, &StatusColumn);

    NListColumnInfo PingColumn;
    PingColumn.Width = 50;
    PingColumn.Height = 15;
    PingColumn.bNumeric = 1;
    PingColumn.SetTitle(*GL2GameData.SysString[601]);          // "Ping"
    PingColumn.SetNormalTex(TEXT("L2UI.control.listButton"));
    PingColumn.SetPushedTex(TEXT("L2UI.control.listButton_click"));
    ServerList->AddColumn(3, &PingColumn);

    NListColumnInfo TypeColumn;
    TypeColumn.Width = 30;
    TypeColumn.Height = 15;
    TypeColumn.SetTitle(*GL2GameData.SysString[602]);          // "Type"
    TypeColumn.SetNormalTex(TEXT("L2UI.control.listButton"));
    TypeColumn.SetPushedTex(TEXT("L2UI.control.listButton_click"));
    ServerList->AddColumn(4, &TypeColumn);

    OKButton = new NCButton(TEXT("L2UI.Control.DefaultButton"),
                            TEXT("L2UI.Control.DefaultButton_click"), 0);
    OKButton->ID = NID_SERVER_OK;
    OKButton->SetWindowRect(117, 501, 67, 19);
    CreateChild(OKButton, NWS_VISIBLE);
    OKButton->SetTextID(140);                                  // "OK"

    CancelButton = new NCButton(TEXT("L2UI.Control.DefaultButton"),
                                TEXT("L2UI.Control.DefaultButton_click"), 0);
    CancelButton->ID = NID_SERVER_CANCEL;
    CancelButton->SetWindowRect(211, 501, 67, 19);
    CreateChild(CancelButton, NWS_VISIBLE);
    CancelButton->SetTextID(141);                              // "Cancel"

    QualityButton = new NCButton(TEXT("L2UI.Control.DefaultButton"),
                                 TEXT("L2UI.Control.DefaultButton_click"), 0);
    QualityButton->ID = NID_SERVER_QUALITY;
    QualityButton->SetWindowRect(25, 501, 67, 19);
    CreateChild(QualityButton, NWS_VISIBLE);
    QualityButton->SetTextID(603);                             // "Quality"

    bPingCacheOld = IsPingCacheOld();
    bPingSaved = 0;
    UnknownC4 = 0;
    return 1;
}
NW_HOOK_METHOD(0x100c98c0, NCServerSelectWnd, OnCreate)

// 0x100c8790 (vslot 37) - C2: a body of its own (guarded)
INT NCServerSelectWnd::OnDestroy()
{
    guard(NCServerSelectWnd::OnDestroy);
    if (ServerList)
    {
        RemoveChildWindow(ServerList->Entry);
        delete ServerList;
        ServerList = NULL;
    }
    if (OKButton)
    {
        RemoveChildWindow(OKButton->Entry);
        delete OKButton;
        OKButton = NULL;
    }
    if (CancelButton)
    {
        RemoveChildWindow(CancelButton->Entry);
        delete CancelButton;
        CancelButton = NULL;
    }
    if (QualityButton)
    {
        RemoveChildWindow(QualityButton->Entry);
        delete QualityButton;
        QualityButton = NULL;
    }
    NCWnd::OnDestroy();
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x100c8790, NCServerSelectWnd, OnDestroy)

// 0x100c88c0 (vslot 55) - under the 21-pixel title bar; the clip goes on before BeginPaint
INT NCServerSelectWnd::OnPaint(NCGDevice* Device)
{
    guard(NCServerSelectWnd::OnPaint);
    Device->SetClip(ScreenX, ScreenY, Width, Height);
    Device->BeginPaint(ScreenX, ScreenY);
    if (BackTex)
        Device->DrawTexture(0, 21, 300, 505, 0, 0, 300, 505, BackTex);
    Device->EndPaint();
    Device->ReleaseClip();
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x100c88c0, NCServerSelectWnd, OnPaint)

// 0x100c83c0 (vslot 58) - Enter logs in
INT NCServerSelectWnd::OnKeyDown(WPARAM wParam, LPARAM lParam)
{
    if (wParam == VK_RETURN)
        OnOK();
    NCWnd::OnKeyDown(wParam, lParam);
    return 1;
}
NW_HOOK_METHOD(0x100c83c0, NCServerSelectWnd, OnKeyDown)

// 0x100c8f70 (vslot 67) - log in to the selected server (address, port and id from its row); its age rating (the
// ping cell's Text2 in C2) picks the Korean loading screen
INT NCServerSelectWnd::OnOK()
{
    guard(NCServerSelectWnd::OnOK);
    NListRow* Row = ServerList->GetSelectedRow();
    if (Row)
    {
        const TCHAR* Age = Row->Cells[CELL_Ping].Text2;
        INT Screen = LOADING_AllAges;
        if (Age)
        {
            if (!appStrcmp(Age, *GL2GameData.SysString[462]))       // "Over 15 years"
                Screen = LOADING_Over15;
            else if (!appStrcmp(Age, *GL2GameData.SysString[461]))  // "Over 18 years"
                Screen = LOADING_Over18;
        }
        ConsoleLoadingScreen() = Screen;
        L2ParamStack Params(10);
        INT ServerID = Row->Param1;
        INT IP = Row->Param2;
        INT Port = appAtoi(Row->Text);
        Params.PushBack((void*)ServerID);
        Params.PushBack((void*)IP);
        Params.PushBack((void*)Port);
        if (!bRequested)
        {
            GConsole->RequestServerLogin(Params);
            SavePingTime();
            bRequested = 1;
        }
        PlayInterfaceSound(SOUND_Click);
    }
    else
        PlayInterfaceSound(SOUND_ClickDisabled);
    PendingPings = 0;
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x100c8f70, NCServerSelectWnd, OnOK)

// 0x100c90e0 (vslot 68) - back to the login box (C2: the account line is no longer focused here)
INT NCServerSelectWnd::OnCancel()
{
    GConsole->Engine->NetHandler->AuthReconnect();
    ConsoleLoginWnd()->ResetWindows();
    ConsoleLoginWnd()->ShowWindow();
    GConsole->SetState(L2CS_Login);
    PendingPings = 0;
    SavePingTime();
    return 1;
}
NW_HOOK_METHOD(0x100c90e0, NCServerSelectWnd, OnCancel)

// Bubble sort of rows First..Last by users (Cells[1].Param1, compared unsigned); inlined three
// times in SortByQuality in the stock code (name ours).
static void SortRowsByUsers(TArray<NListRow*>& Rows, INT First, INT Last)
{
    for (INT i = First; i < Last + 1; i++)
    {
        for (INT j = First; j < Last; j++)
        {
            if ((DWORD)Rows(j)->Cells[1].Param1 > (DWORD)Rows(j + 1)->Cells[1].Param1)
            {
                NListRow* Temp = Rows(j);
                Rows(j) = Rows(j + 1);
                Rows(j + 1) = Temp;
            }
        }
    }
}

// 0x100c9140 (vslot 69) - "Quality": the server played last goes to the top (swapped with the first row), the
// others are sorted by ping (C2: the ping is in the fourth cell), then each ping class (under 10 ms, 10 to 50, over
// 50) by users; unanswered servers (9999 ms) stay last, unsorted. C2 ends by moving the first row flagged "special"
// (bit 8 of its load cell's Param1) to the end
INT NCServerSelectWnd::SortByQuality()
{
    INT NumFast = 0;
    INT NumMid = 0;
    INT NumSlow = 0;
    TArray<NListRow*>& Rows = ServerList->Rows;
    if (LastServerRow >= 0 && LastServerRow < ServerList->GetRowCount())
    {
        NListRow* Temp = Rows(0);
        Rows(0) = Rows(LastServerRow);
        Rows(LastServerRow) = Temp;
        ServerList->SetSelected(0);
        LastServerRow = 0;
    }
    for (INT i = 1; i < Rows.Num(); i++)
    {
        for (INT j = 1; j < Rows.Num() - i && j < Rows.Num() - 1; j++)
        {
            if (appAtoi(Rows(j)->Cells[CELL_Ping].Text) > appAtoi(Rows(j + 1)->Cells[CELL_Ping].Text))
            {
                NListRow* Temp = Rows(j);
                Rows(j) = Rows(j + 1);
                Rows(j + 1) = Temp;
            }
        }
    }
    for (INT i = 1; i < Rows.Num(); i++)
    {
        if (appAtoi(Rows(i)->Cells[CELL_Ping].Text) < 10)
            NumFast++;
        else if (appAtoi(Rows(i)->Cells[CELL_Ping].Text) > 50 && appAtoi(Rows(i)->Cells[CELL_Ping].Text) < PING_None)
            NumSlow++;
        else if (appAtoi(Rows(i)->Cells[CELL_Ping].Text) < PING_None)
            NumMid++;
    }
    SortRowsByUsers(Rows, 1, NumFast);
    SortRowsByUsers(Rows, NumFast + 1, NumFast + NumMid);
    SortRowsByUsers(Rows, NumFast + NumMid + 1, NumFast + NumMid + NumSlow);
    INT NumRows = Rows.Num();
    if (NumRows > 1)
    {
        for (INT i = 1; i < Rows.Num(); i++)
        {
            if (Rows(i)->Cells[CELL_Load].Param1 & SERVERFLAG_Special)
            {
                NListRow* Temp = Rows(i);
                Rows(i) = Rows(NumRows - 1);
                Rows(Rows.Num() - 1) = Temp;
                break;
            }
        }
    }
    return 1;
}
NW_HOOK_METHOD(0x100c9140, NCServerSelectWnd, SortByQuality)

// 0x100c83b0 (vslot 70) - double click on a row
INT NCServerSelectWnd::OnListNotify(INT Row)
{
    OnOK();
    return 1;
}
NW_HOOK_METHOD(0x100c83b0, NCServerSelectWnd, OnListNotify)

// 0x100c81a0 - from the login server's list (NConsoleWnd::ReceiveServerList)
void NCServerSelectWnd::SetLastServerID(INT ServerID)
{
    LastServerID = ServerID;
}
NW_HOOK_METHOD(0x100c81a0, NCServerSelectWnd, SetLastServerID)

// C2 CANDIDATE: the C2 function is not the same code as the C1 one; kept only while it matches the stock one (verify_data)
// 0x100c8310 - NConsoleWnd::EndServerList: the last server gets selected, then the list is
// ordered
void NCServerSelectWnd::OnServerListEnd()
{
    if (LastServerRow >= 0 && LastServerRow < ServerList->GetRowCount())
        ServerList->SetSelected(LastServerRow);
    GConsole->Engine->NetHandler->EndServerPingList();
    if (bPingSaved)
        UnknownC4 = 1;
    SortByQuality();
}
NW_HOOK_METHOD(0x100c8310, NCServerSelectWnd, OnServerListEnd)

// 0x100c8380 - NCLoginWnd::ResetWindows
void NCServerSelectWnd::ClearServers()
{
    ServerList->ClearRows();
    LastServerID = 0;
    LastServerRow = -1;
    PendingPings = 0;
}
NW_HOOK_METHOD(0x100c8380, NCServerSelectWnd, ClearServers)

// 0x100c81b0 - the number text ("0N" under nine rows, else "N"; a "special" server shows 127) and the name of the
// row about to be added: NCsoft's own servers' names (by position) when the login server is theirs, else the
// game data's by position (the special one has a name of its own); a server type puts "[type]" in front of it
__declspec(noinline) void NCServerSelectWnd::FormatServerLabel(INT Index, INT Flags, INT TypeIndex, TCHAR* Name,
                                                               TCHAR* NameText)
{
    TCHAR Tmp[120];
    if (ServerList->GetRowCount() < 9)
        appSprintf(NameText, TEXT("0%d"), Index + 1);
    else if (Flags & SERVERFLAG_Special)
        appSprintf(NameText, TEXT("%03d"), 0x7F);
    else
        appSprintf(NameText, TEXT("%d"), Index + 1);
    const TCHAR* ServerName;
    if (Index <= 15 &&
        (!appStricmp(GConfig->GetStr(TEXT("URL"), TEXT("ServerAddr"), NULL), TEXT("L2Server")) ||
         !appStricmp(GConfig->GetStr(TEXT("URL"), TEXT("ServerAddr"), NULL), TEXT("l2server.ncsoft.co.kr"))))
        ServerName = GServerNames[Index];
    else if (Flags & SERVERFLAG_Special)
        ServerName = GameDataString(0x490E0);
    else
        ServerName = GameDataString(0x48318 + Index * 0x1C);
    appSprintf(Name, TEXT("%s"), ServerName);
    if (TypeIndex > 0)
    {
        appStrncpy(Tmp, Name, appStrlen(Name) + 1);
        appSprintf(Name, TEXT("[%s]%s"), GameDataString(0x498F8 + TypeIndex * 0x1C), Tmp);
    }
}
NW_HOOK_METHOD(0x100c81b0, NCServerSelectWnd, FormatServerLabel)

// 0x100c89a0 - one server of the login server's list (NConsoleWnd::AddServerList): id, address, port, age limit,
// PvP, users, max users, state (0 down, 1 up), flags and (C2) the server type, in that order. Cells: the number (not
// for the special server, nor in language 2), the name, the load (PvP in Text2, the flags in Param1; users and max in
// Cells[1]'s params), the ping (age limit in Text2) and a sandglass in the icon column for flag 2. The saved ping is
// shown when there is a fresh one, else the server is asked (one more answer awaited). For other states the load
// text is left as it was on the stack, as in the stock code.
void NCServerSelectWnd::AddServer(L2ParamStack& Params)
{
    guard(NCServerSelectWnd::AddServer);
    INT ServerID = (INT)Params.Top();
    INT IP = (INT)Params.Top();
    INT Port = (INT)Params.Top();
    INT AgeLimit = (INT)Params.Top();
    INT bPvP = (INT)Params.Top();
    INT Users = (INT)Params.Top();
    INT MaxUsers = (INT)Params.Top();
    INT State = (INT)Params.Top();
    INT Flags = (INT)Params.Top();
    INT TypeIndex = (INT)Params.Top();
    TCHAR LoadText[32];
    TCHAR NameText[32];
    TCHAR AgeText[32];
    TCHAR Name[96];
    TCHAR PingText[32];
    TCHAR PvPText[32];

    NListRow* Row = new NListRow(5);
    if (ServerID == LastServerID)
        LastServerRow = ServerList->GetRowCount();
    FormatServerLabel(ServerList->GetRowCount(), Flags, TypeIndex, Name, NameText);
    if (!(Flags & SERVERFLAG_Special) && GLanguageType != 2)
        Row->Cells[CELL_Number].SetText(NameText);
    Row->Cells[CELL_Name].SetText(Name);
    Row->Cells[CELL_Ping].SetText(TEXT("9999ms"));

    if (State == 0)
        appSprintf(LoadText, TEXT("%s"), *GL2GameData.SysString[456]);
    else if (State == 1)
    {
        if ((SWORD)MaxUsers > 0)
        {
            FLOAT Load = (FLOAT)(SWORD)Users / (SWORD)MaxUsers * 100.f;
            if (Load < 95.f && (SWORD)Users < 4000)
            {
                if (!(Load < 90.f))
                    appSprintf(LoadText, TEXT("%s"), *GL2GameData.SysString[458]);
                else if (Load > 60.f)
                    appSprintf(LoadText, TEXT("%s"), *GL2GameData.SysString[457]);
                else
                    appSprintf(LoadText, TEXT("%s"), *GL2GameData.SysString[466]);
            }
            else
                appSprintf(LoadText, TEXT("%s"), *GL2GameData.SysString[467]);
        }
        else
            appSprintf(LoadText, TEXT("%s"), *GL2GameData.SysString[457]);

        if (!bPingCacheOld && !HasNoPing(ServerList->GetRowCount()))
        {
            INT Ping = PING_None;
            GConfig->GetInt(appItoa(ServerList->GetRowCount()), TEXT("Time"), Ping, TEXT("s_info.ini"));
            if (Ping == PING_None)
                appSprintf(PingText, TEXT("9999ms"));
            else
                appSprintf(PingText, TEXT("%dms"), Ping);
            Row->Cells[CELL_Ping].SetText(PingText);
        }
        else
        {
            Row->Cells[CELL_Ping].SetText(TEXT("9999ms"));
            GConsole->Engine->NetHandler->AddServerPingList(ServerList->GetRowCount(), IP, Port);
            PendingPings++;
        }
    }

    Row->Cells[CELL_Load].SetText(LoadText);
    if (bPvP == 1)
        appSprintf(PvPText, TEXT("%s"), *GL2GameData.SysString[459]);      // "On"
    else
        appSprintf(PvPText, TEXT("%s"), *GL2GameData.SysString[460]);      // "Off"
    if (AgeLimit == 18)
        appSprintf(AgeText, TEXT("%s"), *GL2GameData.SysString[461]);      // "Over 18 years"
    else if (AgeLimit == 15)
        appSprintf(AgeText, TEXT("%s"), *GL2GameData.SysString[462]);      // "Over 15 years"
    else
        appSprintf(AgeText, TEXT("%s"), *GL2GameData.SysString[539]);      // "Teen"
    if (Flags & SERVERFLAG_Sandglass)
        Row->Cells[CELL_Icon].SetIcon(FindTexture(TEXT("symbol.sandglass")), 10, 12);
    Row->SetParams(ServerID, IP, -1);
    Row->SetText(appItoa(Port));
    Row->Cells[CELL_Name].SetParams((SWORD)Users, (SWORD)MaxUsers, -1);
    Row->Cells[CELL_Load].SetText2(PvPText);
    Row->Cells[CELL_Load].SetParams(Flags, -1, -1);
    Row->Cells[CELL_Ping].SetText2(AgeText);
    ServerList->AddRow(Row);
    unguard;
}
NW_HOOK_METHOD(0x100c89a0, NCServerSelectWnd, AddServer)

// 0x100c8e90 - a ping answer (NConsoleWnd vslot 260): the rows of that address show it and save it; the list is
// ordered again when the last awaited answer arrives
void NCServerSelectWnd::SetPing(INT IP, INT Ping)
{
    TCHAR Text[32];
    if (!ServerList)
        return;
    for (INT i = 0; i < ServerList->Rows.Num(); i++)
    {
        NListRow* Row = ServerList->Rows(i);
        if (Row->Param2 == IP && Row->Cells)
        {
            if (Ping == PING_None)
                appSprintf(Text, TEXT("9999ms"));
            else
                appSprintf(Text, TEXT("%dms"), Ping);
            Row->Cells[CELL_Ping].SetText(Text);
            SavePing(Row->Param1 - 1, Ping);
            if (--PendingPings == 0)
            {
                SortByQuality();
                PendingPings = 0;
            }
        }
    }
}
NW_HOOK_METHOD(0x100c8e90, NCServerSelectWnd, SetPing)

// 0x100c8590 - no saved time, or more than 12 units of GetTimeStamp old. Not inlined: the stock OnCreate calls it
__declspec(noinline) UBOOL NCServerSelectWnd::IsPingCacheOld()
{
    INT Stamp;
    if (GConfig->GetInt(TEXT("Date"), TEXT("Time"), Stamp, TEXT("s_info.ini")))
        return Stamp + 12 < GetTimeStamp();
    return 1;
}
NW_HOOK_METHOD(0x100c8590, NCServerSelectWnd, IsPingCacheOld)

// 0x100c85e0 - no saved ping for the server, or 9999
UBOOL NCServerSelectWnd::HasNoPing(INT ServerIndex)
{
    INT Ping = 0;
    if (GConfig->GetInt(appItoa(ServerIndex), TEXT("Time"), Ping, TEXT("s_info.ini")))
        return Ping == PING_None;
    return 1;
}
NW_HOOK_METHOD(0x100c85e0, NCServerSelectWnd, HasNoPing)

// 0x100c8640 - not inlined: the stock caller calls it
__declspec(noinline) void NCServerSelectWnd::SavePing(INT ServerIndex, INT Ping)
{
    TCHAR Section[8];
    appSprintf(Section, TEXT("%d"), ServerIndex);
    GConfig->SetInt(Section, TEXT("Time"), Ping, TEXT("s_info.ini"));
    bPingSaved = 1;
}
NW_HOOK_METHOD(0x100c8640, NCServerSelectWnd, SavePing)

// 0x100c8690 - after new pings were asked: when they were taken. Not inlined: the stock
// callers call it.
__declspec(noinline) void NCServerSelectWnd::SavePingTime()
{
    if (bPingCacheOld)
    {
        GConfig->SetInt(TEXT("Date"), TEXT("Time"), GetTimeStamp(), TEXT("s_info.ini"));
        bPingCacheOld = 0;
    }
}
NW_HOOK_METHOD(0x100c8690, NCServerSelectWnd, SavePingTime)

// 0x100c8410
UBOOL NCServerSelectWnd::IsLeapYear(INT Year)
{
    return (Year % 4 == 0 && Year % 100 != 0) || Year % 400 == 0;
}
NW_HOOK_FUNC(0x100c8410, NCServerSelectWnd::IsLeapYear)

// 0x100c8450 - days of the years 1 .. Years
INT NCServerSelectWnd::DaysInYears(INT Years)
{
    INT Days = 0;
    for (INT i = 1; i <= Years; i++)
        Days += IsLeapYear(i) ? 366 : 365;
    return Days;
}
NW_HOOK_FUNC(0x100c8450, NCServerSelectWnd::DaysInYears)

// 0x100c8490 - days of the months before Month (January = 1)
INT NCServerSelectWnd::DaysInMonths(INT Year, INT Month)
{
    INT February = 28 + (IsLeapYear(Year) != 0);
    INT Days = 0;
    for (INT i = 1; i < Month; i++)
    {
        if (i == 1 || i == 3 || i == 5 || i == 7 || i == 8 || i == 10 || i == 12)
            Days += 31;
        else if (i == 2)
            Days += February;
        else
            Days += 30;
    }
    return Days;
}
NW_HOOK_FUNC(0x100c8490, NCServerSelectWnd::DaysInMonths)

// 0x100c8500 - hours of the days before Day, plus Hour
INT NCServerSelectWnd::HoursInDays(INT Day, INT Hour)
{
    INT Hours = 0;
    if (Day > 1)
        Hours = (Day - 1) * 24;
    return Hours + Hour;
}
NW_HOOK_FUNC(0x100c8500, NCServerSelectWnd::HoursInDays)

// 0x100c8530 - "now" for the ping cache: the days of the past years and of the months before
// last month plus the hours of this month, added as they are (the stock code mixes units)
INT NCServerSelectWnd::GetTimeStamp()
{
    INT Year, Month, Day, Hour, Min;
    appLocalTime(Year, Month, Day, Hour, Min);
    INT Stamp = DaysInMonths(Year, Month - 1);
    Stamp += DaysInYears(Year - 1);
    return HoursInDays(Day, Hour) + Stamp;
}
NW_HOOK_FUNC(0x100c8530, NCServerSelectWnd::GetTimeStamp)

NW_CHECK_VTABLE(NCServerSelectWnd, 0x10124c88, 71)
