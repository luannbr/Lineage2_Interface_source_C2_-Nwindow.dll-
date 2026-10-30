#pragma once
// NCFileLinker - an <a link> of the HTML viewer (UClass "CFileLinker", 0x1A4): an invisible
// control over the text run whose click hands the page to load to the viewer (NWM_COMMAND,
// lParam = File).

#include "nwindow/NControl.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCFileLinker : public NControl
{
    NW_DECLARE_CLASS(NCFileLinker, NControl, 0, 0x101AAF70)

    TCHAR* File;                     // +0x1A4 (name ours) the page, an item's string

    NCFileLinker(TCHAR* InFile = NULL);                                     // 0x1003aba0
    // ~NCFileLinker                                                 // 0x1003ac10

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x10090b60 (shared)
    virtual INT OnLButtonDown(WPARAM wParam, LPARAM lParam);         // 61 0x1003a420 (shared)
};
NW_ASSERT_OFFSET(NCFileLinker, File, 0x1A4);
NW_ASSERT_SIZE(NCFileLinker, 0x1A8);

#pragma pack(pop)