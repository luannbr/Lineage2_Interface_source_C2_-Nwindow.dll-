#pragma once
// NCPledgeWnd - the clan page of the action window (UClass "CPledgeWnd", 0x3B0): crest, name,
// leader, level, castle / hideout, the member list (name, level, on / out; NCListCtrl) and
// nine buttons: invite, title, crest, war, truce, surrender, expel, yield, leave. Only the
// clan leader gets the first seven; only the others get Leave. The console fills it
// (NConsoleWnd::ReceivePledgeInfo, AddPledgeMember...). NCActionWnd +0x9C is this window.

#include "nwindow/NCWnd.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCListCtrl;
class NCButton;

// What the console hands over about the clan (name ours; 0x62C bytes in C2, 0xEC in C1: the three
// texts are 256 characters now; built on the stack by NConsoleWnd::ReceivePledgeInfo). The window
// keeps a copy (Info) but SetPledgeInfo copies only part of it.
// C2 layout: from the stock SetPledgeInfo / UpdatePledgeInfo (the copies field by field) and the
// window's constructor and ClearInfo (the stores into the copy); C1 offsets in brackets.
struct NPledgeInfo
{
    TCHAR Name[256];           // +0x000 (C1: 32)
    INT PledgeID;              // +0x200 the crest texture is looked up by it (C1 +0x40)
    INT CrestID;               // +0x204 > 0: the clan has a crest (C1 +0x44)
    TCHAR LeaderName[256];     // +0x208 (C1 +0x48, 32 characters)
    INT Level;                 // +0x408 (C1 +0x88)
    UBOOL bHasCastle;          // +0x40C (C1 +0x8C)
    UBOOL bHasHideout;         // +0x410 (C1 +0x90)
    INT Unknown94;             // +0x414 (C1 +0x94)
    INT Unknown98;             // +0x418 (C1 +0x98)
    TCHAR StatusText[256];     // +0x41C "In War", "In Siege", "Dispersing"... (not copied) (C1 +0x9C, 32)
    INT Status;                // +0x61C index the console picked StatusText by (C1 +0xDC)
    INT UnknownE0;             // +0x620 (not copied)
    INT UnknownE4;             // +0x624 (not copied)
    INT UnknownE8;             // +0x628 (C1 +0xE8)

    // inlined in the console's slots 75, 76 and 203 (UnknownE8 is left)
    NPledgeInfo()
    {
        Name[0] = 0;
        LeaderName[0] = 0;
        StatusText[0] = 0;
        PledgeID = 0;
        CrestID = 0;
        UnknownE4 = 0;
        Level = 0;
        bHasCastle = 0;
        bHasHideout = 0;
        Unknown94 = 0;
        Unknown98 = 0;
        Status = 0;
        UnknownE0 = 0;
    }
};
NW_ASSERT_OFFSET(NPledgeInfo, PledgeID, 0x200);
NW_ASSERT_OFFSET(NPledgeInfo, LeaderName, 0x208);
NW_ASSERT_OFFSET(NPledgeInfo, Level, 0x408);
NW_ASSERT_OFFSET(NPledgeInfo, StatusText, 0x41C);
NW_ASSERT_OFFSET(NPledgeInfo, Status, 0x61C);
NW_ASSERT_SIZE(NPledgeInfo, 0x62C);

// A member as the console hands it over (name ours; 0x214 bytes, built on the stack by
// NConsoleWnd::AddPledgeMember)
struct NPledgeMember
{
    TCHAR Name[256];           // +0x000
    INT Level;                 // +0x200
    INT ClassID;               // +0x204 (-1)
    INT Unknown208;            // +0x208 (-1)
    INT Unknown20C;            // +0x20C (-1)
    UBOOL bOnline;             // +0x210 C2: the member's id while it is online, 0 when not (the row keeps it as its first
                               //   param: NCPledgeWnd::OnNewButton2 sends it as the member of the privileges request)

    // inlined in the console's slots 78, 79 and 205
    NPledgeMember()
    {
        Name[0] = 0;
        Level = 0;
        ClassID = -1;
        Unknown208 = -1;
        Unknown20C = -1;
        bOnline = 0;
    }
};
NW_ASSERT_OFFSET(NPledgeMember, Level, 0x200);
NW_ASSERT_OFFSET(NPledgeMember, bOnline, 0x210);
NW_ASSERT_SIZE(NPledgeMember, 0x214);

// NWM_COMMAND ids of the buttons (names ours)
enum
{
    NID_PLEDGE_CREST = 0xF00C,
    NID_PLEDGE_WAR = 0xF00D,
    NID_PLEDGE_TRUCE = 0xF00E,
    NID_PLEDGE_SURRENDER = 0xF010,
    NID_PLEDGE_INVITE = 0xF011,
    NID_PLEDGE_YIELD = 0xF012,
    NID_PLEDGE_TITLE = 0xF013,
    NID_PLEDGE_EXPEL = 0xF014,
    NID_PLEDGE_LEAVE = 0xF015,
    NID_PLEDGE_NEW1 = 0xF019,         // new in C2 (text 666)
    NID_PLEDGE_NEW2 = 0xF018,         // new in C2 (text 667)
};

class NCPledgeWnd : public NCWnd
{
    NW_DECLARE_CLASS(NCPledgeWnd, NCWnd, 0, 0x101AE820)

    UTexture* BackTex;               // +0x98 "L2UI.ActionWnd.PledgeBack"
    UTexture* CrestTex;              // +0x9C (NULL) NCGMPledgeWnd: the crest found by each paint
    NPledgeInfo Info;                // +0xA0 (0x62C bytes in C2; C1 +0x9C, 0xEC bytes)
    NCListCtrl* MemberList;          // +0x6CC
    NCButton* InviteButton;          // +0x6D0
    NCButton* TitleButton;           // +0x6D4
    NCButton* CrestButton;           // +0x6D8
    NCButton* WarButton;             // +0x6DC
    NCButton* TruceButton;           // +0x6E0
    NCButton* SurrenderButton;       // +0x6E4
    NCButton* ExpelButton;           // +0x6E8
    NCButton* YieldButton;           // +0x6EC always enabled
    NCButton* LeaveButton;           // +0x6F0
    NCButton* NewButton6F4;          // +0x6F4 new in C2: id 0xF019, text 666 (OnNewButton1)
    NCButton* NewButton6F8;          // +0x6F8 new in C2: id 0xF018, text 667 (OnNewButton2)
    BYTE _Unknown6FC[0x8FC - 0x6FC]; // +0x6FC not used by the stock code of this class

    NCPledgeWnd();                                                   // 0x10040f70
    // ~NCPledgeWnd                                                  // 0x10041020

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x10040260
    virtual INT OnCreate();                                          // 36 0x10041070
    virtual INT OnDestroy();                                         // 37 0x10041d90
    virtual void ShowWindow();                                       // 43 0x10040420
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x100419e0
    // the buttons (names ours): dialogs NCDialogBox answers by ID, or messages
    virtual INT OnInvite();                                          // 67 0x10040750
    virtual INT OnSetCrest();                                        // 68 0x10042430
    virtual INT OnDeclareWar();                                      // 69 0x100424d0
    virtual INT OnEndWar();                                          // 70 0x10042570
    virtual INT OnSurrender();                                       // 71 0x10042610
    virtual INT OnExpel();                                           // 72 0x100426b0
    virtual INT OnSurrenderPersonally();                             // 73 0x100427a0
    virtual INT OnWithdraw();                                        // 74 0x10042840
    virtual INT OnGiveTitle();                                       // 75 0x10042360
    virtual INT OnNewButton1();                                      // 76 0x10042920 (C2 only; name ours) the button 0xF019 (NewButton6F4): the player's own
                                                                     //   privileges (the empower window, view mode)
    virtual INT OnNewButton2();                                      // 77 0x100429c0 (C2 only; name ours) the button 0xF018 (NewButton6F8): the selected
                                                                     //   member's privileges (the empower window, edit mode; the leader's)
    virtual INT OnDialogResult(WPARAM wParam, LPARAM lParam) { return 1; } // 78 0x100d2e70 (folded)

    // the console's calls (names ours)
    void SetPledgeInfo(NPledgeInfo* InInfo);                         // 0x10040590
    void UpdatePledgeInfo(NPledgeInfo* InInfo);                      // 0x10040650
    const TCHAR* GetPledgeName();                                    // 0x100406c0 NULL: no clan
    void ClearMembers();                                             // 0x100406e0
    void ClearInfo();                                                // 0x100406f0
    void Reset();                                                    // 0x10042020
    void AddMember(NPledgeMember* Member);                           // 0x10042030
    void UpdateMember(NPledgeMember* Member);                        // 0x10042240
    void DeleteMember(const TCHAR* Name);                            // 0x10043340
};
NW_ASSERT_OFFSET(NCPledgeWnd, Info, 0xA0);
NW_ASSERT_OFFSET(NCPledgeWnd, MemberList, 0x6CC);
NW_ASSERT_OFFSET(NCPledgeWnd, LeaveButton, 0x6F0);
NW_ASSERT_OFFSET(NCPledgeWnd, NewButton6F8, 0x6F8);
NW_ASSERT_SIZE(NCPledgeWnd, 0x8FC);

#pragma pack(pop)