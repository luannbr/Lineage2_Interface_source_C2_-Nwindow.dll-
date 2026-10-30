/*=============================================================================
    NCGameOptionWnd.cpp - the game page of the options. Stock: 0x1007dd70 .. 0x1007de64,
    0x10080970 .. 0x10080da6, 0x10080de0 .. 0x100817b1, 0x10082440 .. 0x100825a4,
    0x10082f10 .. 0x100837e9.
=============================================================================*/
#include "nwindow/NCGameOptionWnd.h"
#include "nwindow/NCComboBox.h"
#include "nwindow/NCConsole.h"
#include "nwindow/NCShortCutWnd.h"
#include "nwindow/NCPartyWnd.h"
#include "nwindow/NCButton.h"
#include "nwindow/NCCheckBox.h"
#include "nwindow/NCGDevice.h"
#include "nwindow/NConsoleWnd.h"
#include "nwindow/NKeyCrypt.h"

// 0x100a3280 - the controls are left as allocated, but for these
NCGameOptionWnd::NCGameOptionWnd()
{
    Unused9C[0] = NULL;
    Unused9C[1] = NULL;
    Unused9C[2] = NULL;
    GamePadBox = NULL;
}
NW_HOOK_CTOR(0x100a3280, NCGameOptionWnd)

// 0x100a32e0
NW_HOOK_DTOR(0x100a32e0, NCGameOptionWnd)

// 0x100a06a0 (vslot 25)
INT NCGameOptionWnd::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (NCWnd::WindowProc(Msg, wParam, lParam))
        return 1;
    if (Msg == NWM_COMMAND)
    {
        if (wParam == (WPARAM)NID_GAMEOPT_OTHERPC)
            return OnOtherPCClicked() != 0;
        if (wParam == (WPARAM)NID_GAMEOPT_RESET)
            return OnReset() != 0;
    }
    return 0;
}
NW_HOOK_METHOD(0x100a06a0, NCGameOptionWnd, WindowProc)

// one check box of the page, checked, its label a sysstring line (repeated inline in the stock
// OnCreate)
static FORCEINLINE NCCheckBox* CreateOptionBox(NCWnd* Parent, INT LabelID, INT ID, INT X, INT Y)
{
    NCCheckBox* Box = new NCCheckBox(*NCObject::SysString(LabelID), 1, 0);
    Box->ID = ID;
    Box->SetWindowRect(X, Y, 100, 12);
    Parent->CreateChild(Box, NWS_VISIBLE);
    return Box;
}

// 0x100a5c60 (vslot 36) - the name boxes are labelled Player, Monsters, Other PCs, Clan,
// Party, General (sysstring 436 .. 441); the others "Use" (443), their labels drawn by OnPaint
INT NCGameOptionWnd::OnCreate()
{
    NCWnd::OnCreate();
    LanguageCombo = new NCComboBox;
    LanguageCombo->ID = NID_GAMEOPT_LANGUAGE;
    LanguageCombo->SetWindowRect(83, 3, 112, 18);
    CreateChild(LanguageCombo, NWS_VISIBLE);
    NameBoxes[NAMEBOX_Player] = CreateOptionBox(this, 436, NID_GAMEOPT_PLAYER, 83, 26);
    NameBoxes[NAMEBOX_Monster] = CreateOptionBox(this, 437, NID_GAMEOPT_MONSTER, 83, 46);
    NameBoxes[NAMEBOX_OtherPC] = CreateOptionBox(this, 438, NID_GAMEOPT_OTHERPC, 83, 66);
    NameBoxes[NAMEBOX_Clan] = CreateOptionBox(this, 439, NID_GAMEOPT_CLAN, 96, 86);
    NameBoxes[NAMEBOX_Party] = CreateOptionBox(this, 440, NID_GAMEOPT_PARTY, 96, 106);
    NameBoxes[NAMEBOX_General] = CreateOptionBox(this, 441, NID_GAMEOPT_GENERAL, 96, 126);
    TrackingBox = CreateOptionBox(this, 443, NID_GAMEOPT_TRACKING, 83, 146);
    EnterChatBox = CreateOptionBox(this, 443, NID_GAMEOPT_ENTERCHAT, 83, 166);
    OldChatBox = CreateOptionBox(this, 443, NID_GAMEOPT_OLDCHAT, 190, 166);

    ResetButton = new NCButton(TEXT("L2UI.Control.DefaultButton"),
                               TEXT("L2UI.Control.DefaultButton_click"), 0);
    ResetButton->ID = NID_GAMEOPT_RESET;
    ResetButton->SetWindowRect(83, 186, 67, 19);
    CreateChild(ResetButton, NWS_VISIBLE);
    ResetButton->SetTextID(0x1DF);   // "Initialize"

    PartyLootCombo = new NCComboBox;
    PartyLootCombo->ID = NID_GAMEOPT_PARTYLOOT;
    PartyLootCombo->SetWindowRect(83, 213, 132, 18);
    CreateChild(PartyLootCombo, NWS_VISIBLE | 0x10);   // 0x10: meaning unknown
    KeyCryptBox = CreateOptionBox(this, 443, NID_GAMEOPT_KEYCRYPT, 83, 236);   // new in C2
    if (GL2CanUseJoystick)
        GamePadBox = CreateOptionBox(this, 443, NID_GAMEOPT_GAMEPAD, 83, 256);
    InitItems();
    return 1;
}
NW_HOOK_METHOD(0x100a5c60, NCGameOptionWnd, OnCreate)

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

// 0x100a3760 (vslot 37) - the combo boxes are enabled again first (no NULL check)
INT NCGameOptionWnd::OnDestroy()
{
    guard(NCGameOptionWnd::OnDestroy);
    for (INT i = 0; i < NAMEBOX_MAX; i++)
        DestroyChild(this, NameBoxes[i]);
    DestroyChild(this, LanguageCombo);
    DestroyChild(this, ResetButton);
    DestroyChild(this, Unused9C[0]);
    DestroyChild(this, Unused9C[1]);
    DestroyChild(this, Unused9C[2]);
    DestroyChild(this, EnterChatBox);
    DestroyChild(this, OldChatBox);
    DestroyChild(this, TrackingBox);
    DestroyChild(this, PartyLootCombo);
    if (GL2CanUseJoystick && GamePadBox)
        DestroyChild(this, GamePadBox);
    DestroyChild(this, KeyCryptBox);
    NCWnd::OnDestroy();
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x100a3760, NCGameOptionWnd, OnDestroy)

// 0x100a0740 (vslot 43) - the loot rule is fixed while the player's User +0x188 is set
// (meaning not known)
void NCGameOptionWnd::ShowWindow()
{
    NCWnd::ShowWindow();
    User* Me = GConsole->GetMyUser();
    if (Me && Me->Unknown188)
        PartyLootCombo->EnableWindow(0);
    else
        PartyLootCombo->EnableWindow(1);
}
NW_HOOK_METHOD(0x100a0740, NCGameOptionWnd, ShowWindow)

// a label right-aligned on x Right (repeated inline in the stock OnPaint)
static FORCEINLINE void DrawOptionLabel(NCGDevice* Device, INT TextID, INT Right, INT Y)
{
    SIZE Size;
    NCGDevice::GetTextExtent(&Size, *NCObject::SysString(TextID), 0, 0, 0, 0, 0);
    Device->DrawText(Right - Size.cx, Y, 0xFFFFFFFF, *NCObject::SysString(TextID), 0, 0, 0,
                     L2FONT_Normal, 0, 0, 0, 0, 0, 0);
}

// 0x100a3330 (vslot 55) - also keeps the loot box disabled while in a party (the party
// window's +0xA4, its members, not declared here). The guard names the options window.
INT NCGameOptionWnd::OnPaint(NCGDevice* Device)
{
    guard(NCOptionWnd::OnPaint);
    if (GDesktop->PartyWnd->NumMembers > 0)
        PartyLootCombo->EnableWindow(0);
    else
        PartyLootCombo->EnableWindow(1);
    Device->SetClip(ScreenX, ScreenY, Width, Height);
    Device->BeginPaint(ScreenX, ScreenY);
    DrawOptionLabel(Device, 329, 73, 5);     // Language
    DrawOptionLabel(Device, 435, 73, 26);    // Display
    DrawOptionLabel(Device, 463, 73, 146);   // Tracking
    DrawOptionLabel(Device, 566, 180, 166);  // Auto Code (the old chat box)
    DrawOptionLabel(Device, 442, 73, 166);   // Enter Chat
    DrawOptionLabel(Device, 478, 73, 190);   // Interface
    DrawOptionLabel(Device, 486, 73, 215);   // Party Loot
    DrawOptionLabel(Device, 926, 73, 236);   // the key encryption box (new in C2)
    if (GL2CanUseJoystick)
        DrawOptionLabel(Device, 606, 73, 256);   // Game Pad
    Device->EndPaint();
    Device->ReleaseClip();
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x100a3330, NCGameOptionWnd, OnPaint)

// 0x100a4360 (vslot 67) - "Other PCs" switches its three sub-boxes
INT NCGameOptionWnd::OnOtherPCClicked()
{
    if (NameBoxes[NAMEBOX_OtherPC]->bChecked)
    {
        NameBoxes[NAMEBOX_Clan]->bDisabled = 0;
        NameBoxes[NAMEBOX_Party]->bDisabled = 0;
        NameBoxes[NAMEBOX_General]->bDisabled = 0;
    }
    else
    {
        NameBoxes[NAMEBOX_Clan]->bDisabled = 1;
        NameBoxes[NAMEBOX_Party]->bDisabled = 1;
        NameBoxes[NAMEBOX_General]->bDisabled = 1;
    }
    return 1;
}
NW_HOOK_METHOD(0x100a4360, NCGameOptionWnd, OnOtherPCClicked)

// 0x100a0720 (vslot 68)
INT NCGameOptionWnd::OnReset()
{
    GConsole->ResetWindowPositions();
    return 1;
}
NW_HOOK_METHOD(0x100a0720, NCGameOptionWnd, OnReset)

// The nProtect key encryption driver's helper (NKeyCrypt, NConsoleWnd +0x359C): the stock calls the functions that
// never read the object with it in ecx, and the two npkcrypt.dll ones through its own linker thunks (GetLoadStartup
// 0x1003ed30, LoadAtStartup 0x1003eb80, one argument); 0x1003ec20 tells whether the process may install the driver
// (a member of the Administrators group)
static INT KeyCryptGetLoadStartup() { return NKeyCrypt::GetLoadStartup(); }
static void KeyCryptLoadAtStartup(INT bLoad) { NKeyCrypt::LoadAtStartup(bLoad); }
static INT KeyCryptIsAdmin() { return NKeyCrypt::IsAdministrator(); }

// 0x100a3ad0 - Option.ini [Game] into the globals and the boxes. "AutoTrackingPawn" is saved
// by Apply but not read here (the box shows GL2UseAutoTrackingPawn as it is).
void NCGameOptionWnd::LoadSettings()
{
    UBOOL bNative = 1;
    UBOOL bOtherPC = 0;
    if (!GConfig->GetBool(TEXT("Game"), TEXT("IsNative"), bNative, TEXT("Option.ini")))
        bNative = 1;
    if (!GConfig->GetBool(TEXT("Game"), TEXT("MyName"), GL2ShowMyName, TEXT("Option.ini")))
        GL2ShowMyName = 1;
    if (!GConfig->GetBool(TEXT("Game"), TEXT("NPCName"), GL2ShowNPCName, TEXT("Option.ini")))
        GL2ShowNPCName = 1;
    if (!GConfig->GetBool(TEXT("Game"), TEXT("GroupName"), bOtherPC, TEXT("Option.ini")))
        bOtherPC = 1;
    if (!GConfig->GetBool(TEXT("Game"), TEXT("PledgeMemberName"), GL2ShowPledgeMemberName,
                          TEXT("Option.ini")))
        GL2ShowPledgeMemberName = 1;
    if (!GConfig->GetBool(TEXT("Game"), TEXT("PartyMemberName"), GL2ShowPartyMemberName,
                          TEXT("Option.ini")))
        GL2ShowPartyMemberName = 1;
    if (!GConfig->GetBool(TEXT("Game"), TEXT("OtherPCName"), GL2ShowOtherPCName,
                          TEXT("Option.ini")))
        GL2ShowOtherPCName = 1;
    if (!GConfig->GetBool(TEXT("Game"), TEXT("EnterChatting"), GIsEnterChatting,
                          TEXT("Option.ini")))
        GIsEnterChatting = 0;
    if (!GConfig->GetBool(TEXT("Game"), TEXT("OldChatting"), GIsOldChat, TEXT("Option.ini")))
        GIsOldChat = 1;
    if (!GConfig->GetInt(TEXT("Game"), TEXT("PartyLooting"), GL2PartyLooting, TEXT("Option.ini")))
        GL2PartyLooting = 0;
    NameBoxes[NAMEBOX_OtherPC]->bChecked = bOtherPC;
    NameBoxes[NAMEBOX_Player]->bChecked = GL2ShowMyName;
    NameBoxes[NAMEBOX_Monster]->bChecked = GL2ShowNPCName;
    NameBoxes[NAMEBOX_Clan]->bChecked = GL2ShowPledgeMemberName;
    NameBoxes[NAMEBOX_Party]->bChecked = GL2ShowPartyMemberName;
    NameBoxes[NAMEBOX_General]->bChecked = GL2ShowOtherPCName;
    OnOtherPCClicked();
    if (!bOtherPC)
    {
        GL2ShowOtherPCName = 0;
        GL2ShowPartyMemberName = 0;
        GL2ShowPledgeMemberName = 0;
    }
    TrackingBox->bChecked = GL2UseAutoTrackingPawn;
    LanguageCombo->SetSelected(bNative ? 0 : 1);
    if (!GConfig->GetBool(TEXT("Game"), TEXT("UseJoystick"), GL2UsingJoystick, TEXT("Option.ini")))
        GL2UsingJoystick = 0;
    if (!GL2CanUseJoystick)
    {
        if (GamePadBox)
            GamePadBox->bChecked = 0;
        if (GamePadBox)
            GamePadBox->bDisabled = 1;
    }
    if (!GL2UsingJoystick)
    {
        if (GamePadBox)
            GamePadBox->bChecked = 0;
        GDesktop->ShortCutWnd->PadButton->HideWindow();
    }
    else
    {
        if (GamePadBox)
            GamePadBox->bChecked = 1;
        GDesktop->ShortCutWnd->PadButton->ShowWindow();
    }
    if (!GIsEnterChatting)
        EnterChatBox->bChecked = 0;
    else
        EnterChatBox->bChecked = 1;
    if (GL2UseKeyCrypt && KeyCryptGetLoadStartup())
        KeyCryptBox->bChecked = 1;
    else
        KeyCryptBox->bChecked = 0;
    if (GL2UseKeyCrypt && KeyCryptIsAdmin())
        KeyCryptBox->bDisabled = 0;
    else
        KeyCryptBox->bDisabled = 1;
}
NW_HOOK_METHOD(0x100a3ad0, NCGameOptionWnd, LoadSettings)

// 0x100a3ec0 - a language change reloads the game texts; the stock code reloads them a second
// time when the new language also differs from the saved one (reproduced)
void NCGameOptionWnd::Apply()
{
    INT Language = LanguageCombo->Selected;
    if (Language == GIsNative)
    {
        GIsNative = !GIsNative;
        GL2GameData.Load();
        GConsole->ReloadGameTexts();
    }
    UBOOL bSavedNative = 1;
    if (!GConfig->GetBool(TEXT("Game"), TEXT("IsNative"), bSavedNative, TEXT("Option.ini")))
        bSavedNative = 1;
    if (Language == 0)
        GIsNative = 1;
    else if (Language == 1)
        GIsNative = 0;
    if (bSavedNative != GIsNative)
    {
        GL2GameData.Load();
        GConsole->ReloadGameTexts();
    }
    GL2ShowMyName = NameBoxes[NAMEBOX_Player]->bChecked;
    GL2ShowNPCName = NameBoxes[NAMEBOX_Monster]->bChecked;
    if (NameBoxes[NAMEBOX_OtherPC]->bChecked)
    {
        GL2ShowPledgeMemberName = NameBoxes[NAMEBOX_Clan]->bChecked;
        GL2ShowPartyMemberName = NameBoxes[NAMEBOX_Party]->bChecked;
        GL2ShowOtherPCName = NameBoxes[NAMEBOX_General]->bChecked;
    }
    else
    {
        GL2ShowOtherPCName = 0;
        GL2ShowPartyMemberName = 0;
        GL2ShowPledgeMemberName = 0;
    }
    GL2UseAutoTrackingPawn = TrackingBox->bChecked;
    GIsEnterChatting = EnterChatBox->bChecked;
    GIsOldChat = OldChatBox->bChecked;
    GConfig->SetBool(TEXT("Game"), TEXT("IsNative"), GIsNative, TEXT("Option.ini"));
    GConfig->SetBool(TEXT("Game"), TEXT("MyName"), GL2ShowMyName, TEXT("Option.ini"));
    GConfig->SetBool(TEXT("Game"), TEXT("NPCName"), GL2ShowNPCName, TEXT("Option.ini"));
    GConfig->SetBool(TEXT("Game"), TEXT("GroupName"), NameBoxes[NAMEBOX_OtherPC]->bChecked,
                     TEXT("Option.ini"));
    GConfig->SetBool(TEXT("Game"), TEXT("PledgeMemberName"), NameBoxes[NAMEBOX_Clan]->bChecked,
                     TEXT("Option.ini"));
    GConfig->SetBool(TEXT("Game"), TEXT("PartyMemberName"), NameBoxes[NAMEBOX_Party]->bChecked,
                     TEXT("Option.ini"));
    GConfig->SetBool(TEXT("Game"), TEXT("OtherPCName"), NameBoxes[NAMEBOX_General]->bChecked,
                     TEXT("Option.ini"));
    GConfig->SetBool(TEXT("Game"), TEXT("AutoTrackingPawn"), GL2UseAutoTrackingPawn,
                     TEXT("Option.ini"));
    GConfig->SetBool(TEXT("Game"), TEXT("EnterChatting"), GIsEnterChatting, TEXT("Option.ini"));
    GConfig->SetBool(TEXT("Game"), TEXT("OldChatting"), GIsOldChat, TEXT("Option.ini"));
    GConsole->PartyItemDistribution = PartyLootCombo->Selected;
    GL2PartyLooting = PartyLootCombo->Selected;
    GConfig->SetInt(TEXT("Game"), TEXT("PartyLooting"), GL2PartyLooting, TEXT("Option.ini"));
    if (GL2CanUseJoystick)
    {
        if (!GamePadBox->bChecked)
        {
            GL2UsingJoystick = 0;
            GDesktop->ShortCutWnd->PadButton->HideWindow();
            NCShortCutWnd* Bar = GDesktop->ShortCutWnd;
            if (Bar->bPadMode == 1)
            {
                Bar->bPadMode = 0;
                GDesktop->ShortCutWnd->RotateButton->ShowWindow();
                GDesktop->ShortCutWnd->UpdateLayout();
            }
        }
        else
        {
            GL2UsingJoystick = 1;
            GDesktop->ShortCutWnd->PadButton->ShowWindow();
        }
        GConfig->SetBool(TEXT("Game"), TEXT("UseJoystick"), GL2UsingJoystick, TEXT("Option.ini"));
    }
    if (GL2UseKeyCrypt && KeyCryptBox->bChecked)
        KeyCryptLoadAtStartup(1);   // C2: the key encryption driver at startup
    else
        KeyCryptLoadAtStartup(0);
}
NW_HOOK_METHOD(0x100a3ec0, NCGameOptionWnd, Apply)

// 0x100a3a10 (C2 only; name ours) - before the page is filled again (NCOptionWnd::ShowWindow): the language and
// party loot lists emptied, the check boxes cleared (the game pad's only with a game pad, the key encryption box
// always)
void NCGameOptionWnd::ClearItems()
{
    for (INT i = 0; i < NAMEBOX_MAX; i++)
        NameBoxes[i]->bChecked = 0;
    LanguageCombo->ClearItems();
    EnterChatBox->bChecked = 0;
    OldChatBox->bChecked = 0;
    TrackingBox->bChecked = 0;
    PartyLootCombo->ClearItems();
    if (GL2CanUseJoystick)
        GamePadBox->bChecked = 0;
    KeyCryptBox->bChecked = 0;
}
NW_HOOK_METHOD(0x100a3a10, NCGameOptionWnd, ClearItems)

// 0x100a50b0 - the language box offers the client's own language (FontInfo.ini
// [LanguageSet] Language: 0 Korean .. 5 Thai) and English; an English client (1) only English,
// an unknown language nothing. Without GEnableEngSelection the box is disabled again (C2).
void NCGameOptionWnd::InitItems()
{
    INT ClientLanguage = 0;
    if (!GConfig->GetInt(TEXT("LanguageSet"), TEXT("Language"), ClientLanguage,
                         TEXT("FontInfo.ini")))
    {
        ClientLanguage = 0;
        LanguageCombo->AddItem(TEXT("Korean"));
        LanguageCombo->AddItem(TEXT("English"));
        if (!GEnableEngSelection)
            LanguageCombo->EnableWindow(0);
    }
    else if (ClientLanguage == 0)
    {
        LanguageCombo->AddItem(TEXT("Korean"));
        LanguageCombo->AddItem(TEXT("English"));
        if (!GEnableEngSelection)
            LanguageCombo->EnableWindow(0);
    }
    else if (ClientLanguage == 1)
        LanguageCombo->AddItem(TEXT("English"));
    else
    {
        const TCHAR* Own = NULL;
        if (ClientLanguage == 2)
            Own = TEXT("Japanese");
        else if (ClientLanguage == 3)
            Own = TEXT("Chinese(Taiwan)");
        else if (ClientLanguage == 4)
            Own = TEXT("China");
        else if (ClientLanguage == 5)
            Own = TEXT("Thai");
        if (Own)
        {
            LanguageCombo->AddItem(Own);
            LanguageCombo->AddItem(TEXT("English"));
            if (!GEnableEngSelection)
                LanguageCombo->EnableWindow(0);
        }
    }
    PartyLootCombo->AddItem(*SysString(487));   // Finders Keepers
    PartyLootCombo->AddItem(*SysString(488));   // Random
    PartyLootCombo->AddItem(*SysString(798));
    PartyLootCombo->AddItem(*SysString(799));
    PartyLootCombo->AddItem(*SysString(800));
    LoadSettings();
    if (EnterChatBox)
        EnterChatBox->bChecked = GIsEnterChatting != 0;
    if (OldChatBox)
        OldChatBox->bChecked = GIsOldChat != 0;
    PartyLootCombo->SetSelected(GL2PartyLooting);
    GConsole->PartyItemDistribution = PartyLootCombo->Selected;
}
NW_HOOK_METHOD(0x100a50b0, NCGameOptionWnd, InitItems)

NW_CHECK_VTABLE(NCGameOptionWnd, 0x1011efb8, 69)
