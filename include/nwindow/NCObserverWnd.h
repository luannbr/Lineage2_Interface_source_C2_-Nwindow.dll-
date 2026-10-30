#pragma once
// NCObserverWnd - the box that stays on the screen while the player watches a duel or siege
// from the observer camera (UClass "CObserverWnd", 0xA0; C2 only): a panel with the title bar
// and one button that asks the server to end the observation.

#include "nwindow/NCWnd.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCButton;

class NCObserverWnd : public NCWnd
{
    NW_DECLARE_CLASS(NCObserverWnd, NCWnd, 0, 0x101C4748)

    UTexture* BackTex;               // +0x98 "L2UI.ObserverWnd.ObserverWndBack"
    NCButton* ExitButton;            // +0x9C id 0x1012, text SysString 611 (name ours)

    NCObserverWnd();                                                 // 0x10099b70
    // ~NCObserverWnd                                                // 0x10099bc0

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x100991b0
    virtual INT OnCreate();                                          // 36 0x10099c90
    virtual INT OnDestroy();                                         // 37 0x10099210
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x10099c10
    virtual INT OnExit();                                            // 67 0x10099290 (name ours)
};
NW_ASSERT_OFFSET(NCObserverWnd, BackTex, 0x98);
NW_ASSERT_OFFSET(NCObserverWnd, ExitButton, 0x9C);
NW_ASSERT_SIZE(NCObserverWnd, 0xA0);

#pragma pack(pop)
