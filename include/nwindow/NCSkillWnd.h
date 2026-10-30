#pragma once
// NCSkillWnd - the skill list (UClass "CSkillWnd", 0x1AC; the page of NCMagicSkillWnd): tabs
// Active and Passive (sysstring 120 / 121), each an NCSkillItemWnd grid of 6 x 7 icons (80 at
// most). The console fills them from the server's skill list (AddSkill).

#include "nwindow/NControl.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCSkillItemWnd;
class NCGMSkillItemWnd;
class NCTabControl;
struct NItemInfo;

// NWM_COMMAND ids of the tab controls
enum { NID_SKILL_TABS = 0x0AF11000, NID_GMSKILL_TABS = 0xFFE2 };

class NCSkillWnd : public NControl
{
    NW_DECLARE_CLASS(NCSkillWnd, NControl, 0, 0x101C0458)

    NCSkillItemWnd* ActiveList;            // +0x1A4
    NCSkillItemWnd* PassiveList;           // +0x1A8
    NCTabControl* TabCtrl;                 // +0x1AC

    NCSkillWnd();                                                    // 0x10091460
    // ~NCSkillWnd                                                   // 0x100914d0

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x10090b60 (folded)
    virtual INT OnCreate();                                          // 36 0x10091f40
    virtual INT OnDestroy();                                         // 37 0x10091520
    virtual void ShowWindow();                                       // 43 0x10090b80
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x10076ed0 (folded)

    // names ours
    INT AddSkill(NItemInfo* Info);                                   // 0x10073080 the console
    void ClearLists();                                               // 0x10072980 (C1 only: no C2 match)
};
NW_ASSERT_OFFSET(NCSkillWnd, ActiveList, 0x1A4);
NW_ASSERT_OFFSET(NCSkillWnd, TabCtrl, 0x1AC);
NW_ASSERT_SIZE(NCSkillWnd, 0x1B0);

// NCGMSkillWnd - the GM's view of a player's skills (UClass "CGMSkillWnd", 0x1AC; the page of
// NCGMMagicSkillWnd): NCSkillWnd again, a class of its own under NControl with NCGMSkillItemWnd
// grids. Its other methods are copies of NCSkillWnd's that the linker folded into one body
// each (the console's AddSkill calls land on NCSkillWnd's hooks).
class NCGMSkillWnd : public NControl
{
    NW_DECLARE_CLASS(NCGMSkillWnd, NControl, 0, 0x101B6F18)

    NCGMSkillItemWnd* ActiveList;          // +0x1A4
    NCGMSkillItemWnd* PassiveList;         // +0x1A8
    NCTabControl* TabCtrl;                 // +0x1AC

    NCGMSkillWnd();                                                  // 0x10074520
    // ~NCGMSkillWnd                                                 // 0x10074590

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x10090b60 (folded)
    virtual INT OnCreate();                                          // 36 0x100789c0
    virtual INT OnDestroy();                                         // 37 0x100745e0 (folded)
    virtual void ShowWindow();                                       // 43 0x10090b80 (folded)
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x10076ed0 (folded)

    // names ours
    INT AddSkill(NItemInfo* Info);                                   // 0x10073080 (folded)
    void ClearLists();                                               // 0x10072980 (C1 only: no C2 match) (folded)
};
NW_ASSERT_OFFSET(NCGMSkillWnd, ActiveList, 0x1A4);
NW_ASSERT_OFFSET(NCGMSkillWnd, TabCtrl, 0x1AC);
NW_ASSERT_SIZE(NCGMSkillWnd, 0x1B0);

#pragma pack(pop)