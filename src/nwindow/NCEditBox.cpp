/*=============================================================================
    NCEditBox.cpp - single line text input. Stock: 0x100080d0 .. 0x10008bd6,
    0x10008be0 .. 0x10009a5a, 0x10009a60 .. 0x1000bb16; WindowProc shared with NCIMEManager,
    ShowWindow folded.
=============================================================================*/
#include "nwindow/NCEditBox.h"
#include <stdio.h>
#include "nwindow/NCIMEManager.h"
#include "nwindow/NCChineseIME.h"
#include "nwindow/NCCompositeBox.h"
#include "nwindow/NConsoleWnd.h"
#include "nwindow/NCGDevice.h"
#include "nwindow/NL2Font.h"
#include "re/NwDebug.h"

enum { NTIMER_EDIT_CARET = 0xFA0D05 };

// Chinese IME fields the edit box reads
static NCCompositeBox* ChineseCompositeBox(NCIMEManager* Manager)
{
    return ((NCChineseIME*)Manager->ChineseIME)->CompositeBox;
}
static const FString& ChineseLastComp(NCIMEManager* Manager)
{
    return ((NCChineseIME*)Manager->ChineseIME)->LastComp;
}

// the three pieces are measured one after the other, as the stock code does everywhere
#define TEXT_W(Str) GetTextWidth(Str)

// C2: with Unknown1C8 set (the English client) the IME messages are only taken for the codes 0x20..0x81
#define EDIT_CODE_OK(w) (Unknown1C8 != 1 || ((w) > 0x1F && (w) < 0x82))

// 0x1000d5d0 - NControl's constructor is inlined in the stock code; C2: a fifth argument and the
// Thai cluster arrays, which the members' constructors take care of
NCEditBox::NCEditBox(INT InType, const TCHAR* BackTexName, UBOOL bInIMEPosFixed, INT InExtraWidth,
                     INT InUnknown1C8)
    : CaretTimer(0), BackTex(NULL), WhiteTex(NULL)
{
    Type = InType;
    bIMEPosFixed = bInIMEPosFixed;
    if (BackTexName && BackTexName[0])
        BackTex = FindTexture(BackTexName);
    Unknown1C8 = InUnknown1C8;
    CaretX = 0;
    Unknown224 = 0;
    IMECaretX = 0;
    IMECaretY = 0;
    CursorPos = 0;
    ReadingParam = -1;
    bCaretOn = 1;
    bCanInsert = 1;
    Unknown1D8 = 0;
    bReading = 0;
    Unknown1F0 = 0;
    ViewStart = 0;
    ExtraWidth = InExtraWidth;
    ViewStart2 = 0;
    MaxLen = 0;
    Unknown1CC = 1;
    if (GLanguageType == 1)
        Unknown1C8 = 1;
    IMEManager = NULL;
}
NW_HOOK_CTOR_ARGS(0x1000d5d0, NCEditBox, (INT A, const TCHAR* B, UBOOL C, INT D, INT E), (A, B, C, D, E))

// 0x1000d7a0
NW_HOOK_DTOR(0x1000d7a0, NCEditBox)

// 0x10010fa0 (vslot 25) - one body with NCIMEManager's (not hooked there); C2: no enabled test
INT NCEditBox::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (NControl::WindowProc(Msg, wParam, lParam))
        return 1;
    if (Msg == NIM_COMPOSITION)
        return OnComposition(wParam, lParam) != 0;
    if (Msg == NIM_RESULT)
        return OnResult(wParam, lParam) != 0;
    if (Msg == NIM_ATTRIBUTE)
        return OnAttribute(wParam, lParam) != 0;
    if (Msg == NIM_READING)
        return OnReading(wParam, lParam) != 0;
    if (Msg == NIM_14)
        return On14(wParam, lParam) != 0;
    if (Msg == NIM_GETCARET)
        return OnGetCaret(wParam, lParam) != 0;
    if (Msg == NIM_ENDCOMPOSITION)
        return OnEndComposition(wParam, lParam) != 0;
    if (Msg == NIM_CURSORPOS)
        return OnCursorPos(wParam, lParam) != 0;
    return 0;
}
NW_HOOK_METHOD(0x10010fa0, NCEditBox, WindowProc)

// 0x1000a620 (vslot 29)
void NCEditBox::SetFocus()
{
    NCWnd::SetFocus();
    IMEManager->Activate();
}
NW_HOOK_METHOD(0x1000a620, NCEditBox, SetFocus)

// 0x1000d440 (vslot 30) - the pieces are joined (C2: the composition only when it still fits MaxLen; the
// Thai cluster arrays follow in the Thai client)
void NCEditBox::KillFocus()
{
    if (GLanguageType == 5)
    {
        Left += Right;
        Right.Empty();
        NCGDevice::MakeThaiCharArray(*Left, ThaiLeft, 0);
        ThaiRight.Empty();
    }
    else
    {
        if (MaxLen == 0 || (MaxLen > 0 && Left.Len() + Comp.Len() + Right.Len() <= MaxLen))
            Left += Comp;
        Left += Right;
        Comp.Empty();
        Right.Empty();
    }
    IMEManager->Deactivate();
    NCWnd::KillFocus();
}
NW_HOOK_METHOD(0x1000d440, NCEditBox, KillFocus)

// 0x1000a5e0 (vslot 35) - caret blink
INT NCEditBox::OnTimer(INT TimerID)
{
    if (TimerID == NTIMER_EDIT_CARET)
        bCaretOn = !bCaretOn;
    return 1;
}
NW_HOOK_METHOD(0x1000a5e0, NCEditBox, OnTimer)

// 0x1000b260 (vslot 36) - NControl::OnCreate is not called. IMECaretY: where the IME puts its
// windows (Japanese clients above the box), 137 px or 17 px (bIMEPosFixed not set) away, 152
// for some Chinese layouts. C2: the IME manager is told the box's Type (2, a password, keeps the IME closed)
INT NCEditBox::OnCreate()
{
    Edit1 = FindTexture(TEXT("L2UI.Control.Edit1"));
    Edit2 = FindTexture(TEXT("L2UI.Control.Edit2"));
    Edit3 = FindTexture(TEXT("L2UI.Control.Edit3"));
    EditDisable1 = FindTexture(TEXT("L2UI.Control.Edit_disable1"));
    EditDisable2 = FindTexture(TEXT("L2UI.Control.Edit_disable2"));
    EditDisable3 = FindTexture(TEXT("L2UI.Control.Edit_disable3"));
    HighlightTex = FindTexture(TEXT("L2UI.MacroWnd.Macro_Line_HighLight"));
    CaretTimer = SetTimer(NTIMER_EDIT_CARET, 500, NULL);
    Left.Empty();
    Right.Empty();
    Comp.Empty();
    if (!WhiteTex)
        WhiteTex = FindTexture(TEXT("NWindow.WhiteTexture"));
    WhiteTex = FindTexture(TEXT("NWindow.WhiteTexture"));
    if (bIMEPosFixed)
    {
        if (GConsole->GetIMEType() == 5 || GConsole->GetIMEType() == 7 ||
            GConsole->GetIMEType() == 9 || GConsole->GetIMEType() == 12)
            IMECaretY = 152;
        else
            IMECaretY = GLanguageType == 2 ? -137 : 137;
    }
    else
        IMECaretY = GLanguageType == 3 ? -17 : 17;
    IMEManager = new NCIMEManager(Type);
    CreateChild(IMEManager, NWS_VISIBLE);
    return 1;
}
NW_HOOK_METHOD(0x1000b260, NCEditBox, OnCreate)

// 0x1000b470 (vslot 37) - guarded in C2
INT NCEditBox::OnDestroy()
{
    guard(NCEditBox::OnDestroy);
    if (CaretTimer)
    {
        KillTimer(CaretTimer);
        CaretTimer = 0;
    }
    if (IMEManager)
    {
        RemoveChildWindow(IMEManager->Entry);
        delete IMEManager;
        IMEManager = NULL;
    }
    NControl::OnDestroy();
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x1000b470, NCEditBox, OnDestroy)

// 0x1007b460 (vslot 43) - one body for every class that only calls NCWnd's
void NCEditBox::ShowWindow()
{
    NCWnd::ShowWindow();
}
NW_HOOK_METHOD(0x1007b460, NCEditBox, ShowWindow)

// 0x1002bb70 (name ours; stdcall, reads no this: NAmountColor, NCRecipeListBox.cpp) - the colour a number is drawn in by
// its length without separators (white up to 4 digits, then four colours)
DWORD __stdcall NAmountColor(const TCHAR* Text);   // NCRecipeListBox.h
static DWORD NumberColor(NCEditBox* Wnd, const TCHAR* Text)
{
    return NAmountColor(Text);
}

// 0x1000cad0 (vslot 55) - frame (C2: and the highlight texture of the macro lines when Unknown1D8 is set), the
// text from ViewStart (a Type 3 number in the colour NumberColor gives), the IME composition at the caret, the
// rest, then the blinking caret (Unknown1CC off: none) or the composing character. C2: MaxLen replaces the width
// test of bCanInsert, and the Thai client moves the view on by a cluster. The widths measured before the text
// is drawn are thrown away, as in the stock code.
INT NCEditBox::OnPaint(NCGDevice* Device)
{
    TCHAR Stars[258];
    SIZE Size;
    INT TWClauseW = 0;
    DWORD Color = 0xFFFFFFFF;
    Device->BeginPaint(ScreenX, ScreenY);
    Device->SetClip(ScreenX, ScreenY, Width, Height);

    if (Type == 3)
    {
        FString Both = Left + Right;
        Color = NumberColor(this, *Both);
        Both.Empty();
    }

    if (IsFocused())
    {
        if (Edit1)
            Device->DrawTexture(0, 0, 8, 16, 0, 0, 8, 16, Edit1);
        if (Edit2)
            Device->DrawTexture(8, 0, Width - 16, 16, 0, 0, 8, 16, Edit2);
        if (Edit3)
            Device->DrawTexture(Width - 8, 0, 8, 16, 0, 0, 8, 16, Edit3);
    }
    else
    {
        if (EditDisable1)
            Device->DrawTexture(0, 0, 8, 16, 0, 0, 8, Height, EditDisable1);
        if (EditDisable2)
            Device->DrawTexture(8, 0, Width - 16, 16, 0, 0, 8, 16, EditDisable2);
        if (EditDisable3)
            Device->DrawTexture(Width - 8, 0, 8, 16, 0, 0, 8, 16, EditDisable3);
    }
    if (Unknown1D8 != 0 && HighlightTex != NULL)
        Device->DrawStretchedTex(0, 0, Width, Height, 0, 0, 0xC1, 16, HighlightTex);

    if (ExtraWidth > 0)
        MaxWidth = Width + ExtraWidth;
    else
        MaxWidth = Width;

    NCGDevice::GetTextExtent(&Size, *Left, 0, 0, 0, 0, 0, L2FONT_Normal);
    INT TextW = Size.cx;
    NCGDevice::GetTextExtent(&Size, *Comp, 0, 0, 0, 0, 0, L2FONT_Normal);
    TextW += Size.cx;
    NCGDevice::GetTextExtent(&Size, *Right, 0, 0, 0, 0, 0, L2FONT_Normal);
    TextW += Size.cx;
    if (MaxLen == 0)
        bCanInsert = TextW < MaxWidth - 12;

    // proof mode, not stock (re/NwDebug.h): '#' for the password and a red caret
    const TCHAR Star = NW_DEBUG_MARKER() ? '#' : '*';
    const DWORD CaretColor = NW_DEBUG_MARKER() ? 0xFFFF0000 : 0xFFFFFFFF;

    if (Type == 2)
    {
        INT i;
        for (i = 0; i < Left.Len(); i++)
            Stars[i] = Star;
        Stars[i] = 0;
        TextW = Device->DrawText(2, 2, 0xFFFFFFFF, Stars, 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
    }
    else
    {
        TextW = 0;
        UBOOL bFull = 0;
        if (ViewStart < Left.Len())
        {
            for (INT i = ViewStart; i < Left.Len(); i++)
            {
                NCGDevice::GetTextExtent(&Size, *Left.Mid(i, 1), 0, 0, 0, 0, 0, L2FONT_Normal);
                TextW += Size.cx;
                if (TextW >= Width)
                {
                    bFull = 1;
                    break;
                }
            }
        }
        if (!bFull)
        {
            NCGDevice::GetTextExtent(&Size, *Comp, 0, 0, 0, 0, 0, L2FONT_Normal);
            TextW += Size.cx;
            if (TextW < Width)
            {
                for (INT i = 0; i < Right.Len(); i++)
                {
                    NCGDevice::GetTextExtent(&Size, *Right.Mid(i, 1), 0, 0, 0, 0, 0, L2FONT_Normal);
                    TextW += Size.cx;
                    if (TextW >= Width)
                        break;
                }
            }
        }
        if (ViewStart < Left.Len())
        {
            TextW = Device->DrawText(2, 2, Color, *Left + ViewStart, 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
            if (Left.Len() > 0)
                TextW = GetTextWidth(FString(*Left + ViewStart));
        }
        else
            TextW = 0;
    }

    NCGDevice::GetTextExtent(&Size, *Comp, 0, 0, 0, 0, 0, L2FONT_Normal);
    INT CompW = Size.cx;
    NCGDevice::GetTextExtent(&Size, *Reading, 0, 0, 0, 0, 0, L2FONT_Normal);
    DWORD ReadingW = Size.cx;
    if (ReadingW > 0 && GConsole->GetIMEType() == 6)
        ReadingW = 12;

    if (Type != 2)
    {
        if (GLanguageType == 2)
            PaintCompositionJP(2, 2, TextW, 0, Device);
        else if (GLanguageType == 0)
            PaintCompositionKR(2, 2, TextW, 0, Device);
        else if (GLanguageType == 3)
            PaintCompositionTW(2, 2, TextW, &TWClauseW, Device);
        else if (GLanguageType == 4)
            PaintCompositionCN(2, 2, TextW, Device);
    }
    if (Type == 2)
    {
        INT i;
        for (i = 0; i < Right.Len(); i++)
            Stars[i] = Star;
        Stars[i] = 0;
        NCGDevice::GetTextExtent(&Size, *Comp, 0, 0, 0, 0, 0, L2FONT_Normal);
        Device->DrawText(TextW + ReadingW + Size.cx + 2, 2, 0xFFFFFFFF, Stars, 0, 0, 0, L2FONT_Normal, 0, 0,
                         0, 0, 0, 0);
    }
    else
    {
        // Chinese layouts draw the composition in their own window
        if (GConsole->GetIMEType() == 10 || GConsole->GetIMEType() == 9)
            Size.cx = 0;
        else
            NCGDevice::GetTextExtent(&Size, *Comp, 0, 0, 0, 0, 0, L2FONT_Normal);
        Device->DrawText(TextW + ReadingW + Size.cx + 2, 2, Color, *Right, 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0,
                         0, 0);
    }

    if (IMEManager->IsNotComposing() == 0)
    {
        if (IsFocused() && GLanguageType == 0)
            PaintComposingKR(2, 2, TextW, CompW, Device);
        else if (IsFocused() && GLanguageType == 2)
            PaintComposingJP(2, 2, TextW, CompW, Device);
        else if (IsFocused() && GLanguageType == 3)
            PaintComposingTW(2, 2, TextW, CompW, TWClauseW, ReadingW, Device);
        else if (IsFocused() && GLanguageType == 4)
            PaintComposingCN(2, 2, TextW, CompW, Device);
        // C2: an empty composition still shows the caret
        if (IsFocused() && CompW == 0 && Unknown1CC != 0 && bCaretOn)
            Device->DrawText(TextW - 1, 2, 0xFFFFFFFF, TEXT("|"), 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
        if ((DWORD)(CompW + TextW) > (DWORD)Width)
            ViewStart++;
    }
    else
    {
        // the caret stays inside the box: past the right edge the view scrolls one character (cluster)
        CaretX = TextW + CompW;
        if (CaretX == (DWORD)Width)
            CaretX--;
        else if (CaretX > (DWORD)Width)
        {
            if (GLanguageType == 5)
            {
                INT Cluster = IMEManager->GetTextLength(Left.Mid(0, ViewStart));
                ViewStart += ThaiLeft(Cluster).CharCount();
            }
            else
                ViewStart++;
            ViewStart2 = ViewStart;
            CaretX = Width - 1;
        }
        if (Unknown1CC != 0 && IsFocused() && bCaretOn)
            Device->DrawText(Type == 2 ? CaretX : CaretX - 1, 2, CaretColor, TEXT("|"), 0, 0, 0,
                             L2FONT_Normal, 0, 0, 0, 0, 0, 0);
    }
    Device->ReleaseClip();
    Device->EndPaint();
    return 1;
}
NW_HOOK_METHOD(0x1000cad0, NCEditBox, OnPaint)

// 0x1000c740 (vslot 57) - printable codes (the Thai keyboard also takes the Thai characters its spelling
// rules allow); digits only for Type 3. A '"' typed first in the chat line is reported (3). C2: MaxLen
// limits the text, the Thai cluster array and the thousands separators follow, and the view is scrolled by
// clusters in the Thai client. Backspace (8) is let through while the box is full.
INT NCEditBox::OnChar(WPARAM wParam, LPARAM lParam)
{
    TCHAR Temp[20];
    if (wParam != 8 && !bCanInsert)
        return 1;
    // sic: the stock test is "wParam > 0xFFFFFFFF", which never holds
    if (Unknown1C8 == 1 && wParam < 0x20)
        return 1;
    if (Type == 3 && (wParam < '0' || wParam > '9'))
        return 1;
    if (wParam < 0x20)
    {
        // the stock code first asks for IME type 9 here, with a range that nothing is in
        GConsole->GetIMEType();
        if (GConsole->GetIMEType() != 13 || !IsThaiInputValid(wParam))
            goto Tail;
    }
    // the stock code formats the code with swprintf("%c"): the VC 7.0 CRT takes a wide character there, our
    // UCRT's wide "%c" keeps only the low byte (a Thai or Hangul code would be cut), so it is stored as it is
    Temp[0] = (TCHAR)wParam;
    Temp[1] = 0;
    if (MaxLen != 0)
    {
        if (Left.Len() + Right.Len() + appStrlen(Temp) > MaxLen)
            return 1;
    }
    if (Left.Len() > 0)
        Left += Temp;
    else
    {
        Left = Temp;
        if (wParam == '"' && Type == 1)
            NotifyParent(3);
    }
    if (GLanguageType == 5)
        NCGDevice::MakeThaiCharArray(*Left, ThaiLeft, 0);
    if (Type == 3)
        ReformatNumber();
    if (GetTextWidth(FString(*Left + ViewStart)) > Width)
    {
        if (GLanguageType == 5)
        {
            INT Cluster = IMEManager->GetTextLength(Left.Mid(0, ViewStart));
            INT Next = ThaiLeft(Cluster + 1).GetWidth(GCanvas, 0);
            INT Sum = Next + ThaiLeft(Cluster).GetWidth(GCanvas, 0);
            INT Cur = ThaiLeft(Cluster).GetWidth(GCanvas, 0);
            if (Sum <= Cur)
                ViewStart = ViewStart + ThaiLeft(Cluster + 1).CharCount() + ThaiLeft(Cluster).CharCount();
            else
                ViewStart += ThaiLeft(Cluster).CharCount();
        }
        else
        {
            SIZE Shown, Typed;
            NCGDevice::GetTextExtent(&Shown, *Left.Mid(ViewStart, 2), 0, 0, 0, 0, 0, L2FONT_Normal);
            NCGDevice::GetTextExtent(&Typed, Temp, 0, 0, 0, 0, 0, L2FONT_Normal);
            if (Typed.cx < Shown.cx)
                ViewStart += 1;
            else
                ViewStart += 2;
        }
        ViewStart2 = ViewStart;
    }
Tail:
    if (MaxLen != 0)
        bCanInsert = Right.Len() + Left.Len() < MaxLen;
    return 1;
}
NW_HOOK_METHOD(0x1000c740, NCEditBox, OnChar)

// the arrows, Home and End wait for the IME to finish composing (except the Chinese layouts)
#define EDIT_MOVE_OK(IMEType) (IMEManager->IsNotComposing() == 1 || (IMEType) == 9 || (IMEType) == 11 || \
                               (IMEType) == 10 || (IMEType) == 12)

// 0x1000d860 (vslot 58) - editing keys. C2: in the Thai client the caret moves and deletes by clusters (the
// ThaiLeft / ThaiRight arrays follow every change), a digits-only box (Type 3) steps over its thousands
// separators and formats again, and Delete is its own case. Delete without Thai is Right + Backspace.
INT NCEditBox::OnKeyDown(WPARAM wParam, LPARAM lParam)
{
    INT IMEType = GConsole->GetIMEType();
    switch (wParam)
    {
    case VK_LEFT:
        if (EDIT_MOVE_OK(IMEType))
        {
            if (Left.Len() > 0)
            {
                if (GLanguageType == 5)
                {
                    if (CaretX < 13)
                        ViewStart -= ThaiLeft(ThaiLeft.Num() - 1).CharCount();
                    if (ViewStart < 0)
                        ViewStart = 0;
                    FString Old(Right);
                    Right.Empty();
                    INT Count = ThaiLeft(ThaiLeft.Num() - 1).CharCount();
                    Right = Left.Right(Count);
                    Right += Old;
                    Left.Delete(Left.Len() - Count, Count);
                    NCGDevice::MakeThaiCharArray(*Left, ThaiLeft, 0);
                    NCGDevice::MakeThaiCharArray(*Right, ThaiRight, 0);
                }
                else
                {
                    if (CaretX < 13)
                        ViewStart--;
                    if (ViewStart < 0)
                        ViewStart = 0;
                    FString Old(Right);
                    Right.Empty();
                    Right = Left.Right(1);
                    Right += Old;
                    Left.Delete(Left.Len() - 1, 1);
                    if (Type == 3 && (*Right)[0] == ',')
                        OnKeyDown(VK_LEFT, 0);
                }
            }
        }
        break;
    case VK_RIGHT:
        if (EDIT_MOVE_OK(IMEType))
        {
            if (Right.Len() > 0)
            {
                if (GLanguageType == 5)
                {
                    if (ThaiRight.Num() > 0)
                    {
                        if (CaretX >= (DWORD)(Width - 12))
                            ViewStart += ThaiRight(0).CharCount();
                        INT Count = ThaiRight(0).CharCount();
                        Left += Right.Left(Count);
                        Right.Delete(0, Count);
                        NCGDevice::MakeThaiCharArray(*Left, ThaiLeft, 0);
                        NCGDevice::MakeThaiCharArray(*Right, ThaiRight, 0);
                    }
                }
                else
                {
                    if (CaretX >= (DWORD)(Width - 12))
                        ViewStart++;
                    Left += Right.Left(1);
                    Right.Delete(0, 1);
                }
                if (Type == 3 && (*Right)[0] == ',')
                    OnKeyDown(VK_RIGHT, 0);
            }
        }
        break;
    case VK_UP:
        if (Type == 1)
            NotifyParent(4);
        break;
    case VK_DOWN:
        if (Type == 1)
            NotifyParent(5);
        break;
    case VK_HOME:
        if (EDIT_MOVE_OK(IMEType))
        {
            Left += Right;
            Right = Left;
            Left.Empty();
            if (GLanguageType == 5)
            {
                NCGDevice::MakeThaiCharArray(*Left, ThaiLeft, 0);
                NCGDevice::MakeThaiCharArray(*Right, ThaiRight, 0);
            }
            ViewStart = 0;
        }
        break;
    case VK_END:
        if (EDIT_MOVE_OK(IMEType))
        {
            Left += Right;
            Right.Empty();
            if (GLanguageType == 5)
            {
                NCGDevice::MakeThaiCharArray(*Left, ThaiLeft, 0);
                NCGDevice::MakeThaiCharArray(*Right, ThaiRight, 0);
            }
            ViewStart = ViewStart2;
        }
        break;
    case VK_BACK:
        // only while nothing is being composed (the other keys also with the Chinese windows)
        if (IMEManager->IsNotComposing() == 1 && Left.Len() > 0)
        {
            if (Type == 3 && (*Left)[Left.Len() - 1] == ',')
            {
                // a separator goes with the digit before it: Backspace again
                FString Old(Right);
                Right.Empty();
                Right = Left.Right(1);
                Right += Old;
                Left.Delete(Left.Len() - 1, 1);
                OnKeyDown(VK_BACK, 0);
                break;
            }
            // the view scrolls back one character (cluster), or two when a wide one was removed and the two
            // before the view are narrow
            SIZE Removed;
            NCGDevice::GetTextExtent(&Removed, *Left.Mid(Left.Len() - 1, 1), 0, 0, 0, 0, 0, L2FONT_Normal);
            if (GLanguageType == 5)
            {
                TArray<ThaiCharacter> Before;
                NCGDevice::MakeThaiCharArray(*Left.Left(ViewStart), Before, 0);
                INT Num = Before.Num();
                INT LastW = 0;
                INT LastTwoW = 0;
                if (Num >= 1)
                    LastW = Before(Num - 1).GetWidth(GCanvas, 0);
                if (Num >= 2)
                {
                    LastTwoW = Before(Num - 1).GetWidth(GCanvas, 0);
                    LastTwoW += Before(Num - 2).GetWidth(GCanvas, 0);
                }
                if (ViewStart > 0)
                {
                    if (Removed.cx >= 12 && LastTwoW <= 12 && LastTwoW > 6)
                    {
                        INT Cur = Before(Num - 1).CharCount();
                        INT Prev = Before(Num - 2).CharCount();
                        ViewStart -= Cur;
                        ViewStart -= Prev;
                        ViewStart2 -= Cur + Prev;
                    }
                    else if (!(Removed.cx < 12 && LastW >= 12 &&
                               GetTextWidth(FString(*Left + ViewStart)) >= Width - 12) &&
                             !(Removed.cx < LastW && GetTextWidth(FString(*Left + ViewStart)) >= Width - 12))
                    {
                        INT Cur = Before(Num - 1).CharCount();
                        ViewStart -= Cur;
                        ViewStart2 -= Cur;
                    }
                }
            }
            else
            {
                SIZE Before1, Before2;
                if (ViewStart >= 1)
                    NCGDevice::GetTextExtent(&Before1, *Left.Mid(ViewStart - 1, 1), 0, 0, 0, 0, 0, L2FONT_Normal);
                else
                    Before1.cx = 0;
                if (ViewStart >= 2)
                    NCGDevice::GetTextExtent(&Before2, *Left.Mid(ViewStart - 2, 2), 0, 0, 0, 0, 0, L2FONT_Normal);
                else
                    Before2.cx = 0;
                if (ViewStart > 0)
                {
                    if (Removed.cx >= 12 && Before2.cx > 6 && Before2.cx <= 12)
                    {
                        ViewStart -= 2;
                        ViewStart2 -= 2;
                    }
                    else if (!(Removed.cx < 12 && Before1.cx >= 12 &&
                               GetTextWidth(FString(*Left + ViewStart)) >= Width - 12) &&
                             !(Removed.cx < Before1.cx &&
                               GetTextWidth(FString(*Left + ViewStart)) >= Width - 12))
                    {
                        ViewStart--;
                        ViewStart2--;
                    }
                }
            }
            Left.Delete(Left.Len() - 1, 1);
            if (GLanguageType == 5)
            {
                NCGDevice::MakeThaiCharArray(*Left, ThaiLeft, 0);
                NCGDevice::MakeThaiCharArray(*Right, ThaiRight, 0);
            }
            if (GetTextWidth(Left) + GetTextWidth(Comp) + GetTextWidth(Right) <= Width)
                ViewStart = 0;
            if (Type == 3 && Left.Len() + Right.Len() > 1)
                ReformatNumber();
        }
        break;
    case VK_DELETE:
        if (Right.Len() > 0)
        {
            if (GLanguageType == 5)
            {
                INT Count = ThaiRight(0).CharCount();
                Right.Delete(0, Count);
                NCGDevice::MakeThaiCharArray(*Left, ThaiLeft, 0);
                NCGDevice::MakeThaiCharArray(*Right, ThaiRight, 0);
            }
            else
            {
                OnKeyDown(VK_RIGHT, 0);
                OnKeyDown(VK_BACK, 0);
            }
            if (Type == 3 && Right.Num() != 0 && (*Right)[0] == ',')
                OnKeyDown(VK_RIGHT, 0);
        }
        if (MaxLen != 0)
            bCanInsert = Right.Len() + Left.Len() < MaxLen;
        break;
    case VK_RETURN:
        ViewStart = 0;
        ViewStart2 = 0;
        if (Left.Len() == 0 && Comp.Len() == 0 && Right.Len() == 0)
            NotifyParent(0);
        else if (Comp.Len() == 0)
            NotifyParent(1);
        break;
    case VK_PROCESSKEY:
        // Chinese composition window: the arrows move its caret (scan codes in lParam)
        if (IMEType == 10)
        {
            if (lParam == 0x14B0001)
                ChineseCompositeBox(IMEManager)->CaretLeft();
            else if (lParam == 0x14D0001)
                ChineseCompositeBox(IMEManager)->CaretRight();
        }
        break;
    }
    NCWnd::OnKeyDown(wParam, lParam);
    return 1;
}
NW_HOOK_METHOD(0x1000d860, NCEditBox, OnKeyDown)

// 0x1000a070 (vslot 68) - Left + Right joined in Left; NULL when there is no text. C2: a digits-only box
// (Type 3) loses the thousands separators the number formatting put in
const TCHAR* NCEditBox::CommitText()
{
    if (Left.Len() == 0 && Right.Len() == 0)
        return NULL;
    Left += Right;
    Right.Empty();
    if (Type == 3)
    {
        for (INT i = 0; i < Left.Len(); i++)
        {
            if ((*Left)[i] == ',')
                Left.Delete(i, 1);
        }
    }
    return *Left;
}
NW_HOOK_METHOD(0x1000a070, NCEditBox, CommitText)

// 0x1000a170 (vslot 69) - C2: text may be inserted again
void NCEditBox::ClearText()
{
    Left.Empty();
    Right.Empty();
    Comp.Empty();
    ViewStart = 0;
    bCanInsert = 1;
}
NW_HOOK_METHOD(0x1000a170, NCEditBox, ClearText)

// 0x1000a900 (vslot 70) - the composition string, cut to what fits (C2: the box's MaxLen too)
INT NCEditBox::OnComposition(WPARAM wParam, LPARAM lParam)
{
    if (Unknown1C8 == 1 && (wParam < 0x20 || wParam > 0x81))
        return 1;
    bReading = 0;
    Reading.Empty();
    if (Type == 3 && (wParam < '0' || wParam > '9'))
        return 1;
    SIZE Size;
    NCGDevice::GetTextExtent(&Size, *Left, 0, 0, 0, 0, 0, L2FONT_Normal);
    INT TextW = Size.cx;
    NCGDevice::GetTextExtent(&Size, *Right, 0, 0, 0, 0, 0, L2FONT_Normal);
    TextW += Size.cx;
    if (MaxLen == 0)
        bCanInsert = TextW < MaxWidth - 12;
    else
        bCanInsert = Left.Len() + Right.Len() < MaxLen;
    if (bCanInsert)
    {
        Comp.Empty();
        const TCHAR* Str = IMEManager->GetCompStr();
        if (Str)
            Comp = Str;
        NCGDevice::GetTextExtent(&Size, *Comp, 0, 0, 0, 0, 0, L2FONT_Normal);
        if (MaxLen == 0)
        {
            if (Size.cx + TextW < MaxWidth - 12)
                bCanInsert = 1;
            else
            {
                INT i;
                for (i = 0; i < Comp.Len(); i++)
                {
                    Comp = Comp.Left(Comp.Len() - 1);
                    NCGDevice::GetTextExtent(&Size, *Comp, 0, 0, 0, 0, 0, L2FONT_Normal);
                    if (Size.cx + TextW < MaxWidth - 12)
                        break;
                }
                if (i == Comp.Len())
                    Comp.Empty();
                bCanInsert = 0;
            }
        }
        else
            bCanInsert = Left.Len() + Right.Len() < MaxLen;
        if (GConsole->GetIMEType() == 10 || GConsole->GetIMEType() == 9)
        {
            NCCompositeBox* Box = ChineseCompositeBox(IMEManager);
            FString Old(ChineseLastComp(IMEManager));
            Box->SetText(*Comp);
            if (Old.Len() > Comp.Len())
                Box->CaretLeft();
            else if (Old.Len() < Comp.Len())
                Box->CaretRight();
            Box->SetText(*Comp);
        }
        if (MaxLen != 0)
            bCanInsert = Right.Len() + Left.Len() < MaxLen;
    }
    return 1;
}
NW_HOOK_METHOD(0x1000a900, NCEditBox, OnComposition)

// 0x1000a640 (vslot 71) - the result string (digits only for Type 3) is added to Left
INT NCEditBox::OnResult(WPARAM wParam, LPARAM lParam)
{
    if (EDIT_CODE_OK(wParam))
    {
        bReading = 0;
        Reading.Empty();
        if (bCanInsert)
        {
            const TCHAR* Str;
            if (Type == 3)
            {
                for (INT i = 0; i < Comp.Len(); i++)
                {
                    if ((*Comp.Mid(i, 1))[0] < '0' || (*Comp.Mid(i, 1))[0] > '9')
                        return 1;
                }
                Str = *Comp;
            }
            else
                Str = IMEManager->GetResultStr();
            if (Str)
            {
                if (Left.Len() == 0)
                    Left = Str;
                else
                    Left += Str;
                if (TEXT_W(Left) + TEXT_W(Comp) + TEXT_W(Right) < MaxWidth - 12)
                {
                    if (Width < GetTextWidth(FString(*Left + ViewStart)) + 12)
                    {
                        SIZE Size;
                        NCGDevice::GetTextExtent(&Size, *Left.Mid(ViewStart, 1), 0, 0, 0, 0, 0, L2FONT_Normal);
                        if (Size.cx < 12)
                            ViewStart += 2;
                        else
                            ViewStart += 1;
                        ViewStart2 = ViewStart;
                    }
                }
                Comp.Empty();
            }
            if (MaxLen != 0)
                bCanInsert = Right.Len() + Left.Len() < MaxLen;
        }
    }
    return 1;
}
NW_HOOK_METHOD(0x1000a640, NCEditBox, OnResult)

// 0x1000acc0 (vslot 72) - the composition attribute: on (1) shows the composition when the line has
// room, off (0 / 1) clears it. C2: nothing for codes the box does not take, MaxLen limits the text
INT NCEditBox::OnAttribute(WPARAM wParam, LPARAM lParam)
{
    if (EDIT_CODE_OK(wParam))
    {
        bReading = 0;
        Reading.Empty();
        if (Type != 3 || (wParam > 0x2F && wParam < 0x3A))
        {
            const TCHAR* Str = IMEManager->GetCompStr();
            if (Str)
            {
                if (wParam == 0)
                {
                    if (lParam == 0)
                        Comp = Str;
                }
                else if (wParam == 1)
                {
                    if (lParam == 0)
                    {
                        if (TEXT_W(Left) + TEXT_W(Comp) + TEXT_W(Right) < MaxWidth - 12)
                            Comp = Str;
                    }
                    else if (lParam == 1)
                        Comp.Empty();
                }
            }
            if (MaxLen != 0)
                bCanInsert = Right.Len() + Left.Len() < MaxLen;
        }
    }
    return 1;
}
NW_HOOK_METHOD(0x1000acc0, NCEditBox, OnAttribute)

// 0x1000ae40 (vslot 73) - the reading string, when the line has room (C2: and the codes are taken)
INT NCEditBox::OnReading(WPARAM wParam, LPARAM lParam)
{
    if (EDIT_CODE_OK(wParam))
    {
        bReading = 1;
        Reading.Empty();
        if (Type != 3 || (wParam > 0x2F && wParam < 0x3A))
        {
            const TCHAR* Str = IMEManager->GetReadingStr();
            if (Str)
            {
                ReadingParam = lParam;
                if (MaxLen != 0)
                {
                    if (Right.Len() + Left.Len() < MaxLen)
                    {
                        Reading = Str;
                        bCanInsert = 1;
                        return 1;
                    }
                    bCanInsert = 0;
                    return 1;
                }
                if (TEXT_W(Left) + TEXT_W(Comp) + TEXT_W(Right) < MaxWidth - 12)
                    Reading = Str;
            }
            return 1;
        }
    }
    return 1;
}
NW_HOOK_METHOD(0x1000ae40, NCEditBox, OnReading)

// 0x1000afb0 (vslot 75) - answers the IME's NIM_GETCARET to the window that asked
INT NCEditBox::OnGetCaret(WPARAM wParam, LPARAM lParam)
{
    if (EDIT_CODE_OK(wParam) && wParam)
        SendTo((NWndEntry*)wParam, NIM_CARET, IMECaretX, IMECaretY);
    return 1;
}
NW_HOOK_METHOD(0x1000afb0, NCEditBox, OnGetCaret)

// 0x1000aff0 (vslot 76)
INT NCEditBox::OnEndComposition(WPARAM wParam, LPARAM lParam)
{
    if (EDIT_CODE_OK(wParam))
    {
        if (IMEManager->IsNotComposing() == 1)
            Comp.Empty();
    }
    return 1;
}
NW_HOOK_METHOD(0x1000aff0, NCEditBox, OnEndComposition)

// 0x1000b040 (vslot 77) - NIM_CURSORPOS: C2 drops the codes the English client does not take
INT NCEditBox::OnCursorPos(WPARAM wParam, LPARAM lParam)
{
    if (EDIT_CODE_OK(wParam))
        CursorPos = wParam;
    return 1;
}
NW_HOOK_METHOD(0x1000b040, NCEditBox, OnCursorPos)

// 0x1000a060 - Left only
void NCEditBox::SetText(const TCHAR* InText)
{
    Left = InText;
}
NW_HOOK_METHOD(0x1000a060, NCEditBox, SetText)

// 0x10009510 (C2 only) - the longest text the box takes (MaxLen); anything but a positive number leaves it as it is
void NCEditBox::SetMaxLength(INT InMaxLen)
{
    if (InMaxLen > 0)
        MaxLen = InMaxLen;
}
NW_HOOK_METHOD(0x10009510, NCEditBox, SetMaxLength)

// The view start is moved on until what is left of Left, from it, fits the box (Width - 12 px): by
// characters, in the Thai client by clusters. The stock InsertText and DeleteChar both do it inline.
static FORCEINLINE void FitViewToLeft(NCEditBox* E)
{
    if (GLanguageType == 5)
    {
        NCGDevice::MakeThaiCharArray(*E->Left, E->ThaiLeft, 0);
        INT Cluster = E->IMEManager->GetTextLength(E->Left.Mid(0, E->ViewStart));
        while (E->Left.Len() >= E->ViewStart)
        {
            if (E->GetTextWidth(E->Left.Mid(E->ViewStart, E->Left.Len() - E->ViewStart)) <= E->Width - 12)
                break;
            E->ViewStart += E->ThaiLeft(Cluster).CharCount();
            Cluster++;
        }
    }
    else
    {
        while (E->Left.Len() >= E->ViewStart)
        {
            if (E->GetTextWidth(E->Left.Mid(E->ViewStart, E->Left.Len() - E->ViewStart)) <= E->Width - 12)
                break;
            E->ViewStart++;
        }
    }
}

// 0x1000c250 - the text goes after Left (bAppend) or before it. C2: the number format of Type 3, the
// Thai cluster array and a view that is moved on until the end of the text fits
void NCEditBox::InsertText(const TCHAR* InText, UBOOL bAppend)
{
    if (bAppend)
        Left += InText;
    else
    {
        FString Old;
        Old = Left;
        Left = InText;
        Left += Old;
    }
    if (Type == 3)
        ReformatNumber();
    FitViewToLeft(this);
    if (MaxLen != 0)
        bCanInsert = Right.Len() + Left.Len() < MaxLen;
}
NW_HOOK_METHOD(0x1000c250, NCEditBox, InsertText)

// 0x1000c470 - Index counts through Left, Comp and Right. C2: positions in Comp and Right are counted from
// their starts (C1 counted them from the ends); the Thai cluster arrays and MaxLen follow
void NCEditBox::DeleteChar(INT Index)
{
    if (Index < Left.Len())
    {
        Left.Delete(Index, 1);
        FitViewToLeft(this);
    }
    else
    {
        if (Index >= Left.Len() && Index < Comp.Len() + Left.Len())
        {
            if (Left.Num() == 0)
                Comp.Delete(Index, 1);
            else
                Comp.Delete(Index - (Left.Num() - 1), 1);
            goto Tail;
        }
        if (Index >= Comp.Len() + Left.Len() && Index < Right.Len() + Comp.Len() + Left.Len())
        {
            Right.Delete(Index - Comp.Len() - Left.Len(), 1);
            if (GLanguageType == 5)
                NCGDevice::MakeThaiCharArray(*Right, ThaiRight, 0);
        }
    }
Tail:
    if (MaxLen != 0)
        bCanInsert = Right.Len() + Left.Len() < MaxLen;
}
NW_HOOK_METHOD(0x1000c470, NCEditBox, DeleteChar)

// 0x1000a100
INT NCEditBox::GetTextWidth(FString Text)
{
    SIZE Size;
    NCGDevice::GetTextExtent(&Size, *Text, 0, 0, 0, 0, 0, L2FONT_Normal);
    return Size.cx;
}
NW_HOOK_METHOD(0x1000a100, NCEditBox, GetTextWidth)

// 0x1000a1b0 (name ours) - a digits-only box (Type 3): the digits get their thousands separators again
// (Left and Right are joined by CommitText, which strips the old ones) and the caret stays where it was
// among the digits. Called by InsertText, OnChar, OnKeyDown and the calculator's edit control. Only the
// first Length characters of both strings are looked at for separators (as in the stock code); the
// buffers hold 128 characters, as the stock ones.
void NCEditBox::ReformatNumber()
{
    TCHAR Before[128];
    TCHAR After[128];
    INT AfterCommas = 0;
    INT BeforeCommas = 0;
    appStrcpy(Before, *Left);
    INT Length = appStrlen(Before);
    After[0] = 0;   // not in the stock code, which leaves it unset when there is no text (not reachable)
    FormatNumber(After, CommitText());
    for (INT i = 0; i < Length; i++)
    {
        if (After[i] == ',')
            AfterCommas++;
    }
    for (INT j = 0; j < Length; j++)
    {
        if (Before[j] == ',')
            BeforeCommas++;
    }
    Length += AfterCommas - BeforeCommas;
    Left = After;
    if (Length < Left.Len())
    {
        Right = *Left + Length;
        Left.Delete(Length, Left.Len() - Length);
    }
    if (GLanguageType == 5)
    {
        NCGDevice::MakeThaiCharArray(*Left, ThaiLeft, 0);
        NCGDevice::MakeThaiCharArray(*Right, ThaiRight, 0);
    }
    if (MaxLen != 0)
        bCanInsert = Right.Len() + Left.Len() < MaxLen;
}
NW_HOOK_METHOD(0x1000a1b0, NCEditBox, ReformatNumber)

// 0x1000a310 (name ours) - with the Thai keyboard (IME type 13) a Thai character is taken only where the
// Thai spelling allows it after the text typed so far: spaces, consonants, vowels before/after, symbols
// and digits always; sara am after a space, a consonant, or a consonant and a tone mark; a vowel above or
// below after a space or a consonant; a tone mark after those or a vowel above/below. 0 for the rest
// and for every other keyboard.
UBOOL NCEditBox::IsThaiInputValid(UINT Ch)
{
    TCHAR Prev = 0;
    TCHAR Prev2 = 0;
    if (GConsole->GetIMEType() == 13)
    {
        switch (IMEManager->GetThaiCharType(Ch))
        {
        case 1:
        case 2:
        case 5:
        case 6:
        case 7:
            return 1;
        case 3:
            if (Left.Len() > 0)
            {
                Prev = (*Left.Right(1))[0];
                if (Left.Len() > 1)
                    Prev2 = (*Left.Right(2))[0];
                if (IMEManager->GetThaiCharType(Prev) == 1)
                    return 1;
                if (IMEManager->GetThaiCharType(Prev) == 2)
                    return 1;
                if (IMEManager->GetThaiCharType(Prev) == 8 && Left.Len() > 1 &&
                    IMEManager->GetThaiCharType(Prev2) == 2)
                    return 1;
            }
            break;
        case 4:
            if (Left.Len() > 0)
            {
                Prev2 = (*Left.Right(1))[0];
                if (IMEManager->GetThaiCharType(Prev2) == 1)
                    return 1;
                if (IMEManager->GetThaiCharType(Prev2) == 2)
                    return 1;
            }
            break;
        case 8:
        case 9:
            if (Left.Len() > 0)
            {
                Prev2 = (*Left.Right(1))[0];
                if (IMEManager->GetThaiCharType(Prev2) == 1)
                    return 1;
                if (IMEManager->GetThaiCharType(Prev2) == 2)
                    return 1;
                if (IMEManager->GetThaiCharType(Prev2) == 4)
                    return 1;
            }
            break;
        }
    }
    return 0;
}
NW_HOOK_METHOD(0x1000a310, NCEditBox, IsThaiInputValid)

// 0x1000b190 (name ours) - the width of the cluster in the font FontType of Canvas: the base glyph
// (W[7] when there is one, else W[0], else W[5]), or with a mark above (W[1]) the mark's glyph plus the
// one of W[0] (sara am counts as sara aa 0xE32); 0 for a missing canvas, font or glyph
INT ThaiCharacter::GetWidth(UCanvas* Canvas, INT FontType) const
{
    INT Result = 0;
    if (Canvas && Canvas->L2Fonts[FontType])
    {
        UL2Font* Font = Canvas->L2Fonts[FontType];
        FL2FontChar* Glyph;
        if (W[7] == 0)
        {
            if (W[1] != 0)
            {
                Glyph = Font->GetChar(W[1]);
                if (Glyph)
                    Result = Glyph->Width;
                WORD Mark = W[0];
                if (Mark == 0)
                    return Result;
                if (Mark != 0xE33)
                {
                    Glyph = Font->GetChar(Mark);
                    if (!Glyph)
                        return Result;
                    return Result + Glyph->Width;
                }
                Glyph = Font->GetChar(0xE32);
                if (!Glyph)
                    return Result;
                return Result + Glyph->Width;
            }
            if (W[0] == 0)
            {
                if (W[5] == 0)
                    return 0;
                Glyph = Font->GetChar(W[5]);
            }
            else
                Glyph = Font->GetChar(W[0]);
        }
        else
            Glyph = Font->GetChar(W[7]);
        if (Glyph)
            Result = Glyph->Width;
    }
    return Result;
}
NW_HOOK_METHOD(0x1000b190, ThaiCharacter, GetWidth)

// 0x1000a610
void NCEditBox::SetType(INT InType)
{
    Type = InType;
}
NW_HOOK_METHOD(0x1000a610, NCEditBox, SetType)

// C2 CANDIDATE: the C2 function is not the same code as the C1 one; kept only while it matches the stock one (verify_data)
// 0x10009530 - the character being composed: black on white while the caret blinks on
void NCEditBox::PaintComposingKR(INT X, INT Y, INT TextW, INT CompW, NCGDevice* Device)
{
    if (bCaretOn)
    {
        Device->DrawStretchedTex(X + TextW, Y, CompW, 12, WhiteTex);
        Device->DrawText(X + TextW, Y, 0xFF000000, *Comp, 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
        return;
    }
    Device->DrawText(X + TextW, Y, 0xFFFFFFFF, *Comp, 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
}
NW_HOOK_METHOD(0x10009530, NCEditBox, PaintComposingKR)

// 0x10009c50 - the composition character by character; IMECaretX goes to the first target
// clause and characters with attribute bit 2 get a red underline
void NCEditBox::PaintCompositionKR(INT X, INT Y, INT TextW, INT Offset, NCGDevice* Device)
{
    IMECaretX = TextW;
    NCIME* IME = IMEManager->KoreanIME;
    INT LineY = bIMEPosFixed ? Y - 1 : Y + 13;
    DWORD* Clause = &IME->CompClause[1];
    UBOOL bTarget = 0;
    for (INT i = 0; i < Comp.Len(); i++)
    {
        if ((IME->CompAttr[i] & 1) && !bTarget)
        {
            bTarget = 1;
            IMECaretX = X + TextW + Offset;
        }
        INT CharX = X + TextW + Offset;
        Offset += Device->DrawText(CharX, Y, 0xFFFFFFFF, *Comp.Mid(i, 1), 0, 0, 0, L2FONT_Normal, 0, 0,
                                   0, 0, 0, 0, 0);
        INT EndX = X + TextW + Offset;
        if (*Clause == (DWORD)(i + 1))
        {
            Clause++;
            EndX--;
        }
        if (IsFocused() && (IME->CompAttr[i] & 4))
            Device->DrawLine(CharX, LineY, EndX, LineY, 0xFFFF0000, 0);
    }
}
NW_HOOK_METHOD(0x10009c50, NCEditBox, PaintCompositionKR)

// 0x100095f0 - the composition gets a dotted underline (not for the IME types 5 and 7; while
// there is none, the type 7 blinks a caret at TextW - 1, not past X). With a reading string at
// the target clause (ClauseW, see PaintCompositionTW): the type 6 draws it in a column of
// 12 px cells, over the box when the IME position is fixed (the characters from the error
// position on in red on gray, the others in blue) or below it (red and blue only); the type 4
// in the line, black on gray.
void NCEditBox::PaintComposingTW(INT X, INT Y, INT TextW, INT CompW, INT ClauseW, INT ReadingW,
                                 NCGDevice* Device)
{
    if (CompW != 0)
    {
        if (GConsole->GetIMEType() != 5 && GConsole->GetIMEType() != 7)
        {
            for (INT i = 0; i < CompW; i += 3)
                Device->DrawText(X + TextW + i - 2, Y + 3, 0xFFFFFFFF, TEXT("."), 0, 0, 0, L2FONT_Normal, 0,
                                 0, 0, 0, 0, 0, 0);
        }
    }
    else if (GConsole->GetIMEType() == 7 && IsFocused() && bCaretOn)
        Device->DrawText(TextW - 1, Y, 0xFFFFFFFF, TEXT("|"), 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0, 0);

    if (!bReading)
        return;
    if (GConsole->GetIMEType() == 6)
    {
        INT RX = X + TextW + ClauseW;
        if (bIMEPosFixed)
        {
            Device->SetClip(ScreenX, ScreenY - 100, Width, Height + 100);
            INT RY = Y - 42;
            Device->DrawStretchedTex(RX, RY, 12, 48, FindTexture(TEXT("NWindow.WhiteTexture")));
            for (INT i = 0; i < Reading.Len(); i++)
            {
                if (i >= ReadingParam)
                {
                    Device->DrawStretchedTex(RX, RY, 12, 12, FindTexture(TEXT("NWindow.GrayTexture")));
                    Device->DrawText(RX, RY, 0xFFFF0000, *Reading.Mid(i, 1), 0, 0, 0, L2FONT_Normal, 0, 0,
                                     0, 0, 0, 0, 0);
                }
                else
                    Device->DrawText(RX, RY, 0xFF0000FF, *Reading.Mid(i, 1), 0, 0, 0, L2FONT_Normal, 0, 0,
                                     0, 0, 0, 0, 0);
                RY += 12;
            }
        }
        else
        {
            Device->SetClip(ScreenX, ScreenY, Width, Height + 100);
            INT RY = Y + 6;
            Device->DrawStretchedTex(RX, RY, 12, 48, FindTexture(TEXT("NWindow.WhiteTexture")));
            for (INT i = 0; i < Reading.Len(); i++)
            {
                if (i >= ReadingParam)
                    Device->DrawText(RX, RY, 0xFFFF0000, *Reading.Mid(i, 1), 0, 0, 0, L2FONT_Normal, 0, 0,
                                     0, 0, 0, 0, 0);
                else
                    Device->DrawText(RX, RY, 0xFF0000FF, *Reading.Mid(i, 1), 0, 0, 0, L2FONT_Normal, 0, 0,
                                     0, 0, 0, 0, 0);
                RY += 12;
            }
        }
        Device->ReleaseClip();
    }
    else if (GConsole->GetIMEType() == 4)
    {
        INT RX = X + TextW + ClauseW;
        Device->DrawStretchedTex(RX, Y, ReadingW, 12, FindTexture(TEXT("NWindow.GrayTexture")));
        Device->DrawText(RX, Y, 0xFF000000, *Reading, 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0, 0);
    }
}
NW_HOOK_METHOD(0x100095f0, NCEditBox, PaintComposingTW)

// 0x10009a30 - the Korean one's (see there) with the lines under the text by attribute while
// focused, at Y + 13 whatever bIMEPosFixed says: the target clause (bit 0) twice, one line for
// bits 0 or 1 (both only under characters with a width), one more for bit 1, a red one for
// bit 2
void NCEditBox::PaintCompositionJP(INT X, INT Y, INT TextW, INT Offset, NCGDevice* Device)
{
    IMECaretX = TextW;
    NCIME* IME = IMEManager->JapaneseIME;
    INT LineY = Y + 13;
    DWORD* Clause = &IME->CompClause[1];
    UBOOL bTarget = 0;
    for (INT i = 0; i < Comp.Len(); i++)
    {
        if ((IME->CompAttr[i] & 1) && !bTarget)
        {
            bTarget = 1;
            IMECaretX = X + TextW + Offset;
        }
        INT CharX = X + TextW + Offset;
        INT CharW = Device->DrawText(CharX, Y, 0xFFFFFFFF, *Comp.Mid(i, 1), 0, 0, 0, L2FONT_Normal, 0, 0,
                                     0, 0, 0, 0, 0);
        Offset += CharW;
        INT EndX = X + TextW + Offset;
        if (*Clause == (DWORD)(i + 1))
        {
            Clause++;
            EndX--;
        }
        if (IsFocused())
        {
            if ((IME->CompAttr[i] & 1) && CharW > 0)
            {
                Device->DrawLine(CharX, LineY, EndX, LineY + 1, 0xFFFFFFFF, 0);
                Device->DrawLine(CharX, LineY, EndX, LineY, 0xFFFFFFFF, 0);
            }
            if ((IME->CompAttr[i] & 3) && CharW > 0)
                Device->DrawLine(CharX, LineY, EndX, LineY, 0xFFFFFFFF, 0);
            if (IME->CompAttr[i] & 2)
                Device->DrawLine(CharX, LineY, EndX, LineY, 0xFFFFFFFF, 0);
            if (IME->CompAttr[i] & 4)
                Device->DrawLine(CharX, LineY, EndX, LineY, 0xFFFF0000, 0);
        }
    }
}
NW_HOOK_METHOD(0x10009a30, NCEditBox, PaintCompositionJP)

// 0x10009e00 - the Korean one's (see there) with the target clause (attribute bit 0) drawn
// black on gray while focused. *ClauseW gets the offset of the first target character, but
// only when that character is drawn 0 px wide (as in the stock code).
void NCEditBox::PaintCompositionTW(INT X, INT Y, INT TextW, INT* ClauseW, NCGDevice* Device)
{
    IMECaretX = TextW;
    NCIME* IME = IMEManager->TaiwanIME;
    INT Offset = 0;
    INT LineY = bIMEPosFixed ? Y - 1 : Y + 13;
    DWORD* Clause = &IME->CompClause[1];
    UBOOL bTarget = 0;
    for (INT i = 0; i < Comp.Len(); i++)
    {
        INT CharX = X + TextW + Offset;
        INT CharW = Device->DrawText(CharX, Y, 0xFFFFFFFF, *Comp.Mid(i, 1), 0, 0, 0, L2FONT_Normal, 0, 0,
                                     0, 0, 0, 0, 0);
        if ((IME->CompAttr[i] & 1) && !bTarget)
        {
            bTarget = 1;
            IMECaretX = CharX;
            if (!CharW)
                *ClauseW = Offset;
        }
        Offset += CharW;
        INT EndX = X + TextW + Offset;
        if (*Clause == (DWORD)(i + 1))
        {
            Clause++;
            EndX--;
        }
        if (IsFocused())
        {
            if ((IME->CompAttr[i] & 1) && CharW > 0)
            {
                Device->DrawStretchedTex(CharX, Y, EndX - CharX, 12, FindTexture(TEXT("NWindow.GrayTexture")));
                Device->DrawText(CharX, Y, 0xFF000000, *Comp.Mid(i, 1), 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0,
                                 0, 0, 0);
            }
            if (IME->CompAttr[i] & 4)
                Device->DrawLine(CharX, LineY, EndX, LineY, 0xFFFF0000, 0);
        }
    }
}
NW_HOOK_METHOD(0x10009e00, NCEditBox, PaintCompositionTW)

// 0x1000b070 - the caret of the composition: Left's width plus the composition's up to
// CursorPos
INT NCEditBox::GetCompCaretX()
{
    SIZE Size;
    FString Str;
    NCGDevice::GetTextExtent(&Size, *Left, 0, 0, 0, 0, 0, L2FONT_Normal);
    INT LeftW = Size.cx;
    Str = Comp.Left(CursorPos);
    NCGDevice::GetTextExtent(&Size, *Str, 0, 0, 0, 0, 0, L2FONT_Normal);
    return Size.cx + LeftW;
}
NW_HOOK_METHOD(0x1000b070, NCEditBox, GetCompCaretX)

// 0x1000b530 - the characters still being typed (attribute 0) get a dotted underline from the
// first of the last run of them, and the caret blinks as a double bar at CursorPos (see
// GetCompCaretX, which also gives CaretX); CompW is not used
void NCEditBox::PaintComposingJP(INT X, INT Y, INT TextW, INT CompW, NCGDevice* Device)
{
    SIZE Size;
    CaretX = GetCompCaretX();
    INT UnderlineW = 0;
    INT StartX = 0;
    for (INT i = 0; i < IMEManager->JapaneseIME->CompAttrLen; i++)
    {
        NCGDevice::GetTextExtent(&Size, *Comp.Mid(i, 1), 0, 0, 0, 0, 0, L2FONT_Normal);
        NCIME* IME = IMEManager->JapaneseIME;
        if (IME->CompAttr[i] == ATTR_INPUT)
        {
            if (i == 0 || IME->CompAttr[i - 1] != ATTR_INPUT)
            {
                NCGDevice::GetTextExtent(&Size, *Comp.Mid(0, i), 0, 0, 0, 0, 0, L2FONT_Normal);
                StartX = Size.cx;
                NCGDevice::GetTextExtent(&Size, *Comp.Mid(i, 1), 0, 0, 0, 0, 0, L2FONT_Normal);
                UnderlineW += Size.cx;
            }
            else
            {
                NCGDevice::GetTextExtent(&Size, *Comp.Mid(i, 1), 0, 0, 0, 0, 0, L2FONT_Normal);
                UnderlineW += Size.cx;
            }
        }
    }
    if (UnderlineW > 0)
    {
        for (INT j = 0; j < UnderlineW; j += 3)
            Device->DrawText(X + TextW + StartX + j - 2, Y + 3, 0xFFFFFFFF, TEXT("."), 0, 0, 0,
                             L2FONT_Normal, 0, 0, 0, 0, 0, 0, 0);
    }
    if (bCaretOn)
    {
        Device->DrawText(CaretX + X - 1, Y, 0xFFFFFFFF, TEXT("|"), 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0, 0);
        Device->DrawText(CaretX + X, Y, 0xFFFFFFFF, TEXT("|"), 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0, 0);
    }
}
NW_HOOK_METHOD(0x1000b530, NCEditBox, PaintComposingJP)

// 0x1000b7e0 - the IME type 8 draws the composition itself: the part in view (from the view
// start past Left, as far as the box goes) with a dotted underline, the target characters
// (attribute 1) black on white with the composition window over each and the candidate box
// beside it (IMECaretY: 152 px - 137 but for the types 5, 9 and 12 - above the box when the
// IME position is fixed, else 17 px below), the caret after the composition when nothing is
// the target, and the view scrolled to keep the target in it. The types 9 to 12 only blink
// the caret. CompW is not used.
void NCEditBox::PaintComposingCN(INT X, INT Y, INT TextW, INT CompW, NCGDevice* Device)
{
    if (IMEManager->IsNotComposing())
        return;
    if (GConsole->GetIMEType() == 8)
    {
        SIZE Size;
        INT Start = 0;
        INT VisibleW = 0;
        if (bIMEPosFixed)
        {
            if (GConsole->GetIMEType() == 5 || GConsole->GetIMEType() == 9 || GConsole->GetIMEType() == 12)
                IMECaretY = 152;
            else
                IMECaretY = 137;
        }
        else
            IMECaretY = 17;
        if (ViewStart >= Left.Len())
            Start = ViewStart - Left.Len();
        INT i;
        for (i = Start; i < Comp.Len(); i++)
        {
            NCGDevice::GetTextExtent(&Size, *Comp.Mid(i, 1), 0, 0, 0, 0, 0, L2FONT_Normal);
            if ((DWORD)(VisibleW + Size.cx + TextW) > (DWORD)Width)
                break;
            VisibleW += Size.cx;
        }
        INT CompX = X + TextW;
        Device->DrawText(CompX, Y, 0xFFFFFFFF, *Comp.Mid(Start, i - Start), 0, 0, 0, L2FONT_Normal, 0, 0, 0,
                         0, 0, 0, 0);
        for (INT j = 0; j < VisibleW; j += 3)
            Device->DrawText(CompX + j - 2, Y + 3, 0xFFFFFFFF, TEXT("."), 0, 0, 0, L2FONT_Normal, 0, 0, 0,
                             0, 0, 0, 0);

        IMECaretX = 0;
        INT OffX = 0;
        INT TargetW = 0;
        NCCandidateBox* Cand = ((NCChineseIME*)IMEManager->ChineseIME)->CandidateBox;
        for (i = Start; i < IMEManager->ChineseIME->CompAttrLen; i++)
        {
            NCGDevice::GetTextExtent(&Size, *Comp.Mid(i, 1), 0, 0, 0, 0, 0, L2FONT_Normal);
            if (IMEManager->ChineseIME->CompAttr[i] == ATTR_TARGET_CONVERTED)
            {
                INT CharX = CompX + OffX;
                TargetW = Size.cx;
                Device->DrawStretchedTex(CharX, Y, TargetW, 12, FindTexture(TEXT("NWindow.WhiteTexture")));
                Device->DrawText(CharX, Y, 0xFF000000, *Comp.Mid(i, 1), 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0,
                                 0, 0, 0);
                IMECaretX = CompX + OffX + 5;
                ChineseCompositeBox(IMEManager)->SetWindowRect(IMECaretX, Y - 10, 50, 15);
                Cand->SetWindowRect(IMECaretX, bIMEPosFixed ? -IMECaretY : IMECaretY, Cand->Width,
                                    Cand->Height);
            }
            OffX += Size.cx;
        }
        if (!TargetW && IsFocused() && bCaretOn)
            Device->DrawText(X + OffX + TextW - 2, Y, 0xFFFFFFFF, TEXT("|"), 0, 0, 0, L2FONT_Normal, 0, 0,
                             0, 0, 0, 0, 0);

        // the view follows the target: back while it is before the view start, on while the
        // text up to it is wider than the box
        INT ViewW = 0;
        if (ViewStart < Left.Len())
        {
            NCGDevice::GetTextExtent(&Size, *Comp.Mid(ViewStart, Comp.Len() - ViewStart), 0, 0, 0, 0, 0, L2FONT_Normal);
            ViewW = Size.cx;
        }
        INT k;
        for (k = 0; k < IMEManager->ChineseIME->CompAttrLen; k++)
        {
            NCGDevice::GetTextExtent(&Size, *Comp.Mid(k, 1), 0, 0, 0, 0, 0, L2FONT_Normal);
            if (k >= ViewStart - Left.Len())
                ViewW += Size.cx;
            if (IMEManager->ChineseIME->CompAttr[k] == ATTR_TARGET_CONVERTED)
                break;
        }
        if (k < ViewStart - Left.Len())
            ViewStart--;
        else if ((DWORD)ViewW > (DWORD)Width)
        {
            ViewStart++;
            ViewStart2 = ViewStart;
        }
    }
    else if (GConsole->GetIMEType() == 10 || GConsole->GetIMEType() == 9 || GConsole->GetIMEType() == 11 ||
             GConsole->GetIMEType() == 12)
    {
        if (IsFocused() && bCaretOn)
            Device->DrawText(X + TextW - 2, Y, 0xFFFFFFFF, TEXT("|"), 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0,
                             0, 0);
    }
}
NW_HOOK_METHOD(0x1000b7e0, NCEditBox, PaintComposingCN)

// 0x1000be60 - the Chinese IME's windows; nothing composed hides the composition window. The
// IME type 8 shows the reading string in it, 5 px past the composition's first character
// being typed or converted (attribute 0 or 1, counted from the view start) and 10 px up
// (hidden without one). The types 9 to 12 put it at the end of the box's text (not moved while
// Unknown1F0 is set), 152 px - 137 but for the types 5, 9 and 12 - above the box when the IME
// position is fixed, else 17 px below, and the candidate box 100 px right of it. Only the type
// 10 shows the window's caret.
void NCEditBox::PaintCompositionCN(INT X, INT Y, INT TextW, NCGDevice* Device)
{
    if (IMEManager->IsNotComposing())
    {
        ChineseCompositeBox(IMEManager)->HideWindow();
        return;
    }
    SIZE Size;
    INT OffX = 0;
    if (ViewStart < Left.Len())
    {
        for (INT i = 0; i < Comp.Len(); i++)
        {
            NCGDevice::GetTextExtent(&Size, *Comp.Mid(i, 1), 0, 0, 0, 0, 0, L2FONT_Normal);
            BYTE Attr = IMEManager->ChineseIME->CompAttr[i];
            if (Attr == ATTR_TARGET_CONVERTED || Attr == ATTR_INPUT)
                break;
            OffX += Size.cx;
        }
    }
    else
    {
        for (INT i = ViewStart - Left.Len(); i < Comp.Len(); i++)
        {
            NCGDevice::GetTextExtent(&Size, *Comp.Mid(i, 1), 0, 0, 0, 0, 0, L2FONT_Normal);
            BYTE Attr = IMEManager->ChineseIME->CompAttr[i];
            if (Attr == ATTR_TARGET_CONVERTED || Attr == ATTR_INPUT)
                break;
            OffX += Size.cx;
        }
    }
    NCCompositeBox* Box = ChineseCompositeBox(IMEManager);
    if (GConsole->GetIMEType() == 10 || GConsole->GetIMEType() == 9 || GConsole->GetIMEType() == 11 ||
        GConsole->GetIMEType() == 12)
    {
        if (bIMEPosFixed)
        {
            if (GConsole->GetIMEType() == 5 || GConsole->GetIMEType() == 9 || GConsole->GetIMEType() == 12)
                IMECaretY = 152;
            else
                IMECaretY = 137;
        }
        else
            IMECaretY = 17;
        if (!Unknown1F0)
        {
            if (bIMEPosFixed)
                Box->SetWindowRect(X + TextW, -IMECaretY, 100, 15);
            else
                Box->SetWindowRect(X + TextW, IMECaretY, 100, 15);
        }
        Box->ShowWindow();
        if (Unknown1F0)
            IMECaretX = X + TextW + 5;
        else
            IMECaretX = X + TextW + 105;
        NCCandidateBox* Cand = ((NCChineseIME*)IMEManager->ChineseIME)->CandidateBox;
        if (Cand)
            Cand->SetWindowRect(IMECaretX, bIMEPosFixed ? -IMECaretY : IMECaretY, Cand->Width,
                                Cand->Height);
    }
    else if (GConsole->GetIMEType() == 8)
    {
        Box->SetText(*Reading);
        Box->SetWindowRect(X + OffX + TextW + 5, Y - 10, 50, 15);
        if (Reading.Len())
            Box->ShowWindow();
        else
            Box->HideWindow();
    }
    if (GConsole->GetIMEType() == 10)
        Box->SetShowCaret(1);
    else
        Box->SetShowCaret(0);
}
NW_HOOK_METHOD(0x1000be60, NCEditBox, PaintCompositionCN)

NW_CHECK_VTABLE(NCEditBox, 0x1010a418, 78)
