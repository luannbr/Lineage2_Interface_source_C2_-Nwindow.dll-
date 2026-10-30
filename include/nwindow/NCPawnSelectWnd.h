#pragma once
// NCPawnSelectWnd - the character panel of the character selection (UClass "CPawnSelectWnd",
// 0xEEC; NCLobbyWnd +0x94): up to seven characters (their User records by value, each with the
// actor shown on the stage), a name combo box, and the chosen one's level, class, HP / MP / exp
// bars and numbers. Choosing a character marked for deletion asks whether to restore it.

#include <math.h>
#include "nwindow/NCWnd.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCComboBox;

// C2: User::DeleteDays (+0x144) is two things now: the low 16 bits are the days left until the
// deletion, bit 16 (the stock code computes it as pow(2, 16)) locks the character: the character
// selection neither selects it nor starts the game with it (names ours)
#define CHAR_LOCKED_FLAG ((INT)pow(2.0, 16.0))

class NCPawnSelectWnd : public NCWnd
{
    NW_DECLARE_CLASS(NCPawnSelectWnd, NCWnd, 0, 0x101BA288)

    UTexture* BackTex;               // +0x98 "L2UI.statusWnd.selectback"
    UTexture* HPTex;                 // +0x9C "L2UI.StatusWnd.statusHP1"
    UTexture* MPTex;                 // +0xA0 "L2UI.StatusWnd.statusMP1"
    UTexture* EXPTex;                // +0xA4 "L2UI.StatusWnd.statusEXP1"
    INT UnknownA4;                   // +0xA8 (0)
    NCComboBox* NameCombo;           // +0xAC
    User Characters[7];              // +0xB0 the User records by value (0x1F8 each in C2, 0x208 in C1); the actor of each
                                     //   is its Pawn (+0x140)
    INT NumCharacters;               // +0xE78
    INT Selected;                    // +0xE7C -1 = none

    NCPawnSelectWnd();                                               // 0x10081070
    // ~NCPawnSelectWnd                                              // 0x10081140

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x10080a00
    virtual INT OnCreate();                                          // 36 0x10081190
    virtual INT OnDestroy();                                         // 37 0x10081ac0
    virtual void ShowWindow();                                       // 43 0x1007b460 (folded)
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x10081340
    // names ours
    virtual void SelectCharacter(INT Index);                         // 67 0x10080ba0
    virtual INT OnSelect(INT Index);                                 // 68 0x10081b70
    virtual INT OnRestoreAnswer(INT bYes, INT Unused);               // 69 0x10080ca0

    // names ours
    INT AddCharacter(User Info);                                     // 0x10080a80
    INT GetSelected();                                               // 0x10080bf0
    TCHAR* GetCharacterName(INT Index);                              // 0x10080c00
    void ClearCharacters();                                          // 0x10080c20
};
NW_ASSERT_OFFSET(NCPawnSelectWnd, NameCombo, 0xAC);
NW_ASSERT_OFFSET(NCPawnSelectWnd, Characters, 0xB0);
NW_ASSERT_OFFSET(NCPawnSelectWnd, NumCharacters, 0xE78);
NW_ASSERT_SIZE(NCPawnSelectWnd, 0xE80);

#pragma pack(pop)