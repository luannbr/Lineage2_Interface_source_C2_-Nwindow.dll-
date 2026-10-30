#pragma once
// NCGMSnoopManager - the GM's snoop windows (UClass "CGMSnoopManager", 0xA8; C2 only;
// NCConsole makes it): four NCGMSnoopWnd, one per conversation listened to, all hidden until
// the console shows one of them.

#include "nwindow/NCWnd.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCGMSnoopWnd;

class NCGMSnoopManager : public NCWnd
{
    NW_DECLARE_CLASS(NCGMSnoopManager, NCWnd, 0, 0x101D1530)

    NCGMSnoopWnd* Snoops[4];         // +0x98 (name ours)

    NCGMSnoopManager();                                              // 0x1004c6a0
    // ~NCGMSnoopManager                                             // 0x1004c6f0

    virtual INT OnCreate();                                          // 36 0x100b7370
    virtual INT OnDestroy();                                         // 37 0x100b7430
    virtual void ShowWindow();                                       // 43 0x100b6950

    // names ours (the console's Snoop calls them)
    void AddLine(INT SnoopID, const TCHAR* Text, DWORD Color, INT Type, INT Unused); // 0x100b74f0
    void OpenSnoop(INT SnoopID, const TCHAR* TargetName);            // 0x100b7550
};
NW_ASSERT_OFFSET(NCGMSnoopManager, Snoops, 0x98);
NW_ASSERT_SIZE(NCGMSnoopManager, 0xA8);

#pragma pack(pop)
