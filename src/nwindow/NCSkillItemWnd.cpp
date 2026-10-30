/*=============================================================================
    NCSkillItemWnd.cpp - the skill grids of the skill windows. Stock: 0x10071fc0 ..
    0x10073155, 0x100737e0 .. 0x10073a32, 0x10073a40 .. 0x10073aeb, 0x10073770 ..
    0x100737d9, 0x1005fe80 .. 0x1005fedf, and NCGMSkillItemWnd's 0x1005bd80 .. 0x1005bdd1,
    0x10060400 .. 0x100604c7.
=============================================================================*/
#include "nwindow/NCSkillItemWnd.h"
#include "nwindow/NCCoolTimeIcon.h"
#include "nwindow/NCGDevice.h"
#include "nwindow/NCConsole.h"
#include "nwindow/NCEditBox.h"
#include "nwindow/NCMacroEditList.h"
#include "nwindow/NCMacroEditWnd.h"
#include "nwindow/NCShortCutWnd.h"
#include "nwindow/NClasses.gen.h"
#include "nwindow/NConsoleWnd.h"

enum { SOUND_CoolTimeEnd = 9 };  // interface sound of a cool time ending (name ours)

// the "/" a macro line made from a skill starts with: this unit's pointer to the string at 0x10107e78 (name ours)
#if NW_HOOK_MODE
#define GSkillItemCmdPrefix NW_GLOBAL(const TCHAR*, 0x101761cc)
#else
extern const TCHAR* GSkillItemCmdPrefix;
#endif

/*-----------------------------------------------------------------------------
    NCSkillItemWnd
-----------------------------------------------------------------------------*/

// 0x10091d40
NCSkillItemWnd::NCSkillItemWnd()
{
    bPressed = 0;
    IconBackTex = FindTexture(TEXT("l2ui.nwindow.icon_back"));
    IconClickTex = FindTexture(TEXT("l2ui.nwindow.icon_click"));
    ToggleTex = FindTexture(TEXT("L2_SkillTime.ToggleEffect.ToggleEffect001"));
}
NW_HOOK_CTOR(0x10091d40, NCSkillItemWnd)

// 0x10091de0 - the skill window's are 6 x 7 icons of 32 pixels, 80 at most
NCSkillItemWnd::NCSkillItemWnd(INT InColumns, INT InVisibleRows, INT InMaxItems, INT InCellSize)
    : NCItemWnd(InColumns, InVisibleRows, InMaxItems, InCellSize)
{
    bPressed = 0;
    IconBackTex = FindTexture(TEXT("l2ui.nwindow.icon_back"));
    IconClickTex = FindTexture(TEXT("l2ui.nwindow.icon_click"));
    ToggleTex = FindTexture(TEXT("L2_SkillTime.ToggleEffect.ToggleEffect001"));
}
NW_HOOK_CTOR_ARGS(0x10091de0, NCSkillItemWnd, (INT A, INT B, INT C, INT D), (A, B, C, D))

// 0x10078360 - CoolTimes' destructor is TArray<NCCoolTimeIcon*>'s (0x1005fce0)
NW_HOOK_DTOR(0x10078360, NCSkillItemWnd)

// 0x10090b10 (vslot 25) - the body is shared with NCRecipeManufactureItemWnd's; no enabled test in C2
INT NCSkillItemWnd::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    return NCItemWnd::WindowProc(Msg, wParam, lParam) != 0;
}
NW_HOOK_METHOD(0x10090b10, NCSkillItemWnd, WindowProc)

// the cool time icons go (repeated inline in the stock OnDestroy and Clear)
static FORCEINLINE void DeleteCoolTimes(TArray<NCCoolTimeIcon*>& CoolTimes)
{
    for (INT i = 0; i < CoolTimes.Num(); i++)
        if (CoolTimes(i))
            delete CoolTimes(i);
    CoolTimes.Empty();
}

// 0x10091b50 (vslot 37)
INT NCSkillItemWnd::OnDestroy()
{
    guard(NCSkillItemWnd::OnDestroy);
    DeleteCoolTimes(CoolTimes);
    NCItemWnd::OnDestroy();
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x10091b50, NCSkillItemWnd, OnDestroy)

// 0x10091e90 (vslot 70) - the items are deleted here and NCItemWnd's finds none left
void NCSkillItemWnd::Clear()
{
    for (INT i = 0; i < Items.Num(); i++)
        if (Items(i))
            delete Items(i);
    Items.Empty();
    DeleteCoolTimes(CoolTimes);
    NCItemWnd::Clear();
}
NW_HOOK_METHOD(0x10091e90, NCSkillItemWnd, Clear)

// 0x10091c20 - from NCSkillWnd::AddSkill; MaxItems is not checked. The skill's cool time icon is added stopped (the
// console starts it).
INT NCSkillItemWnd::AddSkill(NItemInfo* Info)
{
    Info->Icon = FindTexture(Info->IconName);
    if (!Info->Icon)
        Info->Icon = FindTexture(TEXT("NWindow.BlackTexture"));
    NItemEntry* Item = new NItemEntry(Info);
    CoolTimes.AddItem(new NCCoolTimeIcon);
    Item->FillToolTip(4, 0);
    Items.AddItem(Item);
    UpdateScrollRange();
    return 1;
}
NW_HOOK_METHOD(0x10091c20, NCSkillItemWnd, AddSkill)


// "label : " (the label a sysstring, empty when missing) in grey, the colon in white; returns
// the X of the value, past "label : " (repeated inline for each such line of the stock code)
static FORCEINLINE INT DrawLabel(NCGDevice* Device, INT X, INT Y, TCHAR* Text, INT LabelID)
{
    SIZE Size;
    if (*GL2GameData.SysString[LabelID])
        appSprintf(Text, TEXT("%s"), *GL2GameData.SysString[LabelID]);
    else
        Text[0] = 0;
    Device->DrawText(X, Y, 0xFFA3A3A3, Text, 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
    NCGDevice::GetTextExtent(&Size, Text, 0, 0, 0, 0, 0);
    appSprintf(Text, TEXT(" : "));
    Device->DrawText(X + Size.cx, Y, 0xFFFFFFFF, Text, 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
    if (*GL2GameData.SysString[LabelID])
        appSprintf(Text, TEXT("%s : "), *GL2GameData.SysString[LabelID]);
    else
        Text[0] = 0;
    NCGDevice::GetTextExtent(&Size, Text, 0, 0, 0, 0, 0);
    return X + Size.cx;
}


// 0x10090bb0 (vslot 55) - NCItemWnd's grid (its guard name too), with the selection frame the size of the icon and over
// each skill, in order: the icon frame of an active skill (NItemEntry Type 0) and the click frame of the pressed one,
// the toggle animation of a toggle skill whose effect is on the player, the end flash with its sound once, the cool time
INT NCSkillItemWnd::OnPaint(NCGDevice* Device)
{
    guard(NCItemWnd::OnPaint);
    Device->SetClip(ScreenX - 1, ScreenY - 1, Width + 2, Height + 2);
    Device->BeginPaint(ScreenX, ScreenY);
    INT X = 0;
    INT Y = 0;
    for (INT i = TopRow * Columns; i < Items.Num(); i++)
    {
        NItemEntry* Item = Items(i);
        if (!Item)
            continue;
        if (i % Columns == 0)
        {
            X = 0;
            if (i != TopRow * Columns)
                Y += CellSize + 3;
            if (VisibleRows * (CellSize + 1) <= Y)
                break;
        }
        else
            X += CellSize + 3;
        if (Item->IsSelected())
            Device->DrawTexture(X, Y, CellSize, CellSize, 0, 0, CellSize, CellSize, SelectTex);
        if (Item->GetIcon())
            Device->DrawTexture(X, Y, CellSize, CellSize, 0, 0, CellSize, CellSize, Item->GetIcon());
        if (Item->Type == 0)
        {
            if (IconBackTex)
                Device->DrawTexture(X - 1, Y - 1, 34, 34, 0, 0, 34, 34, IconBackTex);
            if (IconClickTex && Selected == i && bPressed)
                Device->DrawTexture(X - 1, Y - 1, 34, 34, 0, 0, 34, 34, IconClickTex);
        }
        if (GConsole->IsToggleSkill(Item->ID, Item->UnknownC18) && GConsole->HasAbnormal(Item->ID))
            Device->DrawTexture(X, Y, 32, 32, 0, 0, 32, 32, ToggleTex);
        if (CoolTimes(i)->bFlashing)
        {
            Device->DrawBrightenTexture(X, Y, CellSize, CellSize, 0, 0, CellSize, CellSize,
                                        CoolTimes(i)->GetFlashTexture(), 0xFF008080);
            if (CoolTimes(i)->bSoundDue)
            {
                PlayInterfaceSound(SOUND_CoolTimeEnd);
                CoolTimes(i)->bSoundDue = 0;
            }
        }
        if (CoolTimes(i)->bRunning && !CoolTimes(i)->IsIdle() && CoolTimes(i)->GetTexture())
            Device->DrawTexture(X, Y, CellSize, CellSize, 0, 0, CellSize, CellSize, CoolTimes(i)->GetTexture());
    }
    Device->EndPaint();
    Device->ReleaseClip();
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x10090bb0, NCSkillItemWnd, OnPaint)

// 0x10092180 (vslot 56) - C2 draws the tool tip with the window's skin for any item under the mouse (its tool tip
// object by value, the cell's position in the window - the cells counted CellSize + 1 apart here - and the window's
// rectangle; NToolTipSkin::DrawFull (0x10029fa0) moves the box to keep it on the screen)
INT NCSkillItemWnd::OnDrawToolTip(NCGDevice* Device)
{
    Device->BeginPaint(ScreenX, ScreenY);
    Device->SetClip(ScreenX, ScreenY, Width, Height);
    if (HoverIndex >= 0 && HoverIndex < Items.Num())
    {
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
            ToolTipSkin->DrawFull(Device, Items(HoverIndex)->ToolTip, &X, &Y, Wnd);
        }
    }
    Device->ReleaseClip();
    Device->EndPaint();
    return 1;
}
NW_HOOK_METHOD(0x10092180, NCSkillItemWnd, OnDrawToolTip)

// 0x10090b30 (vslot 61)
INT NCSkillItemWnd::OnLButtonDown(WPARAM wParam, LPARAM lParam)
{
    NCItemWnd::OnLButtonDown(wParam, lParam);
    bPressed = 1;
    return 1;
}
NW_HOOK_METHOD(0x10090b30, NCSkillItemWnd, OnLButtonDown)

// 0x10090ee0 (vslot 60) - the icon under the mouse; while a skill is carried the cursor is its icon and, new in C2, an edit
// line of the macro editor (the window the desktop keeps at +0xFC) under the mouse is marked (+0x1D8 of the edit box);
// anywhere else the editor is told to unmark; with nothing carried the cursor goes back unless Ctrl is down
// (NCActionItemWnd::OnMouseMove and NCShortCutWnd::OnMouseMove are the same). The guard string has the original's typo
INT NCSkillItemWnd::OnMouseMove(WPARAM wParam, LPARAM lParam)
{
    guard(NCSKillItemWnd::OnMouseMove);
    INT X = LOWORD(lParam);
    INT Y = HIWORD(lParam);
    HoverIndex = HitTest(X, Y);
    if (IsFocused())
    {
        if (bDragging)
        {
            if (Selected >= 0 && Selected < Items.Num())
            {
                SetCursor(4, Items(Selected)->GetIcon());
                LockCursor();
            }
            NWndEntry* Found = FindWindowAt(X, Y);
            if (Found && Found->Wnd && Found->Wnd->IsA(NClass::NCEditBox()))
            {
                // the edit line's grandparent: the macro editor
                NCWnd* Editor = NULL;
                if (Found->Parent && Found->Parent->Parent)
                    Editor = Found->Parent->Parent->Wnd;
                if (Editor)
                {
                    NCConsole* Desk = GDesktop;
                    if (Desk && Desk->NewWndFC == Editor)
                    {
                        NCMacroEditList* Lines = ((NCMacroEditWnd*)Editor)->EditList;
                        INT Line = Lines->HitLine(X, Y);
                        if (Line >= 0)
                            Lines->Edits(Line)->Unknown1D8 = 1;
                        return 1;
                    }
                    NCWnd* Target = Editor;
                    if (Desk)
                        Target = Desk->NewWndFC;
                    if (Target)
                        ((NCMacroEditWnd*)Target)->ClearMarks();
                    return 1;
                }
            }
            if (GConsole && GDesktop && GDesktop->NewWndFC)
                ((NCMacroEditWnd*)GDesktop->NewWndFC)->ClearMarks();
        }
        else if (!GConsole->IsCtrlDown())
        {
            UnlockCursor();
            SetCursor(0, NULL);
        }
    }
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x10090ee0, NCSkillItemWnd, OnMouseMove)

// 0x100910e0 (vslot 62) - released on the icon clicked: the skill is used (the server gets its id, Ctrl and Shift);
// dropped on a slot of the shortcut bar: the server registers it there (type 2, then 1). C2: dropped on a line of the
// macro window's edit lines (the window the desktop keeps at +0xFC: the macro editor), a skill (Type 0) writes its
// command there - "/<command> <name>" when it fits in 32 characters - and the editor is told the line (it marks it).
// The editor's own functions (0x1008D7D0 the line under a point, 0x1008DED0 the line marked) are not decompiled yet.
INT NCSkillItemWnd::OnLButtonUp(WPARAM wParam, LPARAM lParam)
{
    guard(NCSkillItemWnd::OnLButtonUp);
    bPressed = 0;
    if (!GConsole->IsCtrlDown())
    {
        UnlockCursor();
        SetCursor(0, NULL);
    }
    if (Selected >= 0 && Selected < Items.Num())
    {
        INT X = LOWORD(lParam);
        INT Y = HIWORD(lParam);
        if (HitTest(X, Y) == Selected)
        {
            L2ParamStack Params(10);
            Params.PushBack((void*)(INT_PTR)Items(Selected)->ID);
            Params.PushBack((void*)(INT_PTR)GConsole->IsCtrlDown());
            Params.PushBack(
                (void*)(INT_PTR)GConsole->Engine->Client->Viewports(0)->Input->KeyDown(IK_Shift));
            GConsole->RequestMagicSkillUse(Params);
        }
        else if (bDragging)
        {
            NWndEntry* Target = FindWindowAt(X, Y);
            if (Target && Target->Wnd)
            {
                NCWnd* Wnd = Target->Wnd;
                if (Wnd->IsA(NClass::NCShortCutWnd()))
                {
                    // the bar's slot under the point (by the bar's layout), -1 none
                    INT Slot = ((NCShortCutWnd*)Wnd)->SlotAtPoint(X, Y);
                    if (Slot >= 0)
                    {
                        L2ParamStack Params(10);
                        Params.PushBack((void*)(INT_PTR)SHORTCUT_Skill);
                        Params.PushBack((void*)(INT_PTR)Slot);
                        Params.PushBack((void*)(INT_PTR)Items(Selected)->ID);
                        Params.PushBack((void*)1);
                        GConsole->RequestShortCutReg(Params);
                    }
                }
                else if (Wnd->IsA(NClass::NCEditBox()))
                {
                    // the edit line's grandparent: the macro editor
                    NCWnd* Editor = NULL;
                    if (Target->Parent && Target->Parent->Parent)
                        Editor = Target->Parent->Parent->Wnd;
                    if (Editor && GDesktop->NewWndFC == Editor)
                    {
                        // the editor's lines (a control at +0x9C holds the edit boxes in an array at +0xD0)
                        NCMacroEditList* Lines = ((NCMacroEditWnd*)Editor)->EditList;
                        INT Line = Lines->HitLine(X, Y);
                        if (Line >= 0)
                        {
                            if (Items(Selected)->Type == 0)
                            {
                                TCHAR Command[1028];
                                appSprintf(Command, TEXT("%s%s %s"), GSkillItemCmdPrefix,
                                           *GL2GameData.Unknown54188[25], Items(Selected)->Name);
                                if (appStrlen(Command) <= 32)
                                {
                                    NCEditBox* Edit = Lines->Edits(Line);
                                    Edit->ClearText();
                                    Edit->InsertText(Command, 1);
                                }
                            }
                            ((NCMacroEditWnd*)Editor)->ClearMark(Line);
                        }
                    }
                }
            }
        }
        UnselectAll();
        Selected = -1;
    }
    bDragging = 0;
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x100910e0, NCSkillItemWnd, OnLButtonUp)

NW_CHECK_VTABLE(NCSkillItemWnd, 0x10117b88, 73)

/*-----------------------------------------------------------------------------
    NCGMSkillItemWnd
-----------------------------------------------------------------------------*/

// 0x100788f0
NCGMSkillItemWnd::NCGMSkillItemWnd()
{
}
NW_HOOK_CTOR(0x100788f0, NCGMSkillItemWnd)

// 0x10078990
NCGMSkillItemWnd::NCGMSkillItemWnd(INT InColumns, INT InVisibleRows, INT InMaxItems, INT InCellSize)
    : NCSkillItemWnd(InColumns, InVisibleRows, InMaxItems, InCellSize)
{
}
NW_HOOK_CTOR_ARGS(0x10078990, NCGMSkillItemWnd, (INT A, INT B, INT C, INT D), (A, B, C, D))

// 0x10078940
NW_HOOK_DTOR(0x10078940, NCGMSkillItemWnd)

// 0x10073040 (vslot 25) - NCSkillItemWnd's
INT NCGMSkillItemWnd::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    return NCSkillItemWnd::WindowProc(Msg, wParam, lParam) != 0;
}
NW_HOOK_METHOD(0x10073040, NCGMSkillItemWnd, WindowProc)

// 0x10072f90 (vslot 62) - the carried skill is only put down; also NCGMInvenItemWnd's slots
// 57, 58 and 68
INT NCGMSkillItemWnd::OnLButtonUp(WPARAM wParam, LPARAM lParam)
{
    bDragging = 0;
    return 1;
}
NW_HOOK_METHOD(0x10072f90, NCGMSkillItemWnd, OnLButtonUp)

NW_CHECK_VTABLE(NCGMSkillItemWnd, 0x10117f70, 73)
