#pragma once
// NCSystemMenuWnd - the system menu (UClass "CSystemMenuWnd", 0xB8; Alt+X): Help, Options,
// Restart and Exit Game, and two more buttons in C2 (one column of six). Restart and Exit ask first (dialogs 0x1B / 0x1C); the dialog box
// answers with NWM_DIALOG_RESULT and the dialog id, and the menu sends the request.

#include "nwindow/NCWnd.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCButton;

// Buttons[] (names ours); the fifth slot is never created
enum
{
    SYSMENU_Help,
    SYSMENU_Options,
    SYSMENU_Restart,
    SYSMENU_Exit,
    SYSMENU_Unused,
    SYSMENU_MAX
};

// NWM_COMMAND ids of the buttons and title bar, and the dialogs the menu opens
enum
{
    NID_SYSMENU_HELP = 0xF0F006,
    NID_SYSMENU_OPTIONS = 0xF0F007,
    NID_SYSMENU_RESTART = 0xF0F008,
    NID_SYSMENU_EXIT = 0xF0F009,
    NID_SYSMENU_FRAME = 0xF0F011,
    NID_SYSMENU_BUTTON_A = 0xF0F022,       // (new in C2, names ours) text sysstring 711, above Help
    NID_SYSMENU_BUTTON_B = 0xF0F023,       // (new in C2) text id 470, under Help
    DIALOG_SYSMENU_RESTART = 0x1B,
    DIALOG_SYSMENU_EXIT = 0x1C,
};

class NCSystemMenuWnd : public NCWnd
{
    NW_DECLARE_CLASS(NCSystemMenuWnd, NCWnd, 0, 0x101C37D0)

    UTexture* BackTex;                     // +0x98 "L2UI.etcWnd.systemBack"
    NCButton* Buttons[SYSMENU_MAX];        // +0x9C SYSMENU_*
    NCButton* ButtonA;                     // +0xB0 (new in C2, name ours) NID_SYSMENU_BUTTON_A, OnHelpWindow
    NCButton* ButtonB;                     // +0xB4 (new in C2, name ours) NID_SYSMENU_BUTTON_B, OnHelpPage

    NCSystemMenuWnd();                                               // 0x10097790
    // ~NCSystemMenuWnd                                              // 0x10097800

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x10096da0
    virtual INT OnCreate();                                          // 36 0x10097850
    virtual INT OnDestroy();                                         // 37 0x10097dd0 (folded)
    virtual void ShowWindow();                                       // 43 0x1008c9c0 (folded)
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x10097d40
    // names ours
    virtual INT OnHelp();                                            // 67 0x10096ec0 (C2: not the C1 body, see below)
    virtual INT OnOptions();                                         // 68 0x10082c50 (folded)
    virtual INT OnRestart();                                         // 69 0x10097f80
    virtual INT OnHelpWindow();                                      // 70 0x10096ee0 (the C1 OnHelp: NCConsole::ToggleHelp)
    virtual INT OnExit();                                            // 71 0x10098020
    virtual INT OnHelpPage();                                        // 72 0x10096fb0 (new in C2)
    virtual INT OnDialogResult(WPARAM DialogID, LPARAM Unused);      // 73 0x10096f00
};
NW_ASSERT_OFFSET(NCSystemMenuWnd, BackTex, 0x98);
NW_ASSERT_OFFSET(NCSystemMenuWnd, Buttons, 0x9C);
NW_ASSERT_OFFSET(NCSystemMenuWnd, ButtonA, 0xB0);
NW_ASSERT_SIZE(NCSystemMenuWnd, 0xB8);

#pragma pack(pop)