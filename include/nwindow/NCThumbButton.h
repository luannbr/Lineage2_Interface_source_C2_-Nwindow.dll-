#pragma once
// NCThumbButton - the handle of an NCSliderCtrl (UClass "CThumbButton", 0x264, a plain button
// in size; C2 only): pressing it only marks it pressed (no sound, no hit test), and moving
// the mouse with the left button held reports the pointer's x to the parent (NWM_COMMAND with
// the button's ID).

#include "nwindow/NCButton.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCThumbButton : public NCButton
{
    NW_DECLARE_CLASS(NCThumbButton, NCButton, 0, 0x10198610)

    NCThumbButton();                                                 // 0x10002900
    NCThumbButton(const TCHAR* NormalTexName, const TCHAR* PushedTexName, UBOOL bInAcceptKey); // 0x100029a0
    // ~NCThumbButton                                                // 0x10002950

    virtual INT OnMouseMove(WPARAM wParam, LPARAM lParam);           // 60 0x10001410
    virtual INT OnLButtonDown(WPARAM wParam, LPARAM lParam);         // 61 0x100013f0
    virtual INT OnLButtonUp(WPARAM wParam, LPARAM lParam);           // 62 0x10001430
};
NW_ASSERT_SIZE(NCThumbButton, 0x264);

#pragma pack(pop)
