#pragma once
// NCNPCHtmlViewer - the NPC dialog window (UClass "CNPCHtmlViewer", 0x9C): a title bar, the
// "NPCHtml_BACK" background stretched, and an NCHtmlViewer that closes the window when a page
// command asks for it (NWM_HTML_CLOSE).

#include "nwindow/NCWnd.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCHtmlViewer;

class NCNPCHtmlViewer : public NCWnd
{
    NW_DECLARE_CLASS(NCNPCHtmlViewer, NCWnd, 0, 0x101A95B0)

    NCHtmlViewer* HtmlViewer;        // +0x98
    UTexture* BackTex;               // +0x9C "L2UI.etcwnd.NPCHtml_BACK"

    NCNPCHtmlViewer();                                               // 0x10031020
    // ~NCNPCHtmlViewer                                              // 0x10030270

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x1002fd40
    virtual INT OnCreate();                                          // 36 0x10033650
    virtual INT OnDestroy();                                         // 37 0x10031040 (shared)
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x100310f0
    virtual INT OnClose(WPARAM wParam, LPARAM lParam);               // 67 0x1002fda0 (name ours)

    void LoadFile(const TCHAR* FileName);                            // 0x10036a10 (name ours)
    void ShowPage(TCHAR* Text);                                      // 0x100de3d0 (folded with
                                                                     //   NCTutorialViewerWnd's)
};
NW_ASSERT_OFFSET(NCNPCHtmlViewer, HtmlViewer, 0x98);
NW_ASSERT_SIZE(NCNPCHtmlViewer, 0xA0);

#pragma pack(pop)