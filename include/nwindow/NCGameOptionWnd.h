#pragma once
// NCGameOptionWnd - the game page of the options window (UClass "CGameOptionWnd", 0xD4):
// language (native or English), which names show over characters (player, monsters, other PCs
// - and among those clan, party, general), camera tracking, the chat input options, a button
// that puts every window back in place, party loot and the game pad. The settings live in Core
// globals (GL2Show*, GIsEnterChatting...) and in Option.ini [Game].

#include "nwindow/NCWnd.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCComboBox;
class NCButton;
class NCCheckBox;

// NameBoxes[] (names ours): "Other PCs" switches the last three
enum
{
    NAMEBOX_Player,
    NAMEBOX_Monster,
    NAMEBOX_OtherPC,
    NAMEBOX_Clan,
    NAMEBOX_Party,
    NAMEBOX_General,
    NAMEBOX_MAX
};

// NWM_COMMAND ids of the page's controls
enum
{
    NID_GAMEOPT_LANGUAGE = 0x1F15,
    NID_GAMEOPT_PLAYER = (INT)0xF0F01459,
    NID_GAMEOPT_MONSTER = (INT)0xF0F01460,
    NID_GAMEOPT_OTHERPC = (INT)0xF0F01461,
    NID_GAMEOPT_CLAN = (INT)0xF0F01462,
    NID_GAMEOPT_PARTY = (INT)0xF0F01463,
    NID_GAMEOPT_GENERAL = (INT)0xF0F01464,
    NID_GAMEOPT_TRACKING = (INT)0xF0F01470,
    NID_GAMEOPT_ENTERCHAT = (INT)0xF0F01471,
    NID_GAMEOPT_OLDCHAT = (INT)0xF0F01472,
    NID_GAMEOPT_GAMEPAD = (INT)0xF0F01473,
    NID_GAMEOPT_KEYCRYPT = (INT)0xF0F01474,   // new in C2
    NID_GAMEOPT_RESET = (INT)0xFFFF1400,
    NID_GAMEOPT_PARTYLOOT = (INT)0xFFFF1401,
};

class NCGameOptionWnd : public NCWnd
{
    NW_DECLARE_CLASS(NCGameOptionWnd, NCWnd, 0, 0x101C8A30)

    NCComboBox* LanguageCombo;             // +0x98 0 native, 1 English
    NCButton* ResetButton;                 // +0x9C "Initialize"
    NCWnd* Unused9C[3];                    // +0xA0 destroyed, never created
    NCCheckBox* NameBoxes[NAMEBOX_MAX];    // +0xAC
    NCCheckBox* EnterChatBox;              // +0xC4 GIsEnterChatting
    NCCheckBox* OldChatBox;                // +0xC8 GIsOldChat
    NCCheckBox* TrackingBox;               // +0xCC GL2UseAutoTrackingPawn
    NCComboBox* PartyLootCombo;            // +0xD0 GL2PartyLooting: 0 finders keepers, 1 random
    NCCheckBox* GamePadBox;                // +0xD4 only with a game pad (GL2CanUseJoystick)
    NCCheckBox* KeyCryptBox;               // +0xD8 new in C2 (name ours): the nProtect key encryption at startup
                                           //   (only with GL2UseKeyCrypt; NPKLoadAtStartup)

    NCGameOptionWnd();                                               // 0x100a3280
    // ~NCGameOptionWnd                                              // 0x100a32e0

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x100a06a0
    virtual INT OnCreate();                                          // 36 0x100a5c60
    virtual INT OnDestroy();                                         // 37 0x100a3760
    virtual void ShowWindow();                                       // 43 0x100a0740
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x100a3330
    // names ours
    virtual INT OnOtherPCClicked();                                  // 67 0x100a4360
    virtual INT OnReset();                                           // 68 0x100a0720

    // names ours
    void LoadSettings();                                             // 0x100a3ad0
    void Apply();                                                    // 0x100a3ec0
    void ClearItems();                                               // 0x100a3a10 (C2 only; before InitItems)
    void InitItems();                                                // 0x100a50b0
};
NW_ASSERT_OFFSET(NCGameOptionWnd, Unused9C, 0xA0);
NW_ASSERT_OFFSET(NCGameOptionWnd, NameBoxes, 0xAC);
NW_ASSERT_OFFSET(NCGameOptionWnd, EnterChatBox, 0xC4);
NW_ASSERT_OFFSET(NCGameOptionWnd, PartyLootCombo, 0xD0);
NW_ASSERT_SIZE(NCGameOptionWnd, 0xDC);

#pragma pack(pop)