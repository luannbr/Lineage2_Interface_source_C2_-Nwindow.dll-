#pragma once
// NCHtmlButton / NCHtmlPushButton - the buttons of the HTML pages (UClass "CHtmlButton" 0x220,
// "CHtmlPushButton" 0x228). They paint like NCButton but clip to the window they live in, so a
// page scrolled under the viewer's edge does not show them: NCHtmlButton to the enclosing
// NCHtmlViewer, NCHtmlPushButton to its first ancestor that is not a control (the scroll
// buttons of an HTML edit box). A click on an NCHtmlButton reports Command to the viewer.

#include "nwindow/NCButton.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCHtmlButton : public NCButton
{
    NW_DECLARE_CLASS(NCHtmlButton, NCButton, 0, 0x101AD8A8)

    const TCHAR* Command;      // +0x264 (name ours) sent on a click (NWM_COMMAND lParam): the
                               //   <button> action

    NCHtmlButton();                                                             // 0x1003de60
    NCHtmlButton(const TCHAR* NormalTexName, const TCHAR* PushedTexName, const TCHAR* InCommand,
                 UBOOL bInAcceptKey);                                           // 0x100308e0
    NCHtmlButton(const TCHAR* NormalTexName, const TCHAR* PushedTexName);       // 0x10030910 (C1 only: no C2 match)
    // ~NCHtmlButton                                                            // 0x1003deb0

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);             // 25 0x1003a790 (shared)
    virtual INT OnPaint(NCGDevice* Device);                                     // 55 0x1003b200
    virtual INT OnLButtonUp(WPARAM wParam, LPARAM lParam);                      // 62 0x1003a4c0
};
NW_ASSERT_OFFSET(NCHtmlButton, Command, 0x264);
NW_ASSERT_SIZE(NCHtmlButton, 0x268);

class NCHtmlPushButton : public NCPushButton
{
    NW_DECLARE_CLASS(NCHtmlPushButton, NCPushButton, 0, 0x101AC938)

    NCHtmlPushButton();                                                         // 0x1003df60
    NCHtmlPushButton(const TCHAR* NormalTexName, const TCHAR* PushedTexName);   // 0x10030cc0 (C1 only: no C2 match)
    // ~NCHtmlPushButton                                                        // 0x1003dfb0

    virtual INT OnPaint(NCGDevice* Device);                                     // 55 0x1003b540
};
NW_ASSERT_SIZE(NCHtmlPushButton, 0x270);

#pragma pack(pop)