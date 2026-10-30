/*=============================================================================
    NCActionItemWnd.cpp - a grid of action icons. Stock: 0x10033990 .. 0x10033d95 (but for
    the accessors the stock code shares with other structs), 0x100343e0 .. 0x10034d09,
    0x100368b0 .. 0x10036c64, 0x10036f00 .. 0x1003705f.
=============================================================================*/
#include "nwindow/NCActionItemWnd.h"
#include "nwindow/NCEditBox.h"
#include "nwindow/NCMacroEditList.h"
#include "nwindow/NCMacroEditWnd.h"
#include "nwindow/NCShortCutWnd.h"
#include "nwindow/NItemInfo.h"
#include "nwindow/NCChatWnd.h"
#include "nwindow/NCConsole.h"
#include "nwindow/NCGDevice.h"
#include "nwindow/NConsoleWnd.h"
#include "nwindow/NClasses.gen.h"

#if NW_FEATURES
// phase 6: ManufactureCmd (NWindow_C1 src/inject/ManufactureCmd.cpp)
#include "features/ManufactureCmd.h"
#endif

enum { SOUND_PickUp = 3 };   // the interface sound of picking an icon up (name ours)

// the "/" a macro line made from an action starts with: this unit's pointer to the string at 0x10107e78 (name ours)
#if NW_HOOK_MODE
#define GActionItemCmdPrefix NW_GLOBAL(const TCHAR*, 0x1016462c)
#else
extern const TCHAR* GActionItemCmdPrefix;
#endif

// the chat window
static NCChatWnd* ChatWnd()
{
    return GDesktop->ChatWnd;
}

/*-----------------------------------------------------------------------------
    NActionItem.
-----------------------------------------------------------------------------*/

// 0x10043430 - an entry of the C2 layout built by the default constructor, then the texts and the numbers the action
// window uses copied from Info (the name, the icon's name, the description; the icon, the parameter and the action id)
NActionItem::NActionItem(const NItemInfo& Info)
{
    ActionID = -1;
    Param = -1;
    Icon = NULL;
    Name[0] = 0;
    Description[0] = 0;
    bSelected = 0;
    ActionID = Info.ID;
    Param = Info.Param;
    Icon = Info.Icon;
    appStrcpy(Name, Info.Name);
    appStrcpy(IconName, Info.IconName);
    appStrcpy(Description, Info.Description);
}
NW_HOOK_CTOR_ARGS(0x10043430, NActionItem, (const NItemInfo& A), (A))

// 0x10040190 (its callers, UseAction and OnLButtonUp, are ours)
INT NActionItem::GetActionID()
{
    return ActionID;
}
NW_HOOK_METHOD(0x10040190, NActionItem, GetActionID)

/*-----------------------------------------------------------------------------
    NCActionItemWnd.
-----------------------------------------------------------------------------*/

// 0x10043500 - the tool tip textures are left as allocated (OnCreate loads them)
NCActionItemWnd::NCActionItemWnd(INT InCols, INT InRows, INT InMaxItems, INT InIconSize)
{
    Cols = InCols;
    MaxItems = InMaxItems;
    Rows = InRows;
    PressedIndex = -1;
    HoverIndex = -1;
    ItemClickTex = NULL;
    ToolTipTex = NULL;
    IconSize = InIconSize;
    bDragging = 0;
    bPressed = 0;
    IconBackTex = FindTexture(TEXT("l2ui.nwindow.icon_back"));
    IconClickTex = FindTexture(TEXT("l2ui.nwindow.icon_click"));
}
NW_HOOK_CTOR_ARGS(0x10043500, NCActionItemWnd, (INT A, INT B, INT C, INT D), (A, B, C, D))

// 0x10043600
NW_HOOK_DTOR(0x10043600, NCActionItemWnd)

// stock body shared with NControl's (0x10004e90)
INT NCActionItemWnd::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (Msg != WM_PAINT && !IsEnabled())
        return 0;
    return NCWnd::WindowProc(Msg, wParam, lParam) != 0;
}

// 0x100401a0 (vslot 36) - C2 loads two textures (the tip box ones of C1 are gone: the skin draws the tool tip)
INT NCActionItemWnd::OnCreate()
{
    NCWnd::OnCreate();
    ToolTipTex = FindTexture(TEXT("L2UI.tooltip.mini-tool01"));
    ItemClickTex = FindTexture(TEXT("l2ui.nwindow.item_click"));
    return 1;
}
NW_HOOK_METHOD(0x100401a0, NCActionItemWnd, OnCreate)

// 0x10043de0 (vslot 37) - the entries go (each one's tool tip object, then the block), then the array and the window
INT NCActionItemWnd::OnDestroy()
{
    guard(NCActionItemWnd::OnDestroy);
    for (INT i = 0; i < Items.Num(); i++)
    {
        if (Items(i))
            delete Items(i);
    }
    Items.Empty();
    NCWnd::OnDestroy();
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x10043de0, NCActionItemWnd, OnDestroy)

// 0x10040960 (vslot 55) - icons IconSize + 3 apart (HitTest counts IconSize + 4), each in its frame; the pressed one
// darkened. The clip is set before the origin, as in the stock code.
INT NCActionItemWnd::OnPaint(NCGDevice* Device)
{
    Device->SetClip(ScreenX - 1, ScreenY - 1, Width + 3, Height + 2);
    Device->BeginPaint(ScreenX, ScreenY);
    INT X = 0;
    INT Y = 1;
    for (INT i = 0; i < Items.Num(); i++)
    {
        NActionItem* Item = Items(i);
        if (!Item)
            continue;
        if (i % Cols == 0)
        {
            X = 1;
            if (i)
                Y += IconSize + 3;
        }
        else
            X += IconSize + 3;
        if (Item->GetIcon())
        {
            Device->DrawTexture(X, Y, IconSize, IconSize, 0, 0, IconSize, IconSize, Item->GetIcon());
            Device->DrawTexture(X - 1, Y - 1, 34, 34, 0, 0, 34, 34, IconBackTex);
            if (PressedIndex == i && bPressed)
                Device->DrawTexture(X - 1, Y - 1, 34, 34, 0, 0, 34, 34, IconClickTex);
        }
    }
    Device->EndPaint();
    Device->ReleaseClip();
    return 1;
}
NW_HOOK_METHOD(0x10040960, NCActionItemWnd, OnPaint)

// 0x10043c50 (vslot 56) - C2 draws the tool tip with the window's skin for the icon under the mouse (its tool tip object
// by value, the cell's position - the cells counted IconSize + 4 across and IconSize + 1 down here - and the window's
// rectangle; NToolTipSkin::DrawFull (0x10029fa0) moves the box to keep it on the screen)
INT NCActionItemWnd::OnDrawToolTip(NCGDevice* Device)
{
    Device->BeginPaint(ScreenX, ScreenY);
    Device->SetClip(ScreenX, ScreenY, Width, Height);
    if (HoverIndex >= 0 && HoverIndex < Items.Num())
    {
        INT Y = (IconSize + 1) * (HoverIndex / Cols);
        INT X = (IconSize + 4) * (HoverIndex % Cols);
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
NW_HOOK_METHOD(0x10043c50, NCActionItemWnd, OnDrawToolTip)

// 0x10040ae0 (vslot 60) - the icon under the mouse; while an icon is carried the cursor is its icon and, new in C2,
// an edit line of the macro editor (the window the desktop keeps at +0xFC) under the mouse is marked (+0x1D8 of the
// edit box); anywhere else the editor is told to unmark (0x1008DE20). (NCMacroEditWnd::ClearMarks; HitLine,
// 0x1008D7D0, is the line under a point.)
INT NCActionItemWnd::OnMouseMove(WPARAM wParam, LPARAM lParam)
{
    INT X = LOWORD(lParam);
    INT Y = HIWORD(lParam);
    HoverIndex = HitTest(X, Y);
    if (IsFocused())
    {
        if (!bDragging)
        {
            if (!GConsole->IsCtrlDown())
            {
                UnlockCursor();
                SetCursor(0, NULL);
            }
        }
        else
        {
            if (PressedIndex >= 0 && PressedIndex < Items.Num())
            {
                SetCursor(4, Items(PressedIndex)->GetIcon());
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
                        if (Line < 0)
                            return 1;
                        Lines->Edits(Line)->Unknown1D8 = 1;
                        return 1;
                    }
                    NCWnd* Target = Editor;
                    if (Desk)
                        Target = Desk->NewWndFC;
                    if (!Target)
                        return 1;
                    ((NCMacroEditWnd*)Target)->ClearMarks();
                    return 1;
                }
            }
            if (GConsole && GDesktop && GDesktop->NewWndFC)
            {
                ((NCMacroEditWnd*)GDesktop->NewWndFC)->ClearMarks();
                return 1;
            }
        }
    }
    return 1;
}
NW_HOOK_METHOD(0x10040ae0, NCActionItemWnd, OnMouseMove)

// 0x10042f50 (vslot 61) - the icon under the mouse is picked: the others are unselected, it is selected and carried
INT NCActionItemWnd::OnLButtonDown(WPARAM wParam, LPARAM lParam)
{
    INT Index = HitTest(LOWORD(lParam), HIWORD(lParam));
    PressedIndex = Index;
    HoverIndex = Index;
    if (Index >= 0 && Index < Items.Num())
    {
        ClearSelection();
        Items(Index)->Select();
        bDragging = 1;
        bPressed = 1;
        PlayInterfaceSound(SOUND_PickUp);
    }
    return 1;
}
NW_HOOK_METHOD(0x10042f50, NCActionItemWnd, OnLButtonDown)

// 0x10042fd0 (vslot 62) - released on the icon pressed: the action runs; dropped on a slot of the shortcut bar: the
// server registers it there (type 3, the action id and its kind, 1 for the first fifteen, 2 for the pet and summon
// ones up to 27, none after); new in C2, dropped on a line of the macro editor (the window the desktop keeps at +0xFC)
// the action's command ("/<command>", from the game data) is written there when it fits in 32 characters, and the actions
// a macro cannot do (the ids 0x20, 0x24, 0x27, 0x29..0x30 and 1000) tell so in the chat instead; the editor is told the
// line (0x1008DED0). (NCMacroEditList::HitLine,
// 0x1008D7D0, finds the line under a point; ClearMark is NCMacroEditWnd's.)
INT NCActionItemWnd::OnLButtonUp(WPARAM wParam, LPARAM lParam)
{
    bPressed = 0;
    if (!GConsole->IsCtrlDown())
    {
        UnlockCursor();
        SetCursor(0, NULL);
    }
    INT Pressed = PressedIndex;
    if (Pressed >= 0 && Pressed < Items.Num())
    {
        INT X = LOWORD(lParam);
        INT Y = HIWORD(lParam);
        if (HitTest(X, Y) == Pressed)
            UseAction();
        else if (bDragging && Items(Pressed) != NULL)
        {
            NWndEntry* Found = FindWindowAt(X, Y);
            if (Found && Found->Wnd)
            {
                NCWnd* Wnd = Found->Wnd;
                if (Wnd->IsA(NClass::NCShortCutWnd()))
                {
                    // the bar's slot under the point (by the bar's layout), -1 none
                    INT Slot = ((NCShortCutWnd*)Wnd)->SlotAtPoint(X, Y);
                    if (Slot >= 0)
                    {
                        L2ParamStack Params(10);
                        Params.PushBack((void*)3);  // an action
                        Params.PushBack((void*)(INT_PTR)Slot);
                        Params.PushBack((void*)(INT_PTR)Items(Pressed)->GetActionID());
                        if (Items(Pressed)->GetActionID() >= 0 && Items(Pressed)->GetActionID() <= 14)
                            Params.PushBack((void*)1);
                        else if (Items(Pressed)->GetActionID() > 14 && Items(Pressed)->GetActionID() < 28)
                            Params.PushBack((void*)2);
                        GConsole->RequestShortCutReg(Params);
                    }
                }
                else if (Wnd->IsA(NClass::NCEditBox()))
                {
                    // the edit line's grandparent: the macro editor
                    NCWnd* Editor = NULL;
                    if (Found->Parent && Found->Parent->Parent)
                        Editor = Found->Parent->Parent->Wnd;
                    INT Line = -1;
                    if (Editor && GDesktop && GDesktop->NewWndFC == Editor)
                        Line = ((NCMacroEditWnd*)Editor)->EditList->HitLine(X, Y);
                    if (Line >= 0)
                    {
                        INT ActionID = Items(Pressed)->GetActionID();
                        if (ActionID == 0x24 || ActionID == 0x27 || ActionID == 0x20 || ActionID == 1000 ||
                            (ActionID > 0x28 && ActionID < 0x31))
                        {
                            // a macro cannot do it (systemmsg 1257)
                            GDesktop->ChatWnd->AddChatMessage(*GL2GameData.SystemMsg[1257].Message, 0xFFB09B79, 5, 0);
                        }
                        else
                        {
                            TCHAR Command[1024];
                            if (ActionID < 0)
                                appMemset(Command, 0, 0x400);   // 0x400 bytes of the 2 KB buffer, as in the stock code
                            else
                                appSprintf(Command, TEXT("%s%s"), GActionItemCmdPrefix,
                                           GL2GameData.GetCmdStringByActionID(ActionID));
                            if (appStrlen(Command) <= 32)
                            {
                                NCEditBox* Edit = ((NCMacroEditWnd*)Editor)->EditList->Edits(Line);
                                Edit->ClearText();
                                if (Command[0])
                                    Edit->InsertText(Command, 1);
                            }
                        }
                        ((NCMacroEditWnd*)Editor)->ClearMark(Line);
                    }
                }
            }
        }
        ClearSelection();
        PressedIndex = -1;
    }
    bDragging = 0;
    return 1;
}
NW_HOOK_METHOD(0x10042fd0, NCActionItemWnd, OnLButtonUp)

// 0x10040c70 (vslot 67) - the pressed icon's action runs (C2: the console's UseAction takes two more arguments, 0)
void NCActionItemWnd::UseAction()
{
    NActionItem* Item = Items(PressedIndex);
    if (Item)
        GConsole->UseAction(Item->GetActionID(), 0, 0);   // 0x1006b0b0
}
NW_HOOK_METHOD(0x10040c70, NCActionItemWnd, UseAction)

// 0x100401e0 - the icon under a screen point (cells IconSize + 4 wide)
INT NCActionItemWnd::HitTest(INT X, INT Y)
{
    INT RX = X - ScreenX;
    INT RY = Y - ScreenY;
    if (RX < 0 || RX > Cols * (IconSize + 4) - 1 || RY < 0 || RY > Rows * (IconSize + 4) - 1)
        return -1;
    INT Col = RX / (IconSize + 4);
    if (Col >= Cols)
        Col--;
    INT Row = RY / (IconSize + 4);
    if (Row >= Rows)
        Row--;
    return Cols * Row + Col;
}
NW_HOOK_METHOD(0x100401e0, NCActionItemWnd, HitTest)

// 0x10040ca0
void NCActionItemWnd::ClearSelection()
{
    for (INT i = 0; i < Items.Num(); i++)
        Items(i)->Unselect();
}
NW_HOOK_METHOD(0x10040ca0, NCActionItemWnd, ClearSelection)

// 0x10043660 - a new entry for Info (none when the grid is full), its tool tip filled with the names only (flags 8)
INT NCActionItemWnd::AddItem(NItemInfo* Info)
{
    if (MaxItems <= Items.Num())
        return 0;
    NActionItem* Item = new NActionItem(*Info);
    Item->FillToolTip(8, 0);
    Items.AddItem(Item);
    return 1;
}
NW_HOOK_METHOD(0x10043660, NCActionItemWnd, AddItem)

// 0x100d8950 - the entries deleted and the array emptied. The stock body is one with NCAbnormalStatusWnd::ClearStatus
// (hooked there, which keeps the address)
void NCActionItemWnd::ClearItems()
{
    for (INT i = 0; i < Items.Num(); i++)
    {
        if (Items(i))
            delete Items(i);
    }
    Items.Empty();
}

NW_CHECK_VTABLE(NCActionItemWnd, 0x10112020, 68)
