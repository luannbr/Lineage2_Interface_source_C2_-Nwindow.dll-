/*=============================================================================
    NCAuthWnd.cpp - the login box. Stock: 0x10082810 .. 0x10082a8f (0x100828b0 included),
    0x10083030 .. 0x10083893 (NCLoginWnd's functions sit in between). OnCancel and
    OnPasswordNotify are bodies the linker folded with other classes' (0x1004b820, 0x10088c40).
=============================================================================*/
#include "nwindow/NCAuthWnd.h"
#include "nwindow/NCButton.h"
#include "nwindow/NCEditBox.h"
#include "nwindow/NCFrameCtrl.h"
#include "nwindow/NCGDevice.h"
#include "nwindow/NCIME.h"
#include "nwindow/NCIMEManager.h"
#include "nwindow/NConsoleWnd.h"

// NWM_COMMAND ids of the box's controls, timer id (names ours)
enum
{
    NID_AUTH_OK = 0x1006,
    NID_AUTH_CANCEL = 0x1007,
    NID_AUTH_ACCOUNT = 0x1502,
    NID_AUTH_PASSWORD = 0x1503,
    NID_AUTH_FRAME = (INT)0xAFF12010,
    TIMER_AUTH_RETRY = (INT)0xAFF12013,   // 1 s (C1: 3 s): the next Log In may ask again
};

// systemmsg lines (names ours)
enum
{
    MSG_EnterAccount = 86,     // "Please insert your ID."
    MSG_EnterPassword = 87,    // "Please insert your password."
    MSG_CannotConnect = 90,    // "Unable to connect to server."
    MSG_EnterCardKey = 1236,   // (C2) the card key line is empty
    MSG_CardKeyPrompt = 1237,  // (C2) the prompt of the card key: its argument is the position asked
};

// sysstring lines (names ours): the labels of the two lines
enum
{
    STR_Account = 150,         // "ID"
    STR_Password = 151,        // "PWD"
};

// interface sounds (NCObject::PlayInterfaceSound)
enum { SOUND_Click = 0 };

// NCEditBox's constructor has five arguments in C2 (0x1000d5d0), the last one a new flag
static NCEditBox* NewEditBox(INT Type, UBOOL bIMEPosFixed, INT ExtraWidth, INT bNewFlag)
{
    return new NCEditBox(Type, NULL, bIMEPosFixed, ExtraWidth, bNewFlag);
}

// 0x10083030 - the timer result is left as allocated
NCAuthWnd::NCAuthWnd()
{
    Unknown94 = 0;
    BackTex = NULL;
    AccountEdit = NULL;
    PasswordEdit = NULL;
    OKButton = NULL;
    CancelButton = NULL;
    bRequested = 0;
    Mode = 0;
    CardPos = 0;
    CardID = 0;
}
NW_HOOK_CTOR(0x10083030, NCAuthWnd)

// 0x100830b0
NW_HOOK_DTOR(0x100830b0, NCAuthWnd)

// 0x10082810 (vslot 25) - C2 no longer refuses the messages of a disabled window here
INT NCAuthWnd::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (NCWnd::WindowProc(Msg, wParam, lParam))
        return 1;
    if (Msg == NWM_COMMAND)
    {
        if (wParam == NID_AUTH_OK)
            return OnOK() != 0;
        if (wParam == NID_AUTH_CANCEL)
            return OnCancel() != 0;
        if (wParam == NID_AUTH_PASSWORD)
            return OnPasswordNotify((INT)lParam) != 0;
    }
    return 0;
}
NW_HOOK_METHOD(0x10082810, NCAuthWnd, WindowProc)

// 0x10082a70 (vslot 35)
INT NCAuthWnd::OnTimer(INT TimerID)
{
    if (TimerID == TIMER_AUTH_RETRY)
        bRequested = 0;
    return 1;
}
NW_HOOK_METHOD(0x10082a70, NCAuthWnd, OnTimer)

// 0x10083100 (vslot 36) - TAB cycles account, password, Log In, Exit and back
INT NCAuthWnd::OnCreate()
{
    NCWnd::OnCreate();

    FrameCtrl = new NCFrameCtrl(1);
    FrameID = NID_AUTH_FRAME;
    FrameCtrl->ID = NID_AUTH_FRAME;
    FrameCtrl->SetWindowRect(0, 0, 214, 21);
    CreateChild(FrameCtrl, NWS_VISIBLE);

    BackTex = FindTexture(TEXT("L2UI.LobbyWnd.loginBack"));
    TimerResult = SetTimer(TIMER_AUTH_RETRY, 1000, NULL);

    AccountEdit = NewEditBox(0, 1, 128, 1);
    AccountEdit->ID = NID_AUTH_ACCOUNT;
    AccountEdit->SetWindowRect(87, 48, 82, 16);
    CreateChild(AccountEdit, NWS_VISIBLE);

    PasswordEdit = NewEditBox(2, 1, 128, 0);
    PasswordEdit->ID = NID_AUTH_PASSWORD;
    PasswordEdit->SetWindowRect(87, 76, 82, 16);
    CreateChild(PasswordEdit, NWS_VISIBLE);

    // Korean clients: both lines start with the IME conversion status 0 (C2)
    if (GLanguageType == 0 && AccountEdit->IMEManager
        && nw::Field<NCIME*>(AccountEdit->IMEManager, 0x1A4)
        && PasswordEdit->IMEManager
        && nw::Field<NCIME*>(PasswordEdit->IMEManager, 0x1A4))
    {
        ImmSetConversionStatus(nw::Field<NCIME*>(AccountEdit->IMEManager, 0x1A4)->hIMC, 0, 0);
        ImmSetConversionStatus(nw::Field<NCIME*>(PasswordEdit->IMEManager, 0x1A4)->hIMC, 0, 0);
    }

    OKButton = new NCButton(TEXT("L2UI.control.DefaultButton"),
                            TEXT("L2UI.control.DefaultButton_click"), 0);
    OKButton->ID = NID_AUTH_OK;
    OKButton->SetWindowRect(33, 112, 64, 19);
    CreateChild(OKButton, NWS_VISIBLE);
    OKButton->SetTextID(152);        // "Log In"

    CancelButton = new NCButton(TEXT("L2UI.control.DefaultButton"),
                                TEXT("L2UI.control.DefaultButton_click"), 0);
    CancelButton->ID = NID_AUTH_CANCEL;
    CancelButton->SetWindowRect(118, 112, 64, 19);
    CreateChild(CancelButton, NWS_VISIBLE);
    CancelButton->SetTextID(153);    // "Exit"

    SetTabOrder(AccountEdit->Entry, CancelButton->Entry);
    AccountEdit->SetTabOrder(PasswordEdit->Entry, Entry);
    PasswordEdit->SetTabOrder(OKButton->Entry, AccountEdit->Entry);
    OKButton->SetTabOrder(CancelButton->Entry, PasswordEdit->Entry);
    CancelButton->SetTabOrder(Entry, OKButton->Entry);
    return 1;
}
NW_HOOK_METHOD(0x10083100, NCAuthWnd, OnCreate)

// 0x10083790 (vslot 37)
INT NCAuthWnd::OnDestroy()
{
    if (AccountEdit)
    {
        RemoveChildWindow(AccountEdit->Entry);
        delete AccountEdit;
        AccountEdit = NULL;
    }
    if (PasswordEdit)
    {
        RemoveChildWindow(PasswordEdit->Entry);
        delete PasswordEdit;
        PasswordEdit = NULL;
    }
    if (OKButton)
    {
        RemoveChildWindow(OKButton->Entry);
        delete OKButton;
        OKButton = NULL;
    }
    if (CancelButton)
    {
        RemoveChildWindow(CancelButton->Entry);
        delete CancelButton;
        CancelButton = NULL;
    }
    NCWnd::OnDestroy();
    return 1;
}
NW_HOOK_METHOD(0x10083790, NCAuthWnd, OnDestroy)

// 0x10082a30 (vslot 43) - a new try: the account line (the card key line in Mode 1) gets the focus
void NCAuthWnd::ShowWindow()
{
    NCWnd::ShowWindow();
    if (Mode == 0)
        AccountEdit->SetFocus();
    else
    {
        AccountEdit->HideWindow();
        PasswordEdit->ClearText();
        PasswordEdit->SetFocus();
    }
}
NW_HOOK_METHOD(0x10082a30, NCAuthWnd, ShowWindow)

// 0x10083580 (vslot 55) - under the 21-pixel title bar; the clip goes on before BeginPaint. The labels
// are right-aligned at x = 82 (C1: left-aligned at 42); Mode 1 draws the card key prompt instead
INT NCAuthWnd::OnPaint(NCGDevice* Device)
{
    guard(NCAuthWnd::OnPaint);
    TCHAR Prompt[516];
    SIZE Size;
    Device->SetClip(ScreenX, ScreenY, Width, Height);
    Device->BeginPaint(ScreenX, ScreenY);
    if (BackTex)
        Device->DrawTexture(0, 21, 214, 127, 0, 0, 214, 127, BackTex);
    if (Mode == 0)
    {
        if (*GL2GameData.SysString[STR_Account])
        {
            NCGDevice::GetTextExtent(&Size, *GL2GameData.SysString[STR_Account], 0, 0, 0, 0, 0, L2FONT_Normal);
            Device->DrawText(82 - Size.cx, 50, 0xFFFFFFFF, *GL2GameData.SysString[STR_Account], 0, 0, 0,
                             L2FONT_Normal, 0, 0, 0, 0, 0, 0, 0);
        }
        if (*GL2GameData.SysString[STR_Password])
        {
            NCGDevice::GetTextExtent(&Size, *GL2GameData.SysString[STR_Password], 0, 0, 0, 0, 0, L2FONT_Normal);
            Device->DrawText(82 - Size.cx, 78, 0xFFFFFFFF, *GL2GameData.SysString[STR_Password], 0, 0, 0,
                             L2FONT_Normal, 0, 0, 0, 0, 0, 0, 0);
        }
    }
    else if (Mode == 1)
    {
        GConsole->GetNumberMessage((INT)MSG_CardKeyPrompt, CardPos, Prompt);
        Device->DrawText(20, 40, 0xFFFFFFFF, Prompt, 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0, 0);
    }
    Device->EndPaint();
    Device->ReleaseClip();
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x10083580, NCAuthWnd, OnPaint)

// 0x100828c0 (vslot 67) - Log In (Mode 1: the card key goes to RequestAuthCardKeyLogin as a number)
INT NCAuthWnd::OnOK()
{
    if (!Mode)
    {
        if (!AccountEdit->CommitText())
            GConsole->ShowMessageWnd(*GL2GameData.SystemMsg[MSG_EnterAccount].Message);
        else if (AccountEdit->CommitText() && !PasswordEdit->CommitText())
            GConsole->ShowMessageWnd(*GL2GameData.SystemMsg[MSG_EnterPassword].Message);
        else if (!bRequested)
        {
            if (!GConsole->RequestAuthLogin(AccountEdit->CommitText(), PasswordEdit->CommitText()))
                GConsole->ShowMessageWnd(*GL2GameData.SystemMsg[MSG_CannotConnect].Message);
            bRequested = 1;
        }
    }
    else if (!PasswordEdit->CommitText())
        GConsole->ShowMessageWnd(*GL2GameData.SystemMsg[MSG_EnterCardKey].Message);
    else if (!bRequested)
    {
        if (!GConsole->RequestAuthCardKeyLogin(CardID, appAtoi(PasswordEdit->CommitText())))
            GConsole->ShowMessageWnd(*GL2GameData.SystemMsg[MSG_CannotConnect].Message);
        bRequested = 1;
    }
    PlayInterfaceSound(SOUND_Click);
    return 1;
}
NW_HOOK_METHOD(0x100828c0, NCAuthWnd, OnOK)

// 0x1004b820 (vslot 68) - Exit. The stock body is shared by every Exit slot (NCConsole::OnQuit, NCAuthWnd, ...): it is hooked
// once, as NCConsole::OnQuit (console group), so there is no hook here
INT NCAuthWnd::OnCancel()
{
    appRequestExit(0);
    return 1;
}

// 0x10088c40 (vslot 69) - Enter in the password line (code 1) logs in (folded with other windows')
INT NCAuthWnd::OnPasswordNotify(INT Code)
{
    if (Code == 1)
        OnOK();
    return 1;
}
NW_HOOK_METHOD(0x10088c40, NCAuthWnd, OnPasswordNotify)

// 0x100828b0 - after a failed login (NCLoginWnd::ResetWindows)
void NCAuthWnd::ClearPassword()
{
    PasswordEdit->ClearText();
}
NW_HOOK_METHOD(0x100828b0, NCAuthWnd, ClearPassword)

// 0x10057cd0 - the box starts over with the account and password lines (the security card check ended or was not
// asked): Mode, CardPos and CardID back to 0, the account line shown and focused
void NCAuthWnd::ResetInput()
{
    Mode = 0;
    CardID = 0;
    CardPos = 0;
    AccountEdit->ShowWindow();
    AccountEdit->SetFocus();
}
NW_HOOK_METHOD(0x10057cd0, NCAuthWnd, ResetInput)

// 0x10057c80 - the server asks for the security card key (NConsoleWnd::SCCheckStart): the box goes to Mode 1 with the
// card's id and the position it asks for, the account line is hidden and the password line cleared and focused
void NCAuthWnd::SetCardKey(INT InCardID, INT InCardPos)
{
    CardPos = InCardPos;
    CardID = InCardID;
    Mode = 1;
    AccountEdit->HideWindow();
    PasswordEdit->ClearText();
    PasswordEdit->SetFocus();
}
NW_HOOK_METHOD(0x10057c80, NCAuthWnd, SetCardKey)

NW_CHECK_VTABLE(NCAuthWnd, 0x101196e8, 70)
