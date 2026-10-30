#pragma once
// NCNoticeWnd - the announcements box of the login screen (UClass "CNoticeWnd", 0xA4;
// NCLoginWnd +0x98): the text the login server sends, scrolling in an NCNoticeInfoWnd, and
// OK (or Enter), which goes on to the warning box when the notice says so (Type bit 0) or
// else asks for the server list.

#include "nwindow/NCScrollWnd.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCButton;

// NCNoticeInfoWnd - the scrolling text of the box (UClass "CNoticeInfoWnd", 0x200CC)
class NCNoticeInfoWnd : public NCScrollWnd
{
    NW_DECLARE_CLASS(NCNoticeInfoWnd, NCScrollWnd, 0, 0x101C6630)

    TCHAR Text[65536];               // +0xD0

    NCNoticeInfoWnd();                                               // 0x1009ab10
    // ~NCNoticeInfoWnd                                              // 0x1009ab70

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x10019810 (folded)
    virtual INT OnCreate();                                          // 36 0x1002ce20
    virtual INT OnDestroy();                                         // 37 0x1009a700
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x1009abc0

    // names ours
    void SetText(const TCHAR* InText);                               // 0x1009a780
    void ClearText();                                                // 0x1009a7b0
};
NW_ASSERT_OFFSET(NCNoticeInfoWnd, Text, 0xD0);
NW_ASSERT_SIZE(NCNoticeInfoWnd, 0x200D0);

class NCNoticeWnd : public NCWnd
{
    NW_DECLARE_CLASS(NCNoticeWnd, NCWnd, 0, 0x101C56B0)

    INT Type;                        // +0x98 bit 0: the warning box comes next
    UTexture* BackTex;               // +0x9C "L2UI.etcWnd.Announce_back"
    NCNoticeInfoWnd* InfoWnd;        // +0xA0
    NCButton* OKButton;              // +0xA4

    NCNoticeWnd();                                                   // 0x1009acf0
    // ~NCNoticeWnd                                                  // 0x1009ad40

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x1009a7c0
    virtual INT OnCreate();                                          // 36 0x1009ad90
    virtual INT OnDestroy();                                         // 37 0x1009afa0
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x1009b070
    virtual INT OnKeyDown(WPARAM wParam, LPARAM lParam);             // 58 0x1009b190
    virtual INT OnOK();                                              // 67 0x1009b150 (name ours)

    // names ours
    void SetText(const TCHAR* InText);                               // 0x1009a820
    void ClearText();                                                // 0x1009a840
};
NW_ASSERT_OFFSET(NCNoticeWnd, BackTex, 0x9C);
NW_ASSERT_OFFSET(NCNoticeWnd, OKButton, 0xA4);
NW_ASSERT_SIZE(NCNoticeWnd, 0xA8);

#pragma pack(pop)