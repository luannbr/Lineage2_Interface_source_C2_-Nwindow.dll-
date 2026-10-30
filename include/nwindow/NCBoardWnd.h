#pragma once
// NCBoardWnd - the community board (Alt+B) (UClass "CBoardWnd", 0x6B8): a title bar, a row of
// six buttons (sysstrings 377..382) that each send the bypass command the console gave them,
// and the board's page in an NCBBSHtmlViewer.
//
// NCBBSHtmlViewer - the board's page (UClass "CBBSHtmlViewer", 0x24134): an NCHtmlViewer drawn
// on "L2UI.bbs_back" and clipped inside that frame. A command the console answers with 1 locks it
// until the console clears the flag; the click sound plays even for an empty link. Its check of
// a form value lets anything of 2..3000 bytes through, bad words replaced. The console hands
// the page over in pieces (AddPiece, "keyvalue"): a page, or a post (template, fields,
// comments, thread), a write form (template, fields) or a page in three parts, assembled when
// all their pieces are in.

#include "nwindow/NCHtmlViewer.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCButton;

class NCBBSHtmlViewer : public NCHtmlViewer
{
    NW_DECLARE_CLASS(NCBBSHtmlViewer, NCHtmlViewer, 0, 0x101AA010)

    INT Unknown110;                  // +0x118 (0)
    UTexture* BoardBackTex;          // +0x11C "L2UI.bbs_back", drawn without <html imgsrc>
    INT PageKind;                    // +0x120 last page assembled (names ours): 0 a whole page,
                                     //   1 a page in parts, 2 a post, 3 a write form
    UBOOL bLocked;                   // +0x124 a command was accepted (the console answered 1)
    INT NumPostPieces;               // +0x128 (0) pieces "1".."4" received
    INT NumFormPieces;               // +0x12C (0) pieces "1001", "1002" received
    INT NumPageParts;                // +0x130 (0) pieces "101".."103" received
    TCHAR PostTemplate[0x2000];      // +0x134 "1": the post page, with <!--FIELD--> marks
    TCHAR PostFields[0x2000];        // +0x4134 "2": its fields, separated by ''
    TCHAR PostComments[0x2000];      // +0x8134 "3": for <!--COMMENT-->
    TCHAR PostThread[0x2000];        // +0xC134 "4": for <!--THREAD-->
    TCHAR FormTemplate[0x2000];      // +0x10134 "1001": the write form
    TCHAR FormFields[0x2000];        // +0x14134 "1002": its fields
    TCHAR PageParts[3][0x2000];      // +0x18134 "101", "102", "103"

    NCBBSHtmlViewer();                                               // 0x10033590
    // ~NCBBSHtmlViewer                                              // 0x10033600

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x1002fb00
    virtual INT OnCreate();                                          // 36 0x1002fb60
    virtual INT OnDestroy();                                         // 37 0x100348c0
    virtual void ShowWindow();                                       // 43 0x10030e60
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x10030d60
    virtual TCHAR* GetControlValue(const TCHAR* Name);               // 70 0x10030e70
    virtual INT OnActionLink(TCHAR* Action);                         // 72 0x1002fbe0
    virtual INT OnButton(TCHAR* Action);                             // 73 0x1002fc90
    // name ours: NWM_COMMAND 0xF0F01411 hides the viewer (nothing in NWindow sends it)
    virtual INT OnHideCommand();                                     // 77 0x100a0830 (shared)

    // names ours
    INT ShowPage(TCHAR* Text);                                       // 0x10036240
    void BuildPostPage();                                            // 0x10036250
    void BuildFormPage();                                            // 0x100364e0
    void ShowPageParts();                                            // 0x100367b0
    void AddPiece(TCHAR* Text);                                      // 0x10036840
};
NW_ASSERT_OFFSET(NCBBSHtmlViewer, bLocked, 0x124);
NW_ASSERT_OFFSET(NCBBSHtmlViewer, PostTemplate, 0x134);
NW_ASSERT_OFFSET(NCBBSHtmlViewer, FormTemplate, 0x10134);
NW_ASSERT_OFFSET(NCBBSHtmlViewer, PageParts, 0x18134);
NW_ASSERT_OFFSET(NCBBSHtmlViewer, FormFields, 0x14134);
NW_ASSERT_SIZE(NCBBSHtmlViewer, 0x24134);

class NCBoardWnd : public NCWnd
{
    NW_DECLARE_CLASS(NCBoardWnd, NCWnd, 0, 0x101A8B68)

    NCBBSHtmlViewer* HtmlViewer;     // +0x98
    UTexture* BackTex;               // +0x9C "sek.cbui354", the strip under the title bar
    NCButton* Buttons[6];            // +0xA0 NWM_COMMAND 0xF0F01422 .. 0xF0F01427
    TCHAR Commands[6][128];          // +0xB4 bypass command of each button (from the console)
    UBOOL bLocked;                   // +0x6B8 a button's command was accepted (answered 1)

    NCBoardWnd();                                                    // 0x10031270
    // ~NCBoardWnd                                                   // 0x100312c0

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x1002fe10
    virtual INT OnCreate();                                          // 36 0x10033920
    virtual INT OnDestroy();                                         // 37 0x10031310
    virtual void ShowWindow();                                       // 43 0x1008c9c0 (shared)
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x10031520
    // names ours
    virtual INT OnButton1();                                         // 67 0x1002ff00
    virtual INT OnButton2();                                         // 68 0x1002ff50
    virtual INT OnButton3();                                         // 69 0x1002ffa0
    virtual INT OnButton4();                                         // 70 0x1002fff0
    virtual INT OnButton5();                                         // 71 0x10030040
    virtual INT OnButton6();                                         // 72 0x10030090

    // names ours (called by the console's ShowBoard)
    void SetCommands(TCHAR* Command1, TCHAR* Command2, TCHAR* Command3, TCHAR* Command4,
                     TCHAR* Command5, TCHAR* Command6);              // 0x100269a0
    void LoadPage(TCHAR* Html);                                      // 0x10036a90 into HtmlViewer
};
// NW_ASSERT_OFFSET(NCBoardWnd, Commands, 0xB4);   // C2: field not mapped
NW_ASSERT_OFFSET(NCBoardWnd, bLocked, 0x6B8);
NW_ASSERT_SIZE(NCBoardWnd, 0x6BC);

#pragma pack(pop)