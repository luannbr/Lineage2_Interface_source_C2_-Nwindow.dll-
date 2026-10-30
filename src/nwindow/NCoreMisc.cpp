/*=============================================================================
    NCoreMisc.cpp - the small pieces of the engine's headers that the stock NWindow compiled out of line (one copy
    each, called from the windows): appSeconds, appMemcpy, the std algorithm instance that fills with a value, the
    copy and destructor of a system message line, and the constructor of an array of textures. Names are UE2's or the
    library's; the instances are the C2 build's.
=============================================================================*/
#include "nwindow/NConsoleWnd.h"
#include <intrin.h>
#include <string.h>

// 0x10019a10 - UE2's appSeconds (UnVcWin32.h): the time stamp counter in seconds, plus 2^24 "to make bugs apparent where
// the value is passed to a FLOAT". The pawn viewer's OnStart calls this copy; the other users expand it inline (the
// stock's out-of-line copy at 0x10016620 belongs to the Core build's)
static DOUBLE NwAppSeconds()
{
    unsigned __int64 Cycles = __rdtsc();
    DWORD L = (DWORD)Cycles;
    DWORD H = (DWORD)(Cycles >> 32);
    return ((DOUBLE)H * 4294967296.0 + (DOUBLE)L) * GSecondsPerCycle + 16777216.0;
}
NW_HOOK_FUNC(0x10019a10, NwAppSeconds)

// 0x10001790 - UE2's appMemcpy (UnVcWin32.h), the out-of-line copy: the stock chooses an SSE or a string-move path by
// the size and the alignment (a jump through a sled of movsb for the head and the tail); the result is a plain
// non-overlapping copy
NW_HOOK_FUNC(0x10001790, appMemcpy)

// 0x1000fea0 - std::_Ufill for the 4-byte elements (uninitialized_fill_n: the value is stored in every place that
// is not NULL); cdecl (destination, count, the value by reference)
static void __cdecl NwUfillDword(DWORD* First, UINT Count, const DWORD* Value)
{
    for (; Count > 0; --Count, ++First)
        if (First)
            *First = *Value;
}
NW_HOOK_FUNC(0x1000fea0, NwUfillDword)

// 0x10001990 - the global operator new, out of line: GMalloc->Malloc(Size, "new") (cdecl); the stock code calls it for the
// objects it makes itself (the nProtect library's among them)
static void* __cdecl NwOperatorNew(size_t Size)
{
    return operator new(Size);
}
NW_HOOK_FUNC(0x10001990, NwOperatorNew)

// 0x10055ee0 - the copy of a system message line (the console copies the table's entry before it uses it)
NW_HOOK_CTOR_ARGS(0x10055ee0, FL2SystemMsgData, (const FL2SystemMsgData& A), (A))
// 0x10055f50 - its two texts
NW_HOOK_DTOR(0x10055f50, FL2SystemMsgData)

// 0x10019e00 - the empty array of textures (the constructor of GCoolTimeTextures and GCoolTimeEndTextures, which the
// stock's static initializers call: they have run before the hooks are installed)
typedef TArray<UTexture*> TTextureArray;
NW_HOOK_CTOR(0x10019e00, TTextureArray)
