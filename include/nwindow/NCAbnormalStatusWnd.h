#pragma once
// NCAbnormalStatusWnd - the buffs and debuffs on the player (UClass "CAbnormalStatusWnd",
// 0xD0): up to 31 skill icons, ten per row, behind a vertical title bar on the left. The
// console adds one per server notice (AddStatus) and clears them all (ClearStatus); a one
// second timer counts their time down, and an icon blinks during its last ten seconds. The
// tool tip of the icon under the mouse shows its name, level and description (the skin's full box).
//
// Desktop (NCConsole) +0xC8 is this window.

#include "nwindow/NCWnd.h"
#include "nwindow/NCItemWnd.h"     // NItemEntry: the window's records are built on it
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

// What the console hands to AddStatus (name ours; 0x2310 bytes in C2, 0x1214 in C1: the strings are longer): the
// console builds it on its stack (AddAbnormalStatus 0x10059A00: Name from GetSkillName, IconName from
// GetSkillIconName, Desc from GetSkillDesc; Icon, ToolTip left empty) and the window copies what it needs into its
// own record (NAbnormalEntry).
struct FAbnormalStatus
{
    TCHAR Name[256];                 // +0x0000 the skill's name (C1: 128)
    TCHAR IconName[128];             // +0x0200 texture name
    TCHAR Desc[2048];                // +0x0300 the skill's description (escaped newlines) (C1: 1024)
    TCHAR ToolTip[2048];             // +0x1300 never filled (C1: "name\ndescription")
    UTexture* Icon;                  // +0x2300 NULL
    INT RemainTime;                  // +0x2304 seconds
    INT SkillID;                     // +0x2308
    INT Level;                       // +0x230C

    // inlined in the console's AddAbnormalStatus (Desc is left)
    FAbnormalStatus()
    {
        Name[0] = 0;
        IconName[0] = 0;
        ToolTip[0] = 0;
        Icon = NULL;
    }
};
NW_ASSERT_OFFSET(FAbnormalStatus, IconName, 0x200);
NW_ASSERT_OFFSET(FAbnormalStatus, Desc, 0x300);
NW_ASSERT_OFFSET(FAbnormalStatus, ToolTip, 0x1300);
NW_ASSERT_OFFSET(FAbnormalStatus, Icon, 0x2300);
NW_ASSERT_OFFSET(FAbnormalStatus, SkillID, 0x2308);
NW_ASSERT_SIZE(FAbnormalStatus, 0x2310);

// One buff on the window (name ours; the "heap FAbnormalStatus" of the C2 code, 0x1A30 bytes, allocated one by one: the
// window keeps pointers). An item entry for the skill: Icon (+0x0), Name (+0xC), Description (+0x70C), ID = the skill id
// (+0x1918), UnknownC18 = its level (+0x191C), the tool tip (+0x19B0, filled with NTT_SkillName), and two more fields.
struct NAbnormalEntry : public NItemEntry
{
    INT RemainTime;                  // +0x1A28 seconds (the timer counts it down)
    UBOOL bVisible;                  // +0x1A2C (1) the icon is drawn; flips every second in the last ten seconds

    NAbnormalEntry() : RemainTime(0), bVisible(1) {}   // 0x100D8540 (the base: 0x1001A850)
};
NW_ASSERT_OFFSET(NAbnormalEntry, RemainTime, 0x1A28);
NW_ASSERT_OFFSET(NAbnormalEntry, bVisible, 0x1A2C);
NW_ASSERT_SIZE(NAbnormalEntry, 0x1A30);

enum { TIMER_AbnormalStatus = 0xF9 };

class NCAbnormalStatusWnd : public NCWnd
{
    NW_DECLARE_CLASS(NCAbnormalStatusWnd, NCWnd, 0, 0x101DD3A8)

    TArray<NAbnormalEntry*> Statuses; // +0x98 (C1: TArray<FAbnormalStatus>)
    INT TimerID;                     // +0xA4 SetTimer's result
    UTexture* BackTex;               // +0xA8 "L2UI.EtcWndBack.AbnormalBack" under each icon
    UTexture* Frame1Tex;             // +0xAC "L2UI.EtcWndBack.AbnormalFrame1" left edge for one, two, three rows
    UTexture* Frame2Tex;             // +0xB0 "...AbnormalFrame2"
    UTexture* Frame3Tex;             // +0xB4 "...AbnormalFrame3"
    UTexture* ToolTipTex;            // +0xB8 "L2UI.tooltip.mini-tool01" (not drawn here)
    UTexture* TipTopTex;             // +0xBC "L2UI.Tooltip.Tool02" (C1: the box of the tool tip; C2 draws it with the skin)
    UTexture* TipMidTex;             // +0xC0 "L2UI.Tooltip.Tool01"
    UTexture* TipBottomTex;          // +0xC4 "L2UI.Tooltip.Tool03"
    INT IconSize;                    // +0xC8 constructor argument (24), set to 26 by each tool
                                     //   tip paint; HitTest uses it
    INT HoverIndex;                  // +0xCC icon under the mouse, -1 = none

    NCAbnormalStatusWnd(INT InIconSize = 24);                             // 0x100d8560
    // ~NCAbnormalStatusWnd                                          // 0x100d85f0

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x10043ef0 (folded)
    virtual INT OnTimer(INT InTimerID);                              // 35 0x100d3b90
    virtual INT OnCreate();                                          // 36 0x100d37c0
    virtual INT OnDestroy();                                         // 37 0x100d89b0
    virtual void HideWindow();                                       // 44 0x1007b470 (folded)
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x100d3910
    virtual INT OnDrawToolTip(NCGDevice* Device);                    // 56 0x100d8800 (the db calls it OnPostPaint)
    virtual INT OnMouseMove(WPARAM wParam, LPARAM lParam);           // 60 0x100d3b60

    // names ours
    INT HitTest(INT X, INT Y);                                       // 0x100d2d20
    void UpdateSize();                                               // 0x100d2d70
    INT AddStatus(FAbnormalStatus* Status);                          // 0x100d8650 0 when full
    void ClearStatus();                                              // 0x100d8950
};
NW_ASSERT_OFFSET(NCAbnormalStatusWnd, Statuses, 0x98);
NW_ASSERT_OFFSET(NCAbnormalStatusWnd, TimerID, 0xA4);
NW_ASSERT_OFFSET(NCAbnormalStatusWnd, BackTex, 0xA8);
NW_ASSERT_OFFSET(NCAbnormalStatusWnd, TipTopTex, 0xBC);
NW_ASSERT_OFFSET(NCAbnormalStatusWnd, HoverIndex, 0xCC);
NW_ASSERT_SIZE(NCAbnormalStatusWnd, 0xD0);

#pragma pack(pop)
