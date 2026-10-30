/*=============================================================================
    NCAbnormalStatusWnd.cpp - the buff / debuff icons. C2 stock: 0x100d2d20 .. 0x100d2dee,
    0x100d37c0 .. 0x100d3bf5, 0x100d8540 .. 0x100d85bf (NAbnormalEntry's constructor), 0x100d8560 ..
    0x100d8a62 (0x100d84b0 is the pointer array's destructor).
=============================================================================*/
#include "nwindow/NCAbnormalStatusWnd.h"
#include "nwindow/NCFrameCtrl.h"
#include "nwindow/NCGDevice.h"
#include "nwindow/NConsoleWnd.h"

// 0x100d8540 - a buff's entry: the item entry (0x1001a850), then its time and the visible flag
NW_HOOK_CTOR(0x100d8540, NAbnormalEntry)

#if NW_FEATURES
// phase 6: BuffTimer (NWindow_C1 src/inject/BuffTimer.cpp)
#include "features/BuffTimer.h"
#endif

// title bar id
enum { NID_ABNORMALSTATUS_FRAME = 0xFFA0B3 };

// 0x100d8560 - the icon textures are left as allocated
NCAbnormalStatusWnd::NCAbnormalStatusWnd(INT InIconSize)
{
    TimerID = 0;
    ToolTipTex = NULL;
    TipTopTex = NULL;
    TipMidTex = NULL;
    TipBottomTex = NULL;
    IconSize = InIconSize;
    HoverIndex = -1;
}
NW_HOOK_CTOR_ARGS(0x100d8560, NCAbnormalStatusWnd, (INT A), (A))

// 0x100d85f0
NW_HOOK_DTOR(0x100d85f0, NCAbnormalStatusWnd)

// stock body shared with NControl's (0x10004e90)
INT NCAbnormalStatusWnd::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (Msg != WM_PAINT && !IsEnabled())
        return 0;
    return NCWnd::WindowProc(Msg, wParam, lParam) != 0;
}

// 0x100d3b90 (vslot 35) - the one second timer: the time runs down, and in the last ten seconds the icon blinks
INT NCAbnormalStatusWnd::OnTimer(INT InTimerID)
{
    if (InTimerID == TIMER_AbnormalStatus)
    {
        for (INT i = 0; i < Statuses.Num(); i++)
        {
            NAbnormalEntry* Status = Statuses(i);
            if (Status->RemainTime > 0)
                Status->RemainTime--;
            if (Status->RemainTime >= 0 && Status->RemainTime < 11)
                Status->bVisible = !Status->bVisible;
        }
    }
    return 1;
}
NW_HOOK_METHOD(0x100d3b90, NCAbnormalStatusWnd, OnTimer)

// 0x100d37c0 (vslot 36)
INT NCAbnormalStatusWnd::OnCreate()
{
    NCWnd::OnCreate();
    TimerID = SetTimer(TIMER_AbnormalStatus, 1000, NULL);
    ToolTipTex = FindTexture(TEXT("L2UI.tooltip.mini-tool01"));
    TipTopTex = FindTexture(TEXT("L2UI.Tooltip.Tool02"));
    TipMidTex = FindTexture(TEXT("L2UI.Tooltip.Tool01"));
    TipBottomTex = FindTexture(TEXT("L2UI.Tooltip.Tool03"));
    BackTex = FindTexture(TEXT("L2UI.EtcWndBack.AbnormalBack"));
    Frame1Tex = FindTexture(TEXT("L2UI.EtcWndBack.AbnormalFrame1"));
    Frame2Tex = FindTexture(TEXT("L2UI.EtcWndBack.AbnormalFrame2"));
    Frame3Tex = FindTexture(TEXT("L2UI.EtcWndBack.AbnormalFrame3"));

    FrameCtrl = new NCFrameCtrl(0);
    FrameID = NID_ABNORMALSTATUS_FRAME;
    FrameCtrl->ID = NID_ABNORMALSTATUS_FRAME;
    FrameCtrl->SetWindowRect(0, 0, 12, 26);
    CreateChild(FrameCtrl, NWS_VISIBLE);
    return 1;
}
NW_HOOK_METHOD(0x100d37c0, NCAbnormalStatusWnd, OnCreate)

// 0x100d89b0 (vslot 37) - the records go (the stock code destroys only their tool tip and frees them: all they own)
INT NCAbnormalStatusWnd::OnDestroy()
{
    guard(NCAbnormalStatusWnd::OnDestroy);
    if (TimerID)
    {
        KillTimer(TimerID);
        TimerID = 0;
    }
    for (INT i = 0; i < Statuses.Num(); i++)
    {
        if (Statuses(i))
            delete Statuses(i);
    }
    Statuses.Empty();
    NCWnd::OnDestroy();
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x100d89b0, NCAbnormalStatusWnd, OnDestroy)

// stock body shared with NCChatWnd's (0x1003dab0)
void NCAbnormalStatusWnd::HideWindow()
{
    NCWnd::HideWindow();
}

// 0x100d3910 (vslot 55) - the icons: ten to a row, behind them a cell and, at the left, the frame of the rows
// shown so far; an icon is not drawn while it blinks off. The clip opens before BeginPaint
INT NCAbnormalStatusWnd::OnPaint(NCGDevice* Device)
{
    guard(NCAbnormalStatusWnd::OnPaint);
    Device->SetClip(ScreenX, ScreenY, Width, Height);
    Device->BeginPaint(ScreenX, ScreenY);
    for (INT i = 0; i < Statuses.Num(); i++)
    {
        NAbnormalEntry* Status = Statuses(i);
        if (Status && Status->Icon)
        {
            if (i < 10)
            {
                Device->DrawTexture(0, 0, 12, 26, 0, 0, 12, 26, Frame1Tex);
                Device->DrawTexture(i * 26 + 12, 0, 26, 26, 0, 0, 26, 26, BackTex);
                if (Status->bVisible)
                    Device->DrawStretchedTex(i * 26 + 13, 1, 24, 24, Status->Icon);
            }
            else if (i < 20)
            {
                Device->DrawTexture(0, 0, 12, 51, 0, 0, 12, 51, Frame2Tex);
                Device->DrawTexture(i * 26 - 0xF8, 25, 26, 26, 0, 0, 26, 26, BackTex);
                if (Status->bVisible)
                    Device->DrawStretchedTex(i * 26 - 0xF7, 26, 24, 24, Status->Icon);
            }
            else if (i < 30)
            {
                Device->DrawTexture(0, 0, 12, 76, 0, 0, 12, 76, Frame3Tex);
                Device->DrawTexture(i * 26 - 0x1FC, 50, 26, 26, 0, 0, 26, 26, BackTex);
                if (Status->bVisible)
                    Device->DrawStretchedTex(i * 26 - 0x1FB, 51, 24, 24, Status->Icon);
            }
        }
    }
    Device->EndPaint();
    Device->ReleaseClip();
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x100d3910, NCAbnormalStatusWnd, OnPaint)

// 0x100d8800 (vslot 56) - the tool tip of the icon under the mouse (C1 had a box of text): the skin's full box at the
// icon's cell, which it moves to stay on the screen. IconSize is set to 26 (the hit test of the paint's cells). A hover
// index outside 0 .. 30 leaves without releasing the clip and the origin, as the stock code does
INT NCAbnormalStatusWnd::OnDrawToolTip(NCGDevice* Device)
{
    Device->BeginPaint(ScreenX, ScreenY);
    Device->SetClip(ScreenX, ScreenY, Width, Height);
    INT Index = HoverIndex;
    IconSize = 26;
    if (Index >= 0 && Index <= 30)
    {
        if (Index < Statuses.Num() && Statuses(Index)->bVisible)
        {
            INT X = 0;   // index 30: the stock leaves it as the stack held it
            if (Index < 10)
                X = Index * 26 + 12;
            else if (Index < 20)
                X = Index * 26 - 0xF8;
            else if (Index < 30)
                X = Index * 26 - 0x1FC;
            INT Y = Index / 10 * 25;
            if (ToolTipSkin)
            {
                NToolTipRect Wnd;
                Wnd.X = ScreenX;
                Wnd.Y = ScreenY;
                Wnd.W = Width;
                Wnd.H = Height;
                ToolTipSkin->DrawFull(Device, Statuses(Index)->ToolTip, &X, &Y, Wnd);
            }
        }
        Device->ReleaseClip();
        Device->EndPaint();
    }
    return 1;
}
NW_HOOK_METHOD(0x100d8800, NCAbnormalStatusWnd, OnDrawToolTip)

// 0x100d3b60 (vslot 60)
INT NCAbnormalStatusWnd::OnMouseMove(WPARAM wParam, LPARAM lParam)
{
    HoverIndex = HitTest(LOWORD(lParam), HIWORD(lParam));
    return 1;
}
NW_HOOK_METHOD(0x100d3b60, NCAbnormalStatusWnd, OnMouseMove)

// 0x100d2d20 - the icon under a screen point, row by row of ten; the title bar is not taken
// off, so the hit areas sit 12 pixels left of the icons, as in the stock code
INT NCAbnormalStatusWnd::HitTest(INT X, INT Y)
{
    INT RX = X - ScreenX;
    INT RY = Y - ScreenY;
    if (RX < 0 || RX > (IconSize + 1) * 10 || RY < 0 || RY > (IconSize + 1) * 3)
        return -1;
    return RY / (IconSize + 1) * 10 + RX / (IconSize + 1);
}
NW_HOOK_METHOD(0x100d2d20, NCAbnormalStatusWnd, HitTest)

// 0x100d2d70 - one to three rows (nothing changes from 30 icons on)
void NCAbnormalStatusWnd::UpdateSize()
{
    INT Count = Statuses.Num();
    if (Count < 10)
    {
        SetSize(Count * 26 + 12, 26);
        FrameCtrl->SetSize(12, 26);
    }
    else if (Count < 20)
    {
        SetSize(272, 51);
        FrameCtrl->SetSize(12, 51);
    }
    else if (Count < 30)
    {
        SetSize(272, 76);
        FrameCtrl->SetSize(12, 76);
    }
}
NW_HOOK_METHOD(0x100d2d70, NCAbnormalStatusWnd, UpdateSize)

// 0x100d8650 - a buff: its record is built from the console's (the icon from its texture name, "NWindow.BlackTexture"
// when missing; the name and the description from the skill tables) with the skill's tool tip; 0 when the window
// holds 31 already
INT NCAbnormalStatusWnd::AddStatus(FAbnormalStatus* Status)
{
    if (Statuses.Num() > 30)
        return 0;
    NAbnormalEntry* Entry = new NAbnormalEntry;
    if (Status->IconName)
        Entry->Icon = FindTexture(Status->IconName);
    if (!Entry->Icon)
        Entry->Icon = FindTexture(TEXT("NWindow.BlackTexture"));
    Entry->UnknownC18 = Status->Level;
    Entry->RemainTime = Status->RemainTime;
    Entry->ID = Status->SkillID;
    if (GConsole->GetSkillName(Status->SkillID, Status->Level))
        appStrcpy(Entry->Name, GConsole->GetSkillName(Status->SkillID, Status->Level));
    if (GConsole->GetSkillDesc(Status->SkillID, Status->Level))
        appStrcpy(Entry->Description, GConsole->GetSkillDesc(Status->SkillID, Status->Level));
    Entry->FillToolTip(NTT_SkillName, 0);
    Statuses.AddItem(Entry);
    UpdateSize();
    return 1;
}
NW_HOOK_METHOD(0x100d8650, NCAbnormalStatusWnd, AddStatus)

// 0x100d8950
void NCAbnormalStatusWnd::ClearStatus()
{
    for (INT i = 0; i < Statuses.Num(); i++)
    {
        if (Statuses(i))
            delete Statuses(i);
    }
    Statuses.Empty();
}
NW_HOOK_METHOD(0x100d8950, NCAbnormalStatusWnd, ClearStatus)

NW_CHECK_VTABLE(NCAbnormalStatusWnd, 0x10127290, 67)
