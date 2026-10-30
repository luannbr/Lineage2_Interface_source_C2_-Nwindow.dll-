/*=============================================================================
    NCEmpowerWnd.cpp - the clan member privileges window (C2 only). Stock: 0x10070ca0 ..
    0x10070d4f (WindowProc), 0x10070d50 .. 0x10070e6f, 0x10070e70 .. 0x10071a7f (OnCreate and
    OnDestroy), 0x10071a80 .. 0x1007221f (the helpers and the small virtuals) and
    0x100a0830 (OnCancel, shared with NCOptionWnd's).
=============================================================================*/
#include "nwindow/NCEmpowerWnd.h"
#include "nwindow/NCButton.h"
#include "nwindow/NCCheckBox.h"
#include "nwindow/NCFrameCtrl.h"
#include "nwindow/NCGDevice.h"
#include "nwindow/NConsoleWnd.h"

// ids (names ours)
enum
{
    NID_EMPOWER_FRAME = (INT)0xFF0F1B01,
    NID_EMPOWER_CHECK = (INT)0xFF0F1A01,
    NID_EMPOWER_OK = (INT)0xFF0F1A11,        // the cancel button has it too
    NID_EMPOWER_HEAD_A = (INT)0xFF0F1A15,
    NID_EMPOWER_HEAD_B = (INT)0xFF0F1A16,
    NID_EMPOWER_HEAD_C = (INT)0xFF0F1A17,
};
enum { SYSSTR_EmpowerName = 686 };           // the text after the name (name ours)
enum { PLEDGEPOWER_Set = 3 };                // the second argument of RequestPledgePower (name ours)

// the three groups of the ten privileges: the first box and the count (names ours)
static INT GroupOf(INT Bit)
{
    return Bit < 4 ? 0 : (Bit < 7 ? 1 : 2);
}

// the thirteen check boxes in the order OnCreate makes them: the index in Checks, the label
// (a system string), the id, the place (names ours)
struct NEmpowerCheckDesc
{
    INT Index;
    INT LabelID;
    INT ID;
    INT X;
    INT Y;
};
static const NEmpowerCheckDesc GEmpowerChecks[13] = {
    { 10, 0x2A1, NID_EMPOWER_HEAD_A, 10, 60 },
    { 0, 0x2A2, NID_EMPOWER_CHECK, 20, 75 },
    { 1, 0x2A3, NID_EMPOWER_CHECK, 20, 90 },
    { 2, 0x2A4, NID_EMPOWER_CHECK, 20, 105 },
    { 3, 0x2A5, NID_EMPOWER_CHECK, 20, 120 },
    { 11, 0x2A6, NID_EMPOWER_HEAD_B, 10, 150 },
    { 4, 0x2A7, NID_EMPOWER_CHECK, 20, 165 },
    { 5, 0x2AC, NID_EMPOWER_CHECK, 20, 180 },
    { 6, 0x2AD, NID_EMPOWER_CHECK, 20, 195 },
    { 12, 0x2AA, NID_EMPOWER_HEAD_C, 10, 225 },
    { 7, 0x2AB, NID_EMPOWER_CHECK, 20, 240 },
    { 8, 0x2AC, NID_EMPOWER_CHECK, 20, 255 },
    { 9, 0x2AD, NID_EMPOWER_CHECK, 20, 270 },
};

// a child window goes (repeated inline in the stock OnDestroy)
template <class T> static FORCEINLINE void DestroyChild(NCWnd* Parent, T*& Child)
{
    if (Child)
    {
        Parent->RemoveChildWindow(Child->Entry);
        delete Child;
        Child = NULL;
    }
}

// 0x10070d50 - the privileges, the member and the mode are empty; the name is an empty FString
NCEmpowerWnd::NCEmpowerWnd()
{
    for (INT i = 0; i < 32; i++)
        Power[i] = 0;
    MemberID = 0;
    Mode = 0;
    BackTex = NULL;
}
NW_HOOK_CTOR(0x10070d50, NCEmpowerWnd)

// 0x10070e00
NW_HOOK_DTOR(0x10070e00, NCEmpowerWnd)

// 0x10070ca0 (vslot 25)
INT NCEmpowerWnd::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (NCWnd::WindowProc(Msg, wParam, lParam))
        return 1;
    if (Msg == NWM_COMMAND)
    {
        if ((INT)wParam == NID_EMPOWER_OK)
            return OnOK() != 0;
        if ((INT)wParam == NID_EMPOWER_HEAD_A)
            return OnHeadA() != 0;
        if ((INT)wParam == NID_EMPOWER_HEAD_B)
            return OnHeadB() != 0;
        if ((INT)wParam == NID_EMPOWER_HEAD_C)
            return OnHeadC() != 0;
    }
    return 0;
}
NW_HOOK_METHOD(0x10070ca0, NCEmpowerWnd, WindowProc)

// 0x10071a80 - every box unchecked (the null tests are the stock code's)
void NCEmpowerWnd::ClearChecks()
{
    for (INT i = 0; i < 3; i++)
    {
        if (Checks[i])
            Checks[i]->bChecked = 0;
        if (Checks[4 + i])
            Checks[4 + i]->bChecked = 0;
        if (Checks[7 + i])
            Checks[7 + i]->bChecked = 0;
        if (Checks[10 + i])
            Checks[10 + i]->bChecked = 0;
    }
    if (Checks[3])
        Checks[3]->bChecked = 0;
}
NW_HOOK_METHOD(0x10071a80, NCEmpowerWnd, ClearChecks)

// 0x10071ae0 - the boxes of the bits set are checked, and the head of their group with them
void NCEmpowerWnd::ShowPower()
{
    ClearChecks();
    for (INT i = 0; i < 10; i++)
    {
        if (Power[i >> 3] & (1 << (i & 7)))
        {
            Checks[i]->bChecked = 1;
            Checks[10 + GroupOf(i)]->bChecked = 1;
        }
    }
}
NW_HOOK_METHOD(0x10071ae0, NCEmpowerWnd, ShowPower)

// 0x10071c40 - the boxes checked are the bits of Power (the others cleared)
void NCEmpowerWnd::CollectPower()
{
    for (INT i = 0; i < 32; i++)
        Power[i] = 0;
    for (INT i = 0; i < 10; i++)
    {
        if (Checks[i]->bChecked)
            Power[i >> 3] |= (BYTE)(1 << (i & 7));
    }
}
NW_HOOK_METHOD(0x10071c40, NCEmpowerWnd, CollectPower)

// 0x10071f30 - nothing shown, nobody, no name, every box enabled
void NCEmpowerWnd::Reset()
{
    ClearChecks();
    for (INT i = 0; i < 32; i++)
        Power[i] = 0;
    MemberID = 0;
    Mode = 0;
    Name.Empty();
    for (INT i = 0; i < 13; i++)
        Checks[i]->bDisabled = 0;
}
NW_HOOK_METHOD(0x10071f30, NCEmpowerWnd, Reset)

// 0x100721f0 (ret 0xC) - the console's answer: the 32 bytes of privileges (the first two arguments
// are ignored)
void NCEmpowerWnd::SetPower(INT P1, INT P2, const BYTE* NewPower)
{
    for (INT i = 0; i < 32; i++)
        Power[i] = 0;
    for (INT i = 0; i < 32; i++)
        Power[i] = NewPower[i];
    ShowPower();
}
NW_HOOK_METHOD(0x100721f0, NCEmpowerWnd, SetPower)

// 0x10071d20 (vslot 67) - OK: the privileges of a member are sent when they could be changed
INT NCEmpowerWnd::OnOK()
{
    if (MemberID > 0 && Mode == 2)
    {
        CollectPower();
        GConsole->RequestPledgePower(MemberID, PLEDGEPOWER_Set, Power);
    }
    HideWindow();
    return 1;
}
NW_HOOK_METHOD(0x10071d20, NCEmpowerWnd, OnOK)

// 0x100a0830 (vslot 68) - the stock body is one with NCOptionWnd's (hooked there); nothing calls it here
INT NCEmpowerWnd::OnCancel()
{
    HideWindow();
    return 1;
}

// the head of a group: its boxes follow it (not while the head is disabled)
#define EMPOWER_HEAD(Head, First, Count)                                           \
    {                                                                              \
        NCCheckBox* H = Checks[Head];                                              \
        if (H->bDisabled == 0)                                                     \
        {                                                                          \
            if (H->bChecked != 0)                                                  \
            {                                                                      \
                for (INT i = First; i < First + Count; i++)                        \
                    Checks[i]->bChecked = 1;                                       \
                return 1;                                                          \
            }                                                                      \
            if (H->bChecked == 0)                                                  \
            {                                                                      \
                for (INT i = First; i < First + Count; i++)                        \
                    Checks[i]->bChecked = 0;                                       \
            }                                                                      \
        }                                                                          \
        return 1;                                                                  \
    }

// 0x10071d70 (vslot 69)
INT NCEmpowerWnd::OnHeadA()
{
    EMPOWER_HEAD(10, 0, 4)
}
NW_HOOK_METHOD(0x10071d70, NCEmpowerWnd, OnHeadA)

// 0x10071e10 (vslot 70)
INT NCEmpowerWnd::OnHeadB()
{
    EMPOWER_HEAD(11, 4, 3)
}
NW_HOOK_METHOD(0x10071e10, NCEmpowerWnd, OnHeadB)

// 0x10071ea0 (vslot 71)
INT NCEmpowerWnd::OnHeadC()
{
    EMPOWER_HEAD(12, 7, 3)
}
NW_HOOK_METHOD(0x10071ea0, NCEmpowerWnd, OnHeadC)

#undef EMPOWER_HEAD

// 0x10072010 (vslot 43) - in view mode every box is disabled
void NCEmpowerWnd::ShowWindow()
{
    NCWnd::ShowWindow();
    if (Mode == 1)
    {
        for (INT i = 0; i < 13; i++)
            Checks[i]->bDisabled = 1;
    }
}
NW_HOOK_METHOD(0x10072010, NCEmpowerWnd, ShowWindow)

// 0x100720d0 (vslot 44) - closing empties it
void NCEmpowerWnd::HideWindow()
{
    NCWnd::HideWindow();
    Reset();
}
NW_HOOK_METHOD(0x100720d0, NCEmpowerWnd, HideWindow)

// 0x10070e70 (vslot 36) - the title bar, the thirteen boxes and the two buttons
INT NCEmpowerWnd::OnCreate()
{
    NCWnd::OnCreate();
    BackTex = FindTexture(TEXT("l2ui.actionwnd.empowerback"));

    FrameCtrl = new NCFrameCtrl(1);
    FrameCtrl->SetWindowRect(0, 0, 240, 21);
    FrameCtrl->ID = NID_EMPOWER_FRAME;
    CreateChild(FrameCtrl, NWS_VISIBLE);

    for (INT i = 0; i < 13; i++)
    {
        const NEmpowerCheckDesc& Desc = GEmpowerChecks[i];
        NCCheckBox* Box = new NCCheckBox(Desc.LabelID, 0, 0);
        Checks[Desc.Index] = Box;
        Box->ID = Desc.ID;
        Box->SetWindowRect(Desc.X, Desc.Y, 200, 12);
        CreateChild(Box, NWS_VISIBLE);
    }

    OKButton = new NCButton(TEXT("L2UI.control.DefaultButton"), TEXT("L2UI.control.DefaultButton_click"), 0);
    OKButton->ID = NID_EMPOWER_OK;
    OKButton->SetWindowRect(52, 328, 67, 19);
    CreateChild(OKButton, NWS_VISIBLE);
    OKButton->SetTextID(140);

    CancelButton = new NCButton(TEXT("L2UI.control.DefaultButton"), TEXT("L2UI.control.DefaultButton_click"), 0);
    CancelButton->ID = NID_EMPOWER_OK;
    CancelButton->SetWindowRect(130, 328, 67, 19);
    CreateChild(CancelButton, NWS_VISIBLE);
    CancelButton->SetTextID(141);
    return 1;
}
NW_HOOK_METHOD(0x10070e70, NCEmpowerWnd, OnCreate)

// 0x100718b0 (vslot 37) - guarded in C2; NCWnd's OnDestroy comes first
INT NCEmpowerWnd::OnDestroy()
{
    guard(NCEmpowerWnd::OnDestroy);
    NCWnd::OnDestroy();
    for (INT i = 0; i < 3; i++)
    {
        DestroyChild(this, Checks[i]);
        DestroyChild(this, Checks[4 + i]);
        DestroyChild(this, Checks[7 + i]);
        DestroyChild(this, Checks[10 + i]);
    }
    DestroyChild(this, Checks[3]);
    DestroyChild(this, OKButton);
    DestroyChild(this, CancelButton);
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x100718b0, NCEmpowerWnd, OnDestroy)

// 0x100720e0 (vslot 55) - the panel (331 high) under the title bar and, when there is a name, the
// name and its text; the clip goes on before the origin
INT NCEmpowerWnd::OnPaint(NCGDevice* Device)
{
    TCHAR Text[256];
    Device->SetClip(ScreenX, ScreenY, Width, Height);
    Device->BeginPaint(ScreenX, ScreenY);
    Device->DrawTexture(0, 21, Width, 331, 0, 0, Width, 331, BackTex);
    if (Name.Num() != 0)
    {
        appSprintf(Text, TEXT("%s%s"), *Name, *SysString(SYSSTR_EmpowerName));
        Device->DrawText(10, 28, 0xFFFFFFFF, Text, 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0, 0);
    }
    Device->EndPaint();
    Device->ReleaseClip();
    return 1;
}
NW_HOOK_METHOD(0x100720e0, NCEmpowerWnd, OnPaint)

NW_CHECK_VTABLE(NCEmpowerWnd, 0x10116450, 72)

// 0x10040930 (name ours) - the pledge window sets whose privileges these are: the member's id and the mode, and the name
// (kept when none is given)
void NCEmpowerWnd::SetMember(INT InMemberID, const TCHAR* InName, INT InMode)
{
    MemberID = InMemberID;
    Mode = InMode;
    if (InName)
        Name = InName;
}
NW_HOOK_METHOD(0x10040930, NCEmpowerWnd, SetMember)
