/*=============================================================================
    NCSystemMenuWnd.cpp - the system menu. Stock: 0x10096da0 .. 0x10096fe5, 0x10097790 ..
    0x100980b6 (OnDestroy 0x10097dd0 is its own), plus ShowWindow 0x1008c9c0 (shared by six
    windows) and OnOptions 0x10082c50 (NCLoginWnd's slot 68 too).
=============================================================================*/
#include "nwindow/NCSystemMenuWnd.h"
#include "nwindow/NCConsole.h"
#include "nwindow/NCPetitionBeginWnd.h"
#include "nwindow/NCButton.h"
#include "nwindow/NCFrameCtrl.h"
#include "nwindow/NCIcon.h"
#include "nwindow/NCDialogBox.h"
#include "nwindow/NDialogInfo.h"
#include "nwindow/NCGDevice.h"
#include "nwindow/NConsoleWnd.h"

// 0x10097790 - Buttons[SYSMENU_Unused] is left as allocated
NCSystemMenuWnd::NCSystemMenuWnd()
{
    BackTex = NULL;
    Buttons[SYSMENU_Help] = NULL;
    Buttons[SYSMENU_Options] = NULL;
    Buttons[SYSMENU_Restart] = NULL;
    Buttons[SYSMENU_Exit] = NULL;
    ButtonA = NULL;
}
NW_HOOK_CTOR(0x10097790, NCSystemMenuWnd)

// 0x10097800
NW_HOOK_DTOR(0x10097800, NCSystemMenuWnd)

// 0x10096da0 (vslot 25) - C2 no longer refuses the messages of a disabled window here
INT NCSystemMenuWnd::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (NCWnd::WindowProc(Msg, wParam, lParam))
        return 1;
    if (Msg == NWM_COMMAND)
    {
        if (wParam == NID_SYSMENU_HELP)
            return OnHelp() != 0;
        if (wParam == NID_SYSMENU_OPTIONS)
            return OnOptions() != 0;
        if (wParam == NID_SYSMENU_RESTART)
            return OnRestart() != 0;
        if (wParam == NID_SYSMENU_EXIT)
            return OnExit() != 0;
        if (wParam == NID_SYSMENU_BUTTON_A)
            return OnHelpWindow() != 0;
        if (wParam == NID_SYSMENU_BUTTON_B)
            return OnHelpPage() != 0;
    }
    else if (Msg == (UINT)NWM_DIALOG_RESULT)
        return OnDialogResult(wParam, lParam) != 0;
    return 0;
}
NW_HOOK_METHOD(0x10096da0, NCSystemMenuWnd, WindowProc)

// one button of the menu (repeated inline in the stock OnCreate): C2 stacks all six in one column
static FORCEINLINE NCButton* CreateMenuButton(NCWnd* Parent, INT ID, INT Y)
{
    NCButton* Button = new NCButton(TEXT("L2UI.Control.DefaultButton"),
                                    TEXT("L2UI.Control.DefaultButton_click"), 0);
    Button->ID = ID;
    Button->SetWindowRect(86, Y, 67, 19);
    Parent->CreateChild(Button, NWS_VISIBLE);
    return Button;
}

// 0x10097850 (vslot 36) - C2 order: the unnamed button A (its text is a sysstring), Help, button B, Options,
// Restart, Exit
INT NCSystemMenuWnd::OnCreate()
{
    NCWnd::OnCreate();
    BackTex = FindTexture(TEXT("L2UI.etcWnd.systemBack"));
    FrameCtrl = new NCFrameCtrl(1);
    FrameID = NID_SYSMENU_FRAME;
    FrameCtrl->ID = NID_SYSMENU_FRAME;
    FrameCtrl->SetWindowRect(0, 0, 240, 21);
    CreateChild(FrameCtrl, NWS_VISIBLE);
    ButtonA = CreateMenuButton(this, NID_SYSMENU_BUTTON_A, 38);
    ButtonA->SetText(*GL2GameData.SysString[711]);
    Buttons[SYSMENU_Help] = CreateMenuButton(this, NID_SYSMENU_HELP, 61);
    Buttons[SYSMENU_Help]->SetTextID(0x91);
    ButtonB = CreateMenuButton(this, NID_SYSMENU_BUTTON_B, 84);
    ButtonB->SetTextID(470);
    Buttons[SYSMENU_Options] = CreateMenuButton(this, NID_SYSMENU_OPTIONS, 107);
    Buttons[SYSMENU_Options]->SetTextID(0x92);
    Buttons[SYSMENU_Restart] = CreateMenuButton(this, NID_SYSMENU_RESTART, 130);
    Buttons[SYSMENU_Restart]->SetTextID(0x93);
    Buttons[SYSMENU_Exit] = CreateMenuButton(this, NID_SYSMENU_EXIT, 153);
    Buttons[SYSMENU_Exit]->SetTextID(0x94);
    return 1;
}
NW_HOOK_METHOD(0x10097850, NCSystemMenuWnd, OnCreate)

// 0x10097dd0 (vslot 37) - C2: its own body (the restart menu's has five buttons, this one seven)
INT NCSystemMenuWnd::OnDestroy()
{
    for (INT i = 0; i < SYSMENU_MAX; i++)
    {
        if (Buttons[i])
        {
            RemoveChildWindow(Buttons[i]->Entry);
            delete Buttons[i];
            Buttons[i] = NULL;
        }
    }
    if (ButtonA)
    {
        RemoveChildWindow(ButtonA->Entry);
        delete ButtonA;
        ButtonA = NULL;
    }
    if (ButtonB)
    {
        RemoveChildWindow(ButtonB->Entry);
        delete ButtonB;
        ButtonB = NULL;
    }
    NCWnd::OnDestroy();
    return 1;
}
NW_HOOK_METHOD(0x10097dd0, NCSystemMenuWnd, OnDestroy)

// 0x1008c9c0 (vslot 43) - body shared with NCBoardWnd, NCMatchWnd and three other windows:
// restored from its icon, the window comes back where the icon was
void NCSystemMenuWnd::ShowWindow()
{
    NCWnd::ShowWindow();
    if (Icon)
    {
        SetWindowRect(Icon->ScreenX, Icon->ScreenY, Width, Height);
        Icon->SetWindowRect(0, 0, 0, 0);
        Icon->HideWindow();
    }
}
NW_HOOK_METHOD(0x1008c9c0, NCSystemMenuWnd, ShowWindow)

// 0x10097d40 (vslot 55) - the background is 171 pixels high in C2 (C1: 101)
INT NCSystemMenuWnd::OnPaint(NCGDevice* Device)
{
    Device->SetClip(ScreenX, ScreenY, Width, Height);
    Device->BeginPaint(ScreenX, ScreenY);
    if (BackTex)
        Device->DrawTexture(0, 21, 240, 171, 0, 0, 240, 171, BackTex);
    Device->EndPaint();
    Device->ReleaseClip();
    return 1;
}
NW_HOOK_METHOD(0x10097d40, NCSystemMenuWnd, OnPaint)

// 0x10096ec0 (vslot 67) - the Help button: the desktop's help window with argument 0 (NCConsole
// 0x1004BD80, which shows the macro help page; its C1 equivalent took no argument)
INT NCSystemMenuWnd::OnHelp()
{
    GDesktop->ToggleHelp(0);   // 0x1004BD80
    return 1;
}
NW_HOOK_METHOD(0x10096ec0, NCSystemMenuWnd, OnHelp)

// 0x10096ee0 (vslot 70) - the C1 OnHelp slot: in C2 the desktop's 0x1004C100 toggles the macro window (NCConsole::ToggleMacroList;
// NCConsole::ToggleHelp, 0x1004BD80, is the other help window's toggle)
INT NCSystemMenuWnd::OnHelpWindow()
{
    static_cast<NCConsole*>(GConsole->Desktop)->ToggleMacroList();
    return 1;
}
NW_HOOK_METHOD(0x10096ee0, NCSystemMenuWnd, OnHelpWindow)

// 0x10096fb0 (vslot 72, new in C2) - the petition form (NCPetitionBeginWnd, the desktop's +0xBC) starts over and shows
INT NCSystemMenuWnd::OnHelpPage()
{
    ((NCPetitionBeginWnd*)GDesktop->PetitionBeginWnd)->ResetForm();   // 0x100B6B40
    GDesktop->PetitionBeginWnd->ShowWindow();
    return 1;
}
NW_HOOK_METHOD(0x10096fb0, NCSystemMenuWnd, OnHelpPage)

// body shared with NCLoginWnd's (0x10082c50): "Options" opens or closes the option window
INT NCSystemMenuWnd::OnOptions()
{
    GConsole->ToggleOptionWnd();
    return 1;
}
NW_HOOK_METHOD(0x10082c50, NCSystemMenuWnd, OnOptions)

// the confirmation of Restart / Exit (repeated in both slots)
static FORCEINLINE void AskSystemMenu(NCWnd* Menu, INT DialogID, INT MsgID)
{
    NDialogInfo Info;
    Info.ID = DialogID;
    Info.bMainBox = 1;
    Info.bInput = 0;
    Info.Owner = Menu->Entry;
    Info.SetSystemMessage(MsgID);
    GConsole->ShowDialog(&Info);
}

// 0x10097f80 (vslot 69)
INT NCSystemMenuWnd::OnRestart()
{
    AskSystemMenu(this, DIALOG_SYSMENU_RESTART, 126);
    return 1;
}
NW_HOOK_METHOD(0x10097f80, NCSystemMenuWnd, OnRestart)

// 0x10098020 (vslot 71)
INT NCSystemMenuWnd::OnExit()
{
    AskSystemMenu(this, DIALOG_SYSMENU_EXIT, 125);
    return 1;
}
NW_HOOK_METHOD(0x10098020, NCSystemMenuWnd, OnExit)

// 0x10096f00 (vslot 73) - the answer of the confirmation. C2: Exit Game quits at once when the
// client runs only the L2 console (GIsOnlyL2Console), else it asks the console to log out.
INT NCSystemMenuWnd::OnDialogResult(WPARAM DialogID, LPARAM Unused)
{
    switch (DialogID)
    {
    case DIALOG_SYSMENU_RESTART:
    {
        L2ParamStack Params(10);
        GConsole->RequestRestart(Params);
        HideWindow();
        break;
    }
    case DIALOG_SYSMENU_EXIT:
        if (GIsOnlyL2Console)
            appRequestExit(0);
        else
            GConsole->RequestLogout();
        break;
    }
    return 1;
}
NW_HOOK_METHOD(0x10096f00, NCSystemMenuWnd, OnDialogResult)

NW_CHECK_VTABLE(NCSystemMenuWnd, 0x1011c798, 74)
