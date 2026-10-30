#pragma once
// NCHtmlSliderBar - a scroll bar thumb inside an HTML page (UClass "CHtmlSliderBar", 0x1CC):
// NCSliderBar clipped to its first ancestor that is not a control (the scroll bar of an HTML
// multi-line edit box).

#include "nwindow/NCSliderBar.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCHtmlSliderBar : public NCSliderBar
{
    NW_DECLARE_CLASS(NCHtmlSliderBar, NCSliderBar, 0, 0x101AD388)

    NCHtmlSliderBar();                                               // 0x1003d7b0
    NCHtmlSliderBar(const TCHAR* TopTexName, const TCHAR* MidTexName,
                    const TCHAR* BottomTexName);                     // 0x10033060
    // ~NCHtmlSliderBar                                              // 0x1003d800

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x1003a790 (shared)
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x1003d8d0
};
NW_ASSERT_SIZE(NCHtmlSliderBar, 0x1D0);

#pragma pack(pop)