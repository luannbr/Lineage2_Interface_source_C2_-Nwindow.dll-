#pragma once
// NCListBox - scrolling list of colored text lines (UClass "CListBox", 0x1E8) with scroll up /
// down buttons and a slider on the right. At most MaxLines are kept (the oldest goes first);
// a click selects a line and reports its index (NWM_COMMAND, lParam = index).

#include "nwindow/NControl.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NString;
class NCPushButton;
class NCSliderBar;

struct NListItem
{
    NString* Text;             // Color = line color
    INT Param;
};

// NWM_COMMAND ids of the list's own controls
enum
{
    NID_LIST_SCROLLUP = (INT)0xF0F01406,
    NID_LIST_SCROLLDOWN = (INT)0xF0F01407,
    NID_LIST_SCROLLBAR = (INT)0xF0F01408,
};

class NCListBox : public NControl
{
    NW_DECLARE_CLASS(NCListBox, NControl, 0, 0x101A2CB8)

    UTexture* BackTex;               // +0x1A4 set by the owner
    UTexture* HighlightTex;          // +0x1A8 "NWindow.MenuHighlight" (not drawn here)
    TArray<NListItem> Items;         // +0x1AC
    NCPushButton* ScrollUpButton;    // +0x1B8
    NCPushButton* ScrollDownButton;  // +0x1BC
    NCSliderBar* ScrollBar;          // +0x1C0
    INT VisibleLines;                // +0x1C4 (10)
    INT MaxLines;                    // +0x1C8 (100)
    INT TopIndex;                    // +0x1CC first line shown
    INT NumLines;                    // +0x1D0
    INT Selected;                    // +0x1D4 -1 = none
    INT Unknown1D4;                  // +0x1D8 (12)
    INT TextX;                       // +0x1DC
    INT TextY;                       // +0x1E0
    INT LineGap;                     // +0x1E4 lines are 12 + LineGap high
    UBOOL bAutoScroll;               // +0x1E8 keep the last line in view

    NCListBox();                                                     // 0x1001c0e0
    NCListBox(INT InMaxLines, INT InVisibleLines, INT Unused);       // 0x1001c220 (3rd never read)
    // ~NCListBox                                                    // 0x1001c1c0

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x1001b430
    virtual INT OnCreate();                                          // 36 0x1001bac0
    virtual INT OnDestroy();                                         // 37 0x1001bd00
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x1001b7c0
    virtual INT OnLButtonDown(WPARAM wParam, LPARAM lParam);         // 61 0x1001b5e0
    virtual INT OnMouseWheel(WPARAM wParam, LPARAM lParam);          // 66 0x1001b620
    // names ours
    virtual INT HitTest(INT X, INT Y);                               // 68 0x1001b510
    virtual void AddString(const TCHAR* Text, DWORD Color, INT Param); // 69 0x1001bf00
    virtual void Clear();                                            // 70 0x1001be60
    virtual INT OnLButtonDblClick(WPARAM wParam, LPARAM lParam) { return 1; } // 71 0x100d2e70 (folded)
    virtual INT ScrollUp();                                          // 72 0x1001b6d0
    virtual INT ScrollDown();                                        // 73 0x1001b710
    virtual INT SetTopIndex(INT Index);                              // 74 0x1001b760

    void UpdateScrollBar();                                          // 0x1001b780
    void SetRepeatDelay(INT Delay);                                  // 0x1001b9b0
    void InitScrollBar();                                            // 0x1001b9e0
    INT GetItemParam(INT Index);                                     // 0x1001ba20
};
NW_ASSERT_OFFSET(NCListBox, Items, 0x1AC);
NW_ASSERT_OFFSET(NCListBox, ScrollBar, 0x1C0);
NW_ASSERT_OFFSET(NCListBox, Selected, 0x1D4);
NW_ASSERT_OFFSET(NCListBox, bAutoScroll, 0x1E8);
NW_ASSERT_SIZE(NCListBox, 0x1EC);

#pragma pack(pop)