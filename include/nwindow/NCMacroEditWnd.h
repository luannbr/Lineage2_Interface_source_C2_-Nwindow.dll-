#pragma once
// NCMacroEditWnd - the macro editor (C2 only; UClass "CMacroEditWnd", 0xCC): the macro's name, its icon (seven to choose
// from, with two arrows), the lines (NCMacroEditList) and the buttons Save, Cancel, Help and the one that opens the
// additional information window (NCMacroInfoWnd). Save checks the lines (NCMacroEditList::ValidateLines), packs the
// macro (the slot's id, the name, the slot's short name and description, the icon, the lines) and sends it
// (NConsoleWnd::RequestMakeMacro). The desktop keeps it at +0xFC (name ours).
//
// Layout from the constructor 0x1008FD20, OnCreate 0x1008FDF0 and OnDestroy 0x1008E340.

#include "nwindow/NCWnd.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCButton;
class NCEditBox;
class NCMacroEditList;
class L2ParamStack;

// NWM_COMMAND ids (names ours)
enum
{
    NID_MACROEDIT_NAME = 0x1512,       // the name's edit box
    NID_MACROEDIT_SAVE = 0x1011,       // text 0x292
    NID_MACROEDIT_CANCEL = 0x1018,     // text 0x8D
    NID_MACROEDIT_HELP = 0x100C,       // text 0x91: the macro help page
    NID_MACROEDIT_PREVICON = 0x1013,
    NID_MACROEDIT_NEXTICON = 0x1014,
    NID_MACROEDIT_INFO = 0x1015,       // text 0x291: the additional information window
};

class NCMacroEditWnd : public NCWnd
{
    NW_DECLARE_CLASS(NCMacroEditWnd, NCWnd, 0, 0x101BEA98)

    UTexture* BackTex;               // +0x98 "l2ui.macrownd.macro_edit_window"
    NCMacroEditList* EditList;       // +0x9C the lines
    TArray<UTexture*> IconTexs;      // +0xA0 the seven icons "L2UI.MacroWnd.Macro_Icon1".."7"
    NCButton* PrevIconButton;        // +0xAC
    NCButton* NextIconButton;        // +0xB0
    NCButton* InfoButton;            // +0xB4
    NCEditBox* NameEdit;             // +0xB8 (12 characters at most)
    INT IconIndex;                   // +0xBC the icon chosen (0..6)
    NCButton* SaveButton;            // +0xC0
    NCButton* HelpButton;            // +0xC4
    NCButton* CancelButton;          // +0xC8

    NCMacroEditWnd();                                                // 0x1008FD20
    // ~NCMacroEditWnd                                               // 0x1008FD90

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x1008B750
    virtual INT OnCreate();                                          // 36 0x1008FDF0
    virtual INT OnDestroy();                                         // 37 0x1008E340
    virtual void ShowWindow();                                       // 43 0x1008E280
    virtual void HideWindow();                                       // 44 0x1008B920
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x1008DBD0
    // names ours
    virtual INT OnSave();                                            // 67 0x1008F4F0
    virtual INT OnCancel();                                          // 68 0x100A0830 (shared: HideWindow())
    virtual INT OnHelp();                                            // 69 0x1008B900 (the macro help page; shared with NCMacroListWnd's)
    virtual INT OnPrevIcon();                                        // 70 0x1008B840
    virtual INT OnNextIcon();                                        // 71 0x1008B860
    virtual INT OnInfo();                                            // 72 0x1008B890

    // names ours
    __declspec(noinline) void ClearMarks();                                               // 0x1008DE20 (every line box's mark cleared)
    __declspec(noinline) void ClearMark(INT Line);                                        // 0x1008DED0
    __declspec(noinline) void PushLines(L2ParamStack* Params);                            // 0x1008DF00 the lines of the boxes for the save request
};
NW_ASSERT_OFFSET(NCMacroEditWnd, BackTex, 0x98);
NW_ASSERT_OFFSET(NCMacroEditWnd, EditList, 0x9C);
NW_ASSERT_OFFSET(NCMacroEditWnd, IconTexs, 0xA0);
NW_ASSERT_OFFSET(NCMacroEditWnd, NameEdit, 0xB8);
NW_ASSERT_OFFSET(NCMacroEditWnd, IconIndex, 0xBC);
NW_ASSERT_OFFSET(NCMacroEditWnd, CancelButton, 0xC8);
NW_ASSERT_SIZE(NCMacroEditWnd, 0xCC);

#pragma pack(pop)
