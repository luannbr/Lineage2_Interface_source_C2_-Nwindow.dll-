#pragma once
// NHtmlAnalyzer - an empty scroll window (UClass "HtmlAnalyzer", 0x140): four point deques
// (like NCGDevice's origin stack) and a few fields no code of NWindow reads. Its paint only
// runs the base one; nothing in NWindow creates it.

#include "nwindow/NCScrollWnd.h"
#include "nwindow/NStdLib.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NHtmlAnalyzer : public NCScrollWnd
{
    NW_DECLARE_CLASS(NHtmlAnalyzer, NCScrollWnd, 0, 0x101A77E8)

    INT UnknownCC;                   // +0xD0 (0)
    INT UnknownD0;                   // +0xD4 (0)
    INT UnknownD4;                   // +0xD8 (0)
    NPointDeque Points1;             // +0xDC
    NPointDeque Points2;             // +0xF0
    INT Unknown100;                  // +0x104 (0)
    INT Unknown104;                  // +0x108 (0)
    INT Unknown108;                  // +0x10C (5)
    NPointDeque Points3;             // +0x110
    NPointDeque Points4;             // +0x124
    INT Unknown134[3];               // +0x138 never set   // C2: offset derived, no evidence

    NHtmlAnalyzer();                                                 // 0x1002ced0
    // ~NHtmlAnalyzer                                                // 0x1002cfa0

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x10019810 (shared)
    virtual INT OnCreate();                                          // 36 0x1002ce20 (shared)
    virtual INT OnDestroy();                                         // 37 0x1002ce30 (shared)
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x1002ce40
};
NW_ASSERT_OFFSET(NHtmlAnalyzer, Points1, 0xDC);
NW_ASSERT_OFFSET(NHtmlAnalyzer, Unknown108, 0x10C);
NW_ASSERT_OFFSET(NHtmlAnalyzer, Points4, 0x124);
NW_ASSERT_SIZE(NHtmlAnalyzer, 0x144);

#pragma pack(pop)