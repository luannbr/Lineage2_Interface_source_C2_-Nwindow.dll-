#pragma once
// NCMacroInfoWnd - the macro's additional information window (C2 only; UClass "CMacroInfoWnd", 0xAC): a small
// window beside the macro editor with the short name (up to 4 characters: what the grid's slot shows) and the
// description (up to 32) of the macro being edited, an OK and a Cancel button. OK copies both into the macro's slot of
// NCMacroListWnd (they go to the server with the rest when the editor saves). The desktop keeps it at +0x100
// (name ours).
//
// Layout from the constructor 0x1008E540 and OnCreate 0x1008E5E0.

#include "nwindow/NCWnd.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCButton;
class NCEditBox;
class NCMultiEdit;

// NWM_COMMAND ids (names ours)
enum
{
    NID_MACROINFO_SHORTNAME = 0x1513,  // the short name's edit box
    NID_MACROINFO_DESC = 0x1514,       // the description's multi line edit
    NID_MACROINFO_OK = 0x1016,
    NID_MACROINFO_CANCEL = 0x1017,
};

class NCMacroInfoWnd : public NCWnd
{
    NW_DECLARE_CLASS(NCMacroInfoWnd, NCWnd, 0, 0x101BF4E8)

    UTexture* BackTex;               // +0x98 "l2ui.macrownd.macro_additional_info"
    NCEditBox* ShortNameEdit;        // +0x9C (at most 4 characters)
    NCMultiEdit* DescEdit;           // +0xA0
    NCButton* OKButton;              // +0xA4
    NCButton* CancelButton;          // +0xA8

    NCMacroInfoWnd();                                                // 0x1008E540
    // ~NCMacroInfoWnd                                               // 0x1008E590

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x1008B940
    virtual INT OnCreate();                                          // 36 0x1008E5E0
    virtual INT OnDestroy();                                         // 37 0x1008E940
    virtual void ShowWindow();                                       // 43 0x1008BA20
    virtual void HideWindow();                                       // 44 0x1008BAC0
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x1008EA90
    // names ours
    virtual INT OnOK();                                              // 67 0x1008BAF0
    virtual INT OnCancel();                                          // 68 0x100A0830 (shared: HideWindow())

    // names ours
    __declspec(noinline) void SetShortName(const TCHAR* Text);                            // 0x1008B9C0 the edit box emptied, then Text inserted
    __declspec(noinline) void SetDescription(const TCHAR* Text);                          // 0x1008B9F0 the same for the description
};
NW_ASSERT_OFFSET(NCMacroInfoWnd, BackTex, 0x98);
NW_ASSERT_OFFSET(NCMacroInfoWnd, ShortNameEdit, 0x9C);
NW_ASSERT_OFFSET(NCMacroInfoWnd, CancelButton, 0xA8);
NW_ASSERT_SIZE(NCMacroInfoWnd, 0xAC);

#pragma pack(pop)
