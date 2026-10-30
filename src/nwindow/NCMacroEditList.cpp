/*=============================================================================
    NCMacroEditList.cpp - the lines of the macro editor: twelve edit boxes, eight shown, scrolled by a slider.
    Stock: 0x1008B6B0 .. 0x1008B747 (the slider), 0x1008D7D0 .. 0x1008DE1F (the helpers, the scrolling),
    0x1008F140 .. 0x1008F4EF (OnCreate, OnPaint, OnDestroy), 0x1008FB80 .. 0x1008FD1F (constructors, destructor).
=============================================================================*/
#include "nwindow/NCMacroEditList.h"
#include "nwindow/NCConsole.h"
#include "nwindow/NCEditBox.h"
#include "nwindow/NCGDevice.h"
#include "nwindow/NCSliderBar.h"
#include "nwindow/NConsoleWnd.h"
#include "nwindow/NCrt.h"
#include "nwindow/NClasses.gen.h"

enum
{
    EDIT_FirstID = 0x1506,         // the line boxes' ids (name ours)
    EDIT_LineMaxLen = 32,          // the characters a line holds
    EDIT_LineWidth = 205,
    EDIT_LineHeight = 16,
    EDIT_LinePitch = 26,           // px from a line to the next
    EDIT_ExtraWidth = 0x108,       // NCEditBox's fourth constructor argument
};

/*-----------------------------------------------------------------------------
    Constructors and destructor.
-----------------------------------------------------------------------------*/

// 0x1008fb80 - nothing shown yet: the counters are cleared
NCMacroEditList::NCMacroEditList()
{
    VisibleLines = 0;
    TopLine = 0;
    TotalLines = 0;
}
NW_HOOK_CTOR(0x1008fb80, NCMacroEditList)

// 0x1008fca0 - the number of lines shown is the argument; the range starts at 24
NCMacroEditList::NCMacroEditList(INT InVisibleLines)
{
    TopLine = 0;
    VisibleLines = InVisibleLines;
    TotalLines = 24;
}
NW_HOOK_CTOR_ARGS(0x1008fca0, NCMacroEditList, (INT A), (A))

// 0x1008fc30
NW_HOOK_DTOR(0x1008fc30, NCMacroEditList)

/*-----------------------------------------------------------------------------
    The slider and the lines' positions.
-----------------------------------------------------------------------------*/

// 0x1008b6b0 - the slider repeats every 200 ms; its range and its value
void NCMacroEditList::InitSlider()
{
    SetRepeatDelay(200);
    ScrollBar->SetRange(TotalLines, VisibleLines);
    ScrollBar->SetValue(TopLine);
}
NW_HOOK_METHOD(0x1008b6b0, NCMacroEditList, InitSlider)

// 0x1008b6f0 - the range is the twelve lines; the first line shown is kept where eight lines still fit
void NCMacroEditList::UpdateSlider()
{
    TotalLines = NMACRO_EditLines;
    if (VisibleLines <= NMACRO_EditLines)
    {
        if (NMACRO_EditLines - TopLine < VisibleLines)
            TopLine = NMACRO_EditLines - VisibleLines;
    }
    ScrollBar->SetRange(NMACRO_EditLines, VisibleLines);
    ScrollBar->SetValue(TopLine);
}
NW_HOOK_METHOD(0x1008b6f0, NCMacroEditList, UpdateSlider)

// 0x1008d830 - the first line box that has the focus
INT NCMacroEditList::GetFocusLine()
{
    for (INT i = 0; i < Edits.Num(); i++)
    {
        NCEditBox* Edit = Edits(i);
        if (Edit != NULL && Edit->IsFocused())
            return i;
    }
    return -1;
}
NW_HOOK_METHOD(0x1008d830, NCMacroEditList, GetFocusLine)

// 0x1008d870 - every box hidden, then the eight from TopLine put in the window one under the other and shown
void NCMacroEditList::LayoutLines()
{
    for (INT i = 0; i < NMACRO_EditLines; i++)
        Edits(i)->HideWindow();
    INT Top = TopLine;
    for (INT i = Top; i < Top + NMACRO_ShownLines; i++)
    {
        // the stock code fills four dwords from the window ({ ScreenX, ScreenY, Width, Height }); SetScreenPos reads x and y
        INT Origin[4];
        Origin[0] = ScreenX;
        Origin[1] = ScreenY;
        Origin[3] = Height;
        Origin[2] = Width;
        NCEditBox* Edit = Edits(i);
        Edit->SetScreenPos(18, (i - Top) * EDIT_LinePitch + 8, (const POINT*)Origin);
        Edit->ShowWindow();
        Top = TopLine;
    }
}
NW_HOOK_METHOD(0x1008d870, NCMacroEditList, LayoutLines)

// 0x1008d7d0 - the line box under (X, Y): the window found there when it is an edit box, its number among the twelve
INT NCMacroEditList::HitLine(INT X, INT Y)
{
    NWndEntry* Found = FindWindowAt(X, Y);
    if (Found != NULL && Found->Wnd != NULL)
    {
        NCWnd* Wnd = Found->Wnd;
        if (Wnd->IsA(NClass::NCEditBox()))
        {
            for (INT i = 0; i < NMACRO_EditLines; i++)
            {
                if (Wnd == (NCWnd*)Edits(i))
                    return i;
            }
        }
    }
    return -1;
}
NW_HOOK_METHOD(0x1008d7d0, NCMacroEditList, HitLine)

/*-----------------------------------------------------------------------------
    Scrolling (the slider's value is the first line shown).
-----------------------------------------------------------------------------*/

// 0x1008d910 (vslot 69)
INT NCMacroEditList::OnScrollBar(INT Value)
{
    TopLine = Value;
    LayoutLines();
    UpdateSlider();
    return 1;
}
NW_HOOK_METHOD(0x1008d910, NCMacroEditList, OnScrollBar)

// 0x1008d940 (vslot 67)
INT NCMacroEditList::ScrollUp()
{
    if (VisibleLines < TotalLines)
    {
        TopLine--;
        if (TopLine < 0)
            TopLine = 0;
        ScrollBar->SetValue(TopLine);
        LayoutLines();
        UpdateSlider();
    }
    return 1;
}
NW_HOOK_METHOD(0x1008d940, NCMacroEditList, ScrollUp)

// 0x1008d990 (vslot 68)
INT NCMacroEditList::ScrollDown()
{
    if (VisibleLines < TotalLines)
    {
        INT Max = TotalLines - VisibleLines;
        TopLine++;
        if (TopLine > Max)
            TopLine = Max;
        ScrollBar->SetValue(TopLine);
        LayoutLines();
        UpdateSlider();
    }
    return 1;
}
NW_HOOK_METHOD(0x1008d990, NCMacroEditList, ScrollDown)

/*-----------------------------------------------------------------------------
    The window.
-----------------------------------------------------------------------------*/

// 0x1008f140 (vslot 36) - the twelve line boxes (ids 0x1506.., 32 characters, at 26 pixel steps; the ones that fall past the
// eighth start hidden) chained in the tab order (the last one back to the window), then the slider
INT NCMacroEditList::OnCreate()
{
    NCScrollWnd::OnCreate();
    Edits.Empty();
    INT Index = 0;
    for (INT Y = 8; Y < 0x140; Y += EDIT_LinePitch)
    {
        NCEditBox* Edit = new NCEditBox(0, 0, 1, EDIT_ExtraWidth, 0);
        Edit->SetMaxLength(EDIT_LineMaxLen);   // the box's longest text
        Edit->ID = Index + EDIT_FirstID;
        Edit->SetWindowRect(18, Y, EDIT_LineWidth, EDIT_LineHeight);
        CreateChild(Edit, NWS_VISIBLE);
        if (Y > 0xBE)
            Edit->HideWindow();
        Edits.AddItem(Edit);
        Index++;
    }
    SetTabOrder(Edits(0)->Entry, Edits(11)->Entry);
    for (INT i = 0; i < NMACRO_EditLines; i++)
    {
        NWndEntry* Prev;
        NWndEntry* Next;
        if (i == 0)
        {
            Prev = Entry;
            Next = Edits(1)->Entry;
        }
        else if (i != 11)
        {
            Prev = Edits(i - 1)->Entry;
            Next = Edits(i + 1)->Entry;
        }
        else
        {
            Prev = Edits(10)->Entry;
            Next = Entry;
        }
        Edits(i)->SetTabOrder(Next, Prev);
    }
    InitSlider();
    return 1;
}
NW_HOOK_METHOD(0x1008f140, NCMacroEditList, OnCreate)

// 0x1008f2c0 (vslot 55) - the number of each line shown ("00".."11", in the big digits), then, when a box has the focus
// outside the eight shown, the lines scroll to it
INT NCMacroEditList::OnPaint(NCGDevice* Device)
{
    guard(NCMacroEditList::OnPaint);
    Device->BeginPaint(ScreenX, ScreenY);
    Device->SetClip(ScreenX, ScreenY, Width, Height);
    for (INT i = TopLine; i < TopLine + NMACRO_ShownLines; i++)
    {
        TCHAR Text[68];
        appSprintf(Text, i < 10 ? TEXT("0%d") : TEXT("%d"), i);
        Device->DrawSpecialDigit(1, 2, (i - TopLine) * EDIT_LinePitch + 12, 0xFFFFFFFF, Text);
    }
    INT Focus = GetFocusLine();
    if (Focus >= 0)
    {
        INT Target = Focus - 7;
        if (TopLine < Target || (Target = Focus, Focus < TopLine))
            OnScrollBar(Target);
    }
    Device->EndPaint();
    Device->ReleaseClip();
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x1008f2c0, NCMacroEditList, OnPaint)

// 0x1008f410 (vslot 37) - the line boxes go
INT NCMacroEditList::OnDestroy()
{
    guard(NCMacroEditList::OnDestroy);
    for (INT i = 0; i < Edits.Num(); i++)
    {
        if (Edits(i) != NULL)
        {
            RemoveChildWindow(Edits(i)->Entry);
            delete Edits(i);
            Edits(i) = NULL;
        }
    }
    Edits.Empty();
    NCScrollWnd::OnDestroy();
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x1008f410, NCMacroEditList, OnDestroy)

/*-----------------------------------------------------------------------------
    The check of the lines before a macro is saved.
-----------------------------------------------------------------------------*/

// the type of the shortcut at Index of NCShortCutWnd's array (the desktop keeps the window at +0x104; the window's +0x1AC
// points at the records, 0x1A7C bytes each, the page's ten slots one after the other; the type is at +0x190C: the C2
// NShortCutItem; names ours, NCShortCutWnd.h is w2status's)
static INT ShortCutType(INT Index)
{
    BYTE* Base = nw::Field<BYTE*>(nw::Field<NCWnd*>(GConsole->Desktop, 0x104), 0x1AC);
    return *(INT*)(Base + Index * 0x1A7C + 0x190C);
}

// 0x1008d9f0 - the lines up to the first empty one hold what the parser takes as a command (no text after an empty line;
// the first line may not be empty). The commands that take numbers need them: /delay a positive one, /useshortcut (the
// three forms) a page and a slot from 1 to 10 whose shortcut is not itself a macro; the commands of the GMs need the
// player to be one; plain text is chat
UBOOL NCMacroEditList::ValidateLines()
{
    TCHAR Line[3070];
    UBOOL bFilled = 1;
    for (INT i = 0; i < NMACRO_EditLines; i++)
    {
        const TCHAR* Text = Edits(i)->CommitText();
        if (Text == NULL)
        {
            if (bFilled)
            {
                if (i == 0)
                    return 0;
                bFilled = 0;
            }
        }
        else
        {
            if (!bFilled)
                return 0;
            appStrcpy(Line, Text);
            INT Command = Parser.GetCommand(Line);
            if (Command > 0x4C)
            {
                if (Command > 0x10000001)
                {
                    if (Command != 0x20000000)
                        return 0;
                }
                else if (Command >= 0x10000000)
                {
                    User* Player = GConsole->GetMyUser();
                    if (Player == NULL || Player->BuilderLevel == 0)
                        return 0;
                }
                else if (Command < 0x4D || Command > 0x51)
                    return 0;
            }
            else if (Command == 0x4C)
            {
                // /delay <seconds>
                TCHAR* Token = Parser.GetNextToken();
                if (Token == NULL || NWtol(Token) < 1)
                    return 0;
            }
            else
            {
                if ((DWORD)Command > 0x4B)
                    return 0;
                if (Command >= 0x1B && Command <= 0x1D)
                {
                    // /useshortcut <page> <slot>
                    TCHAR* Token = Parser.GetNextToken();
                    if (Token == NULL)
                        return 0;
                    INT Page = NWtol(Token);
                    if (Page < 1 || Page > 10)
                        return 0;
                    Token = Parser.GetNextToken();
                    if (Token == NULL)
                        return 0;
                    INT Slot = NWtol(Token);
                    if (Slot < 1 || Slot > 10)
                        return 0;
                    if (ShortCutType((Slot + Page * 10) - 11) == 4)
                        return 0;
                }
            }
        }
    }
    return 1;
}
NW_HOOK_METHOD(0x1008d9f0, NCMacroEditList, ValidateLines)

NW_CHECK_VTABLE(NCMacroEditList, 0x1011b2b8, 70)
