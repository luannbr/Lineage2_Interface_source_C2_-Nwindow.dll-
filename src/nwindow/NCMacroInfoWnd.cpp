/*=============================================================================
    NCMacroInfoWnd.cpp - the macro's additional information window (short name and description).
    Stock: 0x1008B940 .. 0x1008BC56 (WindowProc, the two setters, ShowWindow, HideWindow, OnOK),
    0x1008E540 .. 0x1008EBDD (constructor, destructor, OnCreate, OnDestroy, OnPaint).
    OnCancel (0x100A0830) is the body NCOptionWnd::OnCancel hooks.
=============================================================================*/
#include "nwindow/NCMacroInfoWnd.h"
#include "nwindow/NCMacroListWnd.h"
#include "nwindow/NCButton.h"
#include "nwindow/NCConsole.h"
#include "nwindow/NCEditBox.h"
#include "nwindow/NCFrameCtrl.h"
#include "nwindow/NCGDevice.h"
#include "nwindow/NCMultiEdit.h"
#include "nwindow/NConsoleWnd.h"

enum
{
    MSG_MacroDescTooLong = 0x345,      // systemmsg (name ours): the description is longer than 32 characters
    COLOR_SystemMsg = 0xFFB09B79,
};

// sysstring labels (SysString index = (stock offset - 0x4B4E8) / 12; names ours)
enum
{
    STR_MacroDescription = 57,        // 0x4B794: drawn at (17, 60), above the description box
    STR_MacroShortName = 653,         // 0x4D384: drawn at (17, 37)
    STR_OK = 140,
    STR_Cancel = 141,
};

// the desktop's macro list window (NCConsole.h keeps it as an NCWnd* at +0xF8; name ours)
static NCMacroListWnd* DesktopList()
{
    return nw::Field<NCMacroListWnd*>(GConsole->Desktop, 0xF8);
}

// a child window goes (repeated inline in the stock OnDestroys)
template <class T> static FORCEINLINE void DestroyChild(NCWnd* Parent, T*& Child)
{
    if (Child)
    {
        Parent->RemoveChildWindow(Child->Entry);
        delete Child;
        Child = NULL;
    }
}

// 0x1008e540
NCMacroInfoWnd::NCMacroInfoWnd()
{
}
NW_HOOK_CTOR(0x1008e540, NCMacroInfoWnd)

// 0x1008e590
NW_HOOK_DTOR(0x1008e590, NCMacroInfoWnd)

// 0x1008b940 (vslot 25) - the two buttons
INT NCMacroInfoWnd::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (NCWnd::WindowProc(Msg, wParam, lParam))
        return 1;
    if (Msg == NWM_COMMAND)
    {
        if (wParam == NID_MACROINFO_OK)
            return OnOK() != 0;
        if (wParam == NID_MACROINFO_CANCEL)
            return OnCancel() != 0;
    }
    return 0;
}
NW_HOOK_METHOD(0x1008b940, NCMacroInfoWnd, WindowProc)

// 0x1008b9c0 - the short name box emptied, then Text (when given) typed into it
void NCMacroInfoWnd::SetShortName(const TCHAR* Text)
{
    ShortNameEdit->ClearText();
    if (Text != NULL && ShortNameEdit != NULL)
        ShortNameEdit->InsertText(Text, 1);
}
NW_HOOK_METHOD(0x1008b9c0, NCMacroInfoWnd, SetShortName)

// 0x1008b9f0 - the description emptied, then Text (when given) set
void NCMacroInfoWnd::SetDescription(const TCHAR* Text)
{
    DescEdit->Clear();
    if (Text != NULL && DescEdit != NULL)
        DescEdit->SetText(Text);
}
NW_HOOK_METHOD(0x1008b9f0, NCMacroInfoWnd, SetDescription)

// 0x1008ba20 (vslot 43) - shows the short name (4 characters at most) and the description (32) of the macro being edited
void NCMacroInfoWnd::ShowWindow()
{
    if (GConsole != NULL && GConsole->Desktop != NULL)
    {
        NCMacroListWnd* List = DesktopList();
        if (List != NULL)
        {
            NCWnd::ShowWindow();
            NMacroSlot* Slot = &List->Slots[List->Selected];
            const TCHAR* ShortName = Slot->Info.ShortName;
            if (ShortName == NULL || appStrlen(ShortName) > 4)
                ShortName = NULL;
            SetShortName(ShortName);
            const TCHAR* Description = Slot->Info.Description;
            if (Description != NULL && appStrlen(Description) <= 32)
                SetDescription(Description);
            else
                SetDescription(NULL);
        }
    }
}
NW_HOOK_METHOD(0x1008ba20, NCMacroInfoWnd, ShowWindow)

// 0x1008bac0 (vslot 44) - both boxes emptied
void NCMacroInfoWnd::HideWindow()
{
    if (DescEdit != NULL)
        DescEdit->Clear();
    if (ShortNameEdit != NULL)
        ShortNameEdit->ClearText();
    NCWnd::HideWindow();
}
NW_HOOK_METHOD(0x1008bac0, NCMacroInfoWnd, HideWindow)

// 0x1008baf0 (vslot 67) - OK: the description (32 characters at most: else the message 0x345) and the short name (4 at most;
// a longer one is left as it was) go into the slot of the macro being edited, and the window closes
INT NCMacroInfoWnd::OnOK()
{
    if (GConsole != NULL && GConsole->Desktop != NULL)
    {
        NCMacroListWnd* List = DesktopList();
        if (List != NULL && List->Selected < NMACRO_MaxMacros)
        {
            NMacroSlot* Slot = &List->Slots[List->Selected];
            if (DescEdit->GetText() == NULL)
                appMemset(Slot->Info.Description, 0, sizeof(Slot->Info.Description));
            else
            {
                if (appStrlen(DescEdit->GetText()) > 32)
                {
                    GConsole->ShowSystemMessage(MSG_MacroDescTooLong, COLOR_SystemMsg);
                    return 0;
                }
                appMemset(Slot->Info.Description, 0, sizeof(Slot->Info.Description));
                appStrcpy(Slot->Info.Description, DescEdit->GetText());
            }
            if (ShortNameEdit->CommitText() == NULL)
                appMemset(Slot->Info.ShortName, 0, sizeof(Slot->Info.ShortName));
            if (ShortNameEdit->CommitText() != NULL && appStrlen(ShortNameEdit->CommitText()) <= 4)
            {
                appMemset(Slot->Info.ShortName, 0, sizeof(Slot->Info.ShortName));
                appStrcpy(Slot->Info.ShortName, ShortNameEdit->CommitText());
            }
            HideWindow();
            return 1;
        }
    }
    return 0;
}
NW_HOOK_METHOD(0x1008baf0, NCMacroInfoWnd, OnOK)

// vslot 68 - HideWindow (the stock body, 0x100a0830, is NCOptionWnd::OnCancel's, which hooks it)
INT NCMacroInfoWnd::OnCancel()
{
    HideWindow();
    return 1;
}

// 0x1008e5e0 (vslot 36) - the short name box (4 characters), the description box, the title bar (id 3; the stock code
// does not call NCWnd::OnCreate), OK and Cancel
INT NCMacroInfoWnd::OnCreate()
{
    ShortNameEdit = new NCEditBox(0, 0, 1, 0, 0);
    ShortNameEdit->SetMaxLength(4);   // the box's longest text
    ShortNameEdit->ID = NID_MACROINFO_SHORTNAME;
    ShortNameEdit->SetWindowRect(91, 36, 63, 16);
    CreateChild(ShortNameEdit, NWS_VISIBLE);

    DescEdit = new NCMultiEdit();
    DescEdit->ID = NID_MACROINFO_DESC;
    DescEdit->SetWindowRect(14, 76, 211, 74);
    CreateChild(DescEdit, NWS_VISIBLE);

    BackTex = FindTexture(TEXT("l2ui.macrownd.macro_additional_info"));

    FrameCtrl = new NCFrameCtrl(1);
    FrameCtrl->ID = 3;
    FrameCtrl->SetWindowRect(0, 0, 240, 21);
    CreateChild(FrameCtrl, NWS_VISIBLE);

    OKButton = new NCButton(TEXT("L2UI.control.DefaultButton"), TEXT("L2UI.control.DefaultButton_click"), 0);
    OKButton->SetWindowRect(49, 158, 68, 19);
    OKButton->ID = NID_MACROINFO_OK;
    CreateChild(OKButton, NWS_VISIBLE);
    OKButton->SetTextID(STR_OK);

    CancelButton = new NCButton(TEXT("L2UI.control.DefaultButton"), TEXT("L2UI.control.DefaultButton_click"), 0);
    CancelButton->SetWindowRect(121, 158, 68, 19);
    CancelButton->ID = NID_MACROINFO_CANCEL;
    CreateChild(CancelButton, NWS_VISIBLE);
    CancelButton->SetTextID(STR_Cancel);
    return 1;
}
NW_HOOK_METHOD(0x1008e5e0, NCMacroInfoWnd, OnCreate)

// 0x1008e940 (vslot 37)
INT NCMacroInfoWnd::OnDestroy()
{
    guard(NCMacroInfoWnd::OnDestroy);
    DestroyChild(this, ShortNameEdit);
    DestroyChild(this, DescEdit);
    DestroyChild(this, FrameCtrl);
    DestroyChild(this, OKButton);
    DestroyChild(this, CancelButton);
    NCWnd::OnDestroy();
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x1008e940, NCMacroInfoWnd, OnDestroy)

// 0x1008ea90 (vslot 55) - the background and the two labels ("Short name" over the box, "Description" over the other)
INT NCMacroInfoWnd::OnPaint(NCGDevice* Device)
{
    guard(NCMacroInfoWnd::OnPaint);
    Device->SetClip(ScreenX, ScreenY, Width, Height);
    Device->BeginPaint(ScreenX, ScreenY);
    if (BackTex)
        Device->DrawStretchedTex(0, 21, 240, 161, 0, 0, 240, 161, BackTex);
    Device->DrawText(17, 60, 0xFFFFFFFF, *SysString(STR_MacroDescription), 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
    Device->DrawText(17, 37, 0xFFFFFFFF, *SysString(STR_MacroShortName), 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
    Device->EndPaint();
    Device->ReleaseClip();
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x1008ea90, NCMacroInfoWnd, OnPaint)

NW_CHECK_VTABLE(NCMacroInfoWnd, 0x1011ae40, 69)
