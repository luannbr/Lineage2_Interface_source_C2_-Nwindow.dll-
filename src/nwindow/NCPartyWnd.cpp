/*=============================================================================
    NCPartyWnd.cpp - the party window. Stock: 0x100ae9b0 .. 0x100aea8b,
    0x100b1720 .. 0x100b1bdf, 0x100b34f0 .. 0x100b36d3.
=============================================================================*/
#include "nwindow/NCPartyWnd.h"
#include "nwindow/NCStatusWnd.h"
#include "nwindow/NCFrameCtrl.h"
#include "nwindow/NCGDevice.h"

// NWM_COMMAND id that shows / hides the member list (no sender found in NWindow)
enum { NID_PARTY_TOGGLE = 0xF0 };

// title bar id
enum { NID_PARTY_FRAME = 0xFFA0B0 };

// 0x100d6230
NCPartyWnd::NCPartyWnd()
{
    bShown = 0;
    NumMembers = 0;
}
NW_HOOK_CTOR(0x100d6230, NCPartyWnd)

// 0x100d6280
NW_HOOK_DTOR(0x100d6280, NCPartyWnd)

// 0x100d3500 (vslot 25) - C2 no longer checks IsEnabled first (C1 did)
INT NCPartyWnd::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (NCWnd::WindowProc(Msg, wParam, lParam))
        return 1;
    if (Msg == NWM_COMMAND && (INT)wParam == NID_PARTY_TOGGLE)
        return OnToggle() != 0;
    return 0;
}
NW_HOOK_METHOD(0x100d3500, NCPartyWnd, WindowProc)

// 0x100d62d0 (vslot 36) - the eight member windows exist from the start, hidden until used
INT NCPartyWnd::OnCreate()
{
    NCWnd::OnCreate();
    FrameTopTex = FindTexture(TEXT("L2UI.TargetWnd.TargetFrameTop"));
    FrameCenterTex = FindTexture(TEXT("L2UI.TargetWnd.TargetFrameCenter"));
    FrameBottomTex = FindTexture(TEXT("L2UI.TargetWnd.TargetFrameBottom"));

    FrameCtrl = new NCFrameCtrl(0);
    FrameCtrl->SetWindowRect(0, 0, 12, Height);
    FrameCtrl->ID = NID_PARTY_FRAME;
    CreateChild(FrameCtrl, NWS_VISIBLE | 0x20);   // 0x20: meaning unknown

    for (INT i = 0; i < NUM_PARTY_SLOTS; i++)
    {
        Members[i] = new NCPartyStatusWnd(144);
        Members[i]->SetWindowRect(12, 0, 140, 36);
        CreateChild(Members[i], NWS_VISIBLE);
    }
    return 1;
}
NW_HOOK_METHOD(0x100d62d0, NCPartyWnd, OnCreate)

// 0x100d6450 (vslot 37) - NCWnd's first, then the members (C2 guards it)
INT NCPartyWnd::OnDestroy()
{
    guard(NCPartyWnd::OnDestroy);
    NCWnd::OnDestroy();
    for (INT i = 0; i < NUM_PARTY_SLOTS; i++)
    {
        if (Members[i])
        {
            RemoveChildWindow(Members[i]->Entry);
            delete Members[i];
            Members[i] = NULL;
        }
    }
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x100d6450, NCPartyWnd, OnDestroy)

// 0x100d6740 (vslot 43)
void NCPartyWnd::ShowWindow()
{
    NCWnd::ShowWindow();
    UpdateLayout();
}
NW_HOOK_METHOD(0x100d6740, NCPartyWnd, ShowWindow)

// 0x100d6500 (vslot 55) - the left edge, while there is a party
INT NCPartyWnd::OnPaint(NCGDevice* Device)
{
    Device->SetClip(ScreenX, ScreenY, Width, Height);
    Device->BeginPaint(ScreenX, ScreenY);
    if (NumMembers > 0)
    {
        if (FrameTopTex)
            Device->DrawTexture(0, 0, 12, 8, 0, 0, 12, 8, FrameTopTex);
        if (FrameCenterTex)
            Device->DrawTexture(0, 7, 12, Height - 13, 0, 0, 11, 16, FrameCenterTex);
        if (FrameBottomTex)
            Device->DrawTexture(0, Height - 7, 12, 7, 0, 0, 12, 7, FrameBottomTex);
    }
    Device->EndPaint();
    Device->ReleaseClip();
    return 1;
}
NW_HOOK_METHOD(0x100d6500, NCPartyWnd, OnPaint)

// 0x100d8470 (vslot 67)
INT NCPartyWnd::OnToggle()
{
    if (bShown)
    {
        bShown = 0;
        UpdateLayout();
        return 1;
    }
    bShown = 1;
    UpdateLayout();
    return 1;
}
NW_HOOK_METHOD(0x100d8470, NCPartyWnd, OnToggle)

// 0x100d3560 - new HP / MP of a member, found by name among the first NumMembers; C2 hides
// the window when the list is empty afterwards (C1 did not)
void NCPartyWnd::UpdateMember(INT MemberID, const TCHAR* Name, INT HP, INT MaxHP, INT MP, INT MaxMP)
{
    if (!Name)
        return;
    for (INT i = 0; i < NumMembers; i++)
    {
        if (Members[i]->IsName(Name))
        {
            Members[i]->SetInfo(MemberID, Name, HP, MaxHP, MP, MaxMP);
            break;
        }
    }
    if (NumMembers == 0)
        HideWindow();
}
NW_HOOK_METHOD(0x100d3560, NCPartyWnd, UpdateMember)

// 0x100d65e0 - the members shown stacked under each other, 34 pixels apart (36 for the
// first); the window takes their height. Kept out of line like the stock code.
__declspec(noinline) void NCPartyWnd::UpdateLayout()
{
    if (bShown)
    {
        SetSize(152, 0);
        // the stock code fills { LocalX, LocalY, LocalW, LocalH }; SetScreenPos reads x and y
        POINT Origin;
        Origin.x = LocalX;
        Origin.y = LocalY;
        for (INT i = 0; i < NUM_PARTY_SLOTS; i++)
            Members[i]->SetScreenPos(12, 0, &Origin);
        UBOOL bAny = 0;
        for (INT i = 0; i < NUM_PARTY_SLOTS; i++)
        {
            NCPartyStatusWnd* Member = Members[i];
            if (!Member->IsEmpty())
            {
                Member->ShowWindow();
                if (!bAny)
                {
                    SetSize(152, LocalH + 36);
                    bAny = 1;
                }
                else
                    SetSize(152, LocalH + 34);
                for (INT j = i + 1; j < NUM_PARTY_SLOTS; j++)
                    Members[j]->OffsetWindow(0, 34);
            }
            else
                Member->HideWindow();
        }
    }
    else
    {
        for (INT i = 0; i < NUM_PARTY_SLOTS; i++)
            Members[i]->HideWindow();
    }
    FrameCtrl->SetWindowRect(0, 0, 12, Height);
}
NW_HOOK_METHOD(0x100d65e0, NCPartyWnd, UpdateLayout)

// 0x100d82a0 - into the first free slot; C2 shows the window first (vslot 43), the list shows itself
void NCPartyWnd::AddMember(INT MemberID, const TCHAR* Name, INT HP, INT MaxHP, INT MP, INT MaxMP)
{
    if (!Name)
        return;
    ShowWindow();
    for (INT i = 0; i < NUM_PARTY_SLOTS; i++)
    {
        if (Members[i]->IsEmpty())
        {
            Members[i]->SetInfo(MemberID, Name, HP, MaxHP, MP, MaxMP);
            NumMembers++;
            bShown = 1;
            UpdateLayout();
            return;
        }
    }
}
NW_HOOK_METHOD(0x100d82a0, NCPartyWnd, AddMember)

// 0x100d8330 - the members below move up a slot; C2 hides the window with the last one
void NCPartyWnd::DeleteMember(INT MemberID, const TCHAR* Name)
{
    if (!Name)
        return;
    for (INT i = 0; i < NUM_PARTY_SLOTS; i++)
    {
        if (Members[i]->IsName(Name))
        {
            Members[i]->Clear();
            for (INT j = i + 1; j < NUM_PARTY_SLOTS; j++)
            {
                NCPartyStatusWnd* Member = Members[j];
                if (!Member->IsEmpty())
                {
                    Members[j - 1]->SetInfo(Member->MemberID, Member->Name, Member->HP,
                                            Member->MaxHP, Member->MP, Member->MaxMP);
                    Member->Clear();
                }
            }
            if (--NumMembers == 0)
            {
                bShown = 0;
                HideWindow();
            }
            UpdateLayout();
            return;
        }
    }
}
NW_HOOK_METHOD(0x100d8330, NCPartyWnd, DeleteMember)

// 0x100d8410 - C2 hides the window afterwards. Out of line like the stock code (the console's
// PartyDeleteAll calls it).
__declspec(noinline) void NCPartyWnd::DeleteAll()
{
    for (INT i = 0; i < NumMembers; i++)
        Members[i]->Clear();
    NumMembers = 0;
    bShown = 0;
    UpdateLayout();
    HideWindow();
}
NW_HOOK_METHOD(0x100d8410, NCPartyWnd, DeleteAll)

NW_CHECK_VTABLE(NCPartyWnd, 0x10127150, 68)
