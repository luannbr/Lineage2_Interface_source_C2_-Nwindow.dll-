#pragma once
// NCKoreanIME - the IME of Korean and English clients (UClass "CKoreanIME", 0x4204). Keeps the
// Hangul/alphanumeric mode across focus changes: while the chat line has the focus the mode is
// saved on Deactivate and restored on Activate. A composition with clause information is split
// into Clauses (Flag = target clause) for the edit box.

#include "nwindow/NCIME.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCKoreanIME : public NCIME
{
    NW_DECLARE_CLASS(NCKoreanIME, NCIME, 0, 0x101A1758)

    TArray<NImeText> Clauses;        // +0x41FC

    NCKoreanIME(INT InMode = 0);                                     // 0x10017ee0
    // ~NCKoreanIME                                                  // 0x10017f50

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x100116f0
    virtual INT OnCreate();                                          // 36 0x10011770
    virtual void Activate();                                         // 68 0x10011840
    virtual void Deactivate();                                       // 69 0x100118b0
    virtual INT OnImeNotify(WPARAM wParam, LPARAM lParam);           // 70 0x100117c0
    virtual INT OnImeComposition(WPARAM wParam, LPARAM lParam);      // 74 0x10013860
};
NW_ASSERT_OFFSET(NCKoreanIME, Clauses, 0x41FC);
NW_ASSERT_SIZE(NCKoreanIME, 0x4208);

#pragma pack(pop)