#pragma once
// NCTutorialViewerWnd - the tutorial window (UClass "CTutorialViewerWnd", 0xA0): the NPC dialog
// layout around an NCTutorialHtmlViewer, pages given as text.
// NCTutorialQuestionWnd - the blinking question mark that opens a tutorial page (UClass
// "CTutorialQuestionWnd", 0xA8): a click asks the server for the page of its question and
// hides it.

#include "nwindow/NCWnd.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCTutorialHtmlViewer;

class NCTutorialViewerWnd : public NCWnd
{
    NW_DECLARE_CLASS(NCTutorialViewerWnd, NCWnd, 0, 0x101E1150)

    NCTutorialHtmlViewer* HtmlViewer; // +0x98
    INT Unknown98;                   // +0x9C (0)
    UTexture* BackTex;               // +0xA0 "L2UI.etcwnd.NPCHtml_BACK"

    NCTutorialViewerWnd();                                           // 0x100de720
    // ~NCTutorialViewerWnd                                          // 0x100de770

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x1002fd40 (shared)
    virtual INT OnCreate();                                          // 36 0x100de7c0
    virtual INT OnDestroy();                                         // 37 0x100de930 (shared)
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x100de9e0
    virtual INT OnClose(WPARAM wParam, LPARAM lParam);               // 67 0x1002fda0 (shared)

    void ShowPage(TCHAR* Text);                                      // 0x100de3d0 (name ours)
};
NW_ASSERT_OFFSET(NCTutorialViewerWnd, BackTex, 0xA0);
NW_ASSERT_SIZE(NCTutorialViewerWnd, 0xA4);

class NCTutorialQuestionWnd : public NCWnd
{
    NW_DECLARE_CLASS(NCTutorialQuestionWnd, NCWnd, 0, 0x101E0C30)

    UTexture* HelpTex;               // +0x98 "L2UI.etcWnd.TutorialHelp"
    UTexture* HelpClickTex;          // +0x9C "L2UI.etcWnd.TutorialHelp_click"
    INT QuestionID;                  // +0xA0 (name ours) sent when clicked
    UBOOL bLit;                      // +0xA4 HelpTex, else HelpClickTex: the timer blinks it,
                                     //   a press holds it off
    INT TimerID;                     // +0xA8 (0xFA0D06, 500 ms)

    NCTutorialQuestionWnd();                                         // 0x100deac0
    // ~NCTutorialQuestionWnd                                        // 0x100deb20

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x10043ef0 (shared)
    virtual INT OnTimer(INT InTimerID);                              // 35 0x100de4c0
    virtual INT OnCreate();                                          // 36 0x100de3f0
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x100deb70
    virtual INT OnLButtonDown(WPARAM wParam, LPARAM lParam);         // 61 0x100de440
    virtual INT OnLButtonUp(WPARAM wParam, LPARAM lParam);           // 62 0x100de470
};
NW_ASSERT_OFFSET(NCTutorialQuestionWnd, QuestionID, 0xA0);
NW_ASSERT_SIZE(NCTutorialQuestionWnd, 0xAC);

#pragma pack(pop)