#pragma once
// NCLobbyWnd - the character selection screen (UClass "CLobbyWnd", 0xB0; NConsoleWnd +0x36CC):
// the character panel (NCPawnSelectWnd), the characters' names floating over the stage, a
// yes / no box, and "Create", "Delete", "Start", "Previous" in the lower right corner. At most
// seven characters; Delete and Start wait for a choice.

#include "nwindow/NCWnd.h"
#include "nwindow/NCGaraDialogBox.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCPawnSelectWnd;
class NCButton;

class NCLobbyWnd : public NCWnd
{
    NW_DECLARE_CLASS(NCLobbyWnd, NCWnd, 0, 0x101BA7B0)

    NCPawnSelectWnd* PawnSelectWnd;  // +0x98
    NCGaraDialogBox* DialogBox;      // +0x9C deletion / restoration questions (size 0x23C, ctor 0x10006F90)
    NCButton* StartButton;           // +0xA0
    NCButton* PreviousButton;        // +0xA4
    NCButton* CreateButton;          // +0xA8
    NCButton* DeleteButton;          // +0xAC
    INT NumCharacters;               // +0xB0

    NCLobbyWnd();                                                    // 0x10081e00
    // ~NCLobbyWnd                                                   // 0x10081e70

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x10080d50
    virtual INT OnCreate();                                          // 36 0x10082030
    virtual INT OnDestroy();                                         // 37 0x100824d0
    virtual void ShowWindow();                                       // 43 0x10081030
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x10081ec0
    // names ours
    virtual INT OnCommand10001() { return 1; }                       // 67 0x10005b80 (folded)
    virtual INT OnStart();                                           // 68 0x10080ec0
    virtual INT OnPrevious();                                        // 69 0x10080f90
    virtual INT OnCreateCharacter();                                 // 70 0x10080fb0
    virtual INT OnDelete();                                          // 71 0x10082650
    virtual INT OnDeleteAnswer(INT bYes, INT Unused);                // 72 0x10081000

    // names ours
    INT AddCharacter(User Info);                                     // 0x10080e50
    void ClearCharacters();                                          // 0x10080ea0
};
NW_ASSERT_OFFSET(NCLobbyWnd, PawnSelectWnd, 0x98);
NW_ASSERT_OFFSET(NCLobbyWnd, DialogBox, 0x9C);
NW_ASSERT_OFFSET(NCLobbyWnd, DeleteButton, 0xAC);
NW_ASSERT_SIZE(NCLobbyWnd, 0xB4);

#pragma pack(pop)