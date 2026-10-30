#pragma once
// NCCandidateBox - the IME candidate list (UClass "CCandidateBox", 0xFC): pages of PageSize
// candidates, RowHeight px each, the selected one on a white bar. The frame textures come in
// three layouts picked by Mode. Up/Down (keys or the buttons) move the selection a page at a
// time when crossing a page edge; PageUp/PageDown show or hide the box. Filled by the IMEs.

#include "nwindow/NCWnd.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCPushButton;
class NCSliderBar;

// a string of the IMEs (names ours; 0x804 bytes): a candidate of NCCandidateBox, a clause of
// the Korean composition (Flag = target clause)
struct NImeText
{
    TCHAR Text[1024];          // +0x000
    INT Flag;                  // +0x800
};
NW_ASSERT_SIZE(NImeText, 0x804);

// NWM_COMMAND ids of the box's own controls
enum
{
    NID_CANDIDATE_SCROLLBAR = (INT)0xF0F01415,
    NID_CANDIDATE_UP = 0xF01416,
    NID_CANDIDATE_DOWN = 0xF01417,
};

class NCCandidateBox : public NCWnd
{
    NW_DECLARE_CLASS(NCCandidateBox, NCWnd, 0, 0x10199058)

    INT PageSize;                    // +0x98 candidates per page (9)
    UTexture* LeftEnable1;           // +0x9C frame, modes 0..2
    UTexture* LeftDisable1;          // +0xA0   (rows without a candidate)
    UTexture* LeftEnable2;           // +0xA4 frame, modes 3..5
    UTexture* LeftDisable2;          // +0xA8
    UTexture* Center1;               // +0xAC
    UTexture* Center2;               // +0xB0
    UTexture* Right1;                // +0xB4
    UTexture* Right2;                // +0xB8
    UTexture* WhiteTex;              // +0xBC selection bar
    INT RowHeight;                   // +0xC0 (15)
    TArray<NImeText> Items;          // +0xC4
    INT Selected;                    // +0xD0
    INT PageStart;                   // +0xD4 first candidate of the page shown
    INT Count;                       // +0xD8 candidates
    INT MarginX;                     // +0xDC hit test rectangle of the rows
    INT MarginY;                     // +0xE0
    INT UnknownE0;                   // +0xE4
    DWORD TextColor;                 // +0xE8 selected candidate (black)
    INT UnknownE8;                   // +0xEC
    NCPushButton* UpButton;          // +0xF0
    NCPushButton* DownButton;        // +0xF4
    NCSliderBar* ScrollBar;          // +0xF8
    INT Mode;                        // +0xFC frame layout: 0..2, 3..4, 5

    NCCandidateBox(INT InPageSize = 9, INT InRowHeight = 15);        // 0x10004410
    // ~NCCandidateBox                                               // 0x100044b0

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x10002c70
    virtual INT OnCreate();                                          // 36 0x100031f0
    virtual INT OnDestroy();                                         // 37 0x10004250
    virtual UBOOL PtInRect(INT X, INT Y, INT RectX, INT RectY, INT RectW, INT RectH); // 41 0x10002ef0
    virtual void SetSize(INT W, INT H);                              // 42 0x10002f30
    virtual void ShowWindow();                                       // 43 0x10003050
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x100034c0
    virtual INT OnKeyDown(WPARAM wParam, LPARAM lParam);             // 58 0x10002dd0
    virtual INT OnLButtonDown(WPARAM wParam, LPARAM lParam);         // 61 0x100039b0
    virtual INT OnLButtonUp(WPARAM wParam, LPARAM lParam);           // 62 0x10002ec0
    // names ours
    virtual INT OnUpButton();                                        // 67 0x10002e80
    virtual INT OnDownButton();                                      // 68 0x10002ea0
    virtual INT OnScrollBar(INT Pos) { return 1; }                   // 69 0x100ab570 (folded)

    void SelectPrev();                                               // 0x10002d50 (name ours)
    void SelectNext();                                               // 0x10002da0 (name ours)
    // the IMEs fill it with these (names ours)
    void SetScroll(INT Total, INT Value);                            // 0x10002fe0 scroll bar, 9 a page
    void SetMode(INT InMode);                                        // 0x10003010 and the height
    void AddItem(const TCHAR* Text, INT Flag);                       // 0x100041a0 Flag not stored (0)
    void Clear();                                                    // 0x10004220
};
NW_ASSERT_OFFSET(NCCandidateBox, Items, 0xC4);
NW_ASSERT_OFFSET(NCCandidateBox, TextColor, 0xE8);
NW_ASSERT_SIZE(NCCandidateBox, 0x100);

#pragma pack(pop)