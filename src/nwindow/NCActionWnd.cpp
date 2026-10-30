/*=============================================================================
    NCActionWnd.cpp - the action window (Basic and Clan tabs). Stock: 0x100364c0 ..
    0x1003679e, 0x10036870 .. 0x1003687a, 0x10036e20 .. 0x10036e61, 0x10037250 ..
    0x1003735b, 0x1005f200 .. 0x1005f24f (the C1 addresses; C2: OnDestroy is 0x10042df0).
=============================================================================*/
#include "nwindow/NCActionWnd.h"
#include "nwindow/NCActionItemListWnd.h"
#include "nwindow/NCActionItemWnd.h"
#include "nwindow/NCPledgeWnd.h"
#include "nwindow/NCTabControl.h"
#include "nwindow/NCButton.h"
#include "nwindow/NCGDevice.h"
#include "nwindow/NConsoleWnd.h"

// 0x10042b10 - only BackTex is cleared
NCActionWnd::NCActionWnd()
{
    BackTex = NULL;
}
NW_HOOK_CTOR(0x10042b10, NCActionWnd)

// 0x10042b60
NW_HOOK_DTOR(0x10042b60, NCActionWnd)

// stock body shared with NControl's (0x10004e90)
INT NCActionWnd::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (Msg != WM_PAINT && !IsEnabled())
        return 0;
    return NCWnd::WindowProc(Msg, wParam, lParam) != 0;
}

// a page of the window (repeated inline in the stock OnCreate)
static FORCEINLINE void PlacePage(NCWnd* Parent, NCWnd* Page)
{
    Page->SetWindowRect(0, 16, 240, 284);
    Parent->CreateChild(Page, NWS_VISIBLE);
}

// 0x10042bb0 (vslot 36) - tabs "Basic" / "Clan" (sysstring 129 / 314)
INT NCActionWnd::OnCreate()
{
    NCWnd::OnCreate();
    BackTex = FindTexture(TEXT("sek.cbui304"));
    ActionList = new NCActionItemListWnd;
    PlacePage(this, ActionList);
    PledgeWnd = new NCPledgeWnd;
    PlacePage(this, PledgeWnd);
    PledgeWnd->HideWindow();
    TabCtrl = new NCTabControl;
    TabCtrl->ID = NID_ACTION_TABS;
    TabCtrl->SetWindowRect(15, 1, 138, 15);
    CreateChild(TabCtrl, NWS_VISIBLE);
    TabCtrl->AddTab(0x81, ActionList, 69, 16, TEXT("L2UI.NWindow.Tab2"), TEXT("L2UI.NWindow.Tab1"), 0, -1);
    TabCtrl->AddTab(0x13A, PledgeWnd, 69, 16, TEXT("L2UI.NWindow.Tab2"), TEXT("L2UI.NWindow.Tab1"), 0, -1);
    return 1;
}
NW_HOOK_METHOD(0x10042bb0, NCActionWnd, OnCreate)

// 0x10042df0 (vslot 37) - the four children go (each one out of the window's list, then deleted)
INT NCActionWnd::OnDestroy()
{
    guard(NCActionWnd::OnDestroy);
    if (ActionList)
    {
        RemoveChildWindow(ActionList->Entry);
        delete ActionList;
        ActionList = NULL;
    }
    if (PledgeWnd)
    {
        RemoveChildWindow(PledgeWnd->Entry);
        delete PledgeWnd;
        PledgeWnd = NULL;
    }
    if (TabCtrl)
    {
        RemoveChildWindow(TabCtrl->Entry);
        delete TabCtrl;
        TabCtrl = NULL;
    }
    if (UnusedA4)
    {
        RemoveChildWindow(UnusedA4->Entry);
        delete UnusedA4;
        UnusedA4 = NULL;
    }
    NCWnd::OnDestroy();
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x10042df0, NCActionWnd, OnDestroy)

// 0x10043eb0 (vslot 43) - with a clan, the member list is asked for first; the clan page and the grids are reset, the
// actions loaded again (C2)
void NCActionWnd::ShowWindow()
{
    NW_TRACE("NCActionWnd::ShowWindow pledge name [%ls] id=%d", PledgeWnd->GetPledgeName() ? PledgeWnd->GetPledgeName() : TEXT(""),
             PledgeWnd->Info.PledgeID);
    if (PledgeWnd->GetPledgeName())
        GConsole->RequestPledgeMemberList();
    ResetPledgeWnd();
    NCWnd::ShowWindow();
    UpdatePledgeTab();
    ActionList->LoadActions();
}
NW_HOOK_METHOD(0x10043eb0, NCActionWnd, ShowWindow)

// 0x10076ed0 (vslot 55) - draws nothing (BackTex is not used)
INT NCActionWnd::OnPaint(NCGDevice* Device)
{
    Device->SetClip(ScreenX, ScreenY, Width, Height);
    Device->BeginPaint(ScreenX, ScreenY);
    Device->EndPaint();
    Device->ReleaseClip();
    return 1;
}
NW_HOOK_METHOD(0x10076ed0, NCActionWnd, OnPaint)

// 0x10043d70 - the clan page and the grids emptied (C1: the clan page only)
void NCActionWnd::ResetPledgeWnd()
{
    PledgeWnd->Reset();
    ActionList->ClearGrids();
}
NW_HOOK_METHOD(0x10043d70, NCActionWnd, ResetPledgeWnd)

// 0x10043d90 - after a language change (NConsoleWnd::ReloadGameTexts)
void NCActionWnd::ReloadActions()
{
    ActionList->BasicGrid->ClearItems();
    ActionList->PartyGrid->ClearItems();
    ActionList->SocialGrid->ClearItems();
    ActionList->LoadActions();
}
NW_HOOK_METHOD(0x10043d90, NCActionWnd, ReloadActions)

// 0x10043b70 - no clan: the Clan tab is disabled and Basic selected. A hidden window hides
// again (NCWnd's, called directly), which also hides the page just shown.
__declspec(noinline) void NCActionWnd::UpdatePledgeTab()
{
    TArray<NCRadioButton*> Tabs = TabCtrl->GetTabs();
    if (Tabs.Num() > 0)
    {
        if (!PledgeWnd->GetPledgeName())
        {
            Tabs(1)->EnableWindow(0);
            TabCtrl->SetTab(0);
        }
        else
            Tabs(1)->EnableWindow(1);
        TabCtrl->ShowCurrentPage();
    }
    if (!IsShowWindow())
        NCWnd::HideWindow();
}
NW_HOOK_METHOD(0x10043b70, NCActionWnd, UpdatePledgeTab)

NW_CHECK_VTABLE(NCActionWnd, 0x10111e18, 67)
