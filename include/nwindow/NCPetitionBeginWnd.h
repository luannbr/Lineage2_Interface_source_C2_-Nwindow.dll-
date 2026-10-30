#pragma once
// NCPetitionBeginWnd - the form that opens a petition (UClass "CPetitionBeginWnd", 0xB0; C2
// only; the system menu and the console open it): a combo box with the nine kinds of petition,
// the help page of the kind chosen (an NCHtmlViewer), a multi-line edit for the text and OK /
// Cancel. OK checks the text (5 to 254 characters, a kind chosen), sends the petition and
// starts the conversation window (NCPetitionWnd) with it; Cancel withdraws one already open.

#include "nwindow/NCWnd.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCButton;
class NCComboBox;
class NCHtmlViewer;
class NCMultiEdit;

class NCPetitionBeginWnd : public NCWnd
{
    NW_DECLARE_CLASS(NCPetitionBeginWnd, NCWnd, 0, 0x101D0AE8)

    NCButton* OKButton;              // +0x98 text 694 (name ours)
    NCButton* CancelButton;          // +0x9C text 473 (name ours)
    NCHtmlViewer* HelpViewer;        // +0xA0 the help page of the kind chosen (name ours)
    NCComboBox* TypeCombo;           // +0xA4 the kinds: 1 move .. 9 etc (0 = none chosen) (name ours)
    NCMultiEdit* TextEdit;           // +0xA8 the petition (name ours)
    UTexture* BackTex;               // +0xAC "L2UI.PetitionWnd.PetitionBeginBack"

    NCPetitionBeginWnd();                                            // 0x1004c740
    // ~NCPetitionBeginWnd                                           // 0x1004c790

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x100b6980
    virtual INT OnCreate();                                          // 36 0x100b7680
    virtual INT OnDestroy();                                         // 37 0x100b7b00
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x100b7c50
    virtual INT OnSend();                                            // 67 0x100b7db0 (name ours) OK
    virtual INT OnCancel();                                          // 68 0x100b7f30 (name ours)
    virtual INT OnTypeChosen(INT Type);                              // 69 0x100b6a20 (name ours) the combo's selection

    // name ours: the form emptied (kind 0, no text, no help page)
    __declspec(noinline) void ResetForm();                           // 0x100b6b40
};
NW_ASSERT_OFFSET(NCPetitionBeginWnd, OKButton, 0x98);
NW_ASSERT_OFFSET(NCPetitionBeginWnd, HelpViewer, 0xA0);
NW_ASSERT_OFFSET(NCPetitionBeginWnd, TextEdit, 0xA8);
NW_ASSERT_OFFSET(NCPetitionBeginWnd, BackTex, 0xAC);
NW_ASSERT_SIZE(NCPetitionBeginWnd, 0xB0);

#pragma pack(pop)
