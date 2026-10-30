#pragma once
// C2 NOTE: in the C2 build CDialogBox is only the 0x1A4-byte base of the two warning boxes
// (NCWarningDialogBox.h, NCKeyCryptWarningDialogBox.h): it has no state of its own, and the
// message box described below is CGaraDialogBox (0x23C, NCGaraDialogBox.h) there.
//
// NCGaraDialogBox - the desktop's message box (UClass "CDialogBox", 0x608 in C1): a message, OK /
// Cancel, and optionally an edit line (with a keypad when it takes digits) and a progress
// bar that cancels the box when it ends. NConsoleWnd::ShowDialog fills it from an
// NDialogInfo (SetInfo). What OK and Cancel do depends on DialogID: answer the owner window
// (amounts and yes/no, messages 0xF00000xx), or answer the server directly (invitations,
// wars, trade, revival...), checking names first where one is typed.

#include "nwindow/NControl.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCButton;
class NCProgressCtrl;
class NCEditBox;
struct NDialogInfo;

// NWM_COMMAND ids of the message box's controls (NCGaraDialogBox; the warning boxes use the OK id)
enum
{
    NID_DIALOG_OK = 0x1004,
    NID_DIALOG_CANCEL = 0x1005,
    NID_DIALOG_KEYPAD = 0xF5,          // NotifyID = the key: 0..8 = 1..9, 9 = 0, 10 = backspace, 11, 12 (C2)
    NID_DIALOG_EDIT = 0x1504,
    NID_DIALOG_PROGRESS = 0xF00DE,
};

// what the owner window gets (NControl::SendTo), names ours
enum
{
    NWM_DIALOG_RESULT = (INT)0xF0000003,   // wParam: 1 / 0 (OK / Cancel) or the amount typed
};

class NCDialogBox : public NControl
{
    NW_DECLARE_CLASS(NCDialogBox, NControl, 0, 0x1019B988)

    NCDialogBox();                                                   // 0x100090a0
    // ~NCDialogBox                                                  // 0x10006f10

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x10090b60
    virtual INT OnCreate();                                          // 36 0x10006d10
    virtual INT OnDestroy();                                         // 37 0x10006d20
    virtual void HideWindow();                                       // 44 0x10006de0

    // names ours: the two the warning boxes use to come up and go away modal
    void ShowModal();                                                // 0x10006dc0 ShowWindow, then SetModal(Entry)
    void HideModal();                                                // 0x10006da0 HideWindow (NCWnd's), then ClearModal(Entry)

    // The message box's functions that the other windows call through an NCDialogBox* (the
    // console's two boxes are NCGaraDialogBox objects in C2). Forwarders: the bodies are
    // NCGaraDialogBox's (NCGaraDialogBox.cpp hooks them at these addresses).
    void Reset();                                                    // 0x10006740
    void SetButtonTexts(INT OKTextID, INT CancelTextID);             // 0x10006c50 sysstring ids
    NWndEntry* GetOwner();                                           // 0x10006cf0
    void UpdateLayout();                                             // 0x10007860
    void SetInfo(NDialogInfo* Info);                                 // 0x10007a00
};
NW_ASSERT_SIZE(NCDialogBox, 0x1A4);

#pragma pack(pop)
