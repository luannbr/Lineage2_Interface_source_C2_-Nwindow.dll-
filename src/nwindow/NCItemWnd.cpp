/*=============================================================================
    NCItemWnd.cpp - grid of item icons, base of the item windows, and NItemEntry, the record
    of each item. Stock: 0x10015b80 .. 0x100178e0 (NCCoolTimeIcon's code sits in between:
    0x10016620 .. 0x10016850, 0x100170e0 .. 0x100172a0, 0x100173c0 .. 0x10017550) plus
    WindowProc (0x1006bb90).
=============================================================================*/
#include "nwindow/NCItemWnd.h"
#include "nwindow/NCConsole.h"
#include "nwindow/NCDialogBox.h"
#include "nwindow/NCGDevice.h"
#include "nwindow/NCSliderBar.h"
#include "nwindow/NConsoleWnd.h"
#if NW_FEATURES
// phase 6: ItemHoverTip (not in NWindow_C1: the hover tip of the item grids in the recent style)
#include "features/ItemHoverTip.h"
#endif

enum { SOUND_PickUp = 3 };  // an item picked up (name ours)

// the desktop's message box (NDialogInfo::bMainBox) and the second one
static FORCEINLINE NCDialogBox* MainDialogBox()
{
    return GDesktop->MainDialogBox;
}
static FORCEINLINE NCDialogBox* SecondDialogBox()
{
    return GDesktop->SecondDialogBox;
}

/*-----------------------------------------------------------------------------
    NItemEntry
-----------------------------------------------------------------------------*/

/// 0x1001a850 - the tool tip object is constructed first (0x10002230); the strings are emptied, the
// numbers zeroed and the ids -1
NItemEntry::NItemEntry()
{
    Name[0] = 0;
    AdditionalName[0] = 0;
    IconName[0] = 0;
    Unknown20C[0] = 0;
    Unknown30C[0] = 0;
    Description[0] = 0;
    Icon = NULL;
    Unknown004 = 0;
    Unknown008 = 0;
    TradeCount = 0;
    bSelected = 0;
    ObjectID = 0;
    ID = 0;
    Count = 0;
    UnknownC20 = 0;
    BodyPart = 0;
    Weight = 0;
    Unknown1C80 = 0;
    Unknown1C84 = 0;
    Unknown1C88 = 0;
    Unknown1C8C = 0;
    Unknown1C90 = 0;
    Unknown1C94 = 0;
    Unknown1C58 = 0;
    CrystalGrade = 0;
    Unknown1C60 = 0;
    Unknown1C64 = 0;
    Unknown1C68 = 0;
    Unknown1C6C = 0;
    Unknown1C70 = 0;
    Unknown1C78 = 0;
    Unknown1C7C = 0;
    Enchant = 0;
    UnknownC3C = 0;
    bStackable = 0;
    bEquipped = 0;
    Unknown1C98 = 0;
    Unknown1998 = 0;
    Unknown1C9C = 0;
    Unknown1CA0 = 0;
    Unknown1CA4 = 0;
    Unknown1CA8 = 0;
    Unknown19AC = 0;
    DisplayName[0] = 0;
    Type = -1;
    Unknown1910 = -1;
    UnknownC18 = -1;
    UnknownC2C = -1;
    UnknownC30 = -1;
    SubType = -1;
    UnknownC34 = -1;
}
NW_HOOK_CTOR(0x1001a850, NItemEntry)

// 0x1001a9a0 - the fields do not keep NItemInfo's order; Unknown1C60 / Unknown1C64 (NItemInfo
// +0x1758 / +0x175C) stay uninitialized, and so does the display name past its first
// character. The stock code tests the addresses of the texts but the first before copying them
// (never NULL)
NItemEntry::NItemEntry(NItemInfo* Src)
{
    Icon = NULL;
    Unknown004 = 0;
    Unknown008 = 0;
    Name[0] = 0;
    AdditionalName[0] = 0;
    IconName[0] = 0;
    Unknown20C[0] = 0;
    Unknown30C[0] = 0;
    Description[0] = 0;
    DisplayName[0] = 0;
    TradeCount = 0;
    bSelected = 0;

    Icon = Src->Icon;
    Unknown004 = Src->UnknownC04;
    Unknown008 = Src->UnknownC08;
    appStrcpy(Name, Src->Name);
    appStrcpy(AdditionalName, Src->AdditionalName);
    appStrcpy(IconName, Src->IconName);
    appStrcpy(Unknown20C, Src->Unknown200);
    appStrcpy(Unknown30C, Src->Unknown300);
    appStrcpy(Description, Src->Description);
    Type = Src->Param;
    Unknown1910 = Src->Unknown1710;
    ObjectID = Src->ObjectID;
    ID = Src->ID;
    Count = Src->Count;
    UnknownC20 = Src->UnknownC1C;
    UnknownC18 = Src->UnknownC20;
    BodyPart = Src->BodyPart;
    Weight = Src->UnknownC28;
    UnknownC2C = Src->UnknownC2C;
    UnknownC30 = Src->UnknownC30;
    Unknown1C80 = Src->UnknownC34[0];
    Unknown1C84 = Src->UnknownC34[1];
    Unknown1C88 = Src->UnknownC34[2];
    Unknown1C8C = Src->UnknownC34[3];
    Unknown1C90 = Src->UnknownC34[4];
    Unknown1C94 = Src->UnknownC34[5];
    Unknown1C58 = Src->UnknownC34[6];
    CrystalGrade = Src->UnknownC34[7];
    Unknown1C68 = Src->UnknownC34[10];
    Unknown1C6C = Src->UnknownC34[11];
    Unknown1C70 = Src->UnknownC34[12];
    SubType = Src->SubType;
    Unknown1C78 = Src->UnknownC6C;
    Unknown1C7C = Src->UnknownC70;
    Enchant = Src->Enchant;
    UnknownC3C = Src->UnknownC84;
    bStackable = Src->bStackable;
    bEquipped = Src->UnknownC8C;
    UnknownC34 = Src->UnknownC90;
    Unknown1C98 = Src->UnknownC94;
    Unknown1998 = Src->Unknown17A0;
    Unknown1C9C = Src->UnknownC78;
    Unknown1CA0 = Src->UnknownC7C;
    Unknown1CA4 = Src->UnknownC80;
    Unknown1CA8 = Src->UnknownC98;
    Unknown19AC = Src->Unknown1788;
}
NW_HOOK_CTOR_ARGS(0x1001a9a0, NItemEntry, (NItemInfo* A), (A))

// 0x1007faf0 - what is left is the tool tip object's destructor (0x100022d0)
NItemEntry::~NItemEntry()
{
}
NW_HOOK_DTOR(0x1007faf0, NItemEntry)

// 0x10019580
const TCHAR* NItemEntry::GetName()
{
    const TCHAR* S = Name;
    return S && *S ? S : NULL;
}
NW_HOOK_METHOD(0x10019580, NItemEntry, GetName)

// 0x10019590 - the name the tool tip and the lists show: "+N" (the enchant, when above 0 and the
// type in Unknown1910 is not 7, 0xC or 0xD), the name, "-" and the additional name
const TCHAR* NItemEntry::MakeDisplayName()
{
    TCHAR* Out = DisplayName;
    Out[0] = 0;
    if (Enchant > 0 && Unknown1910 != 7 && Unknown1910 != 0xC && Unknown1910 != 0xD)
        appSprintf(Out, TEXT("+%d"), Enchant);
    const TCHAR* S = Name;
    if (S && *S)
        appStrcat(Out, S);
    const TCHAR* Add = AdditionalName;
    if (Add && *Add)
    {
        appStrcat(Out, TEXT("-"));
        appStrcat(Out, Add);
    }
    Out[appStrlen(Out) + 1] = 0;
    return Out;
}
NW_HOOK_METHOD(0x10019590, NItemEntry, MakeDisplayName)

// 0x10019630
const TCHAR* NItemEntry::GetAdditionalName()
{
    const TCHAR* S = AdditionalName;
    return S && *S ? S : NULL;
}
NW_HOOK_METHOD(0x10019630, NItemEntry, GetAdditionalName)

// 0x10019650
const TCHAR* NItemEntry::GetDescription()
{
    const TCHAR* S = Description;
    return S && *S ? S : NULL;
}
NW_HOOK_METHOD(0x10019650, NItemEntry, GetDescription)

// 0x10019670
INT NItemEntry::GetUnknownC20()
{
    return UnknownC20;
}
NW_HOOK_METHOD(0x10019670, NItemEntry, GetUnknownC20)

// 0x10019680
INT NItemEntry::GetUnknownC34()
{
    return UnknownC34;
}
NW_HOOK_METHOD(0x10019680, NItemEntry, GetUnknownC34)

// 0x10019690
void NItemEntry::SetUnknownC34(INT Value)
{
    UnknownC34 = Value;
}
NW_HOOK_METHOD(0x10019690, NItemEntry, SetUnknownC34)

// 0x1001af90 - the texts and the first three fields only when set in Src (the three icon textures
// all look at IconName's first character: the stock code does, whatever the intent was); the
// display name is Src's own (built), the tool tip object is copied whole (0x1001aca0)
void NItemEntry::Copy(NItemEntry* Src)
{
    if (Src->GetIcon())
        Icon = Src->GetIcon();
    if (Src->GetUnknown004())
        Unknown004 = Src->GetUnknown004();
    if (Src->GetUnknown008())
        Unknown008 = Src->GetUnknown008();
    if (Src->GetName())
        appStrcpy(Name, Src->GetName());
    else
        Name[0] = 0;
    const TCHAR* SrcIconName = Src->IconName;
    if (SrcIconName && *SrcIconName)
        appStrcpy(IconName, SrcIconName);
    if (Src->Unknown20C && *SrcIconName)
        appStrcpy(Unknown20C, Src->Unknown20C);
    if (Src->Unknown30C && *SrcIconName)
        appStrcpy(Unknown30C, Src->Unknown30C);
    if (Src->GetAdditionalName())
        appStrcpy(AdditionalName, Src->GetAdditionalName());
    else
        AdditionalName[0] = 0;
    if (Src->GetDescription())
        appStrcpy(Description, Src->GetDescription());
    else
        Description[0] = 0;
    if (Src->MakeDisplayName())
        appStrcpy(DisplayName, Src->MakeDisplayName());
    else
        DisplayName[0] = 0;
    Type = Src->Type;
    Unknown1910 = Src->Unknown1910;
    ObjectID = Src->ObjectID;
    ID = Src->ID;
    Count = Src->Count;
    UnknownC20 = Src->UnknownC20;
    UnknownC18 = Src->UnknownC18;
    BodyPart = Src->BodyPart;
    Weight = Src->Weight;
    UnknownC2C = Src->UnknownC2C;
    UnknownC30 = Src->UnknownC30;
    Enchant = Src->Enchant;
    UnknownC3C = Src->UnknownC3C;
    bStackable = Src->bStackable;
    bEquipped = Src->bEquipped;
    UnknownC34 = Src->UnknownC34;
    bSelected = Src->bSelected;
    Unknown1C58 = Src->Unknown1C58;
    CrystalGrade = Src->CrystalGrade;
    Unknown1C60 = Src->Unknown1C60;
    Unknown1C64 = Src->Unknown1C64;
    Unknown1C68 = Src->Unknown1C68;
    Unknown1C6C = Src->Unknown1C6C;
    Unknown1C70 = Src->Unknown1C70;
    SubType = Src->SubType;
    Unknown1C80 = Src->Unknown1C80;
    Unknown1C84 = Src->Unknown1C84;
    Unknown1C88 = Src->Unknown1C88;
    Unknown1C8C = Src->Unknown1C8C;
    Unknown1C90 = Src->Unknown1C90;
    Unknown1C94 = Src->Unknown1C94;
    Unknown1C78 = Src->Unknown1C78;
    Unknown1C98 = Src->Unknown1C98;
    Unknown1998 = Src->Unknown1998;
    Unknown1C9C = Src->Unknown1C9C;
    Unknown1CA0 = Src->Unknown1CA0;
    Unknown1CA4 = Src->Unknown1CA4;
    Unknown1CA8 = Src->Unknown1CA8;
    Unknown1C7C = Src->Unknown1C7C;
    Unknown19AC = Src->Unknown19AC;
    ToolTip = Src->ToolTip;   // NToolTipInfo's copy assignment (0x1001aca0)
}
NW_HOOK_METHOD(0x1001af90, NItemEntry, Copy)

// 0x10028b20 is NToolTipInfo::Fill (NToolTip.cpp): the tool tip object builds itself from the entry. Style
// NTT_Item (2) is the item windows' usual call, Options 0 or NTTO_Price (4) / NTTO_PriceEach (8) for the shops'
void NItemEntry::FillToolTip(INT Flags, INT Extra)
{
    ToolTip.Fill(this, Flags, Extra);
}

// 0x100196a0
void NItemEntry::Select()
{
    bSelected = 1;
}
NW_HOOK_METHOD(0x100196a0, NItemEntry, Select)

// 0x100196b0
UBOOL NItemEntry::IsSelected()
{
    return bSelected;
}
NW_HOOK_METHOD(0x100196b0, NItemEntry, IsSelected)

// 0x100196c0
void NItemEntry::Unselect()
{
    bSelected = 0;
}
NW_HOOK_METHOD(0x100196c0, NItemEntry, Unselect)

// 0x100196d0
UBOOL NItemEntry::IsStackable()
{
    return bStackable;
}
NW_HOOK_METHOD(0x100196d0, NItemEntry, IsStackable)

// 0x100196e0 (name ours)
INT NItemEntry::GetUnknown19AC()
{
    return Unknown19AC;
}
NW_HOOK_METHOD(0x100196e0, NItemEntry, GetUnknown19AC)

// 0x100196f0 - the constructor's defaults, but the tool tip object, TradeCount, bSelected, Unknown1CA8
// and Unknown19AC are kept
void NItemEntry::Reset()
{
    Name[0] = 0;
    AdditionalName[0] = 0;
    IconName[0] = 0;
    Unknown20C[0] = 0;
    Unknown30C[0] = 0;
    Description[0] = 0;
    Icon = NULL;
    Unknown004 = 0;
    Unknown008 = 0;
    Type = -1;
    Unknown1910 = -1;
    ObjectID = 0;
    ID = 0;
    Count = 0;
    UnknownC20 = 0;
    UnknownC18 = -1;
    BodyPart = 0;
    Weight = 0;
    UnknownC2C = -1;
    UnknownC30 = -1;
    Unknown1C80 = 0;
    Unknown1C84 = 0;
    Unknown1C88 = 0;
    Unknown1C8C = 0;
    Unknown1C90 = 0;
    Unknown1C94 = 0;
    Unknown1C58 = 0;
    CrystalGrade = 0;
    Unknown1C60 = 0;
    Unknown1C64 = 0;
    Unknown1C68 = 0;
    Unknown1C6C = 0;
    Unknown1C70 = 0;
    SubType = -1;
    Unknown1C78 = 0;
    Unknown1C7C = 0;
    Enchant = 0;
    UnknownC3C = 0;
    bStackable = 0;
    bEquipped = 0;
    UnknownC34 = -1;
    Unknown1C98 = 0;
    Unknown1998 = 0;
    Unknown1C9C = 0;
    Unknown1CA0 = 0;
    Unknown1CA4 = 0;
    DisplayName[0] = 0;
}
NW_HOOK_METHOD(0x100196f0, NItemEntry, Reset)

// 0x10006e20 (the stock keeps it among NDialogInfo's: a small record's constructor; some callers build the request
// inline) - Unknown02 is left as it is
NItemRequest::NItemRequest()
{
    Type = -1;
    ObjectID = 0;
    ItemID = 0;
    Count = 0;
    Unknown10 = 0;
    Unknown14 = 0;
    BodyPart = 0;
    Enchant = 0;
    Unknown1C = 0;
    bEquipped = 0;
    Unknown24 = 0;
    Unknown28 = 0;
    Unknown2C = -1;
}
NW_HOOK_CTOR(0x10006e20, NItemRequest)

/*-----------------------------------------------------------------------------
    NCItemWnd
-----------------------------------------------------------------------------*/

// 0x1001ad50 - 5 x 5 icons of 32 pixels
NCItemWnd::NCItemWnd()
{
    BackTex = NULL;
    SelectTex = NULL;
    ToolTipTex = NULL;
    Columns = 5;
    VisibleRows = 5;
    TopRow = 0;
    NumRows = 0;
    bDragging = 0;
    bNoDrag = 0;
    MaxItems = 25;
    CellSize = 32;
    Unknown0C8 = 16;
    Selected = -1;
    HoverIndex = -1;
}
NW_HOOK_CTOR(0x1001ad50, NCItemWnd)

// 0x1001ae70 - NumRows and bNoDrag are left as allocated
NCItemWnd::NCItemWnd(INT InColumns, INT InVisibleRows, INT InMaxItems, INT InCellSize)
{
    BackTex = NULL;
    SelectTex = NULL;
    ToolTipTex = NULL;
    Columns = InColumns;
    VisibleRows = InVisibleRows;
    MaxItems = InMaxItems;
    TopRow = 0;
    bDragging = 0;
    CellSize = InCellSize;
    Unknown0C8 = 16;
    Selected = -1;
    HoverIndex = -1;
}
NW_HOOK_CTOR_ARGS(0x1001ae70, NCItemWnd, (INT A, INT B, INT C, INT D), (A, B, C, D))

// 0x1001ae10 - Items' destructor is TArray<NItemEntry*>'s (0x10017620)
NW_HOOK_DTOR(0x1001ae10, NCItemWnd)

// 0x10019810 (vslot 25) - the base's answer as a plain 0 / 1
INT NCItemWnd::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    return NCScrollWnd::WindowProc(Msg, wParam, lParam) != 0;
}
NW_HOOK_METHOD(0x10019810, NCItemWnd, WindowProc)

// 0x10019e90 (vslot 36)
INT NCItemWnd::OnCreate()
{
    NCScrollWnd::OnCreate();
    BackTex = FindTexture(TEXT("NWindow.BlackTexture"));
    SelectTex = FindTexture(TEXT("l2ui.nwindow.item_click"));
    ToolTipTex = FindTexture(TEXT("L2UI.tooltip.mini-tool01"));
    InitScrollBar();
    return 1;
}
NW_HOOK_METHOD(0x10019e90, NCItemWnd, OnCreate)

// 0x10019ae0 - the range starts as TopRow rows, as in the stock code
void NCItemWnd::InitScrollBar()
{
    SetRepeatDelay(200);
    ScrollBar->SetRange(TopRow, VisibleRows);
    ScrollBar->SetValue(TopRow);
}
NW_HOOK_METHOD(0x10019ae0, NCItemWnd, InitScrollBar)

// 0x10019830 (vslot 37)
INT NCItemWnd::OnDestroy()
{
    guard(NCItemWnd::OnDestroy);
    Clear();
    NCScrollWnd::OnDestroy();
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x10019830, NCItemWnd, OnDestroy)

// 0x10019a60 (vslot 44) - closes the desktop's message boxes this window opened
void NCItemWnd::HideWindow()
{
    if (SecondDialogBox()->GetOwner() == Entry)
        SecondDialogBox()->HideWindow();
    if (MainDialogBox()->GetOwner() == Entry)
        MainDialogBox()->HideWindow();
    NCWnd::HideWindow();
}
NW_HOOK_METHOD(0x10019a60, NCItemWnd, HideWindow)

// 0x100198b0 - the cells count CellSize + 1 pixels here, not the CellSize + 3 of OnPaint;
// the right and bottom edges belong to the last column / row
INT NCItemWnd::HitTest(INT X, INT Y)
{
    X -= ScreenX;
    Y -= ScreenY;
    INT Cell = CellSize + 1;
    if (X < 0 || X > Columns * Cell || Y < 0 || Y > VisibleRows * Cell)
        return -1;
    INT Col = X / Cell;
    if (Col >= Columns)
        Col--;
    INT Row = Y / Cell;
    if (Row >= VisibleRows)
        Row--;
    return (TopRow + Row) * Columns + Col;
}
NW_HOOK_METHOD(0x100198b0, NCItemWnd, HitTest)

// 0x10019930
INT NCItemWnd::GetSelected()
{
    if (Selected >= 0 && Selected < Items.Num())
        return Selected;
    return -1;
}
NW_HOOK_METHOD(0x10019930, NCItemWnd, GetSelected)

// 0x10019950
INT NCItemWnd::GetItemCount()
{
    return Items.Num();
}
NW_HOOK_METHOD(0x10019950, NCItemWnd, GetItemCount)

// 0x10019960 (vslot 66) - a row per notch
INT NCItemWnd::OnMouseWheel(WPARAM wParam, LPARAM lParam)
{
    SWORD Notches = (SWORD)((SWORD)HIWORD(wParam) / 120);
    if (Notches > 0)
    {
        if (TopRow > Notches)
            TopRow -= Notches;
        else
            TopRow = 0;
    }
    else if (NumRows - TopRow + Notches > VisibleRows)
        TopRow -= Notches;
    else if (NumRows >= VisibleRows)
        TopRow = NumRows - VisibleRows;
    else
        TopRow = 0;
    ScrollBar->SetValue(TopRow);
    return 1;
}
NW_HOOK_METHOD(0x10019960, NCItemWnd, OnMouseWheel)

// 0x10019b20 - rows for the items; TopRow comes back when the last row would leave the view
void NCItemWnd::UpdateScrollRange()
{
    NumRows = Items.Num() / Columns;
    if (Items.Num() % Columns)
        NumRows++;
    if (NumRows >= VisibleRows && NumRows - TopRow < VisibleRows)
        TopRow = NumRows - VisibleRows;
    ScrollBar->SetRange(NumRows, VisibleRows);
    ScrollBar->SetValue(TopRow);
}
NW_HOOK_METHOD(0x10019b20, NCItemWnd, UpdateScrollRange)

// 0x10019b90 (vslot 69)
INT NCItemWnd::OnScrollBar(INT Value)
{
    TopRow = Value;
    return 1;
}
NW_HOOK_METHOD(0x10019b90, NCItemWnd, OnScrollBar)

// 0x10019bb0 (vslot 67)
INT NCItemWnd::ScrollUp()
{
    if (NumRows > VisibleRows)
    {
        if (--TopRow < 0)
            TopRow = 0;
        ScrollBar->SetValue(TopRow);
    }
    return 1;
}
NW_HOOK_METHOD(0x10019bb0, NCItemWnd, ScrollUp)

// 0x10019bf0 (vslot 68)
INT NCItemWnd::ScrollDown()
{
    if (NumRows > VisibleRows)
    {
        TopRow++;
        if (TopRow > NumRows - VisibleRows)
            TopRow = NumRows - VisibleRows;
        ScrollBar->SetValue(TopRow);
    }
    return 1;
}
NW_HOOK_METHOD(0x10019bf0, NCItemWnd, ScrollDown)

// 0x10019c40 (vslot 72) - `ret 0x10` shared with other empty four-argument methods
void NCItemWnd::DrawItemToolTip(NCGDevice* Device, NItemEntry* Item, INT X, INT Y)
{
}
NW_HOOK_METHOD(0x10019c40, NCItemWnd, DrawItemToolTip)

// 0x10019ee0 (vslot 55) - the clip opens a pixel around the window before BeginPaint and
// closes after EndPaint; the rows stop at VisibleRows * (CellSize + 1) pixels
INT NCItemWnd::OnPaint(NCGDevice* Device)
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
            Device->DrawTexture(X - 1, Y - 1, 34, 34, 0, 0, 34, 34, SelectTex);
        if (Item->GetIcon())
            Device->DrawTexture(X, Y, CellSize, CellSize, 0, 0, CellSize, CellSize, Item->GetIcon());
    }
    Device->EndPaint();
    Device->ReleaseClip();
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x10019ee0, NCItemWnd, OnPaint)

// 0x1001b2d0 (vslot 56) - C2 draws the tool tip with the window's skin: over the item under the mouse the
// skin gets a copy of its tool tip object by value, the cell's position in the window (X and Y, which the
// routine moves to keep the box on the screen) and the window's rectangle. The selected item's box is drawn
// by 0x10029fa0, any other item's by 0x1002a230 (NToolTipSkin::Draw)
INT NCItemWnd::OnDrawToolTip(NCGDevice* Device)
{
    Device->BeginPaint(ScreenX, ScreenY);
    Device->SetClip(ScreenX, ScreenY, Width, Height);
    if (HoverIndex >= 0 && HoverIndex < Items.Num())
    {
        INT Index = HoverIndex - TopRow * Columns;
        INT Y = (CellSize + 3) * (Index / Columns);
        INT X = (CellSize + 3) * (Index % Columns);
        NItemEntry* Item = Items(HoverIndex);
        if (Item->IsSelected())
        {
            if (ToolTipSkin)
            {
                NToolTipRect Wnd;
                Wnd.X = ScreenX;
                Wnd.Y = ScreenY;
                Wnd.W = Width;
                Wnd.H = Height;
                ToolTipSkin->DrawFull(Device, Item->ToolTip, &X, &Y, Wnd);
            }
        }
        else if (ToolTipSkin)
        {
            NToolTipRect Wnd;
            Wnd.X = ScreenX;
            Wnd.Y = ScreenY;
            Wnd.W = Width;
            Wnd.H = Height;
            ToolTipSkin->Draw(Device, Item->ToolTip, &X, &Y, Wnd);
        }
    }
    Device->ReleaseClip();
    Device->EndPaint();
    return 1;
}
NW_HOOK_METHOD(0x1001b2d0, NCItemWnd, OnDrawToolTip)

// 0x1001a090 (vslot 61) - selects the item under the mouse and picks it up (unless bNoDrag, which lasts one
// click); the desktop's message box closes, and keeps the focus if still up.
INT NCItemWnd::OnLButtonDown(WPARAM wParam, LPARAM lParam)
{
    INT Index = HitTest(LOWORD(lParam), HIWORD(lParam));
    Selected = Index;
    HoverIndex = Index;
    if (Index >= 0 && Index < Items.Num())
    {
        UnselectAll();
        Items(Selected)->Select();
        if (!bNoDrag)
        {
            if (MainDialogBox()->IsShowWindow())
            {
                MainDialogBox()->Reset();
                MainDialogBox()->HideWindow();
            }
            bDragging = 1;
            PlayInterfaceSound(SOUND_PickUp);
        }
        else
            bNoDrag = 0;
        if (MainDialogBox()->IsShowWindow())
            MainDialogBox()->SetFocus();
    }
    return 1;
}
NW_HOOK_METHOD(0x1001a090, NCItemWnd, OnLButtonDown)

// 0x1001a1a0 (vslot 60) - while an item is carried the cursor is its icon (type 4: a color
// cursor built from the texture); otherwise the arrow comes back, unless Ctrl is held
INT NCItemWnd::OnMouseMove(WPARAM wParam, LPARAM lParam)
{
    HoverIndex = HitTest(LOWORD(lParam), HIWORD(lParam));
    if (IsFocused())
    {
        if (bDragging)
        {
            if (Selected >= 0 && Selected < Items.Num())
            {
                SetCursor(4, Items(Selected)->GetIcon());
                LockCursor();
                return 1;
            }
        }
        else if (!GConsole->IsCtrlDown())
        {
            UnlockCursor();
            SetCursor(0, NULL);
        }
    }
    return 1;
}
NW_HOOK_METHOD(0x1001a1a0, NCItemWnd, OnMouseMove)

// 0x1001a240 (vslot 71)
void NCItemWnd::UnselectAll()
{
    for (INT i = 0; i < Items.Num(); i++)
        Items(i)->Unselect();
}
NW_HOOK_METHOD(0x1001a240, NCItemWnd, UnselectAll)

// 0x1001af00 (vslot 70) - the entries are deleted (~NItemEntry, then the memory goes back to GMalloc)
void NCItemWnd::Clear()
{
    for (INT i = 0; i < Items.Num(); i++)
        if (Items(i))
            delete Items(i);
    Items.Empty();
    TopRow = 0;
    NumRows = 0;
    Selected = -1;
    HoverIndex = -1;
}
NW_HOOK_METHOD(0x1001af00, NCItemWnd, Clear)

NW_CHECK_VTABLE(NCItemWnd, 0x1010d940, 73)
