#pragma once
// NCAutoLogWnd - a development screen that logs in by itself (UClass "CAutoLogWnd", 0xAC):
// black, a progress line, and on its first paint the login with the account and password of
// AutoLog.txt ([AutoLogOn] L2ID and L2Passwd, "zodiac" when missing; L2Slot is read too, and
// nothing uses it). Nothing in NWindow creates it, and nothing moves Step past the first line.

#include "nwindow/NCWnd.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

// what OnCreate reads from AutoLog.txt (names ours), and the progress lines: "1.Authorizing....",
// "2.Send infomations of character to server..", "3.Loading World......", "Can not connect to
// server" (name ours)
#if NW_HOOK_MODE
#define GAutoLogID       (reinterpret_cast<TCHAR*>(::nw::Addr(0x101af808)))   // TCHAR[64]
#define GAutoLogPassword (reinterpret_cast<TCHAR*>(::nw::Addr(0x101af788)))   // TCHAR[64]
#define GAutoLogSlot     NW_GLOBAL(INT, 0x101af888)
#define GAutoLogMessages (reinterpret_cast<const TCHAR* const*>(::nw::Addr(0x10166010)))
#else
extern TCHAR GAutoLogID[64];
extern TCHAR GAutoLogPassword[64];
extern INT GAutoLogSlot;
extern const TCHAR* const GAutoLogMessages[];
#endif

class NCAutoLogWnd : public NCWnd
{
    NW_DECLARE_CLASS(NCAutoLogWnd, NCWnd, 0, 0x101AF890)

    UTexture* BackTex;               // +0x98 "NWindow.BlackTexture"
    UBOOL bLoginSent;                // +0x9C
    INT Unknown9C;                   // +0xA0 (0)
    INT Step;                        // +0xA4 lines 0..Step show; 3 shows the fourth alone
    INT TextX;                       // +0xA8 40% of the width
    INT TextY;                       // +0xAC a quarter of the height

    NCAutoLogWnd();                                                  // 0x10044070
    // ~NCAutoLogWnd                                                 // 0x100440d0

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x10043ef0 (folded)
    virtual INT OnCreate();                                          // 36 0x10043f10
    virtual INT OnDestroy();                                         // 37 0x10043ff0 (folded)
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x10044120
};
NW_ASSERT_OFFSET(NCAutoLogWnd, BackTex, 0x98);
NW_ASSERT_OFFSET(NCAutoLogWnd, Step, 0xA4);
NW_ASSERT_SIZE(NCAutoLogWnd, 0xB0);

#pragma pack(pop)