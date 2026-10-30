/*=============================================================================
    NCTrainListBox.cpp - trainer skill list. Stock: 0x100ba9a0 .. 0x100baad5,
    0x100baed0 .. 0x100bb49c, 0x100bc570 .. 0x100bc79f, 0x100bcb40 .. 0x100bcc42; bodies
    shared with NCVIPShopItemListBox at 0x1006bad0 and 0x1006dda0.
=============================================================================*/
#include "nwindow/NCTrainListBox.h"
#include "nwindow/NCButton.h"
#include "nwindow/NCSliderBar.h"
#include "nwindow/NCGDevice.h"

// 0x100ddef0 (no C1 match: C1 0x100bcb40)
NCTrainListBox::NCTrainListBox()
{
    LineGap = 4;
}
NW_HOOK_CTOR(0x100ddef0, NCTrainListBox)

// 0x100ddfb0 (no C1 match: C1 0x100bcc00)
NCTrainListBox::NCTrainListBox(INT InMaxLines, INT InVisibleLines, INT Unused)
    : NCListBox(InMaxLines, InVisibleLines, Unused)
{
    LineGap = 4;
}
NW_HOOK_CTOR_ARGS(0x100ddfb0, NCTrainListBox, (INT A, INT B, INT C), (A, B, C))

// 0x100ddf50 - the entries' array (0x100dde60), then NCListBox (its scalar deleting destructor, 0x100ddf20, is slot 3 and
// calls this)
NW_HOOK_DTOR(0x100ddf50, NCTrainListBox)


// stock body shared with NCDebugWnd's (0x100576d0)
INT NCTrainListBox::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (Msg != WM_PAINT && !IsEnabled())
        return 0;
    return NCListBox::WindowProc(Msg, wParam, lParam) != 0;
}

// 0x100dc160 (vslot 36)
INT NCTrainListBox::OnCreate()
{
    NCListBox::OnCreate();
    BackTex = FindTexture(TEXT("sek.cbui315"));
    return 1;
}
NW_HOOK_METHOD(0x100dc160, NCTrainListBox, OnCreate)

// 0x100ddc20 (vslot 37; C1 0x1006dda0, shared there with NCVIPShopItemListBox; C2: that one has its own)
INT NCTrainListBox::OnDestroy()
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
NW_HOOK_METHOD(0x100ddc20, NCTrainListBox, OnDestroy)

// 0x100dc540 (vslot 50) - icon, name, "Lv <level>" and, from x = 76, "Required SP : <sp>".
// A NULL entry skips only the icon and the name, as in the stock code.
INT NCTrainListBox::OnPaint(NCGDevice* Device)
{
    guard(NCTrainListBox::OnPaint);
    // Stock layout: a 16-character buffer followed by the text size. With the English strings
    // the terminator of "Unacquired state" (16 characters) lands in Size.cx, so that text is
    // drawn at TextX + 36, over "Lv"; kept in one struct so the overflow stays harmless.
    struct
    {
        TCHAR Text[16];
        SIZE Size;
    } T;
    Device->BeginPaint(ScreenX, ScreenY);
    Device->SetClip(ScreenX, ScreenY, Width, Height);
    INT Line = 0;
    for (INT i = TopIndex; i < VisibleLines + TopIndex && i <= NumLines && i < Entries.Num();
         i++, Line++)
    {
        NTrainEntry* Entry = Entries(i);
        if (i == Selected && HighlightTex)
            Device->DrawStretchedTex(0, (LineGap + 32) * Line, Width - 18, 32, HighlightTex);
        if (Entry)
        {
            if (Entry->Icon)
                Device->DrawTexture(TextX, (LineGap + 32) * Line + TextY, 32, 32, 0, 0, 32, 32,
                                    Entry->Icon);
            if (Entry->Name[0])
                Device->DrawText(TextX + 34, (LineGap + 32) * Line + TextY + 2, 0xFFFFFFFF,
                                 Entry->Name, 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
        }

        // "Lv" (SysString 88), then the level or "Unacquired state" (371)
        if (*SysString(88))
            appSprintf(T.Text, TEXT("%s"), *SysString(88));
        else
            T.Text[0] = 0;
        Device->DrawText(TextX + 34, (LineGap + 32) * Line + TextY + 16, 0xFFA3A3A3, T.Text, 0, 0,
                         0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
        NCGDevice::GetTextExtent(&T.Size, T.Text, 0, 0, 0, 0, 0);
        if (Entry->Level > 0)
            appSprintf(T.Text, TEXT("%d"), Entry->Level);
        else if (*SysString(371))
            appSprintf(T.Text, TEXT("%s"), *SysString(371));
        else
            T.Text[0] = 0;
        Device->DrawText(TextX + 34 + T.Size.cx + 2, (LineGap + 32) * Line + TextY + 16,
                         0xFFB09B79, T.Text, 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);

        // "Required SP" (365), " : " and the SP
        if (*SysString(365))
            appSprintf(T.Text, TEXT("%s"), *SysString(365));
        else
            T.Text[0] = 0;
        Device->DrawText(76, (LineGap + 32) * Line + TextY + 16, 0xFFA3A3A3, T.Text, 0, 0, 0,
                         L2FONT_Normal, 0, 0, 0, 0, 0, 0);
        NCGDevice::GetTextExtent(&T.Size, T.Text, 0, 0, 0, 0, 0);
        appSprintf(T.Text, TEXT(" : "));
        Device->DrawText(76 + T.Size.cx, (LineGap + 32) * Line + TextY + 16, 0xFFFFFFFF, T.Text, 0,
                         0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
        if (*SysString(365))
            appSprintf(T.Text, TEXT("%s : "), *SysString(365));
        else
            T.Text[0] = 0;
        NCGDevice::GetTextExtent(&T.Size, T.Text, 0, 0, 0, 0, 0);
        appSprintf(T.Text, TEXT("%d"), Entry->SP);
        Device->DrawText(76 + T.Size.cx, (LineGap + 32) * Line + TextY + 16, 0xFFB09B79, T.Text, 0,
                         0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
    }
    Device->ReleaseClip();
    Device->EndPaint();
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x100dc540, NCTrainListBox, OnPaint)

// 0x10079050 (vslot 61)
INT NCTrainListBox::OnLButtonDown(WPARAM wParam, LPARAM lParam)
{
    Selected = HitTest(LOWORD(lParam), HIWORD(lParam));
    if (Selected >= 0 && Selected < Entries.Num())
        NotifyParent(Selected);
    return 1;
}
NW_HOOK_METHOD(0x10079050, NCTrainListBox, OnLButtonDown)

// 0x10084690 (vslot 68) - NCListBox::HitTest with 32 px rows
INT NCTrainListBox::HitTest(INT X, INT Y)
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
NW_HOOK_METHOD(0x10084690, NCTrainListBox, HitTest)

// vslot 70: the stock slot is 0x100c3310, the body shared with NCHennaListBox's (hooked there): the rows freed, the
// scroll bar and the view reset
void NCTrainListBox::Clear()
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
__declspec(noinline) void NCTrainListBox::UpdateEntryScrollBar()
{
    NumLines = Entries.Num();
    ScrollBar->SetRange(NumLines, VisibleLines);
    ScrollBar->SetValue(TopIndex);
}

// 0x100ddc90 - keeps a copy; the icon comes from Src's texture name (the black texture when
// it is not found). Unlike NCListBox::AddString nothing is dropped when full.
void NCTrainListBox::AddEntry(const NTrainEntry* Src)
{
    NTrainEntry* Entry = new NTrainEntry;
    Entry->Copy(Src);
    if (Src->IconName)
        Entry->Icon = FindTexture(Src->IconName);
    if (!Entry->Icon)
        Entry->Icon = FindTexture(TEXT("NWindow.BlackTexture"));
    if (NumLines < MaxLines)
    {
        NumLines++;
        if (bAutoScroll && NumLines > VisibleLines)
            TopIndex++;
    }
    INT i = Entries.Add();
    Entries(i) = Entry;
    UpdateEntryScrollBar();
}
NW_HOOK_METHOD(0x100ddc90, NCTrainListBox, AddEntry)

// 0x100dca20
UBOOL NCTrainListBox::GetSelectedEntry(NTrainEntry* Out)
{
    if (Selected < 0 || Selected >= Entries.Num())
        return 0;
    NTrainEntry* Entry = Entries(Selected);
    if (!Entry)
        return 0;
    Out->Copy(Entry);
    return 1;
}
NW_HOOK_METHOD(0x100dca20, NCTrainListBox, GetSelectedEntry)

NW_CHECK_VTABLE(NCTrainListBox, 0x10127c50, 75)
