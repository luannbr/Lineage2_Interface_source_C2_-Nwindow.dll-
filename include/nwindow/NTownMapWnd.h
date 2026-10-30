#pragma once
// NTownMapWnd - the map of a town (UClass "TownMapWnd", 0xD0; C2 only; NCConsole makes it): a
// 328 x 329 view into a 512 x 512 picture that the left button drags, with the player's place
// marked by an icon, inside the minimap's frame.

#include "nwindow/NCWnd.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NTownMapWnd : public NCWnd
{
    NW_DECLARE_CLASS(NTownMapWnd, NCWnd, 0, 0x101C4C70)

    UTexture* BackTex;               // +0x98 "L2UI.MinimapWnd.MinimapBack": the frame, drawn last
    UTexture* MapTex;                // +0x9C the town's picture (set from outside) (name ours)
    UTexture* IconTex;               // +0xA0 "town_map.icon_i00": the player's mark
    INT IconX;                       // +0xA4 the mark's place in the picture (set from outside) (name ours)
    INT IconY;                       // +0xA8 (name ours)
    INT MapW;                        // +0xAC the picture's size (512) (name ours)
    INT MapH;                        // +0xB0 (512)
    INT ViewW;                       // +0xB4 the part shown (328 x 329) (name ours)
    INT ViewH;                       // +0xB8
    INT DragX;                       // +0xBC where the drag last was, in the view (name ours)
    INT DragY;                       // +0xC0
    INT ScrollX;                     // +0xC4 the view's corner in the picture (name ours)
    INT ScrollY;                     // +0xC8
    UBOOL bDragging;                 // +0xCC (name ours)

    NTownMapWnd();                                                   // 0x100998a0
    // ~NTownMapWnd                                                  // 0x10099940

    virtual void KillFocus();                                        // 30 0x100991a0 (NCMinimapWnd's body)
    virtual INT OnCreate();                                          // 36 0x10099990
    virtual INT OnDestroy();                                         // 37 0x10098e80
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x10099a60
    virtual INT OnMouseMove(WPARAM wParam, LPARAM lParam);           // 60 0x10099090
    virtual INT OnLButtonDown(WPARAM wParam, LPARAM lParam);         // 61 0x10098fe0
    virtual INT OnLButtonUp(WPARAM wParam, LPARAM lParam);           // 62 0x10099060

    void SetMap(const TCHAR* TexName, INT X, INT Y);                 // 0x10098f00 (name ours) the picture and the mark's place
};
NW_ASSERT_OFFSET(NTownMapWnd, MapTex, 0x9C);
NW_ASSERT_OFFSET(NTownMapWnd, IconX, 0xA4);
NW_ASSERT_OFFSET(NTownMapWnd, MapW, 0xAC);
NW_ASSERT_OFFSET(NTownMapWnd, DragX, 0xBC);
NW_ASSERT_OFFSET(NTownMapWnd, ScrollX, 0xC4);
NW_ASSERT_OFFSET(NTownMapWnd, bDragging, 0xCC);
NW_ASSERT_SIZE(NTownMapWnd, 0xD0);

#pragma pack(pop)
