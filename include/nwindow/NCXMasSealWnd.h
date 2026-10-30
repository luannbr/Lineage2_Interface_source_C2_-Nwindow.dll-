#pragma once
// NCXMasSealWnd - a small window of the Christmas event (UClass "CXMasSealWnd", 0xA4; C2 only,
// NCConsole::OnCreate makes it): a "MinimapBack" panel with a title bar, and a second picture
// drawn over it that nothing in this class sets (the console fills it).

#include "nwindow/NCWnd.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCXMasSealWnd : public NCWnd
{
    NW_DECLARE_CLASS(NCXMasSealWnd, NCWnd, 0, 0x101E1BA0)

    INT Unknown98;                   // +0x98 (0) cleared by the constructor, never read here
    UTexture* BackTex;               // +0x9C "L2UI.MinimapWnd.MinimapBack"
    UTexture* SealTex;               // +0xA0 (name ours) drawn at (3, 22) over the back; set from outside

    NCXMasSealWnd();                                                 // 0x100df910
    // ~NCXMasSealWnd                                                // 0x100df960

    virtual INT OnCreate();                                          // 36 0x100df9b0
    virtual INT OnDestroy();                                         // 37 0x100df890
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x100dfa70

    void SetItem(INT ItemID);                                        // 0x100dfb40 (name ours) the seal's item: its picture
};
NW_ASSERT_OFFSET(NCXMasSealWnd, BackTex, 0x9C);
NW_ASSERT_OFFSET(NCXMasSealWnd, SealTex, 0xA0);
NW_ASSERT_SIZE(NCXMasSealWnd, 0xA4);

#pragma pack(pop)
