/*=============================================================================
    NCVIPShopItemListBox.cpp - VIP shop item list. Stock: 0x100861a0 .. 0x100864ff,
    0x10086cc0 .. 0x10086d52, 0x10086d90 .. 0x10086e9f, 0x10087610 .. 0x100876df; the other
    bodies are identical to NCTrainListBox's and folded with them (hooked in NCTrainListBox.cpp).
=============================================================================*/
#include "nwindow/NCVIPShopItemListBox.h"
#include "nwindow/NCButton.h"
#include "nwindow/NCSliderBar.h"
#include "nwindow/NCGDevice.h"
#include "nwindow/NConsoleWnd.h"

// 0x10087610
NCVIPShopItemListBox::NCVIPShopItemListBox()
{
    LineGap = 4;
}
NW_HOOK_CTOR(0x10087610, NCVIPShopItemListBox)

// 0x100876d0
NCVIPShopItemListBox::NCVIPShopItemListBox(INT InMaxLines, INT InVisibleLines, INT Unused)
    : NCListBox(InMaxLines, InVisibleLines, Unused)
{
    LineGap = 4;
}
NW_HOOK_CTOR_ARGS(0x100876d0, NCVIPShopItemListBox, (INT A, INT B, INT C), (A, B, C))

// 0x10087670
NW_HOOK_DTOR(0x10087670, NCVIPShopItemListBox)

// stock body shared with NCDebugWnd's (0x100576d0)
INT NCVIPShopItemListBox::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (Msg != WM_PAINT && !IsEnabled())
        return 0;
    return NCListBox::WindowProc(Msg, wParam, lParam) != 0;
}

// stock body shared with NCTrainListBox's (0x100ba9a0)
INT NCVIPShopItemListBox::OnCreate()
{
    NCListBox::OnCreate();
    BackTex = FindTexture(TEXT("sek.cbui315"));
    return 1;
}

// 0x10086cc0 (vslot 37) - C2: its own copy of NCTrainListBox's (0x100ddc20)
INT NCVIPShopItemListBox::OnDestroy()
{
    for (INT i = 0; i < Entries.Num(); i++)
    {
        if (Entries(i))
            appFree(Entries(i));
    }
    Entries.Empty();
    NCListBox::OnDestroy();
    return 1;
}
NW_HOOK_METHOD(0x10086cc0, NCVIPShopItemListBox, OnDestroy)

// 0x100861a0 (vslot 55) - icon, name (then "-" and the item's additional name in gold when it
// has one), description and, for type 5, "X <count>" over the description. The guard names
// NCTrainListBox::OnPaint (copied code).
INT NCVIPShopItemListBox::OnPaint(NCGDevice* Device)
{
    guard(NCTrainListBox::OnPaint);
    TCHAR Text[2048];
    SIZE Size;
    TCHAR Count[64];
    Device->BeginPaint(ScreenX, ScreenY);
    Device->SetClip(ScreenX, ScreenY, Width, Height);
    INT Line = 0;
    for (INT i = TopIndex; i < VisibleLines + TopIndex && i <= NumLines && i < Entries.Num();
         i++, Line++)
    {
        NVIPShopEntry* Entry = Entries(i);
        if (i == Selected && HighlightTex)
            Device->DrawStretchedTex(0, (LineGap + 32) * Line, Width - 18, 32, HighlightTex);
        if (Entry && Entry->Icon)
            Device->DrawTexture(TextX, (LineGap + 32) * Line + TextY, 32, 32, 0, 0, 32, 32,
                                Entry->Icon);
        if (Entry->Name[0])
            Device->DrawText(TextX + 34, (LineGap + 32) * Line + TextY + 2, 0xFFFFFFFF, Entry->Name,
                             0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
        // 0x10057730 / 0x100578a0 (the console's): the item's additional name is there / is
        if (GConsole->HasItemAdditionalName(Entry->ItemID))
        {
            NCGDevice::GetTextExtent(&Size, Entry->Name, 0, 0, 0, 0, 0);
            Device->DrawText(Size.cx + TextX + 34, (LineGap + 32) * Line + TextY + 2, 0xFFA0A0A0,
                             TEXT("-"), 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
            appSprintf(Text, TEXT("%s"), GConsole->GetItemAdditionalName(Entry->ItemID));
            Device->DrawText(Size.cx + TextX + 46, (LineGap + 32) * Line + TextY + 2, 0xFFFFD969,
                             Text, 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
        }
        if (Entry->Desc[0])
            Device->DrawText(TextX + 34, (LineGap + 32) * Line + TextY + 16, 0xFFB09B79,
                             Entry->Desc, 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
        if (Entry->Type == 5)
        {
            appSprintf(Count, TEXT("X %d"), Entry->Count);
            Device->DrawText(TextX + 34, (LineGap + 32) * Line + TextY + 16, 0xFFFFFFFF, Count, 0,
                             0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
        }
    }
    Device->ReleaseClip();
    Device->EndPaint();
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x100861a0, NCVIPShopItemListBox, OnPaint)

// stock body shared with NCTrainListBox's (0x1006bad0)
INT NCVIPShopItemListBox::OnLButtonDown(WPARAM wParam, LPARAM lParam)
{
    Selected = HitTest(LOWORD(lParam), HIWORD(lParam));
    if (Selected >= 0 && Selected < Entries.Num())
        NotifyParent(Selected);
    return 1;
}

// stock body shared with NCTrainListBox's (0x100ba9d0)
INT NCVIPShopItemListBox::HitTest(INT X, INT Y)
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
        if (RX < Width && RY >= (LineGap + 32) * Line + TextY &&
            RY < (Line + 1) * (LineGap + 32) + TextY)
        {
            Selected = i;
            break;
        }
    }
    return Selected;
}

// stock body shared with NCTrainListBox's (0x100bc570)
void NCVIPShopItemListBox::Clear()
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

// 0x100c1110 - the list boxes' shared body (hooked on NCRecipeListBox): the array's count into the scroll bar
__declspec(noinline) void NCVIPShopItemListBox::UpdateEntryScrollBar()
{
    NumLines = Entries.Num();
    ScrollBar->SetRange(NumLines, VisibleLines);
    ScrollBar->SetValue(TopIndex);
}

// 0x10086d90 - keeps a copy (empty strings are not copied; the icon is Src's)
void NCVIPShopItemListBox::AddEntry(const NVIPShopEntry* Src)
{
    NVIPShopEntry* Entry = new NVIPShopEntry;
    if (Src->Name[0])
        appStrcpy(Entry->Name, Src->Name);
    if (Src->Desc[0])
        appStrcpy(Entry->Desc, Src->Desc);
    Entry->ProductID = Src->ProductID;
    Entry->Icon = Src->Icon;
    Entry->Count = Src->Count;
    Entry->Type = Src->Type;
    Entry->ItemID = Src->ItemID;
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
NW_HOOK_METHOD(0x10086d90, NCVIPShopItemListBox, AddEntry)

NW_CHECK_VTABLE(NCVIPShopItemListBox, 0x10119de0, 75)
