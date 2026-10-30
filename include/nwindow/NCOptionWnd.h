#pragma once
// NCOptionWnd - the options window (UClass "COptionWnd", 0xB8; system menu > Options): three
// tab pages (video, audio, game) and OK / Cancel / Apply. OK and Apply make every page apply
// its settings and mark the first run done (Option.ini [FirstRun]); OK and Cancel close it.

#include "nwindow/NCWnd.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCButton;
class NCTabControl;
class NCVideoOptionWnd;
class NCAudioOptionWnd;
class NCGameOptionWnd;

// NWM_COMMAND ids of the window's controls
enum
{
    NID_OPTION_FRAME = 0xF001,
    NID_OPTION_OK = 0xF002,
    NID_OPTION_CANCEL = 0xF003,
    NID_OPTION_APPLY = 0xF004,
    NID_OPTION_TABS = 0xF006,
};

class NCOptionWnd : public NCWnd
{
    NW_DECLARE_CLASS(NCOptionWnd, NCWnd, 0, 0x101C7FE8)

    UTexture* BackTex;                     // +0x98 "L2UI.etcWnd.OptionBack"
    NCVideoOptionWnd* VideoWnd;            // +0x9C
    NCAudioOptionWnd* AudioWnd;            // +0xA0
    NCGameOptionWnd* GameWnd;              // +0xA4
    NCButton* OKButton;                    // +0xA8
    NCButton* CancelButton;                // +0xAC
    NCButton* ApplyButton;                 // +0xB0
    NCTabControl* TabCtrl;                 // +0xB4
    NCWnd* UnusedB4;                       // +0xB8 destroyed, never created   // C2: offset derived, no evidence

    NCOptionWnd();                                                   // 0x100a43d0
    // ~NCOptionWnd                                                  // 0x100a4420

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x100a0790
    virtual INT OnCreate();                                          // 36 0x100a4470
    virtual INT OnDestroy();                                         // 37 0x100a4ae0
    virtual void ShowWindow();                                       // 43 0x100a52a0
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x100a4a00
    // names ours
    virtual INT OnOK();                                              // 67 0x100a4cc0
    virtual INT OnCancel();                                          // 68 0x100a0830 (folded)
    virtual INT OnApply();                                           // 69 0x100a4d20
};
NW_ASSERT_OFFSET(NCOptionWnd, BackTex, 0x98);
NW_ASSERT_OFFSET(NCOptionWnd, OKButton, 0xA8);
NW_ASSERT_OFFSET(NCOptionWnd, TabCtrl, 0xB4);
NW_ASSERT_SIZE(NCOptionWnd, 0xBC);

#pragma pack(pop)