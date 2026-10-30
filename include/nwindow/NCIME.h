#pragma once
// NCIME - base of the language IMEs (UClass "CIME", 0x41F8), hidden controls owned by
// NCIMEManager. Each keeps its own input context and a copy of every composition string
// (ImmGetCompositionStringW), which the edit boxes read. WindowProc turns the IME window
// messages into virtual calls; the base handlers only follow the composition state and the
// IME properties. Activate/Deactivate run when an edit box gains or loses the focus.
//
// Private messages between the IMEs, NCIMEManager and the edit boxes (names ours). The IMEs send
// them to their parent (NControl::SendToParent), the manager passes them on to the edit box:
//   0xF0000012 composition changed (wParam: last composed character, 0 when none, or 1)
//   0xF0000013 result string ready          0xF0000017 composition ended (Japanese, Chinese)
//   0xF0000015 caret position wanted: wParam = the window to answer with 0xF0000016 (X, Y)
//   0xF0000018 composition attribute on/off 0xF0000019 cursor position (wParam)
//   0xF000001A reading string changed (Chinese, Taiwanese)

#include "nwindow/NControl.h"
#include "nwindow/NCCandidateBox.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

enum
{
    NIM_COMPOSITION = (INT)0xF0000012,
    NIM_RESULT = (INT)0xF0000013,
    NIM_14 = (INT)0xF0000014,
    NIM_GETCARET = (INT)0xF0000015,
    NIM_CARET = (INT)0xF0000016,
    NIM_ENDCOMPOSITION = (INT)0xF0000017,
    NIM_ATTRIBUTE = (INT)0xF0000018,
    NIM_CURSORPOS = (INT)0xF0000019,
    NIM_READING = (INT)0xF000001A,
};

// C2 (name ours): the input context of the IME that is active, kept for NConsoleWnd::GetIMEType (which asks
// its open status): set by NCIME::Activate, cleared by NCIME::Deactivate (both only with a console)
#if NW_HOOK_MODE
#define GActiveIMC NW_GLOBAL(HIMC, 0x101b1774)
#else
extern HIMC GActiveIMC;
#endif

class NCIME : public NControl
{
    NW_DECLARE_CLASS(NCIME, NControl, 0, 0x101A02D0)

    HIMC hIMC;                       // +0x1A4 own input context (ImmCreateContext)
    UBOOL bNotComposing;             // +0x1A8 cleared between WM_IME_START/ENDCOMPOSITION
    INT Mode;                        // +0x1AC (Korean: 1 starts in native mode)
    UBOOL bActive;                   // +0x1B0 between Activate and Deactivate
    INT Unknown1B0;                  // +0x1B4   // C2: offset derived, no evidence
    INT CursorPos;                   // +0x1B8 GCS_CURSORPOS   // C2: offset derived, no evidence
    INT Unknown1B8;                  // +0x1BC   // C2: offset derived, no evidence
    DWORD Property;                  // +0x1C0 IGP_PROPERTY of the current IME
    DWORD Conversion;                // +0x1C4 conversion / sentence mode kept while inactive
    DWORD Sentence;                  // +0x1C8
    UBOOL OpenStatus;                // +0x1CC ImmGetOpenStatus kept (Japanese, Taiwanese, Chinese)
    UINT CodePage;                   // +0x1D0 of the keyboard layout (non-Unicode systems)
    TCHAR CompStr[1024];             // +0x1D4 GCS_COMPSTR
    TCHAR CompReadStr[1024];         // +0x9D4 GCS_COMPREADSTR
    TCHAR ResultStr[1024];           // +0x11D4 GCS_RESULTSTR
    TCHAR Unknown19D0[1024];         // +0x19D4   // C2: offset derived, no evidence
    TCHAR ReadingStr[1024];          // +0x21D4 the reading string (Taiwanese, Chinese: NIM_READING)
    BYTE CompAttr[1024];             // +0x29D4 GCS_COMPATTR   // C2: offset derived, no evidence
    BYTE CompReadAttr[1024];         // +0x2DD4 GCS_COMPREADATTR   // C2: offset derived, no evidence
    DWORD CompClause[256];           // +0x31D4 GCS_COMPCLAUSE   // C2: offset derived, no evidence
    DWORD CompReadClause[256];       // +0x35D4 GCS_COMPREADCLAUSE   // C2: offset derived, no evidence
    DWORD ResultClause[256];         // +0x39D4 GCS_RESULTCLAUSE   // C2: offset derived, no evidence
    DWORD Unknown3DD0[256];          // +0x3DD4   // C2: offset derived, no evidence
    INT CompStrLen;                  // +0x41D4 characters   // C2: offset derived, no evidence
    INT CompAttrLen;                 // +0x41D8   // C2: offset derived, no evidence
    INT CompClauseLen;               // +0x41DC entries   // C2: offset derived, no evidence
    INT CompReadStrLen;              // +0x41E0   // C2: offset derived, no evidence
    INT CompReadAttrLen;             // +0x41E4   // C2: offset derived, no evidence
    INT CompReadClauseLen;           // +0x41E8   // C2: offset derived, no evidence
    INT ResultStrLen;                // +0x41EC   // C2: offset derived, no evidence
    INT ResultClauseLen;             // +0x41F0   // C2: offset derived, no evidence
    INT Unknown41F0;                 // +0x41F4   // C2: offset derived, no evidence
    INT Unknown41F4;                 // +0x41F8   // C2: offset derived, no evidence

    NCIME(INT InMode = 0);                                           // 0x10012c70
    // ~NCIME                                                        // 0x10012780

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x10011380
    virtual INT OnCreate();                                          // 36 0x100114e0
    virtual INT OnDestroy() { return 1; }                            // 37 0x10005b80 (folded)
    // names ours
    virtual void Activate();                                         // 68 0x100115e0
    virtual void Deactivate();                                       // 69 0x10011620
    virtual INT OnImeNotify(WPARAM wParam, LPARAM lParam) { return 1; }          // 70 0x100d2e70
    virtual INT OnImeStartComposition(WPARAM wParam, LPARAM lParam);             // 71 0x10011520
    virtual INT OnImeEndComposition(WPARAM wParam, LPARAM lParam);               // 72 0x10011540
    virtual INT OnImeSetContext(WPARAM wParam, LPARAM lParam);                   // 73 0x10011570
    virtual INT OnImeComposition(WPARAM wParam, LPARAM lParam) { return 1; }     // 74 0x100d2e70
    virtual INT OnInputLangChange(WPARAM wParam, LPARAM lParam);                 // 75 0x10011550
    virtual INT OnImeControl(WPARAM wParam, LPARAM lParam) { return 1; }         // 76 0x100d2e70 (C1: 71)
    virtual INT OnImeCompositionFull(WPARAM wParam, LPARAM lParam) { return 1; } // 77 0x100d2e70 (C1: 72)
    virtual INT OnImeChar(WPARAM wParam, LPARAM lParam) { return 1; }            // 78 0x100d2e70 (C1: 73)

    // names ours
    TCHAR* GetResultStr();                                           // 0x100115b0
    TCHAR* GetCompStr();                                             // 0x100115c0
    TCHAR* GetReadingStr();                                          // 0x100115d0
    UBOOL IsNotComposing();                                          // 0x10011640
    UBOOL IsUnicodeOS();                                             // 0x10011650 NT or Win98+
    // the Taiwanese and Chinese IMEs' setup (names ours)
    void UpdateCodePage();                                           // 0x100116b0
    static void LoadImmFunctions();                                  // 0x10012250

    // the test every WM_IME_COMPOSITION handler makes before each string (inlined): Chinese,
    // Taiwanese and Japanese IMEs on a system without Unicode IME calls convert from CodePage
    UBOOL IsAnsiIME()
    {
        return (GLanguageType == 2 || GLanguageType == 3 || GLanguageType == 4) && !IsUnicodeOS();
    }
};
NW_ASSERT_OFFSET(NCIME, hIMC, 0x1A4);
NW_ASSERT_OFFSET(NCIME, CodePage, 0x1D0);
NW_ASSERT_OFFSET(NCIME, CompAttr, 0x29D4);
NW_ASSERT_OFFSET(NCIME, ResultClause, 0x39D4);
NW_ASSERT_OFFSET(NCIME, CompStrLen, 0x41D4);
NW_ASSERT_SIZE(NCIME, 0x41FC);

// imm32.dll's input context locks, found by NCIME::LoadImmFunctions: the reading string of the
// Taiwanese and Chinese IMEs without the GetReadingString API is read in the IME's private data
// (names ours)
typedef BYTE*(WINAPI* NImmLockIMC)(HIMC);
typedef BOOL(WINAPI* NImmUnlockIMC)(HIMC);
typedef LPVOID(WINAPI* NImmLockIMCC)(HIMCC);
typedef BOOL(WINAPI* NImmUnlockIMCC)(HIMCC);
#if NW_HOOK_MODE
#define GImmLockIMC NW_GLOBAL(NImmLockIMC, 0x1019fda0)
#define GImmUnlockIMC NW_GLOBAL(NImmUnlockIMC, 0x1019fda4)
#define GImmLockIMCC NW_GLOBAL(NImmLockIMCC, 0x1019fda8)
#define GImmUnlockIMCC NW_GLOBAL(NImmUnlockIMCC, 0x1019fdac)
#else
extern NImmLockIMC GImmLockIMC;
extern NImmUnlockIMC GImmUnlockIMC;
extern NImmLockIMCC GImmLockIMCC;
extern NImmUnlockIMCC GImmUnlockIMCC;
#endif

// an IME's GetReadingString export (names ours)
typedef UINT(WINAPI* NGetReadingString)(HIMC, UINT, LPWSTR, PINT, BOOL*, PUINT);

// INPUTCONTEXT::hPrivate (immdev.h, not included)
inline HIMCC IMCPrivate(BYTE* IC) { return *(HIMCC*)(IC + 0x124); }

// OSVERSIONINFO cache of NCIME::IsUnicodeOS (function statics in the stock code)
#if NW_HOOK_MODE
#define GIMEOSChecked NW_GLOBAL(UBOOL, 0x101a21b4)
#define GIMEUnicodeOS NW_GLOBAL(UBOOL, 0x101a21b0)
#else
extern UBOOL GIMEOSChecked;
extern UBOOL GIMEUnicodeOS;
#endif

#pragma pack(pop)