#pragma once
// NCTabControl - row of tab buttons, each showing one page window (UClass "CTabControl",
// 0x1C4). The tabs are NCRadioButtons whose ID is the control's GroupID and whose Value is
// the tab index: a click comes back as NWM_COMMAND(GroupID, index) and selects that page.

#include "nwindow/NControl.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCRadioButton;

#if NW_HOOK_MODE
#define GNextTabGroupID NW_GLOBAL(INT, 0x101587a8)   // starts at 0xFA0000 (name ours)
#else
extern INT GNextTabGroupID;
#endif

class NCTabControl : public NControl
{
    NW_DECLARE_CLASS(NCTabControl, NControl, 0, 0x101A5618)

    INT NextTabX;                    // +0x1A4 X of the next tab button
    TArray<NCRadioButton*> Tabs;     // +0x1A8
    INT GroupID;                     // +0x1B4 ID of the tab buttons (OnCreate)
    NCWnd* CurrentPage;              // +0x1B8
    TArray<NCWnd*> Pages;            // +0x1BC one per tab (not owned)

    NCTabControl();                                                  // 0x10027270
    // ~NCTabControl                                                 // 0x10027310

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x10026960
    virtual INT OnCreate();                                          // 36 0x100269c0
    virtual INT OnDestroy();                                         // 37 0x100270d0
    virtual void ShowCurrentPage();                                  // 68 0x100269e0 (name ours)
    virtual INT SelectTab(INT Index);                                // 69 0x10026a30 (name ours)

    // names ours; a tab of W x H 0 is sized to its label
    INT AddTab(INT TextID, NCWnd* Page, INT W, INT H, const TCHAR* NormalTexName,
               const TCHAR* PushedTexName, INT Gap, INT ToolTipID);  // 0x10022490
    INT AddTab(const TCHAR* Text, NCWnd* Page, INT W, INT H, const TCHAR* NormalTexName,
               const TCHAR* PushedTexName, INT Gap, INT ToolTipID);  // 0x100226d0
    void SetTab(INT Index);                                          // 0x10026b20 silent SelectTab
    TArray<NCRadioButton*> GetTabs();                                // 0x10027380
};
NW_ASSERT_OFFSET(NCTabControl, Tabs, 0x1A8);
NW_ASSERT_OFFSET(NCTabControl, GroupID, 0x1B4);
NW_ASSERT_OFFSET(NCTabControl, Pages, 0x1BC);
NW_ASSERT_SIZE(NCTabControl, 0x1C8);

#pragma pack(pop)