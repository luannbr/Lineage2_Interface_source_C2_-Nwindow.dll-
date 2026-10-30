/*=============================================================================
    NCHennaListBox.cpp - the tattoo list of the henna windows. Stock: 0x10079510 .. 0x10079772 (paint),
    0x100797b0 .. 0x10079892, 0x1007ac90 .. 0x1007af8f (OnDestroy, AddEntry, the destructor of the array),
    0x1007af90 .. 0x1007b04f (constructors, destructor), 0x1007b050, 0x100c1040 .. 0x100c1102, 0x100c3310.
=============================================================================*/
#include "nwindow/NCHennaListBox.h"
#include "nwindow/NCButton.h"
#include "nwindow/NCSliderBar.h"
#include "nwindow/NCGDevice.h"

#include <wchar.h>

// 0x1007af90 - the array is empty, the rest is the base's
NCHennaListBox::NCHennaListBox()
{
}
NW_HOOK_CTOR(0x1007af90, NCHennaListBox)

// 0x1007b050
NCHennaListBox::NCHennaListBox(INT InMaxLines, INT InVisibleLines, INT Unused)
    : NCListBox(InMaxLines, InVisibleLines, Unused)
{
}
NW_HOOK_CTOR_ARGS(0x1007b050, NCHennaListBox, (INT A, INT B, INT C), (A, B, C))

// 0x1007aff0 - Entries' destructor is TArray<T*>'s (0x1007af00)
NW_HOOK_DTOR(0x1007aff0, NCHennaListBox)

// stock body shared with NCChatListBox's (0x100dc140)
INT NCHennaListBox::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (Msg != WM_PAINT && !IsEnabled())
        return 0;
    return NCListBox::WindowProc(Msg, wParam, lParam) != 0;
}

// stock body shared with NCDebugWnd's (0x1006e930)
INT NCHennaListBox::OnCreate()
{
    NCListBox::OnCreate();
    return 1;
}

// 0x1007ac90 (vslot 37)
INT NCHennaListBox::OnDestroy()
{
    guard(NCHennaListBox::OnDestroy);
    for (INT i = 0; i < Entries.Num(); i++)
    {
        if (Entries(i))
            appFree(Entries(i));
    }
    Entries.Empty();
    NCListBox::OnDestroy();
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x1007ac90, NCHennaListBox, OnDestroy)

// 0x10079510 (vslot 55) - rows of 43 pixels: the icon (32 pixels, 4 down), the name, the description and "Price <n>"
INT NCHennaListBox::OnPaint(NCGDevice* Device)
{
    guard(NCHennaListBox::OnPaint);
    TCHAR Text[36];
    Device->BeginPaint(ScreenX, ScreenY);
    Device->SetClip(ScreenX, ScreenY, Width, Height);
    INT Line = 0;
    for (INT i = TopIndex; i < VisibleLines + TopIndex && i <= NumLines && i < Entries.Num();
         i++, Line++)
    {
        NHennaListEntry* Entry = Entries(i);
        if (i == Selected && HighlightTex)
            Device->DrawStretchedTex(2, (LineGap + 43) * Line + TextY, Width - 18, 43, HighlightTex);
        if (Entry)
        {
            if (Entry->Icon)
                Device->DrawTexture(TextX, (LineGap + 43) * Line + 4 + TextY, 32, 32, 0, 0, 32, 32,
                                    Entry->Icon);
            if (Entry->Name[0])
                Device->DrawText(TextX + 42, (LineGap + 43) * Line + 2 + TextY, 0xFFFFFFFF, Entry->Name,
                                 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
        }
        if (Entry->Description[0])
            Device->DrawText(TextX + 42, (LineGap + 43) * Line + 16 + TextY, 0xFFFFFFFF,
                             Entry->Description, 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
        appSprintf(Text, TEXT("%s %d"), *SysString(637), Entry->Price);
        Device->DrawText(TextX + 42, (LineGap + 43) * Line + 30 + TextY, 0xFFFFFFFF, Text, 0, 0, 0,
                         L2FONT_Normal, 0, 0, 0, 0, 0, 0);
    }
    Device->ReleaseClip();
    Device->EndPaint();
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x10079510, NCHennaListBox, OnPaint)

// stock body shared with NCTrainListBox's (0x10079050)
INT NCHennaListBox::OnLButtonDown(WPARAM wParam, LPARAM lParam)
{
    Selected = HitTest(LOWORD(lParam), HIWORD(lParam));
    if (Selected >= 0 && Selected < Entries.Num())
        NotifyParent(Selected);
    return 1;
}

// 0x100c1040 (vslot 68) - NCListBox::HitTest with rows of 43 pixels (+ LineGap)
INT NCHennaListBox::HitTest(INT X, INT Y)
{
    INT RX = X - ScreenX;
    INT RY = Y - ScreenY;
    if (RX < 0 || RX > Width || RY < 0 || RY > Height)
        return -1;
    INT Line = 0;
    for (INT i = TopIndex; i < VisibleLines + TopIndex; i++, Line++)
    {
        if (i >= NumLines)
            break;
        if (RX < Width && RY >= (LineGap + 43) * Line + TextY &&
            RY < (Line + 1) * (LineGap + 43) + TextY)
        {
            Selected = i;
            break;
        }
    }
    return Selected;
}
NW_HOOK_METHOD(0x100c1040, NCHennaListBox, HitTest)

// 0x100c3310 (vslot 70) - the rows deleted, the scroll bar and the view reset (the same code as NCTrainListBox's and
// NCVIPShopItemListBox's, whose stubs jump here)
void NCHennaListBox::Clear()
{
    for (INT i = 0; i < Entries.Num(); i++)
    {
        if (Entries(i))
            appFree(Entries(i));
    }
    Entries.Empty();
    ScrollBar->Reset();
    TopIndex = 0;
    NumLines = 0;
    Selected = -1;
}
NW_HOOK_METHOD(0x100c3310, NCHennaListBox, Clear)

// 0x100c1110 - the list boxes' shared body (hooked on NCRecipeListBox): the array's count into the scroll bar
__declspec(noinline) void NCHennaListBox::UpdateEntryScrollBar()
{
    NumLines = Entries.Num();
    ScrollBar->SetRange(NumLines, VisibleLines);
    ScrollBar->SetValue(TopIndex);
}

// 0x1007ad60 - keeps a copy (the strings with wcscpy, the static CRT's, and only when they are not empty; the icon is found by its name, the black texture when it is
// not there); nothing is dropped when full, as in NCTrainListBox
void NCHennaListBox::AddEntry(const NHennaListEntry* Src)
{
    NHennaListEntry* Entry = new NHennaListEntry;
    if (Src->Name[0])
        wcscpy(Entry->Name, Src->Name);
    if (Src->IconName[0])
        wcscpy(Entry->IconName, Src->IconName);
    if (Src->Description[0])
        wcscpy(Entry->Description, Src->Description);
    Entry->ItemID = Src->ItemID;
    Entry->Count = Src->Count;
    Entry->Price = Src->Price;
    Entry->bCanUse = Src->bCanUse;
    Entry->HennaID = Src->HennaID;
    Entry->Unknown1418 = Src->Unknown1418;
    if (Entry->IconName)
        Entry->Icon = FindTexture(Entry->IconName);
    if (!Entry->Icon)
        Entry->Icon = FindTexture(TEXT("NWindow.BlackTexture"));
    if (NumLines < MaxLines)
    {
        NumLines++;
        if (bAutoScroll && NumLines > VisibleLines)
            TopIndex++;
    }
    INT Index = Entries.Add();
    Entries(Index) = Entry;
    UpdateEntryScrollBar();
}
NW_HOOK_METHOD(0x1007ad60, NCHennaListBox, AddEntry)

// 0x100797b0 - a copy of the selected row (the strings only when they are not empty); 0 when no row is selected
UBOOL NCHennaListBox::GetSelectedEntry(NHennaListEntry* Out)
{
    if (Selected >= 0 && Selected < Entries.Num() && Entries(Selected) != NULL)
    {
        NHennaListEntry* Src = Entries(Selected);
        if (Src->Name[0])
            wcscpy(Out->Name, Src->Name);
        if (Src->IconName[0])
            wcscpy(Out->IconName, Src->IconName);
        if (Src->Description[0])
            wcscpy(Out->Description, Src->Description);
        if (Src->Icon)
            Out->Icon = Src->Icon;
        Out->HennaID = Src->HennaID;
        Out->ItemID = Src->ItemID;
        Out->Count = Src->Count;
        Out->Price = Src->Price;
        Out->bCanUse = Src->bCanUse;
        Out->Unknown1418 = Src->Unknown1418;
        return 1;
    }
    return 0;
}
NW_HOOK_METHOD(0x100797b0, NCHennaListBox, GetSelectedEntry)

NW_CHECK_VTABLE(NCHennaListBox, 0x101184b8, 75)
