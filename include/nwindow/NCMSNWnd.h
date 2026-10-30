#pragma once
// NCMSNWnd - the window of the MSN messenger client that the C2 build carries and nothing
// creates (UClass "CMSNWnd", 0x98, no state of its own; C2 only): only its constructor,
// OnCreate and OnDestroy exist. MSNSocket (MSNSocket.h) is its connection, just as unused.

#include "nwindow/NCWnd.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCMSNWnd : public NCWnd
{
    NW_DECLARE_CLASS(NCMSNWnd, NCWnd, 0, 0x101E20C8)

    NCMSNWnd();                                                      // 0x100dfb90
    // ~NCMSNWnd                                                     // 0x100dfbe0

    virtual INT OnCreate();                                          // 36 0x100c0be0 (folded)
    virtual INT OnDestroy();                                         // 37 0x100dfb80
};
NW_ASSERT_SIZE(NCMSNWnd, 0x98);

#pragma pack(pop)
