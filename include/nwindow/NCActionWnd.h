#pragma once
// NCActionWnd - the action window (UClass "CActionWnd", 0xA8; a page of the character window,
// desktop +0x94 +0x98): tabs Basic (the action icons, NCActionItemListWnd) and Clan
// (NCPledgeWnd). The Clan tab is disabled without a clan; shown with one, the window asks the
// server for the member list.

#include "nwindow/NCWnd.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCActionItemListWnd;
class NCPledgeWnd;
class NCTabControl;

enum { NID_ACTION_TABS = 0xF00A };

class NCActionWnd : public NCWnd
{
    NW_DECLARE_CLASS(NCActionWnd, NCWnd, 0, 0x101AE2F8)

    UTexture* BackTex;                     // +0x98 "sek.cbui304" (loaded, never drawn)
    NCActionItemListWnd* ActionList;       // +0x9C
    NCPledgeWnd* PledgeWnd;                // +0xA0
    NCTabControl* TabCtrl;                 // +0xA4
    NCWnd* UnusedA4;                       // +0xA8 destroyed, never created   // C2: offset derived, no evidence

    NCActionWnd();                                                   // 0x10042b10
    // ~NCActionWnd                                                  // 0x10042b60

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x10043ef0 (folded)
    virtual INT OnCreate();                                          // 36 0x10042bb0
    virtual INT OnDestroy();                                         // 37 0x10042df0 (folded)
    virtual void ShowWindow();                                       // 43 0x10043eb0
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x10076ed0

    // names ours
    void ResetPledgeWnd();                                           // 0x10036870 (C1 only: no C2 match) (the console)
    void ReloadActions();                                            // 0x10036e20 (C1 only: no C2 match) texts reloaded
    void UpdatePledgeTab();                                          // 0x10043b70 (the console)
};
NW_ASSERT_OFFSET(NCActionWnd, BackTex, 0x98);
NW_ASSERT_OFFSET(NCActionWnd, TabCtrl, 0xA4);
NW_ASSERT_SIZE(NCActionWnd, 0xAC);

#pragma pack(pop)