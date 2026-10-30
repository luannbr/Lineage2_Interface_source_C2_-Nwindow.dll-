#pragma once
// NCChatWnd - the chat window (UClass "CChatWnd", 0x57C): five message lists, one per tab
// (all, market, party, pledge, ally), the input line, the tab row, the filter panel
// (NCChatFilterWnd) and a resize grip on top. Incoming messages go through AddChatMessage,
// which hands each line to every tab whose filter shows its type; the input line reports to
// OnEditCommand (Enter runs the line: chat or a /command; '"' and Up/Down recall the names
// of recent tells). Optionally the chat is logged to a file ("text capture", GL2TextCapture).
//
// Desktop (NCConsole) +0x98 is the chat window.

#include "nwindow/NCWnd.h"
#include "nwindow/NCChatListBox.h"
#include "nwindow/NCommandParser.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCEditBox;
class NCTabControl;
class NCButton;
class NCChatFilterWnd;
class NCResizeFrame;

// Lists[] (names ours). The tabs are in another order: see TabList.
enum
{
    CHATLIST_All,
    CHATLIST_Party,
    CHATLIST_Pledge,
    CHATLIST_Market,
    CHATLIST_Ally,
    CHATLIST_MAX
};

// CurTab: the tab row, left to right (names ours)
enum
{
    CHATTAB_All,
    CHATTAB_Market,
    CHATTAB_Party,
    CHATTAB_Pledge,
    CHATTAB_Ally,
};

// NWM_COMMAND ids of the window's controls
enum
{
    NID_CHAT_EDIT = 0x1501,
    NID_CHAT_LIST = 0x1A01,
    NID_CHAT_TABS = (INT)0xF0F01409,
    NID_CHAT_OPTION = (INT)0xF0F01410,
    NID_CHAT_RESIZE = 0xF007,
};

// Prefix of each Say2 type in the input line, ASCII and full width (name ours; C2 table of pointer pairs at
// 0x10166020, 10 pairs (right after the auto-login messages), the C1 one was at 0x1010cee0): 1 '!',
// 2 '"', 3 '#', 4 '@', 8 '+', 9 '$'...
#if NW_HOOK_MODE
#define GChatPrefix (reinterpret_cast<const TCHAR* const (*)[2]>(::nw::Addr(0x10166020)))
#else
extern const TCHAR* const GChatPrefix[][2];
#endif

// An element of the limiter's list (name ours): a text and the points it counts (the stock code frees the NString)
struct NChatLimiterItem
{
    NString* Text;
    INT Points;
};
NW_ASSERT_SIZE(NChatLimiterItem, 8);

// The chat flood control new in C2 (name ours; 0x34 bytes at NCChatWnd+0x180). What it measures: sent lines add
// points, which fade by Decay every second (NCChatWnd::OnTimer, id 0xF017 every 1000 ms); while the total or one
// line is over Threshold the chat is blocked (bLimited, set by the command / chat functions); the tick that brings
// everything back under tells the player (system message 965, the colour of the other warnings). The limits other than
// Threshold and Decay are not used by the functions decompiled so far.
struct NChatLimiter
{
    INT Points;                // +0x00 the running total
    TArray<NChatLimiterItem> Items;   // +0x04
    INT Limit10;               // +0x10 20000
    INT Threshold;             // +0x14 10000
    INT Limit18;               // +0x18 100000
    INT Decay;                 // +0x1C 200 points per second
    INT Limit20;               // +0x20 60
    INT Limit24;               // +0x24 100
    INT Limit28;               // +0x28 2000
    FLOAT Factor2C;            // +0x2C 0.1
    BYTE bLimited;             // +0x30 (a byte in the stock code)
    BYTE _Pad31[3];

    NChatLimiter();            // 0x10048ca0
    void Clear();              // 0x10047990 the total and the flag to 0, every text freed, the list emptied
    void Tick();               // 0x100453c0 one second passes
    void AddLine(INT Length, const TCHAR* Text);   // 0x100479f0 a sent line of Length characters counts its points (the same text again: more)
    void SetLimits(INT InLimit10, INT InThreshold, INT InLimit18, INT InDecay, INT InLimit20, INT InLimit24,
                   INT InLimit28, FLOAT InFactor2C);   // 0x10048c30 ("///setcl": clears the control, then sets the limits)
    // ~NChatLimiter                                    // 0x10048c90 (the lines' array destructor, 0x10048ba0, with the size of the object's +4)
};
NW_ASSERT_SIZE(NChatLimiter, 0x34);

class NCChatWnd : public NCWnd
{
    NW_DECLARE_CLASS(NCChatWnd, NCWnd, 0, 0x101B0800)

    UTexture* HeadTex;               // +0x98 "L2UI.ChatWnd.Head"
    UTexture* BodyTex;               // +0x9C "L2UI.ChatWnd.Body", repeated down the window
    UTexture* Unknown9C;             // +0xA0 cleared, never loaded
    UTexture* BottomTex;             // +0xA4 "L2UI.ChatWnd.Bottom" (not drawn)
    UTexture* Bottom1Tex;            // +0xA8 "L2UI.ChatWnd.Bottom1", under the input line
    UTexture* Bottom2Tex;            // +0xAC "L2UI.ChatWnd.Bottom2" (not drawn)
    UTexture* IMEJpTex;              // +0xB0 IME state icons: "L2UI.ChatWnd.IME_jp"
    UTexture* IMEEnTex;              // +0xB4 "L2UI.ChatWnd.IME_en"
    UTexture* IMEKrTex;              // +0xB8 "L2UI.ChatWnd.IME_kr"
    UTexture* IMETw1Tex;             // +0xBC "L2UI.ChatWnd.IME_tw1"
    UTexture* IMETw2Tex;             // +0xC0 "L2UI.ChatWnd.IME_tw2"
    UTexture* IMECn1Tex;             // +0xC4 new in C2: "L2UI.ChatWnd.IME_cn1" .. cn4 (Chinese IME), IME_th (Thai)
    UTexture* IMECn2Tex;             // +0xC8
    UTexture* IMECn3Tex;             // +0xCC
    UTexture* IMECn4Tex;             // +0xD0
    UTexture* IMEThTex;              // +0xD4
    UTexture* IMETw3Tex;             // +0xD8 "L2UI.ChatWnd.IME_tw3"
    NCEditBox* EditBox;              // +0xDC the input line
    TCHAR LastTellName[32];          // +0xE0 "name " (SetLastTellName), never read here
    TArray<NString*> TellNames;      // +0x120 recent tell partners, oldest first (64 at most)
    INT TellIndex;                   // +0x12C the one Up/Down shows (-1 before any)
    UBOOL bTellBrowse;               // +0x130 '"' was typed: Up/Down walk TellNames
    NCChatListBox* Lists[CHATLIST_MAX]; // +0x134 CHATLIST_*
    NCTabControl* TabCtrl;           // +0x148
    NCButton* OptionButton;          // +0x14C opens / closes FilterWnd
    NCWnd* Unknown138;               // +0x150 destroyed by OnDestroy, never created   // C2: offset derived, no evidence
    NCChatFilterWnd* FilterWnd;      // +0x154
    UBOOL bFilterHidden;             // +0x158
    NCResizeFrame* ResizeFrame;      // +0x15C grip on the top edge
    INT ResizeDelta;                 // +0x160 drag not applied yet (OnResize, 12 px steps)
    INT Unknown14C;                  // +0x164 (0)   // C2: offset derived, no evidence
    INT Unknown150;                  // +0x168 (2)   // C2: offset derived, no evidence
    INT CreateHeight;                // +0x16C Height when created
    INT Unknown158;                  // +0x170 (0)   // C2: offset derived, no evidence
    INT CurTab;                      // +0x174 CHATTAB_*
    FArchive* CaptureFile;           // +0x178 text capture log
    INT TimerHandle;                 // +0x17C what SetTimer(0xF017, 1000) returned (OnCreate; new in C2)
    NChatLimiter Limiter;            // +0x180 new in C2 (the flood control)
    NCommandParser Parser;           // +0x1B4 (0x418 bytes, up to sizeof)

    NCChatWnd();                                                     // 0x10048dc0
    // ~NCChatWnd                                                    // 0x1004b6d0

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x10044380
    virtual void SetFocus();                                         // 29 0x10045220
    virtual INT OnTimer(INT TimerID);                                // 35 0x10045dc0 (C2: the flood control's second)
    virtual INT OnCreate();                                          // 36 0x10048ec0
    virtual INT OnDestroy();                                         // 37 0x10047dd0
    virtual void ShowWindow();                                       // 43 0x10045240
    virtual void HideWindow();                                       // 44 0x1007b470 (shared)
    virtual void MoveWindow(INT DX, INT DY);                         // 45 0x10045d90 (shared)
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x100458a0
    // names ours
    virtual void StartTextCapture();                                 // 67 0x10044470
    virtual void StopTextCapture();                                  // 68 0x100445f0
    virtual void WriteCapture(const TCHAR* Text);                    // 69 0x100446c0
    virtual void CaptureMessage(const TCHAR* Text, INT Type, INT SubType); // 70 0x10044720
    virtual void UpdateTextCapture();                                // 71 0x10044440
    virtual INT OnTabSelected(INT Tab);                              // 72 0x10048250
    virtual INT OnEditCommand(INT Code);                             // 73 0x1004b4a0
    virtual INT OnOptionButton();                                    // 74 0x10048a20
    virtual INT OnResize(INT Delta);                                 // 75 0x10045b30

    // names ours
    void AddChatMessage(const TCHAR* Text, DWORD Color, INT Type, INT SubType); // 0x100447b0
    void ProcessChatLine(TCHAR* Line);                               // 0x100497d0 (name ours; C1's OnEditCommand body: the /command
                                                                     //   and chat sending, 6986 bytes in C2; not ported)
    void ExpandLine(TCHAR* Out, TCHAR* In);                          // 0x100450b0 (name ours) a chat line with the words ExpandToken
                                                                     //   knows replaced (Out gets the original when none was)
    void SetLastTellName(const TCHAR* Name);                         // 0x10045050
    void BeginTell(const TCHAR* Name);                               // 0x100456e0
    void BrowseTellNames(UBOOL bUp);                                 // 0x10045780
    void AddTellName(const TCHAR* Name);                             // 0x10048060
    // the party, pledge and alliance tabs enabled or greyed (NConsoleWnd::EnablePartyChat...,
    // as the server says); the party's does nothing
    void EnablePartyTab(bool bEnable) {}                             // 0x10001ad0 (folded)
    void EnablePledgeTab(bool bEnable);                              // 0x1004b5a0
    void EnableAllyTab(bool bEnable);                                // 0x1004b610
    // used by the console (names ours)
    INT ResizeBy(INT Delta);                                         // 0x10045290 OnResize out of
                                                                     //   line (WindowsInfo.ini)
    void CycleTab(UBOOL bNext);                                      // 0x100452a0 Alt + PageUp /
                                                                     //   PageDown
    void ClearAll();                                                 // 0x10048ae0 (name ours)

    // list of a tab (inlined in the stock code, name ours)
    NCChatListBox* TabList(INT Tab)
    {
        switch (Tab)
        {
        case CHATTAB_All:    return Lists[CHATLIST_All];
        case CHATTAB_Market: return Lists[CHATLIST_Market];
        case CHATTAB_Party:  return Lists[CHATLIST_Party];
        case CHATTAB_Pledge: return Lists[CHATLIST_Pledge];
        case CHATTAB_Ally:   return Lists[CHATLIST_Ally];
        }
        return NULL;
    }
};
NW_ASSERT_OFFSET(NCChatWnd, EditBox, 0xDC);
NW_ASSERT_OFFSET(NCChatWnd, LastTellName, 0xE0);
NW_ASSERT_OFFSET(NCChatWnd, TellNames, 0x120);
NW_ASSERT_OFFSET(NCChatWnd, Lists, 0x134);
NW_ASSERT_OFFSET(NCChatWnd, FilterWnd, 0x154);
NW_ASSERT_OFFSET(NCChatWnd, ResizeDelta, 0x160);
NW_ASSERT_OFFSET(NCChatWnd, CurTab, 0x174);
NW_ASSERT_OFFSET(NCChatWnd, TimerHandle, 0x17C);
NW_ASSERT_OFFSET(NCChatWnd, Limiter, 0x180);
NW_ASSERT_OFFSET(NCChatWnd, Parser, 0x1B4);
NW_ASSERT_SIZE(NCChatWnd, 0x5CC);

#pragma pack(pop)