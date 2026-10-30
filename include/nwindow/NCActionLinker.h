#pragma once
// NCActionLinker - a link of the HTML viewer that runs an action (UClass "CActionLinker",
// 0x5A4): a click hands its code to the viewer (NWM_COMMAND, lParam = Action), or first asks
// Message in a yes / no box (dialog 0x3C in C2, 0x38 in C1), whose OK comes back as NWM_ACTIONLINK_CONFIRM.

#include "nwindow/NControl.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

enum { NWM_ACTIONLINK_CONFIRM = (INT)0xF0000010 };   // from the dialog box (dialog 0x3C in C2, 0x38 in C1)

class NCActionLinker : public NControl
{
    NW_DECLARE_CLASS(NCActionLinker, NControl, 0, 0x101AC3F8)

    INT Action;                        // +0x1A4 code handed to the parent
    TCHAR Message[512];                // +0x1A8 asked first when not empty (set by the viewer)   // C2: offset derived, no evidence

    NCActionLinker(INT InAction = 0);                                    // 0x1003ac60
    // ~NCActionLinker                                               // 0x1003acd0

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x1003a3c0
    virtual INT OnLButtonDown(WPARAM wParam, LPARAM lParam);         // 61 0x1003e770
    virtual INT OnConfirm(WPARAM wParam, LPARAM lParam);             // 68 0x1003a420 (name ours)
};
NW_ASSERT_OFFSET(NCActionLinker, Action, 0x1A4);
NW_ASSERT_OFFSET(NCActionLinker, Message, 0x1A8);
NW_ASSERT_SIZE(NCActionLinker, 0x5A8);

#pragma pack(pop)