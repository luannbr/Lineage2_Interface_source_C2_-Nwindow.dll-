#pragma once
// NDialogInfo - what a dialog box shows (name ours; 0x78 bytes in C2, 0x162C in C1: the texts are
// FStrings now; built on the stack). Windows fill one and hand it to NConsoleWnd::ShowDialog, which
// opens one of the desktop's dialog boxes (NCDialogBox's source file keeps these helpers). The answer
// goes to Owner with ID. Field meanings after the callers seen so far.
//
// C2 layout: from the constructor (0x10006E60: the six strings, the cleared ints and the three
// defaults), its destructor (0x100067D0: the strings in reverse) and the setters (0x100064B0..0x100065C0).
// C1 offsets in brackets.

#include "nwindow/NWindow.h"

struct NWndEntry;

struct NDialogInfo
{
    INT ID;                    // +0x00 (-1) what the box does with the answer (NCDialogBox)
    INT Type;                  // +0x04 (3) type of the box's edit line (NCEditBox: 3 digits,
                               //   with the keypad)
    NWndEntry* Owner;          // +0x08 window that gets the answer
    UBOOL bMainBox;            // +0x0C (1) ShowDialog: 1 = the desktop's box at +0x148,
                               //   0 = the one at +0x14C (C1 offsets)
    UBOOL bInput;              // +0x10 (0) the box has an edit line
    UBOOL bLargeAmount;        // +0x14 amounts up to 100,000,000 instead of 1,000,000
    UBOOL bProgress;           // +0x18 a progress bar runs; when it ends the box cancels
    FString Text;              // +0x1C the message (SetText, SetSystemMessage) (C1: TCHAR[1024])
    FString Text2;             // +0x28 shown instead of Text when set (C1 +0x81C)
    FString Button1;           // +0x34 (SetButtons) (C1 +0x101C)
    FString Button2;           // +0x40 (C1 +0x121C)
    FString ParamText;         // +0x4C a name the answer carries (SetParamText) (C1 +0x141C)
    INT Param1;                // +0x58 numbers the answer carries (0 = keep the box's) (C1 +0x161C)
    INT Param2;                // +0x5C (C1 +0x1620)
    FString Text60;            // +0x60 new in C2 (name ours; the setter 0x100065C0 is the only one that touches it)
    INT Unknown1624;           // +0x6C (C1 +0x1624)
    INT Unknown1628;           // +0x70 (C1 +0x1628)
    INT Unknown74;             // +0x74 new in C2

    NDialogInfo();                                                   // 0x10006e60 (0x10006e20 is another small record's)
    void SetText(const TCHAR* InText);                               // 0x10006550
    void SetSystemMessage(INT MsgID);                                // 0x10006500 systemmsg line
    void SetButtons(const TCHAR* InButton1, const TCHAR* InButton2); // 0x100064b0
    void SetParamText(const TCHAR* InText);                          // 0x10006570
    void SetParams(INT InParam1, INT InParam2);                      // 0x100065a0
    void SetText60(const TCHAR* InText);                             // 0x100065c0 (name ours)
};
NW_ASSERT_OFFSET(NDialogInfo, Text, 0x1C);
NW_ASSERT_OFFSET(NDialogInfo, Text2, 0x28);
NW_ASSERT_OFFSET(NDialogInfo, Button1, 0x34);
NW_ASSERT_OFFSET(NDialogInfo, Button2, 0x40);
NW_ASSERT_OFFSET(NDialogInfo, ParamText, 0x4C);
NW_ASSERT_OFFSET(NDialogInfo, Param1, 0x58);
NW_ASSERT_OFFSET(NDialogInfo, Text60, 0x60);
NW_ASSERT_OFFSET(NDialogInfo, Unknown1624, 0x6C);
NW_ASSERT_OFFSET(NDialogInfo, Unknown74, 0x74);
NW_ASSERT_SIZE(NDialogInfo, 0x78);
