#pragma once
// NCGaraDialogBox - the desktop's message box (UClass "CGaraDialogBox", 0x23C; C2 only: C1's
// CDialogBox, which kept its name in the class table of the C2 build for the thin 0x1A4 base of the
// warning boxes, NCDialogBox.h): a message, OK / Cancel, and optionally an edit line (with a
// keypad when it takes digits) and a progress bar that cancels the box when it ends.
// NConsoleWnd::ShowDialog fills it from an NDialogInfo (SetInfo). What OK and Cancel do depends
// on DialogID: answer the owner window (amounts and yes / no, messages 0xF00000xx), or answer
// the server directly (invitations, wars, trade, revival...), checking names first where one is
// typed. The console keeps two of them (MainDialogBox, SecondDialogBox) and the other windows
// reach them through an NCDialogBox* (their functions that C1 had on NCDialogBox are stubs there).

#include "nwindow/NControl.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCButton;
class NCEditBox;
class NCProgressCtrl;
struct NDialogInfo;

// the keypad's textures: pairs of names (up, down), 13 of them, in the stock image (C2: 0x1014C110)
#if NW_HOOK_MODE
#define GKeypadTextures (reinterpret_cast<const TCHAR* const (*)[2]>(::nw::Addr(0x1014C110)))
#else
extern const TCHAR* const GKeypadTextures[][2];
#endif

class NCGaraDialogBox : public NControl
{
    NW_DECLARE_CLASS(NCGaraDialogBox, NControl, 0, 0x1019C3D8)

    UTexture* BackTex;               // +0x1A4 "L2UI.EtcWnd.Popup"
    UTexture* InputBackTex;          // +0x1A8 "L2UI.EtcWnd.Popup" (with an edit line)
    UTexture* KeypadTex;             // +0x1AC "L2ui.numpad.numpad_back"
    NCButton* OKButton;              // +0x1B0
    NCButton* CancelButton;          // +0x1B4
    NCButton* Keypad[13];            // +0x1B8 0..8 = 1..9, 9 = 0, 10 = backspace, 11 = the number of Unknown238, 12 = 0
    NCProgressCtrl* Progress;        // +0x1EC
    NCEditBox* EditBox;              // +0x1F0
    INT DialogID;                    // +0x1F4 (-1 = none)
    INT Type;                        // +0x1F8 edit line type (3: digits, the keypad shows)
    NWndEntry* Owner;                // +0x1FC window that gets the answer
    UBOOL bInput;                    // +0x200 the edit line shows
    UBOOL bLargeAmount;              // +0x204 amounts up to 100,000,000 (else 1,000,000)
    UBOOL bProgress;                 // +0x208 the progress bar runs
    FString Message;                 // +0x20C
    FString ParamText;               // +0x218 a name some answers carry
    INT Param1;                      // +0x224 numbers some answers carry
    INT Param2;                      // +0x228
    INT Unknown22C[3];               // +0x22C a place (three floats): SetInfo copies the NDialogInfo's +0x78..+0x83 here;
                                     //   dialog 4 (drop an item) hands it to RequestDropItem
    INT Unknown238;                  // +0x238 a number the keypad's key 11 types; the NDialogInfo's +0x74

    NCGaraDialogBox();                                               // 0x10006f90
    // ~NCGaraDialogBox                                              // 0x10007090

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x100065f0
    virtual void SetFocus();                                         // 29 0x100066d0
    virtual INT OnCreate();                                          // 36 0x10007110
    virtual INT OnDestroy();                                         // 37 0x100076f0
    virtual void ShowWindow();                                       // 43 0x10009500
    virtual void HideWindow();                                       // 44 0x10006790
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x10007560
    virtual INT OnKeyDown(WPARAM wParam, LPARAM lParam);             // 58 0x100066f0
    // names ours (C1's OnOK .. OnProgressEnd, which kept their slots 68 .. 72 here)
    virtual INT OnOK();                                              // 68 0x10007b20
    virtual INT OnCancel();                                          // 69 0x10006860
    virtual INT OnEditNotify(INT Code);                              // 70 0x100067a0
    virtual INT OnKeypad(INT Key);                                   // 71 0x10008fa0
    virtual INT OnProgressEnd();                                     // 72 0x10006d00 (the Cancel's)

    // names ours (the other windows' stubs on NCDialogBox reach these)
    __declspec(noinline) void Reset();                               // 0x10006740 the answer's state cleared (ParamText and the params are kept)
    void SetButtonTexts(INT OKTextID, INT CancelTextID);             // 0x10006c50 sysstring ids
    NWndEntry* GetOwner();                                           // 0x10006cf0
    void UpdateLayout();                                             // 0x10007860
    void SetInfo(NDialogInfo* Info);                                 // 0x10007a00
    void SetEditText(const TCHAR* Text);                             // 0x10006cc0 typed into the edit line (when it shows)
};
NW_ASSERT_OFFSET(NCGaraDialogBox, BackTex, 0x1A4);
NW_ASSERT_OFFSET(NCGaraDialogBox, OKButton, 0x1B0);
NW_ASSERT_OFFSET(NCGaraDialogBox, Keypad, 0x1B8);
NW_ASSERT_OFFSET(NCGaraDialogBox, Progress, 0x1EC);
NW_ASSERT_OFFSET(NCGaraDialogBox, EditBox, 0x1F0);
NW_ASSERT_OFFSET(NCGaraDialogBox, DialogID, 0x1F4);
NW_ASSERT_OFFSET(NCGaraDialogBox, Owner, 0x1FC);
NW_ASSERT_OFFSET(NCGaraDialogBox, bProgress, 0x208);
NW_ASSERT_OFFSET(NCGaraDialogBox, Message, 0x20C);
NW_ASSERT_OFFSET(NCGaraDialogBox, ParamText, 0x218);
NW_ASSERT_OFFSET(NCGaraDialogBox, Param1, 0x224);
NW_ASSERT_OFFSET(NCGaraDialogBox, Unknown22C, 0x22C);
NW_ASSERT_OFFSET(NCGaraDialogBox, Unknown238, 0x238);
NW_ASSERT_SIZE(NCGaraDialogBox, 0x23C);

#pragma pack(pop)
