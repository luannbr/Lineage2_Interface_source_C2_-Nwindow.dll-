#pragma once
// NCActionItemListWnd - the Basic page of the action window (UClass "CActionItemListWnd",
// 0xA4): three icon grids, filled from actionname-*.dat by category (1 basic, 2 party,
// 3 social), under the labels "Party" and "Social".

#include "nwindow/NCWnd.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCActionItemWnd;

class NCActionItemListWnd : public NCWnd
{
    NW_DECLARE_CLASS(NCActionItemListWnd, NCWnd, 0, 0x101AF268)

    NCActionItemWnd* BasicGrid;            // +0x98 6 x 2   // C2: offset derived, no evidence
    NCActionItemWnd* PartyGrid;            // +0x9C 6 x 1   // C2: offset derived, no evidence
    NCActionItemWnd* SocialGrid;           // +0xA0 6 x 3   // C2: offset derived, no evidence
    UTexture* BackTex;                     // +0xA4 "L2UI.ActionWnd.ActionBack"

    NCActionItemListWnd();                                           // 0x10040cd0
    // ~NCActionItemListWnd                                          // 0x10040d20

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x10043ef0 (folded)
    virtual INT OnCreate();                                          // 36 0x10043720
    virtual INT OnDestroy();                                         // 37 0x10040e70
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x10040d70

    void LoadActions();                                              // 0x10043910 (name ours; stock)
    void ClearGrids();                                               // 0x10043d40 (name ours)
};
NW_ASSERT_OFFSET(NCActionItemListWnd, BasicGrid, 0x98);
NW_ASSERT_OFFSET(NCActionItemListWnd, BackTex, 0xA4);
NW_ASSERT_SIZE(NCActionItemListWnd, 0xA8);

#pragma pack(pop)