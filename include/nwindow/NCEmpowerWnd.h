#pragma once
// NCEmpowerWnd - the clan member privileges ("pledge power") window (UClass "CEmpowerWnd", 0x10C;
// C2 only; NCConsole +0xA0): ten check boxes in three groups (4 + 3 + 3), a check box that
// heads each group and sets or clears all of its group's, and two buttons. The ten boxes are the
// bits of the privileges the server keeps for a rank (Power: bit i of the 32 bytes is box i);
// the console fills it (SetPower, when the server answers) after setting MemberID, Mode and
// Name. Mode 1 only shows them (every box disabled), mode 2 lets them be changed and OK
// sends the new bits for MemberID.

#include "nwindow/NCWnd.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCButton;
class NCCheckBox;

class NCEmpowerWnd : public NCWnd
{
    NW_DECLARE_CLASS(NCEmpowerWnd, NCWnd, 0, 0x101B3678)

    NCCheckBox* Checks[13];          // +0x98 (names ours) 0..9 the privileges (groups 0-3, 4-6, 7-9),
                                     //   10..12 the heads of the three groups
    NCButton* OKButton;              // +0xCC text 140 (id 0xFF0F1A11)
    NCButton* CancelButton;          // +0xD0 text 141 (the same id: it does what OK does)
    UTexture* BackTex;               // +0xD4 "l2ui.actionwnd.empowerback"
    BYTE Power[32];                  // +0xD8 the privileges as bits (name ours)
    INT MemberID;                    // +0xF8 whose privileges these are, 0 = none (name ours)
    INT Mode;                        // +0xFC 1 view, 2 edit (name ours)
    FString Name;                    // +0x100 shown above the boxes (name ours)

    NCEmpowerWnd();                                                  // 0x10070d50
    // ~NCEmpowerWnd                                                 // 0x10070e00

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x10070ca0
    virtual INT OnCreate();                                          // 36 0x10070e70
    virtual INT OnDestroy();                                         // 37 0x100718b0
    virtual void ShowWindow();                                       // 43 0x10072010
    virtual void HideWindow();                                       // 44 0x100720d0
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x100720e0
    virtual INT OnOK();                                              // 67 0x10071d20 (name ours)
    virtual INT OnCancel();                                          // 68 0x100a0830 (name ours; NCOptionWnd's body: HideWindow)
    virtual INT OnHeadA();                                           // 69 0x10071d70 (name ours) the first group's head
    virtual INT OnHeadB();                                           // 70 0x10071e10 (name ours)
    virtual INT OnHeadC();                                           // 71 0x10071ea0 (name ours)

    // names ours
    __declspec(noinline) void ClearChecks();                         // 0x10071a80 every box unchecked
    __declspec(noinline) void ShowPower();                           // 0x10071ae0 the boxes (and heads) of the bits set, checked
    __declspec(noinline) void CollectPower();                        // 0x10071c40 the boxes into Power
    __declspec(noinline) void Reset();                               // 0x10071f30 everything back to nothing
    void SetPower(INT P1, INT P2, const BYTE* NewPower);             // 0x100721f0 (ret 0xC; the first two are ignored)
    void SetMember(INT InMemberID, const TCHAR* InName, INT InMode); // 0x10040930 (name ours) whose privileges (the member's id, name, 1 view / 2 edit)
};
NW_ASSERT_OFFSET(NCEmpowerWnd, Checks, 0x98);
NW_ASSERT_OFFSET(NCEmpowerWnd, OKButton, 0xCC);
NW_ASSERT_OFFSET(NCEmpowerWnd, BackTex, 0xD4);
NW_ASSERT_OFFSET(NCEmpowerWnd, Power, 0xD8);
NW_ASSERT_OFFSET(NCEmpowerWnd, MemberID, 0xF8);
NW_ASSERT_OFFSET(NCEmpowerWnd, Name, 0x100);
NW_ASSERT_SIZE(NCEmpowerWnd, 0x10C);

#pragma pack(pop)
