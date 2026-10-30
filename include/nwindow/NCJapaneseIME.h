#pragma once
// NCJapaneseIME - the IME of the Japanese client (UClass "CJapaneseIME", 0x620C). The
// composition is read as the Korean IME reads it (clauses included) but only with a Japanese
// keyboard layout, and the cursor comes from the IME only while some characters are still
// being typed (attribute 0). The IME's candidate list is copied (CandList) and shown in an
// NCCandidateBox, 9 a page, placed at the caret the edit box reports (NIM_CARET).

#include "nwindow/NCIME.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCJapaneseIME : public NCIME
{
    NW_DECLARE_CLASS(NCJapaneseIME, NCIME, 0, 0x1019FDB0)

    TArray<NImeText> Clauses;        // +0x41FC as NCKoreanIME's
    CANDIDATELIST* CandList;         // +0x4208 copy of the IME's list (ImmGetCandidateListW)
    NCCandidateBox* CandidateBox;    // +0x420C
    BYTE Unknown420C[0x2000];        // +0x4210 not used   // C2: offset derived, no evidence

    NCJapaneseIME(INT InMode = 0);                                   // 0x10017fb0
    // ~NCJapaneseIME                                                // 0x10018020

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x100120f0 (shared)
    virtual INT OnCreate();                                          // 36 0x10012ce0
    virtual INT OnDestroy();                                         // 37 0x10014750
    virtual void ShowWindow();                                       // 43 0x10011f10
    virtual void Deactivate();                                       // 69 0x100158b0
    virtual INT OnImeNotify(WPARAM wParam, LPARAM lParam);           // 70 0x10018080
    virtual INT OnImeEndComposition(WPARAM wParam, LPARAM lParam);   // 72 0x10011980
    virtual INT OnImeSetContext(WPARAM wParam, LPARAM lParam) { return 1; } // 73 0x100d2e70 (folded)
    virtual INT OnImeComposition(WPARAM wParam, LPARAM lParam);      // 74 0x10014910
    virtual INT OnInputLangChange(WPARAM wParam, LPARAM lParam) { return 1; } // 75 0x100d2e70 (folded)
    // name ours: the edit box's answer to NIM_GETCARET
    virtual INT OnCaret(INT X, INT Y);                               // 79 0x10012da0

    // names ours
    void FreeCandList();                                             // 0x1000e6a0 (C1 only: no C2 match) (shared)
    void FillCandidateBox();                                         // 0x10012ee0 (shared)
    void ReadCandList();                                             // 0x10014840
    UBOOL IsJapaneseLayout();                                        // 0x10011960 (this unused)
};
NW_ASSERT_OFFSET(NCJapaneseIME, Clauses, 0x41FC);
NW_ASSERT_OFFSET(NCJapaneseIME, CandidateBox, 0x420C);
NW_ASSERT_SIZE(NCJapaneseIME, 0x6210);

#pragma pack(pop)