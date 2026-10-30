/*=============================================================================
    NCMenuWnd.cpp - the main menu bar. Stock: 0x10076db0 .. 0x10076ef5, 0x10077ab0 ..
    0x10077f67, plus OnDestroy 0x100367a0 (the body NCActionWnd and NCServerSelectWnd share).
=============================================================================*/
#include "nwindow/NCMenuWnd.h"
#include "nwindow/NCConsole.h"
#include "nwindow/NCButton.h"
#include "nwindow/NCFrameCtrl.h"
#include "nwindow/NCGDevice.h"
#include "nwindow/NConsoleWnd.h"

// 0x100980c0
NCMenuWnd::NCMenuWnd()
{
    BackTex = NULL;
    SystemButton = NULL;
    InventoryButton = NULL;
    CharacterButton = NULL;
    CommunityButton = NULL;
}
NW_HOOK_CTOR(0x100980c0, NCMenuWnd)

// 0x10098130
NW_HOOK_DTOR(0x10098130, NCMenuWnd)

// 0x10096ff0 (vslot 25)
INT NCMenuWnd::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (NCWnd::WindowProc(Msg, wParam, lParam))
        return 1;
    if (Msg == NWM_COMMAND)
    {
        if (wParam == NID_MENU_INVENTORY)
            return OnInventory() != 0;
        if (wParam == NID_MENU_CHARACTER)
            return OnCharacter() != 0;
        if (wParam == NID_MENU_SYSTEM)
            return OnSystem() != 0;
        if (wParam == NID_MENU_COMMUNITY)
            return OnCommunity() != 0;
    }
    return 0;
}
NW_HOOK_METHOD(0x10096ff0, NCMenuWnd, WindowProc)

// one icon of the bar (repeated inline in the stock OnCreate)
static FORCEINLINE NCButton* CreateMenuIcon(NCWnd* Parent, const TCHAR* NormalTexName,
                                const TCHAR* PushedTexName, INT ID, INT X, INT ToolTipID)
{
    NCButton* Button = new NCButton(NormalTexName, PushedTexName, 0);
    Button->ID = ID;
    Button->SetWindowRect(X, 5, 34, 34);
    Parent->CreateChild(Button, NWS_VISIBLE | 0x20);   // 0x20: meaning unknown
    Button->SetToolTip(ToolTipID, 1);
    return Button;
}

// 0x10098200 (vslot 36) - NCWnd::OnCreate is not called
INT NCMenuWnd::OnCreate()
{
    BackTex = FindTexture(TEXT("L2UI.EtcWndBack.SysMenuBack"));
    FrameCtrl = new NCFrameCtrl(0);
    FrameID = NID_MENU_FRAME;
    FrameCtrl->ID = NID_MENU_FRAME;
    FrameCtrl->SetWindowRect(0, 0, 11, 44);
    CreateChild(FrameCtrl, NWS_VISIBLE);
    CharacterButton = CreateMenuIcon(this, TEXT("L2UI.WindowIcon.MainCharacter_large"),
                                     TEXT("L2UI.WindowIcon.MainCharacter_large_click"),
                                     NID_MENU_CHARACTER, 16, 0x1B2);
    InventoryButton = CreateMenuIcon(this, TEXT("L2UI.WindowIcon.MainInventory_large"),
                                     TEXT("L2UI.WindowIcon.MainInventory_large_click"),
                                     NID_MENU_INVENTORY, 53, 0xC3);
    CommunityButton = CreateMenuIcon(this, TEXT("L2UI.WindowIcon.MainCommunity_large"),
                                     TEXT("L2UI.WindowIcon.MainCommunity_large_click"),
                                     NID_MENU_COMMUNITY, 90, 0x186);
    SystemButton = CreateMenuIcon(this, TEXT("L2UI.WindowIcon.MainSystem_large"),
                                  TEXT("L2UI.WindowIcon.MainSystem_large_click"),
                                  NID_MENU_SYSTEM, 127, 0xC1);
    return 1;
}
NW_HOOK_METHOD(0x10098200, NCMenuWnd, OnCreate)

// 0x10098580 (vslot 37) - C2: its own copy of the body, with a guard (C1 shared one with NCActionWnd and
// NCServerSelectWnd); the four buttons go in the order of their fields
INT NCMenuWnd::OnDestroy()
{
    guard(NCMenuWnd::OnDestroy);
    if (SystemButton)
    {
        RemoveChildWindow(SystemButton->Entry);
        delete SystemButton;
        SystemButton = NULL;
    }
    if (InventoryButton)
    {
        RemoveChildWindow(InventoryButton->Entry);
        delete InventoryButton;
        InventoryButton = NULL;
    }
    if (CharacterButton)
    {
        RemoveChildWindow(CharacterButton->Entry);
        delete CharacterButton;
        CharacterButton = NULL;
    }
    if (CommunityButton)
    {
        RemoveChildWindow(CommunityButton->Entry);
        delete CommunityButton;
        CommunityButton = NULL;
    }
    NCWnd::OnDestroy();
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x10098580, NCMenuWnd, OnDestroy)

// 0x10098180 (vslot 55) - the clip is set before the origin, as in the stock code
INT NCMenuWnd::OnPaint(NCGDevice* Device)
{
    Device->SetClip(ScreenX, ScreenY, Width, Height);
    Device->BeginPaint(ScreenX, ScreenY);
    if (BackTex)
        Device->DrawTexture(0, 0, Width, Height, 0, 0, Width, Height, BackTex);
    Device->EndPaint();
    Device->ReleaseClip();
    return 1;
}
NW_HOOK_METHOD(0x10098180, NCMenuWnd, OnPaint)

// 0x100970a0 (vslot 67)
INT NCMenuWnd::OnSystem()
{
    GDesktop->ToggleSystemMenu();
    return 1;
}
NW_HOOK_METHOD(0x100970a0, NCMenuWnd, OnSystem)

// 0x100970c0 (vslot 68)
INT NCMenuWnd::OnCharacter()
{
    GDesktop->ToggleCharacter(-1);
    return 1;
}
NW_HOOK_METHOD(0x100970c0, NCMenuWnd, OnCharacter)

// 0x100970e0 (vslot 69)
INT NCMenuWnd::OnInventory()
{
    GDesktop->ToggleInventory();
    return 1;
}
NW_HOOK_METHOD(0x100970e0, NCMenuWnd, OnInventory)

// 0x10097100 (vslot 70)
INT NCMenuWnd::OnCommunity()
{
    GDesktop->ToggleBoard();
    return 1;
}
NW_HOOK_METHOD(0x10097100, NCMenuWnd, OnCommunity)

NW_CHECK_VTABLE(NCMenuWnd, 0x1011c928, 71)
