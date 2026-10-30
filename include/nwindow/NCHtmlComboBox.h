#pragma once
// NCHtmlComboBox - the <combobox> of the HTML pages (UClass "CHtmlComboBox", 0x200): an
// NCComboBox whose drop button is an NCHtmlButton (kept in its own field) and that paints
// clipped to the viewer while closed. Its items come from a ';'-separated list; its value is
// the chosen item.

#include "nwindow/NCComboBox.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCHtmlButton;

class NCHtmlComboBox : public NCComboBox
{
    NW_DECLARE_CLASS(NCHtmlComboBox, NCComboBox, 0, 0x101ABED0)

    INT Unknown1F8;                  // +0x1FC   // C2: offset derived, no evidence
    NCHtmlButton* HtmlDropButton;    // +0x200 (NCComboBox::DropButton stays NULL)   // C2: offset derived, no evidence

    NCHtmlComboBox();                                                // 0x100325a0
    // ~NCHtmlComboBox                                               // 0x100325f0

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x1003a290
    virtual INT OnCreate();                                          // 36 0x1003e410
    virtual INT OnDestroy();                                         // 37 0x1003da90
    virtual void ShowWindow();                                       // 43 0x1007b460 (shared)
    virtual void HideWindow();                                       // 44 0x1007b470 (shared)
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x1003a7c0
    virtual const TCHAR* GetValue();                                 // 67 0x1003a7b0

    INT SetList(TCHAR* List, TCHAR* Sel);                            // 0x1003a300 (name ours)
};
NW_ASSERT_OFFSET(NCHtmlComboBox, HtmlDropButton, 0x200);
NW_ASSERT_SIZE(NCHtmlComboBox, 0x204);

#pragma pack(pop)