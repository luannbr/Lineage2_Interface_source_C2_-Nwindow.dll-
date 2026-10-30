/*=============================================================================
    NCHtmlViewer.cpp - the HTML viewer and NCTutorialHtmlViewer. Stock: 0x10025f50 ..
    0x1002be63, interleaved with the page tree (NHtmlLayout.cpp) and the board viewers.
=============================================================================*/
#include "nwindow/NCHtmlViewer.h"
#include "nwindow/NHtmlLayout.h"
#include "nwindow/NCActionLinker.h"
#include "nwindow/NCConsole.h"
#include "nwindow/NCCreditWnd.h"
#include "nwindow/NCDialogBox.h"
#include "nwindow/NCDynamicLinker.h"
#include "nwindow/NCFileLinker.h"
#include "nwindow/NCGaraDialogBox.h"
#include "nwindow/NCHtmlButton.h"
#include "nwindow/NCHtmlComboBox.h"
#include "nwindow/NCHtmlEdit.h"
#include "nwindow/NCMultiEdit.h"
#include "nwindow/NCGDevice.h"
#include "nwindow/NCSliderBar.h"
#include "nwindow/NConsoleWnd.h"
#include "nwindow/NCrt.h"

#include <new>
#include <string.h>
#include <wchar.h>

// NCHtmlViewer::FileLinkID .. ComboBoxID
enum { NID_HTML_FIRST = (INT)0xFF00FF00 };

// the code a click in an edit box sends: MAKELONG(100, its y) (NCHtmlEdit::OnLButtonDown)
enum { HTML_SCROLLTO = 100 };

// style bit 0x200 of the page controls (meaning unknown); C2's link controls also set 0x4 (name ours)
enum { NWS_HTMLCONTROL = 0x200, NWS_LINK = 0x4 };

// dialog of NCActionLinker's question (NCGaraDialogBox::DialogID)
enum { DIALOG_ActionLink = 0x3C };   // C2 (C1: 0x38)

// interface sounds (FL2NetNotify::OnInterfacePlaySound)
enum { SOUND_Click = 0 };

// systemmsg lines (names ours)
enum
{
    MSG_IncorrectName = 204,           // "Incorrect name. Please try again. "
    MSG_TooLong3000 = 329,             // "Please input contents using less than 3000 characters."
};

// <html imgsrc> absent: no background. The stock code tests this pointer, not the text.
static const TCHAR HtmlTransparent[] = TEXT("Transparent");

// a new page control is shown only when its top is on the page's visible part (repeated in
// every Add* of the stock code)
static FORCEINLINE void ShowIfVisible(NCHtmlViewer* Viewer, NControl* Control)
{
    if (Control->LocalY <= Viewer->Height && Control->LocalY >= 0)
        Control->ShowWindow();
    else
        Control->HideWindow();
}

/*-----------------------------------------------------------------------------
    NCHtmlViewer.
-----------------------------------------------------------------------------*/

// 0x10033070 - the alignment stacks of the parser get their bottom 0
NCHtmlViewer::NCHtmlViewer()
{
    BackTex = NULL;
    Unknown100 = 0;
    UnknownFC = 0;
    CreateWidth = 0;
    CreateHeight = 0;
    if (GHtmlAlignStack.Num() <= 0)
        GHtmlAlignStack.AddItem(0);
    if (GHtmlVAlignStack.Num() <= 0)
        GHtmlVAlignStack.AddItem(0);
    bLoaded = 0;
    Unknown100 = 0;
    UnknownFC = 0;
    Owner = NULL;
    Doc = NULL;
    FontType = 0;   // C2
    FileLinkID = NID_HTML_FIRST;
    ActionLinkID = NID_HTML_FIRST + 1;
    ButtonID = NID_HTML_FIRST + 2;
    EditID = NID_HTML_FIRST + 3;
    MultiEditID = NID_HTML_FIRST + 4;
    ComboBoxID = NID_HTML_FIRST + 5;
    Unknown0C8 = 16;
}
NW_HOOK_CTOR(0x10033070, NCHtmlViewer)

// 0x100331a0 - the same with the font the page is drawn in (the credit window's viewer)
NCHtmlViewer::NCHtmlViewer(INT InFontType)
{
    BackTex = NULL;
    Unknown100 = 0;
    UnknownFC = 0;
    CreateWidth = 0;
    CreateHeight = 0;
    if (GHtmlAlignStack.Num() <= 0)
        GHtmlAlignStack.AddItem(0);
    if (GHtmlVAlignStack.Num() <= 0)
        GHtmlVAlignStack.AddItem(0);
    bLoaded = 0;
    Unknown100 = 0;
    UnknownFC = 0;
    Owner = NULL;
    Doc = NULL;
    FontType = InFontType;
    FileLinkID = NID_HTML_FIRST;
    ActionLinkID = NID_HTML_FIRST + 1;
    ButtonID = NID_HTML_FIRST + 2;
    EditID = NID_HTML_FIRST + 3;
    MultiEditID = NID_HTML_FIRST + 4;
    ComboBoxID = NID_HTML_FIRST + 5;
    Unknown0C8 = 16;
}
NW_HOOK_CTOR_ARGS(0x100331a0, NCHtmlViewer, (INT A), (A))

// 0x10032640
NW_HOOK_DTOR(0x10032640, NCHtmlViewer)

// 0x1002f530 (vslot 25) - each kind of page control has its id
INT NCHtmlViewer::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (NCScrollWnd::WindowProc(Msg, wParam, lParam))
        return 1;
    if (Msg == NWM_COMMAND)
    {
        if (FileLinkID == (INT)wParam)
            return OnFileLink((TCHAR*)lParam) != 0;
        if (ActionLinkID == (INT)wParam)
            return OnActionLink((TCHAR*)lParam) != 0;
        if (ButtonID == (INT)wParam)
            return OnButton((TCHAR*)lParam) != 0;
        if (EditID == (INT)wParam)
            return OnEdit(lParam) != 0;
        if (MultiEditID == (INT)wParam)
            return OnMultiEdit(lParam) != 0;
        if (ComboBoxID == (INT)wParam)
            return OnComboBox(lParam) != 0;
    }
    return 0;
}
NW_HOOK_METHOD(0x1002f530, NCHtmlViewer, WindowProc)

// 0x1002f630 (vslot 36)
INT NCHtmlViewer::OnCreate()
{
    NCScrollWnd::OnCreate();
    CreateHeight = Height;
    CreateWidth = Width;
    return 1;
}
NW_HOOK_METHOD(0x1002f630, NCHtmlViewer, OnCreate)

// 0x10034730 (vslot 37) - the page and its controls go
INT NCHtmlViewer::OnDestroy()
{
    guard(NCHtmlViewer::OnDestroy);
    if (bLoaded && Doc)
    {
        Doc->Release();
        if (Doc)
            delete Doc;
        Doc = NULL;
    }
    INT N = Controls.Num();
    for (INT i = 0; i < N; i++)
    {
        NControl* Control = Controls(i);
        if (Control->Entry && Control)
        {
            RemoveChildWindow(Control->Entry);
            delete Control;
        }
    }
    Controls.Empty();
    NCScrollWnd::OnDestroy();
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x10034730, NCHtmlViewer, OnDestroy)

// 0x10030cd0 (vslot 43) - the controls below the visible part stay hidden
void NCHtmlViewer::ShowWindow()
{
    NCWnd::ShowWindow();
    INT N = Controls.Num();
    for (INT i = 0; i < N; i++)
    {
        NControl* Control = Controls(i);
        if (Height + ScrollY < Control->LocalY)
            Control->HideWindow();
        else
            Control->ShowWindow();
    }
}
NW_HOOK_METHOD(0x10030cd0, NCHtmlViewer, ShowWindow)

// 0x10031cb0 (vslot 55) - background (the scroll bar's width of the texture left out while the
// bar shows), the page shifted by the scroll position, whose height sets the scroll range. The
// clip goes on before BeginPaint.
INT NCHtmlViewer::OnPaint(NCGDevice* Device)
{
    guard(NCHtmlViewer::OnPaint);
    Device->SetClip(ScreenX, ScreenY, Width, Height);
    Device->BeginPaint(ScreenX, ScreenY);
    if (BackTex)
    {
        INT SourceW = Width;
        if (ScrollUpButton->IsShowWindow())
            SourceW = Width - Unknown0C8;
        Device->DrawTexture(0, 0, Width, Height, 0, 0, SourceW, Height, BackTex);
    }
    Device->PushOrigin(ScrollX, ScrollY);
    if (bLoaded)
    {
        if (Doc)
            Doc->Draw(Device);
        if (bLoaded && Doc)
            SetContentHeight(Doc->GetContentHeight());
    }
    Device->PopOrigin();
    Device->EndPaint();
    Device->ReleaseClip();
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x10031cb0, NCHtmlViewer, OnPaint)

// 0x1002f990 (vslot 61) - a click on the credits page reaches the credit window (its handler is
// the NewUI group's: stock call)
INT NCHtmlViewer::OnLButtonDown(WPARAM wParam, LPARAM lParam)
{
    NCWnd* Credit = GConsole->CreditWnd;
    if (Entry->Parent->Wnd == Credit)
        ((NCCreditWnd*)Credit)->NextPage();   // 0x1006E360
    return 1;
}
NW_HOOK_METHOD(0x1002f990, NCHtmlViewer, OnLButtonDown)

// 0x1002f9c0 (vslot 58) - Escape on the credits page closes the credit window and goes back to the
// login screen: the login window and the message line show, the credit window hides and loses
// its focus, the audio subsystem is told (slot 44 with 0), the credit window cleans up
// (0x1006E020, the NewUI group's). Every key then goes to NCWnd's handler (0x1002b980: not
// NCScrollWnd's, so the page does not scroll from the keyboard).
INT NCHtmlViewer::OnKeyDown(WPARAM wParam, LPARAM lParam)
{
    NCWnd* Credit = GConsole->CreditWnd;
    if (Entry->Parent->Wnd == Credit && wParam == VK_ESCAPE && GConsole->LoginWnd &&
        GConsole->MessageWnd && Credit && GConsole->Engine && GConsole->Engine->Audio)
    {
        ((NCWnd*)GConsole->LoginWnd)->ShowWindow();
        ((NCWnd*)GConsole->MessageWnd)->ShowWindow();
        GConsole->CreditWnd->HideWindow();
        nw::VirtualCall<void>(GConsole->CreditWnd, 30);
        nw::Field<INT>(GConsole->CreditWnd, 0x1B0) = 0;
        nw::Field<INT>(GConsole->CreditWnd, 0x1AC) = 0;
        nw::VirtualCall<void>(GConsole->Engine->Audio, 44, 0);
        ((NCCreditWnd*)GConsole->CreditWnd)->FreeFont();   // 0x1006E020
    }
    NCWnd::OnKeyDown(wParam, lParam);
    return 1;
}
NW_HOOK_METHOD(0x1002f9c0, NCHtmlViewer, OnKeyDown)

// 0x10030af0 (vslot 66) - the page's controls follow every scroll
INT NCHtmlViewer::OnMouseWheel(WPARAM wParam, LPARAM lParam)
{
    NCScrollWnd::OnMouseWheel(wParam, lParam);
    ScrollControls(0, DeltaY);
    return 1;
}
NW_HOOK_METHOD(0x10030af0, NCHtmlViewer, OnMouseWheel)

// 0x10030a80 (vslot 67)
INT NCHtmlViewer::ScrollUp()
{
    NCScrollWnd::ScrollUp();
    ScrollControls(0, DeltaY);
    return 1;
}
NW_HOOK_METHOD(0x10030a80, NCHtmlViewer, ScrollUp)

// 0x10030aa0 (vslot 68)
INT NCHtmlViewer::ScrollDown()
{
    NCScrollWnd::ScrollDown();
    ScrollControls(0, DeltaY);
    return 1;
}
NW_HOOK_METHOD(0x10030aa0, NCHtmlViewer, ScrollDown)

// 0x10030ac0 (vslot 69)
INT NCHtmlViewer::OnScrollBar(INT Value)
{
    NCScrollWnd::OnScrollBar(Value);
    ScrollControls(0, DeltaY);
    return 1;
}
NW_HOOK_METHOD(0x10030ac0, NCHtmlViewer, OnScrollBar)

// 0x10030b20 (vslot 70) - the value of the control whose caption (variable) is Name, for a
// $Name in a command. A one-line box must hold a name: 1 to 16 bytes in the ANSI code page,
// characters the font has and no '&'; a multi-line box 2 to 3000 bytes. Anything else
// shows a system message and gives NULL.
TCHAR* NCHtmlViewer::GetControlValue(const TCHAR* Name)
{
    ANSICHAR Ansi[512];
    if (!Name)
        return NULL;
    INT N = Controls.Num();
    for (INT i = 0; i < N; i++)
    {
        NControl* Control = Controls(i);
        const TCHAR* Caption = Control->GetCaption();
        if (!Caption || wcscmp(Caption, Name))
            continue;

        if (!Control->GetValue())
            return NULL;
        if (!Controls(i)->IsA(NClass::NCHtmlMultiEdit()))
        {
            if (GConsole->HasInvalidChars((TCHAR*)Control->GetValue()) ||
                !GConsole->HasNoAmpersand((TCHAR*)Control->GetValue()))
            {
                GConsole->ShowSystemMessage(MSG_IncorrectName, 0xFFB09B79);
                return NULL;
            }
        }
        WideCharToMultiByte(CP_ACP, 0, Control->GetValue(), -1, Ansi, 512, NULL, NULL);
        INT Length = (INT)strlen(Ansi);
        if (Controls(i)->IsA(NClass::NCHtmlMultiEdit()))
        {
            if (Length <= 1 || Length > 3000)
            {
                GConsole->ShowSystemMessage(MSG_TooLong3000, 0xFFB09B79);
                return NULL;
            }
        }
        if (!Controls(i)->IsA(NClass::NCHtmlMultiEdit()) && (Length <= 0 || Length > 16))
        {
            GConsole->ShowSystemMessage(MSG_IncorrectName, 0xFFB09B79);
            return NULL;
        }
        return (TCHAR*)Control->GetValue();
    }
    return NULL;
}
NW_HOOK_METHOD(0x10030b20, NCHtmlViewer, GetControlValue)

// 0x10036200 (vslot 71) - <a link>: another page from a file
INT NCHtmlViewer::OnFileLink(TCHAR* File)
{
    wcscpy(GHtmlLinkTarget, File);
    if (GHtmlLinkTarget[0])
        LoadHtmlFile(GHtmlLinkTarget);
    return 1;
}
NW_HOOK_METHOD(0x10036200, NCHtmlViewer, OnFileLink)

// 0x1002f6d0 (vslot 72) - <a action>: the command, a click sound
INT NCHtmlViewer::OnActionLink(TCHAR* Action)
{
    wcscpy(GHtmlLinkTarget, Action);
    if (GHtmlLinkTarget[0])
    {
        if (GConsole->RunHtmlCommand(GHtmlLinkTarget, this) == 2)
        {
            if (Owner)
                Owner->Wnd->WindowProc(NWM_HTML_CLOSE, 0, 0);
            else
                HideWindow();
        }
        PlayInterfaceSound(SOUND_Click);
    }
    return 1;
}
NW_HOOK_METHOD(0x1002f6d0, NCHtmlViewer, OnActionLink)

// 0x1002f750 (vslot 73) - <button action> (no emptiness test, no sound)
INT NCHtmlViewer::OnButton(TCHAR* Action)
{
    wcscpy(GHtmlLinkTarget, Action);
    if (GConsole->RunHtmlCommand(GHtmlLinkTarget, this) == 2)
    {
        if (Owner)
        {
            Owner->Wnd->WindowProc(NWM_HTML_CLOSE, 0, 0);
            return 1;
        }
        HideWindow();
    }
    return 1;
}
NW_HOOK_METHOD(0x1002f750, NCHtmlViewer, OnButton)

// 0x1002f7c0 (vslot 74) - from an edit box: a click scrolls it into view when it is above
// the visible part; Enter runs its action
INT NCHtmlViewer::OnEdit(LPARAM Param)
{
    if ((SWORD)LOWORD(Param) == HTML_SCROLLTO)
    {
        INT EditY = (SWORD)HIWORD(Param);
        if (EditY < 0)
        {
            OnScrollBar(EditY - ScrollY);
            ScrollBar->SetScrollPos(-ScrollY);
        }
        return 1;
    }
    TCHAR* Action = (TCHAR*)Param;
    if (Action && *Action)
    {
        wcscpy(GHtmlLinkTarget, Action);
        if (GConsole->RunHtmlCommand(GHtmlLinkTarget, this) == 2)
        {
            if (Owner)
            {
                Owner->Wnd->WindowProc(NWM_HTML_CLOSE, 0, 0);
                return 1;
            }
            HideWindow();
        }
    }
    return 1;
}
NW_HOOK_METHOD(0x1002f7c0, NCHtmlViewer, OnEdit)

// 0x1002f880 (vslot 75) - the same for a multi-line box (an empty action runs too)
INT NCHtmlViewer::OnMultiEdit(LPARAM Param)
{
    if ((SWORD)LOWORD(Param) == HTML_SCROLLTO)
    {
        INT EditY = (SWORD)HIWORD(Param);
        if (EditY < 0)
        {
            OnScrollBar(EditY - ScrollY);
            ScrollBar->SetScrollPos(-ScrollY);
        }
        return 1;
    }
    TCHAR* Action = (TCHAR*)Param;
    if (Action)
    {
        wcscpy(GHtmlLinkTarget, Action);
        if (GConsole->RunHtmlCommand(GHtmlLinkTarget, this) == 2)
        {
            if (Owner)
            {
                Owner->Wnd->WindowProc(NWM_HTML_CLOSE, 0, 0);
                return 1;
            }
            HideWindow();
        }
    }
    return 1;
}
NW_HOOK_METHOD(0x1002f880, NCHtmlViewer, OnMultiEdit)

// 0x1002f940 (vslot 76) - from a combo box: only the scroll into view
INT NCHtmlViewer::OnComboBox(LPARAM Param)
{
    INT BoxY = (SWORD)HIWORD(Param);
    if (LOWORD(Param) == HTML_SCROLLTO && BoxY < 0)
    {
        OnScrollBar(BoxY - ScrollY);
        ScrollBar->SetScrollPos(-ScrollY);
    }
    return 1;
}
NW_HOOK_METHOD(0x1002f940, NCHtmlViewer, OnComboBox)

// 0x1002f660 - NULL leaves the old one
void NCHtmlViewer::SetBackTexture(const TCHAR* Name)
{
    if (Name)
    {
        if (Name == HtmlTransparent)
            BackTex = NULL;
        else
            BackTex = FindTexture(Name);
    }
}
NW_HOOK_METHOD(0x1002f660, NCHtmlViewer, SetBackTexture)

// 0x1002f6a0 - the window a closing command goes to. One stock body for the setters of the +0x114 field of the grids
// (NCManorShopWnd, NCPrivateBuyWnd, NCPrivateShopWnd, NCRecipeShopWnd, NCShopWnd, NCStoreWnd: their SetMode): this class
// keeps the address
void NCHtmlViewer::SetOwner(NWndEntry* InOwner)
{
    Owner = InOwner;
}
NW_HOOK_METHOD(0x1002f6a0, NCHtmlViewer, SetOwner)

// 0x1002f6b0 - <img src>; no name is the black texture
UTexture* NCHtmlViewer::FindPageTexture(const TCHAR* Name)
{
    if (!Name)
        Name = TEXT("NWindow.BlackTexture");
    return FindTexture(Name);
}
NW_HOOK_METHOD(0x1002f6b0, NCHtmlViewer, FindPageTexture)

// 0x10030a00 - the controls moved with the page, shown while any of them is visible
void NCHtmlViewer::ScrollControls(INT DX, INT DY)
{
    INT N = Controls.Num();
    for (INT i = 0; i < N; i++)
    {
        Controls(i)->MoveWindow(DX, -DY);
        NControl* Control = Controls(i);
        if (Control->LocalY <= Height && Control->LocalY + Control->LocalH >= 0)
            Control->ShowWindow();
        else
            Control->HideWindow();
    }
}
NW_HOOK_METHOD(0x10030a00, NCHtmlViewer, ScrollControls)

// The link control of C2 (an NCDynamicLinker, 0x1E4 bytes) takes the item's whole geometry; its
// twelve-argument constructor is 0x1003AE40. The last argument is the message of an action link
// (the constructor keeps both pointers: the code is the string the page gave).
static FORCEINLINE NControl* NewDynamicLinker(INT AbsX, INT AbsY, INT X, INT Y, INT W, INT H, INT LayoutWidth,
                                  INT TailWidth, INT ParentAbsX, INT ParentAbsY, TCHAR* Target,
                                  TCHAR* Msg)
{
    return new NCDynamicLinker(AbsX, AbsY, X, Y, W, H, LayoutWidth, TailWidth, ParentAbsX, ParentAbsY,
                               (INT)(INT_PTR)Target, Msg);
}

// 0x10031e00 - <a link>: a control over the item that hands the page to load to the viewer
INT NCHtmlViewer::AddFileLink(INT AbsX, INT AbsY, INT X, INT Y, INT W, INT H, INT LayoutWidth,
                              INT TailWidth, INT ParentAbsX, INT ParentAbsY, TCHAR* File)
{
    NControl* Link = NewDynamicLinker(AbsX, AbsY, X, Y, W, H, LayoutWidth, TailWidth, ParentAbsX,
                                      ParentAbsY, File, NULL);
    Link->ID = FileLinkID;
    Link->SetWindowRect(AbsX, AbsY, 0, 0);
    CreateChild(Link, NWS_HTMLCONTROL | NWS_VISIBLE | NWS_LINK);
    Controls.AddItem(Link);
    ShowIfVisible(this, Link);
    return 1;
}
NW_HOOK_METHOD(0x10031e00, NCHtmlViewer, AddFileLink)

// 0x10031f20 - <a action msg>: the same for a command (msg is a systemmsg line asked first)
INT NCHtmlViewer::AddActionLink(INT AbsX, INT AbsY, INT X, INT Y, INT W, INT H, INT LayoutWidth,
                                INT TailWidth, INT ParentAbsX, INT ParentAbsY, TCHAR* Action,
                                TCHAR* Msg)
{
    NControl* Link = NewDynamicLinker(AbsX, AbsY, X, Y, W, H, LayoutWidth, TailWidth, ParentAbsX,
                                      ParentAbsY, Action, Msg);
    Link->ID = ActionLinkID;
    Link->SetWindowRect(AbsX, AbsY, 0, 0);
    CreateChild(Link, NWS_HTMLCONTROL | NWS_VISIBLE | NWS_LINK);
    Controls.AddItem(Link);
    ShowIfVisible(this, Link);
    return 1;
}
NW_HOOK_METHOD(0x10031f20, NCHtmlViewer, AddActionLink)

// 0x10032040 - <button>: a label "&$n;" is sysstring n
INT NCHtmlViewer::AddButton(INT X, INT Y, INT W, INT H, TCHAR* NormalTexName,
                            TCHAR* PushedTexName, TCHAR* Action, TCHAR* Label)
{
    TCHAR Head[8];
    NCHtmlButton* Button = new NCHtmlButton(NormalTexName, PushedTexName, Action, 0);
    Button->ID = ButtonID;
    Button->SetWindowRect(X, Y, W, H);
    CreateChild(Button, NWS_HTMLCONTROL | NWS_VISIBLE);
    if (Label)
    {
        INT Length = appStrlen(Label);
        appStrncpy(Head, Label, 3);
        if (!appStrcmp(Head, TEXT("&$")) && Label[Length - 1] == ';')
            Button->SetTextID(appAtoi(Label + 2));
        else
            Button->SetText(Label);
    }
    ShowIfVisible(this, Button);
    Controls.AddItem(Button);
    return 1;
}
NW_HOOK_METHOD(0x10032040, NCHtmlViewer, AddButton)

// 0x100321a0 - <multiedit var> (the box's constructor is 0x1003DB90)
INT NCHtmlViewer::AddMultiEdit(INT X, INT Y, INT W, INT H, TCHAR* Var)
{
    NCHtmlMultiEdit* Edit = new NCHtmlMultiEdit;
    Edit->ID = MultiEditID;
    Edit->SetWindowRect(X, Y, W, H);
    CreateChild(Edit, NWS_HTMLCONTROL | NWS_VISIBLE);
    Edit->SetCaption(Var);
    ShowIfVisible(this, Edit);
    Controls.AddItem(Edit);
    return 1;
}
NW_HOOK_METHOD(0x100321a0, NCHtmlViewer, AddMultiEdit)

// 0x10032290 - <edit var type action length>: Enter sends the action; a length sets NCEditBox's limit
INT NCHtmlViewer::AddEdit(INT X, INT Y, INT W, INT H, TCHAR* Var, INT Type, TCHAR* Action, INT Length)
{
    NCHtmlEdit* Edit = new NCHtmlEdit(100, Type);   // 0x1003DC50
    Edit->SetWindowRect(X, Y, W, H);
    CreateChild(Edit, NWS_HTMLCONTROL | NWS_VISIBLE);
    Edit->ID = EditID;
    Edit->SetCaption(Var);
    Edit->EnterCommand = Action;
    Edit->SetMaxLength(Length);
    ShowIfVisible(this, Edit);
    Controls.AddItem(Edit);
    return 1;
}
NW_HOOK_METHOD(0x10032290, NCHtmlViewer, AddEdit)

// 0x100332d0 - <combobox var list sel>: always left hidden (ShowWindow shows it)
INT NCHtmlViewer::AddComboBox(INT X, INT Y, INT W, INT H, TCHAR* Var, TCHAR* List, TCHAR* Sel)
{
    NCHtmlComboBox* Combo = new NCHtmlComboBox;
    Combo->SetWindowRect(X, Y, W, H);
    CreateChild(Combo, NWS_HTMLCONTROL | NWS_VISIBLE);
    Combo->SetCaption(Var);
    Combo->ID = ComboBoxID;
    Combo->SetList(List, Sel);
    ShowIfVisible(this, Combo);
    Combo->HideWindow();
    Controls.AddItem(Combo);
    return 1;
}
NW_HOOK_METHOD(0x100332d0, NCHtmlViewer, AddComboBox)

// 0x100333e0 - back to an empty page. The controls are taken off the window but not deleted;
// an open question of an action link is closed.
void NCHtmlViewer::Clear()
{
    if (bLoaded && Doc)
    {
        Doc->Release();
        if (Doc)
            delete Doc;
        Doc = NULL;
    }
    INT N = Controls.Num();
    for (INT i = 0; i < N; i++)
    {
        if (Controls(i)->IsA(NClass::NCActionLinker()))
        {
            NCGaraDialogBox* Box = (NCGaraDialogBox*)GDesktop->MainDialogBox;   // C2: a CGaraDialogBox
            if (Box->DialogID == DIALOG_ActionLink)
            {
                Box->HideWindow();
                Box->Reset();   // 0x10006740
            }
        }
        RemoveChildWindow(Controls(i)->Entry);
    }
    Controls.Empty();
    bLoaded = 0;
    ScrollY = 0;
    if (ScrollBar)
        ScrollBar->SetScrollPos(0);
    DeltaY = 0;
}
NW_HOOK_METHOD(0x100333e0, NCHtmlViewer, Clear)

// 0x10035e90 - the tree from the tag list: <html> (whose imgsrc is the background) or <body>
// starts the page, which runs to its closing tag; laid out at the width without the scroll
// bar. TAB then goes through the controls in page order and back to the viewer.
void NCHtmlViewer::BuildDocument(NHtmlTagList* Tags)
{
    NHtmlTag** It = Tags->First;
    while (It != Tags->Last)
    {
        NHtmlTag* Tag = *It;
        if (Tag->Tag == HTMLTAG_Html)
        {
            SetBackTexture(Tag->HasAttr(ATTR_Html_ImgSrc) ? Tag->GetAttr(ATTR_Html_ImgSrc, 0)
                                                          : HtmlTransparent);
        }
        else if (Tag->Tag != HTMLTAG_Body)
        {
            It++;
            continue;
        }
        void* Mem = appMalloc(sizeof(NHtmlBlock), TEXT("new"));
        Doc = Mem ? ::new (Mem) NHtmlBlock(NULL, this, FontType) : NULL;
        Doc->Parse(&It, 0);
    }
    if (!Doc)
        return;

    Doc->Layout(Width - 16, Height);
    Doc->Arrange(Width - 16, Height);
    INT N = Controls.Num();
    if (N > 1)
    {
        SetTabOrder(Controls(0)->Entry, Controls(N - 1)->Entry);
        Controls(0)->SetTabOrder(Controls(1)->Entry, Entry);
        Controls(N - 1)->SetTabOrder(Entry, Controls(N - 2)->Entry);
        for (INT i = 1; i < N - 1; i++)
            Controls(i)->SetTabOrder(Controls(i + 1)->Entry, Controls(i - 1)->Entry);
    }
}
NW_HOOK_METHOD(0x10035e90, NCHtmlViewer, BuildDocument)

// 0x10036040 - a page from a file (links, the tutorial)
INT NCHtmlViewer::LoadHtmlFile(TCHAR* Source)
{
    NHtmlParser Parser;
    TCHAR* Text = Parser.LoadFile(Source);
    if (!Text)
    {
        Debugf(TEXT("Can not open Html file : %s"), Source);
        return 0;
    }
    if (bLoaded)
        Clear();
    NHtmlTagList* Tags = Parser.Parse(Text);
    BuildDocument(Tags);
    Parser.FreeTags(Tags);
    Parser.FreeFile();
    bLoaded = 1;
    return 1;
}
NW_HOOK_METHOD(0x10036040, NCHtmlViewer, LoadHtmlFile)

// 0x10036130 - a page from text (the server's NPC dialogs)
INT NCHtmlViewer::LoadHtml(TCHAR* Text)
{
    NHtmlParser Parser;
    if (!Text)
        return 0;
    if (bLoaded)
        Clear();
    NHtmlTagList* Tags = Parser.Parse(Text);
    BuildDocument(Tags);
    Parser.FreeTags(Tags);
    bLoaded = 1;
    return 1;
}
NW_HOOK_METHOD(0x10036130, NCHtmlViewer, LoadHtml)

NW_CHECK_VTABLE(NCHtmlViewer, 0x10110120, 77)

/*-----------------------------------------------------------------------------
    NCTutorialHtmlViewer.
-----------------------------------------------------------------------------*/

// 0x100334f0
NCTutorialHtmlViewer::NCTutorialHtmlViewer()
{
}
NW_HOOK_CTOR(0x100334f0, NCTutorialHtmlViewer)

// 0x10033540
NW_HOOK_DTOR(0x10033540, NCTutorialHtmlViewer)

// 0x1002fad0 (vslot 25) - NCHtmlViewer's, as a bool
INT NCTutorialHtmlViewer::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    return NCHtmlViewer::WindowProc(Msg, wParam, lParam) != 0;
}
NW_HOOK_METHOD(0x1002fad0, NCTutorialHtmlViewer, WindowProc)

// 0x1002faf0 (vslot 35)
INT NCTutorialHtmlViewer::OnCreate()
{
    NCHtmlViewer::OnCreate();
    return 1;
}
NW_HOOK_METHOD(0x1002faf0, NCTutorialHtmlViewer, OnCreate)

// 0x10034840 (vslot 37) - guarded in C2
INT NCTutorialHtmlViewer::OnDestroy()
{
    guard(NCTutorialHtmlViewer::OnDestroy);
    NCHtmlViewer::OnDestroy();
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x10034840, NCTutorialHtmlViewer, OnDestroy)

NW_CHECK_VTABLE(NCTutorialHtmlViewer, 0x101102a0, 77)
