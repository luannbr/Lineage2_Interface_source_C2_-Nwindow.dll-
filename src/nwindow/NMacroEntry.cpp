/*=============================================================================
    NMacroEntry.cpp - the macro object of the macro window (NMacroEntry.h) and its line.
    Stock: 0x1008B2D0 .. 0x1008B500 (the line, the getters, Set, Reset), 0x1008F840 / 0x1008F910 (the entry's
    constructor and destructor).
=============================================================================*/
#include "nwindow/NMacroEntry.h"

/*-----------------------------------------------------------------------------
    NMacroLine.
-----------------------------------------------------------------------------*/

// 0x1008b2d0 - the state words and the text cleared
NMacroLine::NMacroLine()
{
    Unknown24 = 0;
    Duration = 0;
    StartTime = 0;
    Clock = 0;
    appMemset(Text, 0, sizeof(Text));
}
NW_HOOK_CTOR(0x1008b2d0, NMacroLine)

// 0x1008b300 - the vector destructor's callback: the state words cleared
NMacroLine::~NMacroLine()
{
    Unknown24 = 0;
    Duration = 0;
    StartTime = 0;
    Clock = 0;
}
NW_HOOK_DTOR(0x1008b300, NMacroLine)

/*-----------------------------------------------------------------------------
    NMacroEntry.
-----------------------------------------------------------------------------*/

// 0x1008f840 - the tool tip, the twelve lines and the parser are constructed, the numbers and the three texts cleared
NMacroEntry::NMacroEntry()
{
    bRunning = 0;
    IconType = 0;
    NumLines = 0;
    CurLine = 0;
    ID = 0;
    bSelected = 0;
    IconTex = NULL;
    appMemset(Description, 0, sizeof(Description));
    appMemset(Name, 0, sizeof(Name));
    appMemset(ShortName, 0, sizeof(ShortName));
}
NW_HOOK_CTOR(0x1008f840, NMacroEntry)

// 0x1008f910 - no icon, nothing running, no lines; the parser, the lines and the tool tip are destroyed
NMacroEntry::~NMacroEntry()
{
    IconTex = NULL;
    Reset();
    NumLines = 0;
    bSelected = 0;
}
NW_HOOK_DTOR(0x1008f910, NMacroEntry)

// 0x1008b310 - NULL when empty (NToolTipInfo::FillMacro reads it inline)
const TCHAR* NMacroEntry::GetName()
{
    const TCHAR* Result = Name;
    if (Result == NULL || *Result == 0)
        Result = NULL;
    return Result;
}
NW_HOOK_METHOD(0x1008b310, NMacroEntry, GetName)

// 0x1008b330
const TCHAR* NMacroEntry::GetShortName()
{
    const TCHAR* Result = ShortName;
    if (Result == NULL || *Result == 0)
        Result = NULL;
    return Result;
}
NW_HOOK_METHOD(0x1008b330, NMacroEntry, GetShortName)

// 0x1008b350
const TCHAR* NMacroEntry::GetDescription()
{
    const TCHAR* Result = Description;
    if (Result == NULL || *Result == 0)
        Result = NULL;
    return Result;
}
NW_HOOK_METHOD(0x1008b350, NMacroEntry, GetDescription)

// 0x1008b370
void NMacroEntry::Deselect()
{
    bSelected = 0;
}
NW_HOOK_METHOD(0x1008b370, NMacroEntry, Deselect)

// 0x1008b380 (ret 0x3fc) - a macro as the server sends it: the id, the three texts, the icon and the number of
// lines, then each line into its place (a LineNo of 0 or 1 is the first line); what a line keeps depends on its type
void NMacroEntry::Set(INT InID, NMacroInfo Info)
{
    ID = InID;
    appStrcpy(Name, Info.Name);
    appStrcpy(Description, Info.Description);
    appSprintf(ShortName, TEXT("%s"), Info.ShortName);
    IconType = (signed char)Info.IconType;
    NumLines = (signed char)Info.NumLines;
    for (INT i = 0; i < NumLines; i++)
    {
        NMacroInfoLine& Src = Info.Lines[i];
        INT Index = (signed char)Src.LineNo;
        if (Index > 0)
            Index--;
        NMacroLine& Dest = Lines[Index];
        Dest.Type = (signed char)Src.Type;
        switch (Dest.Type)
        {
            case NMLT_Skill:
                Dest.Param08 = Src.A;
                Dest.Param10 = (signed char)Src.B;
                break;
            case NMLT_Action:
                Dest.Param08 = Src.A;
                break;
            case NMLT_Command:
                appSprintf(Dest.Text, TEXT("%s"), Src.Text);
                break;
            case NMLT_ShortCut:
                Dest.Param0C = Src.A;
                Dest.Param08 = (signed char)Src.B;
                break;
            case NMLT_Delay:
                Dest.Param0C = Src.A;
                break;
        }
    }
}
NW_HOOK_METHOD(0x1008b380, NMacroEntry, Set)

// 0x1008b4b0 - the lines' counters cleared, back to the first line, not running
void NMacroEntry::Reset()
{
    for (INT i = 0; i < NumLines; i++)
    {
        Lines[i].StartTime = 0;
        Lines[i].Clock = 0;
        Lines[i].Duration = 0;
    }
    CurLine = 0;
    bRunning = 0;
}
NW_HOOK_METHOD(0x1008b4b0, NMacroEntry, Reset)

// 0x1008b4f0
void NMacroEntry::Select()
{
    bSelected = 1;
}
NW_HOOK_METHOD(0x1008b4f0, NMacroEntry, Select)

// 0x1008b500
UTexture* NMacroEntry::GetIconTex()
{
    return IconTex;
}
NW_HOOK_METHOD(0x1008b500, NMacroEntry, GetIconTex)
