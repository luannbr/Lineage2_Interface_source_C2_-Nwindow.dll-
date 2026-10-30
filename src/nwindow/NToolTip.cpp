/*=============================================================================
    NToolTip.cpp - the C2 tool tips: NToolTipInfo (what a tool tip shows: the texts, the property lines,
    the numbers; built by SetText / Fill / FillMacro / FillMatchRow and measured by Layout) and
    NToolTipSkin (the nine textures every window gets in OnCreate and the two draw routines that paint a
    box and the text on it). Stock: 0x10002230 / 0x10002560 / 0x100022d0 / 0x1001aca0 (the object's
    members), 0x100019b0 .. 0x10028a70 (the line array's template code), 0x100273c0 .. 0x10029ef8 (the
    builders and the layout), 0x10029f00 .. 0x1002a4a1 (the skin).
=============================================================================*/
#include "nwindow/NCItemWnd.h"
#include "nwindow/NCListCtrl.h"
#include "nwindow/NCGDevice.h"
#include "nwindow/NConsoleWnd.h"

#define TIPSTR(ID) NCObject::SysString(ID)

// the sysstring ids the tips use (names ours)
enum
{
    TIPSTR_Weight = 52,
    TIPSTR_PAtk = 94,
    TIPSTR_PDef = 95,
    TIPSTR_Evasion = 97,
    TIPSTR_MAtk = 98,
    TIPSTR_MDef = 99,
    TIPSTR_AtkSpd = 111,
    TIPSTR_Level = 88,             // "Lv"
    TIPSTR_SkillSub = 91,          // " (<this>:N)" after a shortcut skill's level
    TIPSTR_ShieldRate = 317,       // "DEF Rate"
    TIPSTR_MPCost = 320,
    TIPSTR_Range = 321,
    TIPSTR_Price = 322,
    TIPSTR_MPBonus = 388,
    TIPSTR_SoulShots = 404,
    TIPSTR_Each = 468,             // "For Each"
    TIPSTR_Adena = 469,
    TIPSTR_SpiritShots = 496,
    TIPSTR_Count = 641,            // NTTO_Count's label
    TIPSTR_RecipeA = 736,          // a recipe's two numbers
    TIPSTR_RecipeB = 737,
    TIPSTR_Wanted = 808,           // the buy list's amount wanted
    TIPSTR_Classes1 = 670,         // the etc items' (henna) two lines
    TIPSTR_Classes2 = 671,
    TIPSTR_Rate = 744,
};

/*-----------------------------------------------------------------------------
    The line array's template code (TArray<NToolTipLine>): the stock module keeps these out of line.
-----------------------------------------------------------------------------*/

// 0x10028a70 - the line copied after the last one (the stock code is `Add(1, 0x414)` and a `rep movsd` of 0x105)
NW_HOOK_METHOD(0x10028a70, NToolTipLineArray, AddItem)

// 0x100020e0 - the copy
NW_HOOK_CTOR_ARGS(0x100020e0, NToolTipLineArray, (const NToolTipLineArray& A), (A))

// 0x1001a7d0 - the assignment
NW_HOOK_METHOD(0x1001a7d0, NToolTipLineArray, operator=)

// 0x10002190 - the destructor (Remove(0, Num) with UnTemplate.h's check()s, then ~FArray)
NW_HOOK_DTOR(0x10002190, NToolTipLineArray)

// 0x100019b0 - new(Array): room for a new last line
static void* NewToolTipLine(size_t Size, NToolTipLineArray& Array)
{
    return operator new(Size, Array);
}
NW_HOOK_FUNC(0x100019b0, NewToolTipLine)

/*-----------------------------------------------------------------------------
    NToolTipInfo: the object.
-----------------------------------------------------------------------------*/

// 0x10002230
NToolTipInfo::NToolTipInfo()
    : Enchant(0), Grade(0), Count(0), Extra24(0), bShowCount(0), Type(-1), Level(0), ItemID(0),
      Style(0), Options(0)
{
    FullBox.X = 0;
    FullBox.Y = 0;
    FullBox.W = 0;
    FullBox.H = 0;
    TitleBox.X = 0;
    TitleBox.Y = 0;
    TitleBox.W = 0;
    TitleBox.H = 0;
}
NW_HOOK_CTOR(0x10002230, NToolTipInfo)

// 0x10002560 - the copy a by-value argument is made of: the strings, the numbers, the lines' array, the boxes
NToolTipInfo::NToolTipInfo(const NToolTipInfo& Src)
    : Name(Src.Name), Name2(Src.Name2), Enchant(Src.Enchant), Grade(Src.Grade), Count(Src.Count),
      Extra24(Src.Extra24), bShowCount(Src.bShowCount), Type(Src.Type), Level(Src.Level),
      ItemID(Src.ItemID), Lines(Src.Lines), Description(Src.Description), Style(Src.Style),
      Options(Src.Options), FullBox(Src.FullBox), TitleBox(Src.TitleBox)
{
}
NW_HOOK_CTOR_ARGS(0x10002560, NToolTipInfo, (const NToolTipInfo& Src), (Src))

// 0x100022d0 - ~FString (Description), the lines' array, ~FString (Name2), ~FString (Name)
NToolTipInfo::~NToolTipInfo()
{
}
NW_HOOK_DTOR(0x100022d0, NToolTipInfo)

// 0x1001aca0 - the copy assignment (compiler-generated in the stock build, kept out of line): the strings,
// the numbers, the lines' array (0x1001a7d0), the boxes
NToolTipInfo& NToolTipInfo::operator=(const NToolTipInfo& Src)
{
    Name = Src.Name;
    Name2 = Src.Name2;
    Enchant = Src.Enchant;
    Grade = Src.Grade;
    Count = Src.Count;
    Extra24 = Src.Extra24;
    bShowCount = Src.bShowCount;
    Type = Src.Type;
    Level = Src.Level;
    ItemID = Src.ItemID;
    Lines = Src.Lines;
    Description = Src.Description;
    Style = Src.Style;
    Options = Src.Options;
    FullBox = Src.FullBox;
    TitleBox = Src.TitleBox;
    return *this;
}
NW_HOOK_METHOD(0x1001aca0, NToolTipInfo, operator=)

/*-----------------------------------------------------------------------------
    NToolTipInfo: numbers.
-----------------------------------------------------------------------------*/

// the buffer of FormatNumber (0x101A5B40 in the stock image; only that routine touches it)
typedef TCHAR NNumberBuffer[32];
#if NW_HOOK_MODE
#define GNumberBuffer NW_GLOBAL(NNumberBuffer, 0x101A5B40)
#else
extern TCHAR GNumberBuffer[32];
#endif

// 0x100273c0 - "%d" with a comma before each group of three digits, counted from the right: written backwards
// into the static buffer (a minus sign counts as a digit: -1234 gives "-,1234"... as the stock loop does)
const TCHAR* NToolTipInfo::FormatNumber(INT Value)
{
    TCHAR Text[1024];
    appSprintf(Text, TEXT("%d"), Value);
    INT Len = wcslen(Text);
    INT Pos = Len - 1;
    INT Digit = 0;
    INT End = Pos / 3 + Len;
    GNumberBuffer[End] = 0;
    if (Pos >= 0)
    {
        TCHAR* Dest = &GNumberBuffer[End - 1];
        do
        {
            if (Digit % 3 == 0 && Pos < (INT)wcslen(Text) - 1)
            {
                *Dest = ',';
                Dest--;
            }
            *Dest = Text[Pos];
            Pos--;
            Digit++;
            Dest--;
        } while (Pos >= 0);
    }
    return GNumberBuffer;
}
NW_HOOK_FUNC(0x100273c0, NToolTipInfo::FormatNumber)

// 0x10027470 - white up to four digits, then by the digits beyond: pink, yellow, green, cyan, pink...
DWORD NToolTipInfo::NumberColor(const TCHAR* Text)
{
    DWORD Color = 0xFFFFFFFF;
    INT Len = appStrlen(Text);
    INT Commas = 0;
    for (INT i = 0; i < Len; i++)
    {
        if (Text[i] == ',')
            Commas++;
    }
    INT Digits = Len - Commas;
    if (Digits > 4)
    {
        switch ((Digits - 2) % 4)
        {
        case 0:
            return 0xFFFF80FF;
        case 1:
            return 0xFFFFFF00;
        case 2:
            return 0xFF00FF00;
        case 3:
            Color = 0xFF00FFFF;
        }
        return Color;
    }
    return 0xFFFFFFFF;
}
NW_HOOK_FUNC(0x10027470, NToolTipInfo::NumberColor)

/*-----------------------------------------------------------------------------
    NToolTipInfo: the layout.
-----------------------------------------------------------------------------*/

// 0x10027500 - what the draw routines need: the size of the full box's text (FullBox) and of the title line's
// (TitleBox), by Style. The title line is the name (and "+N" and the additional name, the amount) and the
// grade's mark after it, the box holds it, the lines and the description wrapped at the box's width
void NToolTipInfo::Layout()
{
    SIZE Space;
    SIZE Ch;
    SIZE Size;
    SIZE LineSize;
    TCHAR Buf[1026];
    FString Tmp;

    NCGDevice::GetTextExtent(&Space, TEXT(" "), 0, 0, 0, 0, 0, L2FONT_Normal);
    NCGDevice::GetCharSize(&Ch, 0x41, 0);
    LineSize.cx = 0;
    LineSize.cy = 0;
    Size.cx = 0;
    Size.cy = 0;
    DWORD S = Style;
    if ((S & 0xC9) == 0 && (S & 0x500) == 0)
    {
        if ((S & NTT_Item) == 0)
        {
            if ((S & NTT_ShortCut) == 0)
            {
                if ((S & (NTT_Skill | NTT_SkillName)) == 0)
                {
                    if (S & NTT_Desc)
                    {
                        FullBox.W = 0xAE;
                        FullBox.H += (Ch.cy + 1) * Lines.Num();
                        if (Description.Num() > 1)
                        {
                            GConsole->ConvertNewLines((TCHAR*)*Description, Tmp);
                            NCGDevice::GetTextExtent(&Size, *Tmp, FullBox.W, 0, 0, 0, 0, L2FONT_Normal);
                            FullBox.H += Size.cy + 1;
                        }
                    }
                }
                else
                {
                    if (Level < 1)
                        Tmp = *Name;
                    else
                    {
                        appSprintf(Buf, TEXT("%s %s %d"), *Name, *TIPSTR(TIPSTR_Level), Level);
                        Tmp = Buf;
                    }
                    NCGDevice::GetTextExtent(&Size, *Tmp, 0, 0, 0, 0, 0, L2FONT_Normal);
                    if (Size.cx < 0x91)
                        FullBox.W = 0x90;
                    else
                        FullBox.W = Size.cx;
                    FullBox.H = (Ch.cy + 1) * Lines.Num() + Ch.cy;
                    if (Description.Num() > 1)
                    {
                        GConsole->ConvertNewLines((TCHAR*)*Description, Tmp);
                        NCGDevice::GetTextExtent(&Size, *Tmp, FullBox.W, 0, 0, 0, 0, L2FONT_Normal);
                        FullBox.H += Size.cy + 1;
                    }
                }
            }
            else
            {
                INT T = Type;
                if (T == 1)
                {
                    if (Enchant > 0)
                    {
                        appSprintf(Buf, TEXT("+%d "), Enchant);
                        Tmp = Buf;
                    }
                    if (Name2.Num() <= 1)
                        Tmp += Name;
                    else
                    {
                        appSprintf(Buf, TEXT("%s %s"), *Name, *Name2);
                        Tmp += Buf;
                    }
                    if (bShowCount != 0)
                    {
                        appSprintf(Buf, TEXT(" (%s)"), FormatNumber(Count));
                        Tmp += Buf;
                    }
                    NCGDevice::GetTextExtent(&Size, *Tmp, 0, 0, 0, 0, 0, L2FONT_Normal);
                    if (GConsole->GetGradeText(Grade))
                        Size.cx += 0xC + Space.cx;
                    FullBox.W = Size.cx;
                    FullBox.H = Ch.cy;
                }
                else
                {
                    if (T == 2)
                    {
                        if (Extra24 < 1)
                            appSprintf(Buf, TEXT("%s %s %d"), *Name, *TIPSTR(TIPSTR_Level), Level);
                        else
                            appSprintf(Buf, TEXT("%s %s %d (%s:%d)"), *Name, *TIPSTR(TIPSTR_Level), Level,
                                       *TIPSTR(TIPSTR_SkillSub), Extra24);
                        Tmp = Buf;
                    }
                    else
                    {
                        if (T != 3 && T != 4)
                            return;
                        Tmp = Name;
                    }
                    NCGDevice::GetTextExtent(&Size, *Tmp, 0, 0, 0, 0, 0, L2FONT_Normal);
                    FullBox.W = Size.cx;
                    FullBox.H = Ch.cy;
                }
            }
            return;
        }
        // an item: the title with "+N" and the amount, the lines and the description under it
        if ((Options & NTTO_NoEnchant) == 0 && Enchant > 0 && (Type == 0 || Type == 1 || Type == 3 || Type == 2))
        {
            appSprintf(Buf, TEXT("+%d "), Enchant);
            Tmp = Buf;
        }
        if (Name2.Num() <= 1)
            Tmp += Name;
        else
        {
            appSprintf(Buf, TEXT("%s %s"), *Name, *Name2);
            Tmp += Buf;
        }
        if ((Options & NTTO_NoCount) == 0 && bShowCount != 0)
        {
            appSprintf(Buf, TEXT(" (%s)"), FormatNumber(Count));
            Tmp += Buf;
        }
        NCGDevice::GetTextExtent(&Size, *Tmp, 0, 0, 0, 0, 0, L2FONT_Normal);
        if (GConsole->GetGradeText(Grade))
            Size.cx += 0xC + Space.cx;
        if (Size.cx < 0x91)
            FullBox.W = 0x90;
        else
            FullBox.W = Size.cx;
        FullBox.H = (Ch.cy + 1) * Lines.Num() + Ch.cy;
        if (Description.Num() > 1)
        {
            GConsole->ConvertNewLines((TCHAR*)*Description, Tmp);
            NCGDevice::GetTextExtent(&Size, *Tmp, FullBox.W, 0, 0, 0, 0, L2FONT_Normal);
            FullBox.H += Size.cy + 1;
        }
        if (Lines.Num() > 0)
        {
            NToolTipLine& First = Lines(0);
            if (First.bLabel == 0)
                appSprintf(Buf, TEXT("%s"), First.Value);
            else
                appSprintf(Buf, TEXT("%s : %s"), First.Label, First.Value);
            NCGDevice::GetTextExtent(&Size, Buf, 0, 0, 0, 0, 0, L2FONT_Normal);
        }
        if (FullBox.W < Size.cx)
            FullBox.W = Size.cx;
        Tmp.Empty();
        // the same title again, for the title line's box
        if ((Options & NTTO_NoEnchant) == 0 && Enchant > 0 && (Type == 0 || Type == 1 || Type == 3 || Type == 2))
        {
            appSprintf(Buf, TEXT("+%d "), Enchant);
            Tmp = Buf;
        }
        if (Name2.Num() <= 1)
            Tmp += Name;
        else
        {
            appSprintf(Buf, TEXT("%s %s"), *Name, *Name2);
            Tmp += Buf;
        }
        if ((Options & NTTO_NoCount) == 0 && bShowCount != 0)
        {
            appSprintf(Buf, TEXT(" (%s)"), FormatNumber(Count));
            Tmp += Buf;
        }
        NCGDevice::GetTextExtent(&Size, *Tmp, 0, 0, 0, 0, 0, L2FONT_Normal);
        if (GConsole->GetGradeText(Grade))
            Size.cx += 0xC + Space.cx;
        TitleBox.W = Size.cx;
        TitleBox.H = Ch.cy;
        if ((Options & (NTTO_Price | NTTO_PriceEach)) == 0)
            return;
        // the price line (the first one) is drawn under the name: the box grows by it
        if (Lines.Num() > 0)
        {
            appSprintf(Buf, TEXT("%s : %s"), Lines(0).Label, Lines(0).Value);
            NCGDevice::GetTextExtent(&LineSize, Buf, 0, 0, 0, 0, 0, L2FONT_Normal);
        }
        if (TitleBox.W < LineSize.cx)
            TitleBox.W = LineSize.cx;
        Ch.cy = TitleBox.H + Ch.cy + 1;
    }
    else
    {
        // a name, an additional name and a description only
        if (Name2.Num() <= 1)
            Tmp = Name;
        else
        {
            appSprintf(Buf, TEXT("%s %s"), *Name, *Name2);
            Tmp = Buf;
        }
        NCGDevice::GetTextExtent(&Size, *Tmp, 0, 0, 0, 0, 0, L2FONT_Normal);
        if (GConsole->GetGradeText(Grade))
            Size.cx += 0xC + Space.cx;
        if (Size.cx < 0x91)
            FullBox.W = 0x90;
        else
            FullBox.W = Size.cx;
        FullBox.H = (Ch.cy + 1) * Lines.Num() + Ch.cy;
        if (Description.Num() > 1)
        {
            GConsole->ConvertNewLines((TCHAR*)*Description, Tmp);
            NCGDevice::GetTextExtent(&Size, *Tmp, FullBox.W, 0, 0, 0, 0, L2FONT_Normal);
            FullBox.H += Size.cy + 1;
        }
        Tmp.Empty();
        if (Name2.Num() <= 1)
            Tmp += Name;
        else
        {
            appSprintf(Buf, TEXT("%s %s"), *Name, *Name2);
            Tmp += Buf;
        }
        if (bShowCount != 0)
        {
            appSprintf(Buf, TEXT(" (%s)"), FormatNumber(Count));
            Tmp += Buf;
        }
        NCGDevice::GetTextExtent(&Size, *Tmp, 0, 0, 0, 0, 0, L2FONT_Normal);
        if (GConsole->GetGradeText(Grade))
            Size.cx += 0xC;
        TitleBox.W = Size.cx;
    }
    TitleBox.H = Ch.cy;
}
NW_HOOK_METHOD(0x10027500, NToolTipInfo, Layout)

/*-----------------------------------------------------------------------------
    NToolTipInfo: the builders.
-----------------------------------------------------------------------------*/

// 0x10028ac0 - back to empty: the names, the lines, the description and the numbers (not Options)
void NToolTipInfo::Clear()
{
    Name.Empty();
    Name2.Empty();
    Style = 0;
    Grade = 0;
    Enchant = 0;
    Count = 0;
    Extra24 = 0;
    bShowCount = 0;
    Type = -1;
    Level = 0;
    ItemID = 0;
    Lines.Empty();
    Description.Empty();
    FullBox.H = 0;
    FullBox.W = 0;
    TitleBox.H = 0;
    TitleBox.W = 0;
}
NW_HOOK_METHOD(0x10028ac0, NToolTipInfo, Clear)

// 0x10028aa0 - the text and its Style, then the layout (NCButton::SetToolTip passes NTT_Plain)
void NToolTipInfo::SetText(const TCHAR* InText, INT InStyle)
{
    Name = InText;
    Style = InStyle;
    Layout();
}
NW_HOOK_METHOD(0x10028aa0, NToolTipInfo, SetText)

// 0x10029ba0 - a macro's tip: `this` is the macro object, which starts with its NToolTipInfo and keeps the
// name at +0x9EC and the description at +0xA06 (both NULL when empty: the stock getters 0x1008b310 / 0x1008b350)
void NToolTipInfo::FillMacro(NToolTipInfo* Macro, DWORD InStyle)
{
    if (Macro != NULL)
    {
        Clear();
        Style = InStyle;
        const TCHAR* MacroName = (const TCHAR*)((BYTE*)this + 0x9EC);
        if (*MacroName == 0)
            MacroName = NULL;
        Name = MacroName;
        const TCHAR* MacroDesc = (const TCHAR*)((BYTE*)this + 0xA06);
        if (*MacroDesc == 0)
            MacroDesc = NULL;
        Description = MacroDesc;
        Layout();
    }
}
NW_HOOK_METHOD(0x10029ba0, NToolTipInfo, FillMacro)

// 0x10029bf0 - a party matching row: the level (its first cell's... second cell's number), the class, the
// purpose of the party by the row's third number; the row's text is the description
void NToolTipInfo::FillMatchRow(NListRow* Row, DWORD InStyle)
{
    if (Row == NULL)
        return;
    Clear();
    Style = InStyle;
    NToolTipLine Line;
    appStrcpy(Line.Label, *TIPSTR(392));
    if (Row->Param1 == 0)
        appStrcpy(Line.Value, *TIPSTR(411));
    else
        appStrcpy(Line.Value, appItoa(Row->Cells[1].Param2));
    Lines.AddItem(Line);
    appStrcpy(Line.Label, *TIPSTR(391));
    if (Row->Param2 == 0)
        appStrcpy(Line.Value, *TIPSTR(411));
    else
        appStrcpy(Line.Value, GConsole->GetClassNameString(Row->Cells[0].Param2));
    Lines.AddItem(Line);
    INT Purpose = Row->Param3;
    if (Purpose == 0)
    {
        appStrcpy(Line.Label, *TIPSTR(489));
        appStrcpy(Line.Value, *TIPSTR(487));
        Lines.AddItem(Line);
    }
    else
    {
        FString* Text;
        if (Purpose == 1)
        {
            appStrcpy(Line.Label, *TIPSTR(489));
            Text = &TIPSTR(488);
        }
        else if (Purpose == 2)
        {
            appStrcpy(Line.Label, *TIPSTR(489));
            Text = &TIPSTR(798);
        }
        else if (Purpose == 3)
        {
            appStrcpy(Line.Label, *TIPSTR(489));
            Text = &TIPSTR(799);
        }
        else
        {
            if (Purpose != 4)
                goto Done;
            appStrcpy(Line.Label, *TIPSTR(489));
            Text = &TIPSTR(800);
        }
        appStrcpy(Line.Value, **Text);
        Lines.AddItem(Line);
    }
Done:
    Description = Row->Text;
    Layout();
}
NW_HOOK_METHOD(0x10029bf0, NToolTipInfo, FillMatchRow)

// 0x10028b20 - the tip of an item entry for a Style and Options: the names and numbers the Style needs, the
// price line(s) the Options ask for, then the property lines of the item (by its Type), the skill (type,
// MP cost, range) or the recipe, and the description; ends with the layout
void NToolTipInfo::Fill(NItemEntry* Item, DWORD InStyle, DWORD InOptions)
{
    if (Item == NULL)
        return;
    Clear();
    Style = InStyle;
    Options = InOptions;
    if ((InStyle & NTT_ItemName) == 0)
    {
        if ((InStyle & NTT_Item) == 0)
        {
            if ((InStyle & (NTT_Skill | NTT_SkillName)) == 0)
            {
                if ((InStyle & (NTT_Action | NTT_Name)) == 0)
                {
                    if ((InStyle & NTT_ShortCut) == 0)
                    {
                        if ((InStyle & NTT_Recipe) != 0)
                        {
                            Name = Item->GetName();
                            Name2 = Item->GetAdditionalName();
                            Enchant = Item->Enchant;
                            Grade = Item->CrystalGrade;
                            Count = Item->Count;
                            bShowCount = Item->bStackable;
                        }
                    }
                    else
                    {
                        Type = Item->Type;
                        if (Item->GetName() != NULL)
                            Name = Item->GetName();
                        if (Item->GetAdditionalName() != NULL)
                            Name2 = Item->GetAdditionalName();
                        Enchant = Item->Enchant;
                        Grade = Item->CrystalGrade;
                        Count = Item->Count;
                        Extra24 = nw::Field<INT>(Item, 0x1A74);   // the shortcut slot's entry is bigger than NItemEntry
                        bShowCount = Item->bStackable;
                        Level = Item->UnknownC18;
                    }
                }
                else
                    Name = Item->GetName();
            }
            else
            {
                Name = Item->GetName();
                Level = Item->UnknownC18;
            }
        }
        else
        {
            Name = Item->GetName();
            Name2 = Item->GetAdditionalName();
            Enchant = Item->Enchant;
            Grade = Item->CrystalGrade;
            if (Item->TradeCount < 1)
                Count = Item->Count;
            else
                Count = Item->TradeCount;
            bShowCount = Item->bStackable;
            Type = Item->Type;
            ItemID = Item->ID;
        }
    }
    else
    {
        Name = Item->GetName();
        Name2 = Item->GetAdditionalName();
        Grade = Item->CrystalGrade;
    }

    // the price lines the Options ask for: the first lines of the tip
    NToolTipLine PriceLine;
    if (Options & NTTO_Price)
    {
        appStrcpy(PriceLine.Label, *TIPSTR(TIPSTR_Price));
        appSprintf(PriceLine.Value, TEXT("%s %s"), FormatNumber(Item->UnknownC20), *TIPSTR(TIPSTR_Adena));
        PriceLine.ValueColor = NumberColor(FormatNumber(Item->UnknownC20));
        Lines.AddItem(PriceLine);
    }
    if (Options & NTTO_PriceEach)
    {
        appStrcpy(PriceLine.Label, *TIPSTR(TIPSTR_Price));
        appSprintf(PriceLine.Value, TEXT("%s %s %s"), *TIPSTR(TIPSTR_Each), FormatNumber(Item->UnknownC20),
                   *TIPSTR(TIPSTR_Adena));
        PriceLine.ValueColor = NumberColor(FormatNumber(Item->UnknownC20));
        Lines.AddItem(PriceLine);
    }
    if (Options & NTTO_Count)
    {
        appStrcpy(PriceLine.Label, *TIPSTR(TIPSTR_Count));
        appStrcpy(PriceLine.Value, FormatNumber(Item->UnknownC20));
        PriceLine.ValueColor = NumberColor(PriceLine.Value);
        Lines.AddItem(PriceLine);
    }

    // the property lines
    NToolTipLine Line;
    DWORD S = Style;
    if (S & NTT_ItemName)
    {
        appStrcpy(Line.Label, *TIPSTR(TIPSTR_Weight));
        appStrcpy(Line.Value, appItoa(Item->Weight));
        Lines.AddItem(Line);
    }
    else if (S & NTT_Item)
    {
        if ((S & NTT_Wanted) && Item->bStackable != 0 && Item->UnknownC34 > 0)
        {
            appStrcpy(Line.Label, *TIPSTR(TIPSTR_Wanted));
            appStrcpy(Line.Value, appItoa(Item->UnknownC34));
            Lines.AddItem(Line);
        }
        FString Slot;
        GConsole->GetItemSlotText(Item->Type, Item->BodyPart, Slot, Item->SubType);
        INT T = Item->Type;
        UBOOL bPending = 0;     // Line holds a line to add before the weight
        UBOOL bWeight = 1;      // the weight line follows
        if (T == 0)
        {
            // a weapon: type / slot, P.Atk, M.Atk, attack speed, soulshots, spiritshots
            if (GConsole->GetWeaponTypeName(Item->UnknownC30) != NULL)
            {
                appSprintf(Line.Value, TEXT("%s / %s"), GConsole->GetWeaponTypeName(Item->UnknownC30), *Slot);
                Line.bLabel = 0;
                Lines.AddItem(Line);
                Line.bLabel = 1;
            }
            appStrcpy(Line.Label, *TIPSTR(TIPSTR_PAtk));
            appStrcpy(Line.Value, appItoa(GConsole->GetEnchantedAttack(Item->CrystalGrade, Item->Enchant,
                                                                       Item->Unknown1C80)));
            Lines.AddItem(Line);
            appStrcpy(Line.Label, *TIPSTR(TIPSTR_MAtk));
            appStrcpy(Line.Value, appItoa(GConsole->GetEnchantedAttack(Item->CrystalGrade, Item->Enchant,
                                                                       Item->Unknown1C84)));
            Lines.AddItem(Line);
            appStrcpy(Line.Label, *TIPSTR(TIPSTR_AtkSpd));
            appStrcpy(Line.Value, GConsole->GetAttackSpeedText(Item->Unknown1C6C));
            Lines.AddItem(Line);
            appStrcpy(Line.Label, *TIPSTR(TIPSTR_SoulShots));
            appSprintf(Line.Value, TEXT("X %d"), Item->Unknown1CA0);
            Lines.AddItem(Line);
            appStrcpy(Line.Label, *TIPSTR(TIPSTR_SpiritShots));
            appSprintf(Line.Value, TEXT("X %d"), Item->Unknown1CA4);
            bPending = 1;
        }
        else if (T == 1 && (Item->BodyPart & 0x100) == 0 && (Item->BodyPart & 0x80) == 0 &&
                 GConsole->IsMagicArmor(Item->ID))
        {
            // a robe: slot, MP bonus, P.Def
            appStrcpy(Line.Value, *Slot);
            Line.bLabel = 0;
            Lines.AddItem(Line);
            Line.bLabel = 1;
            appStrcpy(Line.Label, *TIPSTR(TIPSTR_MPBonus));
            appStrcpy(Line.Value, appItoa(Item->Unknown1C9C));
            Lines.AddItem(Line);
            appStrcpy(Line.Label, *TIPSTR(TIPSTR_PDef));
            appStrcpy(Line.Value, appItoa(GConsole->GetEnchantedDefense(Item->CrystalGrade, Item->Enchant,
                                                                        Item->Unknown1C88)));
            bPending = 1;
        }
        else if (T == 1 && ((Item->BodyPart & 0x100) != 0 || (Item->BodyPart & 0x80) != 0))
        {
            // a shield: P.Def, the shield rate, the evasion (no slot line)
            appStrcpy(Line.Label, *TIPSTR(TIPSTR_PDef));
            appStrcpy(Line.Value, appItoa(GConsole->GetEnchantedDefense(Item->CrystalGrade, Item->Enchant,
                                                                        Item->Unknown1C90)));
            Lines.AddItem(Line);
            appStrcpy(Line.Label, *TIPSTR(TIPSTR_ShieldRate));
            appStrcpy(Line.Value, appItoa(Item->Unknown1C94));
            Lines.AddItem(Line);
            appStrcpy(Line.Label, *TIPSTR(TIPSTR_Evasion));
            appStrcpy(Line.Value, appItoa(Item->Unknown1C78));
            bPending = 1;
        }
        else if (T == 1)
        {
            // other armor: slot, P.Def
            appStrcpy(Line.Value, *Slot);
            Line.bLabel = 0;
            Lines.AddItem(Line);
            Line.bLabel = 1;
            appStrcpy(Line.Label, *TIPSTR(TIPSTR_PDef));
            appStrcpy(Line.Value, appItoa(GConsole->GetEnchantedDefense(Item->CrystalGrade, Item->Enchant,
                                                                        Item->Unknown1C88)));
            bPending = 1;
        }
        else if (T == 2)
        {
            // jewelry: slot, M.Def
            appStrcpy(Line.Value, *Slot);
            Line.bLabel = 0;
            Lines.AddItem(Line);
            Line.bLabel = 1;
            appStrcpy(Line.Label, *TIPSTR(TIPSTR_MDef));
            appStrcpy(Line.Value, appItoa(GConsole->GetEnchantedDefense(Item->CrystalGrade, Item->Enchant,
                                                                        Item->Unknown1C8C)));
            bPending = 1;
        }
        else if (T == 3)
        {
            // the slot only (no weight)
            appStrcpy(Line.Value, *Slot);
            Line.bLabel = 0;
            Lines.AddItem(Line);
            Line.bLabel = 1;
            bWeight = 0;
        }
        else if (T == 5)
        {
            INT Kind = Item->Unknown1910;
            if (Kind == 7)
            {
                // a pet collar: the pet's level
                appStrcpy(Line.Label, *TIPSTR(TIPSTR_Level));
                appStrcpy(Line.Value, appItoa(Item->Enchant));
                bPending = 1;
            }
            else if (Kind == 0xC)
            {
                // a henna-like item: a number and the list of the classes (bits of two words)
                appStrcpy(Line.Label, *TIPSTR(TIPSTR_Classes1));
                appStrcpy(Line.Value, appItoa((WORD)Item->UnknownC3C));
                Lines.AddItem(Line);
                Line.Value[0] = 0;
                appStrcpy(Line.Label, *TIPSTR(TIPSTR_Classes2));
                TArray<INT> List;
                for (INT i = 0; i < 16; i++)
                {
                    if (Item->Enchant & (1 << i))
                        List.AddItem(i + 1);
                }
                for (INT i = 0; i < 16; i++)
                {
                    if (Item->Unknown1C7C & (1 << i))
                        List.AddItem(i + 0x11);
                }
                for (INT i = 0; i < List.Num(); i++)
                {
                    appStrcat(Line.Value, appItoa(List(i)));
                    if (i < List.Num() - 1)
                        appStrcat(Line.Value, TEXT(","));
                }
                List.Empty();
                Lines.AddItem(Line);
            }
            else if (Kind == 0xD)
            {
                appStrcpy(Line.Label, *TIPSTR(TIPSTR_Classes1));
                appStrcpy(Line.Value, appItoa(Item->Enchant));
                Lines.AddItem(Line);
                Line.Value[0] = 0;
                appStrcpy(Line.Label, *TIPSTR(TIPSTR_Classes2));
                WORD Bits = (WORD)Item->UnknownC3C;
                if ((Bits & 0xF) != 0)
                    appStrcat(Line.Value, appItoa(Bits & 0xF));
                if ((Bits >> 4) != 0)
                {
                    appStrcat(Line.Value, TEXT(","));
                    appStrcat(Line.Value, appItoa(Bits >> 4));
                }
                Lines.AddItem(Line);
                appStrcpy(Line.Label, *TIPSTR(TIPSTR_Rate));
                appStrcpy(Line.Value, appItoa(Item->Unknown1C7C * 100));
                bPending = 1;
            }
        }
        else
            bWeight = 0;
        if (bPending)
            Lines.AddItem(Line);
        if (bWeight)
        {
            appStrcpy(Line.Label, *TIPSTR(TIPSTR_Weight));
            appStrcpy(Line.Value, appItoa(Item->Weight));
            Lines.AddItem(Line);
        }
    }
    else if (S & NTT_Skill)
    {
        // the skill's type (no label), MP cost, range
        if (GConsole->GetSkillTypeText(Item->ID, Item->UnknownC18) != NULL)
        {
            appStrcpy(Line.Value, GConsole->GetSkillTypeText(Item->ID, Item->UnknownC18));
            Line.bLabel = 0;
            Lines.AddItem(Line);
            Line.bLabel = 1;
        }
        if (GConsole->GetSkillMpCost(Item->ID, Item->UnknownC18) != 0)
        {
            appStrcpy(Line.Label, *TIPSTR(TIPSTR_MPCost));
            appStrcpy(Line.Value, appItoa(GConsole->GetSkillMpCost(Item->ID, Item->UnknownC18)));
            Lines.AddItem(Line);
        }
        if (GConsole->GetSkillRange(Item->ID, Item->UnknownC18) >= 0)
        {
            appStrcpy(Line.Label, *TIPSTR(TIPSTR_Range));
            appStrcpy(Line.Value, appItoa(GConsole->GetSkillRange(Item->ID, Item->UnknownC18)));
            Lines.AddItem(Line);
        }
    }
    else if (S & NTT_Recipe)
    {
        appStrcpy(Line.Label, *TIPSTR(TIPSTR_RecipeA));
        appStrcpy(Line.Value, appItoa(Item->UnknownC34));
        Lines.AddItem(Line);
        appStrcpy(Line.Label, *TIPSTR(TIPSTR_RecipeB));
        appStrcpy(Line.Value, appItoa(Item->Count));
        Lines.AddItem(Line);
    }
    Description = Item->GetDescription();
    Layout();
}
NW_HOOK_METHOD(0x10028b20, NToolTipInfo, Fill)

/*-----------------------------------------------------------------------------
    NToolTipSkin.
-----------------------------------------------------------------------------*/

// 0x10029f00 - the nine texture slots, then the info
NToolTipSkin::NToolTipSkin()
{
    for (INT i = 0; i < 9; i++)
        Tex[i] = NULL;
}
NW_HOOK_CTOR(0x10029f00, NToolTipSkin)

// 0x10029f20 - the arrays and strings are emptied by hand, then the members go (0x100022d0)
NToolTipSkin::~NToolTipSkin()
{
    Info.Lines.Empty();
    Info.Name.Empty();
    Info.Name2.Empty();
    Info.Description.Empty();
}
NW_HOOK_DTOR(0x10029f20, NToolTipSkin)

// DrawText as the tip routines call it: the font 0 and nothing else set (P5 is the width to wrap at, P9 1 for the
// grade's mark)
static FORCEINLINE DWORD DrawTipText(NCGDevice* Device, INT X, INT Y, DWORD Color, const TCHAR* Text,
                                     DWORD P5 = 0, INT P9 = 0)
{
    return Device->DrawText(X, Y, Color, Text, P5, 0, 0, L2FONT_Normal, P9, 0, 0, 0, 0, 0, 0);
}

// the clamp of the box to the screen and the nine-slice, inlined in both draw routines: *OutX / *OutY come in
// as the position of the thing the tip is about (relative to the device's origin, the window's corner), the box
// goes above-left of it moved to stay on the screen; the clip is left set (the callers release it)
__forceinline void NToolTipSkin::DrawFrame(NCGDevice* Device, INT* OutX, INT* OutY, const NToolTipRect& Wnd, INT W, INT H)
{
    INT ScreenW = GConsole->Desktop->LocalW;
    if (*OutX + Wnd.X + W > ScreenW)
        *OutX = ScreenW - Wnd.X - W;
    else if (*OutX + Wnd.X < 0)
        *OutX = -Wnd.X;
    INT Y = *OutY - H;
    *OutY = Y;
    if (Y + Wnd.Y < 0)
        *OutY = *OutY + H;
    Device->SetClip(*OutX + Wnd.X, *OutY + Wnd.Y, W, H);
    if (Tex[0])
        Device->DrawTexture(*OutX, *OutY, 8, 8, 0, 0, 8, 8, Tex[0]);
    if (Tex[1])
        Device->DrawStretchedTex(*OutX + 8, *OutY, W - 16, 8, Tex[1]);
    if (Tex[2])
        Device->DrawTexture(*OutX + W - 8, *OutY, 8, 8, 0, 0, 8, 8, Tex[2]);
    if (Tex[3])
        Device->DrawStretchedTex(*OutX, *OutY + 8, 8, H - 16, Tex[3]);
    if (Tex[4])
        Device->DrawStretchedTex(*OutX + 8, *OutY + 8, W - 16, H - 16, Tex[4]);
    if (Tex[5])
        Device->DrawStretchedTex(*OutX + W - 8, *OutY + 8, 8, H - 16, Tex[5]);
    if (Tex[6])
        Device->DrawTexture(*OutX, *OutY + H - 8, 8, 8, 0, 0, 8, 8, Tex[6]);
    if (Tex[7])
        Device->DrawStretchedTex(*OutX + 8, *OutY + H - 8, W - 16, 8, Tex[7]);
    if (Tex[8])
        Device->DrawTexture(*OutX + W - 8, *OutY + H - 8, 8, 8, 0, 0, 8, 8, Tex[8]);
}

// 0x1002a230 - the info is stored (0x1001aca0), the box is the title line's size + 10, the text goes 5 inside
void NToolTipSkin::Draw(NCGDevice* Device, NToolTipInfo InInfo, INT* OutX, INT* OutY, NToolTipRect Wnd)
{
    Info = InInfo;
    INT W = InInfo.TitleBox.W + 10;
    INT H = InInfo.TitleBox.H + 10;
    DrawFrame(Device, OutX, OutY, Wnd, W, H);
    DrawTitleText(Device, *OutX + 5, *OutY + 5);
    Device->ReleaseClip();
}
NW_HOOK_METHOD(0x1002a230, NToolTipSkin, Draw)

// 0x10029fa0 - the same with the full box (0 stays 0)
void NToolTipSkin::DrawFull(NCGDevice* Device, NToolTipInfo InInfo, INT* OutX, INT* OutY, NToolTipRect Wnd)
{
    Info = InInfo;
    INT W = 0;
    INT H = 0;
    if (InInfo.FullBox.W != 0)
        W = InInfo.FullBox.W + 10;
    if (InInfo.FullBox.H != 0)
        H = InInfo.FullBox.H + 10;
    DrawFrame(Device, OutX, OutY, Wnd, W, H);
    DrawFullText(Device, *OutX + 5, *OutY + 5);
    Device->ReleaseClip();
}
NW_HOOK_METHOD(0x10029fa0, NToolTipSkin, DrawFull)

// 0x10028760 - the title line: "+N " in orange, the name, the additional name in yellow, the grade's mark,
// " (count)"; then, when the Options carry a price, the first line under it
void NToolTipSkin::DrawTitleText(NCGDevice* Device, INT X, INT Y)
{
    SIZE Ch;
    TCHAR Buf[1026];
    NToolTipLine Line;
    NCGDevice::GetCharSize(&Ch, 0x41, 0);
    INT CX = X;
    if ((Info.Options & NTTO_NoEnchant) == 0 && Info.Enchant > 0 &&
        (Info.Type == 0 || Info.Type == 1 || Info.Type == 3 || Info.Type == 2))
    {
        appSprintf(Buf, TEXT("+%d "), Info.Enchant);
        CX = DrawTipText(Device, X, Y, 0xFFB09B79, Buf) + X;
    }
    CX += DrawTipText(Device, CX, Y, 0xFFFFFFFF, *Info.Name);
    if (Info.Name2.Num() > 1)
    {
        appSprintf(Buf, TEXT(" %s"), *Info.Name2);
        CX += DrawTipText(Device, CX, Y, 0xFFFFD969, Buf);
    }
    if (GConsole->GetGradeText(Info.Grade) != NULL)
    {
        appSprintf(Buf, TEXT(" `%s`"), GConsole->GetGradeText(Info.Grade));
        CX += DrawTipText(Device, CX, Y, 0xFFFFFFFF, Buf, 0, 1);
    }
    if ((Info.Options & NTTO_NoCount) == 0 && Info.bShowCount != 0)
    {
        appSprintf(Buf, TEXT(" (%s)"), NToolTipInfo::FormatNumber(Info.Count));
        DrawTipText(Device, CX, Y, 0xFFFFFFFF, Buf);
    }
    if ((Info.Options & (NTTO_Price | NTTO_PriceEach)) != 0 && Info.Lines.Num() > 0)
    {
        Line = Info.Lines(0);
        INT LineY = Ch.cy + Y + 1;
        INT LX = X;
        if (Line.bLabel != 0)
        {
            INT W = DrawTipText(Device, X, LineY, 0xFFA3A3A3, Line.Label);
            if (Line.bColon == 0)
                appStrcpy(Buf, TEXT(" "));
            else
                appStrcpy(Buf, TEXT(" : "));
            LX = W + X + DrawTipText(Device, W + X, LineY, 0xFFFFFFFF, Buf);
        }
        DrawTipText(Device, LX, LineY, Line.ValueColor, Line.Value);
    }
}
NW_HOOK_METHOD(0x10028760, NToolTipSkin, DrawTitleText)

// 0x10027eb0 - the full box's text: the title by Style (and Type, for a shortcut), the lines (label grey,
// " : " white, value in its color) and the description (wrapped at the box's width) under them
void NToolTipSkin::DrawFullText(NCGDevice* Device, INT X, INT Y)
{
    SIZE Ch;
    TCHAR Buf[1024];
    FString Tmp;
    INT CurY = Y;
    NCGDevice::GetCharSize(&Ch, 0x41, 0);
    DWORD S = Info.Style;
    if ((S & 0xC9) == 0 && (S & 0x100) == 0)
    {
        if ((S & NTT_Item) == 0)
        {
            if ((S & (NTT_Skill | NTT_SkillName)) == 0)
            {
                if ((S & NTT_ShortCut) != 0)
                {
                    INT T = Info.Type;
                    if (T == 1)
                    {
                        INT CX = X;
                        if (Info.Enchant > 0)
                        {
                            appSprintf(Buf, TEXT("+%d "), Info.Enchant);
                            CX = DrawTipText(Device, X, Y, 0xFFB09B79, Buf) + X;
                        }
                        CX += DrawTipText(Device, CX, Y, 0xFFFFFFFF, *Info.Name);
                        if (Info.Name2.Num() > 1)
                        {
                            appSprintf(Buf, TEXT(" %s"), *Info.Name2);
                            CX += DrawTipText(Device, CX, Y, 0xFFFFD969, Buf);
                        }
                        if (GConsole->GetGradeText(Info.Grade) != NULL)
                        {
                            appSprintf(Buf, TEXT(" `%s`"), GConsole->GetGradeText(Info.Grade));
                            CX += DrawTipText(Device, CX, Y, 0xFFFFFFFF, Buf, 0, 1);
                        }
                        if (Info.bShowCount != 0)
                        {
                            appSprintf(Buf, TEXT(" (%s)"), NToolTipInfo::FormatNumber(Info.Count));
                            DrawTipText(Device, CX, Y, 0xFFFFFFFF, Buf);
                        }
                    }
                    else if (T == 2)
                    {
                        INT W1 = DrawTipText(Device, X, Y, 0xFFFFFFFF, *Info.Name);
                        appSprintf(Buf, TEXT(" %s "), *TIPSTR(TIPSTR_Level));
                        INT W2 = DrawTipText(Device, W1 + X, Y, 0xFFA3A3A3, Buf);
                        INT CX = W1 + X + W2;
                        INT W3 = DrawTipText(Device, CX, Y, 0xFFB09B79, appItoa(Info.Level));
                        if (Info.Extra24 > 0)
                        {
                            appSprintf(Buf, TEXT(" (%s:%d)"), *TIPSTR(TIPSTR_SkillSub), Info.Extra24);
                            DrawTipText(Device, CX + W3, Y, 0xFFFFFFFF, Buf);
                        }
                    }
                    else if (T == 3 || T == 4)
                        DrawTipText(Device, X, Y, 0xFFFFFFFF, *Info.Name);
                }
            }
            else
            {
                INT W1 = DrawTipText(Device, X, Y, 0xFFFFFFFF, *Info.Name);
                if (Info.Level > 0)
                {
                    appSprintf(Buf, TEXT(" %s "), *TIPSTR(TIPSTR_Level));
                    INT W2 = DrawTipText(Device, W1 + X, Y, 0xFFA3A3A3, Buf);
                    DrawTipText(Device, W1 + X + W2, Y, 0xFFB09B79, appItoa(Info.Level));
                }
                CurY = Ch.cy + Y;
            }
        }
        else
        {
            INT CX = X;
            if (Info.Enchant > 0 && (Info.Type == 0 || Info.Type == 1 || Info.Type == 3 || Info.Type == 2))
            {
                appSprintf(Buf, TEXT("+%d "), Info.Enchant);
                CX = DrawTipText(Device, X, Y, 0xFFB09B79, Buf) + X;
            }
            CX += DrawTipText(Device, CX, Y, 0xFFFFFFFF, *Info.Name);
            if (Info.Name2.Num() > 1)
            {
                appSprintf(Buf, TEXT(" %s"), *Info.Name2);
                CX += DrawTipText(Device, CX, Y, 0xFFFFD969, Buf);
            }
            if (GConsole->GetGradeText(Info.Grade) != NULL)
            {
                appSprintf(Buf, TEXT(" `%s`"), GConsole->GetGradeText(Info.Grade));
                CX += DrawTipText(Device, CX, Y, 0xFFFFFFFF, Buf, 0, 1);
            }
            if ((Info.Options & NTTO_NoCount) == 0 && Info.bShowCount != 0)
            {
                appSprintf(Buf, TEXT(" (%s)"), NToolTipInfo::FormatNumber(Info.Count));
                DrawTipText(Device, CX, Y, 0xFFFFFFFF, Buf);
            }
            CurY = Ch.cy + Y;
        }
    }
    else
    {
        INT CX = DrawTipText(Device, X, Y, 0xFFFFFFFF, *Info.Name) + X;
        if (Info.Name2.Num() > 1)
        {
            appSprintf(Buf, TEXT(" %s"), *Info.Name2);
            CX += DrawTipText(Device, CX, Y, 0xFFFFD969, Buf);
        }
        if (GConsole->GetGradeText(Info.Grade) != NULL)
        {
            appSprintf(Buf, TEXT(" `%s`"), GConsole->GetGradeText(Info.Grade));
            DrawTipText(Device, CX, Y, 0xFFFFFFFF, Buf, 0, 1);
        }
        CurY = Ch.cy + Y;
    }
    for (INT i = 0; i < Info.Lines.Num(); i++)
    {
        CurY++;
        NToolTipLine Line = Info.Lines(i);
        INT CX = X;
        if (Line.bLabel != 0)
        {
            INT W = DrawTipText(Device, X, CurY, 0xFFA3A3A3, Line.Label);
            if (Line.bColon == 0)
                appStrcpy(Buf, TEXT(" "));
            else
                appStrcpy(Buf, TEXT(" : "));
            CX = W + X + DrawTipText(Device, W + X, CurY, 0xFFFFFFFF, Buf);
        }
        DrawTipText(Device, CX, CurY, Line.ValueColor, Line.Value);
        CurY += Ch.cy;
    }
    GConsole->ConvertNewLines((TCHAR*)*Info.Description, Tmp);
    DrawTipText(Device, X, CurY + 1, 0xFFB2BECF, *Tmp, Info.FullBox.W);
}
NW_HOOK_METHOD(0x10027eb0, NToolTipSkin, DrawFullText)
