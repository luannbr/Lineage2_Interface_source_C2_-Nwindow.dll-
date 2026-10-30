#pragma once
// NCRestartMenuWnd - the menu that opens when the character dies (UClass "CRestartMenuWnd",
// 0xC0): a button per place to restart at (nearest village, hideaway, castle, siege
// headquarters, fixed). The server says which of them apply (SetRestartPoints); the others
// hide when the menu shows. A click sends the choice (RequestRestartPoint) and closes it.

#include "nwindow/NCWnd.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCButton;

// Buttons[] and what the server gets for each (names ours)
enum
{
    RESTART_Village,
    RESTART_Hideaway,
    RESTART_Castle,
    RESTART_SiegeHQ,
    RESTART_Fixed,
    RESTART_MAX
};

// NWM_COMMAND ids of the buttons
enum
{
    NID_RESTART_VILLAGE = 0xF0F012,
    NID_RESTART_HIDEAWAY = 0xF0F013,
    NID_RESTART_CASTLE = 0xF0F014,
    NID_RESTART_SIEGEHQ = 0xF0F015,
    NID_RESTART_FIXED = 0xF0F020,
};

class NCRestartMenuWnd : public NCWnd
{
    NW_DECLARE_CLASS(NCRestartMenuWnd, NCWnd, 0, 0x101C2D90)

    UTexture* BackTex;                     // +0x98 "L2UI.EtcWnd.RestartBack"
    NCButton* Buttons[RESTART_MAX];        // +0x9C RESTART_*
    UBOOL bAvailable[RESTART_MAX];         // +0xB0 (1) from the server (SetRestartPoints)

    NCRestartMenuWnd();                                              // 0x10097120
    // ~NCRestartMenuWnd                                             // 0x100971b0

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x10096980
    virtual INT OnCreate();                                          // 36 0x10097200
    virtual INT OnDestroy();                                         // 37 0x10097630
    virtual void ShowWindow();                                       // 43 0x10097780
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x100975a0
    // names ours
    virtual INT OnVillage();                                         // 67 0x10096b20
    virtual INT OnHideaway();                                        // 68 0x10096ba0
    virtual INT OnCastle();                                          // 69 0x10096c20
    virtual INT OnSiegeHQ();                                         // 70 0x10096ca0
    virtual INT OnFixed();                                           // 71 0x10096d20

    // names ours
    void HideUnavailable();                                          // 0x10096a60
    void SetRestartPoints(UBOOL bVillage, UBOOL bHideaway, UBOOL bCastle, UBOOL bSiegeHQ,
                          UBOOL bFixed);                             // 0x10076930 (the console,
                                                                     //   when the player dies)
};
NW_ASSERT_OFFSET(NCRestartMenuWnd, BackTex, 0x98);
NW_ASSERT_OFFSET(NCRestartMenuWnd, Buttons, 0x9C);
NW_ASSERT_OFFSET(NCRestartMenuWnd, bAvailable, 0xB0);
NW_ASSERT_SIZE(NCRestartMenuWnd, 0xC4);

#pragma pack(pop)