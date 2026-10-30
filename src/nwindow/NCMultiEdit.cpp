/*=============================================================================
    NCMultiEdit.cpp - the multi-line text box, its line list and string array, and the HTML
    pages' version of it. Stock: 0x1001a440 .. 0x10020c9d, 0x10021f00 .. 0x10022147 (NCMultiEdit),
    0x1002ff20 .. 0x10030154, 0x10030ff0 .. 0x100326cf, 0x10033540 .. 0x100338c2
    (NCHtmlMultiEdit). SetFocus and the HTML box's OnLButtonDown are bodies the linker folded with
    other classes'. The getters of the line count (0x10033ac0, hooked as NActionItem's) and of
    FString::Len (0x1001b380) are folded bodies other classes call too.
=============================================================================*/
#include "nwindow/NCMultiEdit.h"
#include "nwindow/NCIMEManager.h"
#include "nwindow/NCChineseIME.h"
#include "nwindow/NCButton.h"
#include "nwindow/NCHtmlButton.h"
#include "nwindow/NCSliderBar.h"
#include "nwindow/NCHtmlSliderBar.h"
#include "nwindow/NCHtmlViewer.h"
#include "nwindow/NCCompositeBox.h"
#include "nwindow/NCGDevice.h"
#include "nwindow/NConsoleWnd.h"
#include "nwindow/NString.h"

#include <string.h>

enum { TIMER_MultiEditCaret = 0xFA0D05 };

// the code a click sends the viewer with the box's y (as NCHtmlEdit's): MAKELONG(100, LocalY)
enum { HTML_SCROLLTO = 100 };

// the composition window of the Chinese IME and its last composition
static FORCEINLINE NCCompositeBox* ChineseCompositeBox(NCIMEManager* Manager)
{
    return ((NCChineseIME*)Manager->ChineseIME)->CompositeBox;
}
static FORCEINLINE FString& ChineseLastComp(NCIMEManager* Manager)
{
    return ((NCChineseIME*)Manager->ChineseIME)->LastComp;
}

// the IME types whose composition is drawn in its own window (Chinese layouts)
static FORCEINLINE UBOOL IsWindowedIME(INT Type)
{
    return Type == 10 || Type == 9 || Type == 11 || Type == 12;
}

/*-----------------------------------------------------------------------------
    NLineNode, NLineList.
-----------------------------------------------------------------------------*/

// 0x1001efe0 (not inlined: the stock callers call it)
__declspec(noinline) NLineNode::NLineNode()
{
    Prev = NULL;
    Next = NULL;
    Text.Empty();
}
NW_HOOK_CTOR(0x1001efe0, NLineNode)

// 0x1001f030 (not inlined: the stock callers call it)
__declspec(noinline) NLineNode::~NLineNode()
{
    Prev = NULL;
    Next = NULL;
    Text.Empty();
}
NW_HOOK_DTOR(0x1001f030, NLineNode)

// 0x1001f920 - one empty line
NLineList::NLineList()
{
    Count = 1;
    Unknown4 = 0;
    NLineNode* Node = new NLineNode;
    Head = Node;
    Tail = Node;
    Current = Node;
    CurIndex = 0;
}
NW_HOOK_CTOR(0x1001f920, NLineList)

// 0x10023d80 - Count is cleared before Clear(), which then frees nothing: the nodes are left
// (sic; OnDestroy has emptied the list before)
NLineList::~NLineList()
{
    Count = 0;
    Unknown4 = 0;
    Clear();
    if (Head)
        delete Head;
}
NW_HOOK_DTOR(0x10023d80, NLineList)

// 0x1001fa10 - the node of a line, walked to from the nearest of the head, the tail and the cursor (which it
// moves). C2 walks the whole distance (C1's walks from the cursor stopped halfway); an index out of range gives NULL
NLineNode* NLineList::GetLine(INT Index)
{
    if (Index >= Count)
        return NULL;
    if (Index == CurIndex)
        return Current;
    NLineNode* Node;
    if (Index < CurIndex)
    {
        INT Distance = CurIndex - Index;
        if (Distance < Index)
        {
            Node = Current;
            for (INT i = Distance; i > 0; i--)
                Node = StepBack();
            return Node;
        }
        Current = Head;
        CurIndex = 0;
        Node = Current;
        for (INT i = Index; i > 0; i--)
            Node = StepForward();
        return Node;
    }
    INT Distance = Index - CurIndex;
    INT FromTail = Count - Index - 1;
    if (FromTail < Distance)
    {
        Node = Tail;
        Current = Tail;
        CurIndex = Count - 1;
        for (INT i = 0; i < FromTail; i++)
            Node = StepBack();
        return Node;
    }
    Node = Current;
    for (INT i = Distance; i > 0; i--)
        Node = StepForward();
    return Node;
}
NW_HOOK_METHOD(0x1001fa10, NLineList, GetLine)

// 0x1001f9a0 - the characters of every line (the cursor ends on the last one)
INT NLineList::TotalLength()
{
    INT Total = 0;
    Current = Head;
    CurIndex = 0;
    for (INT i = 0; i < Count; i++)
    {
        if (!Current)
            break;
        Total += Current->Text.Len();
        if (!StepForward())
            break;
    }
    return Total;
}
NW_HOOK_METHOD(0x1001f9a0, NLineList, TotalLength)

// 0x10021b80 - -1 appends; an index out of range frees the new node and fails
INT NLineList::Insert(INT Index)
{
    NLineNode* Node = new NLineNode;
    if (Index == -1)
    {
        if (Count)
        {
            if (GetLine(Count - 1) && Node)
            {
                GetLine(Count - 1)->Next = Node;
                Node->Prev = GetLine(Count - 1);
                Count++;
                Tail = Node;
                return 1;
            }
        }
        else if (Node)
        {
            if (Head)
                delete Head;
            Head = Node;
            Current = Node;
            CurIndex = 0;
            Tail = Node;
        }
        Count++;
        return 1;
    }
    if (Index > Count || Index < 0)
    {
        if (Node)
            delete Node;
        return 0;
    }
    NLineNode* At = NULL;
    if (Index < Count && GetLine(Index))
        At = GetLine(Index);
    if (Index > 0 && GetLine(Index - 1))
    {
        NLineNode* Before = GetLine(Index - 1);
        if (Before)
        {
            Before->Next = Node;
            Node->Prev = Before;
        }
    }
    if (Node)
        Node->Next = At;
    if (At)
        At->Prev = Node;
    if (Index == 0)
    {
        Head = Node;
        if (Count == 0)
        {
            Current = Node;
            Tail = Node;
            CurIndex = 0;
        }
    }
    if (Index == Count)
        Tail = Node;
    if (Index <= CurIndex && Count)
        CurIndex++;
    Count++;
    return 1;
}
NW_HOOK_METHOD(0x10021b80, NLineList, Insert)

// 0x10021d20 - the line's text is emptied, the node unlinked and freed; the cursor stays on its line
void NLineList::RemoveAt(INT Index)
{
    if (Index < Count && Index > -1 && GetLine(Index))
    {
        NLineNode* Node = GetLine(Index);
        GetLine(Index)->Text.Empty();
        if (Node)
        {
            if (Index < CurIndex && CurIndex > 0)
                CurIndex--;
            else if (Index == CurIndex)
            {
                if (Node->Prev)
                {
                    Current = Node->Prev;
                    CurIndex--;
                }
                else
                    Current = Node->Next;
            }
            if (Node->Prev)
                Node->Prev->Next = Node->Next;
            if (Node->Next)
                Node->Next->Prev = Node->Prev;
            if (Index == 0)
                Head = Node->Next;
            if (Index == Count - 1)
                Tail = Node->Prev;
            delete Node;
            Count--;
        }
    }
}
NW_HOOK_METHOD(0x10021d20, NLineList, RemoveAt)

// 0x10021de0 - every line removed, as many times as there were lines
void NLineList::Clear()
{
    for (INT i = Count; i > 0; i--)
        RemoveAt(0);
    Head = NULL;
    Current = NULL;
    Tail = NULL;
    CurIndex = 0;
}
NW_HOOK_METHOD(0x10021de0, NLineList, Clear)

/*-----------------------------------------------------------------------------
    NStringArray.
-----------------------------------------------------------------------------*/

// 0x10026940
NStringArray::NStringArray()
{
    Num = 0;
    Data = NULL;
    Max = 0;
    InsertAt(-1);
}
NW_HOOK_CTOR(0x10026940, NStringArray)

// 0x10026780 - a jump to Empty
NStringArray::~NStringArray()
{
    Empty();
}
NW_HOOK_DTOR(0x10026780, NStringArray)

// 0x10026710 - every string cleared, then the strings (20 bytes each) and the array freed; returns 0. The
// function list has no entry for it (it is only reached from the destructor's jump), so it has no hook of
// its own: the destructor above is the way in
INT NStringArray::Empty()
{
    for (INT i = 0; i < Num; i++)
        Data[i].Empty();
    Num = 0;
    if (Data)
    {
        delete[] Data;
        Data = NULL;
    }
    Max = 0;
    return 0;
}

// 0x10026790 (not inlined: the stock callers call it) - the first string needs no clearing. Growing copies Num *bytes* of the old array
// before destroying it (sic): past 127 strings their text would be lost.
__declspec(noinline) void NStringArray::InsertAt(INT Index)
{
    if (Index == -1)
        Index = Num;
    if (!Data)
    {
        Data = new NString[128];
        Max += 128;
    }
    else
    {
        if (Num >= Max - 1)
        {
            Max += 128;
            NString* Old = Data;
            Data = new NString[Max];
            appMemcpy(Data, Old, Num);
            if (Old)
                delete[] Old;
        }
        if (Index < Num)
            memmove(&Data[Index + 1], &Data[Index], (Num - Index + 1) * sizeof(NString));
        appMemset(&Data[Index], 0, sizeof(NString));
    }
    Num++;
}
NW_HOOK_METHOD(0x10026790, NStringArray, InsertAt)

/*-----------------------------------------------------------------------------
    NCMultiEdit.
-----------------------------------------------------------------------------*/

// 0x10023ea0 - NControl's constructor is inlined
NCMultiEdit::NCMultiEdit()
{
    BackTex = NULL;
    WhiteTex = NULL;
    TextBuf = NULL;
    CaretTimer = 0;
    CaretLine = 0;
    CaretCol = 0;
    bCaretOn = 0;
    VisibleRows = 0;
    TopLine = 0;
    NumLines = 1;
    CaretRow = 0;
    CaretX = 0;
    bCompRow = 0;
    Unknown220 = 0;
    UpButton = NULL;
    DownButton = NULL;
    ScrollBar = NULL;
    bReading = 0;
    Unknown214 = 0;
    Unknown216 = 0;
    Unknown254 = 0;
    Unknown258 = 0;
    IMECaretX = 0;
    IMECaretY = 0;
    TotalRows = 1;
    Unknown250 = 1;
    Unknown24C = 1;
    Unknown25C = 16;
}
NW_HOOK_CTOR(0x10023ea0, NCMultiEdit)

// 0x10023df0 - the implicit destructor: Reading, Comp, IntList (TIntArray's, 0x1001FB70), Lines, Strings
NW_HOOK_DTOR(0x10023df0, NCMultiEdit)


// 0x1001dfe0 (vslot 25) - the scroll controls, then the IME's messages
INT NCMultiEdit::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (NControl::WindowProc(Msg, wParam, lParam))
        return 1;
    if (Msg == NWM_COMMAND)
    {
        if (wParam == (WPARAM)NID_MULTIEDIT_UP)
            return OnScrollUp() != 0;
        if (wParam == (WPARAM)NID_MULTIEDIT_DOWN)
            return OnScrollDown() != 0;
        if (wParam == (WPARAM)NID_MULTIEDIT_BAR)
            return OnScrollBar(lParam) != 0;
        return 0;
    }
    if (Msg == (UINT)NIM_COMPOSITION)
        return OnComposition(wParam, lParam) != 0;
    if (Msg == (UINT)NIM_RESULT)
        return OnResult(wParam, lParam) != 0;
    if (Msg == (UINT)NIM_GETCARET)
        return OnGetCaret(wParam, lParam) != 0;
    if (Msg == (UINT)NIM_READING)
        return OnReading(wParam, lParam) != 0;
    if (Msg == (UINT)NIM_ATTRIBUTE)
        return OnAttribute(wParam, lParam) != 0;
    return 0;
}
NW_HOOK_METHOD(0x1001dfe0, NCMultiEdit, WindowProc)

// 0x1001e130 - the caret's row stays among the visible ones (hooked together with OnKeyDown, which keeps ecx
// across a call to it)
void NCMultiEdit::ScrollToCaret()
{
    if (TopLine + VisibleRows <= CaretRow)
        TopLine = CaretRow - VisibleRows + 1;
    if (CaretRow < TopLine)
        TopLine = CaretRow;
}
NW_HOOK_METHOD(0x1001e130, NCMultiEdit, ScrollToCaret)

// stock body shared with NCEditBox's (0x10009140)
void NCMultiEdit::SetFocus()
{
    NCWnd::SetFocus();
    IMEManager->Activate();
}

// 0x1001e170 (vslot 30)
void NCMultiEdit::KillFocus()
{
    IMEManager->Deactivate();
    NCWnd::KillFocus();
}
NW_HOOK_METHOD(0x1001e170, NCMultiEdit, KillFocus)

// 0x1001e190 (vslot 35) - blinks the caret, whatever the timer
INT NCMultiEdit::OnTimer(INT TimerID)
{
    bCaretOn = !bCaretOn;
    return 1;
}
NW_HOOK_METHOD(0x1001e190, NCMultiEdit, OnTimer)

// 0x1001ef10 (vslot 74) - answers with the caret position the painters left
INT NCMultiEdit::OnGetCaret(WPARAM wParam, LPARAM lParam)
{
    if (wParam)
        SendTo((NWndEntry*)wParam, NIM_CARET, IMECaretX, IMECaretY);
    return 1;
}
NW_HOOK_METHOD(0x1001ef10, NCMultiEdit, OnGetCaret)

// 0x1001ef40 (vslot 75)
INT NCMultiEdit::OnReading(WPARAM wParam, LPARAM lParam)
{
    bReading = 1;
    Reading.Empty();
    TCHAR* Str = IMEManager->GetReadingStr();
    if (Str)
    {
        ReadingParam = lParam;
        Reading = Str;
    }
    return 1;
}
NW_HOOK_METHOD(0x1001ef40, NCMultiEdit, OnReading)

// C2 CANDIDATE: the C2 function is not the same code as the C1 one; kept only while it matches the stock one (verify_data)
// 0x1001ef90 (vslot 76)
INT NCMultiEdit::OnAttribute(WPARAM wParam, LPARAM lParam)
{
    bReading = 0;
    Reading.Empty();
    return 1;
}
NW_HOOK_METHOD(0x1001ef90, NCMultiEdit, OnAttribute)

// 0x1001efb0 (vslot 71) - the scroll bar was dragged
INT NCMultiEdit::OnScrollBar(INT Value)
{
    TopLine = Value;
    return 1;
}
NW_HOOK_METHOD(0x1001efb0, NCMultiEdit, OnScrollBar)

// 0x1001ed60 - the composing characters, on white while the caret blinks on
void NCMultiEdit::PaintCompositionKR(FString Text, INT X, INT Y, NCGDevice* Device)
{
    SIZE Size;
    NCGDevice::GetTextExtent(&Size, *Text, 0, 0, 0, 0, 0, L2FONT_Normal);
    if (bCaretOn)
    {
        Device->DrawStretchedTex(LineGap + X, Y, Size.cx, 12, WhiteTex);
        Device->DrawText(LineGap + X, Y, 0xFF000000, *Text, 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0,
                         0, 0);
    }
    else
        Device->DrawText(LineGap + X, Y, 0xFFFFFFFF, *Text, 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0,
                         0, 0);
}
NW_HOOK_METHOD(0x1001ed60, NCMultiEdit, PaintCompositionKR)

// 0x1001e1b0 - NCEditBox's Japanese painters (PaintCompositionJP, PaintComposingJP) in one, for the row's part of
// the composition (Text, from CompStart; the characters drawn come from Comp): the lines by attribute along the
// clauses (from the clause the row starts in), then the dotted underline of the characters still being typed. The
// first target character of the composition (*Out still 0) gives the IME its caret, below the row (RowNum).
void NCMultiEdit::PaintCompositionJP(FString Text, INT CompStart, INT X, INT Y, INT RowNum, NCGDevice* Device, INT* Out)
{
    SIZE Size;
    INT LineY = Y + 13;
    NCIME* IME = IMEManager->JapaneseIME;
    INT Offset = 0;
    UBOOL bTarget = 0;
    FString Ch;
    INT ClauseIdx = 0;
    for (INT c = 0; c < IME->CompClauseLen; c++)
    {
        if (CompStart < (INT)IME->CompClause[c])
        {
            ClauseIdx = c - 1;
            break;
        }
    }
    DWORD* Clause = &IME->CompClause[ClauseIdx + 1];
    for (INT i = 0; i < Text.Len(); i++)
    {
        BYTE* Attr = &IME->CompAttr[i + CompStart];
        if ((*Attr & 1) && !bTarget)
        {
            bTarget = 1;
            if (!*Out)
            {
                IMECaretX = LineGap + Offset + X;
                IMECaretY = RowNum * 15 + 2;
                *Out = 1;
            }
        }
        INT CharX = LineGap + Offset + X;
        INT CharW = Device->DrawText(CharX, Y, 0xFFFFFFFF, *Comp.Mid(i + CompStart, 1), 0, 0, 0, L2FONT_Normal,
                                     0, 0, 0, 0, 0, 0);
        Offset += CharW;
        INT EndX = LineGap + Offset + X;
        if (*Clause == (DWORD)(i + CompStart + 1))
        {
            Clause++;
            EndX--;
        }
        if (IsFocused())
        {
            if ((*Attr & 1) && CharW > 0)
            {
                Device->DrawLine(CharX, LineY, EndX, Y + 14, 0xFFFFFFFF, 0);
                Device->DrawLine(CharX, LineY, EndX, LineY, 0xFFFFFFFF, 0);
            }
            if ((*Attr & 3) && CharW > 0)
                Device->DrawLine(CharX, LineY, EndX, LineY, 0xFFFFFFFF, 0);
            if (*Attr & 2)
                Device->DrawLine(CharX, LineY, EndX, LineY, 0xFFFFFFFF, 0);
            if (*Attr & 4)
                Device->DrawLine(CharX, LineY, EndX, LineY, 0xFFFF0000, 0);
        }
    }
    INT UnderlineW = 0;
    INT StartX = 0;
    INT Pos = CompStart;
    for (INT j = 0; j < Text.Len(); j++)
    {
        if (IMEManager->JapaneseIME->CompAttr[Pos] == ATTR_INPUT)
        {
            if (j == 0 || IMEManager->JapaneseIME->CompAttr[Pos - 1] != ATTR_INPUT)
            {
                Ch = Comp.Mid(CompStart, j);
                NCGDevice::GetTextExtent(&Size, *Ch, 0, 0, 0, 0, 0, L2FONT_Normal);
                StartX = Size.cx;
                Ch = Comp.Mid(Pos, 1);
                NCGDevice::GetTextExtent(&Size, *Ch, 0, 0, 0, 0, 0, L2FONT_Normal);
                UnderlineW += Size.cx;
            }
            else
            {
                Ch = Comp.Mid(Pos, 1);
                NCGDevice::GetTextExtent(&Size, *Ch, 0, 0, 0, 0, 0, L2FONT_Normal);
                UnderlineW += Size.cx;
            }
        }
        Pos++;
    }
    if (UnderlineW > 0)
    {
        for (INT k = 0; k < UnderlineW; k += 3)
            Device->DrawText(StartX + X + LineGap + k, Y + 3, 0xFFFFFFFF, TEXT("."), 0, 0, 0, L2FONT_Normal, 0, 0,
                             0, 0, 0, 0);
    }
}
NW_HOOK_METHOD(0x1001e1b0, NCMultiEdit, PaintCompositionJP)

// 0x1001e670 - NCEditBox's Taiwanese painters in one, for the row's part of the composition (Text, from
// CompStart), with the clause the row starts in: the target clause black on gray while focused, bit 2 underlined in
// red, and for the IME types 5 and 7 the other characters in red; a dotted underline for the other types. The first
// target character gives the IME its caret (*Out still 0; the Y is negative, the Taiwanese IME turns it); only when
// that character is 0 px wide does the reading string show there (as NCEditBox's ClauseW): a column of 12 px
// cells below the row for the type 6 (error position on in red on gray, the rest in blue), in the line black on
// gray for the type 4.
void NCMultiEdit::PaintCompositionTW(FString Text, INT CompStart, INT X, INT Y, INT RowNum, NCGDevice* Device, INT* Out)
{
    SIZE Size;
    NCGDevice::GetTextExtent(&Size, *Reading, 0, 0, 0, 0, 0, L2FONT_Normal);
    DWORD ReadingW = Size.cx;
    if (ReadingW > 0 && GConsole->GetIMEType() == 6)
        ReadingW = 12;
    NCIME* IME = IMEManager->TaiwanIME;
    INT Offset = 0;
    INT ClauseX = 0;
    UBOOL bTarget = 0;
    UBOOL bClauseX = 0;
    INT ClauseIdx = 0;
    for (INT c = 0; c < IME->CompClauseLen; c++)
    {
        if (CompStart < (INT)IME->CompClause[c])
        {
            ClauseIdx = c - 1;
            break;
        }
    }
    DWORD* Clause = &IME->CompClause[ClauseIdx + 1];
    for (INT i = 0; i < Text.Len(); i++)
    {
        INT CharX = LineGap + Offset + X;
        INT CharW = Device->DrawText(CharX, Y, 0xFFFFFFFF, *Text.Mid(i, 1), 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0,
                                     0);
        BYTE* Attr = &IME->CompAttr[i + CompStart];
        if ((*Attr & 1) && !bTarget)
        {
            bTarget = 1;
            if (!*Out)
            {
                IMECaretX = LineGap + Offset + X;
                IMECaretY = -2 - RowNum * 15;
                *Out = 1;
            }
            if (!CharW)
            {
                ClauseX = Offset;
                bClauseX = 1;
            }
        }
        Offset += CharW;
        INT EndX = LineGap + Offset + X;
        if (*Clause == (DWORD)(i + CompStart + 1))
        {
            Clause++;
            EndX--;
        }
        if (IsFocused())
        {
            INT Marks = 0;
            if ((*Attr & 1) && CharW > 0)
            {
                Device->DrawStretchedTex(CharX, Y, EndX - CharX, 12, FindTexture(TEXT("NWindow.GrayTexture")));
                Device->DrawText(CharX, Y, 0xFF000000, *Text.Mid(i, 1), 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
                Marks = 1;
            }
            if (*Attr & 4)
            {
                Device->DrawLine(CharX, Y + 13, EndX, Y + 13, 0xFFFF0000, 0);
                Marks++;
            }
            if (!Marks && (GConsole->GetIMEType() == 5 || GConsole->GetIMEType() == 7))
                Device->DrawText(CharX, Y, 0xFFFF0000, *Text.Mid(i, 1), 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
        }
    }
    if (Text.Len() > 0 && GConsole->GetIMEType() != 5 && GConsole->GetIMEType() != 7)
    {
        NCGDevice::GetTextExtent(&Size, *Text, 0, 0, 0, 0, 0, L2FONT_Normal);
        for (INT k = 0; k < Size.cx; k += 3)
            Device->DrawText(X + k, Y + 3, 0xFFFFFFFF, TEXT("."), 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
    }
    if (bReading && bClauseX)
    {
        INT RX = LineGap + ClauseX + X;
        if (GConsole->GetIMEType() == 6)
        {
            Device->SetClip(ScreenX, ScreenY, Width, Height + 100);
            INT RY = Y + 6;
            Device->DrawStretchedTex(RX, RY, 12, 48, FindTexture(TEXT("NWindow.WhiteTexture")));
            for (INT k = 0; k < Reading.Len(); k++)
            {
                if (k >= ReadingParam)
                {
                    Device->DrawStretchedTex(RX, RY, 12, 12, FindTexture(TEXT("NWindow.GrayTexture")));
                    Device->DrawText(RX, RY, 0xFFFF0000, *Reading.Mid(k, 1), 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0,
                                     0, 0);
                }
                else
                    Device->DrawText(RX, RY, 0xFF0000FF, *Reading.Mid(k, 1), 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0,
                                     0, 0);
                RY += 12;
            }
            Device->ReleaseClip();
        }
        else if (GConsole->GetIMEType() == 4)
        {
            Device->DrawStretchedTex(RX, Y, ReadingW, 12, FindTexture(TEXT("NWindow.GrayTexture")));
            Device->DrawText(RX, Y, 0xFF000000, *Reading, 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
        }
    }
}
NW_HOOK_METHOD(0x1001e670, NCMultiEdit, PaintCompositionTW)

// 0x1001f080 - the Chinese IME's part (nothing composed hides the composition window). The IME type 8 draws the
// row's part of the composition with a dotted underline; the first target character (or typed one: that test
// reads the attribute at the row's index, not the composition's, as in the stock code) goes black on white with the
// IME's caret 5 px past its start and 15 px down, where the composition window shows the reading string (hidden
// without one) and the candidate box goes; the caret follows the row's part when the whole composition has
// neither. The types 9 to 12 put the composition window at the row's start 15 px down (9 and 10 fill it with the
// row's part), the candidate box 105 px right of the row's start, and blink the caret at the row's start. Only the
// type 10 shows the window's caret.
void NCMultiEdit::PaintCompositionCN(FString Text, INT CompStart, INT X, INT Y, INT RowNum, NCGDevice* Device, INT* Out)
{
    if (IMEManager->IsNotComposing())
    {
        ChineseCompositeBox(IMEManager)->HideWindow();
        return;
    }
    NCChineseIME* IME = (NCChineseIME*)IMEManager->ChineseIME;
    NCCandidateBox* Cand = IME->CandidateBox;
    INT CandW = 0;
    INT CandH = 0;
    if (Cand)
    {
        CandH = Cand->Height;
        CandW = Cand->Width;
    }
    NCCompositeBox* Box = IME->CompositeBox;
    if (GConsole->GetIMEType() == 10 || GConsole->GetIMEType() == 9)
    {
        IMECaretX = LineGap + X + 105;
        IMECaretY = Y + 15;
        Box->SetText(*Text);
        Box->SetWindowRect(LineGap + X, IMECaretY, 100, 15);
        Box->ShowWindow();
        if (Cand)
            Cand->SetWindowRect(IMECaretX, IMECaretY, CandW, CandH);
        if (IsFocused() && bCaretOn)
            Device->DrawText(LineGap + X - 2, Y, 0xFFFFFFFF, TEXT("|"), 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
    }
    else if (GConsole->GetIMEType() == 11 || GConsole->GetIMEType() == 12)
    {
        IMECaretX = LineGap + X + 105;
        IMECaretY = Y + 15;
        Box->SetWindowRect(LineGap + X, Y + 15, 100, 15);
        Box->ShowWindow();
        if (Cand)
            Cand->SetWindowRect(IMECaretX, IMECaretY, CandW, CandH);
        if (IsFocused() && bCaretOn)
            Device->DrawText(LineGap + X - 2, Y, 0xFFFFFFFF, TEXT("|"), 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
    }
    else if (GConsole->GetIMEType() == 8)
    {
        SIZE Size;
        FString Ch;
        Device->DrawText(LineGap + X, Y, 0xFFFFFFFF, *Text, 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
        INT Offset = 0;
        NCGDevice::GetTextExtent(&Size, *Text, 0, 0, 0, 0, 0, L2FONT_Normal);
        INT TextW = Size.cx;
        for (INT k = 0; k < TextW; k += 3)
            Device->DrawText(LineGap + k - 2 + X, Y + 3, 0xFFFFFFFF, TEXT("."), 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0,
                             0, 0);
        INT i;
        for (i = 0; i < Text.Len(); i++)
        {
            Ch = Text.Mid(i, 1);
            NCGDevice::GetTextExtent(&Size, *Ch, 0, 0, 0, 0, 0, L2FONT_Normal);
            NCIME* CN = IMEManager->ChineseIME;
            if (CN->CompAttr[i + CompStart] == ATTR_TARGET_CONVERTED || CN->CompAttr[i] == ATTR_INPUT)
            {
                IMECaretX = LineGap + Offset + 5 + X;
                IMECaretY = Y + 15;
                break;
            }
            Offset += Size.cx;
        }
        Ch = Text.Mid(i, 1);
        NCGDevice::GetTextExtent(&Size, *Ch, 0, 0, 0, 0, 0, L2FONT_Normal);
        INT CharW = Size.cx;
        if (IsFocused() && bCaretOn && i == Comp.Len() - CompStart)
        {
            INT j;
            for (j = 0; j < Comp.Len(); j++)
            {
                BYTE Attr = IMEManager->ChineseIME->CompAttr[j];
                if (Attr == ATTR_TARGET_CONVERTED || Attr == ATTR_INPUT)
                    break;
            }
            if (j == Comp.Len())
                Device->DrawText(LineGap + TextW + X - 2, Y, 0xFFFFFFFF, TEXT("|"), 0, 0, 0, L2FONT_Normal, 0, 0,
                                 0, 0, 0, 0);
        }
        Device->DrawStretchedTex(LineGap + Offset + X, Y, CharW, 12, FindTexture(TEXT("NWindow.WhiteTexture")));
        Device->DrawText(LineGap + Offset + X, Y, 0xFF000000, *Text.Mid(i, 1), 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0,
                         0, 0);
        Box->SetText(*Reading);
        Box->SetWindowRect(IMECaretX, IMECaretY, 50, 15);
        if (Reading.Len())
            Box->ShowWindow();
        else
            Box->HideWindow();
        if (Cand)
            Cand->SetWindowRect(IMECaretX, IMECaretY, CandW, CandH);
    }
    if (GConsole->GetIMEType() == 10)
        Box->SetShowCaret(1);
    else
        Box->SetShowCaret(0);
}
NW_HOOK_METHOD(0x1001f080, NCMultiEdit, PaintCompositionCN)

// 0x1001f700 (vslot 66) - rows per notch; scrolling down is bounded by the number of *lines*
// (not rows)
INT NCMultiEdit::OnMouseWheel(WPARAM wParam, LPARAM lParam)
{
    SWORD Delta = (SWORD)((SWORD)HIWORD(wParam) / 120);
    if (Delta > 0)
    {
        if (TopLine > Delta)
            TopLine -= Delta;
        else
            TopLine = 0;
    }
    else
    {
        INT Visible = VisibleRows;
        INT D = Delta;
        if (Lines.Num() >= Visible)
        {
            if (Lines.Num() + (D - TopLine) > Visible)
                TopLine -= D;
            else
                TopLine = Lines.Num() - Visible;
        }
        else if (Lines.Num() + (D - TopLine) > Visible)
            TopLine -= D;
        else
            TopLine = 0;
    }
    ScrollBar->SetValue(TopLine);
    return 1;
}
NW_HOOK_METHOD(0x1001f700, NCMultiEdit, OnMouseWheel)

// 0x1001f7d0 (vslot 69)
INT NCMultiEdit::OnScrollUp()
{
    if (Lines.Num() > VisibleRows)
    {
        if (--TopLine < 0)
            TopLine = 0;
        ScrollBar->SetValue(TopLine);
    }
    return 1;
}
NW_HOOK_METHOD(0x1001f7d0, NCMultiEdit, OnScrollUp)

// 0x1001f810 (vslot 70)
INT NCMultiEdit::OnScrollDown()
{
    INT Visible = VisibleRows;
    if (Lines.Num() > Visible)
    {
        TopLine++;
        if (TopLine > Lines.Num() - Visible)
            TopLine = Lines.Num() - Visible;
        ScrollBar->SetValue(TopLine);
    }
    return 1;
}
NW_HOOK_METHOD(0x1001f810, NCMultiEdit, OnScrollDown)

// 0x1001f870 - repeat period of both scroll buttons
void NCMultiEdit::SetRepeatDelay(INT Delay)
{
    if (UpButton && DownButton)
    {
        UpButton->RepeatDelay = Delay;
        DownButton->RepeatDelay = Delay;
    }
}
NW_HOOK_METHOD(0x1001f870, NCMultiEdit, SetRepeatDelay)

// 0x1001f8a0 - the range is given the scroll position for a total (sic)
void NCMultiEdit::InitScrollBar()
{
    SetRepeatDelay(200);
    ScrollBar->SetRange(TopLine, VisibleRows);
    ScrollBar->SetValue(TopLine);
}
NW_HOOK_METHOD(0x1001f8a0, NCMultiEdit, InitScrollBar)

// 0x1001f8e0 (vslot 68) - the bar counts lines, not rows
void NCMultiEdit::UpdateScrollBar()
{
    ScrollBar->SetRange(Lines.Num(), VisibleRows);
    ScrollBar->SetValue(TopLine);
}
NW_HOOK_METHOD(0x1001f8e0, NCMultiEdit, UpdateScrollBar)

// 0x1001fc00 - screen rows of a line; with bComp the composition is counted at the caret
INT NCMultiEdit::CountRows(INT Line, UBOOL bComp)
{
    if (Line >= NumLines)
        return 0;
    INT X = 0;
    INT Rows = 1;
    FString Text(Lines.GetLine(Line)->Text);
    FString Ch;
    SIZE Size;
    for (INT i = 0; i <= Text.Len(); i++)
    {
        if (bComp && Line == CaretLine && i == CaretCol)
        {
            for (INT k = 0; k < Comp.Len(); k++)
            {
                Ch = Comp.Mid(k, 1);
                NCGDevice::GetTextExtent(&Size, *Ch, 0, 0, 0, 0, 0, L2FONT_Normal);
                X += Size.cx;
                if (X > Width - 36)
                {
                    Rows++;
                    X = Size.cx;
                }
            }
        }
        if (i == Text.Len())
            break;
        Ch = Text.Mid(i, 1);
        NCGDevice::GetTextExtent(&Size, *Ch, 0, 0, 0, 0, 0, L2FONT_Normal);
        X += Size.cx;
        if (X > Width - 36)
        {
            Rows++;
            X = Size.cx;
        }
    }
    return Rows;
}
NW_HOOK_METHOD(0x1001fc00, NCMultiEdit, CountRows)

// 0x1001fe20 - adds the rows a line gained since OldRows to TotalRows (it never takes any away here)
void NCMultiEdit::UpdateRowCount(INT Line, INT OldRows, UBOOL bComp)
{
    INT Rows = CountRows(Line, bComp);
    if (OldRows < Rows)
        TotalRows += Rows - OldRows;
}
NW_HOOK_METHOD(0x1001fe20, NCMultiEdit, UpdateRowCount)

// 0x1001ee80 (name ours) - the length of a line without its '\n' or '\r' (0 for an empty line)
INT NCMultiEdit::LineLength(FString Line)
{
    if (Line.Num() != 0 && Line.Num() - 1 > 0)
    {
        INT Length = Line.Num() - 1;
        TCHAR Last = (*Line)[Length - 1];
        if (Last == '\n' || Last == '\r')
            Length--;
        return Length;
    }
    return 0;
}
NW_HOOK_METHOD(0x1001ee80, NCMultiEdit, LineLength)

// 0x1001fe50 - counts every screen row again and puts the caret on its row. A character that does not fit is
// measured again on the next row. A line ending in '\n' or '\r' adds a row. With Param set the caret is not
// moved once its line has wrapped before it. The size of a line that cannot be fetched is the last one measured.
// C2: the columns of a line run to its length without the line break, inclusive.
void NCMultiEdit::RecalcRows(INT Param)
{
    INT Wraps = 0;
    TotalRows = 1;
    UBOOL bCaretPlaced = 0;
    SIZE Size;
    Size.cx = 0;
    Size.cy = 0;
    for (INT Line = 0; Line < Lines.Num(); Line++)
    {
        INT X = 0;
        INT LineLen = LineLength(Lines.GetLine(Line)->Text);
        INT Col = 0;
        if (LineLen >= 0)
        {
            do
            {
                if (Line == CaretLine && Col == CaretCol)
                {
                    if (!bCaretPlaced && (!Param || !Wraps))
                    {
                        CaretX = X;
                        CaretRow = TotalRows - 1;
                    }
                    if (IMEManager->IsNotComposing() == 0)
                    {
                        if (!bCaretPlaced)
                        {
                            // the IME's cursor inside the composition gets the caret
                            NCIME* IME = NULL;
                            if (IMEManager)
                            {
                                if (GLanguageType == 2)
                                    IME = IMEManager->JapaneseIME;
                                else if (GLanguageType == 0)
                                    IME = IMEManager->KoreanIME;
                                else if (GLanguageType == 3)
                                    IME = IMEManager->TaiwanIME;
                            }
                            for (INT k = 0; k < Comp.Len() + 1; k++)
                            {
                                if (IME && IME->CursorPos == k)
                                {
                                    CaretX = X;
                                    CaretRow = TotalRows - 1;
                                }
                                if (GLanguageType != 4 || GConsole->GetIMEType() == 8)
                                {
                                    NCGDevice::GetTextExtent(&Size, *Comp.Mid(k, 1), 0, 0, 0, 0, 0, L2FONT_Normal);
                                    X += Size.cx;
                                    if (X > Width - 36)
                                    {
                                        X = 0;
                                        TotalRows++;
                                        k--;
                                    }
                                }
                            }
                        }
                        if (Lines.GetLine(Line))
                            NCGDevice::GetTextExtent(&Size, *Lines.GetLine(Line)->Text.Mid(Col, 1), 0, 0, 0, 0, 0,
                                                     L2FONT_Normal);
                        X += Size.cx;
                        if (X > Width - 36)
                        {
                            bCaretPlaced = 1;
                            X = 0;
                            TotalRows++;
                            Col--;
                        }
                    }
                    else
                    {
                        if (Lines.GetLine(Line))
                            NCGDevice::GetTextExtent(&Size, *Lines.GetLine(Line)->Text.Mid(Col, 1), 0, 0, 0, 0, 0,
                                                     L2FONT_Normal);
                        X += Size.cx;
                        if (X > Width - 36)
                        {
                            X = 0;
                            Col--;
                            Wraps++;
                            TotalRows++;
                        }
                    }
                }
                else
                {
                    if (Lines.GetLine(Line))
                        NCGDevice::GetTextExtent(&Size, *Lines.GetLine(Line)->Text.Mid(Col, 1), 0, 0, 0, 0, 0,
                                                 L2FONT_Normal);
                    X += Size.cx;
                    if (X > Width - 36)
                    {
                        X = 0;
                        TotalRows++;
                        Col--;
                    }
                }
                Col++;
            } while (Col <= LineLen);
        }
        if (Lines.GetLine(Line))
        {
            INT Num = Lines.GetLine(Line)->Text.Num();
            if (Num != 0 && Num != 1)
            {
                INT Last = Lines.GetLine(Line)->Text.Len() - 1;
                if (Lines.GetLine(Line)->Text.Mid(Last, 1) == TEXT("\n") ||
                    Lines.GetLine(Line)->Text.Mid(Lines.GetLine(Line)->Text.Len() - 1, 1) == TEXT("\r"))
                    TotalRows++;
            }
        }
    }
    ScrollToCaret();
}
NW_HOOK_METHOD(0x1001fe50, NCMultiEdit, RecalcRows)

// 0x10020390 - a result string goes into the line at the caret (the composition keeps what is left of it); a
// composition string only replaces Comp
void NCMultiEdit::InsertString(TCHAR* Str, UBOOL bResult)
{
    if (bResult)
    {
        if (Str && Lines.GetLine(CaretLine))
        {
            INT OldRows = CountRows(CaretLine, 1);
            FString Line(Lines.GetLine(CaretLine)->Text);
            INT Length = wcslen(Str);
            FString S(Str);
            FString L = Line.Left(CaretCol);
            FString R = Line.Right(Line.Len() - CaretCol);
            Lines.GetLine(CaretLine)->Text = L + S + R;
            CaretCol += Length;
            for (INT i = 0; i < S.Len(); i++)
            {
                SIZE Size;
                NCGDevice::GetTextExtent(&Size, *S.Mid(i, 1), 0, 0, 0, 0, 0, L2FONT_Normal);
                CaretX += Size.cx;
                if (CaretX > Width - 36)
                {
                    CaretRow++;
                    CaretX = Size.cx;
                }
            }
            if (Comp.Len() > appStrlen(Str))
                Comp = Comp.Right(Comp.Len() - appStrlen(Str));
            else
                Comp.Empty();
            UpdateRowCount(CaretLine, OldRows, 1);
        }
    }
    else if (Str)
    {
        INT OldRows = CountRows(CaretLine, 1);
        Comp = FString(Str);
        UpdateRowCount(CaretLine, OldRows, 1);
    }
    ScrollToCaret();
}
NW_HOOK_METHOD(0x10020390, NCMultiEdit, InsertString)

// 0x100206c0 (vslot 36) - NControl::OnCreate is not called. C2: another background texture, and the Thai client's
// text height is 12 whatever the font says (the scroll controls and the rows follow it)
INT NCMultiEdit::OnCreate()
{
    BackTex = FindTexture(TEXT("L2UI.EtcWnd.Edit_Back"));
    WhiteTex = FindTexture(TEXT("NWindow.WhiteTexture"));
    IMEManager = new NCIMEManager(0);
    CreateChild(IMEManager, NWS_VISIBLE);
    CaretTimer = SetTimer(TIMER_MultiEditCaret, 500, 0);
    SIZE Size;
    NCGDevice::GetTextExtent(&Size, TEXT("\xAC00"), 0, 0, 0, 0, 0, L2FONT_Normal);
    if (GLanguageType == 5)
        CharH = 12;
    else
        CharH = Size.cy;
    CharH2 = Size.cy;
    CharW = Size.cx;
    INT W = LocalW;
    INT H = LocalH;
    LineGap = 2;
    NumLines = 1;
    VisibleRows = (Height - 2) / (CharH + 2);

    UpButton = new NCPushButton(TEXT("L2UI.NWindow.WindowScrollUp"), TEXT("L2UI.NWindow.WindowScrollUp"));
    UpButton->ID = NID_MULTIEDIT_UP;
    UpButton->SetWindowRect(W + 2, 0, 12, 12);
    CreateChild(UpButton, NWS_VISIBLE);

    DownButton = new NCPushButton(TEXT("L2UI.NWindow.WindowScrollDown"), TEXT("L2UI.NWindow.WindowScrollDown"));
    DownButton->ID = NID_MULTIEDIT_DOWN;
    DownButton->SetWindowRect(W + 2, H - 12, 12, 12);
    CreateChild(DownButton, NWS_VISIBLE);

    ScrollBar = new NCSliderBar(TEXT("L2UI.NWindow.WindowScrollBar1"), TEXT("L2UI.NWindow.WindowScrollBar2"),
                                TEXT("L2UI.NWindow.WindowScrollBar3"));
    ScrollBar->ID = NID_MULTIEDIT_BAR;
    ScrollBar->SetWindowRect(W + 2, 12, 12, H >= 24 ? H - 24 : 0);
    CreateChild(ScrollBar, NWS_VISIBLE);
    ScrollBar->DoNothing(0);
    InitScrollBar();
    return 1;
}
NW_HOOK_METHOD(0x100206c0, NCMultiEdit, OnCreate)

// 0x10020d60 - x of a caret column, the line wrapped as it is drawn; a column past the end leaves CaretX alone
void NCMultiEdit::SetCaretX(INT Line, INT Col)
{
    if (Line < NumLines)
    {
        FString Text;
        FString Ch;
        if (Lines.GetLine(Line))
            Text = Lines.GetLine(Line)->Text;
        INT X = 0;
        if (Col <= Text.Len())
        {
            for (INT i = 0; i < Col; i++)
            {
                SIZE Size;
                Ch = Text.Mid(i, 1);
                NCGDevice::GetTextExtent(&Size, *Ch, 0, 0, 0, 0, 0, L2FONT_Normal);
                X += Size.cx;
                if (X > Width - 36)
                    X = Size.cx;
            }
            CaretX = X;
        }
    }
}
NW_HOOK_METHOD(0x10020d60, NCMultiEdit, SetCaretX)

// 0x10020ea0 - column of its line where a screen row starts (-1 past the text); bCompRow is set when the row
// starts inside the composition, the result is then an index into Comp
INT NCMultiEdit::GetRowStart(INT Row)
{
    INT CurRow = 0;
    if (Row >= TotalRows || Lines.Num() <= 0)
    {
        bCompRow = 0;
        return -1;
    }
    INT Line = 0;
    do
    {
        if (CurRow == Row)
        {
            if (Line == CaretLine && CaretCol == 0 && !IMEManager->IsNotComposing())
            {
                bCompRow = 1;
                return 0;
            }
            bCompRow = 0;
            return 0;
        }
        INT X = 0;
        INT LineLen = 0;
        if (Lines.GetLine(Line))
            LineLen = Lines.GetLine(Line)->Text.Len() + 1;
        for (INT Col = 0; Col < LineLen; Col++)
        {
            SIZE Size;
            if (Line == CaretLine && Col == CaretCol && !IMEManager->IsNotComposing() &&
                !IsWindowedIME(GConsole->GetIMEType()))
            {
                for (INT k = 0; k < Comp.Len(); k++)
                {
                    NCGDevice::GetTextExtent(&Size, *Comp.Mid(k, 1), 0, 0, 0, 0, 0, L2FONT_Normal);
                    X += Size.cx;
                    if (X > Width - 36)
                    {
                        X = 0;
                        if (++CurRow == Row)
                        {
                            bCompRow = 1;
                            return k;
                        }
                        k--;
                    }
                }
            }
            if (Lines.GetLine(Line))
                NCGDevice::GetTextExtent(&Size, *Lines.GetLine(Line)->Text.Mid(Col, 1), 0, 0, 0, 0, 0,
                                         L2FONT_Normal);
            else
                Size.cx = 0;
            X += Size.cx;
            if (X > Width - 36)
            {
                X = 0;
                if (++CurRow == Row)
                {
                    bCompRow = 0;
                    return Col;
                }
                Col--;
            }
        }
        CurRow++;
        Line++;
    } while (Line < Lines.Num());
    bCompRow = 0;
    return -1;
}
NW_HOOK_METHOD(0x10020ea0, NCMultiEdit, GetRowStart)

// 0x10021220 - column where the row starting at Start of Line ends (0: at the end of the line, -1 past the text).
// A row that starts inside the composition (bCompRow) measures the rest of it first; one that breaks inside
// the composition returns an index into Comp with bCompRow set. The line is fetched without a NULL test.
INT NCMultiEdit::GetRowEnd(INT Start, INT Line)
{
    INT X = 0;
    if (Line >= Lines.Num())
    {
        bCompRow = 0;
        return -1;
    }
    SIZE Size;
    if (bCompRow && !IMEManager->IsNotComposing() && Line == CaretLine)
    {
        if (!IsWindowedIME(GConsole->GetIMEType()))
        {
            INT CompLen = Comp.Len();
            for (INT i = Start; i < CompLen; i++)
            {
                NCGDevice::GetTextExtent(&Size, *Comp.Mid(i, 1), 0, 0, 0, 0, 0, L2FONT_Normal);
                X += Size.cx;
                if (X > Width - 36)
                    return i;
            }
        }
        Start = CaretCol;
    }
    INT LineLen = Lines.GetLine(Line)->Text.Len();
    for (INT Col = Start; Col < LineLen + 1; Col++)
    {
        if (Line == CaretLine && !bCompRow && Col == CaretCol && !IMEManager->IsNotComposing() &&
            !IsWindowedIME(GConsole->GetIMEType()) && CaretCol != Start)
        {
            for (INT k = 0; k < Comp.Len(); k++)
            {
                NCGDevice::GetTextExtent(&Size, *Comp.Mid(k, 1), 0, 0, 0, 0, 0, L2FONT_Normal);
                X += Size.cx;
                if (X > Width - 36)
                {
                    bCompRow = 1;
                    return k;
                }
            }
        }
        if (Lines.GetLine(Line))
            NCGDevice::GetTextExtent(&Size, *Lines.GetLine(Line)->Text.Mid(Col, 1), 0, 0, 0, 0, 0,
                                     L2FONT_Normal);
        else
            Size.cx = 0;
        X += Size.cx;
        if (X > Width - 36)
        {
            bCompRow = 0;
            return Col;
        }
    }
    if (Line + 1 == CaretLine && CaretCol == 0 && !IMEManager->IsNotComposing())
        bCompRow = 1;
    else
        bCompRow = 0;
    return 0;
}
NW_HOOK_METHOD(0x10021220, NCMultiEdit, GetRowEnd)

// 0x100215e0 - line of a screen row (-1 past the text); unlike GetRowStart it does not count the position after
// a line's last character
INT NCMultiEdit::GetRowLine(INT Row)
{
    INT CurRow = 0;
    if (Row >= TotalRows || Lines.Num() <= 0)
        return -1;
    INT Line = 0;
    do
    {
        if (CurRow == Row)
            return Line;
        INT X = 0;
        INT LineLen = 0;
        if (Lines.GetLine(Line))
            LineLen = Lines.GetLine(Line)->Text.Len();
        for (INT Col = 0; Col < LineLen; Col++)
        {
            SIZE Size;
            if (Line == CaretLine && Col == CaretCol && !IMEManager->IsNotComposing() &&
                !IsWindowedIME(GConsole->GetIMEType()))
            {
                for (INT k = 0; k < Comp.Len(); k++)
                {
                    NCGDevice::GetTextExtent(&Size, *Comp.Mid(k, 1), 0, 0, 0, 0, 0, L2FONT_Normal);
                    X += Size.cx;
                    if (X > Width - 36)
                    {
                        X = 0;
                        if (++CurRow == Row)
                            return Line;
                        k--;
                    }
                }
            }
            if (Lines.GetLine(Line))
                NCGDevice::GetTextExtent(&Size, *Lines.GetLine(Line)->Text.Mid(Col, 1), 0, 0, 0, 0, 0,
                                         L2FONT_Normal);
            else
                Size.cx = 0;
            X += Size.cx;
            if (X > Width - 36)
            {
                X = 0;
                if (++CurRow == Row)
                    return Line;
                Col--;
            }
        }
        CurRow++;
        Line++;
    } while (Line < Lines.Num());
    return -1;
}
NW_HOOK_METHOD(0x100215e0, NCMultiEdit, GetRowLine)

// 0x100218f0 (vslot 73)
INT NCMultiEdit::OnResult(WPARAM wParam, LPARAM lParam)
{
    bReading = 0;
    Reading.Empty();
    TCHAR* Str = IMEManager->GetResultStr();
    if (Str)
    {
        InsertString(Str, 1);
        return 1;
    }
    Comp.Empty();
    return 1;
}
NW_HOOK_METHOD(0x100218f0, NCMultiEdit, OnResult)

// 0x10021940 (vslot 72) - the Chinese layouts show the composition in the IME's window, whose
// caret follows the length change
INT NCMultiEdit::OnComposition(WPARAM wParam, LPARAM lParam)
{
    bReading = 0;
    Reading.Empty();
    TCHAR* Str = IMEManager->GetCompStr();
    if (Str)
        InsertString(Str, 0);
    else
        Comp.Empty();
    if (GConsole->GetIMEType() == 10 || GConsole->GetIMEType() == 9)
    {
        NCCompositeBox* Box = ChineseCompositeBox(IMEManager);
        FString Old(ChineseLastComp(IMEManager));
        Box->SetText(*Comp);
        if (Old.Len() > Comp.Len())
            Box->CaretLeft();
        else if (Old.Len() < Comp.Len())
            Box->CaretRight();
        Box->SetText(*Comp);
    }
    return 1;
}
NW_HOOK_METHOD(0x10021940, NCMultiEdit, OnComposition)

// 0x10021a80 - the lines joined (their '\n' included) in a buffer the box keeps; NULL when
// empty
TCHAR* NCMultiEdit::GetText()
{
    if (TextBuf)
    {
        delete[] TextBuf;
        TextBuf = NULL;
    }
    UINT Total = Lines.TotalLength();
    if (!Total)
        return NULL;
    TextBuf = new TCHAR[Total + 1];
    INT N = Lines.Num();
    INT Pos = 0;
    for (INT i = 0; i < N; i++)
    {
        INT Length = 0;
        if (Lines.GetLine(i))
        {
            Length = Lines.GetLine(i)->Text.Len();
            appMemcpy(TextBuf + Pos, *Lines.GetLine(i)->Text, Length * sizeof(TCHAR));
        }
        Pos += Length;
    }
    TextBuf[Pos] = 0;
    return TextBuf;
}
NW_HOOK_METHOD(0x10021a80, NCMultiEdit, GetText)

// 0x10021e10 - a character at the caret; '\n' or '\r' ends the line there (and stays at its end), the rest of it
// going to a new line. C2: the width is measured on the formatted string (not on the code), CountRows counts the
// composition (bComp).
void NCMultiEdit::InsertChar(TCHAR Ch)
{
    FString Right;
    FString Left;
    TCHAR Buf[260];
    appSprintf(Buf, TEXT("%c"), Ch);
    FString Str(Buf);
    if (Ch == '\n' || Ch == '\r')
    {
        Lines.Insert(CaretLine + 1);
        if (Lines.GetLine(CaretLine + 1) && Lines.GetLine(CaretLine))
        {
            Lines.GetLine(CaretLine + 1)->Text =
                Lines.GetLine(CaretLine)->Text.Right(Lines.GetLine(CaretLine)->Text.Len() - CaretCol);
            Lines.GetLine(CaretLine)->Text = Lines.GetLine(CaretLine)->Text.Left(CaretCol) + Str;
        }
        CaretLine++;
        NumLines = Lines.Num();
        CaretCol = 0;
        CaretX = 0;
        CaretRow++;
        TotalRows++;
    }
    else
    {
        INT OldRows = CountRows(CaretLine, 1);
        if (Lines.GetLine(CaretLine))
        {
            Left = Lines.GetLine(CaretLine)->Text.Left(CaretCol);
            Right = Lines.GetLine(CaretLine)->Text.Right(Lines.GetLine(CaretLine)->Text.Len() - CaretCol);
            Lines.GetLine(CaretLine)->Text = Left + Str + Right;
        }
        CaretCol++;
        SIZE Size;
        NCGDevice::GetTextExtent(&Size, *Str, 0, 0, 0, 0, 0, L2FONT_Normal);
        CaretX += Size.cx;
        if (CaretX > Width - 36)
        {
            CaretRow++;
            CaretX = Size.cx;
        }
        UpdateRowCount(CaretLine, OldRows, 0);
    }
    ScrollToCaret();
}
NW_HOOK_METHOD(0x10021e10, NCMultiEdit, InsertChar)

// 0x10022230 - the characters one by one, as typed
void NCMultiEdit::SetText(const TCHAR* Text)
{
    INT Length = wcslen(Text);
    for (INT i = 0; i < Length; i++)
        InsertChar(Text[i]);
    UpdateScrollBar();
}
NW_HOOK_METHOD(0x10022230, NCMultiEdit, SetText)

// C2 CANDIDATE: the C2 function is not the same code as the C1 one; kept only while it matches the stock one (verify_data)
// 0x10022280 (vslot 37)
INT NCMultiEdit::OnDestroy()
{
    CaretLine = 0;
    CaretCol = 0;
    Lines.Clear();
    if (TextBuf)
    {
        delete[] TextBuf;
        TextBuf = NULL;
    }
    if (CaretTimer)
    {
        KillTimer(CaretTimer);
        CaretTimer = 0;
    }
    if (UpButton)
    {
        RemoveChildWindow(UpButton->Entry);
        if (UpButton)
            delete UpButton;
        UpButton = NULL;
    }
    if (DownButton)
    {
        RemoveChildWindow(DownButton->Entry);
        if (DownButton)
            delete DownButton;
        DownButton = NULL;
    }
    if (ScrollBar)
    {
        RemoveChildWindow(ScrollBar->Entry);
        if (ScrollBar)
            delete ScrollBar;
        ScrollBar = NULL;
    }
    if (IMEManager)
    {
        RemoveChildWindow(IMEManager->Entry);
        if (IMEManager)
            delete IMEManager;
        IMEManager = NULL;
    }
    NControl::OnDestroy();
    return 1;
}
NW_HOOK_METHOD(0x10022280, NCMultiEdit, OnDestroy)

// 0x10022400 - one empty line again
void NCMultiEdit::Clear()
{
    CaretLine = 0;
    CaretCol = 0;
    Lines.Clear();
    Lines.Insert(-1);
    NumLines = 1;
    TotalRows = 1;
    CaretLine = 0;
    CaretCol = 0;
    TopLine = 0;
    CaretX = 0;
    CaretRow = 0;
    bCompRow = 0;
    bReading = 0;
    if (TextBuf)
    {
        delete[] TextBuf;
        TextBuf = NULL;
    }
}
NW_HOOK_METHOD(0x10022400, NCMultiEdit, Clear)

// The rows shown, from TopLine: the text of each row, the composition drawn at the caret by the
// painter of the client language, and the caret (Japanese: drawn apart). The width of what is
// drawn is kept across rows (TextW) and used where the stock code uses it.
static FORCEINLINE void PaintMultiEditText(NCMultiEdit* E, NCGDevice* Device)
{
    Device->DrawStretchedTex(0, 0, E->Width, E->Height, E->BackTex);
    INT Y = E->LineGap;
    INT TextW = 0;
    FString Seg;
    INT PainterOut = 0;
    INT Start = E->GetRowStart(E->TopLine);
    INT Line = E->GetRowLine(E->TopLine);
    for (INT Row = E->TopLine; Row < E->TopLine + E->VisibleRows; Row++)
    {
        if (Row >= E->TotalRows)
            break;
        INT RowNum = Row - E->TopLine + 1;
        INT End;
        if (E->bCompRow)
        {
            // the row starts inside the composition
            End = E->GetRowEnd(Start, Line);
            if (E->bCompRow)
            {
                // ... and ends inside it
                Seg = E->Comp.Mid(Start, End - Start);
                if (Seg.Len())
                {
                    if (GLanguageType == 2)
                    {
                        Device->DrawText(E->LineGap, Y, 0xFFFFFFFF, *Seg, 0, 0, 0, L2FONT_Normal, 0, 0,
                                         0, 0, 0, 0);
                        E->PaintCompositionJP(Seg, Start, 0, Y, RowNum, Device, &PainterOut);
                    }
                    else if (GLanguageType == 3)
                    {
                        Device->DrawText(E->LineGap, Y, 0xFFFFFFFF, *Seg, 0, 0, 0, L2FONT_Normal, 0, 0,
                                         0, 0, 0, 0);
                        E->PaintCompositionTW(Seg, Start, 0, Y, RowNum, Device, &PainterOut);
                    }
                    else if (GLanguageType == 4)
                        E->PaintCompositionCN(Seg, Start, 0, Y, RowNum, Device, &PainterOut);
                    else if (GLanguageType == 0)
                        E->PaintCompositionKR(Seg, 0, Y, Device);
                    else
                        Device->DrawText(E->LineGap, Y, 0xFFFFFFFF, *Seg, 0, 0, 0, L2FONT_Normal, 0, 0,
                                         0, 0, 0, 0);
                }
            }
            else
            {
                // ... and the line's text follows the rest of it
                Seg = E->Comp.Mid(Start, E->Comp.Len() - Start);
                TextW = 0;
                if (GLanguageType == 2)
                {
                    TextW = Device->DrawText(E->LineGap, Y, 0xFFFFFFFF, *Seg, 0, 0, 0, L2FONT_Normal, 0,
                                             0, 0, 0, 0, 0);
                    E->PaintCompositionJP(Seg, Start, 0, Y, RowNum, Device, &PainterOut);
                }
                else if (GLanguageType == 3)
                {
                    TextW = Device->DrawText(E->LineGap, Y, 0xFFFFFFFF, *Seg, 0, 0, 0, L2FONT_Normal, 0,
                                             0, 0, 0, 0, 0);
                    E->PaintCompositionTW(Seg, Start, 0, Y, RowNum, Device, &PainterOut);
                }
                else if (GLanguageType == 4)
                    E->PaintCompositionCN(Seg, Start, 0, Y, RowNum, Device, &PainterOut);
                else if (GLanguageType == 0)
                {
                    TextW = Device->DrawText(E->LineGap, Y, 0xFFFFFFFF, *Seg, 0, 0, 0, L2FONT_Normal, 0,
                                             0, 0, 0, 0, 0);
                    E->PaintCompositionKR(Seg, 0, Y, Device);
                }
                else
                    TextW = Device->DrawText(E->LineGap, Y, 0xFFFFFFFF, *Seg, 0, 0, 0, L2FONT_Normal, 0,
                                             0, 0, 0, 0, 0);
                if (E->Lines.GetLine(Line))
                {
                    if (End > 0)
                        Seg = E->Lines.GetLine(Line)->Text.Mid(E->CaretCol, End - E->CaretCol);
                    else
                        Seg = E->Lines.GetLine(Line)->Text.Mid(
                            E->CaretCol, E->Lines.GetLine(Line)->Text.Len() - E->CaretCol);
                    if (Seg.Len())
                        Device->DrawText(E->LineGap + TextW, Y, 0xFFFFFFFF, *Seg, 0, 0, 0,
                                         L2FONT_Normal, 0, 0, 0, 0, 0, 0);
                }
            }
            if (Row == E->CaretRow && E->bCaretOn && GLanguageType == 2)
                Device->DrawText(E->CaretX + E->LineGap - 3, Y, 0xFFFFFFFF, TEXT("|"), 0, 0, 0,
                                 L2FONT_Normal, 0, 0, 0, 0, 0, 0);
        }
        else if (Row == E->CaretRow)
        {
            // the caret's row: the text before the caret, the composition, the rest
            TextW = 0;
            if (E->CaretX && E->Lines.GetLine(Line))
            {
                Seg = E->Lines.GetLine(Line)->Text.Mid(Start, E->CaretCol - Start);
                TextW = Device->DrawText(E->LineGap, Y, 0xFFFFFFFF, *Seg, 0, 0, 0, L2FONT_Normal, 0, 0,
                                         0, 0, 0, 0);
            }
            if (!E->IMEManager->IsNotComposing())
            {
                End = E->GetRowEnd(Start, Line);
                if (E->bCompRow)
                {
                    // the composition goes on in the next row
                    if (GLanguageType == 2)
                    {
                        Device->DrawText(E->LineGap + TextW, Y, 0xFFFFFFFF, *E->Comp.Mid(0, End), 0, 0,
                                         0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
                        E->PaintCompositionJP(E->Comp.Mid(0, End), 0, TextW, Y, RowNum, Device,
                                              &PainterOut);
                    }
                    else if (GLanguageType == 3)
                    {
                        Device->DrawText(E->LineGap + TextW, Y, 0xFFFFFFFF, *E->Comp.Mid(0, End), 0, 0,
                                         0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
                        E->PaintCompositionTW(E->Comp.Mid(0, End), 0, TextW, Y, RowNum, Device,
                                              &PainterOut);
                    }
                    else if (GLanguageType == 4)
                        E->PaintCompositionCN(E->Comp.Mid(0, End), 0, TextW, Y, RowNum, Device,
                                              &PainterOut);
                    else
                        Device->DrawText(E->LineGap + TextW, Y, 0xFFFFFFFF, *E->Comp.Mid(0, End), 0, 0,
                                         0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
                }
                else
                {
                    INT CompW = 0;
                    if (GLanguageType == 2)
                    {
                        CompW = Device->DrawText(E->LineGap + TextW, Y, 0xFFFFFFFF, *E->Comp, 0, 0, 0,
                                                 L2FONT_Normal, 0, 0, 0, 0, 0, 0);
                        E->PaintCompositionJP(E->Comp, 0, TextW, Y, RowNum, Device, &PainterOut);
                    }
                    else if (GLanguageType == 3)
                    {
                        CompW = Device->DrawText(E->LineGap + TextW, Y, 0xFFFFFFFF, *E->Comp, 0, 0, 0,
                                                 L2FONT_Normal, 0, 0, 0, 0, 0, 0);
                        E->PaintCompositionTW(E->Comp, 0, TextW, Y, RowNum, Device, &PainterOut);
                    }
                    else if (GLanguageType == 4)
                    {
                        if (GConsole->GetIMEType() == 8)
                            CompW = Device->DrawText(E->LineGap + TextW, Y, 0xFFFFFFFF, *E->Comp, 0, 0,
                                                     0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
                        E->PaintCompositionCN(E->Comp, 0, TextW, Y, RowNum, Device, &PainterOut);
                    }
                    else if (GLanguageType == 0)
                    {
                        CompW = Device->DrawText(E->LineGap + TextW, Y, 0xFFFFFFFF, *E->Comp, 0, 0, 0,
                                                 L2FONT_Normal, 0, 0, 0, 0, 0, 0);
                        E->PaintCompositionKR(E->Comp, TextW, Y, Device);
                    }
                    else
                        CompW = Device->DrawText(E->LineGap + TextW, Y, 0xFFFFFFFF, *E->Comp, 0, 0, 0,
                                                 L2FONT_Normal, 0, 0, 0, 0, 0, 0);
                    TextW += CompW;
                    if (E->Lines.GetLine(Line))
                    {
                        if (End > 0)
                            Seg = E->Lines.GetLine(Line)->Text.Mid(E->CaretCol, End - E->CaretCol);
                        else
                            Seg = E->Lines.GetLine(Line)->Text.Mid(
                                E->CaretCol, E->Lines.GetLine(Line)->Text.Len() - E->CaretCol);
                        if (Seg.Len())
                            Device->DrawText(E->LineGap + TextW, Y, 0xFFFFFFFF, *Seg, 0, 0, 0,
                                             L2FONT_Normal, 0, 0, 0, 0, 0, 0);
                    }
                }
                if (E->IsFocused() && E->bCaretOn && GLanguageType == 2)
                    Device->DrawText(E->CaretX + E->LineGap, Y, 0xFFFFFFFF, TEXT("|"), 0, 0, 0,
                                     L2FONT_Normal, 0, 0, 0, 0, 0, 0);
            }
            else
            {
                // C2: the manager and its Chinese IME are looked at first
                if (GConsole->GetIMEType() == 10 || GConsole->GetIMEType() == 9 ||
                    GConsole->GetIMEType() == 11 || GConsole->GetIMEType() == 8 ||
                    GConsole->GetIMEType() == 12)
                {
                    if (E->IMEManager && E->IMEManager->ChineseIME)
                        ChineseCompositeBox(E->IMEManager)->HideWindow();
                }
                // the caret goes after the text drawn so far on this row (TextW), not at CaretX
                if (E->IsFocused() && E->bCaretOn)
                    Device->DrawText(E->LineGap + TextW - 3, Y, 0xFFFFFFFF, TEXT("|"), 0, 0, 0,
                                     L2FONT_Normal, 0, 0, 0, 0, 0, 0);
                End = E->GetRowEnd(Start, Line);
                if (E->Lines.GetLine(Line))
                {
                    if (Row + 1 < E->TotalRows && End)
                        Seg = E->Lines.GetLine(Line)->Text.Mid(E->CaretCol, End - E->CaretCol);
                    else
                        Seg = E->Lines.GetLine(Line)->Text.Mid(
                            E->CaretCol, E->Lines.GetLine(Line)->Text.Len() - E->CaretCol);
                    if (Seg.Len())
                        Device->DrawText(E->LineGap + TextW, Y, 0xFFFFFFFF, *Seg, 0, 0, 0,
                                         L2FONT_Normal, 0, 0, 0, 0, 0, 0);
                }
            }
        }
        else if (Row + 1 < E->TotalRows)
        {
            End = E->GetRowEnd(Start, Line);
            if (E->bCompRow)
            {
                // the composition starts in this row, after the line's text
                if (E->Lines.GetLine(Line))
                {
                    if (E->CaretCol > 0)
                        Seg = E->Lines.GetLine(Line)->Text.Mid(Start, E->CaretCol - Start);
                    else
                        Seg = E->Lines.GetLine(Line)->Text.Mid(
                            Start, E->Lines.GetLine(Line)->Text.Len() - Start);
                    if (Seg.Len())
                        TextW = Device->DrawText(E->LineGap, Y, 0xFFFFFFFF, *Seg, 0, 0, 0,
                                                 L2FONT_Normal, 0, 0, 0, 0, 0, 0);
                }
                if (GLanguageType == 2)
                {
                    Device->DrawText(E->LineGap + TextW, Y, 0xFFFFFFFF, *E->Comp.Mid(0, End), 0, 0, 0,
                                     L2FONT_Normal, 0, 0, 0, 0, 0, 0);
                    E->PaintCompositionJP(E->Comp.Mid(0, End), 0, TextW, Y, RowNum, Device,
                                          &PainterOut);
                }
                else if (GLanguageType == 3)
                {
                    Device->DrawText(E->LineGap + TextW, Y, 0xFFFFFFFF, *E->Comp.Mid(0, End), 0, 0, 0,
                                     L2FONT_Normal, 0, 0, 0, 0, 0, 0);
                    E->PaintCompositionTW(E->Comp.Mid(0, End), 0, TextW, Y, RowNum, Device,
                                          &PainterOut);
                }
                else
                    Device->DrawText(E->LineGap + TextW, Y, 0xFFFFFFFF, *E->Comp.Mid(0, End), 0, 0, 0,
                                     L2FONT_Normal, 0, 0, 0, 0, 0, 0);
            }
            else if (E->Lines.GetLine(Line))
            {
                if (End)
                    Seg = E->Lines.GetLine(Line)->Text.Mid(Start, End - Start);
                else
                    Seg = E->Lines.GetLine(Line)->Text.Mid(Start,
                                                           E->Lines.GetLine(Line)->Text.Len() - Start);
                if (Seg.Len())
                    TextW = Device->DrawText(E->LineGap, Y, 0xFFFFFFFF, *Seg, 0, 0, 0, L2FONT_Normal,
                                             0, 0, 0, 0, 0, 0);
            }
        }
        else
        {
            // the last row
            End = E->GetRowEnd(Start, Line);
            if (E->Lines.GetLine(Line))
            {
                Seg = E->Lines.GetLine(Line)->Text.Mid(Start, E->Lines.GetLine(Line)->Text.Len() - Start);
                if (Seg.Len())
                    Device->DrawText(E->LineGap, Y, 0xFFFFFFFF, *Seg, 0, 0, 0, L2FONT_Normal, 0, 0, 0,
                                     0, 0, 0);
            }
        }
        Y += E->CharH + E->LineGap;
        Start = End;
        if (End == 0 && (!E->bCompRow || E->CaretCol == 0))
            Line++;
    }
}

// 0x10022490 (vslot 55)
INT NCMultiEdit::OnPaint(NCGDevice* Device)
{
    Device->BeginPaint(ScreenX, ScreenY);
    Device->SetClip(ScreenX, ScreenY, Width, Height);
    PaintMultiEditText(this, Device);
    Device->ReleaseClip();
    Device->EndPaint();
    return 1;
}
NW_HOOK_METHOD(0x10022490, NCMultiEdit, OnPaint)

// 0x10020a30 (name ours) - with the Thai keyboard (IME type 13) a Thai character is taken only where the Thai
// spelling allows it after the text of the caret's line up to the caret (see NCEditBox::IsThaiInputValid, which
// does the same on its Left piece); 0 for the rest and for every other keyboard or without a line
UBOOL NCMultiEdit::IsThaiInputValid(UINT Ch)
{
    TCHAR Prev = 0;
    FString Text;
    if (GConsole->GetIMEType() == 13 && Lines.GetLine(CaretLine))
    {
        if (Lines.GetLine(CaretLine)->Text.Len() < CaretCol)
            return 0;
        Text = Lines.GetLine(CaretLine)->Text.Left(CaretCol);
        switch (IMEManager->GetThaiCharType(Ch))
        {
        case 1:
        case 2:
        case 5:
        case 6:
        case 7:
            return 1;
        case 3:
            if (Text.Len() < 1)
                return 0;
            {
                TCHAR Last = (*Text.Right(1))[0];
                if (Text.Len() > 1)
                    Prev = (*Text.Right(2))[0];
                if (IMEManager->GetThaiCharType(Last) == 1 || IMEManager->GetThaiCharType(Last) == 2)
                    return 1;
                if (IMEManager->GetThaiCharType(Last) != 8 || Text.Len() < 2)
                    return 0;
                return IMEManager->GetThaiCharType(Prev) == 2;
            }
        case 4:
            if (Text.Len() < 1)
                return 0;
            Prev = (*Text.Right(1))[0];
            if (IMEManager->GetThaiCharType(Prev) == 1)
                return 1;
            return IMEManager->GetThaiCharType(Prev) == 2;
        case 8:
        case 9:
            if (Text.Len() < 1)
                return 0;
            Prev = (*Text.Right(1))[0];
            if (IMEManager->GetThaiCharType(Prev) == 1 || IMEManager->GetThaiCharType(Prev) == 2)
                return 1;
            return IMEManager->GetThaiCharType(Prev) == 4;
        }
    }
    return 0;
}
NW_HOOK_METHOD(0x10020a30, NCMultiEdit, IsThaiInputValid)

// 0x10023940 (vslot 57) - printable codes, the upper characters with the IME type 9, and the Thai characters the
// spelling rules allow with the Thai keyboard (C2)
INT NCMultiEdit::OnChar(WPARAM wParam, LPARAM lParam)
{
    TCHAR Buf[20];
    if (wParam > 0x1F || (GConsole->GetIMEType() == 9 && wParam > 0x81) ||
        (GConsole->GetIMEType() == 13 && IsThaiInputValid(wParam)))
    {
        appSprintf(Buf, TEXT("%c"), wParam);  // not used
        InsertChar((TCHAR)wParam);
    }
    return 1;
}
NW_HOOK_METHOD(0x10023940, NCMultiEdit, OnChar)

// 0x100239c0 (name ours) - the Down arrow: the caret goes to the row below, to the same cluster / character offset
// in it (or to its end when it is shorter). Thai: offsets count clusters, and the column is the sum of their
// characters. C2 simplified the C1 case analysis through LineLength.
void NCMultiEdit::MoveCaretDown()
{
    TArray<ThaiCharacter> Before;
    TArray<ThaiCharacter> Next;
    INT BeforeCount = 0;
    INT NextCount = 0;
    if (Lines.GetLine(CaretLine) && CaretRow < TotalRows - 1)
    {
        INT RowStart = GetRowStart(CaretRow);
        INT RowEnd = GetRowEnd(RowStart, CaretLine);
        INT NextEnd = GetRowEnd(RowEnd, CaretLine);
        if (GLanguageType == 5)
        {
            NCGDevice::MakeThaiCharArray(*Lines.GetLine(CaretLine)->Text.Mid(RowStart, CaretCol - RowStart), Before, 0);
            BeforeCount = Before.Num();
        }
        else
            BeforeCount = CaretCol - RowStart;
        if (RowEnd == 0)
            CaretLine++;
        if (CaretRow > TotalRows - 3)
        {
            // the row below is the last one: it runs to the end of its line
            if (GLanguageType == 5)
            {
                NCGDevice::MakeThaiCharArray(
                    *Lines.GetLine(CaretLine)->Text.Mid(RowEnd, LineLength(Lines.GetLine(CaretLine)->Text) - RowEnd),
                    Next, 0);
                NextCount = Next.Num();
            }
            else
                NextCount = LineLength(Lines.GetLine(CaretLine)->Text) - RowEnd;
        }
        else if (GLanguageType == 5)
        {
            if (NextEnd != 0)
                NCGDevice::MakeThaiCharArray(*Lines.GetLine(CaretLine)->Text.Mid(RowEnd, NextEnd - RowEnd), Next, 0);
            else
                NCGDevice::MakeThaiCharArray(
                    *Lines.GetLine(CaretLine)->Text.Mid(RowEnd, LineLength(Lines.GetLine(CaretLine)->Text) - RowEnd),
                    Next, 0);
            NextCount = Next.Num();
        }
        else if (NextEnd == 0)
            NextCount = LineLength(Lines.GetLine(CaretLine)->Text) - RowEnd;
        else
            NextCount = NextEnd - RowEnd;
        CaretRow++;
        if (BeforeCount > NextCount)
        {
            if (NextEnd != 0)
                CaretCol = NextEnd;
            else
                CaretCol = LineLength(Lines.GetLine(CaretLine)->Text);
        }
        else if (GLanguageType == 5)
        {
            CaretCol = RowEnd;
            for (INT i = 0; i < BeforeCount; i++)
                CaretCol += Next(i).CharCount();
        }
        else
            CaretCol = BeforeCount + RowEnd;
        if (CaretX > 0)
            SetCaretX(CaretLine, CaretCol);
    }
}
NW_HOOK_METHOD(0x100239c0, NCMultiEdit, MoveCaretDown)

// the keys that move the caret wait for the IME to finish composing (IME type 18 commits the composition first)
#define MULTI_MOVE_OK(IMEType) ((IMEType) == 18 || IMEManager->IsNotComposing() == 1 || (IMEType) == 9 || \
                                (IMEType) == 11 || (IMEType) == 10 || (IMEType) == 12)

// 0x10024010 (vslot 58) - the caret keys; with the IME type 18 (and the Chinese ones for some keys) a pending
// composition is committed first. Delete is Right then Backspace. C2: the Thai client moves by clusters
// (Left, Right, Up, Down, Delete), Up/Down go through GetRowStart / GetRowEnd, and the scroll bar is updated
// after the caret keys (slot 68 UpdateScrollBar).
INT NCMultiEdit::OnKeyDown(WPARAM wParam, LPARAM lParam)
{
    INT IMEType = GConsole->GetIMEType();
    FString Ch;
    Ch.Empty();
    TArray<ThaiCharacter> ArrA;
    TArray<ThaiCharacter> ArrB;
    switch (wParam)
    {
    case VK_BACK:
        if (IMEType == 18 || IMEManager->IsNotComposing() == 1)
        {
            if (Comp.Len())
                Comp.Delete(0, Comp.Len());
            FString Left;
            FString Right;
            if (CaretCol < 1)
            {
                // joins the line to the previous one, whose '\n' goes
                if (CaretCol == 0 && CaretLine > 0 && Lines.GetLine(CaretLine) && Lines.GetLine(CaretLine - 1))
                {
                    CaretCol = Lines.GetLine(CaretLine - 1)->Text.Len() - 1;
                    Left = Lines.GetLine(CaretLine - 1)->Text.Left(Lines.GetLine(CaretLine - 1)->Text.Len() - 1);
                    Right = Lines.GetLine(CaretLine)->Text;
                    Lines.GetLine(CaretLine - 1)->Text = Left + Right;
                    Lines.RemoveAt(CaretLine);
                    CaretLine--;
                    NumLines--;
                }
            }
            else
            {
                if (Lines.GetLine(CaretLine))
                {
                    Left = Lines.GetLine(CaretLine)->Text.Left(CaretCol - 1);
                    Right = Lines.GetLine(CaretLine)->Text.Right(Lines.GetLine(CaretLine)->Text.Len() - CaretCol);
                    Lines.GetLine(CaretLine)->Text = Left + Right;
                }
                CaretCol--;
            }
            RecalcRows(0);
        }
        NumLines = Lines.Num();
        ScrollToCaret();
        UpdateScrollBar();
        break;

    case VK_RETURN:
        if (IMEType == 18)
        {
            if (Comp.Len())
                InsertString((TCHAR*)*Comp, 1);
            SetText(TEXT("\n"));
        }
        else if (IMEManager->IsNotComposing() == 1)
            SetText(TEXT("\n"));
        break;

    case VK_END:
        if (MULTI_MOVE_OK(IMEType))
        {
            if (Comp.Len() && IMEType == 18)
                InsertString((TCHAR*)*Comp, 1);
            if (Lines.GetLine(CaretLine))
            {
                if (CaretRow + 1 < TotalRows)
                {
                    if (GetRowStart(CaretRow + 1) == 0)
                        CaretCol = Lines.GetLine(CaretLine)->Text.Len() - 1;
                    else
                        CaretCol = GetRowStart(CaretRow + 1);
                }
                else
                    CaretCol = Lines.GetLine(CaretLine)->Text.Len();
            }
        }
        SetCaretX(CaretLine, CaretCol);
        break;

    case VK_HOME:
        if (MULTI_MOVE_OK(IMEType))
        {
            if (Comp.Len() && IMEType == 18)
                InsertString((TCHAR*)*Comp, 1);
            CaretCol = GetRowStart(CaretRow);
        }
        CaretX = 0;
        break;

    case VK_LEFT:
        if (MULTI_MOVE_OK(IMEType))
        {
            INT Step;
            if (Comp.Len() && IMEType == 18)
            {
                Step = Comp.Len();
                InsertString((TCHAR*)*Comp, 1);
            }
            else if (GLanguageType == 5)
            {
                NCGDevice::MakeThaiCharArray(*Lines.GetLine(CaretLine)->Text.Mid(0, CaretCol), ArrA, 0);
                if (ArrA.Num() > 0)
                    Step = ArrA(ArrA.Num() - 1).CharCount();
                else
                    Step = 0;
            }
            else
                Step = 1;
            if (CaretCol > 0)
            {
                Ch = Lines.GetLine(CaretLine)->Text.Mid(CaretCol - Step, Step);
                SIZE Size;
                NCGDevice::GetTextExtent(&Size, *Ch, 0, 0, 0, 0, 0, L2FONT_Normal);
                CaretCol -= Step;
                if (CaretX > 0)
                    CaretX -= Size.cx;
                else
                {
                    CaretRow--;
                    SetCaretX(CaretLine, CaretCol);
                }
            }
            else
            {
                if (CaretLine > 0)
                {
                    CaretLine--;
                    if (Lines.GetLine(CaretLine))
                        CaretCol = Lines.GetLine(CaretLine)->Text.Len() - 1;
                }
                if (CaretRow > 0)
                    CaretRow--;
                SetCaretX(CaretLine, CaretCol);
            }
        }
        ScrollToCaret();
        UpdateScrollBar();
        break;

    case VK_RIGHT:
        if (MULTI_MOVE_OK(IMEType))
        {
            if (Comp.Len() && IMEType == 18)
                InsertString((TCHAR*)*Comp, 1);
            if (Lines.GetLine(CaretLine))
            {
                INT Step;
                if (GLanguageType == 5)
                {
                    NCGDevice::MakeThaiCharArray(
                        *Lines.GetLine(CaretLine)->Text.Right(Lines.GetLine(CaretLine)->Text.Len() - CaretCol), ArrA, 0);
                    if (ArrA.Num() > 0)
                        Step = ArrA(0).CharCount();
                    else
                        Step = 0;
                }
                else
                    Step = 1;
                INT Col = CaretCol;
                UBOOL bAdvance = 0;
                if (CaretLine + 1 < NumLines)
                {
                    if (Col < Lines.GetLine(CaretLine)->Text.Len() - 1)
                    {
                        Ch = Lines.GetLine(CaretLine)->Text.Mid(Col, Step);
                        bAdvance = 1;
                    }
                    else
                    {
                        // past the line's '\n': the start of the next line
                        CaretLine++;
                        CaretRow++;
                        CaretX = 0;
                        CaretCol = 0;
                    }
                }
                else if (Col < Lines.GetLine(CaretLine)->Text.Len())
                {
                    Ch = Lines.GetLine(CaretLine)->Text.Mid(Col, Step);
                    bAdvance = 1;
                }
                if (bAdvance)
                {
                    SIZE Size;
                    NCGDevice::GetTextExtent(&Size, *Ch, 0, 0, 0, 0, 0, L2FONT_Normal);
                    CaretX += Size.cx;
                    if (CaretX > Width - 36)
                    {
                        CaretRow++;
                        CaretX = Size.cx;
                    }
                    CaretCol += Step;
                }
            }
        }
        ScrollToCaret();
        UpdateScrollBar();
        break;

    case VK_UP:
        if (IMEType == 18 || IMEManager->IsNotComposing() == 1)
        {
            if (Comp.Len())
                InsertString((TCHAR*)*Comp, 1);
            if (!Lines.GetLine(CaretLine))
                break;
            if (CaretRow > 0)
            {
                INT RowStart = GetRowStart(CaretRow);
                INT PrevStart = GetRowStart(CaretRow - 1);
                if (RowStart == 0)
                {
                    // the row above ends the previous line
                    if (GLanguageType == 5)
                    {
                        NCGDevice::MakeThaiCharArray(*Lines.GetLine(CaretLine)->Text.Mid(RowStart, CaretCol), ArrA, 0);
                        NCGDevice::MakeThaiCharArray(
                            *Lines.GetLine(CaretLine - 1)->Text.Mid(
                                PrevStart, Lines.GetLine(CaretLine - 1)->Text.Len() - PrevStart - 1),
                            ArrB, 0);
                        if (ArrB.Num() < ArrA.Num())
                        {
                            INT PrevNum = Lines.GetLine(CaretLine - 1)->Text.Num();
                            if (PrevNum == 0 || PrevNum - 1 <= 0)
                            {
                                CaretCol = 0;
                                CaretLine--;
                            }
                            else
                            {
                                PrevNum = Lines.GetLine(CaretLine - 1)->Text.Num();
                                CaretCol = PrevNum == 0 ? -1 : PrevNum - 2;
                                CaretLine--;
                            }
                        }
                        else
                        {
                            CaretCol = PrevStart;
                            for (INT i = 0; i < ArrA.Num(); i++)
                                CaretCol += ArrB(i).CharCount();
                            CaretLine--;
                        }
                    }
                    else
                    {
                        INT PrevNum = Lines.GetLine(CaretLine - 1)->Text.Num();
                        if (PrevNum == 0 || PrevNum - 1 == 0)
                        {
                            CaretCol = 0;
                            CaretLine--;
                        }
                        else
                        {
                            INT PrevLen = Lines.GetLine(CaretLine - 1)->Text.Len();
                            if (PrevLen - PrevStart - 1 < CaretCol)
                            {
                                PrevNum = Lines.GetLine(CaretLine - 1)->Text.Num();
                                CaretCol = PrevNum == 0 ? -1 : PrevNum - 2;
                                CaretLine--;
                            }
                            else
                            {
                                CaretCol += PrevStart;
                                CaretLine--;
                            }
                        }
                    }
                }
                else if (GLanguageType == 5)
                {
                    NCGDevice::MakeThaiCharArray(*Lines.GetLine(CaretLine)->Text.Mid(RowStart, CaretCol - RowStart), ArrA, 0);
                    NCGDevice::MakeThaiCharArray(*Lines.GetLine(CaretLine)->Text.Mid(PrevStart, RowStart - PrevStart),
                                                 ArrB, 0);
                    if (ArrA.Num() > ArrB.Num())
                        CaretCol = RowStart;
                    else
                    {
                        CaretCol = PrevStart;
                        for (INT i = 0; i < ArrA.Num(); i++)
                            CaretCol += ArrB(i).CharCount();
                    }
                }
                else if (CaretCol - RowStart < RowStart - PrevStart)
                    CaretCol = CaretCol - RowStart + PrevStart;
                else
                    CaretCol = RowStart;
                CaretRow--;
                if (CaretX > 0)
                    SetCaretX(CaretLine, CaretCol);
            }
        }
        ScrollToCaret();
        UpdateScrollBar();
        break;

    case VK_DOWN:
        if (IMEType == 18 || IMEManager->IsNotComposing() == 1)
        {
            if (Comp.Len())
                InsertString((TCHAR*)*Comp, 1);
            MoveCaretDown();
        }
        ScrollToCaret();
        UpdateScrollBar();
        break;

    case VK_DELETE:
        if ((IMEType == 18 || IMEManager->IsNotComposing() == 1) && Lines.GetLine(CaretLine))
        {
            INT Line = CaretLine;
            if (Line >= Lines.Num() - 1)
            {
                if (Lines.GetLine(Line)->Text.Len() <= CaretCol)
                    break;
            }
            OnKeyDown(VK_RIGHT, 0);
            if (GLanguageType == 5)
            {
                if (CaretCol == 0 && CaretLine > 0)
                {
                    OnKeyDown(VK_BACK, 0);
                    break;
                }
                NCGDevice::MakeThaiCharArray(*Lines.GetLine(CaretLine)->Text.Left(CaretCol), ArrA, 0);
                INT Step;
                if (ArrA.Num() != 0)
                    Step = ArrA(ArrA.Num() - 1).CharCount();
                else
                    Step = 0;
                Lines.GetLine(CaretLine)->Text.Delete(CaretCol - Step, Step);
                CaretCol -= Step;
                RecalcRows(0);
                NumLines = Lines.Num();
                ScrollToCaret();
                UpdateScrollBar();
            }
            else
                OnKeyDown(VK_BACK, 0);
        }
        break;

    case VK_PROCESSKEY:
        if (IMEType == 10)
        {
            if (lParam == 0x14B0001)
                ChineseCompositeBox(IMEManager)->CaretLeft();
            else if (lParam == 0x14D0001)
                ChineseCompositeBox(IMEManager)->CaretRight();
        }
        break;
    }
    return 1;
}
NW_HOOK_METHOD(0x10024010, NCMultiEdit, OnKeyDown)

NW_CHECK_VTABLE(NCMultiEdit, 0x1010e080, 77)

/*-----------------------------------------------------------------------------
    NCHtmlMultiEdit.
-----------------------------------------------------------------------------*/

// 0x1003db90 - NCMultiEdit's constructor, then the three scroll controls cleared
NCHtmlMultiEdit::NCHtmlMultiEdit()
{
    HtmlUpButton = NULL;
    HtmlDownButton = NULL;
    HtmlScrollBar = NULL;
}
NW_HOOK_CTOR(0x1003db90, NCHtmlMultiEdit)

// 0x1003dbf0 - the implicit destructor
NW_HOOK_DTOR(0x1003dbf0, NCHtmlMultiEdit)

// 0x1003a500 (vslot 25) - NCMultiEdit's (C2: no enabled test any more)
INT NCHtmlMultiEdit::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    return NCMultiEdit::WindowProc(Msg, wParam, lParam) != 0;
}
NW_HOOK_METHOD(0x1003a500, NCHtmlMultiEdit, WindowProc)

// 0x1003a520 (vslot 67)
const TCHAR* NCHtmlMultiEdit::GetValue()
{
    return GetText();
}
NW_HOOK_METHOD(0x1003a520, NCHtmlMultiEdit, GetValue)

// 0x1003a530 (vslot 66) - NCMultiEdit's, over Strings
INT NCHtmlMultiEdit::OnMouseWheel(WPARAM wParam, LPARAM lParam)
{
    SWORD Delta = (SWORD)((SWORD)HIWORD(wParam) / 120);
    if (Delta > 0)
    {
        if (TopLine > Delta)
            TopLine -= Delta;
        else
            TopLine = 0;
    }
    else
    {
        INT D = Delta;
        if (Strings.Num >= VisibleRows)
        {
            if (Strings.Num + (D - TopLine) > VisibleRows)
                TopLine -= D;
            else
                TopLine = Strings.Num - VisibleRows;
        }
        else if (Strings.Num + (D - TopLine) > VisibleRows)
            TopLine -= D;
        else
            TopLine = 0;
    }
    HtmlScrollBar->SetValue(TopLine);
    return 1;
}
NW_HOOK_METHOD(0x1003a530, NCHtmlMultiEdit, OnMouseWheel)

// 0x1003a620 (vslot 69)
INT NCHtmlMultiEdit::OnScrollUp()
{
    if (Strings.Num > VisibleRows)
    {
        if (--TopLine < 0)
            TopLine = 0;
        HtmlScrollBar->SetValue(TopLine);
    }
    return 1;
}
NW_HOOK_METHOD(0x1003a620, NCHtmlMultiEdit, OnScrollUp)

// 0x1003a670 (vslot 70)
INT NCHtmlMultiEdit::OnScrollDown()
{
    if (Strings.Num > VisibleRows)
    {
        TopLine++;
        if (TopLine > Strings.Num - VisibleRows)
            TopLine = Strings.Num - VisibleRows;
        HtmlScrollBar->SetValue(TopLine);
    }
    return 1;
}
NW_HOOK_METHOD(0x1003a670, NCHtmlMultiEdit, OnScrollDown)

// 0x1003a6e0 (vslot 68)
void NCHtmlMultiEdit::UpdateScrollBar()
{
    HtmlScrollBar->SetRange(Strings.Num, VisibleRows);
    HtmlScrollBar->SetValue(TopLine);
}
NW_HOOK_METHOD(0x1003a6e0, NCHtmlMultiEdit, UpdateScrollBar)

// stock body shared with NCHtmlEdit's (0x100310a0): the viewer scrolls the box into view
INT NCHtmlMultiEdit::OnLButtonDown(WPARAM wParam, LPARAM lParam)
{
    NotifyParent(((DWORD)(_WORD)LocalY << 16) | HTML_SCROLLTO);
    return 1;
}

// 0x1003e060 (vslot 36) - NCMultiEdit's, with the scroll controls inside the box (C2: the IME manager is made
// first, the background texture is "L2UI.EtcWnd.Edit_Back")
INT NCHtmlMultiEdit::OnCreate()
{
    IMEManager = new NCIMEManager(0);
    CreateChild(IMEManager, NWS_VISIBLE);
    BackTex = FindTexture(TEXT("L2UI.EtcWnd.Edit_Back"));
    WhiteTex = FindTexture(TEXT("NWindow.WhiteTexture"));
    CaretTimer = SetTimer(TIMER_MultiEditCaret, 500, 0);
    SIZE Size;
    NCGDevice::GetTextExtent(&Size, TEXT("\xAC00"), 0, 0, 0, 0, 0, L2FONT_Normal);
    CharW = Size.cx;
    CharH = Size.cy;
    CharH2 = Size.cy;
    INT W = LocalW;
    LineGap = 2;
    NumLines = 1;
    VisibleRows = (Height - 2) / (Size.cy + 2);
    INT H = LocalH;

    HtmlUpButton = new NCHtmlPushButton(TEXT("L2UI.NWindow.WindowScrollUp"), TEXT("L2UI.NWindow.WindowScrollUp"));
    HtmlUpButton->ID = NID_MULTIEDIT_UP;
    HtmlUpButton->SetWindowRect(W - 12, 0, 12, 12);
    CreateChild(HtmlUpButton, NWS_VISIBLE);

    HtmlDownButton = new NCHtmlPushButton(TEXT("L2UI.NWindow.WindowScrollDown"), TEXT("L2UI.NWindow.WindowScrollDown"));
    HtmlDownButton->ID = NID_MULTIEDIT_DOWN;
    HtmlDownButton->SetWindowRect(W - 12, H - 12, 12, 12);
    CreateChild(HtmlDownButton, NWS_VISIBLE);

    HtmlScrollBar = new NCHtmlSliderBar(TEXT("L2UI.NWindow.WindowScrollBar1"), TEXT("L2UI.NWindow.WindowScrollBar2"),
                                        TEXT("L2UI.NWindow.WindowScrollBar3"));
    HtmlScrollBar->ID = NID_MULTIEDIT_BAR;
    HtmlScrollBar->SetWindowRect(W - 12, 12, 12, H >= 24 ? H - 24 : 0);
    CreateChild(HtmlScrollBar, NWS_VISIBLE);
    HtmlScrollBar->DoNothing(0);
    HtmlUpButton->RepeatDelay = 200;
    HtmlDownButton->RepeatDelay = 200;
    HtmlScrollBar->SetRange(TopLine, VisibleRows);
    HtmlScrollBar->SetValue(TopLine);
    return 1;
}
NW_HOOK_METHOD(0x1003e060, NCHtmlMultiEdit, OnCreate)

// 0x1003b870 (vslot 37)
INT NCHtmlMultiEdit::OnDestroy()
{
    Clear();
    if (CaretTimer)
    {
        KillTimer(CaretTimer);
        CaretTimer = 0;
    }
    if (HtmlUpButton)
    {
        RemoveChildWindow(HtmlUpButton->Entry);
        if (HtmlUpButton)
            delete HtmlUpButton;
        HtmlUpButton = NULL;
    }
    if (HtmlDownButton)
    {
        RemoveChildWindow(HtmlDownButton->Entry);
        if (HtmlDownButton)
            delete HtmlDownButton;
        HtmlDownButton = NULL;
    }
    if (HtmlScrollBar)
    {
        RemoveChildWindow(HtmlScrollBar->Entry);
        if (HtmlScrollBar)
            delete HtmlScrollBar;
        HtmlScrollBar = NULL;
    }
    if (IMEManager)
    {
        RemoveChildWindow(IMEManager->Entry);
        if (IMEManager)
            delete IMEManager;
        IMEManager = NULL;
    }
    NCMultiEdit::OnDestroy();
    return 1;
}
NW_HOOK_METHOD(0x1003b870, NCHtmlMultiEdit, OnDestroy)

// 0x1003b9c0 (vslot 55) - NCMultiEdit's, clipped to its own width and the height of the first ancestor that is
// not a control (as NCHtmlEdit)
INT NCHtmlMultiEdit::OnPaint(NCGDevice* Device)
{
    Device->BeginPaint(ScreenX, ScreenY);
    NCWnd* Owner = FirstNonControlParent(this);
    if (Owner)
        Device->SetClip(ScreenX, Owner->ScreenY, Width, Owner->Height);
    PaintMultiEditText(this, Device);
    Device->ReleaseClip();
    Device->EndPaint();
    return 1;
}
NW_HOOK_METHOD(0x1003b9c0, NCHtmlMultiEdit, OnPaint)

NW_CHECK_VTABLE(NCHtmlMultiEdit, 0x10110dd0, 77)
