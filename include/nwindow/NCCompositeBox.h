#pragma once
// NCCompositeBox - the IME composition window (UClass "CCompositeBox", 0xB0): the string
// being composed, drawn one character at a time from FirstChar until about 100 px, with a
// "|" caret before the character at CaretPos while bShowCaret is set. Filled by the IMEs.

#include "nwindow/NCWnd.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCCompositeBox : public NCWnd
{
    NW_DECLARE_CLASS(NCCompositeBox, NCWnd, 0, 0x10199580)

    UTexture* BackTex;         // +0x98 NWindow.BlackTexture
    FString Text;              // +0x9C composition string
    INT FirstChar;             // +0xA8 first character drawn
    INT CaretPos;              // +0xAC
    UBOOL bShowCaret;          // +0xB0

    NCCompositeBox();                                                // 0x10003ac0
    // the Chinese IME's: size and place given (BackTex is left as allocated)
    NCCompositeBox(INT W, INT H, INT X, INT Y, UBOOL bInShowCaret);  // 0x10003bd0
    // ~NCCompositeBox                                               // 0x10003b60

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x10043ef0 (NControl's)
    virtual INT OnCreate();                                          // 36 0x10003090
    virtual INT OnDestroy();                                         // 37 0x10003130 (shared)
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x10003c60

    // names ours; the text scrolls so that the caret stays within 97 px
    void CaretLeft();                                                // 0x100030c0
    void CaretRight();                                               // 0x10003e40
    void SetText(const TCHAR* InText);                               // 0x10003fb0
    FString GetText();                                               // 0x100030f0 a copy
    void SetShowCaret(UBOOL bShow);                                  // 0x10003120
};
NW_ASSERT_OFFSET(NCCompositeBox, Text, 0x9C);
NW_ASSERT_SIZE(NCCompositeBox, 0xB4);

#pragma pack(pop)