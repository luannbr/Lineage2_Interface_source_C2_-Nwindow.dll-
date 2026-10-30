#pragma once
// NCThaiIME - the IME of the Thai client (UClass "CThaiIME", 0x61FC; new in C2, GLanguageType 5). It only keeps
// the input context of the game window: no composition is read (the Thai text is typed with the keyboard layout,
// not composed), so the base class's string buffers stay empty and the edit box tests the characters itself
// (NCEditBox::IsThaiInputValid). The class is as big as the Taiwanese IME's (0x620C minus one pointer) but nothing
// in its code touches the fields past NCIME's.

#include "nwindow/NCIME.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCThaiIME : public NCIME
{
    NW_DECLARE_CLASS(NCThaiIME, NCIME, 0, 0x101A1238)

    BYTE Unknown41FC[0x2000];        // +0x41FC not used by any function of this class (allocated zeroed)

    NCThaiIME(INT InMode = 0);                                       // 0x10013790
    // ~NCThaiIME                                                    // 0x100137e0

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x100125a0
    virtual INT OnCreate();                                          // 36 0x10012640
    virtual INT OnDestroy();                                         // 37 0x10012650
    virtual void ShowWindow();                                       // 43 0x1007b460 (shared)
    virtual void Deactivate();                                       // 69 0x100126b0
    virtual INT OnImeNotify(WPARAM wParam, LPARAM lParam);           // 70 0x10012670
    virtual INT OnImeSetContext(WPARAM wParam, LPARAM lParam) { return 1; }      // 73 0x100d2e70 (folded)
    virtual INT OnInputLangChange(WPARAM wParam, LPARAM lParam) { return 1; }    // 75 0x100d2e70 (folded)
};
NW_ASSERT_OFFSET(NCThaiIME, Unknown41FC, 0x41FC);
NW_ASSERT_SIZE(NCThaiIME, 0x61FC);

#pragma pack(pop)
