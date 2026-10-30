/*=============================================================================
    NwGameGuard.cpp - the two stock globals of the nProtect GameGuard glue, for the standalone build.

    The statically linked nProtect library is not part of the standalone NWindow.dll (include/nwindow/NGameGuard.h);
    these two objects belong to the game's side of the glue, which stays. Hook mode reaches them in the stock image
    (NW_GLOBAL, NConsoleWndWindows.cpp / NConsoleWndInput.cpp); here they are ours, with the stock initial values
    (both .bss, zero).
=============================================================================*/
#include "nwindow/NWindow.h"

#if NW_HOOK_MODE
#error "NwGameGuard.cpp is for the standalone build only"
#endif

class NConsoleWnd;

// 0x101b1c9c .bss (name ours); NConsoleWndWindows.cpp - the console, set at the end of Initialize. The stock reads it
// in the library's GameMon callback (0x10057940, which only the library calls); the standalone has no reader
NConsoleWnd* GGameGuardConsole = NULL;

// 0x101b1ca4 .bss (name ours); NConsoleWndInput.cpp - the seconds since the game guard's last check: Tick adds the
// frame time to it, and the check it would trigger past 10 seconds never runs (the packager's patch jumps over it)
FLOAT GGameGuardTimer = 0.0f;
