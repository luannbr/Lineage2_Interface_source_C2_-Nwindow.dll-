#pragma once
// NCTestUIWnd - a test window (UClass "CTestUIWnd", 0xD0): a 512 px title bar over the chat
// background stretched on a scroll window. Nothing in NWindow creates it.

#include "nwindow/NCScrollWnd.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCTestUIWnd : public NCScrollWnd
{
    NW_DECLARE_CLASS(NCTestUIWnd, NCScrollWnd, 0, 0x101C18F0)

    UTexture* BackTex;               // +0xD0 "NWindow.ChatBack"

    NCTestUIWnd();                                                   // 0x100957e0
    // ~NCTestUIWnd                                                  // 0x10095830

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x10019810 (folded)
    virtual INT OnCreate();                                          // 36 0x10095880
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x10095940
};
NW_ASSERT_OFFSET(NCTestUIWnd, BackTex, 0xD0);
NW_ASSERT_SIZE(NCTestUIWnd, 0xD4);

#pragma pack(pop)