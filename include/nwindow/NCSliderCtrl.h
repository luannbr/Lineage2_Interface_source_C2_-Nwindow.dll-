#pragma once
// NCSliderCtrl - a slider (UClass "CSliderCtrl", 0x1BC; C2 only): a black bar with an
// NCThumbButton that follows the mouse while it is dragged; Pos runs from 0 to Max over the
// control's width. Whoever creates it sets Max, Pos and the thumb's size before OnCreate;
// nothing in this class reports the position to its parent.

#include "nwindow/NControl.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCThumbButton;

class NCSliderCtrl : public NControl
{
    NW_DECLARE_CLASS(NCSliderCtrl, NControl, 0, 0x101A50F8)

    UTexture* BackTex;               // +0x1A4 "NWindow.BlackTexture"
    NCThumbButton* Thumb;            // +0x1A8 "NWindow.MenuHighlight" / "NWindow.WhiteTexture"
    INT Max;                         // +0x1AC (name ours) the largest Pos
    INT Pos;                         // +0x1B0 (name ours) the thumb's place, 0..Max
    INT ThumbW;                      // +0x1B4 (name ours)
    INT ThumbH;                      // +0x1B8 (name ours)

    NCSliderCtrl();                                                  // 0x10026200
    // ~NCSliderCtrl                                                 // 0x10026270

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x10026140
    virtual INT OnCreate();                                          // 36 0x100262c0
    virtual INT OnDestroy();                                         // 37 0x100263c0
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x10026460
    virtual INT OnThumbMove(INT X);                                  // 68 0x100261a0 (name ours)
};
NW_ASSERT_OFFSET(NCSliderCtrl, BackTex, 0x1A4);
NW_ASSERT_OFFSET(NCSliderCtrl, Thumb, 0x1A8);
NW_ASSERT_OFFSET(NCSliderCtrl, Max, 0x1AC);
NW_ASSERT_OFFSET(NCSliderCtrl, ThumbH, 0x1B8);
NW_ASSERT_SIZE(NCSliderCtrl, 0x1BC);

#pragma pack(pop)
