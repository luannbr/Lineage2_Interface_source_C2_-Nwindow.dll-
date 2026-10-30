#pragma once
// NCEditBox - single line text input (UClass "CEditBox", 0x238): the chat line, login fields,
// amounts in dialogs... The text is kept in pieces around the caret: Left (before it), Comp
// (the IME composition, drawn at the caret) and Right (after it); arrows move characters
// between Left and Right. The text may grow ExtraWidth px past the box (0: the box width)
// and scrolls from ViewStart. A hidden NCIMEManager child gives it an IME, whose private
// messages (NIM_*) arrive in 65..72.
//
// Type: 0 plain, 1 chat (" first and Up/Down are reported to the parent), 2 password (drawn
// as '*'), 3 digits only. The parent gets NWM_COMMAND: 0 Enter on an empty box, 1 Enter,
// 3 '"' typed first, 4 Up, 5 Down (chat).

#include "nwindow/NControl.h"
#include "nwindow/NCGDevice.h"   // ThaiCharacter
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCIMEManager;

class NCEditBox : public NControl
{
    NW_DECLARE_CLASS(NCEditBox, NControl, 0, 0x1019C8F8)

    NCIMEManager* IMEManager;        // +0x1A4
    UTexture* Edit1;                 // +0x1A8 focused frame: left, middle, right
    UTexture* Edit2;                 // +0x1AC
    UTexture* Edit3;                 // +0x1B0
    UTexture* EditDisable1;          // +0x1B4 frame without the focus
    UTexture* EditDisable2;          // +0x1B8
    UTexture* EditDisable3;          // +0x1BC
    UTexture* HighlightTex;          // +0x1C0 new in C2 (name ours): "L2UI.MacroWnd.Macro_Line_HighLight"
    INT Type;                        // +0x1C4 see above (C1: +0x1BC)
    INT Unknown1C8;                  // +0x1C8 new in C2 (name ours): the constructor's fifth argument, 1 when
                                     //   GLanguageType is 1; NIM_CURSORPOS keeps only printable codes when it is 1
    INT Unknown1CC;                  // +0x1CC new in C2 (1): OnPaint draws the caret only while it is set (nothing
                                     //   clears it in the stock code we know)
    UBOOL bCanInsert;                // +0x1D0 the text is narrower than MaxWidth - 12
    UBOOL bCaretOn;                  // +0x1D4 caret blink phase
    INT Unknown1D8;                  // +0x1D8 new in C2 (0): OnPaint draws HighlightTex over the box while it is set
                                     //   (the macro line editor of the new macro windows)
    INT CaretTimer;                  // +0x1DC SetTimer id (500 ms)
    UTexture* BackTex;               // +0x1E0 constructor argument (not drawn here)
    UTexture* WhiteTex;              // +0x1E4 composing character background
    UBOOL bIMEPosFixed;              // +0x1E8 constructor argument: IME window placement
    UBOOL bReading;                  // +0x1EC a reading string is shown (NIM_READING)
    INT Unknown1F0;                  // +0x1F0 (C1 Unknown1DC)
    INT ViewStart;                   // +0x1F4 first character of Left drawn
    INT ExtraWidth;                  // +0x1F8 px the text may grow past the box
    INT MaxWidth;                    // +0x1FC Width + ExtraWidth (OnPaint)
    INT ViewStart2;                  // +0x200 copy of ViewStart restored by End
    INT MaxLen;                      // +0x204 new in C2 (name ours): the most characters the text may hold, 0 = no limit
    FString Left;                    // +0x208
    FString Right;                   // +0x214
    TArray<ThaiCharacter> ThaiLeft;  // +0x220 new in C2 (name ours): the Thai clusters of Left (GLanguageType 5)
    TArray<ThaiCharacter> ThaiRight; // +0x22C   and of Right
    FString Comp;                    // +0x238
    FString Reading;                 // +0x244
    DWORD CaretX;                    // +0x250 caret position in the box (OnPaint)
    INT Unknown224;                  // +0x254
    INT IMECaretX;                   // +0x258 answer to NIM_GETCARET
    INT IMECaretY;                   // +0x25C
    INT CursorPos;                   // +0x260 NIM_CURSORPOS
    INT ReadingParam;                // +0x264 lParam of NIM_READING

    NCEditBox(INT InType = 0, const TCHAR* BackTexName = NULL, UBOOL bInIMEPosFixed = 1,
              INT InExtraWidth = 0, INT InUnknown1C8 = 0);           // 0x1000d5d0 (C2: five arguments)
    // ~NCEditBox                                                    // 0x1000d7a0

    // WindowProc: the stock body is shared with NCIMEManager (hooked there)
    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x10010fa0
    virtual void SetFocus();                                         // 29 0x1000a620
    virtual void KillFocus();                                        // 30 0x1000d440
    virtual INT vfn032() { return 1; }                               // 33 0x10005b80 (folded)
    virtual INT OnTimer(INT TimerID);                                // 35 0x1000a5e0
    virtual INT OnCreate();                                          // 36 0x1000b260
    virtual INT OnDestroy();                                         // 37 0x1000b470
    virtual void ShowWindow();                                       // 43 0x1007b460 (shared)
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x1000cad0
    virtual INT OnChar(WPARAM wParam, LPARAM lParam);                // 57 0x1000c740
    virtual INT OnKeyDown(WPARAM wParam, LPARAM lParam);             // 58 0x1000d860
    // names ours
    virtual const TCHAR* CommitText();                               // 68 0x1000a070
    virtual void ClearText();                                        // 69 0x1000a170
    virtual INT OnComposition(WPARAM wParam, LPARAM lParam);         // 70 0x1000a900
    virtual INT OnResult(WPARAM wParam, LPARAM lParam);              // 71 0x1000a640
    virtual INT OnAttribute(WPARAM wParam, LPARAM lParam);           // 72 0x1000acc0
    virtual INT OnReading(WPARAM wParam, LPARAM lParam);             // 73 0x1000ae40
    virtual INT On14(WPARAM wParam, LPARAM lParam) { return 1; }     // 74 0x100d2e70 (folded)
    virtual INT OnGetCaret(WPARAM wParam, LPARAM lParam);            // 75 0x1000afb0
    virtual INT OnEndComposition(WPARAM wParam, LPARAM lParam);      // 76 0x1000aff0
    virtual INT OnCursorPos(WPARAM wParam, LPARAM lParam);           // 77 0x1000b040 (C1: 72)

    // names ours
    void SetText(const TCHAR* InText);                               // 0x1000a060 Left only
    void SetMaxLength(INT InMaxLen);                                 // 0x10009510 (C2 only; name ours) the longest text, when positive
    void InsertText(const TCHAR* InText, UBOOL bAppend);             // 0x1000c250 (C1 0x10008c40)
    void DeleteChar(INT Index);                                      // 0x1000c470 (C1 0x10008d30)
    // new in C2 (names ours); NCCalcEditCtrl and the HTML box call ReformatNumber too
    void ReformatNumber();                                           // 0x1000a1b0 Type 3: thousands separators
    UBOOL IsThaiInputValid(UINT Ch);                                 // 0x1000a310 Thai spelling test, Thai keyboard
    // not inlined: the stock callers call it (and it destroys the by-value argument itself)
    __declspec(noinline) INT GetTextWidth(FString Text);             // 0x1000a100
    void SetType(INT InType);                                        // 0x1000a610
    INT GetCompCaretX();                                             // 0x1000b070 (Japanese)

    // composition drawing of the Korean client (names ours)
    void PaintComposingKR(INT X, INT Y, INT TextW, INT CompW, NCGDevice* Device);   // 0x10009530
    void PaintCompositionKR(INT X, INT Y, INT TextW, INT Offset, NCGDevice* Device); // 0x10009c50
    // the same for the other IMEs (names ours)
    void PaintComposingTW(INT X, INT Y, INT TextW, INT CompW, INT ClauseW, INT ReadingW,
                          NCGDevice* Device);                                        // 0x10008180
    void PaintCompositionJP(INT X, INT Y, INT TextW, INT Offset, NCGDevice* Device); // 0x10009a30
    void PaintCompositionTW(INT X, INT Y, INT TextW, INT* ClauseW, NCGDevice* Device); // 0x10009e00
    void PaintComposingJP(INT X, INT Y, INT TextW, INT CompW, NCGDevice* Device);   // 0x1000b530
    void PaintComposingCN(INT X, INT Y, INT TextW, INT CompW, NCGDevice* Device);   // 0x1000b7e0
    void PaintCompositionCN(INT X, INT Y, INT TextW, NCGDevice* Device);            // 0x1000be60
};
NW_ASSERT_OFFSET(NCEditBox, IMEManager, 0x1A4);
NW_ASSERT_OFFSET(NCEditBox, Type, 0x1C4);
NW_ASSERT_OFFSET(NCEditBox, CaretTimer, 0x1DC);
NW_ASSERT_OFFSET(NCEditBox, WhiteTex, 0x1E4);
NW_ASSERT_OFFSET(NCEditBox, ThaiLeft, 0x220);
NW_ASSERT_OFFSET(NCEditBox, Comp, 0x238);
NW_ASSERT_OFFSET(NCEditBox, ViewStart, 0x1F4);
NW_ASSERT_OFFSET(NCEditBox, Left, 0x208);
NW_ASSERT_OFFSET(NCEditBox, Reading, 0x244);
NW_ASSERT_OFFSET(NCEditBox, CaretX, 0x250);
NW_ASSERT_SIZE(NCEditBox, 0x268);

#pragma pack(pop)