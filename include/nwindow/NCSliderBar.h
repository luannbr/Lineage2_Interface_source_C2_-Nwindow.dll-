#pragma once
// NCSliderBar - vertical scroll bar thumb (UClass "CSliderBar", 0x1CC), used by the lists,
// edit boxes and scroll windows. SetRange(Total, Page) sizes the thumb (Height * Page /
// Total, at least 15 px); dragging it sets Value (first visible item) and reports it to the
// parent (NWM_COMMAND, lParam = Value).

#include "nwindow/NControl.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCSliderBar : public NControl
{
    NW_DECLARE_CLASS(NCSliderBar, NControl, 0, 0x101A4BC8)

    INT ThumbSize;             // +0x1A4 px
    INT ThumbPos;              // +0x1A8 px from the top
    INT Total;                 // +0x1AC items
    INT TrackLen;              // +0x1B0 px the thumb can travel (Height - ThumbSize)
    INT Value;                 // +0x1B4 first visible item
    INT Page;                  // +0x1B8 visible items
    UBOOL bDragging;           // +0x1BC
    INT LastY;                 // +0x1C0 mouse Y of the drag, relative
    UTexture* ThumbTopTex;     // +0x1C4 5x5
    UTexture* ThumbMidTex;     // +0x1C8 stretched
    UTexture* ThumbBottomTex;  // +0x1CC 5x5

    NCSliderBar();                                                   // 0x10025e20
    NCSliderBar(const TCHAR* TopTexName, const TCHAR* MidTexName,
                const TCHAR* BottomTexName);                         // 0x10021ac0
    // ~NCSliderBar                                                  // 0x10025ec0

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x10090b60 (folded)
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x10025fd0
    virtual INT OnMouseMove(WPARAM wParam, LPARAM lParam);           // 60 0x10025d30
    virtual INT OnLButtonDown(WPARAM wParam, LPARAM lParam);         // 61 0x10025cc0
    virtual INT OnLButtonUp(WPARAM wParam, LPARAM lParam);           // 62 0x10025d10

    // names ours
    void SetRange(INT InTotal, INT InPage);                          // 0x10025bf0
    void SetValue(INT NewValue);                                     // 0x10025c50
    void Reset();                                                    // 0x10025e00
    void SetScrollRange(INT InTotal, INT InPage);                    // 0x10026110 (jmp SetRange)
    void SetScrollPos(INT NewValue);                                 // 0x10026120 (jmp SetValue)
    // an empty body the multi-line edit boxes call on their bar (name ours; the "ret 4" many
    // empty one-argument methods were folded into, too small to hook)
    void DoNothing(INT A) {}                                         // 0x10001ad0 (folded)
};
NW_ASSERT_OFFSET(NCSliderBar, TrackLen, 0x1B0);
NW_ASSERT_OFFSET(NCSliderBar, LastY, 0x1C0);
NW_ASSERT_SIZE(NCSliderBar, 0x1D0);

#pragma pack(pop)