/*=============================================================================
    NCMacroEditWnd.cpp - the macro editor.
    Stock: 0x1008B750 .. 0x1008B93D (WindowProc, the small handlers, HideWindow), 0x1008DBD0 (OnPaint),
    0x1008DE20 .. 0x1008E33F (the marks, the lines for the request, ShowWindow), 0x1008E340 (OnDestroy),
    0x1008F4F0 (OnSave), 0x1008FD20 .. 0x1008FDEF (constructor, destructor), 0x1008FDF0 (OnCreate).
    OnCancel (0x100A0830) is the body NCOptionWnd::OnCancel hooks.
=============================================================================*/
#include "nwindow/NCMacroEditWnd.h"
#include "nwindow/NCMacroEditList.h"
#include "nwindow/NCMacroListWnd.h"
#include "nwindow/NCButton.h"
#include "nwindow/NCConsole.h"
#include "nwindow/NCEditBox.h"
#include "nwindow/NCFrameCtrl.h"
#include "nwindow/NCGDevice.h"
#include "nwindow/NCIcon.h"
#include "nwindow/NConsoleWnd.h"
#include "nwindow/NCrt.h"

enum
{
    MSG_MacroLinesInvalid = 0x32A,     // systemmsg (names ours): a line is not a command, or there is a gap
    MSG_MacroNameEmpty = 0x346,
    MSG_MacroNameUsed = 0x347,
    COLOR_SystemMsg = 0xFFB09B79,
    MACRO_NumIcons = 7,                // the icons to choose from
};

// sysstring labels (SysString index = (stock offset - 0x4B4E8) / 12; names ours)
enum
{
    STR_MacroName = 656,              // 0x4D3A8: drawn at (28, 25) next to the name box
    STR_Cancel = 141,
    STR_MacroHelp = 145,              // the button 0x100C's text (0x91)
    STR_MacroSave = 658,              // 0x292
    STR_MacroInfo = 657,              // 0x291
};

// the desktop's macro windows (NCConsole.h keeps them as NCWnd*: +0xF8 the list, +0xFC this, +0x100 the information
// window; names ours)
static NCMacroListWnd* DesktopList()
{
    return nw::Field<NCMacroListWnd*>(GConsole->Desktop, 0xF8);
}
static NCWnd* DesktopInfo()
{
    return nw::Field<NCWnd*>(GConsole->Desktop, 0x100);
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

/*-----------------------------------------------------------------------------
    Constructor, destructor, WindowProc and the small handlers.
-----------------------------------------------------------------------------*/

// 0x1008fd20
NCMacroEditWnd::NCMacroEditWnd()
{
    SaveButton = NULL;
    HelpButton = NULL;
    CancelButton = NULL;
}
NW_HOOK_CTOR(0x1008fd20, NCMacroEditWnd)

// 0x1008fd90
NW_HOOK_DTOR(0x1008fd90, NCMacroEditWnd)

// 0x1008b750 (vslot 25) - the buttons
INT NCMacroEditWnd::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (NCWnd::WindowProc(Msg, wParam, lParam))
        return 1;
    if (Msg == NWM_COMMAND)
    {
        if (wParam == NID_MACROEDIT_SAVE)
            return OnSave() != 0;
        if (wParam == NID_MACROEDIT_CANCEL)
            return OnCancel() != 0;
        if (wParam == NID_MACROEDIT_PREVICON)
            return OnPrevIcon() != 0;
        if (wParam == NID_MACROEDIT_NEXTICON)
            return OnNextIcon() != 0;
        if (wParam == NID_MACROEDIT_INFO)
            return OnInfo() != 0;
        if (wParam == NID_MACROEDIT_HELP)
            return OnHelp() != 0;
    }
    return 0;
}
NW_HOOK_METHOD(0x1008b750, NCMacroEditWnd, WindowProc)

// 0x1008b840 (vslot 70) - the icon before (round the seven)
INT NCMacroEditWnd::OnPrevIcon()
{
    IconIndex--;
    if (IconIndex < 0)
        IconIndex = MACRO_NumIcons - 1;
    return 1;
}
NW_HOOK_METHOD(0x1008b840, NCMacroEditWnd, OnPrevIcon)

// 0x1008b860 (vslot 71) - the icon after
INT NCMacroEditWnd::OnNextIcon()
{
    IconIndex++;
    if (IconIndex > MACRO_NumIcons - 1)
        IconIndex = 0;
    return 1;
}
NW_HOOK_METHOD(0x1008b860, NCMacroEditWnd, OnNextIcon)

// 0x1008b890 (vslot 72) - the additional information window (short name, description) shown, or hidden when it is up
INT NCMacroEditWnd::OnInfo()
{
    if (GConsole != NULL && GConsole->Desktop != NULL && DesktopInfo() != NULL)
    {
        if (DesktopInfo()->IsShowWindow())
        {
            DesktopInfo()->HideWindow();
            return 1;
        }
        DesktopInfo()->ShowWindow();
        DesktopInfo()->SetFocus();
    }
    return 1;
}
NW_HOOK_METHOD(0x1008b890, NCMacroEditWnd, OnInfo)

// 0x1008b900 (vslot 69) - the macro help page
INT NCMacroEditWnd::OnHelp()
{
    GDesktop->ToggleHelp(1);
    return 1;
}
NW_HOOK_METHOD(0x1008b900, NCMacroEditWnd, OnHelp)

// vslot 68 - HideWindow (the stock body, 0x100a0830, is NCOptionWnd::OnCancel's, which hooks it)
INT NCMacroEditWnd::OnCancel()
{
    HideWindow();
    return 1;
}

// 0x1008b920 (vslot 44) - the information window goes with it
void NCMacroEditWnd::HideWindow()
{
    NCWnd::HideWindow();
    DesktopInfo()->HideWindow();
}
NW_HOOK_METHOD(0x1008b920, NCMacroEditWnd, HideWindow)

// 0x1008e280 (vslot 43) - the window takes the size of its minimize icon's place (the icon hides), the lines are laid out, the
// icon of the macro being edited is the current one, and the information window is hidden
void NCMacroEditWnd::ShowWindow()
{
    NCWnd::ShowWindow();
    if (Icon != NULL)
    {
        SetWindowRect(Icon->ScreenX, Icon->ScreenY, Width, Height);
        Icon->SetWindowRect(0, 0, 0, 0);
        Icon->HideWindow();
    }
    EditList->UpdateSlider();
    EditList->LayoutLines();
    NCMacroListWnd* List = NULL;
    if (GConsole != NULL && GConsole->Desktop != NULL && DesktopList() != NULL)
        List = DesktopList();
    if (List->Selected < List->Macros.Num())
        IconIndex = List->Macros(List->Selected)->IconType;
    DesktopInfo()->HideWindow();
}
NW_HOOK_METHOD(0x1008e280, NCMacroEditWnd, ShowWindow)

// 0x1008de20 - the mark of every line box cleared (NCEditBox +0x1D8, the line highlight)
void NCMacroEditWnd::ClearMarks()
{
    for (INT i = 0; i < NMACRO_EditLines; i++)
        EditList->Edits(i)->Unknown1D8 = 0;
}
NW_HOOK_METHOD(0x1008de20, NCMacroEditWnd, ClearMarks)

// 0x1008ded0 - the mark of one line box cleared
void NCMacroEditWnd::ClearMark(INT Line)
{
    if (Line < NMACRO_EditLines)
        EditList->Edits(Line)->Unknown1D8 = 0;
}
NW_HOOK_METHOD(0x1008ded0, NCMacroEditWnd, ClearMark)

/*-----------------------------------------------------------------------------
    The window.
-----------------------------------------------------------------------------*/

// 0x1008fdf0 (vslot 36) - the seven icons, Cancel / Save / Help buttons (OnSave 0x1011 at x 87, Cancel 0x1018 at 159,
// Help 0x100C at 15), the arrows and the information button, the name box, the title bar and the lines
INT NCMacroEditWnd::OnCreate()
{
    NCWnd::OnCreate();
    BackTex = FindTexture(TEXT("l2ui.macrownd.macro_edit_window"));
    IconTexs.Empty();
    for (INT i = 1; i <= MACRO_NumIcons; i++)
    {
        TCHAR Name[50];
        appSprintf(Name, TEXT("%s%d"), TEXT("L2UI.MacroWnd.Macro_Icon"), i);
        IconTexs.AddItem(FindTexture(Name));
    }
    IconIndex = 0;

    HelpButton = new NCButton(TEXT("L2UI.control.DefaultButton"), TEXT("L2UI.control.DefaultButton_click"), 0);
    HelpButton->ID = NID_MACROEDIT_HELP;
    HelpButton->SetWindowRect(15, 336, 67, 19);
    CreateChild(HelpButton, NWS_VISIBLE);
    HelpButton->SetTextID(145);

    SaveButton = new NCButton(TEXT("L2UI.control.DefaultButton"), TEXT("L2UI.control.DefaultButton_click"), 0);
    SaveButton->ID = NID_MACROEDIT_SAVE;
    SaveButton->SetWindowRect(87, 336, 67, 19);
    CreateChild(SaveButton, NWS_VISIBLE);
    SaveButton->SetTextID(0x292);

    CancelButton = new NCButton(TEXT("L2UI.control.DefaultButton"), TEXT("L2UI.control.DefaultButton_click"), 0);
    CancelButton->ID = NID_MACROEDIT_CANCEL;
    CancelButton->SetWindowRect(159, 336, 67, 19);
    CreateChild(CancelButton, NWS_VISIBLE);
    CancelButton->SetTextID(STR_Cancel);

    PrevIconButton = new NCButton(TEXT("L2UI.ShortcutWnd.LeftBtn1"), TEXT("L2UI.ShortcutWnd.LeftBtn2"), 0);
    PrevIconButton->ID = NID_MACROEDIT_PREVICON;
    PrevIconButton->SetWindowRect(18, 83, 12, 12);
    CreateChild(PrevIconButton, NWS_VISIBLE);

    NextIconButton = new NCButton(TEXT("L2UI.ShortcutWnd.RightBtn1"), TEXT("L2UI.ShortcutWnd.RightBtn2"), 0);
    NextIconButton->ID = NID_MACROEDIT_NEXTICON;
    NextIconButton->SetWindowRect(38, 83, 12, 12);
    CreateChild(NextIconButton, NWS_VISIBLE);

    InfoButton = new NCButton(TEXT("L2UI.control.DefaultButton"), TEXT("L2UI.control.DefaultButton_click"), 0);
    InfoButton->ID = NID_MACROEDIT_INFO;
    InfoButton->SetWindowRect(61, 78, 67, 17);
    CreateChild(InfoButton, NWS_VISIBLE);
    InfoButton->SetTextID(0x291);

    NameEdit = new NCEditBox(0, 0, 1, 0x108, 0);
    NameEdit->ID = NID_MACROEDIT_NAME;
    NameEdit->SetMaxLength(12);   // the box's longest text
    NameEdit->SetWindowRect(62, 51, 162, 16);
    CreateChild(NameEdit, NWS_VISIBLE);

    FrameCtrl = new NCFrameCtrl(1);
    FrameCtrl->ID = 1;
    FrameCtrl->SetWindowRect(0, 0, 240, 21);
    CreateChild(FrameCtrl, NWS_VISIBLE);

    EditList = new NCMacroEditList(8);
    EditList->SetWindowRect(1, 109, 238, 217);
    CreateChild(EditList, NWS_VISIBLE);
    return 1;
}
NW_HOOK_METHOD(0x1008fdf0, NCMacroEditWnd, OnCreate)

// 0x1008e340 (vslot 37)
INT NCMacroEditWnd::OnDestroy()
{
    guard(NCMacroEditWnd::OnDestroy);
    DestroyChild(this, FrameCtrl);
    DestroyChild(this, HelpButton);
    DestroyChild(this, SaveButton);
    DestroyChild(this, CancelButton);
    DestroyChild(this, PrevIconButton);
    DestroyChild(this, NextIconButton);
    DestroyChild(this, InfoButton);
    DestroyChild(this, NameEdit);
    DestroyChild(this, EditList);
    NCWnd::OnDestroy();
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x1008e340, NCMacroEditWnd, OnDestroy)

// 0x1008dbd0 (vslot 55) - the background, the icon chosen, the label of the name box and the macro's short name (up to
// four characters, two to a line) next to the icon
INT NCMacroEditWnd::OnPaint(NCGDevice* Device)
{
    guard(NCMacroEditWnd::OnPaint);
    Device->BeginPaint(ScreenX, ScreenY);
    Device->SetClip(ScreenX, ScreenY, Width, Height);
    if (GConsole != NULL && GConsole->Desktop != NULL && DesktopList() != NULL)
    {
        NCMacroListWnd* List = DesktopList();
        if (BackTex)
            Device->DrawStretchedTex(0, 21, 240, 344, 0, 0, 240, 344, BackTex);
        Device->DrawTexture(19, 51, 32, 32, 0, 0, 32, 32, IconTexs(IconIndex));
        Device->DrawText(28, 25, 0xFFFFFFFF, *SysString(STR_MacroName), 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
        const TCHAR* ShortName = List->Slots[List->Selected].Info.ShortName;
        if (ShortName != NULL)
        {
            INT X = 23;
            INT Y = 55;
            FString Text(ShortName);
            for (INT k = 0; k < Text.Len(); k++)
            {
                Device->DrawText(X, Y, 0xFFFFFFFF, *Text.Mid(k, 1), 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
                if ((k & 1) == 0)
                    X += 13;
                else
                {
                    Y += 13;
                    X -= 13;
                }
            }
        }
        Device->EndPaint();
        Device->ReleaseClip();
        return 1;
    }
    return 0;
    unguard;
}
NW_HOOK_METHOD(0x1008dbd0, NCMacroEditWnd, OnPaint)

/*-----------------------------------------------------------------------------
    Saving.
-----------------------------------------------------------------------------*/

// 0x1008f4f0 (vslot 67) - Save: the lines must be valid (message 0x32A), the name given (0x346) and not another macro's
// (0x347, compared without case); then the window closes and the macro goes to the server: its id (0 for a new one), the name
// (12 characters at most), the description and the short name of the slot, the icon and the lines
INT NCMacroEditWnd::OnSave()
{
    if (GConsole == NULL || GConsole->Desktop == NULL || DesktopList() == NULL)
        return 0;
    NCMacroListWnd* List = DesktopList();
    if (!EditList->ValidateLines())
    {
        GConsole->ShowSystemMessage(MSG_MacroLinesInvalid, COLOR_SystemMsg);
        return 0;
    }
    if (NameEdit->CommitText() == NULL)
    {
        GConsole->ShowSystemMessage(MSG_MacroNameEmpty, COLOR_SystemMsg);
        return 0;
    }
    INT Num = List->Macros.Num();
    INT Sel = List->Selected;
    INT i;
    for (i = 0; i < Num; i++)
    {
        if (Sel < Num)
        {
            if (List->Macros(Sel)->ID != List->Macros(i)->ID && List->Macros(i)->GetName() != NULL)
            {
                if (appStricmp(List->Macros(i)->GetName(), NameEdit->CommitText()) == 0)
                    break;
            }
        }
        else if (List->Macros(i)->GetName() != NULL)
        {
            if (appStricmp(List->Macros(i)->GetName(), NameEdit->CommitText()) == 0)
                break;
        }
        Num = List->Macros.Num();
    }
    if (i < List->Macros.Num())
    {
        GConsole->ShowSystemMessage(MSG_MacroNameUsed, COLOR_SystemMsg);
        return 0;
    }
    HideWindow();
    Sel = List->Selected;
    if (Sel < 0 || Sel >= NMACRO_MaxMacros)
        return 1;
    L2ParamStack Params(0x44);
    if (Sel < List->Macros.Num())
        Params.PushBack((void*)(INT_PTR)List->Macros(Sel)->ID);
    else if (Sel == List->Macros.Num())
        Params.PushBack((void*)0);
    if (NameEdit->CommitText() != NULL && appStrlen(NameEdit->CommitText()) <= 12)
        Params.PushBack((void*)NameEdit->CommitText());
    else
        Params.PushBack((void*)0);
    NMacroSlot* Slot = &List->Slots[Sel];
    Params.PushBack((void*)Slot->Info.Description);
    Params.PushBack((void*)Slot->Info.ShortName);
    Params.PushBack((void*)(INT_PTR)IconIndex);
    PushLines(&Params);
    GConsole->RequestMakeMacro(Params);
    return 1;
}
NW_HOOK_METHOD(0x1008f4f0, NCMacroEditWnd, OnSave)

// the command code of the lines the stock parser recognizes (NCommandParser::GetCommand); names ours
enum
{
    LINECMD_UseSkill = 0x18,           // /useskill <name>: the macro keeps the skill's id
    LINECMD_UseShortCut = 0x1B,        // /useshortcut <page> <slot>
    LINECMD_Delay = 0x4C,              // /delay <seconds>
};

// 0x1008df00 - the lines for the save request: how many of the boxes hold text (the first empty one's number, 12 when none
// is), then for each line a record of five values: its number (from 1), its type (1 skill, 3 text, 4 shortcut, 6 delay), two
// numbers (the skill's id and whether the name was found; the page and the slot, from 0; the delay) and the text (only for
// the type 3). A line the parser rejects is left out. The lines' numbers advance even for a /delay or /useshortcut that
// turns out invalid, as in the stock code
void NCMacroEditWnd::PushLines(L2ParamStack* Params)
{
    if (Params == NULL)
        return;
    INT Count = 0;
    do
    {
        if (EditList->Edits(Count)->CommitText() == NULL)
        {
            Params->PushBack((void*)(INT_PTR)Count);
            break;
        }
        Count++;
    }
    while (Count < NMACRO_EditLines);
    if (Count == NMACRO_EditLines)
        Params->PushBack((void*)NMACRO_EditLines);

    INT Number = 0;
    for (INT i = 0; i < NMACRO_EditLines; i++)
    {
        const TCHAR* Text = EditList->Edits(i)->CommitText();
        INT Next = Number;
        TCHAR Line[34];
        if (Text == NULL)
        {
            appMemset(Line, 0, 0x42);   // 33 characters of the 34 (as in the stock code)
        }
        else
        {
            appStrcpy(Line, Text);
            INT Command = EditList->Parser.GetCommand(Line);
            UBOOL bPlain = 0;
            if (Command > 0x33)
            {
                if (Command < 0x52)
                {
                    if (Command > 0x4C)
                        bPlain = 1;
                    else if (Command < 0x4C)
                        bPlain = 1;
                    else
                    {
                        // /delay
                        Next = Number + 1;
                        TCHAR* Token = EditList->Parser.GetNextToken();
                        INT Seconds;
                        if (Token != NULL && (Seconds = NWtol(Token)) > 0)
                        {
                            Params->PushBack((void*)(INT_PTR)Next);
                            Params->PushBack((void*)6);
                            Params->PushBack((void*)(INT_PTR)Seconds);
                            Params->PushBack((void*)0);
                            Params->PushBack((void*)0);
                        }
                    }
                }
                else if (Command >= 0x10000000)
                {
                    if (Command < 0x10000002)
                    {
                        User* Player = GConsole->GetMyUser();
                        if (Player != NULL && Player->BuilderLevel != 0)
                            bPlain = 1;
                    }
                    else if (Command == 0x20000000)
                        bPlain = 1;
                }
            }
            else if (Command < 0x2E)
            {
                // (a negative code, no command, is skipped: the stock switch's range check is unsigned)
                if ((DWORD)Command > 0x2D)
                {
                }
                else if (Command == LINECMD_UseSkill)
                {
                    TCHAR SkillName[300];
                    appSprintf(SkillName, TEXT("%s"), EditList->Parser.GetNextToken());
                    TCHAR* Token = EditList->Parser.GetNextToken();
                    while (Token != NULL)
                    {
                        appSprintf(SkillName, TEXT("%s %s"), SkillName, Token);
                        Token = EditList->Parser.GetNextToken();
                    }
                    FL2MagicSkillCommandData* Data = GL2GameData.GetMSCommandData(SkillName);
                    INT SkillID = Data != NULL ? *(INT*)Data : 0;
                    INT bFound = Data != NULL;
                    Params->PushBack((void*)(INT_PTR)(Number + 1));
                    Params->PushBack((void*)1);
                    Params->PushBack((void*)(INT_PTR)SkillID);
                    Params->PushBack((void*)(INT_PTR)bFound);
                    Params->PushBack((void*)0);
                    Next = Number + 1;
                }
                else if (Command == LINECMD_UseShortCut)
                {
                    Next = Number + 1;
                    TCHAR* Token = EditList->Parser.GetNextToken();
                    INT Page;
                    INT Slot;
                    if (Token != NULL && (Page = NWtol(Token)) > 0 && Page < 11
                        && (Token = EditList->Parser.GetNextToken()) != NULL && (Slot = NWtol(Token)) > 0 && Slot < 11)
                    {
                        Params->PushBack((void*)(INT_PTR)Next);
                        Params->PushBack((void*)4);
                        Params->PushBack((void*)(INT_PTR)(Page - 1));
                        Params->PushBack((void*)(INT_PTR)(Slot - 1));
                        Params->PushBack((void*)0);
                    }
                }
                else
                    bPlain = 1;
            }
            else
                bPlain = 1;
            if (bPlain)
            {
                Params->PushBack((void*)(INT_PTR)(Number + 1));
                Params->PushBack((void*)3);
                Params->PushBack((void*)0);
                Params->PushBack((void*)0);
                Params->PushBack((void*)Text);
                Next = Number + 1;
            }
        }
        Number = Next;
    }
}
NW_HOOK_METHOD(0x1008df00, NCMacroEditWnd, PushLines)

NW_CHECK_VTABLE(NCMacroEditWnd, 0x1011b3d0, 73)
