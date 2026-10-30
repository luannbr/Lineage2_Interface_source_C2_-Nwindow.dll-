#pragma once
// NCIMEManager - the IME of an edit box (UClass "CIMEManager", 0x1BC in C2): a hidden child that
// creates the IME of the client language (GLanguageType 0/1 Korean, 2 Japanese, 3 Taiwanese,
// 4 Chinese, 5 Thai (C2)), forwards Activate/Deactivate to it and passes the IME's private messages
// (NIM_*) on to the edit box. The getters read the IME's strings.

#include "nwindow/NCIME.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCIMEManager : public NControl
{
    NW_DECLARE_CLASS(NCIMEManager, NControl, 0, 0x101A0D10)

    NCIME* KoreanIME;                // +0x1A4 NCKoreanIME (GLanguageType 0 and 1)
    NCIME* JapaneseIME;              // +0x1A8 NCJapaneseIME (2)
    NCIME* TaiwanIME;                // +0x1AC NCTaiwanIME (3)
    NCIME* ChineseIME;               // +0x1B0 NCChineseIME (4)
    NCIME* ThaiIME;                  // +0x1B4 new in C2: NCThaiIME (5, NCThaiIME.h, class CThaiIME 0x61FC).
                                     //   Activate, Deactivate and IsNotComposing know it; the getters (GetCompStr,
                                     //   GetResultStr, GetReadingStr) do not
    INT Mode;                        // +0x1B8 passed to the IME

    NCIMEManager(INT InMode = 0);                                    // 0x10012900
    // ~NCIMEManager                                                 // 0x10012990

    // WindowProc: the stock body is NCEditBox's (0x10007f80): NIM_* messages to 65..72
    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x10010fa0
    virtual INT vfn033() { return 1; }                               // 34 0x10005b80 (folded)
    virtual INT OnCreate();                                          // 36 0x10018be0
    virtual INT OnDestroy();                                         // 37 0x100129e0
    // names ours
    virtual void Activate();                                         // 68 0x100111e0
    virtual void Deactivate();                                       // 69 0x10011240
    virtual INT OnComposition(WPARAM wParam, LPARAM lParam);         // 70 0x100112a0
    virtual INT OnResult(WPARAM wParam, LPARAM lParam);              // 71 0x100112c0
    virtual INT OnAttribute(WPARAM wParam, LPARAM lParam);           // 72 0x100112e0
    virtual INT OnReading(WPARAM wParam, LPARAM lParam);             // 73 0x10011300
    virtual INT On14(WPARAM wParam, LPARAM lParam) { return 1; }     // 74 0x100d2e70 (folded)
    virtual INT OnGetCaret(WPARAM wParam, LPARAM lParam);            // 75 0x10011320
    virtual INT OnEndComposition(WPARAM wParam, LPARAM lParam);      // 76 0x10011340
    virtual INT OnCursorPos(WPARAM wParam, LPARAM lParam);           // 77 0x10011360

    // names ours; NULL (-1 for IsNotComposing) without an IME
    TCHAR* GetCompStr();                                             // 0x10012b70
    TCHAR* GetResultStr();                                           // 0x10012bb0
    TCHAR* GetReadingStr();                                          // 0x10012bf0
    UBOOL IsNotComposing();                                          // 0x10012c30

    // new in C2 (names ours), members of this class by their calling convention (the edit boxes call them with
    // ecx = IMEManager; they do not read it): the class of a Thai character (GLanguageType 5 only, else 0): 1
    // space, 2 consonant, 3 sara am, 4 vowel above/below, 5 vowel before/after, 6 symbol, 7 digit, 8 tone mark,
    // 9 thanthakhat
    INT GetThaiCharType(UINT Ch);                                    // 0x100110e0
    // the length of Text in cursor steps: its Thai clusters (GLanguageType 5), else its characters
    INT GetTextLength(FString Text);                                 // 0x10017e40

    // the first IME created (inlined in the getters; the Thai one is not looked at)
    NCIME* GetIME()
    {
        if (KoreanIME)
            return KoreanIME;
        if (JapaneseIME)
            return JapaneseIME;
        if (TaiwanIME)
            return TaiwanIME;
        return ChineseIME;
    }
};
NW_ASSERT_OFFSET(NCIMEManager, KoreanIME, 0x1A4);
NW_ASSERT_OFFSET(NCIMEManager, ThaiIME, 0x1B4);
NW_ASSERT_OFFSET(NCIMEManager, Mode, 0x1B8);
NW_ASSERT_SIZE(NCIMEManager, 0x1BC);

#pragma pack(pop)