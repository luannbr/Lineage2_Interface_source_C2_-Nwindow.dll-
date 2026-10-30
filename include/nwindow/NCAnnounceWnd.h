#pragma once
// NCAnnounceWnd - the announce box of the login screen (UClass "CAnnounceWnd", 0xA4; C2 only;
// NCLoginWnd +0xA8): a picture ("Announce_cn") and an OK button (or Enter) that shows the
// announcements (NCNoticeWnd, with the EULA text) when there is one, or else asks for the
// server list.

#include "nwindow/NCWnd.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCButton;

class NCAnnounceWnd : public NCWnd
{
    NW_DECLARE_CLASS(NCAnnounceWnd, NCWnd, 0, 0x101C6108)

    UTexture* BackTex;               // +0x98 "L2UI.etcWnd.Announce_cn" below the title bar
    NCButton* OKButton;              // +0x9C
    INT Type;                        // +0xA0 (name ours) set from outside; OK gives it to the notice box's Type

    NCAnnounceWnd();                                                 // 0x1009bdd0
    // ~NCAnnounceWnd                                                // 0x1009be20

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x1009aab0
    virtual INT OnCreate();                                          // 36 0x1009be70
    virtual INT OnDestroy();                                         // 37 0x1009bff0
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x1009c0a0
    virtual INT OnKeyDown(WPARAM wParam, LPARAM lParam);             // 58 0x1009c280
    virtual INT OnOK();                                              // 67 0x1009c180 (name ours)
};
NW_ASSERT_OFFSET(NCAnnounceWnd, BackTex, 0x98);
NW_ASSERT_OFFSET(NCAnnounceWnd, OKButton, 0x9C);
NW_ASSERT_OFFSET(NCAnnounceWnd, Type, 0xA0);
NW_ASSERT_SIZE(NCAnnounceWnd, 0xA4);

#pragma pack(pop)
