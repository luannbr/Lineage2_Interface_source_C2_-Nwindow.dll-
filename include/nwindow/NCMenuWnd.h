#pragma once
// NCMenuWnd - the main menu bar (UClass "CMenuWnd", 0xA8): four big icons, each opening or
// closing one of the desktop's windows (Character Status Alt+I, Inventory Alt+V, Community
// Alt+B, System Alt+X), and a thin title bar on the left to drag it.

#include "nwindow/NCWnd.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCButton;

// NWM_COMMAND ids of the buttons and the title bar
enum
{
    NID_MENU_INVENTORY = 0xF0F000,
    NID_MENU_CHARACTER = 0xF0F001,
    NID_MENU_SYSTEM = 0xF0F003,
    NID_MENU_COMMUNITY = 0xF0F006,
    NID_MENU_FRAME = 0xF0F021,
};

class NCMenuWnd : public NCWnd
{
    NW_DECLARE_CLASS(NCMenuWnd, NCWnd, 0, 0x101C32B0)

    UTexture* BackTex;                     // +0x98 "L2UI.EtcWndBack.SysMenuBack"
    NCButton* SystemButton;                // +0x9C
    NCButton* InventoryButton;             // +0xA0
    NCButton* CharacterButton;             // +0xA4
    NCButton* CommunityButton;             // +0xA8

    NCMenuWnd();                                                     // 0x100980c0
    // ~NCMenuWnd                                                    // 0x10098130

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x10096ff0
    virtual INT OnCreate();                                          // 36 0x10098200
    virtual INT OnDestroy();                                         // 37 0x10098580 (folded)
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x10098180
    // names ours
    virtual INT OnSystem();                                          // 67 0x100970a0
    virtual INT OnCharacter();                                       // 68 0x100970c0
    virtual INT OnInventory();                                       // 69 0x100970e0
    virtual INT OnCommunity();                                       // 70 0x10097100
};
NW_ASSERT_OFFSET(NCMenuWnd, BackTex, 0x98);
NW_ASSERT_OFFSET(NCMenuWnd, CommunityButton, 0xA8);
NW_ASSERT_SIZE(NCMenuWnd, 0xAC);

#pragma pack(pop)