/*=============================================================================
    NCMacroListWnd.cpp - the macro window: the grid, the counter, the buttons, and the macros it keeps.
    Stock: 0x1008B510 (WindowProc), 0x1008B590 / 0x1008B5E0 (the icons' hit tests), 0x1008C330 .. 0x1008C450 (what the
    console's events do to the macro being run), 0x1008C7A0 (SetIcon), 0x1008C810 (OnPaint), 0x1008CA10 (OnNew),
    0x1008F0F0 (AddMacro), 0x1008F990 .. 0x1008FA7F (constructor, destructor), 0x10090490 (OnCreate),
    0x10090820 (OnDestroy). ShowWindow (0x1008C9C0) is NCSystemMenuWnd's, OnLButtonUp (0x100D2E70) a folded `return 1`.
=============================================================================*/
#include "nwindow/NCMacroListWnd.h"
#include "nwindow/NCMacroArrayWnd.h"
#include "nwindow/NCMacroEditList.h"
#include "nwindow/NCMacroEditWnd.h"
#include "nwindow/NCButton.h"
#include "nwindow/NCConsole.h"
#include "nwindow/NCEditBox.h"
#include "nwindow/NCFrameCtrl.h"
#include "nwindow/NCGDevice.h"
#include "nwindow/NConsoleWnd.h"
#include "nwindow/NCrt.h"

enum
{
    MSG_MacroListFull = 0x31D,         // systemmsg (name ours): no more than 24 macros
    COLOR_SystemMsg = 0xFFB09B79,
    MACRO_NumIcons = 7,                // the icons "L2UI.MacroWnd.Macro_Icon1".."7"
    MACRO_FrameID = 2,                 // the title bar's id (name ours)
};

// sysstring labels (SysString index = (stock offset - 0x4B4E8) / 12; names ours)
enum
{
    STR_Macro = 655,                  // 0x4D39C: the window's heading, at (28, 25)
    STR_MacroNew = 0x28E,             // the "new" button
    STR_MacroHelp = 0x91,             // the help button
};

// the command codes of NCommandParser::GetCommand the running line's wait is cut short for (/pickup; /target,
// /targetnext, /assist); the seconds it is cut to
enum { LINECMD_PickUp = 0xE, LINECMD_Target = 0xC, LINECMD_TargetNext = 0xD, LINECMD_Assist = 0xF };

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
    Constructor, destructor, WindowProc.
-----------------------------------------------------------------------------*/

// 0x1008f990 - the icons' rectangles are cleared (the array of macros is empty)
NCMacroListWnd::NCMacroListWnd()
{
    DeleteY = 0;
    DeleteX = 0;
    DeleteW = 0;
    DeleteH = 0;
    EditY = 0;
    EditX = 0;
    EditW = 0;
    EditH = 0;
}
NW_HOOK_CTOR(0x1008f990, NCMacroListWnd)

// 0x1008fa20
NW_HOOK_DTOR(0x1008fa20, NCMacroListWnd)

// 0x1008b510 (vslot 25) - the two buttons
INT NCMacroListWnd::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (NCWnd::WindowProc(Msg, wParam, lParam))
        return 1;
    if (Msg == NWM_COMMAND)
    {
        if (wParam == NID_MACROLIST_NEW)
            return OnNew() != 0;
        if (wParam == NID_MACROLIST_HELP)
            return OnHelp() != 0;
    }
    return 0;
}
NW_HOOK_METHOD(0x1008b510, NCMacroListWnd, WindowProc)

// vslot 68 - the macro help page (the stock body, 0x1008b900, is NCMacroEditWnd::OnHelp's, which hooks it)
INT NCMacroListWnd::OnHelp()
{
    GDesktop->ToggleHelp(1);
    return 1;
}

/*-----------------------------------------------------------------------------
    The icons' hit tests and the macros.
-----------------------------------------------------------------------------*/

// 0x1008b590 - (X, Y) in the delete icon's rectangle (borders included)
UBOOL NCMacroListWnd::HitDelete(INT X, INT Y)
{
    INT DX = X - (DeleteX + ScreenX);
    INT DY = Y - (DeleteY + ScreenY);
    if (DX < 0 || DX > DeleteW || DY < 0 || DY > DeleteH)
        return 0;
    return 1;
}
NW_HOOK_METHOD(0x1008b590, NCMacroListWnd, HitDelete)

// 0x1008b5e0 - (X, Y) in the edit icon's rectangle
UBOOL NCMacroListWnd::HitEdit(INT X, INT Y)
{
    INT DX = X - (EditX + ScreenX);
    INT DY = Y - (EditY + ScreenY);
    if (DX < 0 || DX > EditW || DY < 0 || DY > EditH)
        return 0;
    return 1;
}
NW_HOOK_METHOD(0x1008b5e0, NCMacroListWnd, HitEdit)

// 0x1008c330 - the console's event (NConsoleWnd slot 219): the macro being run waits for /pickup to finish: its line's
// wait is cut to one second
void NCMacroListWnd::OnPickUpDone()
{
    INT i;
    for (i = 0; i < Macros.Num(); i++)
    {
        if (Macros(i) != NULL && Macros(i)->bRunning != 0)
            break;
    }
    if (i >= Macros.Num())
        return;
    NMacroEntry* Macro = Macros(i);
    INT Offset = Macro->CurLine;
    if (Macro->Lines[Offset].Type == NMLT_Command && Macro->Parser.GetCommand(Macro->Lines[Offset].Text) == LINECMD_PickUp)
        Macros(i)->Lines[Offset].Duration = 1.0f;
}
NW_HOOK_METHOD(0x1008c330, NCMacroListWnd, OnPickUpDone)

// 0x1008c3c0 - the console's event (NConsoleWnd slot 218): the same for /target, /targetnext and /assist, cut to a thousandth
// of a second
void NCMacroListWnd::OnTargetDone()
{
    INT i;
    for (i = 0; i < Macros.Num(); i++)
    {
        if (Macros(i) != NULL && Macros(i)->bRunning != 0)
            break;
    }
    if (i >= Macros.Num())
        return;
    NMacroEntry* Macro = Macros(i);
    INT Offset = Macro->CurLine;
    if (Macro->Lines[Offset].Type == NMLT_Command)
    {
        INT Command = Macro->Parser.GetCommand(Macro->Lines[Offset].Text);
        if (Command == LINECMD_Target || Command == LINECMD_TargetNext || Command == LINECMD_Assist)
            Macros(i)->Lines[Offset].Duration = 0.001f;
    }
}
NW_HOOK_METHOD(0x1008c3c0, NCMacroListWnd, OnTargetDone)

// the record of the shortcut at Index of NCShortCutWnd's array (the desktop keeps the window at +0x104, the window's +0x1AC points at
// the records, 0x1A7C bytes each; the type is at +0x190C, the id of the skill or action at +0x1918: the C2 NShortCutItem; names ours)
static FORCEINLINE BYTE* ShortCutRecord(INT Index)
{
    return nw::Field<BYTE*>(nw::Field<NCWnd*>(GConsole->Desktop, 0x104), 0x1AC) + Index * 0x1A7C;
}

// 0x1008c450 - the console's SkillCoolTime event (NConsoleWnd slot 217): skill SkillID (0: any) is cooling for TimeMS milliseconds. The macro
// being run, when its line uses that skill (as a skill line, a /useskill command or a shortcut of the skill), waits that long (in whole
// seconds, rounded up; one second when the time is 0)
void NCMacroListWnd::OnSkillCoolTime(INT SkillID, INT TimeMS)
{
    for (INT i = 0; i < Macros.Num(); i++)
    {
        if (Macros(i) != NULL && Macros(i)->bRunning != 0)
        {
            NMacroEntry* Macro = Macros(i);
            INT Offset = Macro->CurLine;
            NMacroLine* Line = &Macro->Lines[Offset];
            INT Skill;
            if (Line->Type == NMLT_Skill)
                Skill = Line->Param08;
            else if (Line->Type == NMLT_ShortCut)
            {
                INT Page = Line->Param0C;
                INT Slot = Line->Param08;
                if (Page < 0 || Page > 9 || Slot < 0 || Slot > 9)
                    return;
                Skill = *(INT*)(ShortCutRecord(Slot + Page * 10) + 0x1918);
            }
            else if (Line->Type != NMLT_Command)
                return;
            else
            {
                NCommandParser Parser;
                INT Command = Parser.GetCommand(Line->Text);
                if (Command == 0x19 || Command == 0x1A)
                {
                    // /useskill <name of the skill>: the name is the tokens joined with blanks, the skill the table of the commands' skills gives
                    TCHAR Name[300];
                    appMemset(Name, 0, 600);
                    TCHAR* Token = Parser.GetNextToken();
                    Parser.ExpandToken(Token);   // 0x1003E900
                    appSprintf(Name, TEXT("%s"), Token);
                    Token = Parser.GetNextToken();
                    Parser.ExpandToken(Token);   // 0x1003E900
                    while (Token != NULL)
                    {
                        appStrcat(Name, TEXT(" "));
                        appStrcat(Name, Token);
                        Token = Parser.GetNextToken();
                        Parser.ExpandToken(Token);   // 0x1003E900
                    }
                    Skill = *(INT*)GL2GameData.GetMSCommandData(Name);
                }
                else if (Command == 0x1C || Command == 0x1D)
                {
                    // /useshortcut <page> <slot>: the skill of that shortcut
                    TCHAR* Token = Parser.GetNextToken();
                    Parser.ExpandToken(Token);   // 0x1003E900
                    if (Token == NULL)
                        return;
                    INT Page = NWtol(Token);
                    if (Page < 1 || Page > 10)
                        return;
                    Token = Parser.GetNextToken();
                    Parser.ExpandToken(Token);   // 0x1003E900
                    if (Token == NULL)
                        return;
                    INT Slot = NWtol(Token);
                    if (Slot < 1 || Slot > 10)
                        return;
                    BYTE* Record = ShortCutRecord(Slot + Page * 10 - 11);
                    if (*(INT*)(Record + 0x190C) == 2)
                        Skill = *(INT*)(Record + 0x1918);
                    else
                        Skill = (INT)(INT_PTR)this;   // the stock code leaves the register holding this: never a skill (kept as is)
                }
                else
                    Skill = (INT)(INT_PTR)this;       // the same: no skill known for any other command
            }
            if (SkillID != 0 && Skill != SkillID)
                return;
            if (TimeMS % 1000 > 0)
                Macros(i)->Lines[Offset].Duration = (FLOAT)(TimeMS / 1000 + 1);
            else if (TimeMS != 0)
                Macros(i)->Lines[Offset].Duration = (FLOAT)(TimeMS / 1000 + 1);
            else
                Macros(i)->Lines[Offset].Duration = 1.0f;
            return;
        }
    }
}
NW_HOOK_METHOD(0x1008c450, NCMacroListWnd, OnSkillCoolTime)

// 0x1008c7a0 - the icon texture of macro Index from its icon number
void NCMacroListWnd::SetIcon(INT Index)
{
    if (Index < Macros.Num() && Index >= 0)
    {
        NMacroEntry* Macro = Macros(Index);
        INT Type = Macro->IconType;
        if (Type >= 0 && Type < MACRO_NumIcons)
        {
            TCHAR Name[50];
            appSprintf(Name, TEXT("%s%d"), TEXT("L2UI.MacroWnd.Macro_Icon"), Type + 1);
            Macro->IconTex = FindTexture(Name);
        }
    }
}
NW_HOOK_METHOD(0x1008c7a0, NCMacroListWnd, SetIcon)

// 0x1008f0f0 - a macro added to the list: its tool tip is built (name and description) and its icon found
void NCMacroListWnd::AddMacro(NMacroEntry* Macro)
{
    if (Macro != NULL)
    {
        Macro->ToolTip.FillMacro(&Macro->ToolTip, NTT_Text);
        Macros.AddItem(Macro);
        SetIcon(Macros.Num() - 1);
    }
}
NW_HOOK_METHOD(0x1008f0f0, NCMacroListWnd, AddMacro)

/*-----------------------------------------------------------------------------
    The window.
-----------------------------------------------------------------------------*/

// 0x10090490 (vslot 36) - the "new" and help buttons, the two icons, the title bar and the grid (6 columns of 7 rows,
// 80 cells at most, 32 pixels each); the slots are cleared
INT NCMacroListWnd::OnCreate()
{
    NCWnd::OnCreate();
    BackTex = FindTexture(TEXT("l2ui.macrownd.macro_list"));

    NewButton = new NCButton(TEXT("L2UI.control.DefaultButton"), TEXT("L2UI.control.DefaultButton_click"), 0);
    NewButton->ID = NID_MACROLIST_NEW;
    NewButton->SetWindowRect(13, 328, 67, 19);
    CreateChild(NewButton, NWS_VISIBLE);
    NewButton->SetTextID(STR_MacroNew);

    HelpButton = new NCButton(TEXT("L2UI.control.DefaultButton"), TEXT("L2UI.control.DefaultButton_click"), 0);
    HelpButton->ID = NID_MACROLIST_HELP;
    HelpButton->SetWindowRect(83, 328, 67, 19);
    CreateChild(HelpButton, NWS_VISIBLE);
    HelpButton->SetTextID(STR_MacroHelp);

    EditTex = FindTexture(TEXT("L2UI.macrownd.macro_edit_button"));
    DeleteTex = FindTexture(TEXT("L2UI.WindowIcon.DestroyItem"));
    DeleteW = 32;
    DeleteH = 32;
    DeleteX = 189;
    DeleteY = 314;
    EditW = 32;
    EditH = 32;
    EditX = 152;
    EditY = 314;

    FrameCtrl = new NCFrameCtrl(1);
    FrameCtrl->ID = MACRO_FrameID;
    FrameCtrl->SetWindowRect(0, 0, 240, 21);
    CreateChild(FrameCtrl, NWS_VISIBLE);

    Array = new NCMacroArrayWnd(6, 7, 80, 32);
    Array->SetWindowRect(15, 48, 222, 245);
    CreateChild(Array, NWS_VISIBLE | 0x20);

    for (INT i = 0; i < NMACRO_MaxMacros; i++)
    {
        NMacroInfo* Info = &Slots[i].Info;
        appMemset(Info->ShortName, 0, sizeof(Info->ShortName));
        appMemset(Info->Name, 0, sizeof(Info->Name));
        appMemset(Info->Description, 0, sizeof(Info->Description));
        for (INT j = 0; j < NMACRO_MaxLines; j++)
            appMemset(Info->Lines[j].Text, 0, sizeof(Info->Lines[j].Text));
    }
    Selected = 0;
    return 1;
}
NW_HOOK_METHOD(0x10090490, NCMacroListWnd, OnCreate)

// 0x10090820 (vslot 37) - the macros are destroyed and their memory freed; the buttons, the title bar and the grid go
INT NCMacroListWnd::OnDestroy()
{
    guard(NCMacroListWnd::OnDestroy);
    for (INT i = 0; i < Macros.Num(); i++)
    {
        NMacroEntry* Macro = Macros(i);
        if (Macro != NULL)
        {
            Macro->~NMacroEntry();
            appFree(Macro);
        }
    }
    Macros.Empty();
    DestroyChild(this, NewButton);
    DestroyChild(this, HelpButton);
    DestroyChild(this, FrameCtrl);
    DestroyChild(this, Array);
    NCWnd::OnDestroy();
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x10090820, NCMacroListWnd, OnDestroy)

// 0x1008c810 (vslot 55) - the background, the two icons, the counter "(n/24)" at the right and the heading
INT NCMacroListWnd::OnPaint(NCGDevice* Device)
{
    guard(NCMacroListWnd::OnPaint);
    Device->SetClip(ScreenX - 1, ScreenY - 1, Width + 2, Height + 3);
    Device->BeginPaint(ScreenX, ScreenY);
    if (BackTex)
        Device->DrawStretchedTex(0, 21, 240, 344, 0, 0, 240, 344, BackTex);
    if (EditTex)
        Device->DrawTexture(EditX, EditY, 32, 32, 0, 0, 32, 32, EditTex);
    if (DeleteTex)
        Device->DrawTexture(DeleteX, DeleteY, 32, 32, 0, 0, 32, 32, DeleteTex);
    TCHAR Count[14];
    appSprintf(Count, Macros.Num() < 10 ? TEXT("(0%d/%d)") : TEXT("(%d/%d)"), Macros.Num(), NMACRO_MaxMacros);
    Device->DrawText(Width - 50, 25, 0xFFFFFFFF, Count, 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
    Device->DrawText(28, 25, 0xFFFFFFFF, *SysString(STR_Macro), 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
    Device->EndPaint();
    Device->ReleaseClip();
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x1008c810, NCMacroListWnd, OnPaint)

// 0x1008ca10 (vslot 67) - New: the editor opens with empty boxes for the next slot (message 0x31D when all 24 are used)
INT NCMacroListWnd::OnNew()
{
    if (GConsole != NULL && GConsole->Desktop != NULL)
    {
        NCMacroEditWnd* Edit = nw::Field<NCMacroEditWnd*>(GConsole->Desktop, 0xFC);
        if (Edit != NULL && Edit->EditList != NULL)
        {
            NCMacroEditList* EditList = Edit->EditList;
            if (Edit->NameEdit != NULL)
                Edit->NameEdit->ClearText();
            Selected = Macros.Num();
            if (Macros.Num() >= NMACRO_MaxMacros)
            {
                GConsole->ShowSystemMessage(MSG_MacroListFull, COLOR_SystemMsg);
                return 1;
            }
            for (INT i = 0; i < NMACRO_EditLines; i++)
            {
                if (EditList->Edits(i) != NULL)
                    EditList->Edits(i)->ClearText();
            }
            GDesktop->ToggleMacroEdit();   // NCConsole: the macro editor shown with its sound
        }
    }
    return 1;
}
NW_HOOK_METHOD(0x1008ca10, NCMacroListWnd, OnNew)

NW_CHECK_VTABLE(NCMacroListWnd, 0x1011b078, 69)
