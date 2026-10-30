#pragma once
// NGameGuard - the game's calls into nProtect GameGuard's client library (NPGameLib, linked statically into the stock
// image at 0x100DFEE0 .. 0x100E7050: 83 functions of a third party, no UI; the three entry points the game calls are the
// NPGameLibApi ones). Name ours.
//
// Hook mode: the library is still the stock code and is called in place (it is the only thing of it that hook mode still
// runs). Standalone: the library is NOT linked (docs/notas/C2_Fase5.md, "nProtect e NKeyCrypt no standalone"), the three
// entry points do nothing, as the library itself while it has not been started (its object pointer 0x101E2E70 is 0:
// SetHwnd and the query return at once; Init answers 0, which StartGameGuard would report as an error, so the standalone
// answers success: there is nothing to start and nothing to report). The client the project reconstructs (the
// binary-patched one) skips the GameGuard start-up and its tick as well, so the server's authentication query
// (SendAuthQuery) is never answered there either.
//
// The calls are cdecl.

#include "nwindow/NWindow.h"

namespace NGameGuard
{

// the library's answer to a good start (the stock keeps it at 0x10129364): the standalone Init answers it, so that
// NConsoleWnd::StartGameGuard goes on to SetHwnd and shows no error
enum { InitSuccess = 0x755 };

#if NW_HOOK_MODE

// NPGameLibApi::InitNPGameMon 0x100DFF20 - the start-up answer (0x755 = success)
FORCEINLINE INT Init()
{
    typedef INT(__cdecl * Fn)();
    return ((Fn)nw::Addr(0x100DFF20))();
}

// NPGameLibApi::SetHwndToGameMon 0x100DFF40 - the game window, after a good start
FORCEINLINE void SetHwnd(INT hWnd)
{
    typedef void(__cdecl * Fn)(INT);
    ((Fn)nw::Addr(0x100DFF40))(hWnd);
}

// NPGameLibApi::SendAuthQuery 0x100E0130 - the server's authentication query (four dwords) to GameMon
FORCEINLINE INT SendAuthQuery(INT* Query)
{
    typedef INT(__cdecl * Fn)(INT*);
    return ((Fn)nw::Addr(0x100E0130))(Query);
}

#else

// no library: documented no-ops (see above)
FORCEINLINE INT Init() { return InitSuccess; }
FORCEINLINE void SetHwnd(INT) {}
FORCEINLINE INT SendAuthQuery(INT*) { return 0; }

#endif

}  // namespace NGameGuard
