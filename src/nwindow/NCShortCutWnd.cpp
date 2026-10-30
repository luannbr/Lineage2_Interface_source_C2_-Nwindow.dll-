/*=============================================================================
    NCShortCutWnd.cpp - the shortcut bar. Stock: 0x100cc700 .. 0x100cd7xx (the slot, the bar's helpers), 0x100cd790 ..
    0x100ce733 (the painters), 0x100ce740 .. 0x100cee39 (OnPaint, the mouse), 0x100cee40 .. 0x100cf4df, 0x100cf500 ..
    0x100cf62b (the slot's constructors), 0x100cf630 .. 0x100cfe85 (OnCreate, OnDestroy, OnDrawToolTip), 0x100cfe90 ..
    0x100d00df (SetShortCut).
=============================================================================*/
#include "nwindow/NCShortCutWnd.h"
#include "nwindow/NCActionItemWnd.h"
#include "nwindow/NCItemWnd.h"
#include "nwindow/NCButton.h"
#include "nwindow/NCChatWnd.h"
#include "nwindow/NCConsole.h"
#include "nwindow/NCPetWnd.h"
#include "nwindow/NCInventoryWnd.h"
#include "nwindow/NCPetInventoryWnd.h"
#include "nwindow/NCFrameCtrl.h"
#include "nwindow/NCGDevice.h"
#include "nwindow/NConsoleWnd.h"
#include "nwindow/NCEditBox.h"
#include "nwindow/NCMacroEditList.h"
#include "nwindow/NCMacroEditWnd.h"
#include "nwindow/NDialogInfo.h"
#include "nwindow/NMacroEntry.h"
#include "nwindow/NClasses.gen.h"

#if NW_FEATURES
// phase 6: AutoShot, ManufactureCmd (NWindow_C1 src/inject/)
#include "features/AutoShot.h"
#include "features/ManufactureCmd.h"
// phase 6: ItemHoverTip (not in NWindow_C1: the hover tip in the recent style)
#include "features/ItemHoverTip.h"
#endif

// an item of the player's inventory, of the pet's (names ours)
static FORCEINLINE NItemEntry* FindInventoryItem(INT ObjectID)
{
    return GDesktop->InventoryWnd->FindItem(ObjectID);
}
static FORCEINLINE NItemEntry* FindPetInventoryItem(INT ObjectID)
{
    return GDesktop->PetWnd->InventoryWnd->GetItem(ObjectID);
}

/*-----------------------------------------------------------------------------
    NShortCutItem.
-----------------------------------------------------------------------------*/

// 0x100cf500
NShortCutItem::NShortCutItem()
{
    bUsed = 0;
}
NW_HOOK_CTOR(0x100cf500, NShortCutItem)

// 0x100cf560 - the item entry of Info; the slot's number is NItemInfo's +0x1768 (not mapped there)
NShortCutItem::NShortCutItem(const NItemInfo& Info) : NItemEntry((NItemInfo*)&Info)
{
    Unknown1A74 = nw::Field<INT>(&Info, 0x1768);
    bUsed = 0;
}
NW_HOOK_CTOR_ARGS(0x100cf560, NShortCutItem, (const NItemInfo& A), (A))

// 0x100cf5d0
NW_HOOK_DTOR(0x100cf5d0, NShortCutItem)

// 0x100cc700 - IconName is not copied (the entry's Copy did); the cool time stops
void NShortCutItem::CopyFrom(NShortCutItem* Src)
{
    Copy(Src);
    if (Src->GetIcon())
        Icon = Src->GetIcon();
    if (Src->GetName())
        appStrcpy(Name, Src->GetName());
    Type = Src->Type;
    ObjectID = Src->ObjectID;
    ID = Src->ID;
    Count = Src->Count;
    UnknownC18 = Src->UnknownC18;
    Enchant = Src->Enchant;
    bStackable = Src->bStackable;
    UnknownC34 = Src->UnknownC34;
    Unknown1A74 = Src->Unknown1A74;
    CrystalGrade = Src->CrystalGrade;
    bUsed = Src->bUsed;
    Owner = Src->Owner;
    CoolTime.Stop();
}
NW_HOOK_METHOD(0x100cc700, NShortCutItem, CopyFrom)

// 0x100cc7f0
void NShortCutItem::Clear()
{
    Reset();
    Unknown1A74 = 0;
    bUsed = 0;
    CoolTime.Stop();
}
NW_HOOK_METHOD(0x100cc7f0, NShortCutItem, Clear)

/*-----------------------------------------------------------------------------
    NCShortCutWnd.
-----------------------------------------------------------------------------*/

// 0x100cd630 - the arrays, the pad mode flag and the buttons are left as allocated (OnCreate builds them)
NCShortCutWnd::NCShortCutWnd(INT InCols, INT InPages, INT InIconSize)
{
    Keys = NULL;
    BackTex = NULL;
    MaskTex = NULL;
    ToolTipTex = NULL;
    Cols = InCols;
    IconSize = InIconSize;
    Pages = InPages;
    PressedSlot = -1;
    HoverSlot = -1;
    Page = 0;
    bHorizontal = 0;
    IconBackTex = FindTexture(TEXT("l2ui.nwindow.icon_back"));
    bPressed = 0;
    PadHold = 0;
    IconClickTex = FindTexture(TEXT("l2ui.nwindow.icon_click"));
}
NW_HOOK_CTOR_ARGS(0x100cd630, NCShortCutWnd, (INT A, INT B, INT C), (A, B, C))

// 0x100cd740
NW_HOOK_DTOR(0x100cd740, NCShortCutWnd)


// 0x100cc820 (vslot 25)
INT NCShortCutWnd::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (NControl::WindowProc(Msg, wParam, lParam))
        return 1;
    if (Msg == NWM_COMMAND)
    {
        if (wParam == (WPARAM)NID_SHORTCUT_NEXT)
            return OnNextClicked() != 0;
        if (wParam == (WPARAM)NID_SHORTCUT_PREV)
            return OnPrevClicked() != 0;
        if (wParam == (WPARAM)NID_SHORTCUT_ROTATE)
            return OnRotateClicked() != 0;
        if (wParam == (WPARAM)NID_SHORTCUT_PAD)
            return OnPadClicked() != 0;   // new in C2
    }
    return 0;
}
NW_HOOK_METHOD(0x100cc820, NCShortCutWnd, WindowProc)

// an arrow or the turn button, repeating every 300 ms while held (repeated inline in the stock
// OnCreate)
static FORCEINLINE NCPushButton* CreateBarButton(NCWnd* Parent, const TCHAR* NormalTex,
                                                 const TCHAR* PushedTex, INT ID, INT X, INT Y)
{
    NCPushButton* Button = new NCPushButton(NormalTex, PushedTex);
    Button->ID = ID;
    Button->SetWindowRect(X, Y, 12, 12);
    Parent->CreateChild(Button, NWS_VISIBLE);
    Button->RepeatDelay = 300;
    return Button;
}

// 0x100cf630 (vslot 36) - C2: the game pad textures and the pad button (hidden until the pad is in use) are new; the
// title bar is the 44 x 11 strip
INT NCShortCutWnd::OnCreate()
{
    NControl::OnCreate();
    BackTex = FindTexture(TEXT("L2UI.ShortcutWnd.Back"));
    MaskTex = FindTexture(TEXT("L2UI.ShortcutWnd.mask"));
    ToolTipTex = FindTexture(TEXT("L2UI.tooltip.mini-tool01"));
    ToggleTex = FindTexture(TEXT("L2_SkillTime.ToggleEffect.ToggleEffect001"));
    PadHoldLTex = FindTexture(TEXT("L2UI.JoypadShortcutWnd.Joypad_HoldL"));
    PadHoldRTex = FindTexture(TEXT("L2UI.JoypadShortcutWnd.Joypad_HoldR"));
    FrameCtrl = new NCFrameCtrl(0);
    FrameID = NID_SHORTCUT_FRAME;
    FrameCtrl->ID = NID_SHORTCUT_FRAME;
    FrameCtrl->SetWindowRect(0, 0, 44, 11);
    CreateChild(FrameCtrl, NWS_VISIBLE);
    Items = new NShortCutItem[Cols * Pages];
    Keys = new NShortCutKey[Cols * Pages];
    NextButton = CreateBarButton(this, TEXT("L2UI.ShortcutWnd.RightBtn1"),
                                 TEXT("L2UI.ShortcutWnd.RightBtn2"), NID_SHORTCUT_NEXT, 31, 11);
    PrevButton = CreateBarButton(this, TEXT("L2UI.ShortcutWnd.LeftBtn1"),
                                 TEXT("L2UI.ShortcutWnd.LeftBtn2"), NID_SHORTCUT_PREV, 1, 11);
    RotateButton = CreateBarButton(this, TEXT("L2UI.ShortcutWnd.shortcut_rotatev"),
                                   TEXT("L2UI.ShortcutWnd.shortcut_rotatev"), NID_SHORTCUT_ROTATE, 16, 403);
    PadButton = CreateBarButton(this, TEXT("L2UI.JoypadShortcutWnd.Joypad_topadv"),
                                TEXT("L2UI.JoypadShortcutWnd.Joypad_topadv"), NID_SHORTCUT_PAD, 30, 403);
    PadButton->HideWindow();
    return 1;
}
NW_HOOK_METHOD(0x100cf630, NCShortCutWnd, OnCreate)

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

// 0x100cfb00 (vslot 37)
INT NCShortCutWnd::OnDestroy()
{
    guard(NCShortCutWnd::OnDestroy);
    if (Items)
    {
        delete[] Items;
        Items = NULL;
    }
    if (Keys)
    {
        delete[] Keys;
        Keys = NULL;
    }
    DestroyChild(this, NextButton);
    DestroyChild(this, PrevButton);
    DestroyChild(this, RotateButton);
    DestroyChild(this, PadButton);
    NControl::OnDestroy();
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x100cfb00, NCShortCutWnd, OnDestroy)

// The macros of the console's macro window (NConsoleWnd::GetMacroCount, 0x10056A20, counts them, 0 without the window,
// GetMacro, 0x10060CE0, gets one by its index, NULL out of range; the slot of a macro holds the macro's id in ID)
static FORCEINLINE INT MacroCount()
{
    return GConsole->GetMacroCount();
}
static FORCEINLINE NMacroEntry* MacroAt(INT Index)
{
    return GConsole->GetMacro(Index);
}

// The index of the macro whose id is MacroID, MacroCount() when there is none (the stock code's do-while: the first
// macro is looked at only when there is one)
static INT FindMacro(INT MacroID)
{
    INT Index = 0;
    if (MacroCount() > 0)
    {
        do
        {
            if (MacroID == MacroAt(Index)->ID)
                break;
            Index++;
        } while (Index < MacroCount());
    }
    return Index;
}

// a slot of the current page, drawn with its icon at (X, Y) (repeated for the three layouts in the stock painters): the
// icon, its frame, the pressed mark, a macro's short name (two letters a row, from the icon's corner + 4), the toggle
// skill mark, the flash when its cool time ends (with a sound) and the cool time itself; an empty slot draws nothing
static FORCEINLINE void PaintSlot(NCShortCutWnd* Wnd, NCGDevice* Device, INT Slot, INT X, INT Y)
{
    NShortCutItem* Item = &Wnd->Items[Slot];
    if (!Item->bUsed)
        return;
    INT Size = Wnd->IconSize;
    Device->DrawTexture(X, Y, Size, Size, 0, 0, Size, Size, Item->GetIcon());
    Device->DrawTexture(X - 1, Y - 1, 0x22, 0x22, 0, 0, 0x22, 0x22, Wnd->IconBackTex);
    if (Wnd->PressedSlot == Slot && Wnd->bPressed)
        Device->DrawTexture(X - 1, Y - 1, 0x22, 0x22, 0, 0, 0x22, 0x22, Wnd->IconClickTex);
    if (Item->Type == SHORTCUT_Macro)
    {
        INT Index = FindMacro(Item->ID);
        if (Index < MacroCount())
        {
            const TCHAR* ShortName = MacroAt(Index)->GetShortName();
            if (ShortName)
            {
                INT TextX = X + 4;
                INT TextY = Y + 4;
                FString Text(ShortName);
                for (INT i = 0; i < Text.Len(); i++)
                {
                    Device->DrawText(TextX, TextY, 0xFFFFFFFF, *Text.Mid(i, 1), 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
                    if (i & 1)
                    {
                        TextY += 13;
                        TextX -= 13;
                    }
                    else
                        TextX += 13;
                }
            }
        }
    }
    if (GConsole->IsToggleSkill(Item->ID, Item->UnknownC18) && GConsole->HasAbnormal(Item->ID))
    {
        Wnd->ToggleTex->UnknownA4 = 9.0f;
        Device->DrawTexture(X, Y, 0x20, 0x20, 0, 0, 0x20, 0x20, Wnd->ToggleTex);
    }
    if (Item->CoolTime.bFlashing)
    {
        Size = Wnd->IconSize;
        Device->DrawBrightenTexture(X, Y, Size, Size, 0, 0, Size, Size, Item->CoolTime.GetFlashTexture(), 0xFF008080);
        if (Item->CoolTime.bSoundDue)
        {
            GConsole->PlayInterfaceSound(9);
            Item->CoolTime.bSoundDue = 0;
        }
    }
    if (Item->CoolTime.bRunning && !Item->CoolTime.IsIdle() && Item->CoolTime.GetTexture())
    {
        Size = Wnd->IconSize;
        Device->DrawTexture(X, Y, Size, Size, 0, 0, Size, Size, Item->CoolTime.GetTexture());
    }
}

// the key's number next to a slot's icon, in the small digits (0 for the tenth)
static void PaintSlotNumber(NCGDevice* Device, INT Number, INT X, INT Y)
{
    TCHAR Text[6];
    appSprintf(Text, TEXT("%d"), Number % 10);
    Device->DrawSpecialDigit(1, X + 1, Y + 1, 0xFFB09B79, Text);
}

// the page's number in the large digits, centered at CenterX (the tenth page, two digits, from X10 and X0)
static void PaintPageNumber(NCGDevice* Device, INT PageNumber, INT CenterX, INT X10, INT X0, INT Y)
{
    TCHAR Text[6];
    appSprintf(Text, TEXT("%d"), PageNumber);
    SIZE Size;
    NCGDevice::GetTextExtent(&Size, Text, 0, 0, 0, 0, 0);
    if (PageNumber < 10)
        Device->DrawSpecialDigit(1, CenterX - Size.cx / 2, Y, 0xFFFFFFFF, Text);
    else if (PageNumber == 10)
    {
        appSprintf(Text, TEXT("%d"), 1);
        Device->DrawSpecialDigit(1, X10, Y, 0xFFFFFFFF, Text);
        appSprintf(Text, TEXT("%d"), 0);
        Device->DrawSpecialDigit(1, X0, Y, 0xFFFFFFFF, Text);
    }
}

// 0x100cd790 (name ours) - the vertical bar (44 x 417: the back, the page's number at the top, the slots down from 31)
// or the horizontal one (the same turned: 417 x 44), each slot IconSize + 5 apart with its key's number
void NCShortCutWnd::PaintBar(NCGDevice* Device)
{
    if (!bHorizontal)
    {
        if (BackTex)
            Device->DrawTexture(0, 0, 0x2C, 0x1A1, 0, 0, 0x2C, 0x1A1, BackTex);
        PaintPageNumber(Device, Page + 1, 0x16, 0x10, 0x14, 0xE);
    }
    else
    {
        if (BackTex)
            Device->DrawTexture(0, 0, 0x1A1, 0x2C, 0, 0, 0x1A1, 0x2C, BackTex);
        PaintPageNumber(Device, Page + 1, 0x11, 0xC, 0x10, 0x12);
    }
    INT First = Page * Cols;
    INT Along = 0x1F;
    for (INT i = 1; i <= Cols; i++)
    {
        INT X = bHorizontal ? Along : 6;
        INT Y = bHorizontal ? 6 : Along;
        PaintSlot(this, Device, First + i - 1, X, Y);
        PaintSlotNumber(Device, i, X, Y);
        Along += IconSize + 5;
    }
}
NW_HOOK_METHOD(0x100cd790, NCShortCutWnd, PaintBar)

// 0x100ce160 (name ours) - the game pad layout (167 x 84): the back, the hold marks of the left (1) and right (2) buttons,
// the page's number, and the four slots PadHold selects (0 .. 3, 4 .. 7, 8 .. 9) at the pad's cross: left, bottom, right,
// top
void NCShortCutWnd::PaintPad(NCGDevice* Device)
{
    if (BackTex)
        Device->DrawTexture(0, 0, 0xA7, 0x54, 0, 0, 0xA7, 0x54, BackTex);
    if (PadHold == 1)
        Device->DrawTexture(0x22, 5, 0x14, 0x13, 0, 0, 0x14, 0x13, PadHoldLTex);
    else if (PadHold == 2)
        Device->DrawTexture(0x7E, 5, 0x14, 0x13, 0, 0, 0x14, 0x13, PadHoldRTex);
    PaintPageNumber(Device, Page + 1, 0x13, 0xD, 0x11, 0x25);
    INT First = Cols * Page;
    for (INT k = PadHold * 4; k < PadHold * 4 + 4 && k + 1 < 11; k++)
    {
        INT X = 0;
        INT Y = 0;
        switch (k % 4)
        {
        case 0:
            X = 0x23;
            Y = 0x1A;
            break;
        case 1:
            X = 0x49;
            Y = 0x2C;
            break;
        case 2:
            X = 0x6F;
            Y = 0x1A;
            break;
        case 3:
            X = 0x49;
            Y = 7;
            break;
        }
        PaintSlot(this, Device, First + k, X, Y);
        PaintSlotNumber(Device, k + 1, X, Y);
    }
}
NW_HOOK_METHOD(0x100ce160, NCShortCutWnd, PaintPad)

// 0x100ce740 (vslot 55) - the layout's painter, clipped to the window: the vertical / horizontal bar (PaintBar) or the
// game pad's (PaintPad)
INT NCShortCutWnd::OnPaint(NCGDevice* Device)
{
    Device->BeginPaint(ScreenX, ScreenY);
    Device->SetClip(ScreenX, ScreenY, Width, Height);
    if (bPadMode == 0)
        PaintBar(Device);
    else if (bPadMode == 1)
        PaintPad(Device);
    Device->ReleaseClip();
    Device->EndPaint();
    return 1;
}
NW_HOOK_METHOD(0x100ce740, NCShortCutWnd, OnPaint)

// 0x100cfc80 (vslot 56) - the tool tip of the slot under the mouse (C1 had a box of text): the slot's tool tip in the
// skin's full box, which it places from the slot's corner (vertical bar: 6 and 31 + (icon + 5) per row; horizontal: the
// same swapped; game pad layout: the four positions of the pad's cross) and moves to keep on the screen. Only slots
// that hold something answer
INT NCShortCutWnd::OnDrawToolTip(NCGDevice* Device)
{
    Device->BeginPaint(ScreenX, ScreenY);
    Device->SetClip(ScreenX, ScreenY, Width, Height);
    INT X = 0;
    INT Y = 0;
    NShortCutItem* Item = NULL;
    if (bPadMode == 0)
    {
        INT Slot = HoverSlot;
        if (Slot >= 0 && Items[Slot].bUsed)
        {
            Item = &Items[Slot];
            INT Index = Slot - Page * Cols;
            if (!bHorizontal)
            {
                X = 6;
                Y = (IconSize + 5) * Index + 31;
            }
            else
            {
                Y = 6;
                X = (IconSize + 5) * Index + 31;
            }
        }
    }
    else if (bPadMode == 1)
    {
        INT Slot = HoverSlot;
        if (Slot >= 0 && Items[Slot].bUsed)
        {
            Item = &Items[Slot];
            switch (Slot % 4)
            {
            case 0:
                X = 0x23;
                Y = 0x1A;
                break;
            case 1:
                Y = 0x2C;
                X = 0x49;
                break;
            case 2:
                X = 0x6F;
                Y = 0x1A;
                break;
            case 3:
                Y = 7;
                X = 0x49;
                break;
            }
        }
    }
    if (Item && ToolTipSkin)
    {
        NToolTipRect Wnd;
        Wnd.X = ScreenX;
        Wnd.Y = ScreenY;
        Wnd.W = Width;
        Wnd.H = Height;
        ToolTipSkin->DrawFull(Device, Item->ToolTip, &X, &Y, Wnd);
    }
    Device->ReleaseClip();
    Device->EndPaint();
    return 1;
}
NW_HOOK_METHOD(0x100cfc80, NCShortCutWnd, OnDrawToolTip)

// 0x100ce7c0 (vslot 61)
INT NCShortCutWnd::OnLButtonDown(WPARAM wParam, LPARAM lParam)
{
    INT Slot = SlotAt(LOWORD(lParam), HIWORD(lParam));
    PressedSlot = Slot;
    HoverSlot = Slot;
    if (Slot >= 0 && Items[Slot].bUsed)
    {
        bDragging = 1;
        bPressed = 1;
    }
    return 1;
}
NW_HOOK_METHOD(0x100ce7c0, NCShortCutWnd, OnLButtonDown)

// 0x100ce820 (vslot 60) - the slot under the mouse (by the layout); while a slot is carried the cursor is its icon and,
// new in C2, an edit line of the macro editor (the window the desktop keeps at +0xFC) under the mouse is marked (+0x1D8
// of the edit box); anywhere else the editor is told to unmark; with nothing carried the cursor goes back unless Ctrl is
// down (NCActionItemWnd::OnMouseMove does the same)
INT NCShortCutWnd::OnMouseMove(WPARAM wParam, LPARAM lParam)
{
    INT X = LOWORD(lParam);
    INT Y = HIWORD(lParam);
    HoverSlot = SlotAtPoint(X, Y);
    if (IsFocused())
    {
        if (bDragging)
        {
            if (PressedSlot >= 0 && Items[PressedSlot].GetIcon())
            {
                SetCursor(4, Items[PressedSlot].GetIcon());
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
}
NW_HOOK_METHOD(0x100ce820, NCShortCutWnd, OnMouseMove)

// the dialog boxes of the shortcut items (NDialogInfo::ID; names ours, the console's UseShortCut has them too): an item with
// a warning asks first, an item with a message of its own shows it
enum { DIALOG_CONFIRM_ITEM = 0x21, DIALOG_ITEM_MESSAGE = 0x45 };

// the command prefix of the macro editor's lines ("/"): the string at 0x10107E78 through this unit's pointer
#if NW_HOOK_MODE
#define GShortCutCmdPrefix NW_GLOBAL(const TCHAR*, 0x101878EC)
#else
extern const TCHAR* GShortCutCmdPrefix;
#endif

// the console command that uses a shortcut slot ("/useshortcut <page> <slot>": the text 28 of the second table)
enum { CMD_UseShortCut = 0x1C };

// An item with a warning asks before it is used: system message 0x31E with the item's name on the button; the answer
// comes back to the bar's entry with the item's object id (repeated inline in the stock OnLButtonUp and OnShortCutKeyDown)
static FORCEINLINE void ShowItemWarning(NCShortCutWnd* Wnd, NShortCutItem* Item)
{
    TCHAR Name[64];
    appSprintf(Name, TEXT("%s"), Item->NItemEntry::GetName());
    NDialogInfo Info;
    Info.ID = DIALOG_CONFIRM_ITEM;
    Info.Owner = Wnd->Entry;
    Info.bMainBox = 1;
    Info.bInput = 0;
    Info.SetSystemMessage(0x31E);
    Info.SetButtons(Name, NULL);
    Info.bLargeAmount = 0;
    Info.SetParams(Item->ObjectID, -1);
    GConsole->ShowDialog(&Info);
}

// An item with a message of its own shows it (system message number from the item: NItemEntry::GetUnknown19AC)
static void ShowItemMessage(NCShortCutWnd* Wnd, NShortCutItem* Item)
{
    NDialogInfo Info;
    Info.ID = DIALOG_ITEM_MESSAGE;
    Info.Owner = Wnd->Entry;
    Info.bMainBox = 1;
    Info.bInput = 0;
    Info.SetSystemMessage(Item->GetUnknown19AC());
    Info.bLargeAmount = 0;
    Info.SetParams(Item->ObjectID, -1);
    GConsole->ShowDialog(&Info);
}

// 0x100ce9d0 (vslot 62) - released on the slot pressed (and the key is ready): the slot is used (an item with a warning
// or a message of its own asks first or shows it; any slot with such a message, the stock does not look at the type), by
// UseShortCut otherwise; released over the macro editor's line it carried to, the slot's command ("/<command> <page>
// <slot>", when it fits in 32 characters) is written there (new in C2); released anywhere that is no window (not even
// this one) the slot is deleted on the server and cleared
INT NCShortCutWnd::OnLButtonUp(WPARAM wParam, LPARAM lParam)
{
    INT X = LOWORD(lParam);
    INT Y = HIWORD(lParam);
    bPressed = 0;
    if (PressedSlot < 0)
    {
        bDragging = 0;
        return 1;
    }
    if (!GConsole->IsCtrlDown())
    {
        UnlockCursor();
        SetCursor(0, NULL);
    }
    INT Slot = SlotAtPoint(X, Y);
    if (PressedSlot >= 0 && !Keys[PressedSlot].bReady)
    {
        bDragging = 0;
        return 1;
    }
    if (Slot == PressedSlot && Items[PressedSlot].bUsed)
    {
        NShortCutItem* Item = &Items[PressedSlot];
        if (Item->Type == SHORTCUT_Item && Item->Unknown1998 != 0 && nw::Field<INT>(GConsole->GetMyUser(), 0x1D8) != 0)
            ShowItemWarning(this, &Items[PressedSlot]);
        else if (Items[PressedSlot].GetUnknown19AC() > 0)
            ShowItemMessage(this, &Items[PressedSlot]);
        else
        {
            GConsole->UseShortCut(PressedSlot, 0, 0);
            if (PressedSlot >= 0)
                Keys[PressedSlot].OnKeyUp();
        }
    }
    else if (bDragging)
    {
        NWndEntry* Found = FindWindowAt(X, Y);
        if (Found && Found->Wnd && Found->Wnd->IsA(NClass::NCEditBox()))
        {
            // the edit line's grandparent: the macro editor
            NCWnd* Editor = NULL;
            if (Found->Parent && Found->Parent->Parent)
                Editor = Found->Parent->Parent->Wnd;
            if (Editor && GDesktop && GDesktop->NewWndFC == Editor)
            {
                NCMacroEditList* Lines = ((NCMacroEditWnd*)Editor)->EditList;
                INT Line = Lines->HitLine(X, Y);
                if (Line >= 0)
                {
                    TCHAR Command[256];
                    appSprintf(Command, TEXT("%s%s %d %d"), GShortCutCmdPrefix, *GL2GameData.Unknown54188[CMD_UseShortCut],
                               PressedSlot / 10 + 1, PressedSlot % 10 + 1);
                    if (appStrlen(Command) <= 32)
                    {
                        NCEditBox* Edit = Lines->Edits(Line);
                        Edit->ClearText();
                        Edit->InsertText(Command, 1);
                    }
                    ((NCMacroEditWnd*)Editor)->ClearMark(Line);
                }
            }
        }
        else if (!IsInWindow(X, Y))
        {
            L2ParamStack Params(10);
            Params.PushBack((void*)(INT_PTR)PressedSlot);
            GConsole->RequestShortCutDel(Params);
            Items[PressedSlot].Clear();
            PressedSlot = -1;
        }
    }
    bDragging = 0;
    return 1;
}
NW_HOOK_METHOD(0x100ce9d0, NCShortCutWnd, OnLButtonUp)

#if NW_FEATURES
// phase 6: AutoShot (NWindow_C1 src/inject/AutoShot.cpp): a right click on a shot toggles its
// automatic use (not stock: NCWnd's no-op otherwise, which a 0 from AutoShot stays)
INT NCShortCutWnd::OnRButtonDown(WPARAM wParam, LPARAM lParam)
{
    return nwf::AutoShot_OnRButtonDown(this, wParam, lParam);
}
#endif

// 0x100cf460 (vslot 71)
INT NCShortCutWnd::OnNextClicked()
{
    bDragging = 0;
    NextPage();
    return 1;
}
NW_HOOK_METHOD(0x100cf460, NCShortCutWnd, OnNextClicked)

// 0x100cf480 (vslot 72)
INT NCShortCutWnd::OnPrevClicked()
{
    bDragging = 0;
    PrevPage();
    return 1;
}
NW_HOOK_METHOD(0x100cf480, NCShortCutWnd, OnPrevClicked)

// 0x100cd220 (vslot 73) - turn the bar (C1: slot 65, 0x100a88b0)
INT NCShortCutWnd::OnRotateClicked()
{
    bDragging = 0;
    bHorizontal = (bHorizontal == 0);
    UpdateLayout();
    return 1;
}
NW_HOOK_METHOD(0x100cd220, NCShortCutWnd, OnRotateClicked)

// 0x100ccb30 - the slots that hold the inventory item: gone from the inventory, the slot clears; enchanted, the new
// level; a stack, the new count (the tool tip is made again)
INT NCShortCutWnd::UpdateItem(INT ObjectID)
{
    for (INT i = 0; i < Cols * Pages; i++)
    {
        NShortCutItem* Slot = &Items[i];
        if (Slot->bUsed && Slot->Type == SHORTCUT_Item && Slot->ObjectID == ObjectID)
        {
            NItemEntry* Item = FindInventoryItem(ObjectID);
            if (!Item)
                Slot->Clear();
            else if (Item->Enchant < 1)
            {
                if (Item->IsStackable())
                {
                    Items[i].Count = Item->Count;
                    Items[i].FillToolTip(NTT_ShortCut, 0);
                }
            }
            else
            {
                Items[i].Enchant = Item->Enchant;
                Items[i].FillToolTip(NTT_ShortCut, 0);
            }
        }
    }
    return 1;
}
NW_HOOK_METHOD(0x100ccb30, NCShortCutWnd, UpdateItem)

// 0x100ccc50 - the same for the pet's inventory
INT NCShortCutWnd::UpdatePetItem(INT ObjectID)
{
    for (INT i = 0; i < Cols * Pages; i++)
    {
        NShortCutItem* Slot = &Items[i];
        if (Slot->bUsed && Slot->Type == SHORTCUT_Item && Slot->ObjectID == ObjectID)
        {
            NItemEntry* Item = FindPetInventoryItem(ObjectID);
            if (!Item)
                Slot->Clear();
            else
            {
                if (Item->Enchant < 1)
                {
                    if (!Item->IsStackable())
                        continue;
                    Items[i].Count = Item->Count;
                }
                else
                    Items[i].Enchant = Item->Enchant;
                Items[i].FillToolTip(NTT_ShortCut, 0);
            }
        }
    }
    return 1;
}
NW_HOOK_METHOD(0x100ccc50, NCShortCutWnd, UpdatePetItem)

// 0x100ccd50 - the slot of the current page under a screen point: icons IconSize wide from
// 31 on, 5 apart, at 6 across the bar
INT NCShortCutWnd::SlotAt(INT X, INT Y)
{
    INT Slot = -1;
    if (!bHorizontal)
    {
        INT RX = X - ScreenX;
        INT RY = Y - ScreenY;
        INT Top = 31;
        if (RX < 6 || RX > IconSize + 6)
            return -1;
        for (INT i = 0; i < Cols; i++)
        {
            if (RY >= Top && RY <= IconSize + Top)
                Slot = Page * Cols + i;
            Top += IconSize + 5;
        }
    }
    else
    {
        INT RX = X - ScreenX;
        INT RY = Y - ScreenY;
        INT Left = 31;
        if (RY < 6 || RY > IconSize + 6)
            return -1;
        for (INT i = 0; i < Cols; i++)
        {
            if (RX >= Left && RX <= IconSize + Left)
                Slot = Page * Cols + i;
            Left += IconSize + 5;
        }
    }
    return Slot;
}
NW_HOOK_METHOD(0x100ccd50, NCShortCutWnd, SlotAt)

// 0x100cce40 (name ours) - the game pad layout's slot under a screen point: the four cells of the cross (left, bottom, right,
// top: the places PaintPad draws them at, IconSize wide from x, IconSize + 1 tall from y) give PadHold * 4 + 0 .. 3; -1 none
INT NCShortCutWnd::PadSlotAt(INT X, INT Y)
{
    INT RX = X - ScreenX;
    INT RY = Y - ScreenY;
    for (INT i = 0; i < 4; i++)
    {
        INT CellX = 0;
        INT CellY = 0;
        switch (i)
        {
        case 0:
            CellX = 0x23;
            CellY = 0x1A;
            break;
        case 1:
            CellX = 0x49;
            CellY = 0x2C;
            break;
        case 2:
            CellX = 0x6F;
            CellY = 0x1A;
            break;
        case 3:
            CellX = 0x49;
            CellY = 7;
            break;
        }
        if (RX >= CellX && RX < IconSize + CellX && RY >= CellY && RY <= IconSize + CellY)
            return PadHold * 4 + i;
    }
    return -1;
}
NW_HOOK_METHOD(0x100cce40, NCShortCutWnd, PadSlotAt)

// 0x100ccef0 (name ours) - the slot under a screen point by the bar's layout: the vertical / horizontal bar's (SlotAt), the
// game pad's (PadSlotAt) or -1 (the other callers: the item windows' drops and the macro window's)
INT NCShortCutWnd::SlotAtPoint(INT X, INT Y)
{
    if (bPadMode == 0)
        return SlotAt(X, Y);
    if (bPadMode == 1)
        return PadSlotAt(X, Y);
    return -1;
}
NW_HOOK_METHOD(0x100ccef0, NCShortCutWnd, SlotAtPoint)

// 0x100cc8d0 (name ours; the console's UpdateMacroShortCut hands it over) - what a change of the macro list does to the
// slots that hold the macro MacroID: Mode 2 (the list arrived), the slot takes the macro's icon ("L2UI.MacroWnd.Macro_Icon<n>"),
// name and tool tip again (when the macro is still in the list); Mode 0 (the macro was deleted), the slot is deleted on the
// server and cleared. 1 always
INT NCShortCutWnd::UpdateMacro(INT MacroID, INT Mode)
{
    if (Mode == 2)
    {
        for (INT i = 0; i < Cols * Pages; i++)
        {
            NShortCutItem* Slot = &Items[i];
            if (Slot->bUsed && Slot->Type == SHORTCUT_Macro && Slot->ID == MacroID)
            {
                INT Index = FindMacro(MacroID);
                if (Index < MacroCount())
                {
                    appSprintf(Items[i].IconName, TEXT("%s%d"), TEXT("L2UI.MacroWnd.Macro_Icon"),
                               GConsole->GetMacroIconType(MacroID) + 1);   // the macro's icon index (0x10060DD0)
                    Items[i].Icon = FindTexture(Items[i].IconName);
                    appSprintf(Items[i].Name, TEXT("%s"), GConsole->GetMacroName(MacroID));   // its name (0x10060D70)
                    Items[i].FillToolTip(NTT_ShortCut, 0);
                }
            }
        }
    }
    else if (Mode == 0)
    {
        for (INT i = 0; i < Cols * Pages; i++)
        {
            NShortCutItem* Slot = &Items[i];
            if (Slot->bUsed && Slot->Type == SHORTCUT_Macro && Slot->ID == MacroID)
            {
                L2ParamStack Params(10);
                Params.PushBack((void*)(INT_PTR)i);
                GConsole->RequestShortCutDel(Params);
                Items[i].Clear();
                PressedSlot = -1;
            }
        }
    }
    return 1;
}
NW_HOOK_METHOD(0x100cc8d0, NCShortCutWnd, UpdateMacro)

// 0x100cd610 (name ours)
void NShortCutKey::OnKeyUp()
{
    if (bOnKeyUp && !bReady)
        bReady = 1;
}
NW_HOOK_METHOD(0x100cd610, NShortCutKey, OnKeyUp)

// 0x100ccf40 (vslot 68) - the bar after a turn: it keeps its top-left corner's place by swapping the roles of its width
// and height (a clamp at 0), takes the vertical or horizontal size, textures and button places (the arrows, the
// rotate button and, new in C2, the game pad button, shown only while the game pad is in use)
void NCShortCutWnd::UpdateLayout()
{
    INT W = Width;
    INT H = Height;
    if (!bHorizontal)
    {
        INT Y = (ScreenY - W) + H;
        if (Y < 0)
            Y = 0;
        SetWindowRect((ScreenX - H) + W, Y, 0x2C, 0x1A1);
        SetSize(0x2C, 0x1A1);
        FrameCtrl->SetWindowRect(0, 0, 0x2C, 0xB);
        BackTex = FindTexture(TEXT("L2UI.ShortcutWnd.Back"));
        NextButton->SetWindowRect(0x1F, 0xB, 0xC, 0xC);
        PrevButton->SetWindowRect(1, 0xB, 0xC, 0xC);
        RotateButton->SetWindowRect(0x10, 0x193, 0xC, 0xC);
        PadButton->SetWindowRect(0x1E, 0x193, 0xC, 0xC);
        NextButton->SetNormalTex(TEXT("L2UI.ShortcutWnd.RightBtn1"));
        NextButton->SetPushedTex(TEXT("L2UI.ShortcutWnd.RightBtn2"));
        PrevButton->SetNormalTex(TEXT("L2UI.ShortcutWnd.LeftBtn1"));
        PrevButton->SetPushedTex(TEXT("L2UI.ShortcutWnd.LeftBtn2"));
        RotateButton->SetNormalTex(TEXT("L2UI.JoypadShortcutWnd.shortcut_rotatev"));
        RotateButton->SetPushedTex(TEXT("L2UI.JoypadShortcutWnd.shortcut_rotatev"));
        PadButton->SetPushedTex(TEXT("L2UI.JoypadShortcutWnd.Joypad_topadv"));
        PadButton->SetPushedTex(TEXT("L2UI.JoypadShortcutWnd.Joypad_topadv"));
    }
    else
    {
        INT X = (ScreenX - H) + W;
        if (X < 0)
            X = 0;
        SetWindowRect(X, (ScreenY - W) + H, 0x1A1, 0x2C);
        SetSize(0x1A1, 0x2C);
        FrameCtrl->SetWindowRect(0, 0, 0xB, 0x2C);
        BackTex = FindTexture(TEXT("L2UI.ShortcutWnd.Back1"));
        PrevButton->SetWindowRect(0xB, 0x1F, 0xC, 0xC);
        NextButton->SetWindowRect(0xB, 1, 0xC, 0xC);
        RotateButton->SetWindowRect(0x193, 0x10, 0xC, 0xC);
        PadButton->SetWindowRect(0x193, 2, 0xC, 0xC);
        PrevButton->SetNormalTex(TEXT("L2UI.ShortcutWnd.DownBtn1"));
        PrevButton->SetPushedTex(TEXT("L2UI.ShortcutWnd.DownBtn2"));
        NextButton->SetNormalTex(TEXT("L2UI.ShortcutWnd.UpBtn1"));
        NextButton->SetPushedTex(TEXT("L2UI.ShortcutWnd.UpBtn2"));
        RotateButton->SetNormalTex(TEXT("L2UI.JoypadShortcutWnd.shortcut_rotateh"));
        RotateButton->SetPushedTex(TEXT("L2UI.JoypadShortcutWnd.shortcut_rotateh"));
        PadButton->SetPushedTex(TEXT("L2UI.JoypadShortcutWnd.Joypad_topadh"));
        PadButton->SetPushedTex(TEXT("L2UI.JoypadShortcutWnd.Joypad_topadh"));
    }
    if (GL2UsingJoystick && GL2CanUseJoystick)
        PadButton->ShowWindow();
    else
        PadButton->HideWindow();
}
NW_HOOK_METHOD(0x100ccf40, NCShortCutWnd, UpdateLayout)

// 0x100cd430 (vslot 69)
void NCShortCutWnd::SetPadHoldL(bool bHold)
{
    PadHold = bHold;
}
NW_HOOK_METHOD(0x100cd430, NCShortCutWnd, SetPadHoldL)

// 0x100cd450 (vslot 70)
void NCShortCutWnd::SetPadHoldR(bool bHold)
{
    PadHold = bHold ? 2 : 0;
}
NW_HOOK_METHOD(0x100cd450, NCShortCutWnd, SetPadHoldR)

// 0x100cd470 (vslot 74) - the pad button: the bar takes the game pad layout (a narrow strip, only while the game pad is
// in use) or goes back to the vertical / horizontal one
INT NCShortCutWnd::OnPadClicked()
{
    if (bPadMode == 0)
    {
        if (GL2UsingJoystick && GL2CanUseJoystick)
        {
            BackTex = FindTexture(TEXT("L2UI.JoypadShortcutWnd.joypad_back"));
            NextButton->SetNormalTex(TEXT("L2UI.JoypadShortcutWnd.joypad_next"));
            NextButton->SetPushedTex(TEXT("L2UI.JoypadShortcutWnd.joypad_next_down"));
            PrevButton->SetNormalTex(TEXT("L2UI.JoypadShortcutWnd.joypad_prev"));
            PrevButton->SetPushedTex(TEXT("L2UI.JoypadShortcutWnd.joypad_prev_down"));
            PadButton->SetPushedTex(TEXT("L2UI.JoypadShortcutWnd.Joypad_tokey"));
            PadButton->SetPushedTex(TEXT("L2UI.JoypadShortcutWnd.Joypad_tokey"));
            PadButton->SetWindowRect(0x9A, 0x24, 0xC, 0xC);
            NextButton->SetWindowRect(0xD, 0, 0xC, 0x21);
            PrevButton->SetWindowRect(0xD, 0x33, 0xC, 0x21);
            RotateButton->HideWindow();
            FrameCtrl->SetWindowRect(0, 0, 0xD, 0x54);
            SetSize(0xA7, 0x54);
            PadHold = 0;
            bPadMode = 1;
        }
    }
    else if (bPadMode == 1)
    {
        bPadMode = 0;
        RotateButton->ShowWindow();
        UpdateLayout();
    }
    return 1;
}
NW_HOOK_METHOD(0x100cd470, NCShortCutWnd, OnPadClicked)

// 0x100cd5d0 (vslot 43) - the pad button shows with the bar only while the game pad is in use
void NCShortCutWnd::ShowWindow()
{
    NCWnd::ShowWindow();
    if (GL2UsingJoystick && GL2CanUseJoystick)
        PadButton->ShowWindow();
    else
        PadButton->HideWindow();
}
NW_HOOK_METHOD(0x100cd5d0, NCShortCutWnd, ShowWindow)

// 0x100cd250 - a new language: the name of every slot that holds something is looked up again (the item's, the skill's
// with its level, the action's or the macro's, from the console's tables) and its tool tip made again; a name that is no
// longer there leaves the slot as it was
void NCShortCutWnd::ReloadNames()
{
    for (INT i = 0; i < Cols * Pages; i++)
    {
        NShortCutItem* Slot = &Items[i];
        if (!Slot->bUsed)
            continue;
        // the stock asks the table twice (is there a name, then the name), as written here
        const TCHAR* Name = NULL;
        if (Slot->Type == SHORTCUT_Item && GConsole->GetItemName(Slot->ID))
            Name = GConsole->GetItemName(Slot->ID);
        else if (Slot->Type == SHORTCUT_Skill && GConsole->GetSkillName(Slot->ID, Slot->UnknownC18))
            Name = GConsole->GetSkillName(Slot->ID, Slot->UnknownC18);
        else if (Slot->Type == SHORTCUT_Action && GConsole->GetActionName(Slot->ID))
            Name = GConsole->GetActionName(Slot->ID);
        else if (Slot->Type == SHORTCUT_Macro && GConsole->GetMacroName(Slot->ID))   // the macro's name (0x10060D70)
            Name = GConsole->GetMacroName(Slot->ID);
        if (Name)
        {
            appStrcpy(Items[i].Name, Name);
            Items[i].FillToolTip(NTT_ShortCut, 0);
        }
    }
}
NW_HOOK_METHOD(0x100cd250, NCShortCutWnd, ReloadNames)

// 0x100ccf10 - the slot of the current page's Index
INT NCShortCutWnd::PageSlot(INT Index)
{
    if (!Page)
        return Index;
    return Cols * Page + Index;
}
NW_HOOK_METHOD(0x100ccf10, NCShortCutWnd, PageSlot)

// 0x100cee40 - the slot's item cleared, its key ready again
__declspec(noinline) INT NCShortCutWnd::DeleteShortCut(INT Slot)
{
    if (Pages * Cols <= Slot)
        return 0;
    ItemAt(Slot)->Clear();
    Keys[Slot] = NShortCutKey();
    return 1;
}
NW_HOOK_METHOD(0x100cee40, NCShortCutWnd, DeleteShortCut)

// 0x100ceea0 - leaving a page makes its keys ready again
void NCShortCutWnd::SetPage(INT NewPage)
{
    if (NewPage < 0 || NewPage >= Pages)
        return;
    if (NewPage != Page)
    {
        for (INT i = 0; i < Cols; i++)
        {
            Keys[Page * Cols + i].bReady = 1;
            Keys[Page * Cols + i].Elapsed = 0;
        }
    }
    Page = NewPage;
}
NW_HOOK_METHOD(0x100ceea0, NCShortCutWnd, SetPage)

// 0x100cef10 - the keys waiting for a cool time or a fixed time become ready
void NCShortCutWnd::Tick(FLOAT DeltaTime)
{
    INT Ms = (INT)(DeltaTime * 1000.f);
    INT Count = Pages * Cols;
    for (INT i = 0; i < Count; i++)
    {
        if (Keys[i].bOnCoolTime)
        {
            if (ItemCoolTime(i)->IsIdle() && Keys[i].bOnCoolTime && !Keys[i].bReady)
                Keys[i].bReady = 1;
        }
        else if (!Keys[i].bOnKeyUp && !Keys[i].bOnCoolTime && !Keys[i].bReady)
        {
            Keys[i].Elapsed += Ms;
            if (Keys[i].Elapsed > Keys[i].Interval)
            {
                Keys[i].Elapsed = 0;
                Keys[i].bReady = 1;
            }
        }
    }
}
NW_HOOK_METHOD(0x100cef10, NCShortCutWnd, Tick)

// the slot a key uses on the current page: F1 .. F10, or 1 .. 9, 0 while the chat line needs
// Enter (and no edit box has the focus); -1 for other keys (repeated inline in both key
// handlers)
static FORCEINLINE INT KeySlot(NCShortCutWnd* Wnd, UINT Key)
{
    INT Slot = -1;
    if (Key >= VK_F1 && Key <= VK_F10)
        Slot = Wnd->Page * Wnd->Cols + Key - VK_F1;
    if (GIsEnterChatting && (!GFocusEntry || !GFocusEntry->Wnd->vfn032()))
    {
        if (Key == '0')
            Slot = Wnd->Page * Wnd->Cols + 9;
        else if (Key > '0' && Key <= '9')
            Slot = Wnd->Page * Wnd->Cols + Key - '1';
    }
    return Slot;
}

// 0x100cefd0 - a released key makes its slot ready when it waits for that
INT NCShortCutWnd::OnShortCutKeyUp(UINT Key, LPARAM lParam)
{
    INT Slot = KeySlot(this, Key);
    if (Slot >= 0 && Keys[Slot].bOnKeyUp && !Keys[Slot].bReady)
        Keys[Slot].bReady = 1;
    return 0;
}
NW_HOOK_METHOD(0x100cefd0, NCShortCutWnd, OnShortCutKeyUp)

// 0x100cf080 - a pressed key (F1 .. F10, or 1 .. 0 while the chat line needs Enter): 1 when its slot's key is not ready (the key
// is taken, nothing else happens); an empty slot beeps; a slot that holds something is used (an item with a warning or a
// message of its own asks first or shows it, the others by UseShortCut: C2 moved the use of every type there); 0 otherwise,
// so the console goes on with the key
INT NCShortCutWnd::OnShortCutKeyDown(UINT Key, LPARAM lParam)
{
    INT Slot = KeySlot(this, Key);
    if (Slot < 0)
        return 0;
    if (!Keys[Slot].bReady)
        return 1;
    if (Slot < Pages * Cols)
    {
        if (Items[Slot].bUsed)
        {
            NShortCutItem* Item = &Items[Slot];
            if (Item->Type == SHORTCUT_Item && Item->Unknown1998 != 0 && nw::Field<INT>(GConsole->GetMyUser(), 0x1D8) != 0)
                ShowItemWarning(this, &Items[Slot]);
            else if (Items[Slot].Type == SHORTCUT_Item && Items[Slot].GetUnknown19AC() > 0)
                ShowItemMessage(this, &Items[Slot]);
            else
                GConsole->UseShortCut(Slot, 0, 0);
        }
        else if (Slot < Pages * Cols && !Items[Slot].bUsed)
            GConsole->PlayInterfaceSound(2);
    }
    return 0;
}
NW_HOOK_METHOD(0x100cf080, NCShortCutWnd, OnShortCutKeyDown)

// the slots of the page just shown stop flashing (repeated inline in NextPage and PrevPage)
static FORCEINLINE void StopPageFlashes(NCShortCutWnd* Wnd)
{
    INT First = Wnd->Page * Wnd->Cols;
    for (INT i = 0; i < Wnd->Cols; i++)
        Wnd->ItemCoolTime(First + i)->bFlashing = 0;
}

// 0x100cf380 - after the last page back to the first (page 9 fixed)
void NCShortCutWnd::NextPage()
{
    if (Page < Pages - 1)
        Page++;
    else if (Page == 9)
        Page = 0;
    StopPageFlashes(this);
}
NW_HOOK_METHOD(0x100cf380, NCShortCutWnd, NextPage)

// 0x100cf3f0 - before the first page to page 9
void NCShortCutWnd::PrevPage()
{
    if (Page > 0)
        Page--;
    else if (Page == 0)
        Page = 9;
    StopPageFlashes(this);
}
NW_HOOK_METHOD(0x100cf3f0, NCShortCutWnd, PrevPage)

// 0x100cf4a0
void NCShortCutWnd::ClearShortCuts()
{
    for (INT i = 0; i < Cols * Pages; i++)
        DeleteShortCut(i);
}
NW_HOOK_METHOD(0x100cf4a0, NCShortCutWnd, ClearShortCuts)

// 0x100cfe90 - the server's shortcut Info (its slot: NItemInfo +0x1798, not checked below 0; its type: +0x170C, 1 item, 2
// skill, 3 action, 4 macro) into its slot, with the key's wait: an item until its release (shots every 200 ms), a skill
// that costs no MP until its cool time is over (a toggle every second), an action until its release (attack every
// 200 ms, pick up every 500 ms); a macro none. The icon is looked up by its name (the first macro icon / the black texture
// when there is none); the slot's tool tip is made (C2). 0 when the slot is out of range, else 1
INT NCShortCutWnd::SetShortCut(NItemInfo* Info, INT Owner)
{
    if (Info->UnknownC90 >= Pages * Cols)
        return 0;
    const TCHAR* IconName = Info->IconName;
    if (IconName)
        Info->Icon = FindTexture(IconName);
    if (!Info->Icon)
        Info->Icon = FindTexture(Info->Param == SHORTCUT_Macro ? TEXT("L2UI.MacroWnd.Macro_Icon1") : TEXT("NWindow.BlackTexture"));
    NShortCutItem* Item = new NShortCutItem(*Info);
    Item->Owner = Owner;
    Item->CoolTime.Stop();
    switch (Info->Param)
    {
    case SHORTCUT_Item:
        if (GConsole->IsShotItem(Item->ID))
            Keys[Info->UnknownC90].Interval = 200;
        else
            Keys[Info->UnknownC90].bOnKeyUp = 1;
        break;
    case SHORTCUT_Skill:
        if (Item->Unknown1A74 == 0)
        {
            if (GConsole->IsToggleSkill(Item->ID, Item->UnknownC18))
                Keys[Info->UnknownC90].Interval = 1000;
            else
                Keys[Info->UnknownC90].bOnCoolTime = 1;
        }
        break;
    case SHORTCUT_Action:
        if (Item->ID == ACTION_Attack)
            Keys[Info->UnknownC90].Interval = 200;
        else if (Item->ID == ACTION_PickUp)
            Keys[Info->UnknownC90].Interval = 500;
        else
            Keys[Info->UnknownC90].bOnKeyUp = 1;
        break;
    }
    Items[Info->UnknownC90].CopyFrom(Item);
    Items[Info->UnknownC90].bUsed = 1;
    Items[Info->UnknownC90].FillToolTip(NTT_ShortCut, 0);
    delete Item;
    return 1;
}
NW_HOOK_METHOD(0x100cfe90, NCShortCutWnd, SetShortCut)

NW_CHECK_VTABLE(NCShortCutWnd, 0x10125608, 75)
