#pragma once
// NCMainWnd - the character window (UClass "CMainWnd", 0xAC; Alt+T, Alt+K, Alt+C, Alt+U): four
// pages under a row of tab buttons - status (NCDetailStatusWnd), skills (NCMagicSkillWnd),
// actions (NCActionWnd) and quests (NCQuestWnd) - and the title of the page shown. Minimized,
// its icon is the page's.
//
// Desktop (NCConsole) +0x94 is this window.

#include "nwindow/NCWnd.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCDetailStatusWnd;
class NCActionWnd;
class NCTabControl;

// NWM_COMMAND ids (name ours)
enum
{
    NID_MAIN_FRAME = 0xF2A6,
    NID_MAIN_TABS = 0xF2A7,           // lParam: the tab clicked
};

// the pages, in tab order (names ours)
enum
{
    MAINTAB_Status = 0,
    MAINTAB_Skills = 1,
    MAINTAB_Actions = 2,
    MAINTAB_Quests = 3,
};

class NCMainWnd : public NCWnd
{
    NW_DECLARE_CLASS(NCMainWnd, NCWnd, 0, 0x101B0D20)

    NCDetailStatusWnd* DetailWnd;    // +0x98 tab 0
    NCActionWnd* ActionWnd;          // +0x9C tab 2
    NCWnd* SkillWnd;                 // +0xA0 tab 1 NCMagicSkillWnd
    NCWnd* QuestWnd;                 // +0xA4 tab 3 NCQuestWnd
    NCTabControl* TabCtrl;           // +0xA8
    UTexture* BackTex;               // +0xAC "L2UI.NWindow.mainwndback" (behind the tabs)

    NCMainWnd();                                                     // 0x1004c7e0
    // ~NCMainWnd                                                    // 0x1004c830

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x1004c4f0
    virtual INT OnCreate();                                          // 36 0x1004fe90
    virtual INT OnDestroy();                                         // 37 0x100502e0
    virtual void ShowWindow();                                       // 43 0x10050590
    virtual void Minimize();                                         // 48 0x100506a0
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x10050610
    virtual INT OnTabChanged(INT Tab);                               // 67 0x1004c550 (name ours)

    // names ours
    void ShowTab(INT Tab);                                           // 0x10050430 (shortcut keys)
};
NW_ASSERT_OFFSET(NCMainWnd, DetailWnd, 0x98);
NW_ASSERT_OFFSET(NCMainWnd, TabCtrl, 0xA8);
NW_ASSERT_SIZE(NCMainWnd, 0xB0);

#pragma pack(pop)