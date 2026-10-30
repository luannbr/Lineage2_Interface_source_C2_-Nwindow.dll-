#pragma once
// NCMultiEdit - text box of several lines with word wrap (UClass "CMultiEdit", 0x264): the memo
// of the party matching options, and (as NCHtmlMultiEdit) the <multiedit> boxes of the HTML
// pages. The text is a list of logical lines (NLineList, a line ends with its '\n'); each line
// is wrapped into screen rows at Width - 36 px, TopLine is the first row shown. The caret has a
// logical position (CaretLine, CaretCol) and a screen one (CaretRow, CaretX). The scroll
// buttons and the scroll bar sit outside the box, at x = LocalW + 2. A hidden NCIMEManager child
// gives it an IME, whose private messages (NIM_*) arrive in 67..71; the composition is kept in
// Comp and drawn at the caret, by the painter of the client language (Korean here, the other
// ones not decompiled yet).

#include "nwindow/NControl.h"
#include "nwindow/NCGDevice.h"   // ThaiCharacter
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCIMEManager;
class NCPushButton;
class NCSliderBar;
class NCHtmlPushButton;
class NCHtmlSliderBar;
class NString;

// NWM_COMMAND ids of the scroll controls (names ours)
enum
{
    NID_MULTIEDIT_UP = (INT)0xF0F01409,
    NID_MULTIEDIT_DOWN = (INT)0xF0F01410,
    NID_MULTIEDIT_BAR = (INT)0xF0F01411,
};

// NLineNode - one logical line of NCMultiEdit (0x14 bytes, allocated with the global new; name
// ours). The text is at the start, so a node pointer is used as an FString*.
struct NLineNode
{
    FString Text;              // +0x00
    NLineNode* Prev;           // +0x0C
    NLineNode* Next;           // +0x10

    NLineNode();                                                     // 0x1001b390
    ~NLineNode();                                                    // 0x1001b3e0
};
NW_ASSERT_SIZE(NLineNode, 0x14);

// NLineList - the lines of NCMultiEdit (0x18 bytes, name ours): a doubly linked list with a
// cached cursor (Current, CurIndex) that GetLine walks from, whichever end is nearest.
struct NLineList
{
    INT Count;                 // +0x00
    INT Unknown4;              // +0x04 (0)
    NLineNode* Head;           // +0x08
    NLineNode* Current;        // +0x0C cursor of GetLine
    NLineNode* Tail;           // +0x10
    INT CurIndex;              // +0x14 index of Current

    NLineList();                                                     // 0x1001bc20 one empty line
    __declspec(noinline) ~NLineList();                               // 0x10023d80 (not inlined: the stock destructors call it)

    // the getter is a folded body the other classes share (0x10033ac0, hooked as NActionItem's)
    INT Num() { return Count; }
    NLineNode* GetLine(INT Index);                                   // 0x1001bd10
    INT TotalLength();                                               // 0x1001bca0 characters
    INT Insert(INT Index);                                           // 0x1001de60 -1 = at the end
    void RemoveAt(INT Index);                                        // 0x1001e000
    void Clear();                                                    // 0x1001e0c0

    // cursor steps (inlined in the stock code); NULL at an end
    NLineNode* StepBack()
    {
        if (Current && Current->Prev)
        {
            Current = Current->Prev;
            if (CurIndex > 0)
                CurIndex--;
            return Current;
        }
        return NULL;
    }
    NLineNode* StepForward()
    {
        if (Current && Current->Next)
        {
            Current = Current->Next;
            if (CurIndex < Count - 1)
                CurIndex++;
            return Current;
        }
        return NULL;
    }
};
NW_ASSERT_SIZE(NLineList, 0x18);

// NStringArray - growable array of NString (0xC bytes, name ours), grown 128 at a time. It
// starts with one empty string and nothing adds to it: only NCHtmlMultiEdit's scrolling reads
// its count.
struct NStringArray
{
    INT Num;                   // +0x00
    NString* Data;             // +0x04 new NString[Max]
    INT Max;                   // +0x08

    NStringArray();                                                  // 0x10022130
    ~NStringArray();                                                 // 0x10021f70 (jmp Empty)

    INT Empty();                                                     // 0x10021f00 returns 0
    void InsertAt(INT Index);                                        // 0x10021f80 -1 = at the end
};
NW_ASSERT_SIZE(NStringArray, 0xC);

class NCMultiEdit : public NControl
{
    NW_DECLARE_CLASS(NCMultiEdit, NControl, 0, 0x101A3C28)

    NCIMEManager* IMEManager;        // +0x1A4
    UTexture* BackTex;               // +0x1A8 "L2UI.EtcWnd.Edit_Back" (C1: "NWindow.chatback"), stretched over the box
    UTexture* WhiteTex;              // +0x1AC "NWindow.WhiteTexture", composing character
    NStringArray Strings;            // +0x1B0 see above
    NLineList Lines;                 // +0x1BC
    TArray<INT> IntList;             // +0x1D4 new in C2 (name ours): an array of 4-byte elements (the destructor is
                                     //   TIntArray's, 0x1001FB70); constructed and destroyed, nothing else reads it
    FString Comp;                    // +0x1E0 IME composition, drawn at the caret
    TCHAR* TextBuf;                  // +0x1EC GetText's result (new TCHAR[])
    INT CharH;                       // +0x1F0 height of the text
    INT LineGap;                     // +0x1F4 2: between the rows, and the text's left margin
    INT CaretTimer;                  // +0x1F8 SetTimer id (0xFA0D05, 500 ms)
    INT CaretLine;                   // +0x1FC logical line of the caret
    INT CaretCol;                    // +0x200 character of the caret in its line
    UBOOL bCaretOn;                  // +0x204 caret blink phase
    INT VisibleRows;                 // +0x208 (Height - 2) / (CharH + 2)
    INT TopLine;                     // +0x20C first screen row shown (scroll position)
    INT NumLines;                    // +0x210 copy of Lines.Count
    INT CaretRow;                    // +0x214 screen row of the caret
    INT CaretX;                      // +0x218 px of the caret in its row
    UBOOL bCompRow;                  // +0x21C GetRowStart/GetRowEnd: the row breaks inside the
                                     //   composition
    // C2: not placed (no evidence for where it went): INT Unknown210;                  // +0x210 RecalcRows's argument
    _WORD Unknown214;                // +0x220 (0)
    _WORD Unknown216;                // +0x222 (0)
    INT CharW;                       // +0x224 width of a Hangul character
    INT CharH2;                      // +0x228 CharH again
    INT Unknown220;                  // +0x22C (0)
    NCPushButton* UpButton;          // +0x230 0xF0F01409
    NCPushButton* DownButton;        // +0x234 0xF0F01410
    NCSliderBar* ScrollBar;          // +0x238 0xF0F01411
    INT IMECaretX;                   // +0x23C answer to NIM_GETCARET
    INT IMECaretY;                   // +0x240
    UBOOL bReading;                  // +0x244 a reading string is shown (NIM_READING)
    FString Reading;                 // +0x248
    INT ReadingParam;                // +0x254 lParam of NIM_READING
    INT Unknown24C;                  // +0x258 (1)
    INT Unknown250;                  // +0x25C (1)
    INT Unknown254;                  // +0x260 (0)
    INT Unknown258;                  // +0x264 (0)
    INT Unknown25C;                  // +0x268 (16)
    INT TotalRows;                   // +0x26C screen rows of the whole text

    NCMultiEdit();                                                   // 0x10023ea0
    // ~NCMultiEdit                                                  // 0x10023df0

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x1001dfe0
    virtual void SetFocus();                                         // 29 0x1000a620 (shared)
    virtual void KillFocus();                                        // 30 0x1001e170
    virtual INT vfn032() { return 1; }                               // 33 0x10005b80 (folded)
    virtual INT OnTimer(INT TimerID);                                // 35 0x1001e190
    virtual INT OnCreate();                                          // 36 0x100206c0
    virtual INT OnDestroy();                                         // 37 0x10022280
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x10022490
    virtual INT OnChar(WPARAM wParam, LPARAM lParam);                // 57 0x10023940
    virtual INT OnKeyDown(WPARAM wParam, LPARAM lParam);             // 58 0x10024010
    virtual INT OnMouseWheel(WPARAM wParam, LPARAM lParam);          // 66 0x1001f700
    // names ours
    virtual void UpdateScrollBar();                                  // 68 0x1001f8e0
    virtual INT OnScrollUp();                                        // 69 0x1001f7d0
    virtual INT OnScrollDown();                                      // 70 0x1001f810
    virtual INT OnScrollBar(INT Value);                              // 71 0x1001efb0
    virtual INT OnComposition(WPARAM wParam, LPARAM lParam);         // 72 0x10021940
    virtual INT OnResult(WPARAM wParam, LPARAM lParam);              // 73 0x100218f0
    virtual INT OnGetCaret(WPARAM wParam, LPARAM lParam);            // 74 0x1001ef10
    virtual INT OnReading(WPARAM wParam, LPARAM lParam);             // 75 0x1001ef40
    virtual INT OnAttribute(WPARAM wParam, LPARAM lParam);           // 76 0x1001ef90

    // names ours; not inlined, the stock callers call them
    __declspec(noinline) void ScrollToCaret();                       // 0x1001e130
    void SetRepeatDelay(INT Delay);                                  // 0x1001f870
    void InitScrollBar();                                            // 0x1001f8a0
    __declspec(noinline) INT CountRows(INT Line, UBOOL bComp);       // 0x1001fc00 (C2: bComp is a second argument)
    __declspec(noinline) void UpdateRowCount(INT Line, INT OldRows, UBOOL bComp); // 0x1001fe20 (C1 0x1001c120)
    __declspec(noinline) void RecalcRows(INT Param);                 // 0x1001fe50 (C1 0x1001c3d0)
    // new in C2 (names ours)
    __declspec(noinline) INT LineLength(FString Line);               // 0x1001ee80 Line's length without its '\n' / '\r'
    __declspec(noinline) UBOOL IsThaiInputValid(UINT Ch);            // 0x10020a30 OnChar: Thai spelling test at the caret
    __declspec(noinline) void MoveCaretDown();                       // 0x100239c0 OnKeyDown's Down arrow
    __declspec(noinline) void InsertString(TCHAR* Str, UBOOL bResult); // 0x10020390
    __declspec(noinline) void SetCaretX(INT Line, INT Col);          // 0x10020d60
    __declspec(noinline) INT GetRowStart(INT Row);                   // 0x10020ea0
    __declspec(noinline) INT GetRowEnd(INT Start, INT Line);         // 0x10021220
    __declspec(noinline) INT GetRowLine(INT Row);                    // 0x100215e0
    TCHAR* GetText();                                                // 0x10021a80
    __declspec(noinline) void InsertChar(TCHAR Ch);                  // 0x10021e10
    void SetText(const TCHAR* Text);                                 // 0x10022230
    void Clear();                                                    // 0x10022400

    // composition painters of the client languages (names ours): Korean, Japanese, Taiwanese
    // and Chinese
    void PaintCompositionKR(FString Text, INT X, INT Y, NCGDevice* Device);           // 0x1001ed60
    void PaintCompositionJP(FString Text, INT CompStart, INT X, INT Y, INT RowNum,
                            NCGDevice* Device, INT* Out);                               // 0x1001e1b0
    void PaintCompositionTW(FString Text, INT CompStart, INT X, INT Y, INT RowNum,
                            NCGDevice* Device, INT* Out);                               // 0x1001e670
    void PaintCompositionCN(FString Text, INT CompStart, INT X, INT Y, INT RowNum,
                            NCGDevice* Device, INT* Out);                               // 0x1001f080
};
NW_ASSERT_OFFSET(NCMultiEdit, Strings, 0x1B0);
NW_ASSERT_OFFSET(NCMultiEdit, Lines, 0x1BC);
NW_ASSERT_OFFSET(NCMultiEdit, Comp, 0x1E0);
NW_ASSERT_OFFSET(NCMultiEdit, CaretLine, 0x1FC);
NW_ASSERT_OFFSET(NCMultiEdit, CharW, 0x224);
NW_ASSERT_OFFSET(NCMultiEdit, Reading, 0x248);
NW_ASSERT_OFFSET(NCMultiEdit, TotalRows, 0x26C);
NW_ASSERT_SIZE(NCMultiEdit, 0x270);

// NCHtmlMultiEdit - the <multiedit> box of the HTML pages (UClass "CHtmlMultiEdit", 0x270): its
// own scroll controls (NCHtmlPushButton, NCHtmlSliderBar) inside the box's right edge, clipped
// like the page's other controls; the value is the text. Its scrolling counts Strings instead of
// the lines (always 1): the scroll buttons do nothing and scrolling down goes back to the top.
class NCHtmlMultiEdit : public NCMultiEdit
{
    NW_DECLARE_CLASS(NCHtmlMultiEdit, NCMultiEdit, 0, 0x101AB9B0)

    NCHtmlPushButton* HtmlUpButton;  // +0x270 NID_MULTIEDIT_UP
    NCHtmlPushButton* HtmlDownButton; // +0x274 NID_MULTIEDIT_DOWN
    NCHtmlSliderBar* HtmlScrollBar;  // +0x278 NID_MULTIEDIT_BAR

    NCHtmlMultiEdit();                                               // 0x1003db90
    // ~NCHtmlMultiEdit                                              // 0x10031050 (C1 only: no C2 match)

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x1003a500
    virtual INT OnCreate();                                          // 36 0x1003e060
    virtual INT OnDestroy();                                         // 37 0x1003b870
    virtual INT OnPaint(NCGDevice* Device);                          // C2: slot gone (was 50)
    virtual INT OnLButtonDown(WPARAM wParam, LPARAM lParam);         // 61 0x1003d790 (shared)
    virtual INT OnMouseWheel(WPARAM wParam, LPARAM lParam);          // 66 0x1003a530
    virtual const TCHAR* GetValue();                                 // 67 0x1003a520
    virtual void UpdateScrollBar();                                  // 68 0x1003a6e0
    virtual INT OnScrollUp();                                        // 69 0x1003a620
    virtual INT OnScrollDown();                                      // 70 0x1003a670
};
NW_ASSERT_OFFSET(NCHtmlMultiEdit, HtmlUpButton, 0x270);
NW_ASSERT_SIZE(NCHtmlMultiEdit, 0x27C);

#pragma pack(pop)