/*=============================================================================
    NDialogInfo.cpp - dialog box parameters. Stock: 0x100064b0 .. 0x100065e4 (the setters),
    0x10006e60 .. 0x10006f0f (the constructor, 0x100067d0 the destructor).
=============================================================================*/
#include "nwindow/NDialogInfo.h"

// 0x10006e60 - the six strings, then the ints (-1, 3, 1 and zeros). 0x10006e20 is another record's
// constructor (a smaller one with no strings)
NDialogInfo::NDialogInfo()
{
    Owner = NULL;
    bInput = 0;
    bLargeAmount = 0;
    Param1 = 0;
    Param2 = 0;
    Unknown1624 = 0;
    Unknown1628 = 0;
    bProgress = 0;
    Unknown74 = 0;
    ID = -1;
    Type = 3;
    bMainBox = 1;
}
NW_HOOK_CTOR(0x10006e60, NDialogInfo)

// 0x100067d0 - the six strings, last first (the stock's out-of-line copy: the callers that keep an NDialogInfo
// on the stack call it)
NW_HOOK_DTOR(0x100067d0, NDialogInfo)

// 0x10006550 - NULL empties it (C1 emptied for "" too)
void NDialogInfo::SetText(const TCHAR* InText)
{
    if (InText)
        Text = InText;
    else
        Text.Empty();
}
NW_HOOK_METHOD(0x10006550, NDialogInfo, SetText)

// 0x10006500
void NDialogInfo::SetSystemMessage(INT MsgID)
{
    if (*GL2GameData.SystemMsg[MsgID].Message)
        Text = *GL2GameData.SystemMsg[MsgID].Message;
    else
        Text.Empty();
}
NW_HOOK_METHOD(0x10006500, NDialogInfo, SetSystemMessage)

// 0x100064b0
void NDialogInfo::SetButtons(const TCHAR* InButton1, const TCHAR* InButton2)
{
    if (InButton1 && *InButton1)
        Button1 = InButton1;
    else
        Button1.Empty();
    if (InButton2 && *InButton2)
        Button2 = InButton2;
    else
        Button2.Empty();
}
NW_HOOK_METHOD(0x100064b0, NDialogInfo, SetButtons)

// 0x10006570
void NDialogInfo::SetParamText(const TCHAR* InText)
{
    if (InText && *InText)
        ParamText = InText;
    else
        ParamText.Empty();
}
NW_HOOK_METHOD(0x10006570, NDialogInfo, SetParamText)

// 0x100065a0
void NDialogInfo::SetParams(INT InParam1, INT InParam2)
{
    Param1 = InParam1;
    Param2 = InParam2;
}
NW_HOOK_METHOD(0x100065a0, NDialogInfo, SetParams)

// 0x100065c0 (name ours) - the sixth string, new in C2
void NDialogInfo::SetText60(const TCHAR* InText)
{
    if (InText && *InText)
        Text60 = InText;
    else
        Text60.Empty();
}
NW_HOOK_METHOD(0x100065c0, NDialogInfo, SetText60)
