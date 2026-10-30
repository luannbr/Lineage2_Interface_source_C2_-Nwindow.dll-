#pragma once
// NCChineseIME - the IME of the Chinese client (UClass "CChineseIME", 0x6224). The composition
// is read as the Japanese IME reads it, but only with a Chinese IME (see IsChineseIME); the
// candidate list goes into an NCCandidateBox (9 a page, 10 for the IME type 9). The IME types
// 11 and 12 (NConsoleWnd::GetIMEType; 11 is never returned) also show the letters typed in an
// NCCompositeBox, rebuilt from the keyboard state on IMN_PRIVATE, and 12 takes a candidate with
// the digit keys. The reading string (ReadingStr, NIM_READING) comes from the IME's
// GetReadingString export or, for the old Microsoft Pinyin IMEs without it, from the IME's
// private data (IMN_PRIVATE).

#include "nwindow/NCIME.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCCompositeBox;

// the IME's GetReadingString (NCChineseIME::LoadReadingString; name ours), and
// NCChineseIME::IsNT's function statics in the stock code (names ours)
#if NW_HOOK_MODE
#define GChineseGetReadingString NW_GLOBAL(NGetReadingString, 0x1019fd9c)
#define GChineseOSVersion NW_GLOBAL(OSVERSIONINFOA, 0x101a21b8)
#define GChineseOSVersionRead NW_GLOBAL(bool, 0x101a224c)
#define GChineseNT NW_GLOBAL(bool, 0x101a224d)
#define GChineseNTChecked NW_GLOBAL(bool, 0x101a224e)
#else
extern NGetReadingString GChineseGetReadingString;
extern OSVERSIONINFOA GChineseOSVersion;
extern bool GChineseOSVersionRead;
extern bool GChineseNT;
extern bool GChineseNTChecked;
#endif

class NCChineseIME : public NCIME
{
    NW_DECLARE_CLASS(NCChineseIME, NCIME, 0, 0x101A07F0)

    TArray<NImeText> Clauses;        // +0x41FC as NCKoreanIME's
    CANDIDATELIST* CandList;         // +0x4208 copy of the IME's list (ImmGetCandidateListW)
    NCCandidateBox* CandidateBox;    // +0x420C
    NCCompositeBox* CompositeBox;    // +0x4210 letters typed (IME types 11 and 12)
    FString LastComp;                // +0x4214 the composition before the last one (IME type 10;
                                     //   the edit boxes move the box's caret by it)
    BYTE Unknown421C[0x2000];        // +0x4220 not used   // C2: offset derived, no evidence
    UBOOL bCandOpened;               // +0x6220 IME type 11: a candidate list opened since the
                                     //   last key (its changes are read only then)
    UBOOL bOpenResultSent;           // +0x6224 IME type 12: an opening candidate list sent a
                                     //   result (the next opening sends none)

    NCChineseIME(INT InMode = 0);                                    // 0x10018480
    // ~NCChineseIME                                                 // 0x10018530

    // WindowProc: the stock body is the Japanese IME's (0x1000ee70)
    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x100120f0
    virtual INT OnCreate();                                          // 36 0x10013170
    virtual INT OnDestroy();                                         // 37 0x10016a90
    virtual void ShowWindow();                                       // 43 0x10012350
    virtual void Deactivate();                                       // 69 0x10017d30
    virtual INT OnImeNotify(WPARAM wParam, LPARAM lParam);           // 70 0x100185b0
    virtual INT OnImeEndComposition(WPARAM wParam, LPARAM lParam);   // 72 0x100124e0
    virtual INT OnImeSetContext(WPARAM wParam, LPARAM lParam) { return 1; }   // 73 0x100d2e70 (folded)
    virtual INT OnImeComposition(WPARAM wParam, LPARAM lParam);      // 74 0x10016cc0
    virtual INT OnInputLangChange(WPARAM wParam, LPARAM lParam) { return 1; } // 75 0x100d2e70 (folded)
    // name ours: the edit box's answer to NIM_GETCARET
    virtual INT OnCaret(INT X, INT Y);                               // 79 0x100136b0

    // names ours
    void FreeCandList();                                             // 0x1000e6a0 (C1 only: no C2 match) (shared)
    void FillCandidateBox();                                         // 0x10013520
    void ReadCandList();                                             // 0x10016bb0
    void ReadReadingString();                                        // 0x100132e0
    UBOOL IsChineseIME();                                            // 0x100122a0 (this unused)
    static void LoadReadingString();                                 // 0x100121f0
    static DWORD GetImeId();                                         // 0x10012380 the old IMEs'
                                                                     //   version, 0 for the others
    static bool IsNT();                                              // 0x10012860
};
NW_ASSERT_OFFSET(NCChineseIME, Clauses, 0x41FC);
NW_ASSERT_OFFSET(NCChineseIME, CompositeBox, 0x4210);
NW_ASSERT_OFFSET(NCChineseIME, LastComp, 0x4214);
NW_ASSERT_OFFSET(NCChineseIME, bCandOpened, 0x6220);
NW_ASSERT_SIZE(NCChineseIME, 0x6228);

#pragma pack(pop)