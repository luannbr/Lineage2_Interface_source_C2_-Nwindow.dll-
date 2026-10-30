#pragma once
// NCSiegeInfoMainWnd - the castle siege window (UClass "CSiegeInfoMainWnd", 0x3C0; desktop
// NCConsole +0x110): the castle's name and its owner (lord, clan and alliance, with their
// crests) over three tabs: the siege date, the attacking clans and the defending clans. The
// server's siege info (NConsoleWnd slot 190) opens it (SetCastleInfo); the tab of a side asks
// for its list once, and the console fills it (AddAttacker / AddDefender, slots 191 / 192)
// and then updates its buttons (slots 194 / 195).
//
// NCSiegePartInfoWnd - a side's tab (UClass "CSiegePartInfoWnd", 0xD0): the clans in an
// NCSiegeInfoListCtrl, their count, and the buttons: apply / cancel the application for
// everybody but the owner; on the defending side the owner approves or refuses the clans
// waiting. Each button asks first (NCDialogBox ids 0x3A..0x3F answer the server).
//
// NCSiegeDateInfoWnd - the date tab (UClass "CSiegeDateInfoWnd", 0xB8): the current time
// and the siege's; while no date is set, the owner picks one in a combo box among those the
// server offers (slot 193 adds them), after a confirmation.
//
// NCSiegeInfoListCtrl - the clan list of a side (UClass "CSiegeInfoListCtrl", 0x20C): the
// clan's crest and name, then the alliance name after its crest (attackers) or the status
// (defenders); the tool tip of a row has the leaders, the alliance and the date of the
// application.
//
// Row data (NCSiegePartInfoWnd::AddClan): Cells[0] clan name, Text2 its leader, Param1 the
// application time, Param2 the side (1 = defenders); Cells[1] alliance name or status, Text2
// the alliance's leader; row Text the alliance name, Param1 the clan's pledge id, Param2
// NSiegeClanInfo::CrestID.

#include "nwindow/NCListCtrl.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCButton;
class NCComboBox;
class NCTabControl;

// The server's siege info of a castle (name ours; 0x618 bytes in C2, 0x318 in C1: the texts are 256
// characters; NConsoleWnd slot 190 builds it on its stack). C2 layout: from the stock operator= and the
// window's reads.
struct NSiegeCastleInfo
{
    TCHAR LordName[256];       // +0x000 the owner clan's leader (C1: 128)
    TCHAR ClanName[256];       // +0x200 the owner clan (C1 +0x100)
    TCHAR AllianceName[256];   // +0x400 its alliance (C1 +0x200)
    INT CastleID;              // +0x600 (C1 +0x300)
    UBOOL bIsOwner;            // +0x604 the player's clan owns the castle
    INT PledgeID;              // +0x608 the owner clan (the crests are drawn from it)
    INT AllianceID;            // +0x60C (not used)
    INT CurrentTime;           // +0x610 server time
    INT SiegeTime;             // +0x614 0 while not set

    // inlined (the constructor in the console's slot 190 and the window's, Clear in Reset)
    NSiegeCastleInfo() { Clear(); }
    void Clear()
    {
        CastleID = 0;
        bIsOwner = 0;
        PledgeID = 0;
        AllianceID = 0;
        CurrentTime = 0;
        SiegeTime = 0;
        LordName[0] = 0;
        ClanName[0] = 0;
        AllianceName[0] = 0;
    }
    NSiegeCastleInfo& operator=(const NSiegeCastleInfo& Other);      // 0x100ab750
};
NW_ASSERT_OFFSET(NSiegeCastleInfo, CastleID, 0x600);
NW_ASSERT_OFFSET(NSiegeCastleInfo, SiegeTime, 0x614);
NW_ASSERT_SIZE(NSiegeCastleInfo, 0x618);

// NSiegeClanInfo::Type on the defending side (names ours)
enum
{
    SIEGECLAN_Owner = 1,
    SIEGECLAN_Waiting = 2,
    SIEGECLAN_Approved = 3,
    SIEGECLAN_Refused = 4,
};

// A clan of a side's list (name ours; 0x814 bytes in C2, 0x414 in C1: the texts are 256 characters;
// NConsoleWnd slots 191 / 192 build it on their stack; the tab keeps a copy of each). C2 layout: from the
// stock copy constructor (0x100D0320), C1 offsets in brackets.
struct NSiegeClanInfo
{
    INT PledgeID;              // +0x000
    TCHAR ClanName[256];       // +0x004
    TCHAR LeaderName[256];     // +0x204 (C1 +0x104)
    INT CrestID;               // +0x404 (the row's Param2, not read) (C1 +0x204)
    INT ApplyTime;             // +0x408 when it applied, 0 = not known
    INT Type;                  // +0x40C defenders: SIEGECLAN_*
    INT AllianceID;            // +0x410 (not used)
    TCHAR AllianceName[256];   // +0x414 (C1 +0x214)
    TCHAR AllianceLeader[256]; // +0x614 (C1 +0x314)

    // inlined in the console's slots 191 / 192
    NSiegeClanInfo()
    {
        PledgeID = 0;
        ClanName[0] = 0;
        LeaderName[0] = 0;
        CrestID = 0;
        ApplyTime = 0;
        Type = 0;
        AllianceID = 0;
        AllianceName[0] = 0;
        AllianceLeader[0] = 0;
    }
    NSiegeClanInfo(const NSiegeClanInfo& Other);                     // 0x100ab7e0
};
NW_ASSERT_OFFSET(NSiegeClanInfo, LeaderName, 0x204);
NW_ASSERT_OFFSET(NSiegeClanInfo, Type, 0x40C);
NW_ASSERT_OFFSET(NSiegeClanInfo, AllianceName, 0x414);
NW_ASSERT_OFFSET(NSiegeClanInfo, AllianceLeader, 0x614);
NW_ASSERT_SIZE(NSiegeClanInfo, 0x814);

// NCSiegePartInfoWnd::Side (names ours)
enum
{
    SIEGESIDE_Attacker = 0,
    SIEGESIDE_Defender = 1,
};

class NCSiegeInfoListCtrl : public NCListCtrl
{
    NW_DECLARE_CLASS(NCSiegeInfoListCtrl, NCListCtrl, 0, 0x101DA548)

    NCSiegeInfoListCtrl();                                           // 0x100d23f0
    NCSiegeInfoListCtrl(INT InMaxRows, INT InVisibleRows, INT InMaxColumns, INT Unused); // 0x100d2490
    // ~NCSiegeInfoListCtrl                                          // 0x100d2440

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x100d00e0
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x100d0480
    void DrawRowToolTip(NCGDevice* Device, NListRow* Row, INT X, INT Y); // C2: slot gone (was 69)

    // names ours
    // (C1's BuildToolTip is gone in C2: AddClan no longer builds a tool tip text)
    INT GetSelectedPledgeID();                                       // 0x100d03b0 0: none
    INT GetSelectedStatus();                                         // 0x100d03e0 SIEGECLAN_Approved
                                                                     //   / _Waiting by the status
                                                                     //   text, -1 otherwise
};
NW_ASSERT_SIZE(NCSiegeInfoListCtrl, 0x210);

class NCSiegePartInfoWnd : public NCWnd
{
    NW_DECLARE_CLASS(NCSiegePartInfoWnd, NCWnd, 0, 0x101DA020)

    UTexture* BackTex;               // +0x98 "L2UI.SiegeWnd.siege_back2"
    NCSiegeInfoListCtrl* List;       // +0x9C
    INT Side;                        // +0xA0 SIEGESIDE_* (SetSide)
    NCButton* ApplyButton;           // +0xA4
    NCButton* CancelApplyButton;     // +0xA8
    NCButton* ApproveButton;         // +0xAC
    NCButton* RefuseButton;          // +0xB0
    BYTE bRegistered;                // +0xB4 the player's clan is in the list
    INT NumApproved;                 // +0xB8 defenders
    INT NumWaiting;                  // +0xBC defenders (the owner counts here too)
    TArray<NSiegeClanInfo*> Clans;   // +0xC0 copies of what AddClan got (never read back)
    INT CastleID;                    // +0xCC
    BYTE bListed;                    // +0xD0 the list was asked for: the tab does not ask again

    NCSiegePartInfoWnd();                                            // 0x100d24c0
    // ~NCSiegePartInfoWnd                                           // 0x100d2570

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x100d0100
    virtual INT OnCreate();                                          // 36 0x100d25d0
    virtual INT OnDestroy();                                         // 37 0x100d1c20
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x100d07c0
    // names ours
    virtual INT OnApply();                                           // 67 0x100d0a80
    virtual INT OnCancelApply();                                     // 68 0x100d0b60
    virtual INT OnApprove();                                         // 69 0x100d0c40
    virtual INT OnRefuse();                                          // 70 0x100d0d60

    // names ours
    void SetSide(INT InSide);                                        // 0x100d0920 and the columns
    void UpdateButtons();                                            // 0x100d0e90
    void Reset();                                                    // 0x100d1dc0
    void ClearList();                                                // 0x100d1e40
    void AddClan(NSiegeClanInfo* Info);                              // 0x100d1e80
};
NW_ASSERT_OFFSET(NCSiegePartInfoWnd, List, 0x9C);
NW_ASSERT_OFFSET(NCSiegePartInfoWnd, bRegistered, 0xB4);
NW_ASSERT_OFFSET(NCSiegePartInfoWnd, Clans, 0xC0);
NW_ASSERT_OFFSET(NCSiegePartInfoWnd, bListed, 0xD0);
NW_ASSERT_SIZE(NCSiegePartInfoWnd, 0xD4);

class NCSiegeDateInfoWnd : public NCWnd
{
    NW_DECLARE_CLASS(NCSiegeDateInfoWnd, NCWnd, 0, 0x101DAA70)

    UTexture* BackTex;               // +0x98 "L2UI.SiegeWnd.siege_back3"
    TArray<INT> Times;               // +0x9C the dates offered, the combo's items after the first
    NCComboBox* TimeCombo;           // +0xA8
    INT CastleID;                    // +0xAC
    INT SelectedIndex;               // +0xB0 combo item chosen, 0 = automatic setting
    INT CurrentTime;                 // +0xB4
    INT SiegeTime;                   // +0xB8 0 while not set

    NCSiegeDateInfoWnd();                                            // 0x100d20c0
    // ~NCSiegeDateInfoWnd                                           // 0x100d2160

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x100d01b0
    virtual INT OnCreate();                                          // 36 0x100d0fd0
    virtual INT OnDestroy();                                         // 37 0x100d1080
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x100d1130
    // names ours
    virtual INT OnSelectTime(INT Index);                             // 67 0x100d1370
    virtual INT OnConfirmTime(WPARAM wParam, LPARAM lParam);         // 68 0x100d1440

    // names ours
    void Reset();                                                    // 0x100d1300
    void SetTimes(INT InCurrentTime, INT InSiegeTime);               // 0x100d14e0
    void AddTime(INT Time);                                          // 0x100d21c0
};
NW_ASSERT_OFFSET(NCSiegeDateInfoWnd, Times, 0x9C);
NW_ASSERT_OFFSET(NCSiegeDateInfoWnd, TimeCombo, 0xA8);
NW_ASSERT_SIZE(NCSiegeDateInfoWnd, 0xBC);

class NCSiegeInfoMainWnd : public NCWnd
{
    NW_DECLARE_CLASS(NCSiegeInfoMainWnd, NCWnd, 0, 0x101D9B00)

    UTexture* BackTex;               // +0x98 "L2UI.SiegeWnd.Siege_Back1"
    NSiegeCastleInfo Castle;         // +0x9C (0x618 bytes in C2; C1 +0x98)
    NCTabControl* TabCtrl;           // +0x6B4
    NCSiegePartInfoWnd* AttackerInfo; // +0x6B8
    NCSiegePartInfoWnd* DefenderInfo; // +0x6BC
    NCSiegeDateInfoWnd* DateInfo;    // +0x6C0

    NCSiegeInfoMainWnd();                                            // 0x100d1540
    // ~NCSiegeInfoMainWnd                                           // 0x100d1600

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x100d0230
    virtual INT OnCreate();                                          // 36 0x100d2960
    virtual INT OnDestroy();                                         // 37 0x100d1650
    virtual void ShowWindow();                                       // 43 0x100d2320
    virtual void HideWindow();                                       // 44 0x100d2310
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x100d1780
    virtual INT OnTab(INT Tab);                                      // 67 0x100d1a50 (name ours)

    // names ours (the console calls the last three)
    void Reset();                                                    // 0x100d2220
    void SetCastleInfo(NSiegeCastleInfo* Info);                      // 0x100d2280
    void AddAttacker(NSiegeClanInfo* Info);                          // 0x100d22f0
    void AddDefender(NSiegeClanInfo* Info);                          // 0x100d2300
};
NW_ASSERT_OFFSET(NCSiegeInfoMainWnd, Castle, 0x9C);
NW_ASSERT_OFFSET(NCSiegeInfoMainWnd, TabCtrl, 0x6B4);
NW_ASSERT_OFFSET(NCSiegeInfoMainWnd, DateInfo, 0x6C0);
NW_ASSERT_SIZE(NCSiegeInfoMainWnd, 0x6C4);

#pragma pack(pop)