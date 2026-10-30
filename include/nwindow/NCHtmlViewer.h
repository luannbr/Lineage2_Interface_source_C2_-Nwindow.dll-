#pragma once
// NCHtmlViewer - shows an HTML page (UClass "CHtmlViewer", 0x110): the NPC dialogs, the
// tutorial, the board. The page is parsed into a tree (NHtmlLayout.h) that is laid out at the
// viewer's width and drawn scrolled; links and form fields become child controls (NCFileLinker,
// NCActionLinker, NCHtmlButton, NCHtmlEdit, NCHtmlMultiEdit, NCHtmlComboBox) that move with the
// page and report back through NWM_COMMAND with the viewer's ids. A command goes to the
// console (bypass to the server, $name replaced by the value of the named control); when it
// asks for it (-h) the owner window, or else the viewer, closes.
//
// NCTutorialHtmlViewer - the same for the tutorial window (UClass "CTutorialHtmlViewer").

#include "nwindow/NCScrollWnd.h"
#include "nwindow/NClasses.gen.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NControl;
class NHtmlBlock;
struct NHtmlTagList;

// sent to the owner window when a command asks to close (name ours)
enum { NWM_HTML_CLOSE = (INT)0xF0000004 };

class NCHtmlViewer : public NCScrollWnd
{
    NW_DECLARE_CLASS(NCHtmlViewer, NCScrollWnd, 0, 0x101A9AD8)

    NHtmlBlock* Doc;                 // +0xD0 the page, NULL while none is loaded
    UTexture* BackTex;               // +0xD4 <html imgsrc>, NULL = transparent
    INT FileLinkID;                  // +0xD8 NWM_COMMAND ids of the controls it creates
    INT ActionLinkID;                // +0xDC   (0xFF00FF00 .. 0xFF00FF05)
    INT ButtonID;                    // +0xE0
    INT EditID;                      // +0xE4
    INT MultiEditID;                 // +0xE8
    INT ComboBoxID;                  // +0xEC
    INT FontType;                    // +0xF0 new in C2 (name ours): UCanvas::L2Fonts index the page is
                                     //   drawn in (0; the credits' viewer passes its own): handed to every NHtmlBlock
    TArray<NControl*> Controls;      // +0xF4 the page's controls, in page order
    UBOOL bLoaded;                   // +0x100 Doc holds a page
    INT UnknownFC;                   // +0x104 (0)
    INT Unknown100;                  // +0x108 (0)
    INT CreateWidth;                 // +0x10C size at OnCreate
    INT CreateHeight;                // +0x110
    NWndEntry* Owner;                // +0x114 window a closing command goes to, NULL = itself

    NCHtmlViewer();                                                  // 0x10033070
    NCHtmlViewer(INT InFontType);                                    // 0x100331a0 (only the credit window uses it)
    // ~NCHtmlViewer                                                 // 0x10032640

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x1002f530
    virtual INT OnCreate();                                          // 36 0x1002f630
    virtual INT OnDestroy();                                         // 37 0x10034730
    virtual void ShowWindow();                                       // 43 0x10030cd0
    virtual INT OnKeyDown(WPARAM wParam, LPARAM lParam);             // 58 0x1002f9c0
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x10031cb0
    virtual INT OnLButtonDown(WPARAM wParam, LPARAM lParam);         // 61 0x1002f990
    virtual INT OnMouseWheel(WPARAM wParam, LPARAM lParam);          // 66 0x10030af0
    virtual INT ScrollUp();                                          // 67 0x10030a80
    virtual INT ScrollDown();                                        // 68 0x10030aa0
    virtual INT OnScrollBar(INT Value);                              // 69 0x10030ac0
    // names ours
    virtual TCHAR* GetControlValue(const TCHAR* Name);               // 70 0x10030b20 checked
    virtual INT OnFileLink(TCHAR* File);                             // 71 0x10036200
    virtual INT OnActionLink(TCHAR* Action);                         // 72 0x1002f6d0
    virtual INT OnButton(TCHAR* Action);                             // 73 0x1002f750
    virtual INT OnEdit(LPARAM Param);                                // 74 0x1002f7c0
    virtual INT OnMultiEdit(LPARAM Param);                           // 75 0x1002f880
    virtual INT OnComboBox(LPARAM Param);                            // 76 0x1002f940

    // names ours
    void SetBackTexture(const TCHAR* Name);                          // 0x1002f660
    void SetOwner(NWndEntry* InOwner);                               // 0x1002f6a0
    UTexture* FindPageTexture(const TCHAR* Name);                    // 0x1002f6b0
    void ScrollControls(INT DX, INT DY);                             // 0x10030a00
    // C2: the link control (an NCDynamicLinker) is told where the item and its parent are, and
    // the width the item was laid out at and how far its last line reaches (NHtmlItem::Arrange)
    INT AddFileLink(INT AbsX, INT AbsY, INT X, INT Y, INT W, INT H, INT LayoutWidth,
                    INT TailWidth, INT ParentAbsX, INT ParentAbsY, TCHAR* File);   // 0x10031e00
    INT AddActionLink(INT AbsX, INT AbsY, INT X, INT Y, INT W, INT H, INT LayoutWidth,
                      INT TailWidth, INT ParentAbsX, INT ParentAbsY, TCHAR* Action,
                      TCHAR* Msg);                                          // 0x10031f20
    INT AddButton(INT X, INT Y, INT W, INT H, TCHAR* NormalTexName, TCHAR* PushedTexName,
                  TCHAR* Action, TCHAR* Label);                      // 0x10028380
    INT AddMultiEdit(INT X, INT Y, INT W, INT H, TCHAR* Var);        // 0x100321a0
    INT AddEdit(INT X, INT Y, INT W, INT H, TCHAR* Var, INT Type, TCHAR* Action,
                INT Length);                                         // 0x10032290
    INT AddComboBox(INT X, INT Y, INT W, INT H, TCHAR* Var, TCHAR* List, TCHAR* Sel); // 0x100332d0
    void Clear();                                                    // 0x100333e0
    void BuildDocument(NHtmlTagList* Tags);                          // 0x10035e90
    INT LoadHtmlFile(TCHAR* Source);                                 // 0x10036040
    INT LoadHtml(TCHAR* Text);                                       // 0x10036130
};
NW_ASSERT_OFFSET(NCHtmlViewer, Doc, 0xD0);
NW_ASSERT_OFFSET(NCHtmlViewer, Controls, 0xF4);
NW_ASSERT_OFFSET(NCHtmlViewer, Owner, 0x114);
NW_ASSERT_SIZE(NCHtmlViewer, 0x118);

class NCTutorialHtmlViewer : public NCHtmlViewer
{
    NW_DECLARE_CLASS(NCTutorialHtmlViewer, NCHtmlViewer, 0, 0x101AA530)

    NCTutorialHtmlViewer();                                          // 0x100334f0
    // ~NCTutorialHtmlViewer                                         // 0x10033540

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x1002fad0
    virtual INT OnCreate();                                          // 36 0x1002faf0
    virtual INT OnDestroy();                                         // 37 0x10034840 (shared)
};
NW_ASSERT_SIZE(NCTutorialHtmlViewer, 0x118);

// The windows the controls of a page clip to (the loop is inlined in each control's OnPaint in
// the stock code; names ours): the enclosing HTML viewer, or the first ancestor that is not a
// control. NULL at the top of the tree (no clip is set then).
inline NCWnd* EnclosingHtmlViewer(NCWnd* Wnd)
{
    NWndEntry* E = Wnd->Entry;
    for (;;)
    {
        NCWnd* W = E->Parent->Wnd;
        if (!W || W->IsA(NCHtmlViewer::StaticClass()))
            return W;
        E = W->Entry;
    }
}
inline NCWnd* FirstNonControlParent(NCWnd* Wnd)
{
    NWndEntry* E = Wnd->Entry;
    for (;;)
    {
        NCWnd* W = E->Parent->Wnd;
        if (!W || !W->IsA(NClass::NControl()))
            return W;
        E = W->Entry;
    }
}

#pragma pack(pop)