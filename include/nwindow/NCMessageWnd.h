#pragma once
// NCMessageWnd - a line of text on a background (UClass "CMessageWnd", 0x498): the console's
// message window (NConsoleWnd +0x36D8, NConsoleWnd::ShowMessageWnd), e.g. while the login
// waits for the server list. The text is centered, or drawn from (50, 8) and wrapped to the
// screen width when it is wider than the window.

#include "nwindow/NCWnd.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCMessageWnd : public NCWnd
{
    NW_DECLARE_CLASS(NCMessageWnd, NCWnd, 0, 0x101C3CF8)

    UTexture* BackTex;               // +0x98 "sek.cbui141" stretched over the window
    FString Text;                    // +0x9C C2: an FString (C1: TCHAR Text[512])

    NCMessageWnd();                                                  // 0x100987c0
    // ~NCMessageWnd                                                 // 0x10098850

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x10043ef0 (NControl's body)
    virtual INT OnCreate();                                          // 36 0x100986e0
    virtual INT OnDestroy();                                         // 37 0x10098710 (NCCompositeBox's body)
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x100988c0

    // names ours
    void ClearText();                                                // 0x10098790 (C1 0x10077fd0)
    void SetText(const TCHAR* NewText);                              // 0x100987a0 NULL keeps the text
};
NW_ASSERT_OFFSET(NCMessageWnd, BackTex, 0x98);
NW_ASSERT_OFFSET(NCMessageWnd, Text, 0x9C);
NW_ASSERT_SIZE(NCMessageWnd, 0xA8);

#pragma pack(pop)