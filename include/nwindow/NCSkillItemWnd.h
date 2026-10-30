#pragma once
// NCSkillItemWnd - a skill grid of the skill window (UClass "CSkillItemWnd", 0x134; the Active
// and Passive lists of NCSkillWnd, 6 x 7 icons of 32 pixels). Each skill has a cool time icon
// drawn over it (CoolTimes, one per item, same index); the active ones get the icon frame and,
// while pressed, the click frame, and a toggle skill whose effect is on the player the toggle
// animation. Released over the same icon a skill is used (Ctrl forces, Shift stays in place);
// dropped on a shortcut bar slot it is registered there. The tool tip is always the box: name
// and type, "MP cost" and "Range" when the skill has them, description.
// NCGMSkillItemWnd, the GM view of a player's skills, uses nothing on release.

#include "nwindow/NCItemWnd.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCCoolTimeIcon;

class NCSkillItemWnd : public NCItemWnd
{
    NW_DECLARE_CLASS(NCSkillItemWnd, NCItemWnd, 0, 0x101BFF30)

    UTexture* IconBackTex;             // +0x110 "l2ui.nwindow.icon_back" frame of an active skill
    UTexture* IconClickTex;            // +0x114 "l2ui.nwindow.icon_click" while pressed
    UTexture* ToggleTex;               // +0x118 "L2_SkillTime.ToggleEffect.ToggleEffect001"
    UBOOL bPressed;                    // +0x11C the left button is down on the grid
    TArray<NCCoolTimeIcon*> CoolTimes; // +0x120 one per item (the console starts them)

    NCSkillItemWnd();                                                // 0x10091d40
    NCSkillItemWnd(INT InColumns, INT InVisibleRows, INT InMaxItems, INT InCellSize); // 0x10091de0
    // ~NCSkillItemWnd                                               // 0x10078360

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x10090b10
    virtual INT OnDestroy();                                         // 37 0x10091b50
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x10090bb0
    virtual INT OnDrawToolTip(NCGDevice* Device);                    // 56 0x10092180
    virtual INT OnMouseMove(WPARAM wParam, LPARAM lParam);           // 60 0x10090ee0
    virtual INT OnLButtonDown(WPARAM wParam, LPARAM lParam);         // 61 0x10090b30
    virtual INT OnLButtonUp(WPARAM wParam, LPARAM lParam);           // 62 0x100910e0
    virtual void Clear();                                            // 70 0x10091e90

    // names ours
    INT AddSkill(NItemInfo* Info);                                   // 0x10091c20 no limit
};
NW_ASSERT_OFFSET(NCSkillItemWnd, IconBackTex, 0x110);
NW_ASSERT_OFFSET(NCSkillItemWnd, bPressed, 0x11C);
NW_ASSERT_OFFSET(NCSkillItemWnd, CoolTimes, 0x120);
NW_ASSERT_SIZE(NCSkillItemWnd, 0x12C);

// NCGMSkillItemWnd - the grids of NCGMSkillWnd (UClass "CGMSkillItemWnd", 0x134): a click
// only selects.
class NCGMSkillItemWnd : public NCSkillItemWnd
{
    NW_DECLARE_CLASS(NCGMSkillItemWnd, NCSkillItemWnd, 0, 0x101B5028)

    NCGMSkillItemWnd();                                              // 0x100788f0
    NCGMSkillItemWnd(INT InColumns, INT InVisibleRows, INT InMaxItems, INT InCellSize); // 0x10078990
    // ~NCGMSkillItemWnd                                             // 0x10078940

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x10073040
    virtual INT OnLButtonUp(WPARAM wParam, LPARAM lParam);           // 62 0x10072f90 (folded)
};
NW_ASSERT_SIZE(NCGMSkillItemWnd, 0x12C);

#pragma pack(pop)