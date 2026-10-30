/*=============================================================================
    NCChatWnd.cpp - the chat window. Stock: 0x100376f0 .. 0x10038395, 0x100383a0 .. 0x1003844a,
    0x100387f0 .. 0x10038e0f, 0x10038e20 .. 0x10039064, 0x1003aed0 .. 0x1003b693,
    0x1003b6a0 .. 0x1003ca63, 0x1003cc20 .. 0x1003d73f; 0x1003dab0 (HideWindow, shared).
=============================================================================*/
#include "nwindow/NCChatWnd.h"
#include "nwindow/NCChatFilterWnd.h"
#include "nwindow/NCConsole.h"
#include "nwindow/NCEditBox.h"
#include "nwindow/NCIMEManager.h"
#include "nwindow/NCChineseIME.h"
#include "nwindow/NCCompositeBox.h"
#include "nwindow/NCTabControl.h"
#include "nwindow/NCButton.h"
#include "nwindow/NCFrameCtrl.h"
#include "nwindow/NCSliderBar.h"
#include "nwindow/NCGDevice.h"
#include "nwindow/NCPetitionBeginWnd.h"
#include "nwindow/NCPetitionWnd.h"
#include "nwindow/NCMultiEdit.h"
#include "nwindow/NCTutorialViewerWnd.h"
#include "nwindow/NConsoleWnd.h"
#include "nwindow/NString.h"
#include "nwindow/NDialogInfo.h"
#include "nwindow/NCrt.h"

#if NW_FEATURES
// phase 6: CombatLog (NWindow_C1 src/inject/combatlog/CombatLog.cpp)
#include "features/CombatLog.h"
#endif

// Say2 types (the chat channels) as the tabs use them (names ours)
enum
{
    SAY2_Normal = 0,
    SAY2_Shout = 1,
    SAY2_Tell = 2,
    SAY2_Party = 3,
    SAY2_Pledge = 4,
    SAY2_System = 5,
    SAY2_Market = 8,
    SAY2_Ally = 9,
};

/*-----------------------------------------------------------------------------
    The flood control (new in C2).
-----------------------------------------------------------------------------*/

// 0x10048ca0
NChatLimiter::NChatLimiter()
{
    Limit10 = 20000;
    Threshold = 10000;
    Limit18 = 100000;
    Decay = 200;
    Limit20 = 60;
    Limit24 = 100;
    Limit28 = 2000;
    Factor2C = 0.1f;
}
NW_HOOK_CTOR(0x10048ca0, NChatLimiter)

// 0x10047990 - the total and the flag to 0, every text freed, the list emptied
void NChatLimiter::Clear()
{
    Points = 0;
    bLimited = 0;
    for (INT i = 0; i < Items.Num(); i++)
        delete Items(i).Text;
    Items.Empty();
}
NW_HOOK_METHOD(0x10047990, NChatLimiter, Clear)

// 0x100453c0 - one second: the total and every line's points fade by Decay (not below 0); when everything is back
// under the threshold and the chat was blocked, it is freed and the player is told
void NChatLimiter::Tick()
{
    INT Old = Points;
    Points = Old - Decay;
    if (Points < 0)
        Points = 0;
    UBOOL bUnder = Points <= Threshold;
    for (INT i = 0; i < Items.Num(); i++)
    {
        Items(i).Points -= Decay;
        if (Items(i).Points < 0)
            Items(i).Points = 0;
        if (Threshold < Items(i).Points)
            bUnder = 0;
    }
    if (bLimited && bUnder)
    {
        bLimited = 0;
        GConsole->ShowSystemMessage(965, 0xFFB09B79);
    }
}
NW_HOOK_METHOD(0x100453c0, NChatLimiter, Tick)

// 0x100479f0 - a line sent: Limit20 points a character and Limit24 for the line, and the running total's Factor2C
// share; a text that was sent before (the list keeps what it saw) counts again with its own points and Limit28.
// Over Limit10, the total or one line's points block the chat (bLimited) and the player is told (964, then 1078)
void NChatLimiter::AddLine(INT Length, const TCHAR* Text)
{
    if (!Text)
        return;
    INT Old = Points;
    Points = Old + (INT)(__int64)((DOUBLE)(Limit20 * Length + Limit24) + (DOUBLE)Old * Factor2C);   // _ftol: 64 bits, the low dword
    if (!bLimited && Points > Limit10)
    {
        bLimited = 1;
        GConsole->ShowSystemMessage(964, 0xFFB09B79);
        GConsole->ShowSystemMessage(1078, 0xFFB09B79);
    }
    for (INT i = 0; i < Items.Num(); i++)
    {
        if (!appStrcmp(Items(i).Text->GetAt(0), Text))
        {
            Items(i).Points += (INT)(__int64)((DOUBLE)Items(i).Points * Factor2C + (DOUBLE)Limit28);
            if (!bLimited && Items(i).Points > Limit10)
            {
                bLimited = 1;
                GConsole->ShowSystemMessage(964, 0xFFB09B79);
                GConsole->ShowSystemMessage(1078, 0xFFB09B79);
            }
            return;
        }
    }
    NString* NewText = new NString(Text);
    INT Added = Items.Add();
    Items(Added).Text = NewText;
    Items(Added).Points = Limit28;
}
NW_HOOK_METHOD(0x100479f0, NChatLimiter, AddLine)

// 0x10048c30 - "///setcl start end server sec char enter senten weight" (a builder's console command): the control
// starts over with these limits
void NChatLimiter::SetLimits(INT InLimit10, INT InThreshold, INT InLimit18, INT InDecay, INT InLimit20, INT InLimit24,
                             INT InLimit28, FLOAT InFactor2C)
{
    Clear();
    Limit10 = InLimit10;
    Threshold = InThreshold;
    Limit18 = InLimit18;
    Decay = InDecay;
    Limit20 = InLimit20;
    Limit24 = InLimit24;
    Limit28 = InLimit28;
    Factor2C = InFactor2C;
}
NW_HOOK_METHOD(0x10048c30, NChatLimiter, SetLimits)

// 0x10048c90 - the lines' array destructor (the stock keeps this copy for its unwind code; the array is at +4)
NW_HOOK_DTOR(0x10048c90, NChatLimiter)

// 0x10048dc0 - TellNames and Parser are built first; Lists[3], Lists[4] and the other children are left as allocated
NCChatWnd::NCChatWnd()
{
    HeadTex = NULL;
    BodyTex = NULL;
    Unknown9C = NULL;
    EditBox = NULL;
    TellIndex = -1;
    bTellBrowse = 0;
    Lists[CHATLIST_All] = NULL;
    Lists[CHATLIST_Party] = NULL;
    Lists[CHATLIST_Pledge] = NULL;
    OptionButton = NULL;
    CreateHeight = 0;
    Unknown14C = 0;
    Unknown158 = 0;
    Unknown150 = 2;
    LastTellName[0] = 0;
    CurTab = CHATTAB_All;
    bFilterHidden = 0;
    CaptureFile = NULL;
    Limiter.Clear();
}
NW_HOOK_CTOR(0x10048dc0, NCChatWnd)

// 0x10045dc0 (vslot 35) - the second of the flood control (the timer of OnCreate)
INT NCChatWnd::OnTimer(INT TimerID)
{
    if (TimerID == 0xF017)
        Limiter.Tick();
    return 1;
}
NW_HOOK_METHOD(0x10045dc0, NCChatWnd, OnTimer)

// 0x10044380 (vslot 25)
INT NCChatWnd::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (NCWnd::WindowProc(Msg, wParam, lParam))
        return 1;
    if (Msg == NWM_COMMAND)
    {
        if (wParam == (WPARAM)NID_CHAT_TABS)
            return OnTabSelected(lParam) != 0;
        if (wParam == (WPARAM)NID_CHAT_EDIT)
            return OnEditCommand(lParam) != 0;
        if (wParam == (WPARAM)NID_CHAT_OPTION)
            return OnOptionButton() != 0;
        if (wParam == (WPARAM)NID_CHAT_RESIZE)
            return OnResize(lParam) != 0;
    }
    return 0;
}
NW_HOOK_METHOD(0x10044380, NCChatWnd, WindowProc)

// 0x10045220 (vslot 29) - the focus goes to the input line
void NCChatWnd::SetFocus()
{
    EditBox->SetFocus();
    KillFocus();
}
NW_HOOK_METHOD(0x10045220, NCChatWnd, SetFocus)

// 0x10048ec0 (vslot 36)
INT NCChatWnd::OnCreate()
{
    NCWnd::OnCreate();
    HeadTex = FindTexture(TEXT("L2UI.ChatWnd.Head"));
    BodyTex = FindTexture(TEXT("L2UI.ChatWnd.Body"));
    BottomTex = FindTexture(TEXT("L2UI.ChatWnd.Bottom"));
    Bottom1Tex = FindTexture(TEXT("L2UI.ChatWnd.Bottom1"));
    Bottom2Tex = FindTexture(TEXT("L2UI.ChatWnd.Bottom2"));
    IMEJpTex = FindTexture(TEXT("L2UI.ChatWnd.IME_jp"));
    IMEEnTex = FindTexture(TEXT("L2UI.ChatWnd.IME_en"));
    IMEKrTex = FindTexture(TEXT("L2UI.ChatWnd.IME_kr"));
    IMETw1Tex = FindTexture(TEXT("L2UI.ChatWnd.IME_tw1"));
    IMETw2Tex = FindTexture(TEXT("L2UI.ChatWnd.IME_tw2"));
    IMECn1Tex = FindTexture(TEXT("L2UI.ChatWnd.IME_cn1"));    // new in C2
    IMECn2Tex = FindTexture(TEXT("L2UI.ChatWnd.IME_cn2"));
    IMECn3Tex = FindTexture(TEXT("L2UI.ChatWnd.IME_cn3"));
    IMECn4Tex = FindTexture(TEXT("L2UI.ChatWnd.IME_cn4"));
    IMEThTex = FindTexture(TEXT("L2UI.ChatWnd.IME_th"));
    IMETw3Tex = FindTexture(TEXT("L2UI.ChatWnd.IME_tw3"));

    ResizeFrame = new NCResizeFrame(RESIZE_Vertical);
    ResizeFrame->ID = NID_CHAT_RESIZE;
    ResizeFrame->SetWindowRect(0, 0, 24, 10);
    CreateChild(ResizeFrame, NWS_VISIBLE | 0x20);   // 0x20: meaning unknown

    EditBox = new NCEditBox(1, NULL, 1, 0x108);
    EditBox->ID = NID_CHAT_EDIT;
    EditBox->SetWindowRect(28, Height - 22, 312, 16);
    CreateChild(EditBox, NWS_VISIBLE);

    // the lists fill the window above the tab row; only the first one starts visible
    for (INT i = 0; i < CHATLIST_MAX; i++)
    {
        Lists[i] = new NCChatListBox(200, 10, 12);
        Lists[i]->ID = NID_CHAT_LIST;
        Lists[i]->SetWindowRect(6, 7, Width - 10, 128);
        CreateChild(Lists[i], NWS_NOHITTEST);
        Lists[i]->bAutoScroll = 1;
        Lists[i]->LineGap = 0;
        if (i != CHATLIST_All)
            Lists[i]->HideWindow();
    }

    TabCtrl = new NCTabControl();
    TabCtrl->ID = NID_CHAT_TABS;
    TabCtrl->SetWindowRect(28, Height - 43, 315, 17);   // C2: 315 wide (C1 251)
    CreateChild(TabCtrl, NWS_VISIBLE);
    TabCtrl->AddTab(0x90, Lists[CHATLIST_All], 63, 17, TEXT("L2UI.ChatWnd.ChatTab2"),
                    TEXT("L2UI.ChatWnd.ChatTab1"), 0, -1);
    TabCtrl->AddTab(0x163, Lists[CHATLIST_Market], 63, 17, TEXT("L2UI.ChatWnd.ChatTab2"),
                    TEXT("L2UI.ChatWnd.ChatTab1"), 0, -1);
    TabCtrl->AddTab(0xBC, Lists[CHATLIST_Party], 63, 17, TEXT("L2UI.ChatWnd.ChatTab2"),
                    TEXT("L2UI.ChatWnd.ChatTab1"), 0, -1);
    TabCtrl->AddTab(0x80, Lists[CHATLIST_Pledge], 63, 17, TEXT("L2UI.ChatWnd.ChatTab2"),
                    TEXT("L2UI.ChatWnd.ChatTab1"), 0, -1);
    TabCtrl->AddTab(0x22F, Lists[CHATLIST_Ally], 63, 17, TEXT("L2UI.ChatWnd.ChatTab2"),
                    TEXT("L2UI.ChatWnd.ChatTab1"), 0, -1);

    OptionButton = new NCButton(TEXT("L2UI.ChatWnd.OptionBtn"), TEXT("L2UI.ChatWnd.OptionBtn_click"), 0);
    OptionButton->ID = NID_CHAT_OPTION;
    OptionButton->SetWindowRect(3, Height - 40, 16, 16);
    CreateChild(OptionButton, NWS_VISIBLE);
    CreateHeight = Height;

    FilterWnd = new NCChatFilterWnd();
    FilterWnd->SetWindowRect(0, -161, 240, 161);
    CreateChild(FilterWnd, NWS_VISIBLE);
    FilterWnd->LoadFilters();
    TimerHandle = SetTimer(0xF017, 1000, NULL);   // new in C2: the flood control's second
    return 1;
}
NW_HOOK_METHOD(0x10048ec0, NCChatWnd, OnCreate)

// 0x10047dd0 (vslot 37) - the names in TellNames are freed, the array itself goes with the
// destructor
INT NCChatWnd::OnDestroy()
{
    guard(NCChatWnd::OnDestroy);
    if (EditBox)
    {
        RemoveChildWindow(EditBox->Entry);
        delete EditBox;
        EditBox = NULL;
    }
    for (INT i = 0; i < CHATLIST_MAX; i++)
    {
        if (Lists[i])
        {
            RemoveChildWindow(Lists[i]->Entry);
            delete Lists[i];
            Lists[i] = NULL;
        }
    }
    if (TabCtrl)
    {
        RemoveChildWindow(TabCtrl->Entry);
        delete TabCtrl;
        TabCtrl = NULL;
    }
    if (OptionButton)
    {
        RemoveChildWindow(OptionButton->Entry);
        delete OptionButton;
        OptionButton = NULL;
    }
    if (Unknown138)
    {
        RemoveChildWindow(Unknown138->Entry);
        delete Unknown138;
        Unknown138 = NULL;
    }
    if (FilterWnd)
    {
        RemoveChildWindow(FilterWnd->Entry);
        delete FilterWnd;
        FilterWnd = NULL;
    }
    if (ResizeFrame)
    {
        RemoveChildWindow(ResizeFrame->Entry);
        delete ResizeFrame;
        ResizeFrame = NULL;
    }
    Limiter.Clear();   // C2
    for (INT i = 0; i < TellNames.Num(); i++)
        delete TellNames(i);
    NCWnd::OnDestroy();
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x10047dd0, NCChatWnd, OnDestroy)

// 0x10045240 (vslot 43) - NCWnd shows every child; the tab row shows the current list
void NCChatWnd::ShowWindow()
{
    NCWnd::ShowWindow();
    Lists[CHATLIST_Pledge]->HideWindow();
    Lists[CHATLIST_Party]->HideWindow();
    TabCtrl->ShowCurrentPage();
    FilterWnd->HideWindow();
    bFilterHidden = 1;
}
NW_HOOK_METHOD(0x10045240, NCChatWnd, ShowWindow)

// 0x1007b470 (vslot 44) - the same body serves NCAbnormalStatusWnd, NCPetitionWnd and others
void NCChatWnd::HideWindow()
{
    NCWnd::HideWindow();
}
NW_HOOK_METHOD(0x1007b470, NCChatWnd, HideWindow)

// vslot 44 - stock body shared with NCWnd::OffsetWindow (0x10039080): the chat window does
// not snap
void NCChatWnd::MoveWindow(INT DX, INT DY)
{
    LocalX += DX;
    LocalY += DY;
    OffsetScreen(DX, DY);
}

// 0x100458a0 (vslot 55) - frame, the IME state icon left of the input line and, for the
// Chinese client, the place of the IME's window
INT NCChatWnd::OnPaint(NCGDevice* Device)
{
    Device->BeginPaint(ScreenX, ScreenY);
    Device->SetClip(ScreenX, ScreenY, Width, Height);
    if (HeadTex)
        Device->DrawTexture(0, 1, 345, 10, 0, 0, 345, 10, HeadTex);
    if (Bottom1Tex)
        Device->DrawTexture(0, Height - 43, 345, 44, 0, 0, 345, 44, Bottom1Tex);
    for (INT i = 0; i < (INT)((DWORD)(Height - 44) / 12); i++)
    {
        if (BodyTex)
            Device->DrawTexture(0, 11 + i * 12, 345, 12, 0, 0, 345, 12, BodyTex);
    }

    // the icon of the IME (C2 has the Chinese IMEs and the Thai one too)
    INT IME = GConsole->GetIMEType();
    UTexture* IMETex = NULL;
    if (IME == 0)
        IMETex = IMEKrTex;
    else if (IME == 1)
        IMETex = IMEEnTex;
    else if (IME == 2)
        IMETex = IMEJpTex;
    else if (IME == 6)
        IMETex = IMETw1Tex;
    else if (IME == 4)
        IMETex = IMETw2Tex;
    else if (IME == 5)
        IMETex = IMECn1Tex;
    else if (IME == 8)
        IMETex = IMECn2Tex;
    else if (IME == 9)
        IMETex = IMECn3Tex;
    else if (IME == 10)
        IMETex = IMECn4Tex;
    else if (IME == 11)
        IMETex = IMEThTex;
    else if (IME == 12)
        IMETex = IMEThTex;
    else if (IME == 13)
        IMETex = IMETw3Tex;
    else if (IME == -1)
        IMETex = FindTexture(TEXT("NWindow.BlackTexture"));
    if (IMETex)
        Device->DrawTexture(3, Height - 21, 16, 16, 0, 0, 16, 16, IMETex);

    if (EditBox && GLanguageType == 4 && EditBox->Unknown1F0 && GConsole->GetIMEType() != 8)
    {
        NCIMEManager* Manager = EditBox->IMEManager;
#if NW_HOOK_MODE
        // the stock code reads the Chinese IME at NCIMEManager +0x1B0 (that header has it at +0x1B4)
        NCChineseIME* Chinese = Manager ? nw::Field<NCChineseIME*>(Manager, 0x1B0) : NULL;
#else
        NCChineseIME* Chinese = Manager ? static_cast<NCChineseIME*>(Manager->ChineseIME) : NULL;
#endif
        if (Chinese)
        {
            // the Chinese IME's composition window goes above the chat
            NCWnd* IMEWnd = (NCWnd*)Chinese->CompositeBox;
            if (IMEWnd)
                IMEWnd->SetWindowRect(0, 6 - LocalH, 100, 15);
        }
    }
    Device->ReleaseClip();
    Device->EndPaint();
    return 1;
}
NW_HOOK_METHOD(0x100458a0, NCChatWnd, OnPaint)

/*-----------------------------------------------------------------------------
    Text capture: GL2TextCapture is set by the options (1 start, 3 stop), vslot 66 acts on
    it; while it is 2 every line the current tab shows goes to L2_mm_dd_hh_mm.log (UTF-16).
-----------------------------------------------------------------------------*/

// 0x10044470 (vslot 67)
void NCChatWnd::StartTextCapture()
{
    if (GL2TextCapture != 1)
        return;
    INT Year, Month, DayOfWeek, Day, Hour, Min, Sec, MSec;
    TCHAR Line[1024];
    TCHAR FileName[1024];
    appSystemTime(Year, Month, DayOfWeek, Day, Hour, Min, Sec, MSec);
    appSprintf(FileName, TEXT("L2_%02d_%02d_%02d_%02d.log"), Month, Day, Hour, Min);
    CaptureFile = GFileManager->CreateFileWriter(FileName, FILEWRITE_Unbuffered, GNull);
    if (CaptureFile)
    {
        INT ByteOrderMark = 0xFEFF;   // the file is UTF-16 LE: FF FE
        CaptureFile->Serialize(&ByteOrderMark, 2);
        appSprintf(Line, TEXT("[ L2 Text Capture] [ %02d/%02d %02d:%02d:%02d ]"), Month, Day, Hour,
                   Min, Sec);
        WriteCapture(Line);
    }
    appSprintf(Line, TEXT("[ Text Capture Start ][ FileName : %s ]"), FileName);
    NCChatListBox* List = TabList(CurTab);
    if (List)
        List->AddString(Line, (DWORD)-1, -1);
    GL2TextCapture = 2;
}
NW_HOOK_METHOD(0x10044470, NCChatWnd, StartTextCapture)

// 0x100445f0 (vslot 68)
void NCChatWnd::StopTextCapture()
{
    if (GL2TextCapture != 3)
        return;
    if (CaptureFile)
    {
        CaptureFile->Close();
        delete CaptureFile;
        CaptureFile = NULL;
    }
    NCChatListBox* List = TabList(CurTab);
    if (List)
        List->AddString(TEXT("=====[ Text Capture End ]====="), (DWORD)-1, -1);
    GL2TextCapture = 0;
}
NW_HOOK_METHOD(0x100445f0, NCChatWnd, StopTextCapture)

// 0x100446c0 (vslot 69)
void NCChatWnd::WriteCapture(const TCHAR* Text)
{
    if (CaptureFile)
    {
        CaptureFile->Serialize((void*)Text, appStrlen(Text) * sizeof(TCHAR));
        CaptureFile->Serialize((void*)TEXT("\r\n"), appStrlen(TEXT("\r\n")) * sizeof(TCHAR));
    }
}
NW_HOOK_METHOD(0x100446c0, NCChatWnd, WriteCapture)

// 0x10044720 (vslot 70)
void NCChatWnd::CaptureMessage(const TCHAR* Text, INT Type, INT SubType)
{
    if (GL2TextCapture != 2)
        return;
    NCChatListBox* List = TabList(CurTab);
    if (List && List->IsChannelShown(Type, SubType))
        WriteCapture(Text);
}
NW_HOOK_METHOD(0x10044720, NCChatWnd, CaptureMessage)

// 0x10044440 (vslot 71)
void NCChatWnd::UpdateTextCapture()
{
    if (GL2TextCapture == 1)
        StartTextCapture();
    else if (GL2TextCapture == 3)
        StopTextCapture();
}
NW_HOOK_METHOD(0x10044440, NCChatWnd, UpdateTextCapture)

/*-----------------------------------------------------------------------------
    Tabs, filter panel, resizing.
-----------------------------------------------------------------------------*/

// the input line starts with a channel prefix, ASCII or full width
static UBOOL StartsWithPrefix(const TCHAR* First, INT Type)
{
    return !wcscmp(First, GChatPrefix[Type][0]) || !wcscmp(First, GChatPrefix[Type][1]);
}
static UBOOL StartsWithTabPrefix(const TCHAR* First)
{
    return StartsWithPrefix(First, SAY2_Party) || StartsWithPrefix(First, SAY2_Pledge) ||
           StartsWithPrefix(First, SAY2_Market) || StartsWithPrefix(First, SAY2_Ally);
}

// 0x10048250 (vslot 72) - the tab row picked a tab: its list is shown and the filter panel
// shows its filter. With GIsOldChat the input line follows the tab: the tab's prefix
// (# party, @ pledge, + market, $ ally) replaces the one of another tab.
INT NCChatWnd::OnTabSelected(INT Tab)
{
    static const INT TabType[] = { SAY2_Normal, SAY2_Market, SAY2_Party, SAY2_Pledge, SAY2_Ally };
    TCHAR First[2];
    First[0] = 0;
    if (Tab < CHATTAB_All || Tab > CHATTAB_Ally)
        return 1;
    CurTab = Tab;
    NCChatListBox* Shown = TabList(Tab);
    static const INT Order[] = { CHATLIST_All, CHATLIST_Pledge, CHATLIST_Party, CHATLIST_Market,
                                 CHATLIST_Ally };
    for (INT i = 0; i < CHATLIST_MAX; i++)
    {
        if (Lists[Order[i]] == Shown)
            Lists[Order[i]]->ShowWindow();
        else
            Lists[Order[i]]->HideWindow();
    }
    if (GIsOldChat)
    {
        if (EditBox->CommitText())
            wcsncpy(First, EditBox->CommitText(), 1);
        First[1] = 0;
        if (Tab == CHATTAB_All)
        {
            if (StartsWithTabPrefix(First))
                EditBox->DeleteChar(0);
        }
        else if (!StartsWithPrefix(First, TabType[Tab]))
        {
            if (StartsWithTabPrefix(First))
                EditBox->DeleteChar(0);
            EditBox->InsertText(GChatPrefix[TabType[Tab]][0], 0);
        }
    }
    FilterWnd->SetFilter(TabList(Tab)->Filter);
    return 1;
}
NW_HOOK_METHOD(0x10048250, NCChatWnd, OnTabSelected)

// 0x10048a20 (vslot 74) - the option button opens the filter panel on the current tab's
// filter (re-read from chatfilter.ini) or closes it
INT NCChatWnd::OnOptionButton()
{
    if (bFilterHidden)
    {
        FilterWnd->LoadFilters();
        FilterWnd->ShowWindow();
        bFilterHidden = 0;
    }
    else
    {
        FilterWnd->HideWindow();
        bFilterHidden = 1;
    }
    if (CurTab < CHATTAB_All || CurTab > CHATTAB_Ally)
        return 1;
    FilterWnd->SetFilter(TabList(CurTab)->Filter);
    return 1;
}
NW_HOOK_METHOD(0x10048a20, NCChatWnd, OnOptionButton)

// 0x10045b30 (vslot 75) - the grip on the top edge sends the mouse's vertical moves; the
// window grows upwards in 12 px steps (one line), between 174 and 500 px. The lists keep
// their last lines in view.
INT NCChatWnd::OnResize(INT Delta)
{
    ResizeDelta += Delta;
    if (ResizeDelta < 12 && ResizeDelta > -12)
        return 1;
    ResizeDelta = -(ResizeDelta / 12) * 12;
    DWORD NewHeight = (DWORD)(ResizeDelta + Height);
    if (NewHeight >= 174 && NewHeight <= 500)
    {
        static const INT Order[] = { CHATLIST_All, CHATLIST_Pledge, CHATLIST_Party,
                                     CHATLIST_Market, CHATLIST_Ally };
        NCChatListBox* List = NULL;
        for (INT i = 0; i < CHATLIST_MAX; i++)
        {
            if (Lists[Order[i]])
                List = Lists[Order[i]];
            // the visible line count is taken from the height before the resize
            INT H = List->LocalH;
            List->SetSize(Width - 10, ResizeDelta + H);
            List->ScrollBar->SetSize(12, H + ResizeDelta - 24);
            INT OldVisible = List->VisibleLines;
            List->VisibleLines = (DWORD)H / 12;
            if (ResizeDelta > 0 || List->Items.Num() > H / 12)
            {
                INT Top = List->TopIndex - List->VisibleLines + OldVisible;
                if (Top < 0)
                    Top = 0;
                List->TopIndex = Top;
            }
            List->ScrollBar->SetRange(List->NumLines, List->VisibleLines);
            List->ScrollBar->SetValue(List->TopIndex);
            List->ScrollDownButton->MoveWindow(0, ResizeDelta);
        }
        MoveWindow(0, -ResizeDelta);
        EditBox->MoveWindow(0, ResizeDelta);
        OptionButton->MoveWindow(0, ResizeDelta);
        TabCtrl->MoveWindow(0, ResizeDelta);
        SetSize(345, Height + ResizeDelta);
        ResizeFrame->SetWindowRect(0, 0, 24, 10);
        // the tutorial's question mark, docked above the chat
        GDesktop->TutorialQuestionWnd->SetWindowRect(5, GSEKScreenY - Height - 37, 0, 0);
    }
    ResizeDelta = 0;
    return 1;
}
NW_HOOK_METHOD(0x10045b30, NCChatWnd, OnResize)

/*-----------------------------------------------------------------------------
    Messages.
-----------------------------------------------------------------------------*/

// every list whose filter has both switches on gets the line (inlined per channel in the
// stock code; the list pointers are not tested there)
static void AddToLists(NCChatListBox* const* Lists, UBOOL NChatFilter::*Channel,
                       UBOOL NChatFilter::*Master, const TCHAR* Text, DWORD Color)
{
    for (INT i = 0; i < CHATLIST_MAX; i++)
        if (Lists[i]->Filter.*Channel && Lists[i]->Filter.*Master)
            Lists[i]->AddString(Text, Color, -1);
}

// 0x100447b0 - a chat or system line from the server (NConsoleWnd): each tab filters it by
// its type; a GM's answer to a petition (5/6) goes to the petition windows instead
void NCChatWnd::AddChatMessage(const TCHAR* Text, DWORD Color, INT Type, INT SubType)
{
#if NW_FEATURES
    // phase 6: CombatLog (NWindow_C1 src/inject/combatlog/CombatLog.cpp): the system lines go
    // to the combat log instead
    if (nwf::CombatLog_DivertChatMessage(Text, Color, Type, SubType))
        return;
#endif
    switch (Type)
    {
    case SAY2_Market:
        AddToLists(Lists, &NChatFilter::bMarket, &NChatFilter::bChat, Text, Color);
        break;
    case SAY2_Pledge:
        AddToLists(Lists, &NChatFilter::bPledge, &NChatFilter::bChat, Text, Color);
        break;
    case SAY2_Party:
        AddToLists(Lists, &NChatFilter::bParty, &NChatFilter::bChat, Text, Color);
        break;
    case SAY2_Shout:
        AddToLists(Lists, &NChatFilter::bShout, &NChatFilter::bChat, Text, Color);
        break;
    case SAY2_Tell:
        AddToLists(Lists, &NChatFilter::bTell, &NChatFilter::bChat, Text, Color);
        break;
    case SAY2_Normal:
        AddToLists(Lists, &NChatFilter::bNormal, &NChatFilter::bChat, Text, Color);
        break;
    case SAY2_Ally:
        AddToLists(Lists, &NChatFilter::bAlly, &NChatFilter::bChat, Text, Color);
        break;
    case 6:
    case 7:
    case 10:
        for (INT i = 0; i < CHATLIST_MAX; i++)
            if (Lists[i])
                Lists[i]->AddString(Text, Color, -1);
        break;
    }
    if (Type == SAY2_System)
    {
        if (SubType == 3)
            AddToLists(Lists, &NChatFilter::bDamage, &NChatFilter::bSystem, Text, Color);
        else if (SubType == 7)   // new in C2: the messages of used items
            AddToLists(Lists, &NChatFilter::bUseItems, &NChatFilter::bSystem, Text, Color);
        else if (SubType == 6)
        {
            // the petition chat goes to the GM's window on the desktop; while that one is
            // hidden, also to the player's, which is shown
            GDesktop->GMPetitionWnd->AddMessage((TCHAR*)Text, 0xFF80FFFF, 5, 0);
            if (!GDesktop->GMPetitionWnd->IsShowWindow())
            {
                GConsole->PetitionWnd->ShowWindow();
                static_cast<NCPetitionWnd*>(GConsole->PetitionWnd)
                    ->AddMessage((TCHAR*)Text, 0xFF80FFFF, 5, 0);
            }
        }
        else
        {
            for (INT i = 0; i < CHATLIST_MAX; i++)
                if (Lists[i] && Lists[i]->Filter.bSystem)
                    Lists[i]->AddString(Text, Color, -1);
        }
    }
    if (GL2TextCapture == 2)
        CaptureMessage(Text, Type, SubType);
}
NW_HOOK_METHOD(0x100447b0, NCChatWnd, AddChatMessage)

/*-----------------------------------------------------------------------------
    Tell partners: '"' typed first puts the last one in the line, Up/Down walk the list.
-----------------------------------------------------------------------------*/

// 0x10045050 - "name " of a tell just sent. A 31-character name writes the terminator over
// TellNames (stock bug; names have 16 characters at most).
void NCChatWnd::SetLastTellName(const TCHAR* Name)
{
    if (Name && *Name && wcslen(Name) < 32)
    {
        appStrcpy(LastTellName, Name);
        INT Len = (INT)wcslen(LastTellName);
        TCHAR* P = LastTellName;
        P[Len] = ' ';
        P[Len + 1] = 0;
    }
}
NW_HOOK_METHOD(0x10045050, NCChatWnd, SetLastTellName)

// 0x100456e0 - NULL: the last partner
__declspec(noinline) void NCChatWnd::BeginTell(const TCHAR* Name)
{
    TCHAR Buffer[24];
    if (Name)
    {
        appStrcpy(Buffer, Name);
        INT Len = (INT)wcslen(Buffer);
        Buffer[Len] = ' ';
        Buffer[Len + 1] = 0;
        EditBox->InsertText(Buffer, 1);
    }
    else if (TellNames.Num() > 0)
    {
        appStrcpy(Buffer, TellNames(TellNames.Num() - 1)->GetAt(0));
        INT Len = (INT)wcslen(Buffer);
        Buffer[Len] = ' ';
        Buffer[Len + 1] = 0;
        EditBox->InsertText(Buffer, 1);
    }
    bTellBrowse = 1;
}
NW_HOOK_METHOD(0x100456e0, NCChatWnd, BeginTell)

// 0x10045780 - the line becomes "name ; Down past the newest leaves only the "
__declspec(noinline) void NCChatWnd::BrowseTellNames(UBOOL bUp)
{
    if (!bTellBrowse)
        return;
    if (bUp)
    {
        if (--TellIndex < 0)
            TellIndex = 0;
        if (TellNames.Num() <= 0)
            return;
    }
    else
    {
        INT Last = TellNames.Num() - 1;
        if (TellIndex == Last)
        {
            TellIndex++;
            EditBox->ClearText();
            EditBox->InsertText(GChatPrefix[SAY2_Tell][0], 1);
            return;
        }
        if (TellIndex > Last)
            return;
        if (++TellIndex > Last)
            TellIndex = Last;
        if (TellNames.Num() <= 0)
            return;
    }
    if (!TellNames(TellIndex)->GetAt(0))
        return;
    EditBox->ClearText();
    EditBox->InsertText(GChatPrefix[SAY2_Tell][0], 1);
    EditBox->InsertText(TellNames(TellIndex)->GetAt(0), 1);
    EditBox->InsertText(TEXT(" "), 1);
}
NW_HOOK_METHOD(0x10045780, NCChatWnd, BrowseTellNames)

// 0x10048060 - a known name moves to the newest place; past 64 names the oldest goes.
// Names starting with '-' are not kept.
void NCChatWnd::AddTellName(const TCHAR* Name)
{
    if (!Name || !*Name || wcslen(Name) >= 32 || Name[0] == '-')
        return;
    for (INT i = 0; i < TellNames.Num(); i++)
    {
        if (!_wcsicmp(TellNames(i)->GetAt(0), Name))
        {
            NString* Found = TellNames(i);
            INT j;
            for (j = i; j < TellNames.Num() - 1; j++)
                TellNames(j) = TellNames(j + 1);
            TellNames(j) = Found;
            TellIndex = TellNames.Num() - 1;
            return;
        }
    }
    if (TellNames.Num() > 63)
    {
        delete TellNames(0);
        INT j;
        for (j = 0; j < TellNames.Num() - 1; j++)
            TellNames(j) = TellNames(j + 1);
        TellNames(j) = new NString(Name);
    }
    else
    {
        NString* Str = new NString(Name);
        TellNames.AddItem(Str);
    }
    TellIndex = TellNames.Num() - 1;
}
NW_HOOK_METHOD(0x10048060, NCChatWnd, AddTellName)

// 0x1004b5a0
void NCChatWnd::EnablePledgeTab(bool bEnable)
{
    TArray<NCRadioButton*> Tabs = TabCtrl->GetTabs();
    Tabs(CHATTAB_Pledge)->EnableWindow(bEnable);
}
NW_HOOK_METHOD(0x1004b5a0, NCChatWnd, EnablePledgeTab)

// 0x1004b610
void NCChatWnd::EnableAllyTab(bool bEnable)
{
    TArray<NCRadioButton*> Tabs = TabCtrl->GetTabs();
    Tabs(CHATTAB_Ally)->EnableWindow(bEnable);
}
NW_HOOK_METHOD(0x1004b610, NCChatWnd, EnableAllyTab)

/*-----------------------------------------------------------------------------
    The input line: Enter runs it (chat or a /command).
-----------------------------------------------------------------------------*/

// C is a channel prefix, ASCII or full width
static FORCEINLINE UBOOL IsPrefixChar(TCHAR C, INT Type)
{
    return C == GChatPrefix[Type][0][0] || C == GChatPrefix[Type][1][0];
}

// the word a command takes, its %target-style names replaced in place by what they stand for (0x1003e900, a method of
// the parser: 1 when the word was replaced; the stock calls it on every token, and mostly ignores what it returns)
static FORCEINLINE INT ExpandToken(NCommandParser& Parser, TCHAR* Token)
{
    return Parser.ExpandToken(Token);
}

// 0x100450b0 - a chat line (In) with the %target-style words of its words replaced (ExpandToken, which returns 1 for a
// word it replaced, else 0), put together in Out: the first word as it is (a line that starts with '$' keeps the '$'
// apart: a space follows it only when the first word was not replaced), every other word after a space, except after a
// word that was replaced. When no word was replaced, Out is a copy of In. The last two words' results are kept in turn
void NCChatWnd::ExpandLine(TCHAR* Out, TCHAR* In)
{
    INT Found[2];
    Found[0] = 0;
    Found[1] = 0;
    UBOOL bUnchanged = 1;
    if (In && Out)
    {
        TCHAR* Token = Parser.SetString(In);
        Found[0] = ExpandToken(Parser, Token);
        if (In[0] == '$')
        {
            if (Found[0])
                appSprintf(Out, TEXT("%s%s"), TEXT("$"), Token);
            else
                appSprintf(Out, TEXT("%s %s"), TEXT("$"), Token);
        }
        else
            appSprintf(Out, TEXT("%s"), Token);
        Token = Parser.GetNextToken();
        Found[1] = ExpandToken(Parser, Token);
        INT Index = 0;
        if (Found[0] || Found[1])
            bUnchanged = 0;
        while (Token)
        {
            INT Slot = Index % 2;
            if (Found[Slot])
                appStrcat(Out, Token);
            else
            {
                appStrcat(Out, TEXT(" "));
                appStrcat(Out, Token);
            }
            Token = Parser.GetNextToken();
            Found[Slot] = ExpandToken(Parser, Token);
            Index++;
            if (Found[0] || Found[1])
                bUnchanged = 0;
        }
        if (bUnchanged)
            appStrcpy(Out, In);
    }
}
NW_HOOK_METHOD(0x100450b0, NCChatWnd, ExpandLine)

// 0x1004b6d0 - the tell names, the flood control's lines and the command parser go with the window
NW_HOOK_DTOR(0x1004b6d0, NCChatWnd)

// the rest of the command's words as one text: each expanded, the first copied, the others joined to it by spaces
// (the stock repeats this in every command that takes a name; an absent first word is copied as it is)
static FORCEINLINE void JoinTokens(NCommandParser& Parser, TCHAR* Out)
{
    TCHAR* Token = Parser.GetNextToken();
    ExpandToken(Parser, Token);
    appSprintf(Out, TEXT("%s"), Token);
    Token = Parser.GetNextToken();
    ExpandToken(Parser, Token);
    while (Token)
    {
        appStrcat(Out, TEXT(" "));
        appStrcat(Out, Token);
        Token = Parser.GetNextToken();
        ExpandToken(Parser, Token);
    }
}

// the two numbers "page slot" (1..10 each) of /useshortcut and its kind: the shortcut's index (page - 1) * 10 + slot - 1,
// -1 when a number is absent or out of range
static FORCEINLINE INT ReadShortcutIndex(NCommandParser& Parser)
{
    TCHAR* Token = Parser.GetNextToken();
    ExpandToken(Parser, Token);
    INT Page;
    if (!Token || (Page = NWtol(Token)) < 1 || Page > 10)
        return -1;
    Token = Parser.GetNextToken();
    ExpandToken(Parser, Token);
    INT Slot;
    if (!Token || (Slot = NWtol(Token)) < 1 || Slot > 10)
        return -1;
    return Slot - 11 + Page * 10;
}

// the dialog boxes the alliance commands open (NDialogInfo::ID; names ours): leave, expel, dismiss and crest, their
// buttons answered by the chat window
enum { DIALOG_ALLY_CREST = 0x2F, DIALOG_ALLY_DISMISS = 0x30, DIALOG_ALLY_LEAVE = 0x31, DIALOG_ALLY_OUST = 0x32 };

// 0x100497d0 - a line of the input line (OnEditCommand's Enter): a "/command" or a chat line. The commands are
// commandname-*.dat's codes (NCommandParser::GetCommand): "//line" is a builder's (to the server), "///line" a console
// command (builders: level 9 only "stat fps"; "///setcl start end server sec char enter senten weight" also sets the flood
// control), the user commands the server runs (0, 0x34, 0x4D, 0x51) go to it with their code, the rest are the client's
// own. Plain chat: the channel is the tab's (unless GIsOldChat) or the line's first character; the line goes out
// through the flood control (NChatLimiter::AddLine 0x100479f0: while limited, system message 966 instead), except for the
// party, pledge, alliance and the two others, which are not counted
void NCChatWnd::ProcessChatLine(TCHAR* Line)
{
    INT BuilderLevel = 0;
    User* Me = GConsole->GetMyUser();
    if (Me)
        BuilderLevel = Me->BuilderLevel;
    TCHAR Args[100];
    appMemset(Args, 0, 200);
    TCHAR Text[1024];
    appStrcpy(Text, Line);
    INT Command = Parser.GetCommand(Text);
    if (Command > NCMD_Builder)
    {
        if (Command == NCMD_Exec)
        {
            if (BuilderLevel)
            {
                if (Text[3] == 's' && Text[4] == 'e' && Text[5] == 't' && Text[6] == 'c' && Text[7] == 'l')
                {
                    // "///setcl": the limits of the flood control, the weight last
                    INT Start, End, Server, Sec, Char;
                    TCHAR* Token1 = Parser.GetNextToken();
                    if (Token1)
                        Start = appAtoi(Token1);
                    TCHAR* Token2 = Parser.GetNextToken();
                    if (Token2)
                        End = appAtoi(Token2);
                    TCHAR* Token3 = Parser.GetNextToken();
                    if (Token3)
                        Server = appAtoi(Token3);
                    TCHAR* Token4 = Parser.GetNextToken();
                    if (Token4)
                        Sec = appAtoi(Token4);
                    TCHAR* Token5 = Parser.GetNextToken();
                    if (Token5)
                        Char = appAtoi(Token5);
                    TCHAR* Token6 = Parser.GetNextToken();
                    INT Enter = Start;
                    if (Token6)
                        Enter = appAtoi(Token6);
                    TCHAR* Token7 = Parser.GetNextToken();
                    INT Senten = Start;
                    if (Token7)
                        Senten = appAtoi(Token7);
                    TCHAR* Token8 = Parser.GetNextToken();
                    if (!Token8 || !Token7 || !Token6 || !Token5 || !Token4 || !Token3 || !Token2 || !Token1)
                        GConsole->ShowChatText(TEXT("[howto] ///setcl start end server sec char enter senten weight"), (DWORD)-1);
                    else
                    {
                        FLOAT Weight = (FLOAT)appAtof(Token8);
                        Limiter.SetLimits(Start, End, Server, Sec, Char, Enter, Senten, Weight);
                    }
                }
                if (BuilderLevel != 9 || !_wcsicmp(&Text[4], TEXT("stat fps")))
                    GConsole->ExecCommand(&Text[3]);
            }
            return;
        }
        if (Command != NCMD_Chat)
            return;

        // a chat line: its channel, from the tab (unless GIsOldChat) then from the line's first character
        TCHAR Chat[1024];
        ExpandLine(Chat, Text);
        TCHAR* Message = &Chat[1];
        INT Type = SAY2_Normal;
        if (!GIsOldChat)
        {
            switch (CurTab)
            {
            case CHATTAB_All:    Type = SAY2_Normal; break;
            case CHATTAB_Market: Type = SAY2_Market; break;
            case CHATTAB_Party:  Type = SAY2_Party;  break;
            case CHATTAB_Pledge: Type = SAY2_Pledge; break;
            case CHATTAB_Ally:   Type = SAY2_Ally;   break;
            }
        }
        TCHAR First = Chat[0];
        if (IsPrefixChar(First, SAY2_Shout))
            Type = SAY2_Shout;
        else if (IsPrefixChar(First, SAY2_Pledge))
            Type = SAY2_Pledge;
        else if (IsPrefixChar(First, SAY2_Party))
            Type = SAY2_Party;
        else if (First == 0xFF02 || IsPrefixChar(First, SAY2_Tell))
            Type = SAY2_Tell;
        else if (IsPrefixChar(First, 6))
            Type = 6;
        else if (IsPrefixChar(First, 7))
            Type = 7;
        else if (IsPrefixChar(First, SAY2_Market))
            Type = SAY2_Market;
        else if (IsPrefixChar(First, SAY2_Ally))
            Type = SAY2_Ally;
        if (!GIsOldChat)
        {
            // '~' is the normal channel from any tab
            if (IsPrefixChar(First, SAY2_Normal))
                Type = SAY2_Normal;
        }
        else if (Type == SAY2_Normal)
        {
            // the old chat: the whole line, counted by the flood control
            Limiter.AddLine(appStrlen(Chat), Chat);
            if (!Limiter.bLimited)
                GConsole->SendChatMsgToServer(SAY2_Normal, Chat, NULL);
            else
                GConsole->ShowSystemMessage(0x3C6, 0xFFB09B79);
            return;
        }
        if (Type == SAY2_Tell)
        {
            // "name text; a line that starts with a space (or a name of no letter) goes to the last partner
            INT NameLen = 0;
            if (Chat[1] != ' ')
            {
                do
                {
                    if (Chat[1 + NameLen] == 0x3000 || (INT)wcslen(&Chat[1]) <= NameLen)
                        break;
                    NameLen++;
                } while (Chat[1 + NameLen] != ' ');
            }
            if (NameLen == 0)
            {
                Limiter.AddLine(appStrlen(Chat), Chat);
                if (!Limiter.bLimited)
                    GConsole->SendChatMsgToServer(SAY2_Tell, Chat, NULL);
                else
                    GConsole->ShowSystemMessage(0x3C6, 0xFFB09B79);
                return;
            }
            TCHAR Name[128];
            wcsncpy(Name, &Chat[1], NameLen);
            Name[NameLen] = 0;
            if ((INT)wcslen(Chat) <= NameLen + 2)
                return;
            TCHAR* TellText = &Chat[NameLen + 2];
            SetLastTellName(Name);
            AddTellName(Name);
            Limiter.AddLine(appStrlen(TellText), TellText);
            if (!Limiter.bLimited)
                GConsole->SendChatMsgToServer(SAY2_Tell, TellText, Name);
            else
                GConsole->ShowSystemMessage(0x3C6, 0xFFB09B79);
            return;
        }
        if (Type == SAY2_Normal && GIsOldChat)
            return;
        // the prefix is not part of the message
        if (!GIsOldChat &&
            (IsPrefixChar(First, SAY2_Shout) || IsPrefixChar(First, SAY2_Pledge) || IsPrefixChar(First, SAY2_Party) ||
             IsPrefixChar(First, SAY2_Tell) || IsPrefixChar(First, 6) || IsPrefixChar(First, 7) ||
             IsPrefixChar(First, SAY2_Market) || IsPrefixChar(First, SAY2_Ally) || IsPrefixChar(First, SAY2_Normal)))
            Message = &Chat[1];
        else
            Message = &Chat[0];
        if (!*Message)
            return;
        if (Type == SAY2_Shout || Type == SAY2_Market)
        {
            Limiter.AddLine(appStrlen(Message), Message);
            if (!Limiter.bLimited)
                GConsole->SendChatMsgToServer(Type, Message, NULL);
            else
                GConsole->ShowSystemMessage(0x3C6, 0xFFB09B79);
        }
        else if (Type != SAY2_Normal)
            GConsole->SendChatMsgToServer(Type, Message, NULL);
        else
        {
            Limiter.AddLine(appStrlen(Message), Message);
            if (!Limiter.bLimited)
                GConsole->SendChatMsgToServer(SAY2_Normal, Message, NULL);
            else
                GConsole->ShowSystemMessage(0x3C6, 0xFFB09B79);
        }
        return;
    }
    if (Command == NCMD_Builder)
    {
        if (!BuilderLevel)
            return;
        L2ParamStack Params(10);
        Params.PushBack(&Text[2]);
        GConsole->BuilderCommand(Params);
        return;
    }
    switch (Command)
    {
    case NCMD_Loc:
    case NCMD_Escape:
    case 0x4D:
    case 0x51:
    {
        // the user commands the server runs: its code goes along
        L2ParamStack Params(10);
        Params.PushBack((void*)(INT_PTR)Command);
        GConsole->BypassUserCmd(Params);
        break;
    }
    case NCMD_Petition:
    {
        // "/gm text": the petition window (the desktop's +0xBC) starts over with the text in its edit (+0xA8) and shows
        TCHAR* Rest = Parser.GetRemainStr();
        NCPetitionBeginWnd* Petition = (NCPetitionBeginWnd*)GDesktop->PetitionBeginWnd;
        Petition->ResetForm();                              // 0x100B6B40
        if (Rest)
            Petition->TextEdit->SetText(Rest);              // 0x10022230
        GDesktop->PetitionBeginWnd->ShowWindow();
        break;
    }
    case NCMD_PetitionCancel:
        GConsole->RequestPetitionCancel();
        break;
    case NCMD_GMList:
        GConsole->RequestGMList();
        break;
    case NCMD_Sit:
        GConsole->RequestChangeWaitType(0);
        break;
    case NCMD_Stand:
        GConsole->RequestChangeWaitType(1);
        break;
    case NCMD_Walk:
        GConsole->RequestChangeMoveType(0);
        break;
    case NCMD_Run:
        GConsole->RequestChangeMoveType(1);
        break;
    case NCMD_Attack:
    {
        User* Target = GConsole->GetSelectedActor();
        User* Self = GConsole->GetMyUser();
        if (Self && Target)
            GConsole->RequestAttack(Target->ObjectID, Self->Pawn->Location, 0, 0);
        break;
    }
    case NCMD_AttackForce:
    {
        User* Target = GConsole->GetSelectedActor();
        User* Self = GConsole->GetMyUser();
        if (Self && Target)
            GConsole->RequestAttackForce(Target->ObjectID, Self->Pawn->Location);
        break;
    }
    case NCMD_AttackStand:
    {
        User* Target = GConsole->GetSelectedActor();
        User* Self = GConsole->GetMyUser();
        if (Self && Target)
            GConsole->RequestAttackStand(Target->ObjectID, Self->Pawn->Location);
        break;
    }
    case NCMD_Trade:
    {
        User* Target = GConsole->GetSelectedActor();
        if (Target && Target->Pawn)
            GConsole->RequestTrade(Target->Pawn->CreatureID);
        break;
    }
    case NCMD_Target:
    {
        JoinTokens(Parser, Args);
        if (Args[0])
        {
            User* Found = GConsole->FindUserByName(Args);
            if (Found && Found->Pawn)
                GConsole->RequestTarget(Found->ObjectID);
        }
        break;
    }
    case NCMD_TargetNext:
    {
        User* Target = GConsole->GetSelectedActor();
        GConsole->RequestTargetNext(200, !Target || !Target->Pawn ? -1 : Target->Pawn->CreatureID);
        break;
    }
    case NCMD_PickUp:
        GConsole->RequestPickUp(200, -1);
        break;
    case NCMD_Assist:
        GConsole->RequestAssist();
        break;
    case NCMD_PartyInvite:
    {
        User* Target = GConsole->GetSelectedActor();
        if (!Target || Target->Unknown8)
        {
            GConsole->ShowSystemMessage(185, 0xFFB09B79);
            break;
        }
        L2ParamStack Params(10);
        Params.PushBack((void*)(INT_PTR)Target->Pawn->CreatureID);
        Params.PushBack((void*)(INT_PTR)GConsole->PartyItemDistribution);
        GConsole->RequestPartyInvite(Params);
        break;
    }
    case NCMD_PartyLeave:
    {
        L2ParamStack Params(10);
        GConsole->RequestPartyLeave(Params);
        break;
    }
    case NCMD_PartyDismiss:
    {
        // the name from the words, else the target's
        TCHAR* Token = Parser.GetNextToken();
        ExpandToken(Parser, Token);
        if (!Token)
        {
            User* Target = GConsole->GetSelectedActor();
            if (Target)
            {
                L2ParamStack Params(10);
                Params.PushBack(Target->GetName());
                GConsole->RequestPartyDismiss(Params);
                break;
            }
        }
        else
            appSprintf(Args, TEXT("%s"), Token);
        Token = Parser.GetNextToken();
        ExpandToken(Parser, Token);
        while (Token)
        {
            appStrcat(Args, TEXT(" "));
            appStrcat(Args, Token);
            Token = Parser.GetNextToken();
            ExpandToken(Parser, Token);
        }
        if (!Args[0])
        {
            GConsole->ShowSystemMessage(187, 0xFFB09B79);
            break;
        }
        L2ParamStack Params(10);
        Params.PushBack(Args);
        GConsole->RequestPartyDismiss(Params);
        break;
    }
    case NCMD_Vendor:
        GConsole->RequestVendor();
        break;
    case NCMD_PartyMatching:
        GDesktop->TogglePartyMatch();
        break;
    case NCMD_SocialHello:
        GConsole->RequestSocialAction(2);
        break;
    case NCMD_SocialVictory:
        GConsole->RequestSocialAction(3);
        break;
    case NCMD_SocialCharge:
        GConsole->RequestSocialAction(4);
        break;
    case NCMD_UseSkill:
    case NCMD_UseSkillForce:
    case NCMD_UseSkillStand:
    {
        // "/useskill name": the skill of commandname's table, with Ctrl (force) / Shift (stand)
        JoinTokens(Parser, Args);
        FL2MagicSkillCommandData* Skill = GL2GameData.GetMSCommandData(Args);
        if (!Skill)
            break;
        L2ParamStack Params(10);
        Params.PushBack((void*)(INT_PTR)Skill->SkillID);
        Params.PushBack((void*)(INT_PTR)(Command == NCMD_UseSkillForce));
        Params.PushBack((void*)(INT_PTR)(Command == NCMD_UseSkillStand));
        GConsole->RequestMagicSkillUse(Params);
        break;
    }
    case NCMD_UseShortcut:
    {
        INT Index = ReadShortcutIndex(Parser);
        if (Index >= 0)
            GConsole->UseShortCut(Index, 0, 0);
        break;
    }
    case NCMD_UseShortcutForce:
    {
        INT Index = ReadShortcutIndex(Parser);
        if (Index >= 0)
            GConsole->UseShortCut(Index, 1, 0);
        break;
    }
    case NCMD_UseShortcutStand:
    {
        INT Index = ReadShortcutIndex(Parser);
        if (Index >= 0)
            GConsole->UseShortCut(Index, 0, 1);
        break;
    }
    case NCMD_AllyInvite:
    {
        User* Target = GConsole->GetSelectedActor();
        if (!Target || Target->Unknown8)
        {
            GConsole->ShowSystemMessage(511, 0xFFB09B79);
            break;
        }
        L2ParamStack Params(10);
        Params.PushBack((void*)(INT_PTR)Target->Pawn->CreatureID);
        GConsole->ShowSystemMessage(510, 0xFFB09B79);
        GConsole->RequestAllyInvite(Params);
        break;
    }
    // the alliance commands that need a confirmation: the dialog box sends the request
    case NCMD_AllyLeave:
    {
        NDialogInfo Info;
        Info.ID = DIALOG_ALLY_LEAVE;
        Info.bInput = 0;
        Info.bMainBox = 1;
        Info.Owner = Entry;
        Info.SetSystemMessage(0x200);
        GConsole->ShowDialog(&Info);
        break;
    }
    case NCMD_AllyOust:
    {
        NDialogInfo Info;
        Info.Owner = Entry;
        Info.ID = DIALOG_ALLY_OUST;
        Info.bMainBox = 1;
        Info.bInput = 1;
        Info.Type = 0;
        Info.SetSystemMessage(0x201);
        GConsole->ShowDialog(&Info);
        break;
    }
    case NCMD_AllyDismiss:
    {
        NDialogInfo Info;
        Info.Owner = Entry;
        Info.ID = DIALOG_ALLY_DISMISS;
        Info.bMainBox = 1;
        Info.bInput = 0;
        Info.SetSystemMessage(0x202);
        GConsole->ShowDialog(&Info);
        break;
    }
    case NCMD_AllyCrest:
    {
        NDialogInfo Info;
        Info.ID = DIALOG_ALLY_CREST;
        Info.bInput = 1;
        Info.bMainBox = 1;
        Info.Type = 0;
        Info.Owner = Entry;
        Info.SetSystemMessage(0x203);
        GConsole->ShowDialog(&Info);
        break;
    }
    case NCMD_AllyInfo:
        GConsole->RequestAllyInfo();
        break;
    case NCMD_FriendInvite:
    {
        JoinTokens(Parser, Args);
        L2ParamStack Params(10);
        Params.PushBack(Args);
        GConsole->RequestFriendInvite(Params);
        break;
    }
    case NCMD_FriendList:
        GConsole->RequestFriendList();
        break;
    case NCMD_FriendDel:
    {
        JoinTokens(Parser, Args);
        L2ParamStack Params(10);
        Params.PushBack(Args);
        GConsole->RequestFriendDel(Params);
        break;
    }
    case NCMD_Buy:
        GConsole->RequestBuy();
        break;
    case NCMD_AllyWarStart:
    {
        JoinTokens(Parser, Args);
        L2ParamStack Params(10);
        Params.PushBack(Args);
        GConsole->RequestStartAllianceWar(Params);
        break;
    }
    case NCMD_AllyWarStop:
    {
        JoinTokens(Parser, Args);
        L2ParamStack Params(10);
        Params.PushBack(Args);
        GConsole->RequestStopAllianceWar(Params);
        break;
    }
    case NCMD_AllyWarSurrender:
    {
        JoinTokens(Parser, Args);
        L2ParamStack Params(10);
        Params.PushBack(Args);
        GConsole->RequestSurrenderAllianceWar(Params);
        break;
    }
    case NCMD_BlockList:
    {
        L2ParamStack Params(10);
        Params.PushBack((void*)2);
        GConsole->RequestBlock(Params);
        break;
    }
    case NCMD_Block:
    {
        JoinTokens(Parser, Args);
        L2ParamStack Params(10);
        Params.PushBack((void*)0);
        Params.PushBack(Args);
        GConsole->RequestBlock(Params);
        break;
    }
    case NCMD_Unblock:
    {
        JoinTokens(Parser, Args);
        L2ParamStack Params(10);
        Params.PushBack((void*)1);
        Params.PushBack(Args);
        GConsole->RequestBlock(Params);
        break;
    }
    case NCMD_SocialNo:
        GConsole->RequestSocialAction(5);
        break;
    case NCMD_SocialYes:
        GConsole->RequestSocialAction(6);
        break;
    case NCMD_SocialBow:
        GConsole->RequestSocialAction(7);
        break;
    case NCMD_SocialUnaware:
        GConsole->RequestSocialAction(8);
        break;
    case NCMD_SocialWaiting:
        GConsole->RequestSocialAction(9);
        break;
    case NCMD_SocialLaugh:
        GConsole->RequestSocialAction(10);
        break;
    case NCMD_DwarvenManufacture:
        GConsole->RequestDwarvenManufacture();
        break;
    // the commands C2 added (their numbers go past commandname's C1 table): more socials, the actions by their ids, the
    // recipe shop, the pet and the block kinds
    case 0x36:
        GConsole->RequestSocialAction(0xB);
        break;
    case 0x37:
        GConsole->RequestSocialAction(0xC);
        break;
    case 0x38:
        GConsole->RequestSocialAction(0xD);
        break;
    case 0x39:
        GConsole->UseAction(0, 0, 0);
        break;
    case 0x3A:
        GConsole->UseAction(1, 0, 0);
        break;
    case 0x3B:
        GConsole->RequestRecipeShopManageList();
        break;
    case 0x3C:
    case 0x3D:
    case 0x3E:
        GConsole->UseAction(0x26, 0, 0);
        break;
    case 0x3F:
        GConsole->UseAction(0xF, 0, 0);
        break;
    case 0x40:
        GConsole->UseAction(0x10, 0, 0);
        break;
    case 0x41:
        GConsole->UseAction(0x11, 0, 0);
        break;
    case 0x42:
        GConsole->PetPickUp(200, -1);
        break;
    case 0x43:
        GConsole->UseAction(0x13, 0, 0);
        break;
    case 0x45:
        GConsole->UseAction(0x15, 0, 0);
        break;
    case 0x46:
        GConsole->UseAction(0x16, 0, 0);
        break;
    case 0x47:
        GConsole->UseAction(0x17, 0, 0);
        break;
    case 0x4B:
        GConsole->UseAction(0x28, 0, 0);
        break;
    case 0x4E:
    {
        L2ParamStack Params(10);
        Params.PushBack((void*)3);
        GConsole->RequestBlock(Params);
        break;
    }
    case 0x4F:
    {
        L2ParamStack Params(10);
        Params.PushBack((void*)4);
        GConsole->RequestBlock(Params);
        break;
    }
    case 0x50:
    {
        // the shortcut slot (page, slot): its item is put on when it is an equipment not worn yet (0x10066fd0)
        INT Index = ReadShortcutIndex(Parser);
        if (Index >= 0)
            GConsole->UseShortCutEquip(Index);
        break;
    }
    }
}
NW_HOOK_METHOD(0x100497d0, NCChatWnd, ProcessChatLine)

// 0x1004b4a0 (vslot 73) - the input line's events (C2: the sending itself moved to ProcessChatLine): Up / Down (4, 5)
// walk the tell names, '"' (3) starts a tell to the last partner, Enter (1) sends the line, clears it, puts the tab's
// prefix back with GIsOldChat and, with GIsEnterChatting (Enter opens and closes the chat), drops the focus
INT NCChatWnd::OnEditCommand(INT Code)
{
    if (Code == 3)
    {
        BeginTell(NULL);
        return 1;
    }
    if (Code != 4)
    {
        if (Code != 5)
        {
            if (Code == 1)
            {
                bTellBrowse = 0;
                TCHAR* Text = (TCHAR*)EditBox->CommitText();
                if (Text)
                {
                    ProcessChatLine(Text);
                    EditBox->ClearText();
                    if (GIsOldChat)
                    {
                        const TCHAR* Prefix = NULL;
                        if (CurTab == CHATTAB_Pledge)
                            Prefix = GChatPrefix[SAY2_Pledge][0];
                        else if (CurTab == CHATTAB_Party)
                            Prefix = GChatPrefix[SAY2_Party][0];
                        else if (CurTab == CHATTAB_Market)
                            Prefix = GChatPrefix[SAY2_Market][0];
                        else if (CurTab == CHATTAB_Ally)
                            Prefix = GChatPrefix[SAY2_Ally][0];
                        if (Prefix)
                            EditBox->InsertText(Prefix, 0);
                    }
                    if (GIsEnterChatting)
                        EditBox->KillFocus();
                }
            }
            return 1;
        }
        BrowseTellNames(0);
        return 1;
    }
    BrowseTellNames(1);
    return 1;
}
NW_HOOK_METHOD(0x1004b4a0, NCChatWnd, OnEditCommand)

// 0x10045290 - the saved height restored (NConsoleWnd::LoadWindowPositions: the default
// height minus the saved one)
INT NCChatWnd::ResizeBy(INT Delta)
{
    return OnResize(Delta);
}
NW_HOOK_METHOD(0x10045290, NCChatWnd, ResizeBy)

// 0x100452a0 - the next tab, or the one before, around the five
void NCChatWnd::CycleTab(UBOOL bNext)
{
    if (bNext)
        TabCtrl->SetTab(CurTab < 4 ? CurTab + 1 : 0);
    else
        TabCtrl->SetTab(CurTab > 0 ? CurTab - 1 : 4);
}
NW_HOOK_METHOD(0x100452a0, NCChatWnd, CycleTab)


// 0x10048ae0 - the input line, every list and the tell partners emptied (desktop reset)
void NCChatWnd::ClearAll()
{
    EditBox->ClearText();
    Lists[CHATLIST_All]->Clear();
    Lists[CHATLIST_Pledge]->Clear();
    Lists[CHATLIST_Party]->Clear();
    Lists[CHATLIST_Market]->Clear();
    Lists[CHATLIST_Ally]->Clear();
    bTellBrowse = 0;
    for (INT i = 0; i < TellNames.Num(); i++)
    {
        NString* Name = TellNames(i);
        if (Name)
        {
            Name->~NString();
            appFree(Name);
        }
    }
    TellNames.Empty();
}
NW_HOOK_METHOD(0x10048ae0, NCChatWnd, ClearAll)

NW_CHECK_VTABLE(NCChatWnd, 0x10112a50, 76)
