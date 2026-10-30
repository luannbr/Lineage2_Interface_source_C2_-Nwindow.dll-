#pragma once
// NCMatchOptionWnd - party matching options (UClass "CMatchOptionWnd", 0xBC): automatic
// registration, level open, class open and a memo, kept per character in MatchOption.ini
// ([character name] autoregister / levelopen / classopen / memo) and sent to the server on
// OK. Showing the window loads them back. The desktop (NCConsole +0xF0) owns it.

#include "nwindow/NControl.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCCheckBox;
class NCButton;
class NCMultiEdit;

class NCMatchOptionWnd : public NCWnd
{
    NW_DECLARE_CLASS(NCMatchOptionWnd, NCWnd, 0, 0x101C1E10)

    UTexture* BackTex;               // +0x98 "L2UI.MatchWnd.option"
    NCCheckBox* LevelOpenCheck;      // +0x9C
    NCCheckBox* ClassOpenCheck;      // +0xA0
    NCCheckBox* AutoRegisterCheck;   // +0xA4
    NCMultiEdit* MemoEdit;           // +0xA8
    NCButton* OKButton;              // +0xAC
    NCButton* CancelButton;          // +0xB0
    UBOOL bAutoRegister;             // +0xB4
    UBOOL bLevelOpen;                // +0xB8
    UBOOL bClassOpen;                // +0xBC

    NCMatchOptionWnd();                                              // 0x100947b0
    // ~NCMatchOptionWnd                                             // 0x10094820

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x100945e0
    virtual INT OnCreate();                                          // 36 0x10094870
    virtual INT OnDestroy();                                         // 37 0x10094c80
    virtual void ShowWindow();                                       // 43 0x10095a90
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x10094e00
    // names ours
    virtual INT OnOK();                                              // 67 0x10094ee0
    virtual INT OnCancel();                                          // 68 0x100a0830 (folded)

    void LoadOptions();                                              // 0x100950e0 (name ours)
};
NW_ASSERT_OFFSET(NCMatchOptionWnd, MemoEdit, 0xA8);
NW_ASSERT_OFFSET(NCMatchOptionWnd, bAutoRegister, 0xB4);
NW_ASSERT_SIZE(NCMatchOptionWnd, 0xC0);

#pragma pack(pop)