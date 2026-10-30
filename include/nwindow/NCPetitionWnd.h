#pragma once
// NCPetitionWnd - the player's petition window (UClass "CPetitionWnd", 0xA8; NConsoleWnd
// +0x36EC): the conversation with the GM (NCPetitionListBox), a line to write in (Enter sends
// it as chat type 6) and "Cancel petition" (sysstring 473). NCGMPetitionWnd is the GM's side
// (desktop +0xAC, the same layout): its line takes "//" builder commands too, and "Start" /
// "End" (474 / 475) send the builder commands startpet / endpet. The desktop adds the
// server's petition chat to them (AddMessage).

#include "nwindow/NCWnd.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCEditBox;
class NCPetitionListBox;
class NCButton;

// NWM_COMMAND ids of the two windows' controls
enum
{
    NID_PETITION_LIST = 0xF0A0B1,
    NID_PETITION_EDIT = 0xF0A0B2,
    NID_PETITION_SEND = 0xF0A0B3,      // no such control (the command does nothing)
    NID_PETITION_CANCEL = 0xF0A0B4,
    NID_PETITION_FRAME = 0xF0A0B5,
    NID_GMPETITION_LIST = 0xF0A0C1,
    NID_GMPETITION_EDIT = 0xF0A0C2,
    NID_GMPETITION_START = 0xF0A0C3,
    NID_GMPETITION_END = 0xF0A0C4,
    NID_GMPETITION_FRAME = 0xF0A0C5,
};

class NCPetitionWnd : public NCWnd
{
    NW_DECLARE_CLASS(NCPetitionWnd, NCWnd, 0, 0x101CFB78)

    UTexture* BackTex;                     // +0x98 "L2UI.etcwnd.Petitionback"
    NCEditBox* EditBox;                    // +0x9C
    NCPetitionListBox* ListBox;            // +0xA0
    NCButton* UnusedA0;                    // +0xA4 (NCGMPetitionWnd's Start button)   // C2: offset derived, no evidence
    NCButton* CancelButton;                // +0xA8

    NCPetitionWnd();                                                 // 0x10057be0
    // ~NCPetitionWnd                                                // 0x10057c30

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x100b6470
    virtual INT OnCreate();                                          // 36 0x100b82e0
    virtual INT OnDestroy();                                         // 37 0x100b6e00
    virtual void ShowWindow();                                       // 43 0x1008c9c0 (folded)
    virtual void HideWindow();                                       // 44 0x1007b470 (folded)
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x100b71d0
    // names ours
    virtual INT OnEdit(LPARAM lParam);                               // 67 0x100b6f00 1: Enter
    virtual INT OnSend() { return 1; }                               // 68 0x10005b80 (folded)
    virtual INT OnCancel();                                          // 69 0x100b6510 the petition is cancelled (C1: slot 64)

    // names ours (also called on the desktop's NCGMPetitionWnd)
    void AddMessage(TCHAR* Text, DWORD Color, INT Type, INT Param);  // 0x100b6740
    void Clear();                                                    // 0x100b6780
};
NW_ASSERT_OFFSET(NCPetitionWnd, EditBox, 0x9C);
NW_ASSERT_OFFSET(NCPetitionWnd, CancelButton, 0xA8);
NW_ASSERT_SIZE(NCPetitionWnd, 0xAC);

class NCGMPetitionWnd : public NCWnd
{
    NW_DECLARE_CLASS(NCGMPetitionWnd, NCWnd, 0, 0x101D1010)

    UTexture* BackTex;                     // +0x98 "L2UI.etcwnd.Petitionback"
    NCEditBox* EditBox;                    // +0x9C
    NCPetitionListBox* ListBox;            // +0xA0
    NCButton* StartButton;                 // +0xA4
    NCButton* EndButton;                   // +0xA8

    NCGMPetitionWnd();                                               // 0x1004c600
    // ~NCGMPetitionWnd                                              // 0x1004c650

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x100b6530
    virtual INT OnCreate();                                          // 36 0x100b85c0
    virtual INT OnDestroy();                                         // 37 0x100b6f90
    virtual void ShowWindow();                                       // 43 0x1008c9c0 (folded)
    virtual void HideWindow();                                       // 44 0x1007b470 (folded)
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x100b71d0 (folded)
    // names ours
    virtual INT OnEdit(LPARAM lParam);                               // 67 0x100b65d0 1: Enter
    virtual INT OnStart();                                           // 68 0x100b67a0
    virtual INT OnEnd();                                             // 69 0x100b6820

    // names ours: NCPetitionWnd's bodies
    void AddMessage(TCHAR* Text, DWORD Color, INT Type, INT Param);  // 0x100b6740 (folded)
    void Clear();                                                    // 0x100b6780 (folded)
};
NW_ASSERT_OFFSET(NCGMPetitionWnd, EditBox, 0x9C);
NW_ASSERT_OFFSET(NCGMPetitionWnd, EndButton, 0xA8);
NW_ASSERT_SIZE(NCGMPetitionWnd, 0xAC);

#pragma pack(pop)