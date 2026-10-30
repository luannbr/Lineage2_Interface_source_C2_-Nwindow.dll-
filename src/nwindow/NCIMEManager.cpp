/*=============================================================================
    NCIMEManager.cpp - the IME of an edit box. Stock: 0x1000dd60 .. 0x1000ded9,
    0x1000f400 .. 0x1000f6e4, 0x100152a0 .. 0x100154e9; WindowProc body shared with NCEditBox.
=============================================================================*/
#include "nwindow/NCIMEManager.h"
#include "nwindow/NCChineseIME.h"
#include "nwindow/NCJapaneseIME.h"
#include "nwindow/NCKoreanIME.h"
#include "nwindow/NCTaiwanIME.h"
#include "nwindow/NCThaiIME.h"
#include "nwindow/NCGDevice.h"

// 0x10012900 - NControl's constructor is inlined in the stock code
NCIMEManager::NCIMEManager(INT InMode)
{
    KoreanIME = NULL;
    JapaneseIME = NULL;
    TaiwanIME = NULL;
    ChineseIME = NULL;
    ThaiIME = NULL;
    Mode = InMode;
}
NW_HOOK_CTOR_ARGS(0x10012900, NCIMEManager, (INT A), (A))

// 0x10012990
NW_HOOK_DTOR(0x10012990, NCIMEManager)

// 0x10010fa0 (vslot 25) - one body with NCEditBox's (hooked there); C2: no enabled test
INT NCIMEManager::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
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

// 0x10018be0 (vslot 36) - Korean for both 0 and 1 (the English client too)
INT NCIMEManager::OnCreate()
{
    switch (GLanguageType)
    {
    case 0:
    case 1:
        KoreanIME = new NCKoreanIME(Mode);
        CreateChild(KoreanIME, NWS_VISIBLE);
        break;
    case 2:
        JapaneseIME = new NCJapaneseIME(Mode);
        JapaneseIME->AttachEntry(AddChildWindow(JapaneseIME, NWS_VISIBLE));
        break;
    case 3:
        TaiwanIME = new NCTaiwanIME(Mode);
        TaiwanIME->AttachEntry(AddChildWindow(TaiwanIME, NWS_VISIBLE));
        break;
    case 4:
        ChineseIME = new NCChineseIME(Mode);
        ChineseIME->AttachEntry(AddChildWindow(ChineseIME, NWS_VISIBLE));
        break;
    case 5:
        ThaiIME = new NCThaiIME(Mode);
        ThaiIME->AttachEntry(AddChildWindow(ThaiIME, NWS_VISIBLE));
        break;
    }
    return 1;
}
NW_HOOK_METHOD(0x10018be0, NCIMEManager, OnCreate)

// 0x100129e0 (vslot 37) - guarded in C2; NControl::OnDestroy is not called
INT NCIMEManager::OnDestroy()
{
    guard(NCIMEManager::OnDestroy);
    switch (GLanguageType)
    {
    case 0:
    case 1:
        if (KoreanIME)
        {
            RemoveChildWindow(KoreanIME->Entry);
            delete KoreanIME;
            KoreanIME = NULL;
        }
        break;
    case 2:
        if (JapaneseIME)
        {
            RemoveChildWindow(JapaneseIME->Entry);
            delete JapaneseIME;
            JapaneseIME = NULL;
        }
        break;
    case 3:
        if (TaiwanIME)
        {
            RemoveChildWindow(TaiwanIME->Entry);
            delete TaiwanIME;
            TaiwanIME = NULL;
        }
        break;
    case 4:
        if (ChineseIME)
        {
            RemoveChildWindow(ChineseIME->Entry);
            delete ChineseIME;
            ChineseIME = NULL;
        }
        break;
    case 5:
        if (ThaiIME)
        {
            RemoveChildWindow(ThaiIME->Entry);
            delete ThaiIME;
            ThaiIME = NULL;
        }
        break;
    }
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x100129e0, NCIMEManager, OnDestroy)

// 0x100111e0 (vslot 68) - the first IME there is, the Thai one last
void NCIMEManager::Activate()
{
    if (KoreanIME)
        KoreanIME->Activate();
    else if (JapaneseIME)
        JapaneseIME->Activate();
    else if (TaiwanIME)
        TaiwanIME->Activate();
    else if (ChineseIME)
        ChineseIME->Activate();
    else if (ThaiIME)
        ThaiIME->Activate();
}
NW_HOOK_METHOD(0x100111e0, NCIMEManager, Activate)

// 0x10011240 (vslot 69)
void NCIMEManager::Deactivate()
{
    if (KoreanIME)
        KoreanIME->Deactivate();
    else if (JapaneseIME)
        JapaneseIME->Deactivate();
    else if (TaiwanIME)
        TaiwanIME->Deactivate();
    else if (ChineseIME)
        ChineseIME->Deactivate();
    else if (ThaiIME)
        ThaiIME->Deactivate();
}
NW_HOOK_METHOD(0x10011240, NCIMEManager, Deactivate)

// 0x100112a0 (vslot 70) - 65..72 pass the IME's messages on to the edit box
INT NCIMEManager::OnComposition(WPARAM wParam, LPARAM lParam)
{
    SendToParent(NIM_COMPOSITION, wParam, lParam);
    return 1;
}
NW_HOOK_METHOD(0x100112a0, NCIMEManager, OnComposition)

// 0x100112c0 (vslot 71)
INT NCIMEManager::OnResult(WPARAM wParam, LPARAM lParam)
{
    SendToParent(NIM_RESULT, wParam, lParam);
    return 1;
}
NW_HOOK_METHOD(0x100112c0, NCIMEManager, OnResult)

// 0x100112e0 (vslot 72)
INT NCIMEManager::OnAttribute(WPARAM wParam, LPARAM lParam)
{
    SendToParent(NIM_ATTRIBUTE, wParam, lParam);
    return 1;
}
NW_HOOK_METHOD(0x100112e0, NCIMEManager, OnAttribute)

// 0x10011300 (vslot 73)
INT NCIMEManager::OnReading(WPARAM wParam, LPARAM lParam)
{
    SendToParent(NIM_READING, wParam, lParam);
    return 1;
}
NW_HOOK_METHOD(0x10011300, NCIMEManager, OnReading)

// 0x10011320 (vslot 75)
INT NCIMEManager::OnGetCaret(WPARAM wParam, LPARAM lParam)
{
    SendToParent(NIM_GETCARET, wParam, lParam);
    return 1;
}
NW_HOOK_METHOD(0x10011320, NCIMEManager, OnGetCaret)

// 0x10011340 (vslot 76)
INT NCIMEManager::OnEndComposition(WPARAM wParam, LPARAM lParam)
{
    SendToParent(NIM_ENDCOMPOSITION, wParam, lParam);
    return 1;
}
NW_HOOK_METHOD(0x10011340, NCIMEManager, OnEndComposition)

// 0x10011360 (vslot 77)
INT NCIMEManager::OnCursorPos(WPARAM wParam, LPARAM lParam)
{
    SendToParent(NIM_CURSORPOS, wParam, lParam);
    return 1;
}
NW_HOOK_METHOD(0x10011360, NCIMEManager, OnCursorPos)

// 0x10012b70 - the getters look at the first of four IMEs (not the Thai one)
TCHAR* NCIMEManager::GetCompStr()
{
    NCIME* IME = GetIME();
    return IME ? IME->GetCompStr() : NULL;
}
NW_HOOK_METHOD(0x10012b70, NCIMEManager, GetCompStr)

// 0x10012bb0
TCHAR* NCIMEManager::GetResultStr()
{
    NCIME* IME = GetIME();
    return IME ? IME->GetResultStr() : NULL;
}
NW_HOOK_METHOD(0x10012bb0, NCIMEManager, GetResultStr)

// 0x10012bf0
TCHAR* NCIMEManager::GetReadingStr()
{
    NCIME* IME = GetIME();
    return IME ? IME->GetReadingStr() : NULL;
}
NW_HOOK_METHOD(0x10012bf0, NCIMEManager, GetReadingStr)

// 0x10012c30 - -1 without an IME; this one knows the Thai IME
UBOOL NCIMEManager::IsNotComposing()
{
    NCIME* IME = KoreanIME;
    if (!IME)
        IME = JapaneseIME;
    if (!IME)
        IME = TaiwanIME;
    if (!IME)
        IME = ChineseIME;
    if (!IME)
        IME = ThaiIME;
    return IME ? IME->IsNotComposing() : -1;
}
NW_HOOK_METHOD(0x10012c30, NCIMEManager, IsNotComposing)

// 0x100110e0 (name ours) - the class of a Thai character; 0 for everything else and outside the Thai client
INT NCIMEManager::GetThaiCharType(UINT Ch)
{
    if (GLanguageType == 5)
    {
        if (Ch == 0x20)
            return 1;
        if ((Ch > 0xE00 && Ch < 0xE2F) || Ch == 0xE2F)
            return 2;
        if (Ch == 0xE33)
            return 3;
        if (Ch == 0xE31 || (Ch > 0xE33 && Ch < 0xE3B) || Ch == 0xE47 || Ch == 0xE4D)
            return 4;
        if (Ch == 0xE30 || Ch == 0xE32 || (Ch > 0xE3F && Ch < 0xE46))
            return 5;
        if (Ch == 0xE3F || Ch == 0xE46)
            return 6;
        if (Ch > 0xE4F && Ch < 0xE5A)
            return 7;
        if (Ch > 0xE47 && Ch < 0xE4C)
            return 8;
        if (Ch == 0xE4C)
            return 9;
    }
    return 0;
}
NW_HOOK_METHOD(0x100110e0, NCIMEManager, GetThaiCharType)

// 0x10017e40 (name ours) - the Thai clusters of Text (GLanguageType 5), else its characters
INT NCIMEManager::GetTextLength(FString Text)
{
    INT Length;
    if (GLanguageType == 5)
    {
        TArray<ThaiCharacter> Chars;
        NCGDevice::MakeThaiCharArray(*Text, Chars, 0);
        Length = Chars.Num();
    }
    else
        Length = Text.Len();
    return Length;
}
NW_HOOK_METHOD(0x10017e40, NCIMEManager, GetTextLength)

NW_CHECK_VTABLE(NCIMEManager, 0x1010ad28, 78)
