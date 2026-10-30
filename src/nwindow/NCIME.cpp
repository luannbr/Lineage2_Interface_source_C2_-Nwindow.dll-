/*=============================================================================
    NCIME.cpp - base of the language IMEs. Stock: 0x1000dee0 .. 0x1000e22f,
    0x1000e840 .. 0x1000e88f, 0x1000f2b0 .. 0x1000f2ff, 0x1000f6f0 .. 0x1000f758.
=============================================================================*/
#include "nwindow/NCIME.h"
#include "nwindow/NCrt.h"

#include <stdlib.h>

// (GActiveIMC: src/standalone/NwGlobals.cpp in the standalone build)

// 0x10012c70 - NControl's constructor is inlined in the stock code
NCIME::NCIME(INT InMode)
{
    Property = 0;
    OpenStatus = 0;
    Conversion = 0;
    Sentence = 0;
    CodePage = 0;
    bActive = 0;
    Mode = InMode;
    bNotComposing = 1;
}
NW_HOOK_CTOR_ARGS(0x10012c70, NCIME, (INT A), (A))

// 0x10012780
NW_HOOK_DTOR(0x10012780, NCIME)

// 0x10011380 (vslot 25)
INT NCIME::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (NControl::WindowProc(Msg, wParam, lParam))
        return 1;
    if (Msg == WM_IME_NOTIFY)
        return OnImeNotify(wParam, lParam) != 0;
    if (Msg == WM_IME_SETCONTEXT)
        return OnImeSetContext(wParam, lParam) != 0;
    if (Msg == WM_IME_STARTCOMPOSITION)
        return OnImeStartComposition(wParam, lParam) != 0;
    if (Msg == WM_IME_ENDCOMPOSITION)
        return OnImeEndComposition(wParam, lParam) != 0;
    if (Msg == WM_IME_CONTROL)
        return OnImeControl(wParam, lParam) != 0;
    if (Msg == WM_INPUTLANGCHANGE)
        return OnInputLangChange(wParam, lParam) != 0;
    if (Msg == WM_IME_COMPOSITIONFULL)
        return OnImeCompositionFull(wParam, lParam) != 0;
    if (Msg == WM_IME_COMPOSITION)
        return OnImeComposition(wParam, lParam) != 0;
    if (Msg == WM_IME_CHAR)
        return OnImeChar(wParam, lParam) != 0;
    return 0;
}
NW_HOOK_METHOD(0x10011380, NCIME, WindowProc)

// 0x100114e0 (vslot 36) - own input context; the IME's default window stays hidden
INT NCIME::OnCreate()
{
    NControl::OnCreate();
    hIMC = ImmCreateContext();
    HWND IMEWnd = ImmGetDefaultIMEWnd(GViewportHWnd);
    if (IMEWnd)
        ::ShowWindow(IMEWnd, SW_HIDE);
    return 1;
}
NW_HOOK_METHOD(0x100114e0, NCIME, OnCreate)

// 0x100115e0 (vslot 68) - the game window uses this IME's context; C2 also records it for the console
void NCIME::Activate()
{
    bActive = 1;
    if (hIMC)
        ImmAssociateContext(GViewportHWnd, hIMC);
    if (GConsole)
        GActiveIMC = hIMC;
}
NW_HOOK_METHOD(0x100115e0, NCIME, Activate)

// 0x10011620 (vslot 69) - C2: no longer a bare return
void NCIME::Deactivate()
{
    if (GConsole)
        GActiveIMC = NULL;
}
NW_HOOK_METHOD(0x10011620, NCIME, Deactivate)

// 0x10011520 (vslot 71)
INT NCIME::OnImeStartComposition(WPARAM wParam, LPARAM lParam)
{
    bNotComposing = 0;
    return 1;
}
NW_HOOK_METHOD(0x10011520, NCIME, OnImeStartComposition)

// 0x10011540 (vslot 72)
INT NCIME::OnImeEndComposition(WPARAM wParam, LPARAM lParam)
{
    bNotComposing = 1;
    return 1;
}
NW_HOOK_METHOD(0x10011540, NCIME, OnImeEndComposition)

// 0x10011570 (vslot 73) - lets the IME draw its own UI when it has to; an IME that draws at
// the caret gets the context without its composition window
INT NCIME::OnImeSetContext(WPARAM wParam, LPARAM lParam)
{
    if (Property & IME_PROP_SPECIAL_UI)
        ImeIsUIMessage(WM_IME_SETCONTEXT, wParam, lParam);
    else if (Property & IME_PROP_AT_CARET)
        ImeIsUIMessage(WM_IME_SETCONTEXT, wParam, lParam & ~ISC_SHOWUICOMPOSITIONWINDOW);
    return 1;
}
NW_HOOK_METHOD(0x10011570, NCIME, OnImeSetContext)

// 0x10011550 (vslot 75)
INT NCIME::OnInputLangChange(WPARAM wParam, LPARAM lParam)
{
    Property = ImeGetProperty(IGP_PROPERTY);
    return 1;
}
NW_HOOK_METHOD(0x10011550, NCIME, OnInputLangChange)

// 0x100115b0
TCHAR* NCIME::GetResultStr()
{
    return ResultStr;
}
NW_HOOK_METHOD(0x100115b0, NCIME, GetResultStr)

// 0x100115c0
TCHAR* NCIME::GetCompStr()
{
    return CompStr;
}
NW_HOOK_METHOD(0x100115c0, NCIME, GetCompStr)

// 0x100115d0
TCHAR* NCIME::GetReadingStr()
{
    return ReadingStr;
}
NW_HOOK_METHOD(0x100115d0, NCIME, GetReadingStr)

// 0x10011640
UBOOL NCIME::IsNotComposing()
{
    return bNotComposing;
}
NW_HOOK_METHOD(0x10011640, NCIME, IsNotComposing)

// 0x10011650 - NT, or Windows 4.x from 98 on; asked once
UBOOL NCIME::IsUnicodeOS()
{
    if (!GIMEOSChecked)
    {
        OSVERSIONINFOW Info;
        Info.dwOSVersionInfoSize = sizeof(Info);
#pragma warning(suppress : 4996)  // deprecated in current SDKs; the stock code calls it
        GetVersionExW(&Info);
        if (Info.dwPlatformId == VER_PLATFORM_WIN32_NT || Info.dwMajorVersion > 4 ||
            (Info.dwMajorVersion == 4 && Info.dwMinorVersion > 0))
            GIMEUnicodeOS = 1;
        GIMEOSChecked = 1;
    }
    return GIMEUnicodeOS;
}
NW_HOOK_METHOD(0x10011650, NCIME, IsUnicodeOS)

// 0x100116b0 - CodePage becomes the default ANSI code page of the keyboard layout's language
void NCIME::UpdateCodePage()
{
    ANSICHAR Buf[12];
    if (GetLocaleInfoA(LOWORD(GetKeyboardLayout(0)), LOCALE_IDEFAULTANSICODEPAGE, Buf, 10))
        CodePage = NAtol(Buf);
}
NW_HOOK_METHOD(0x100116b0, NCIME, UpdateCodePage)

// 0x10012250 (static) - imm32.dll's four input context locks (the library is not freed)
void NCIME::LoadImmFunctions()
{
    HMODULE Imm = LoadLibraryA("imm32.dll");
    if (Imm)
    {
        GImmLockIMC = (NImmLockIMC)GetProcAddress(Imm, "ImmLockIMC");
        GImmUnlockIMC = (NImmUnlockIMC)GetProcAddress(Imm, "ImmUnlockIMC");
        GImmLockIMCC = (NImmLockIMCC)GetProcAddress(Imm, "ImmLockIMCC");
        GImmUnlockIMCC = (NImmUnlockIMCC)GetProcAddress(Imm, "ImmUnlockIMCC");
    }
}
NW_HOOK_FUNC(0x10012250, NCIME::LoadImmFunctions)

NW_CHECK_VTABLE(NCIME, 0x1010abe8, 79)
