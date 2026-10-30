/*=============================================================================
    NCPetitionBeginWnd.cpp - the petition form (C2 only). Stock: 0x100b6980 .. 0x100b6a0f
    (WindowProc), 0x100b6a20 .. 0x100b6b0c, 0x100b6b40 .. 0x100b6b66, 0x100b7680 .. 0x100b7f55
    and the constructor and destructor at 0x1004c740 .. 0x1004c7df.
=============================================================================*/
#include "nwindow/NCPetitionBeginWnd.h"
#include "nwindow/NCButton.h"
#include "nwindow/NCComboBox.h"
#include "nwindow/NCFrameCtrl.h"
#include "nwindow/NCGDevice.h"
#include "nwindow/NCHtmlViewer.h"
#include "nwindow/NCMultiEdit.h"
#include "nwindow/NCPetitionWnd.h"
#include "nwindow/NDialogInfo.h"
#include "nwindow/NConsoleWnd.h"

// ids (names ours)
enum
{
    NID_PETBEGIN_COMBO = 0xF0E0A1,
    NID_PETBEGIN_OK = 0xF0E0A2,
    NID_PETBEGIN_CANCEL = 0xF0E0A3,
    NID_PETBEGIN_EDIT = 0xF0E0A4,
    NID_PETBEGIN_FRAME = 0xF0A0B5,
};
// system messages the form tells (names ours) and the box's id (the plain notice)
enum
{
    MSG_PetitionNoKind = 0x324,
    MSG_PetitionTooShort = 0x182,
    MSG_PetitionTooLong = 0x3CB,
    DIALOG_Notice = 0x1F,
};
// the sysstring ids of the buttons, the kinds and the three texts the panel shows (names ours)
enum
{
    SYSSTR_PetitionOK = 694,
    SYSSTR_PetitionCancel = 473,
    SYSSTR_PetitionKindFirst = 695,    // 695 .. 704, ten entries
    SYSSTR_PetitionKindLabel = 705,
    SYSSTR_PetitionTextLabel = 707,
    SYSSTR_PetitionSender = 708,       // "<sender> : <text>" in the conversation's first line
    SYSSTR_PetitionHelpLabel = 412,
};

// 0x1004c740 - the controls come with OnCreate
NCPetitionBeginWnd::NCPetitionBeginWnd()
{
}
NW_HOOK_CTOR(0x1004c740, NCPetitionBeginWnd)

// 0x1004c790
NW_HOOK_DTOR(0x1004c790, NCPetitionBeginWnd)

// 0x100b6980 (vslot 25)
INT NCPetitionBeginWnd::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (NCWnd::WindowProc(Msg, wParam, lParam))
        return 1;
    if (Msg == NWM_COMMAND)
    {
        if (wParam == NID_PETBEGIN_OK)
            return OnSend() != 0;
        if (wParam == NID_PETBEGIN_CANCEL)
            return OnCancel() != 0;
        if (wParam == NID_PETBEGIN_COMBO)
            return OnTypeChosen(lParam) != 0;
    }
    return 0;
}
NW_HOOK_METHOD(0x100b6980, NCPetitionBeginWnd, WindowProc)

// 0x100b6a20 (vslot 69) - the help page of the kind chosen (1..9); 0 and the rest leave the page
INT NCPetitionBeginWnd::OnTypeChosen(INT Type)
{
    switch (Type)
    {
    case 1:
        HelpViewer->LoadHtmlFile((TCHAR*)TEXT("..\\L2text\\pet_help_move.htm"));
        return 1;
    case 2:
        HelpViewer->LoadHtmlFile((TCHAR*)TEXT("..\\L2text\\pet_help_recover.htm"));
        return 1;
    case 3:
        HelpViewer->LoadHtmlFile((TCHAR*)TEXT("..\\L2text\\pet_help_bug.htm"));
        return 1;
    case 4:
        HelpViewer->LoadHtmlFile((TCHAR*)TEXT("..\\L2text\\pet_help_quest.htm"));
        return 1;
    case 5:
        HelpViewer->LoadHtmlFile((TCHAR*)TEXT("..\\L2text\\pet_help_report.htm"));
        return 1;
    case 6:
        HelpViewer->LoadHtmlFile((TCHAR*)TEXT("..\\L2text\\pet_help_suggest.htm"));
        return 1;
    case 7:
        HelpViewer->LoadHtmlFile((TCHAR*)TEXT("..\\L2text\\pet_help_game.htm"));
        return 1;
    case 8:
        HelpViewer->LoadHtmlFile((TCHAR*)TEXT("..\\L2text\\pet_help_oprn.htm"));
        return 1;
    case 9:
        HelpViewer->LoadHtmlFile((TCHAR*)TEXT("..\\L2text\\pet_help_etc.htm"));
    }
    return 1;
}
NW_HOOK_METHOD(0x100b6a20, NCPetitionBeginWnd, OnTypeChosen)

// 0x100b6b40 - kind none, the text emptied, the help page cleared
void NCPetitionBeginWnd::ResetForm()
{
    TypeCombo->SetSelected(0);
    TextEdit->Clear();
    HelpViewer->Clear();
}
NW_HOOK_METHOD(0x100b6b40, NCPetitionBeginWnd, ResetForm)

// 0x100b7680 (vslot 36) - title bar, the kinds, the help page, the text and the two buttons
INT NCPetitionBeginWnd::OnCreate()
{
    NCWnd::OnCreate();
    BackTex = FindTexture(TEXT("L2UI.PetitionWnd.PetitionBeginBack"));

    FrameCtrl = new NCFrameCtrl(1);
    FrameCtrl->ID = NID_PETBEGIN_FRAME;
    FrameCtrl->SetWindowRect(0, 0, 240, 21);
    CreateChild(FrameCtrl, NWS_VISIBLE);

    TypeCombo = new NCComboBox();
    TypeCombo->ID = NID_PETBEGIN_COMBO;
    TypeCombo->SetWindowRect(55, 46, 120, 15);
    CreateChild(TypeCombo, NWS_VISIBLE);

    HelpViewer = new NCHtmlViewer();
    HelpViewer->SetWindowRect(4, 64, 234, 130);
    CreateChild(HelpViewer, NWS_VISIBLE);

    TextEdit = new NCMultiEdit();
    TextEdit->ID = NID_PETBEGIN_EDIT;
    TextEdit->SetWindowRect(4, 220, 218, 120);
    CreateChild(TextEdit, NWS_VISIBLE);

    OKButton = new NCButton(TEXT("L2UI.control.DefaultButton"), TEXT("L2UI.control.DefaultButton_click"), 0);
    OKButton->ID = NID_PETBEGIN_OK;
    OKButton->SetWindowRect(47, 342, 64, 19);
    CreateChild(OKButton, NWS_VISIBLE);
    OKButton->SetTextID(SYSSTR_PetitionOK);

    CancelButton = new NCButton(TEXT("L2UI.control.DefaultButton"), TEXT("L2UI.control.DefaultButton_click"), 0);
    CancelButton->ID = NID_PETBEGIN_CANCEL;
    CancelButton->SetWindowRect(133, 342, 64, 19);
    CreateChild(CancelButton, NWS_VISIBLE);
    CancelButton->SetTextID(SYSSTR_PetitionCancel);

    // the ten kinds (the first is "choose one"; the nine after it are the ones OnTypeChosen knows)
    for (INT i = 0; i < 10; i++)
        TypeCombo->AddItem(SYSSTR_PetitionKindFirst + i);
    return 1;
}
NW_HOOK_METHOD(0x100b7680, NCPetitionBeginWnd, OnCreate)

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

// 0x100b7b00 (vslot 37) - guarded in C2
INT NCPetitionBeginWnd::OnDestroy()
{
    guard(NCPetitionBeginWnd::OnDestroy);
    DestroyChild(this, OKButton);
    DestroyChild(this, CancelButton);
    DestroyChild(this, HelpViewer);
    DestroyChild(this, TypeCombo);
    DestroyChild(this, TextEdit);
    NCWnd::OnDestroy();
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x100b7b00, NCPetitionBeginWnd, OnDestroy)

// 0x100b7c50 (vslot 55) - the panel (240 x 344) under the title bar, the help page's frame and the
// three labels; the origin goes on before the clip. The frame texture is looked up on every paint.
INT NCPetitionBeginWnd::OnPaint(NCGDevice* Device)
{
    Device->BeginPaint(ScreenX, ScreenY);
    Device->SetClip(ScreenX, ScreenY, Width, Height);
    Device->DrawTexture(0, 21, 240, 344, 0, 0, 240, 344, BackTex);
    UTexture* EditBack = FindTexture(TEXT("L2UI.EtcWnd.Edit_Back"));
    Device->DrawTexture(4, 64, 219, 130, 0, 0, 219, 130, EditBack);
    Device->DrawText(20, 25, 0xFFFFFFFF, *SysString(SYSSTR_PetitionKindLabel), 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0, 0);
    Device->DrawText(20, 47, 0xFFFFFFFF, *SysString(SYSSTR_PetitionHelpLabel), 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0, 0);
    Device->DrawText(20, 200, 0xFFFFFFFF, *SysString(SYSSTR_PetitionTextLabel), 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0, 0);
    Device->ReleaseClip();
    Device->EndPaint();
    return 1;
}
NW_HOOK_METHOD(0x100b7c50, NCPetitionBeginWnd, OnPaint)

// 0x100b7db0 (vslot 67) - OK: a kind must be chosen and the text be 5 to 254 characters; then the
// petition goes out, the conversation window starts with the player's own line and this form is
// emptied and closed. Else a notice box tells what is wrong (the form stays).
INT NCPetitionBeginWnd::OnSend()
{
    INT Type = TypeCombo->Selected;
    NDialogInfo Info;
    INT MsgID;
    if (Type == 0)
        MsgID = MSG_PetitionNoKind;
    else
    {
        TCHAR* Text = TextEdit->GetText();
        if (Text && appStrlen(Text) >= 5)
        {
            if (appStrlen(Text) >= 255)
                MsgID = MSG_PetitionTooLong;
            else
            {
                GConsole->RequestPetition(Text, Type);
                ((NCPetitionWnd*)GConsole->PetitionWnd)->Clear();
                TCHAR Line[516];
                appSprintf(Line, TEXT("%s : %s"), *SysString(SYSSTR_PetitionSender), Text);
                ((NCPetitionWnd*)GConsole->PetitionWnd)->AddMessage(Line, 0xFFFFFFFF, 0, 0);
                ResetForm();
                HideWindow();
                return 1;
            }
        }
        else
            MsgID = MSG_PetitionTooShort;
    }
    Info.SetSystemMessage(MsgID);
    Info.ID = DIALOG_Notice;
    Info.bMainBox = 1;
    Info.bInput = 0;
    GConsole->ShowDialog(&Info);
    return 1;
}
NW_HOOK_METHOD(0x100b7db0, NCPetitionBeginWnd, OnSend)

// 0x100b7f30 (vslot 68) - Cancel: a petition already open is withdrawn; the form is emptied and closed
INT NCPetitionBeginWnd::OnCancel()
{
    GConsole->RequestPetitionCancel();
    ResetForm();
    HideWindow();
    return 1;
}
NW_HOOK_METHOD(0x100b7f30, NCPetitionBeginWnd, OnCancel)

NW_CHECK_VTABLE(NCPetitionBeginWnd, 0x101132e8, 70)
