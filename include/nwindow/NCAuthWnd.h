#pragma once
// NCAuthWnd - the login box of the login screen (UClass "CAuthWnd", 0xB4): account and password
// lines, "Log In" and "Exit" (NCLoginWnd creates it). Log In, or Enter in the password line,
// checks that both lines are filled and asks NConsoleWnd::RequestAuthLogin - once, until the
// box is shown again or its 3 s timer fires. Exit quits the game.

#include "nwindow/NCWnd.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCEditBox;
class NCButton;

class NCAuthWnd : public NCWnd
{
    NW_DECLARE_CLASS(NCAuthWnd, NCWnd, 0, 0x101BACD0)

    INT Mode;                        // +0x98 (new in C2, name ours) 0 account and password, 1 the card key
                                     //   (the security card: only the password line is used, with the
                                     //   prompt of systemmsg 1237 for CardPos)
    INT CardPos;                     // +0x9C (new in C2, name ours) the argument of systemmsg 1237 (Mode 1)
    INT CardID;                      // +0xA0 (new in C2, name ours) first argument of RequestAuthCardKeyLogin
    INT Unknown94;                   // +0xA4 (0, never used)
    UTexture* BackTex;               // +0xA8 "L2UI.LobbyWnd.loginBack"
    NCEditBox* AccountEdit;          // +0xAC
    NCEditBox* PasswordEdit;         // +0xB0
    NCButton* OKButton;              // +0xB4 "Log In"
    NCButton* CancelButton;          // +0xB8 "Exit"
    INT TimerResult;                 // +0xBC what SetTimer returned (never read)
    UBOOL bRequested;                // +0xC0 a login request went out

    NCAuthWnd();                                                     // 0x10083030
    // ~NCAuthWnd                                                    // 0x100830b0

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x10082810
    virtual INT OnTimer(INT TimerID);                                // 35 0x10082a70
    virtual INT OnCreate();                                          // 36 0x10083100
    virtual INT OnDestroy();                                         // 37 0x10083790
    virtual void ShowWindow();                                       // 43 0x10082a30
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x10083580
    // names ours
    virtual INT OnOK();                                              // 67 0x100828c0
    virtual INT OnCancel();                                          // 68 0x1004b820 (folded)
    virtual INT OnPasswordNotify(INT Code);                          // 69 0x10088c40 (folded)

    void ClearPassword();                                            // 0x100828b0 (name ours)
    void ResetInput();                                               // 0x10057cd0 (name ours) back to the account and password lines
    void SetCardKey(INT InCardID, INT InCardPos);                    // 0x10057c80 (name ours) the card key prompt (Mode 1)
};
NW_ASSERT_OFFSET(NCAuthWnd, Mode, 0x98);
NW_ASSERT_OFFSET(NCAuthWnd, CardID, 0xA0);
NW_ASSERT_OFFSET(NCAuthWnd, BackTex, 0xA8);
NW_ASSERT_OFFSET(NCAuthWnd, AccountEdit, 0xAC);
NW_ASSERT_OFFSET(NCAuthWnd, OKButton, 0xB4);
NW_ASSERT_OFFSET(NCAuthWnd, bRequested, 0xC0);
NW_ASSERT_SIZE(NCAuthWnd, 0xC4);

#pragma pack(pop)