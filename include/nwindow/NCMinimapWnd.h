#pragma once
// NCMinimapWnd - the map window (UClass "CMinimapWnd", 0xE0): a 328 x 329 piece of the map
// texture (Aden or Elmore, named in FontInfo.ini) under a numbered 61-pixel grid, the player's
// position and the targets the console marks. Dragging the map scrolls it; the button in the
// corner scrolls back to the player.
//
// Desktop (NCConsole) +0xF4 is the map window. The console marks the same targets on the radar
// (NCRadarWnd) by calling AddTarget / RemoveTarget on it too (same array at +0x94).

#include "nwindow/NCWnd.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCButton;

// A position the console marks on the map and the radar (name ours; world coordinates).
struct FMinimapTarget
{
    INT X;
    INT Y;
    INT Z;
};
NW_ASSERT_SIZE(FMinimapTarget, 0xC);

// map ids the console sets (SetMapID); names ours, after the textures
enum
{
    MINIMAP_Aden = 0x681,
    MINIMAP_Elmore = 0x747,
};

// NWM_COMMAND id of the button that scrolls back to the player
enum { NID_MINIMAP_INITPOS = 0xB000F1 };

class NCMinimapWnd : public NCWnd
{
    NW_DECLARE_CLASS(NCMinimapWnd, NCWnd, 0, 0x101C4220)

    TArray<FMinimapTarget> Targets;  // +0x98
    UTexture* BackTex;               // +0xA4 "L2UI.MinimapWnd.MinimapBack" (frame over the map)
    UTexture* AdenMapTex;            // +0xA8 FontInfo.ini "<country>_AdenMinimap"
    UTexture* ElmoreMapTex;          // +0xAC FontInfo.ini "<country>_ElmoreMinimap"
    UTexture* MyPosTex;              // +0xB0 "L2UI.MinimapWnd.MinimapPos"
    UTexture* TargetPosTex;          // +0xB4 "L2UI.MinimapWnd.TargetPos"
    NCButton* InitPosButton;         // +0xB8   // C2: offset derived, no evidence
    INT MapID;                       // +0xBC MINIMAP_*
    UBOOL bDragging;                 // +0xC0
    INT MyX;                         // +0xC4 the player, relative to the map area (each paint)
    INT MyY;                         // +0xC8
    INT ScrollX;                     // +0xCC map pixel at the map area's corner
    INT ScrollY;                     // +0xD0
    INT DragX;                       // +0xD4 last mouse position, relative to the map area
    INT DragY;                       // +0xD8
    INT UnknownD8[2];                // +0xDC   // C2: offset derived, no evidence

    NCMinimapWnd();                                                  // 0x1009a5f0
    // ~NCMinimapWnd                                                 // 0x1009a690

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x10098a20
    virtual void KillFocus();                                        // 30 0x100991a0
    virtual INT OnCreate();                                          // 36 0x10099300
    virtual INT OnDestroy();                                         // 37 0x100995b0
    virtual void ShowWindow();                                       // 43 0x1008c9c0 (folded)
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x10099ec0
    virtual INT OnMouseMove(WPARAM wParam, LPARAM lParam);           // 60 0x10098b00
    virtual INT OnLButtonDown(WPARAM wParam, LPARAM lParam);         // 61 0x10098a80
    virtual INT OnLButtonUp(WPARAM wParam, LPARAM lParam);           // 62 0x10098ca0
    virtual INT OnInitPos();                                         // 67 0x1009a530 (name ours)

    // names ours; the map id arguments are not read (MapID is)
    void WorldToMap(INT WX, INT WY, INT* OutX, INT* OutY, INT InMapID); // 0x10098cd0
    void SetMapID(INT InMapID);                                      // 0x10098e70
    void GetMyMapPos(INT* OutX, INT* OutY, INT InMapID);             // 0x10099660
    void CenterOnPlayer();                                           // 0x1009a3d0
    void AddTarget(INT X, INT Y, INT Z);                             // 0x10079230 (C1 only: no C2 match)
    void RemoveTarget(INT X, INT Y, INT Z);                          // 0x1009a4d0
    // the stock body (0x1009a540) is NCRadarWnd::ClearTargets's, one for both windows: the console clears the map's
    // marks through it; ours is this copy (NCRadarWnd's is the hooked one)
    __declspec(noinline) void ClearTargets();
};
NW_ASSERT_OFFSET(NCMinimapWnd, Targets, 0x98);
NW_ASSERT_OFFSET(NCMinimapWnd, InitPosButton, 0xB8);
NW_ASSERT_OFFSET(NCMinimapWnd, MapID, 0xBC);
NW_ASSERT_OFFSET(NCMinimapWnd, ScrollX, 0xCC);
NW_ASSERT_OFFSET(NCMinimapWnd, DragY, 0xD8);
NW_ASSERT_SIZE(NCMinimapWnd, 0xE4);

#pragma pack(pop)