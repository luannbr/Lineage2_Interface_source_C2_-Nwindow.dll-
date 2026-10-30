/*=============================================================================
    NCRecipeListBox.cpp - the recipe list of the dwarf's shop (new in C2).
    Stock: 0x100c1a00 (OnPaint), 0x100c1ef0 (GetSelectedEntry), 0x100c3390 (AddEntry), 0x100c41e0 .. 0x100c42e2
    (constructors, destructor), 0x100c37c0 (the entries array's destructor, which the compiler inlines here),
    0x10057150 (NFormatThousands).
    WindowProc, OnCreate, OnDestroy, OnLButtonDown, HitTest and Clear are bodies shared with the NCTrainListBox /
    NCVIPShopItemListBox / henna list code (hooked there); UpdateEntryScrollBar (0x100c1110) is shared the same way
    and hooked here.
=============================================================================*/
#include "nwindow/NCRecipeListBox.h"
#include "nwindow/NCButton.h"
#include "nwindow/NCSliderBar.h"
#include "nwindow/NCGDevice.h"

enum { RECIPE_ROW_HEIGHT = 43 };   // a row is this plus LineGap high
enum
{
    STR_Price = 641,    // sysstring (GL2GameData + 0x4D2F4): the price line's label, in yellow
    STR_Adena = 469,    // (+0x4CAE4) after the amount
    STR_Rate = 642,     // (+0x4D300): the success rate line's label, in grey
};

// 0x10057150 (stdcall, ret 8) - Value's decimal digits with a comma between thousands (a sign is counted as a digit, so a
// negative amount of 4 characters gets its comma in front of the sign); the stock code builds the digits in a local buffer
// first
void __stdcall NFormatThousands(TCHAR* Out, INT Value)
{
    TCHAR Digits[128];
    appStrcpy(Digits, appItoa(Value));
    INT Len = (INT)wcslen(Digits);
    INT Index = Len - 1;
    INT OutLen = Index / 3 + Len;
    Out[OutLen] = 0;
    INT Counter = 0;
    TCHAR* P = Out + OutLen - 1;
    while (Index >= 0)
    {
        if (Counter % 3 == 0 && Index < (INT)wcslen(Digits) - 1)
        {
            *P = ',';
            P--;
        }
        *P = Digits[Index];
        Index--;
        Counter++;
        P--;
    }
}
NW_HOOK_FUNC(0x10057150, NFormatThousands)

// 0x1002bb70 (stdcall, ret 4; NCEditBox::OnPaint calls it too) - the color an amount's text is drawn in by the number of its
// characters that are not commas: white (-1) up to four, then by (count - 2) % 4: 0xFFFF80FF (6, 10, ..), yellow (7), green (8),
// cyan (5, 9)
DWORD __stdcall NAmountColor(const TCHAR* Text)
{
    INT Len = appStrlen(Text);
    INT Commas = 0;
    for (INT i = 0; i < Len; i++)
    {
        if (Text[i] == ',')
            Commas++;
    }
    INT Digits = Len - Commas;
    if (Digits < 5)
        return 0xFFFFFFFF;
    switch ((Digits - 2) % 4)
    {
    case 0:
        return 0xFFFF80FF;
    case 1:
        return 0xFFFFFF00;
    case 2:
        return 0xFF00FF00;
    case 3:
        return 0xFF00FFFF;
    }
    return 0xFFFFFFFF;
}
NW_HOOK_FUNC(0x1002bb70, NAmountColor)

// 0x100c41e0
NCRecipeListBox::NCRecipeListBox()
{
    LineGap = 4;
}
NW_HOOK_CTOR(0x100c41e0, NCRecipeListBox)

// 0x100c42a0
NCRecipeListBox::NCRecipeListBox(INT InMaxLines, INT InVisibleLines, INT Unused)
    : NCListBox(InMaxLines, InVisibleLines, Unused)
{
    LineGap = 4;
}
NW_HOOK_CTOR_ARGS(0x100c42a0, NCRecipeListBox, (INT A, INT B, INT C), (A, B, C))

// 0x100c4240
NW_HOOK_DTOR(0x100c4240, NCRecipeListBox)

// stock body shared with NCChatListBox's (0x100dc140)
INT NCRecipeListBox::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (Msg != WM_PAINT && !IsEnabled())
        return 0;
    return NCListBox::WindowProc(Msg, wParam, lParam) != 0;
}

// stock body shared with NCTrainListBox's (0x100dc160)
INT NCRecipeListBox::OnCreate()
{
    NCListBox::OnCreate();
    BackTex = FindTexture(TEXT("sek.cbui315"));
    return 1;
}

// stock body shared with NCTrainListBox's (0x100ddc20)
INT NCRecipeListBox::OnDestroy()
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

// stock body shared with NCTrainListBox's (0x10079050)
INT NCRecipeListBox::OnLButtonDown(WPARAM wParam, LPARAM lParam)
{
    Selected = HitTest(LOWORD(lParam), HIWORD(lParam));
    if (Selected >= 0 && Selected < Entries.Num())
        NotifyParent(Selected);
    return 1;
}

// stock body shared with the henna list's (0x100c1040): the row under the point, -1 outside the box
INT NCRecipeListBox::HitTest(INT X, INT Y)
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
        if (RX < Width && RY >= (LineGap + RECIPE_ROW_HEIGHT) * Line + TextY &&
            RY < (Line + 1) * (LineGap + RECIPE_ROW_HEIGHT) + TextY)
        {
            Selected = i;
            break;
        }
    }
    return Selected;
}

// stock body shared with the henna list's (0x100c3310)
void NCRecipeListBox::Clear()
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

// 0x100c1110 - the list boxes' shared body (NCVIPShopItemListBox, NCTrainListBox and the henna list call the same address)
__declspec(noinline) void NCRecipeListBox::UpdateEntryScrollBar()
{
    NumLines = Entries.Num();
    ScrollBar->SetRange(NumLines, VisibleLines);
    ScrollBar->SetValue(TopIndex);
}
NW_HOOK_METHOD(0x100c1110, NCRecipeListBox, UpdateEntryScrollBar)

// 0x100c1ef0 - the selected row's texts and numbers copied into Out (the texts and the icon only when the row has
// them); 0 with no row selected
__declspec(noinline) UBOOL NCRecipeListBox::GetSelectedEntry(NRecipeRow* Out)
{
    if (Selected < 0 || Selected >= Entries.Num())
        return 0;
    NRecipeRow* Row = Entries(Selected);
    if (!Row)
        return 0;
    if (Row->Name[0])
        wcscpy(Out->Name, Row->Name);
    if (Row->IconName[0])
        wcscpy(Out->IconName, Row->IconName);
    if (Row->Description[0])
        wcscpy(Out->Description, Row->Description);
    if (Row->Icon)
        Out->Icon = Row->Icon;
    Out->RecipeID = Row->RecipeID;
    Out->Unknown0A08 = Row->Unknown0A08;
    Out->SuccessRate = Row->SuccessRate;
    Out->Price = Row->Price;
    Out->Unknown0A18 = Row->Unknown0A18;
    return 1;
}
NW_HOOK_METHOD(0x100c1ef0, NCRecipeListBox, GetSelectedEntry)

// 0x100c3390 - a new row, a copy of Src (the texts only when Src has them, the icon found by its name, or
// "NWindow.BlackTexture"); the list counts it (up to MaxLines; scrolled when it keeps the last row in view)
__declspec(noinline) void NCRecipeListBox::AddEntry(const NRecipeRow* Src)
{
    NRecipeRow* Row = (NRecipeRow*)appMalloc(sizeof(NRecipeRow), TEXT("new"));
    if (Row)
    {
        Row->Name[0] = 0;
        Row->IconName[0] = 0;
        Row->Description[0] = 0;
        Row->Icon = NULL;
        Row->RecipeID = -1;
        Row->Unknown0A08 = -1;
        Row->SuccessRate = 0;
        Row->Unknown0A14 = 0;
        Row->Unknown0A18 = 0;
        Row->Unknown0A1C = -1;
    }
    if (Src->Name[0])
        wcscpy(Row->Name, Src->Name);
    if (Src->IconName[0])
        wcscpy(Row->IconName, Src->IconName);
    if (Src->Description[0])
        wcscpy(Row->Description, Src->Description);
    Row->RecipeID = Src->RecipeID;
    Row->Unknown0A08 = Src->Unknown0A08;
    Row->Price = Src->Price;
    Row->Unknown0A18 = Src->Unknown0A18;
    Row->SuccessRate = Src->SuccessRate;
    Row->Icon = FindTexture(Row->IconName);
    if (!Row->Icon)
        Row->Icon = FindTexture(TEXT("NWindow.BlackTexture"));
    if (NumLines < MaxLines)
    {
        NumLines++;
        if (bAutoScroll && NumLines > VisibleLines)
            TopIndex++;
    }
    Entries.AddItem(Row);
    UpdateEntryScrollBar();
}
NW_HOOK_METHOD(0x100c3390, NCRecipeListBox, AddEntry)

// 0x100c1a00 (vslot 55) - the visible rows: the highlight under the selected one, the icon, the name, then the price
// line ("<label> : <amount> <Adena>", the amount colored by its size) and the success rate line ("<label> : <n>%")
INT NCRecipeListBox::OnPaint(NCGDevice* Device)
{
    TCHAR Rate[128];
    TCHAR Amount[132];
    SIZE Size;
    INT Row = 0;
    Device->BeginPaint(ScreenX, ScreenY);
    Device->SetClip(ScreenX, ScreenY, Width, Height);
    for (INT i = TopIndex; i < VisibleLines + TopIndex && i <= NumLines && i < Entries.Num(); i++)
    {
        NRecipeRow* R = Entries(i);
        INT RowY = (LineGap + RECIPE_ROW_HEIGHT) * Row + TextY;
        if (i == Selected && HighlightTex)
            Device->DrawStretchedTex(2, RowY, Width - 18, RECIPE_ROW_HEIGHT, HighlightTex);
        if (R)
        {
            if (R->Icon)
                Device->DrawTexture(TextX, RowY + 4, 32, 32, 0, 0, 32, 32, R->Icon);
            if (R->Name[0])
                Device->DrawText(TextX + 42, RowY + 2, 0xFFFFFFFF, R->Name, 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
        }
        // the price line
        NCGDevice::GetTextExtent(&Size, *SysString(STR_Price), 0, 0, 0, 0, 0);
        Device->DrawText(TextX + 42, RowY + 17, 0xFFFFFF00, *SysString(STR_Price), 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
        INT ColonW = Device->DrawText(TextX + 42 + Size.cx, RowY + 17, 0xFFFFFFFF, TEXT(" : "), 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
        NFormatThousands(Amount, R->Price);
        DWORD Color = NAmountColor(Amount);
        Device->DrawText(TextX + Size.cx + 42 + ColonW, RowY + 17, Color, Amount, 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
        INT LabelW = Size.cx;
        NCGDevice::GetTextExtent(&Size, Amount, 0, 0, 0, 0, 0);
        Device->DrawText(TextX + LabelW + Size.cx + 47 + ColonW, RowY + 17, 0xFFFFFF00, *SysString(STR_Adena), 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
        // the success rate line
        NCGDevice::GetTextExtent(&Size, *SysString(STR_Rate), 0, 0, 0, 0, 0);
        Device->DrawText(TextX + 42, RowY + 32, 0xFFA3A3A3, *SysString(STR_Rate), 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
        ColonW = Device->DrawText(TextX + 42 + Size.cx, RowY + 32, 0xFFFFFFFF, TEXT(" : "), 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
        appSprintf(Rate, TEXT("%d%%"), R->SuccessRate);
        Device->DrawText(TextX + ColonW + 42 + Size.cx, RowY + 32, 0xFFB09B79, Rate, 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
        Row++;
    }
    Device->ReleaseClip();
    Device->EndPaint();
    return 1;
}
NW_HOOK_METHOD(0x100c1a00, NCRecipeListBox, OnPaint)

NW_CHECK_VTABLE(NCRecipeListBox, 0x10123cb8, 75)
