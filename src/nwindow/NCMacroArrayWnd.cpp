/*=============================================================================
    NCMacroArrayWnd.cpp - the grid of the macros of NCMacroListWnd.
    Stock: 0x1008B630 (OnDestroy), 0x1008CAD0 .. 0x1008D24F (the cell under a point, OnPaint, the mouse, the
    delete's answer), 0x1008D250 .. 0x1008D784 (OnLButtonUp), 0x1008D790 (UnselectAll), 0x1008FA80 .. 0x1008FB7D
    (constructors, destructor), 0x10090990 (the tool tip).
    WindowProc (0x100BEC50) and OnCreate (0x100C46E0) are bodies other classes keep the addresses of.
=============================================================================*/
#include "nwindow/NCMacroArrayWnd.h"
#include "nwindow/NCMacroEditList.h"
#include "nwindow/NCMacroEditWnd.h"
#include "nwindow/NCMacroListWnd.h"
#include "nwindow/NCShortCutWnd.h"
#include "nwindow/NCConsole.h"
#include "nwindow/NCEditBox.h"
#include "nwindow/NCGDevice.h"
#include "nwindow/NDialogInfo.h"
#include "nwindow/NConsoleWnd.h"
#include "nwindow/NClasses.gen.h"

enum
{
    MSG_DeleteMacro = 0x33C,           // systemmsg (name ours): "Do you want to delete <macro>?"
    MACRO_CellPitch = 3,               // px between two cells
};

// the commands' names the lines of a macro are written with (GL2GameData's table of command names at +0x54188, indexed
// by the C2 command codes of NCommandParser::GetCommand; names ours)
enum
{
    CMD_UseSkill = 0x19,
    CMD_UseShortCut = 0x1C,
    CMD_Delay = 0x4D,
};

// the "/" every command starts with (a pointer to it at 0x1017549C; name ours)
#if NW_HOOK_MODE
#define GCommandSlash NW_GLOBAL(const TCHAR*, 0x1017549C)
#else
extern const TCHAR* GCommandSlash;
#endif

// the desktop's macro windows (NCConsole.h keeps them as NCWnd*: +0xF8 the list, +0xFC the editor; names ours)
static NCMacroEditWnd* DesktopEdit()
{
    return nw::Field<NCMacroEditWnd*>(GConsole->Desktop, 0xFC);
}

/*-----------------------------------------------------------------------------
    Constructors and destructor.
-----------------------------------------------------------------------------*/

// 0x1008fa80
NCMacroArrayWnd::NCMacroArrayWnd()
{
}
NW_HOOK_CTOR(0x1008fa80, NCMacroArrayWnd)

// 0x1008fb40 - the grid's size is the NCItemWnd's arguments (the list window asks for 6 x 7 cells of 32, 80 at most)
NCMacroArrayWnd::NCMacroArrayWnd(INT InColumns, INT InVisibleRows, INT InMaxItems, INT InCellSize)
    : NCItemWnd(InColumns, InVisibleRows, InMaxItems, InCellSize)
{
}
NW_HOOK_CTOR_ARGS(0x1008fb40, NCMacroArrayWnd, (INT A, INT B, INT C, INT D), (A, B, C, D))

// 0x1008fae0
NW_HOOK_DTOR(0x1008fae0, NCMacroArrayWnd)

// 0x1008b630 (vslot 37)
INT NCMacroArrayWnd::OnDestroy()
{
    guard(NCMacroArrayWnd::OnDestroy);
    NCItemWnd::OnDestroy();
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x1008b630, NCMacroArrayWnd, OnDestroy)

/*-----------------------------------------------------------------------------
    The cells.
-----------------------------------------------------------------------------*/

// 0x1008cad0 - the cell under (X, Y): the grid is 6 cells wide and 7 high (42), 35 pixels apart, each 32 square
INT NCMacroArrayWnd::HitCell(INT X, INT Y)
{
    for (INT i = 0; i < 42; i++)
    {
        INT CellX = ScreenX;
        INT CellY = ScreenY;
        switch (i)
        {
            case 0:  break;
            case 1: CellX += 0x23; break;
            case 2: CellX += 0x46; break;
            case 3: CellX += 0x69; break;
            case 4: CellX += 0x8C; break;
            case 5: CellX += 0xAF; break;
            case 6: CellY += 0x23; break;
            case 7: CellX += 0x23; CellY += 0x23; break;
            case 8: CellX += 0x46; CellY += 0x23; break;
            case 9: CellX += 0x69; CellY += 0x23; break;
            case 10: CellX += 0x8C; CellY += 0x23; break;
            case 11: CellX += 0xAF; CellY += 0x23; break;
            case 12: CellY += 0x46; break;
            case 13: CellX += 0x23; CellY += 0x46; break;
            case 14: CellX += 0x46; CellY += 0x46; break;
            case 15: CellX += 0x69; CellY += 0x46; break;
            case 16: CellX += 0x8C; CellY += 0x46; break;
            case 17: CellX += 0xAF; CellY += 0x46; break;
            case 18: CellY += 0x69; break;
            case 19: CellX += 0x23; CellY += 0x69; break;
            case 20: CellX += 0x46; CellY += 0x69; break;
            case 21: CellX += 0x69; CellY += 0x69; break;
            case 22: CellX += 0x8C; CellY += 0x69; break;
            case 23: CellX += 0xAF; CellY += 0x69; break;
            case 24: CellY += 0x8C; break;
            case 25: CellX += 0x23; CellY += 0x8C; break;
            case 26: CellX += 0x46; CellY += 0x8C; break;
            case 27: CellX += 0x69; CellY += 0x8C; break;
            case 28: CellX += 0x8C; CellY += 0x8C; break;
            case 29: CellX += 0xAF; CellY += 0x8C; break;
            case 30: CellY += 0xAF; break;
            case 31: CellX += 0x23; CellY += 0xAF; break;
            case 32: CellX += 0x46; CellY += 0xAF; break;
            case 33: CellX += 0x69; CellY += 0xAF; break;
            case 34: CellX += 0x8C; CellY += 0xAF; break;
            case 35: CellX += 0xAF; CellY += 0xAF; break;
            case 36: CellY += 0xD2; break;
            case 37: CellX += 0x23; CellY += 0xD2; break;
            case 38: CellX += 0x46; CellY += 0xD2; break;
            case 39: CellX += 0x69; CellY += 0xD2; break;
            case 40: CellX += 0x8C; CellY += 0xD2; break;
            case 41: CellX += 0xAF; CellY += 0xD2; break;
        }
        if (PtInRect(X, Y, CellX, CellY, 32, 32))
            return i;
    }
    return -1;
}
NW_HOOK_METHOD(0x1008cad0, NCMacroArrayWnd, HitCell)

// 0x1008cd70 (vslot 55) - each macro's icon in its cell (as many rows as fit) and its short name over it, two characters
// to a line
INT NCMacroArrayWnd::OnPaint(NCGDevice* Device)
{
    guard(NCMacroArrayWnd::OnPaint);
    Device->BeginPaint(ScreenX, ScreenY);
    Device->SetClip(ScreenX, ScreenY, Width, Height);
    NCMacroListWnd* List = (NCMacroListWnd*)Entry->Parent->Wnd;
    INT X = 0;
    INT Y = 0;
    for (INT i = 0; i < List->Macros.Num(); i++)
    {
        if (List->Macros(i) != NULL)
        {
            if (i % Columns == 0)
            {
                if (i != TopRow * Columns)
                    Y += MACRO_CellPitch + CellSize;
                INT Limit = (CellSize + 1) * VisibleRows;
                X = 0;
                if (Limit - Y == 0 || Limit < Y)
                    break;
            }
            else
                X += MACRO_CellPitch + CellSize;
            if (List->Macros(i)->GetIconTex() != NULL)
                Device->DrawTexture(X, Y, CellSize, CellSize, 0, 0, CellSize, CellSize, List->Macros(i)->GetIconTex());
            const TCHAR* ShortName = List->Macros(i)->GetShortName();
            if (ShortName != NULL)
            {
                INT TextX = X + 4;
                INT TextY = Y + 4;
                FString Text(ShortName);
                for (INT k = 0; k < Text.Len(); k++)
                {
                    Device->DrawText(TextX, TextY, 0xFFFFFFFF, *Text.Mid(k, 1), 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
                    if ((k & 1) == 0)
                        TextX += 13;
                    else
                    {
                        TextY += 13;
                        TextX -= 13;
                    }
                }
            }
        }
    }
    Device->EndPaint();
    Device->ReleaseClip();
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x1008cd70, NCMacroArrayWnd, OnPaint)

// 0x10090990 (vslot 56, named OnPostPaint in the database) - the tool tip of the macro under the mouse: the full box, at
// the cell's position
INT NCMacroArrayWnd::OnDrawToolTip(NCGDevice* Device)
{
    guard(NCMacroArrayWnd::OnPostPaint);
    Device->BeginPaint(ScreenX, ScreenY);
    Device->SetClip(ScreenX, ScreenY, Width, Height);
    NCMacroListWnd* List = (NCMacroListWnd*)Entry->Parent->Wnd;
    if (List == NULL)
        return 0;
    if (HoverIndex >= 0 && HoverIndex < List->Macros.Num())
    {
        NMacroEntry* Macro = List->Macros(HoverIndex);
        if (Macro == NULL)
            return 0;
        INT Index = HoverIndex - TopRow * Columns;
        INT Y = (CellSize + 1) * (Index / Columns);
        INT X = (CellSize + 1) * (Index % Columns);
        if (ToolTipSkin)
        {
            NToolTipRect Wnd;
            Wnd.X = ScreenX;
            Wnd.Y = ScreenY;
            Wnd.W = Width;
            Wnd.H = Height;
            ToolTipSkin->DrawFull(Device, Macro->ToolTip, &X, &Y, Wnd);
        }
    }
    Device->ReleaseClip();
    Device->EndPaint();
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x10090990, NCMacroArrayWnd, OnDrawToolTip)

/*-----------------------------------------------------------------------------
    The mouse.
-----------------------------------------------------------------------------*/

// 0x1008cfc0 (vslot 61) - the cell under the mouse is selected (the other macros are not) and picked up
INT NCMacroArrayWnd::OnLButtonDown(WPARAM wParam, LPARAM lParam)
{
    if (Entry->Parent == NULL || Entry->Parent->Wnd == NULL)
        return 0;
    NCMacroListWnd* List = (NCMacroListWnd*)Entry->Parent->Wnd;
    INT Cell = HitCell(LOWORD(lParam), HIWORD(lParam));
    Selected = Cell;
    HoverIndex = Cell;
    if (Cell >= 0 && Cell < List->Macros.Num())
    {
        UnselectAll();
        List->Macros(Selected)->Select();
        bDragging = 1;
        return 1;
    }
    return 1;
}
NW_HOOK_METHOD(0x1008cfc0, NCMacroArrayWnd, OnLButtonDown)

// 0x1008d050 (vslot 60) - while the selected macro is carried the cursor is its icon; else the normal one (unless Ctrl is held)
INT NCMacroArrayWnd::OnMouseMove(WPARAM wParam, LPARAM lParam)
{
    HoverIndex = HitCell(LOWORD(lParam), HIWORD(lParam));
    if (Entry->Parent == NULL || Entry->Parent->Wnd == NULL)
        return 0;
    NCMacroListWnd* List = (NCMacroListWnd*)Entry->Parent->Wnd;
    if (Selected < 0 || Selected >= List->Macros.Num())
        bDragging = 0;
    else if (IsFocused())
    {
        if (bDragging)
        {
            if (Selected >= 0 && Selected < List->Macros.Num())
            {
                SetCursor(4, List->Macros(Selected)->GetIconTex());
                LockCursor();
                return 1;
            }
        }
        else if (!GConsole->IsCtrlDown())
        {
            UnlockCursor();
            SetCursor(0, NULL);
            return 1;
        }
    }
    return 1;
}
NW_HOOK_METHOD(0x1008d050, NCMacroArrayWnd, OnMouseMove)

// 0x1008d790 (vslot 71) - no macro selected
void NCMacroArrayWnd::UnselectAll()
{
    if (Entry->Parent != NULL && Entry->Parent->Wnd != NULL)
    {
        NCMacroListWnd* List = (NCMacroListWnd*)Entry->Parent->Wnd;
        for (INT i = 0; i < List->Macros.Num(); i++)
            List->Macros(i)->Deselect();
    }
}
NW_HOOK_METHOD(0x1008d790, NCMacroArrayWnd, UnselectAll)

// 0x1008d250 (vslot 62) - the carried macro dropped: on its own cell it runs; on the list window's delete icon the
// question "delete?" comes up (the answer is OnDialogResult); on the edit icon the editor opens with the macro's lines
// written back as the commands they stand for; on a shortcut slot the macro goes there
INT NCMacroArrayWnd::OnLButtonUp(WPARAM wParam, LPARAM lParam)
{
    INT X = LOWORD(lParam);
    INT Y = HIWORD(lParam);
    if (!GConsole->IsCtrlDown())
    {
        UnlockCursor();
        SetCursor(0, NULL);
    }
    if (bDragging == 0)
        return 0;
    bDragging = 0;
    if (Entry->Parent == NULL || Entry->Parent->Wnd == NULL)
        return 0;
    NCMacroListWnd* List = (NCMacroListWnd*)Entry->Parent->Wnd;
    INT Cell = HitCell(X, Y);
    INT Sel = Selected;
    if (Cell == Sel)
    {
        NMacroEntry* Macro = List->Macros(Sel);
        if (Macro != NULL)
            Macro->bRunning = 1;
    }
    else if (List->HitDelete(X, Y))
    {
        if (Sel >= 0 && Sel < List->Macros.Num())
        {
            L2ParamStack Unused(10);
            NDialogInfo Info;
            Info.Owner = Entry;
            Info.ID = DIALOG_MACRO_DELETE;
            Info.bMainBox = 1;
            Info.bInput = 0;
            Info.SetSystemMessage(MSG_DeleteMacro);
            Info.SetButtons(List->Macros(Selected)->Name, NULL);
            Info.bLargeAmount = 0;
            GConsole->ShowDialog(&Info);
        }
    }
    else if (List->HitEdit(X, Y))
    {
        if (GConsole != NULL && GConsole->Desktop != NULL)
        {
            NCMacroEditWnd* Edit = DesktopEdit();
            if (Edit != NULL)
            {
                NCMacroEditList* EditList = Edit->EditList;
                if (EditList != NULL)
                {
                    for (INT i = 0; i < NMACRO_EditLines; i++)
                    {
                        if (EditList->Edits(i) != NULL)
                            EditList->Edits(i)->ClearText();
                    }
                    List->Selected = Selected;
                    if (Edit->NameEdit != NULL)
                    {
                        Edit->NameEdit->ClearText();
                        Edit->NameEdit->InsertText(List->Macros(Selected)->Name, 1);
                    }
                    NMacroEntry* Macro = List->Macros(Selected);
                    if (Macro != NULL)
                    {
                        for (INT i = 0; i < List->Macros(Selected)->NumLines; i++)
                        {
                            TCHAR Text[3072];
                            appMemset(Text, 0, sizeof(Text));
                            if (EditList->Edits(i) != NULL)
                            {
                                NMacroLine* Line = &List->Macros(Selected)->Lines[i];
                                switch (Line->Type)
                                {
                                    case NMLT_Skill:
                                        appSprintf(Text, TEXT("%s%s %s"), GCommandSlash,
                                                   *GL2GameData.Unknown54188[CMD_UseSkill],
                                                   GConsole->GetSkillName(Line->Param08, Line->Param10));
                                        break;
                                    case NMLT_Action:
                                        appSprintf(Text, TEXT("%s%s"), GCommandSlash,
                                                   GConsole->GetActionName(Line->Param08));   // 0x10060c90
                                        break;
                                    case NMLT_ShortCut:
                                        appSprintf(Text, TEXT("%s%s %d %d"), GCommandSlash,
                                                   *GL2GameData.Unknown54188[CMD_UseShortCut], Line->Param0C + 1,
                                                   Line->Param08 + 1);
                                        break;
                                    case NMLT_Command:
                                        appSprintf(Text, TEXT("%s"), Line->Text);
                                        break;
                                    case NMLT_Delay:
                                        appSprintf(Text, TEXT("%s%s %d"), GCommandSlash,
                                                   *GL2GameData.Unknown54188[CMD_Delay], Line->Param0C);
                                        break;
                                }
                                EditList->Edits(i)->InsertText(Text, 1);
                            }
                        }
                    }
                    if (!DesktopEdit()->IsShowWindow())
                        GDesktop->ToggleMacroEdit();   // NCConsole: the macro editor shown with its sound
                }
            }
        }
    }
    else
    {
        NWndEntry* Found = FindWindowAt(X, Y);
        if (Found != NULL && Found->Wnd != NULL && Found->Wnd->IsA(NClass::NCShortCutWnd()))
        {
            INT Slot = ((NCShortCutWnd*)Found->Wnd)->SlotAtPoint(X, Y);   // 0x100CCEF0, NCShortCutWnd: the slot under the point
            if (Slot >= 0)
            {
                L2ParamStack Params(10);
                Params.PushBack((void*)4);                                              // a macro
                Params.PushBack((void*)(INT_PTR)Slot);
                Params.PushBack((void*)(INT_PTR)List->Macros(Selected)->ID);
                Params.PushBack((void*)1);
                GConsole->RequestShortCutReg(Params);                                 // 0x10067080, the console's shortcut registration
            }
        }
    }
    return 1;
}
NW_HOOK_METHOD(0x1008d250, NCMacroArrayWnd, OnLButtonUp)

// 0x1008d130 (vslot 73) - the answer to the question: the macro selected is deleted on the server; when the editor is
// open on it, it closes
INT NCMacroArrayWnd::OnDialogResult(WPARAM wParam, LPARAM lParam)
{
    L2ParamStack Params(10);
    if (Entry->Parent == NULL || Entry->Parent->Wnd == NULL)
        return 0;
    NCMacroListWnd* List = (NCMacroListWnd*)Entry->Parent->Wnd;
    Params.PushBack((void*)(INT_PTR)List->Macros(Selected)->ID);
    GConsole->RequestDeleteMacro(Params);
    if (GConsole != NULL && GConsole->Desktop != NULL)
    {
        NCMacroEditWnd* Edit = DesktopEdit();
        NCMacroListWnd* DesktopList = nw::Field<NCMacroListWnd*>(GConsole->Desktop, 0xF8);
        if (Edit != NULL && DesktopList != NULL && Edit->IsShowWindow())
        {
            if (DesktopList->Selected == Selected)
                DesktopEdit()->HideWindow();
        }
    }
    return 1;
}
NW_HOOK_METHOD(0x1008d130, NCMacroArrayWnd, OnDialogResult)

NW_CHECK_VTABLE(NCMacroArrayWnd, 0x1011b190, 74)
