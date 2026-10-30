#pragma once
// NCPartyWnd - the party (UClass "CPartyWnd", 0xD0): a column of eight NCPartyStatusWnd, one
// per member, under a vertical title bar on the left. The console hands it the server's party
// packets (AddMember, UpdateMember, DeleteMember, DeleteAll); the window grows and shrinks with
// the members shown.
//
// Desktop (NCConsole) +0xB0 is the party window; the console's vslots 130..133 (add, update,
// delete, delete all) forward the party packets here.

#include "nwindow/NCWnd.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCPartyStatusWnd;

enum { NUM_PARTY_SLOTS = 8 };

class NCPartyWnd : public NCWnd
{
    NW_DECLARE_CLASS(NCPartyWnd, NCWnd, 0, 0x101DC950)

    INT Unknown94;                   // +0x98   // C2: offset derived, no evidence
    UTexture* FrameTopTex;           // +0x9C left edge, shown while there are members
    UTexture* FrameCenterTex;        // +0xA0
    UTexture* FrameBottomTex;        // +0xA4
    INT NumMembers;                  // +0xA8
    UBOOL bShown;                    // +0xAC members listed (0: only the title bar)
    NCPartyStatusWnd* Members[NUM_PARTY_SLOTS]; // +0xB0 filled from the top, empty = no name
    INT UnknownCC;                   // +0xD0   // C2: offset derived, no evidence

    NCPartyWnd();                                                    // 0x100d6230
    // ~NCPartyWnd                                                   // 0x100d6280

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x100d3500
    virtual INT OnCreate();                                          // 36 0x100d62d0
    virtual INT OnDestroy();                                         // 37 0x100d6450
    virtual void ShowWindow();                                       // 43 0x100d6740
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x100d6500
    virtual INT OnToggle();                                          // 67 0x100d8470 (name ours)

    // names ours
    void UpdateMember(INT MemberID, const TCHAR* Name, INT HP, INT MaxHP, INT MP,
                      INT MaxMP);                                    // 0x100aea20 found by name
    void UpdateLayout();                                             // 0x100d65e0
    void AddMember(INT MemberID, const TCHAR* Name, INT HP, INT MaxHP, INT MP,
                   INT MaxMP);                                       // 0x100b34f0
    void DeleteMember(INT MemberID, const TCHAR* Name);              // 0x100d8330 by name, the
                                                                     //   id is not read
    void DeleteAll();                                                // 0x100d8410
};
NW_ASSERT_OFFSET(NCPartyWnd, FrameTopTex, 0x9C);
NW_ASSERT_OFFSET(NCPartyWnd, NumMembers, 0xA8);
NW_ASSERT_OFFSET(NCPartyWnd, Members, 0xB0);
NW_ASSERT_SIZE(NCPartyWnd, 0xD4);

#pragma pack(pop)