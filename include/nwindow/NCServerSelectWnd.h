#pragma once
// NCServerSelectWnd - the server list of the login screen (UClass "CServerSelectWnd", 0xC8;
// NCLoginWnd +0xA0). The login server's list arrives a row at a time (AddServer: name, load,
// ping, type) and ends with OnServerListEnd; OK (or Enter, or a double click) logs in to the
// selected server, Cancel goes back to the login box, "Quality" orders the list: the server
// played last on top, the others by ping and, within a ping class, by users. Pings are asked
// of the network handler and kept in s_info.ini (section = server index, key "Time"), with
// the time they were taken ([Date] Time); the old ones are asked again.

#include "nwindow/NCWnd.h"
#include "nwindow/NCListCtrl.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCButton;
class L2ParamStack;

// NCServerSelectListCtrl - the list itself (UClass "CServerSelectListCtrl", 0x20C): sorting by
// a column also keeps the window's LastServerRow on its server
class NCServerSelectListCtrl : public NCListCtrl
{
    NW_DECLARE_CLASS(NCServerSelectListCtrl, NCListCtrl, 0, 0x101D8668)

    NCServerSelectListCtrl();                                        // 0x100c97f0
    NCServerSelectListCtrl(INT InMaxRows, INT InVisibleRows, INT InMaxColumns, INT Unused); // 0x100c9890
    // ~NCServerSelectListCtrl                                       // 0x100c9840

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x100c8080
    virtual INT SortByColumn(INT Column);                            // 73 0x100c9550
};
NW_ASSERT_SIZE(NCServerSelectListCtrl, 0x210);

class NCServerSelectWnd : public NCWnd
{
    NW_DECLARE_CLASS(NCServerSelectWnd, NCWnd, 0, 0x101D8140)

    UTexture* BackTex;               // +0x98 "L2UI.lobbywnd.serverback"
    NCServerSelectListCtrl* ServerList; // +0x9C
    NCButton* OKButton;              // +0xA0   // C2: offset derived, no evidence
    NCButton* CancelButton;          // +0xA4   // C2: offset derived, no evidence
    NCButton* QualityButton;         // +0xA8   // C2: offset derived, no evidence
    INT LastServerID;                // +0xAC server the account played on last (login server)
    INT LastServerRow;               // +0xB0 its row, -1 = none
    INT PendingPings;                // +0xB4 ping answers still awaited
    INT TimerResult;                 // +0xB8 what SetTimer returned (never read)   // C2: offset derived, no evidence
    UBOOL bRequested;                // +0xBC a server login went out (cleared by the timer)
    UBOOL bPingCacheOld;             // +0xC0 the saved pings are too old: ask every server
    UBOOL bPingSaved;                // +0xC4 a ping went to s_info.ini
    UBOOL UnknownC4;                 // +0xC8 set at the end of the list when bPingSaved (never
                                     //   read)

    NCServerSelectWnd();                                             // 0x100c86d0
    // ~NCServerSelectWnd                                            // 0x100c8740

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x100c80e0
    virtual INT OnTimer(INT TimerID);                                // 35 0x100c83f0
    virtual INT OnCreate();                                          // 36 0x100c98c0
    virtual INT OnDestroy();                                         // 37 0x100c8790 (folded)
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x100c88c0
    virtual INT OnKeyDown(WPARAM wParam, LPARAM lParam);             // 58 0x100c83c0
    // names ours
    virtual INT OnOK();                                              // 67 0x100c8f70
    virtual INT OnCancel();                                          // 68 0x100c90e0
    virtual INT SortByQuality();                                     // 69 0x100c9140
    virtual INT OnListNotify(INT Row);                               // 70 0x100c83b0

    // names ours
    void SetLastServerID(INT ServerID);                              // 0x100c81a0
    void OnServerListEnd();                                          // 0x100c8310
    void ClearServers();                                             // 0x100c8380
    void AddServer(L2ParamStack& Params);                            // 0x100c89a0
    // the row's number and name text (new in C2, a function of its own; name ours)
    void FormatServerLabel(INT Index, INT Flags, INT TypeIndex, TCHAR* Name, TCHAR* NameText); // 0x100c81b0
    void SetPing(INT IP, INT Ping);                                  // 0x100c8e90
    UBOOL IsPingCacheOld();                                          // 0x100c8590
    UBOOL HasNoPing(INT ServerIndex);                                // 0x100c85e0
    void SavePing(INT ServerIndex, INT Ping);                        // 0x100c8640
    void SavePingTime();                                             // 0x100c8690

    // calendar of the ping cache (names ours)
    static UBOOL IsLeapYear(INT Year);                               // 0x100c8410
    static INT DaysInYears(INT Years);                               // 0x100c8450
    static INT DaysInMonths(INT Year, INT Month);                    // 0x100c8490
    static INT HoursInDays(INT Day, INT Hour);                       // 0x100c8500
    static INT GetTimeStamp();                                       // 0x100c8530
};
NW_ASSERT_OFFSET(NCServerSelectWnd, ServerList, 0x9C);
NW_ASSERT_OFFSET(NCServerSelectWnd, LastServerID, 0xAC);
NW_ASSERT_OFFSET(NCServerSelectWnd, bRequested, 0xBC);
NW_ASSERT_OFFSET(NCServerSelectWnd, UnknownC4, 0xC8);
NW_ASSERT_SIZE(NCServerSelectWnd, 0xCC);

#pragma pack(pop)