#pragma once
// NCMacroEditList - the lines of the macro editor (C2 only; UClass "CMacroEditList", 0x500): a scroll window that holds
// twelve single-line edit boxes (one per line of a macro, ids 0x1506..0x1511) of which eight show at a time, with
// the line numbers painted down its left side; the slider scrolls by lines. It keeps the command parser it
// checks the lines with (ValidateLines) and the number of lines shown.
//
// Layout from the constructors 0x1008FB80 / 0x1008FCA0, OnCreate 0x1008F140 and the scroll functions.

#include "nwindow/NCScrollWnd.h"
#include "nwindow/NCommandParser.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCEditBox;

enum
{
    NMACRO_EditLines = 12,         // the line boxes (NMACRO_MaxLines)
    NMACRO_ShownLines = 8,         // shown at a time
};

class NCMacroEditList : public NCScrollWnd
{
    NW_DECLARE_CLASS(NCMacroEditList, NCScrollWnd, 0, 0x101BEFC0)

    TArray<NCEditBox*> Edits;        // +0xD0 the twelve line boxes (OnCreate)
    INT VisibleLines;                // +0xDC the lines shown (8; the constructor with an argument; 0 in the other)
    INT TopLine;                     // +0xE0 the first line shown: the slider's value
    INT TotalLines;                  // +0xE4 the slider's range (the constructor with an argument sets 0x18, UpdateSlider 12)
    NCommandParser Parser;           // +0xE8 recognizes the commands of the lines (ValidateLines)

    NCMacroEditList();                                               // 0x1008FB80
    NCMacroEditList(INT InVisibleLines);                             // 0x1008FCA0 (name of the argument ours)
    // ~NCMacroEditList                                              // 0x1008FC30

    // WindowProc (25): 0x10019810, the body of NCItemWnd::WindowProc (shared, hooked there)
    virtual INT OnCreate();                                          // 36 0x1008F140
    virtual INT OnDestroy();                                         // 37 0x1008F410
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x1008F2C0
    virtual INT ScrollUp();                                          // 67 0x1008D940
    virtual INT ScrollDown();                                        // 68 0x1008D990
    virtual INT OnScrollBar(INT Value);                              // 69 0x1008D910

    // names ours
    __declspec(noinline) INT GetFocusLine();                                              // 0x1008D830 the line whose box has the focus, -1 none
    __declspec(noinline) void LayoutLines();                                              // 0x1008D870 hides the boxes, shows the eight from TopLine
    __declspec(noinline) void UpdateSlider();                                             // 0x1008B6F0 the range and the value
    __declspec(noinline) void InitSlider();                                               // 0x1008B6B0 first setup (repeat delay 200 ms)
    __declspec(noinline) INT HitLine(INT X, INT Y);                                       // 0x1008D7D0 the line box under a point, -1 none
    __declspec(noinline) UBOOL ValidateLines();                                           // 0x1008D9F0 the lines hold valid commands, no gap
};
NW_ASSERT_OFFSET(NCMacroEditList, Edits, 0xD0);
NW_ASSERT_OFFSET(NCMacroEditList, VisibleLines, 0xDC);
NW_ASSERT_OFFSET(NCMacroEditList, TopLine, 0xE0);
NW_ASSERT_OFFSET(NCMacroEditList, Parser, 0xE8);
NW_ASSERT_SIZE(NCMacroEditList, 0x500);

#pragma pack(pop)
