#pragma once
// NCScrollWnd - window whose content scrolls vertically (UClass "CScrollWnd", 0xCC): base of
// the HTML viewers, item windows and quest/notice info windows. ScrollY is the content offset
// (0 at the top, down to MinScrollY = Height - content height); Delta* is how much the last
// change moved it, for the subclasses that repaint incrementally. Scroll up / down buttons and
// a slider sit on the right edge; the keyboard (PageUp/PageDown/arrows) and the wheel scroll.

#include "nwindow/NCWnd.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCPushButton;
class NCSliderBar;

#if NW_HOOK_MODE
#define GNextScrollCtrlID NW_GLOBAL(INT, 0x101552e0)   // ids of the scroll controls, from 0x1200 (C2 address; C1 0x10109de8)
#else
extern INT GNextScrollCtrlID;
#endif

class NCScrollWnd : public NCWnd
{
    NW_DECLARE_CLASS(NCScrollWnd, NCWnd, 0, 0x101A46A0)

    INT ScrollX;                     // +0x98 never changed here
    INT ScrollY;                     // +0x9C <= 0
    INT DeltaX;                      // +0xA0 previous - current, after each scroll
    INT DeltaY;                      // +0xA4
    INT PageStep;                    // +0xA8 (100) PageUp / PageDown
    INT LineStep;                    // +0xAC (15) arrows, buttons, wheel (3 per notch)
    INT MinScrollY;                  // +0xB0 Height - content height (SetContentHeight)
    NCPushButton* ScrollUpButton;    // +0xB4
    NCPushButton* ScrollDownButton;  // +0xB8
    INT ScrollUpID;                  // +0xBC
    INT ScrollDownID;                // +0xC0
    NCSliderBar* ScrollBar;          // +0xC4
    INT ScrollBarID;                 // +0xC8
    INT Unknown0C8;                  // +0xCC (12; named after its C1 offset)

    NCScrollWnd();                                                   // 0x10025590
    // ~NCScrollWnd                                                  // 0x10019e10

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x100253d0
    virtual INT OnCreate();                                          // 36 0x10025610
    virtual INT OnDestroy();                                         // 37 0x10025870
    virtual void SetSize(INT W, INT H);                              // 42 0x10025470
    virtual INT OnPaint(NCGDevice* Device) { return 1; }             // 55 0x100ab570 (folded)
    virtual INT OnKeyDown(WPARAM wParam, LPARAM lParam);             // 58 0x10025970
    virtual INT OnMouseWheel(WPARAM wParam, LPARAM lParam);          // 66 0x100bf190
    // names ours
    virtual INT ScrollUp();                                          // 67 0x10025ab0
    virtual INT ScrollDown();                                        // 68 0x10025b40
    virtual INT OnScrollBar(INT Value);                              // 69 0x100bf130

    void ShowBars(INT bShow);                                        // 0x100254d0 (name ours)
    void SetContentHeight(INT ContentHeight);                        // 0x10025540
    void SetRepeatDelay(INT Delay);                                  // 0x100255e0
};
NW_ASSERT_OFFSET(NCScrollWnd, ScrollY, 0x9C);
NW_ASSERT_OFFSET(NCScrollWnd, MinScrollY, 0xB0);
NW_ASSERT_OFFSET(NCScrollWnd, ScrollBar, 0xC4);
NW_ASSERT_SIZE(NCScrollWnd, 0xD0);

#pragma pack(pop)