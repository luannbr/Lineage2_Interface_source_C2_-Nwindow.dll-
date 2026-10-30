#pragma once
// NCDynamicLinker - a link of an HTML page that the layout broke across lines (UClass
// "CDynamicLinker", 0x1E4; C2 only; NCHtmlViewer::AddFileLink and AddActionLink create it, the
// stock constructor with twelve arguments): one, two or three
// NCActionLinker children cover the pieces (the first line's part, the lines between, the last
// line's part), as many as the link's height asks for; a click on any of them is passed on to the
// parent with the child's code. The geometry fields come from the constructor's arguments (the
// HTML layout's measures of the link); their meaning is only known from the placement arithmetic.

#include "nwindow/NControl.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCActionLinker;

class NCDynamicLinker : public NControl
{
    NW_DECLARE_CLASS(NCDynamicLinker, NControl, 0, 0x101AB490)

    INT Unknown1A4;                  // +0x1A4 the first argument (AddFileLink's X: the link's left); the pieces start at Unknown1B4 - Unknown1A4
    INT Unknown1A8;                  // +0x1A8 the second (its Y)
    INT Unknown1AC;                  // +0x1AC added to the first piece's width
    INT Unknown1B0;                  // +0x1B0
    INT Unknown1B4;                  // +0x1B4 the first line's indent; the pieces after it are placed at Unknown1B4 - Unknown1A4
    INT Unknown1B8;                  // +0x1B8
    INT Unknown1BC;                  // +0x1BC the right edge of a one-line link
    INT Unknown1C0;                  // +0x1C0 the link's height (it decides the number of pieces)
    INT Unknown1C4;                  // +0x1C4 the right edge of the first line (more than one piece), the width of the middle piece of three
    INT Unknown1C8;                  // +0x1C8 the width of the last piece (and of the second of two)
    INT Action;                      // +0x1CC the code every piece hands on; 0: no pieces (name ours)
    const TCHAR* Message;            // +0x1D0 the question the pieces ask first (NCActionLinker::Message), or NULL (name ours)
    NCActionLinker* Links[3];        // +0x1D4 the pieces (name ours)
    INT LinkCount;                   // +0x1E0 (name ours)

    NCDynamicLinker();                                               // 0x1003ad20 (the class constructor InternalConstructor calls)
    // the one the HTML viewer calls: the arguments in the order of the stock call, stored at +1A4, +1A8, +1AC, +1B0,
    // +1BC, +1C0, +1C4, +1C8, +1B4, +1B8, then Action and Message
    NCDynamicLinker(INT A1, INT A2, INT A3, INT A4, INT A5, INT A6, INT A7, INT A8, INT A9, INT A10,
                    INT InAction, const TCHAR* InMessage);           // 0x1003ae40 (ret 0x30)
    // ~NCDynamicLinker: the stock body is NCHtmlButton's (0x1003adf0, ICF), hooked there; its scalar deleting destructor is 0x1003adc0

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x1003a440
    virtual INT OnCreate();                                          // 36 0x1003af30
    virtual INT OnDestroy();                                         // 37 0x1003b150
    virtual INT OnLink(INT Code);                                    // 68 0x1003a4a0 (name ours) a piece was clicked: the parent is told
};
NW_ASSERT_OFFSET(NCDynamicLinker, Unknown1A4, 0x1A4);
NW_ASSERT_OFFSET(NCDynamicLinker, Unknown1C0, 0x1C0);
NW_ASSERT_OFFSET(NCDynamicLinker, Action, 0x1CC);
NW_ASSERT_OFFSET(NCDynamicLinker, Links, 0x1D4);
NW_ASSERT_OFFSET(NCDynamicLinker, LinkCount, 0x1E0);
NW_ASSERT_SIZE(NCDynamicLinker, 0x1E4);

#pragma pack(pop)
