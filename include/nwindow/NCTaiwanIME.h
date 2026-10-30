#pragma once
// NCTaiwanIME - the IME of the Taiwanese client (UClass "CTaiwanIME", 0x620C). The composition
// is read as the Korean IME reads it, but only with a Taiwanese IME (see IsTaiwanIME); the
// candidate list goes into an NCCandidateBox as the Japanese IME's does, laid out by the IME
// type the console reports. The reading string (ReadingStr, NIM_READING) comes from the IME's
// GetReadingString export or, for the old Microsoft IMEs without it, from the IME's private
// data (IMN_PRIVATE).

#include "nwindow/NCIME.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

// the IME's GetReadingString (NCTaiwanIME::LoadReadingString; name ours)
#if NW_HOOK_MODE
#define GTaiwanGetReadingString NW_GLOBAL(NGetReadingString, 0x1019fd98)
#else
extern NGetReadingString GTaiwanGetReadingString;
#endif

class NCTaiwanIME : public NCIME
{
    NW_DECLARE_CLASS(NCTaiwanIME, NCIME, 0, 0x101A1C88)

    TArray<NImeText> Clauses;        // +0x41FC as NCKoreanIME's
    CANDIDATELIST* CandList;         // +0x4208 copy of the IME's list (ImmGetCandidateListW)
    NCCandidateBox* CandidateBox;    // +0x420C
    BYTE Unknown420C[0x2000];        // +0x4210 not used   // C2: offset derived, no evidence

    NCTaiwanIME(INT InMode = 0);                                     // 0x10018120
    // ~NCTaiwanIME                                                  // 0x10018190

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x100119a0
    virtual INT OnCreate();                                          // 36 0x10012e10
    virtual INT OnDestroy();                                         // 37 0x100159b0 (shared)
    virtual void ShowWindow();                                       // 43 0x10011f10 (shared)
    virtual void Deactivate();                                       // 69 0x10012000
    virtual INT OnImeNotify(WPARAM wParam, LPARAM lParam);           // 70 0x100181f0
    virtual INT OnImeSetContext(WPARAM wParam, LPARAM lParam);       // 73 0x10011f30
    virtual INT OnImeComposition(WPARAM wParam, LPARAM lParam);      // 74 0x10015aa0
    virtual INT OnInputLangChange(WPARAM wParam, LPARAM lParam) { return 1; } // 75 0x100d2e70 (folded)
    // name ours: the edit box's answer to NIM_GETCARET
    virtual INT OnCaret(INT X, INT Y);                               // 79 0x10013050

    // names ours
    void FreeCandList();                                             // 0x1000e6a0 (C1 only: no C2 match) (shared)
    void FillCandidateBox();                                         // 0x10012ee0 (shared)
    void ReadCandList();                                             // 0x100169a0
    void ReadReadingString();                                        // 0x10011c50
    UBOOL IsTaiwanIME();                                             // 0x10011e70 (this unused)
    static void LoadReadingString();                                 // 0x10011a80
    static DWORD GetImeId();                                         // 0x10011ae0 the old IMEs'
                                                                     //   version, 0 for the others
};
NW_ASSERT_OFFSET(NCTaiwanIME, Clauses, 0x41FC);
NW_ASSERT_OFFSET(NCTaiwanIME, CandidateBox, 0x420C);
NW_ASSERT_SIZE(NCTaiwanIME, 0x6210);

#pragma pack(pop)