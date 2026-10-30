/*=============================================================================
    NCMSNWnd.cpp - the unused MSN messenger window (C2 only). Stock: 0x100dfb80 .. 0x100dfc2f.
=============================================================================*/
#include "nwindow/NCMSNWnd.h"

// 0x100dfb90
NCMSNWnd::NCMSNWnd()
{
}
NW_HOOK_CTOR(0x100dfb90, NCMSNWnd)

// 0x100dfbe0
NW_HOOK_DTOR(0x100dfbe0, NCMSNWnd)

// 0x100c0be0 (vslot 36) - the stock body is one with NCRadarWnd's (hooked there)
INT NCMSNWnd::OnCreate()
{
    NCWnd::OnCreate();
    return 1;
}

// 0x100dfb80 (vslot 37) - not guarded
INT NCMSNWnd::OnDestroy()
{
    NCWnd::OnDestroy();
    return 1;
}
NW_HOOK_METHOD(0x100dfb80, NCMSNWnd, OnDestroy)

NW_CHECK_VTABLE(NCMSNWnd, 0x101291a0, 67)
