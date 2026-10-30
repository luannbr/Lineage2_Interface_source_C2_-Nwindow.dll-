#pragma once
// NCTestHtmlViewer - the HTML test page (UClass "CTestHtmlViewer", 0xA4; the desktop's
// TestHtmlWnd, made when GIsTestHtml is set): the NPC dialog window with an edit line and a
// button under the page, which loads ..\test_html\<the name typed>. Its class registration
// names NCWnd as the super class, though the C++ base is NCNPCHtmlViewer.

#include "nwindow/NCNPCHtmlViewer.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCButton;
class NCEditBox;

class NCTestHtmlViewer : public NCNPCHtmlViewer
{
    NW_DECLARE_CLASS(NCTestHtmlViewer, NCNPCHtmlViewer, 0, 0x101A9088)

    NCEditBox* FileEdit;             // +0xA0 created hidden
    NCButton* LoadButton;            // +0xA4 a white square   // C2: offset derived, no evidence

    NCTestHtmlViewer();                                              // 0x100311d0
    // ~NCTestHtmlViewer                                             // 0x10031220

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x1002fdb0
    virtual INT OnCreate();                                          // 36 0x100337b0
    virtual INT OnLoad();                                            // 68 0x10036a30 (name ours)
};
NW_ASSERT_OFFSET(NCTestHtmlViewer, FileEdit, 0xA0);
NW_ASSERT_SIZE(NCTestHtmlViewer, 0xA8);

#pragma pack(pop)