#pragma once
// NCPetitionListBox - the petition conversation list (UClass "CPetitionListBox", 0x1E8). Adds
// messages like the chat list (wrapped to the width, continuation lines indented 12 px), but
// the scroll controls stay on the right, the text starts at the left edge and the visible line
// count is fixed.

#include "nwindow/NCListBox.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCPetitionListBox : public NCListBox
{
    NW_DECLARE_CLASS(NCPetitionListBox, NCListBox, 0, 0x101D0098)

    NCPetitionListBox();                                             // 0x100b8210
    NCPetitionListBox(INT InMaxLines, INT InVisibleLines, INT Unused); // 0x100b82b0
    // ~NCPetitionListBox                                            // 0x100b8260

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x100dc140 (NCDebugWnd's)
    virtual INT OnCreate();                                          // 36 0x100b6c10
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x100b6c90
    virtual void AddString(const TCHAR* Text, DWORD Color, INT Param); // 69 0x100b7fb0
};
NW_ASSERT_SIZE(NCPetitionListBox, 0x1EC);

#pragma pack(pop)