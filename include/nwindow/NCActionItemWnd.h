#pragma once
// NCActionItemWnd - a grid of action icons (UClass "CActionItemWnd", 0x154): one of the three
// on the Basic page of the action window (basic, party, social). A click runs the action
// (UseAction: requests to the server, social motions...); an icon dragged onto the shortcut
// bar is registered there. The selected icon's tool tip shows its name and description.

#include "nwindow/NCWnd.h"
#include "nwindow/NCItemWnd.h"   // NItemEntry
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

struct NItemInfo;

// actionname-*.dat ids the actions use (names from the English table; the shortcut bar
// uses them too)
enum
{
    ACTION_SitStand, ACTION_WalkRun, ACTION_Attack, ACTION_Exchange, ACTION_NextTarget,
    ACTION_PickUp, ACTION_Assist, ACTION_Invite, ACTION_LeaveParty, ACTION_Dismiss,
    ACTION_PrivateStoreSell, ACTION_PartyMatching, ACTION_Greeting, ACTION_Victory,
    ACTION_Advance, ACTION_PetStay, ACTION_PetAttack, ACTION_PetStop, ACTION_PetPickUp,
    ACTION_PetReturn, ACTION_PetSkill, ACTION_SummonStay, ACTION_SummonAttack,
    ACTION_SummonStop, ACTION_SocialYes, ACTION_SocialNo, ACTION_SocialBow, ACTION_SummonSkill,
    ACTION_PrivateStoreBuy, ACTION_SocialUnaware, ACTION_SocialWaiting, ACTION_SocialLaugh,
};

// An icon of the grid (name ours; 0x1A30 bytes in C2, created by AddItem): an NItemEntry (the name, the icon, the
// description and the tool tip object are its own) with the action's id and parameter after it. The unused fields stay
// as the default constructor leaves them.
struct NActionItem : public NItemEntry
{
    INT ActionID;              // +0x1A28 (-1) FL2ActionData::ActionID (NItemInfo +0x1718)
    INT Param;                 // +0x1A2C (-1) the social action of the social ones (NItemInfo +0x170C)

    __declspec(noinline) NActionItem(const NItemInfo& Info);         // 0x10043430
    __declspec(noinline) INT GetActionID();                          // 0x10040190
};
NW_ASSERT_OFFSET(NActionItem, ActionID, 0x1A28);
NW_ASSERT_SIZE(NActionItem, 0x1A30);

class NCActionItemWnd : public NCWnd
{
    NW_DECLARE_CLASS(NCActionItemWnd, NCWnd, 0, 0x101AED48)

    TArray<NActionItem*> Items;            // +0x98
    BYTE _UnknownA4[0x38];  // +0xA4 (a C1 pad, its C2 extent)
    UTexture* ItemClickTex;                // +0xDC "l2ui.nwindow.item_click" (not drawn here)
    UTexture* ToolTipTex;                  // +0xE0 "L2UI.tooltip.mini-tool01" (not drawn here)
    UTexture* TipTopTex;                   // +0xE4 "L2UI.Tooltip.Tool02"  tool tip box: top,   // C2: offset derived, no evidence
    UTexture* TipMidTex;                   // +0xE8 "L2UI.Tooltip.Tool01"  a strip per text line,   // C2: offset derived, no evidence
    UTexture* TipBottomTex;                // +0xEC "L2UI.Tooltip.Tool03"  bottom   // C2: offset derived, no evidence
    UTexture* IconBackTex;                 // +0xF0 "l2ui.nwindow.icon_back" frame of each icon
    UTexture* IconClickTex;                // +0xF4 "l2ui.nwindow.icon_click" over the pressed one
    UBOOL bPressed;                        // +0xF8 mouse button down on PressedIndex
    UBOOL bDragging;                       // +0xFC an icon is being dragged
    INT Cols;                              // +0x100
    INT Rows;                              // +0x104
    INT MaxItems;                          // +0x108
    INT IconSize;                          // +0x10C (32)
    INT PressedIndex;                      // +0x110 (-1)
    INT HoverIndex;                        // +0x114 (-1) icon under the mouse
    BYTE _Unknown118[0x40];  // +0x118 (a C1 pad, its C2 extent)

    NCActionItemWnd(INT InCols = 5, INT InRows = 2, INT InMaxItems = 15, INT InIconSize = 32);  // 0x10043500
    // ~NCActionItemWnd                                              // 0x10043600

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x10043ef0 (folded)
    virtual INT OnCreate();                                          // 36 0x100401a0
    virtual INT OnDestroy();                                         // 37 0x10043de0
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x10040960
    virtual INT OnDrawToolTip(NCGDevice* Device);                    // 56 0x10043c50
    virtual INT OnMouseMove(WPARAM wParam, LPARAM lParam);           // 60 0x10040ae0
    virtual INT OnLButtonDown(WPARAM wParam, LPARAM lParam);         // 61 0x10042f50
    virtual INT OnLButtonUp(WPARAM wParam, LPARAM lParam);           // 62 0x10042fd0
    // names ours
    virtual void UseAction();                                        // 67 0x10040c70 the pressed icon's action

    // names ours
    INT HitTest(INT X, INT Y);                                       // 0x100401e0 icon, -1 = none
    void ClearSelection();                                           // 0x10040ca0
    __declspec(noinline) INT AddItem(NItemInfo* Info);               // 0x10043660 0 when full
    void ClearItems();                                               // 0x100d8950 (shared with other grids)
};
NW_ASSERT_OFFSET(NCActionItemWnd, Items, 0x98);
NW_ASSERT_OFFSET(NCActionItemWnd, ItemClickTex, 0xDC);
NW_ASSERT_OFFSET(NCActionItemWnd, bPressed, 0xF8);
NW_ASSERT_OFFSET(NCActionItemWnd, IconSize, 0x10C);
NW_ASSERT_OFFSET(NCActionItemWnd, HoverIndex, 0x114);
NW_ASSERT_SIZE(NCActionItemWnd, 0x158);

#pragma pack(pop)