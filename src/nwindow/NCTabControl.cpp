/*=============================================================================
    NCTabControl.cpp - tabs and their pages. Stock: 0x10022150 .. 0x10022b41.
=============================================================================*/
#include "nwindow/NCTabControl.h"
#include "nwindow/NCButton.h"
#include "nwindow/NCGDevice.h"
#include "nwindow/NConsoleWnd.h"

// interface sound of a tab change
enum { SOUND_Tab = 5 };

// 0x10027270 - GroupID is set by OnCreate
NCTabControl::NCTabControl()
{
    NextTabX = 0;
    CurrentPage = NULL;
}
NW_HOOK_CTOR(0x10027270, NCTabControl)

// 0x10027310 - Pages and Tabs are destroyed with the object (TArray<T*> destructor 0x10022980)
NW_HOOK_DTOR(0x10027310, NCTabControl)

// 0x10026960 (vslot 25) - the tab buttons report here
INT NCTabControl::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (NControl::WindowProc(Msg, wParam, lParam))
        return 1;
    if (Msg == NWM_COMMAND && (WPARAM)GroupID == wParam)
        return SelectTab((INT)lParam) != 0;
    return 0;
}
NW_HOOK_METHOD(0x10026960, NCTabControl, WindowProc)

// 0x100269c0 (vslot 36)
INT NCTabControl::OnCreate()
{
    NControl::OnCreate();
    GroupID = GNextTabGroupID++;
    return 1;
}
NW_HOOK_METHOD(0x100269c0, NCTabControl, OnCreate)

// 0x100270d0 (vslot 37) - the tab buttons are ours, the pages are not
INT NCTabControl::OnDestroy()
{
    guard(NCTabControl::OnDestroy);
    for (INT i = 0; i < Tabs.Num(); i++)
    {
        if (Tabs(i))
        {
            RemoveChildWindow(Tabs(i)->Entry);
            delete Tabs(i);
            Tabs(i) = NULL;
        }
        delete Tabs(i);  // NULL by now: the stock code deletes twice
    }
    Tabs.Empty();
    Pages.Empty();
    NControl::OnDestroy();
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x100270d0, NCTabControl, OnDestroy)

// 0x100269e0 (vslot 68)
void NCTabControl::ShowCurrentPage()
{
    for (INT i = 0; i < Pages.Num(); i++)
        if (Pages(i))
            Pages(i)->HideWindow();
    if (CurrentPage)
        CurrentPage->ShowWindow();
}
NW_HOOK_METHOD(0x100269e0, NCTabControl, ShowCurrentPage)

// 0x10026a30 (vslot 69) - the tab sound plays twice on a change, once otherwise (stock)
INT NCTabControl::SelectTab(INT Index)
{
    if (Tabs.Num() >= 0 && Index < Tabs.Num() && Tabs(Index)->IsEnabled())
    {
        if (CurrentPage != Pages(Index))
            PlayInterfaceSound(SOUND_Tab);
        if (CurrentPage)
            CurrentPage->HideWindow();
        CurrentPage = Pages(Index);
        if (CurrentPage)
        {
            CurrentPage->ShowWindow();
            CurrentPage->SetFocus();
        }
        for (INT i = 0; i < Tabs.Num(); i++)
            Tabs(i)->bReleased = 1;
        Tabs(Index)->bReleased = 0;
        NotifyParent(Index);
        PlayInterfaceSound(SOUND_Tab);
    }
    return 1;
}
NW_HOOK_METHOD(0x10026a30, NCTabControl, SelectTab)

// 0x10026b20 - SelectTab without sound nor focus; every page is hidden first
void NCTabControl::SetTab(INT Index)
{
    if (Tabs.Num() >= 0 && Index < Tabs.Num() && Tabs(Index)->IsEnabled())
    {
        for (INT i = 0; i < Pages.Num(); i++)
            Pages(i)->HideWindow();
        CurrentPage = Pages(Index);
        if (CurrentPage)
            CurrentPage->ShowWindow();
        for (INT i = 0; i < Tabs.Num(); i++)
            Tabs(i)->bReleased = 1;
        Tabs(Index)->bReleased = 0;
        NotifyParent(Index);
    }
}
NW_HOOK_METHOD(0x10026b20, NCTabControl, SetTab)

/*-----------------------------------------------------------------------------
    Adding tabs. The two stock overloads repeat the same code around the label;
    it is split here in the part before the label and the part after it.
-----------------------------------------------------------------------------*/

// the tab button, registered at the end of the row (name ours)
static __forceinline NCRadioButton* NewTabButton(NCTabControl* Ctrl, INT W, INT H,
                                                 const TCHAR* NormalTexName,
                                                 const TCHAR* PushedTexName)
{
    NCRadioButton* Tab;
    if (NormalTexName && PushedTexName)
        Tab = new NCRadioButton(NormalTexName, PushedTexName);
    else
        Tab = new NCRadioButton(TEXT("NWindow.BlackTexture"), TEXT("NWindow.Darkyellow"));
    Tab->ID = Ctrl->GroupID;
    Tab->SetWindowRect(Ctrl->NextTabX, 0, W, H);
    Ctrl->CreateChild(Tab, NWS_VISIBLE | NWS_UNKNOWN20);   // C2: 0x22
    Tab->Value = Ctrl->Tabs.Num();
    return Tab;
}

// the first tab is the current one; the others start hidden and released (name ours)
static __forceinline INT AddTabButton(NCTabControl* Ctrl, NCRadioButton* Tab, NCWnd* Page,
                                      INT W, INT Gap, INT ToolTipID)
{
    Tab->TextOffsetX = 2;
    Tab->TextOffsetY = 1;
    Tab->SetToolTip(ToolTipID, 1);
    if (Ctrl->Tabs.Num() == 0)
    {
        Ctrl->CurrentPage = Page;
        Tab->bReleased = 0;
    }
    else
    {
        Tab->bReleased = 1;
        Tab->HideWindow();
    }
    Ctrl->NextTabX += W + Gap;
    Ctrl->Tabs.AddItem(Tab);
    Ctrl->Pages.AddItem(Page);
    return Ctrl->Tabs.Num() - 1;
}

// 0x10026c80 - label from the SysString table
INT NCTabControl::AddTab(INT TextID, NCWnd* Page, INT W, INT H, const TCHAR* NormalTexName,
                         const TCHAR* PushedTexName, INT Gap, INT ToolTipID)
{
    if (!*SysString(TextID))
        return -1;
    if (Tabs.Num() == 0)
        CurrentPage = Page;
    if (!W || !H)
    {
        SIZE Size;
        NCGDevice::GetTextExtent(&Size, *SysString(TextID), 0, 0, 0, 0, 0, L2FONT_Normal);
        W = Size.cx + 10;
        H = Size.cy + 5;
    }
    NCRadioButton* Tab = NewTabButton(this, W, H, NormalTexName, PushedTexName);
    Tab->SetTextID(TextID);
    return AddTabButton(this, Tab, Page, W, Gap, ToolTipID);
}
NW_HOOK_METHOD_SIG(0x10026c80, NCTabControl, AddTab,
                   INT (NCTabControl::*)(INT, NCWnd*, INT, INT, const TCHAR*, const TCHAR*, INT, INT))

// 0x10026ec0
INT NCTabControl::AddTab(const TCHAR* Text, NCWnd* Page, INT W, INT H,
                         const TCHAR* NormalTexName, const TCHAR* PushedTexName, INT Gap,
                         INT ToolTipID)
{
    if (!Text)
        return -1;
    if (Tabs.Num() == 0)
        CurrentPage = Page;
    if (!W || !H)
    {
        SIZE Size;
        NCGDevice::GetTextExtent(&Size, Text, 0, 0, 0, 0, 0, L2FONT_Normal);
        W = Size.cx + 10;
        H = Size.cy + 5;
    }
    NCRadioButton* Tab = NewTabButton(this, W, H, NormalTexName, PushedTexName);
    Tab->SetText(Text);
    return AddTabButton(this, Tab, Page, W, Gap, ToolTipID);
}
NW_HOOK_METHOD_SIG(0x10026ec0, NCTabControl, AddTab,
                   INT (NCTabControl::*)(const TCHAR*, NCWnd*, INT, INT, const TCHAR*, const TCHAR*,
                                         INT, INT))


// 0x10027380 - a copy
TArray<NCRadioButton*> NCTabControl::GetTabs()
{
    return Tabs;
}
NW_HOOK_METHOD(0x10027380, NCTabControl, GetTabs)

NW_CHECK_VTABLE(NCTabControl, 0x1010e570, 70)
