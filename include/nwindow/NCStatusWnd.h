#pragma once
// NCStatusWnd - base of the HP / MP windows (UClass "CStatusWnd", 0x2FC): the player's
// (NCPlayerStatusWnd: bars, level, exp), the target's (NCTargetStatusWnd: name colored by the
// level difference, HP / MP of monsters and party members, clan and alliance) and one per
// party member (NCPartyStatusWnd, inside NCPartyWnd), the pet's (NCPetStatusWnd) and the
// summon's (NCSummonedStatusWnd). The base only keeps the textures and numbers the subclasses
// draw.
//
// Desktop (NCConsole) +0xC0 is the target window, +0xC4 the player window, +0xB4 the pet
// window, +0xB8 the summon window.

#include "nwindow/NCWnd.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCButton;
class NCResizeFrame;
class NCProgressCtrl;

class NCStatusWnd : public NCWnd
{
    NW_DECLARE_CLASS(NCStatusWnd, NCWnd, 0, 0x101DAF98)

    UTexture* MPTex;                 // +0x98
    UTexture* HPTex;                 // +0x9C
    UTexture* EXPTex;                // +0xA0
    UTexture* BackTex;               // +0xA4 "NWindow.WhiteTexture" (OnCreate), the subclasses
                                     //   load their own
    UTexture* BackTopTex;            // +0xA8 target window: background (top, center, bottom)   // C2: offset derived, no evidence
    UTexture* BackCenterTex;         // +0xAC   // C2: offset derived, no evidence
    UTexture* BackBottomTex;         // +0xB0   // C2: offset derived, no evidence
    UTexture* FrameTopTex;           // +0xB4 target window: left edge (top, center, bottom)   // C2: offset derived, no evidence
    UTexture* FrameCenterTex;        // +0xB8   // C2: offset derived, no evidence
    UTexture* FrameBottomTex;        // +0xBC   // C2: offset derived, no evidence
    UTexture* UpBtnTex;              // +0xC0 target window: loaded, never drawn (the toggle   // C2: offset derived, no evidence
    UTexture* UpBtnClickTex;         // +0xC4   button loads its own)   // C2: offset derived, no evidence
    UTexture* DownBtnTex;            // +0xC8   // C2: offset derived, no evidence
    UTexture* DownBtnClickTex;       // +0xCC   // C2: offset derived, no evidence
    UTexture* LeftBackTex;           // +0xD0 player window: edges
    UTexture* RightBackTex;          // +0xD4
    UTexture* MPBackTex;             // +0xD8 empty bars
    UTexture* HPBackTex;             // +0xDC
    INT HP;                          // +0xE0 party member (SetInfo)
    INT MaxHP;                       // +0xE4
    INT MP;                          // +0xE8
    INT MaxMP;                       // +0xEC
    INT ObjectID;                    // +0xF0 target window: whom a click targets / attacks
    INT BarWidth;                    // +0xF4 constructor argument (the target window sets 132)
    INT MaxExtraWidth;               // +0xF8 player window: how far it can be widened (202)
    INT ExtraWidth;                  // +0xFC player window: widened by this (bars grow with it)
    TCHAR Name[256];                 // +0x100 party member, empty = free slot

    NCStatusWnd(INT InBarWidth = 100);                                     // 0x100d3c10
    // ~NCStatusWnd                                                  // 0x100d3740

    virtual __declspec(noinline) INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam); // 25 0x10043ef0 (folded; the
                                                                     //   derived WindowProcs call it, so out of line)
    virtual __declspec(noinline) INT OnCreate();                     // 36 0x100d3c50 (called by the derived OnCreates: out of line)
    virtual INT OnDestroy();                                         // 37 0x100d2df0 (folded)
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x100d3c80
    virtual INT OnLButtonDown(WPARAM wParam, LPARAM lParam) { return 1; } // 61 0x100d2e70 (folded)

    // names ours
    UBOOL IsName(const TCHAR* InName);                               // 0x100d2e80
    void Clear();                                                    // 0x100d2eb0
    UBOOL IsEmpty();                                                 // 0x100d34f0
};
NW_ASSERT_OFFSET(NCStatusWnd, MPTex, 0x98);
NW_ASSERT_OFFSET(NCStatusWnd, LeftBackTex, 0xD0);
NW_ASSERT_OFFSET(NCStatusWnd, HP, 0xE0);
NW_ASSERT_OFFSET(NCStatusWnd, ObjectID, 0xF0);
NW_ASSERT_OFFSET(NCStatusWnd, ExtraWidth, 0xFC);
NW_ASSERT_OFFSET(NCStatusWnd, Name, 0x100);
NW_ASSERT_SIZE(NCStatusWnd, 0x300);

// NWM_COMMAND ids of the status windows' controls
enum
{
    NID_PLAYERSTATUS_RESIZE = 0xF20F04,  // right edge of the player window
    NID_TARGETSTATUS_CLOSE = 0xFA,
    NID_TARGETSTATUS_TOGGLE = 0xFB,      // shows / hides the clan and alliance lines
};

// NCPlayerStatusWnd - the player's name, level, HP / MP / exp bars (UClass "CPlayerStatusWnd",
// 0x308). The right edge widens it (the bars grow with it); a click targets the player.
class NCPlayerStatusWnd : public NCStatusWnd
{
    NW_DECLARE_CLASS(NCPlayerStatusWnd, NCStatusWnd, 0, 0x101DBF08)

    User* Player;                    // +0x300 set by each paint
    INT Unknown300;                  // +0x304 (0)
    NCResizeFrame* ResizeFrame;      // +0x308

    NCPlayerStatusWnd(INT InBarWidth = 100);                               // 0x100d4990
    // ~NCPlayerStatusWnd                                            // 0x100d49f0

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x100d3110
    virtual INT OnCreate();                                          // 36 0x100d4a40
    virtual INT OnDestroy();                                         // 37 0x100d5380
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x100d4c00
    virtual INT OnLButtonDown(WPARAM wParam, LPARAM lParam);         // 61 0x100d3180
    // names ours
    virtual INT OnResize(INT Delta);                                 // 67 0x100d5430

    INT Resize(INT Delta);                                           // 0x100d3170 (name ours)
};
NW_ASSERT_OFFSET(NCPlayerStatusWnd, Player, 0x300);
NW_ASSERT_OFFSET(NCPlayerStatusWnd, ResizeFrame, 0x308);
NW_ASSERT_SIZE(NCPlayerStatusWnd, 0x30C);

// NCTargetStatusWnd - the target (UClass "CTargetStatusWnd", 0x310): its name, HP / MP bars
// for monsters, siege NPCs, the player, its pet and party members; a door or other static
// object shows its name only. For players the window can be expanded to show the clan and
// the alliance. A click targets again (Ctrl: attacks); the close button cancels the target.
class NCTargetStatusWnd : public NCStatusWnd
{
    NW_DECLARE_CLASS(NCTargetStatusWnd, NCStatusWnd, 0, 0x101DC428)

    NCButton* CloseButton;           // +0x300   // C2: offset derived, no evidence
    INT Unknown300;                  // +0x304   // C2: offset derived, no evidence
    INT LevelDiff;                   // +0x308 the player's level minus the target's (name color)
    UBOOL bExpanded;                 // +0x30C clan and alliance lines shown
    NCButton* ToggleButton;          // +0x310

    NCTargetStatusWnd(INT InBarWidth = 100);                               // 0x100d3d30
    // ~NCTargetStatusWnd                                            // 0x100d3d90

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x100d2ee0
    virtual INT OnCreate();                                          // 36 0x100d3de0
    virtual INT OnDestroy();                                         // 37 0x100d40e0
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x100d4190
    virtual INT OnLButtonDown(WPARAM wParam, LPARAM lParam);         // 61 0x100d2f60
    // names ours
    virtual INT OnClose();                                           // 67 0x100d30a0
    virtual INT OnToggle();                                          // 68 0x100d3000

    void SetLevelDiff(INT Diff);                                     // 0x100d2ff0 (name ours)

private:
    void Collapse();                 // inlined in the stock OnPaint (name ours)
};
NW_ASSERT_OFFSET(NCTargetStatusWnd, CloseButton, 0x300);
NW_ASSERT_OFFSET(NCTargetStatusWnd, LevelDiff, 0x308);
NW_ASSERT_OFFSET(NCTargetStatusWnd, ToggleButton, 0x310);
NW_ASSERT_SIZE(NCTargetStatusWnd, 0x314);

// NCPartyStatusWnd - one party member (UClass "CPartyStatusWnd", 0x300): name, HP / MP bars.
// A click targets the member (Ctrl: attacks), a right click targets what the member targets.
class NCPartyStatusWnd : public NCStatusWnd
{
    NW_DECLARE_CLASS(NCPartyStatusWnd, NCStatusWnd, 0, 0x101DB4C0)

    INT MemberID;                    // +0x300 object id of the member (-1)

    NCPartyStatusWnd(INT InBarWidth = 100);                                // 0x100d5ef0
    // ~NCPartyStatusWnd                                             // 0x100d5f50

    virtual INT OnCreate();                                          // 36 0x100d5fa0
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x100d6010
    virtual INT OnLButtonDown(WPARAM wParam, LPARAM lParam);         // 61 0x100d3370
    virtual INT OnRButtonUp(WPARAM wParam, LPARAM lParam);           // 64 0x100d3400

    void SetInfo(INT InMemberID, const TCHAR* InName, INT InHP, INT InMaxHP, INT InMP,
                 INT InMaxMP);                                       // 0x100d3490 (name ours)
};
NW_ASSERT_OFFSET(NCPartyStatusWnd, MemberID, 0x300);
NW_ASSERT_SIZE(NCPartyStatusWnd, 0x304);

// NCPetStatusWnd - the pet (UClass "CPetStatusWnd", 0x3D0): name, HP / MP bars and the food
// bar, from what the network handler keeps about the pet (read again by each paint). Shown
// only while the player has a pet (not a summon); a click right of the title bar acts on it.
class NCPetStatusWnd : public NCStatusWnd
{
    NW_DECLARE_CLASS(NCPetStatusWnd, NCStatusWnd, 0, 0x101DD8C8)

    UTexture* FedTex;                // +0x300 "L2UI.PetWnd.PetGreenBar1"
    UTexture* FedBackTex;            // +0x304 "L2UI.PetWnd.PetGreenBar2"
    PetInfo Pet;                     // +0x308 (0xAC in C2) copy of the handler's record, read again by each paint
    INT Unknown3B4;                  // +0x3B4 (0) the constructor clears it
    INT Unknown3B8[2];               // +0x3B8 (no stock code touches them)
    INT Unknown3C0;                  // +0x3C0 SetInfo
    INT Unknown3C4;                  // +0x3C4
    INT Unknown3C8;                  // +0x3C8

    NCPetStatusWnd(INT InBarWidth = 100);                                  // 0x100d54d0
    // ~NCPetStatusWnd                                               // 0x100d5540

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x10090b60 (folded)
    virtual INT OnCreate();                                          // 36 0x100d5590
    virtual void ShowWindow();                                       // 43 0x100d3280
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x100d56c0
    virtual INT OnLButtonDown(WPARAM wParam, LPARAM lParam);         // 61 0x100d3220

    void SetInfo(L2ParamStack& Params);                              // 0x100d31a0 (name ours)
};
NW_ASSERT_OFFSET(NCPetStatusWnd, Pet, 0x308);
NW_ASSERT_OFFSET(NCPetStatusWnd, Unknown3B4, 0x3B4);
NW_ASSERT_OFFSET(NCPetStatusWnd, Unknown3C0, 0x3C0);
NW_ASSERT_SIZE(NCPetStatusWnd, 0x3CC);

// NCSummonedStatusWnd - the summon (UClass "CSummonedStatusWnd", 0x3BC): name, HP / MP bars
// and a bar for the time it has left, run by the console (StartTimer, SetRemaining). Shown only
// while the player has a summon (not a pet).
class NCSummonedStatusWnd : public NCStatusWnd
{
    NW_DECLARE_CLASS(NCSummonedStatusWnd, NCStatusWnd, 0, 0x101DB9E0)

    NCProgressCtrl* TimeBar;         // +0x300
    PetInfo Summon;                  // +0x304 (0xAC in C2) copy of the handler's record, read again by each paint
    UBOOL bTimerStarted;             // +0x3B0 (StartTimer)
    INT Unknown3B4;                  // +0x3B4 (0) the constructor clears it

    NCSummonedStatusWnd(INT InBarWidth = 100);                             // 0x100d5950
    // ~NCSummonedStatusWnd                                          // 0x100d59c0

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x10090b60 (folded)
    virtual INT OnCreate();                                          // 36 0x100d5a10
    virtual INT OnDestroy();                                         // 37 0x100d5be0
    virtual void ShowWindow();                                       // 43 0x100d3330
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x100d5cc0
    virtual INT OnLButtonDown(WPARAM wParam, LPARAM lParam);         // 61 0x100d32d0

    // names ours
    void SetRemaining(DWORD Remaining);                              // 0x100d32c0
    void StartTimer(DWORD Max);                                      // 0x100d5c90
};
NW_ASSERT_OFFSET(NCSummonedStatusWnd, Summon, 0x304);
NW_ASSERT_OFFSET(NCSummonedStatusWnd, bTimerStarted, 0x3B0);
NW_ASSERT_OFFSET(NCSummonedStatusWnd, Unknown3B4, 0x3B4);
NW_ASSERT_SIZE(NCSummonedStatusWnd, 0x3B8);

#pragma pack(pop)