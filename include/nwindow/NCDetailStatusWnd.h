#pragma once
// NCDetailStatusWnd - the character tab of the status window (UClass "CDetailStatusWnd",
// 0xD4; Alt+T): title and name, clan, level and class, HP / MP / exp / weight bars, the combat
// values, the six stats, karma and PvP, all read from the player's User at each paint.
// NCGMDetailStatusWnd derives from it.
//
// NCMainWnd +0x94 is this window (tab 0, created at 0, 65, 240 x 300).

#include "nwindow/NCWnd.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCDetailStatusWnd : public NCWnd
{
    NW_DECLARE_CLASS(NCDetailStatusWnd, NCWnd, 0, 0x101DCE80)

    User* Player;                    // +0x98 the player (each paint)   // C2: offset derived, no evidence
    UTexture* BackTex;               // +0x9C "L2UI.StatusWnd.statusback"
    UTexture* HPTex;                 // +0xA0 "L2UI.StatusWnd.statusHP1"
    UTexture* MPTex;                 // +0xA4 "L2UI.StatusWnd.statusMP1"
    UTexture* EXPTex;                // +0xA8 "L2UI.StatusWnd.statusEXP1"
    UTexture* SPTex;                 // +0xAC "L2UI.StatusWnd.statusSP" (loaded, never drawn)
    UTexture* WeightTex[4];          // +0xB0 "L2UI.StatusWnd.statusWt1" .. 4: up to 50%, 66.66%,
                                     //   80% of the weight limit, above
    UTexture* NameLeftTex;           // +0xC0 "L2UI.StatusWnd.statusName" (left of the name)
    UTexture* NameRightTex;          // +0xC4 "L2UI.StatusWnd.statusName-1" (right of the name)
    UTexture* CrestTex;              // +0xC8 NCGMDetailStatusWnd: the clan crest found by each   // C2: offset derived, no evidence
                                     //   paint (not used by this class)
    INT UnknownCC[17];               // +0xCC .. +0x110 cleared by the C2 constructor (no other stock code touches them) except for
                                     //   six that OnPaint reads as the stats' bonuses, shown "N(+B)": see BONUS_* (names ours)
    enum { BONUS_Int = (0xE4 - 0xCC) / 4, BONUS_Str = (0xEC - 0xCC) / 4, BONUS_Con = (0xF4 - 0xCC) / 4,
           BONUS_Men = (0xFC - 0xCC) / 4, BONUS_Dex = (0x104 - 0xCC) / 4, BONUS_Wit = (0x10C - 0xCC) / 4 };
    BYTE _Unknown110[0x11C - 0x110]; // +0x110 (new in C2, up to sizeof)

    NCDetailStatusWnd();                                             // 0x100d6750
    // ~NCDetailStatusWnd                                            // 0x10073b50

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x10043ef0 (folded)
    virtual INT OnCreate();                                          // 36 0x100d35e0
    virtual INT OnDestroy();                                         // 37 0x100d36c0 (folded)
    virtual void ShowWindow();                                       // 43 0x1007b460 (folded)
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x100d67d0
};
NW_ASSERT_OFFSET(NCDetailStatusWnd, Player, 0x98);
NW_ASSERT_OFFSET(NCDetailStatusWnd, WeightTex, 0xB0);
NW_ASSERT_OFFSET(NCDetailStatusWnd, NameRightTex, 0xC4);
NW_ASSERT_OFFSET(NCDetailStatusWnd, CrestTex, 0xC8);
NW_ASSERT_SIZE(NCDetailStatusWnd, 0x11C);

// NCGMDetailStatusWnd - a GM's view of another player's status (UClass "CGMDetailStatusWnd",
// 0x2DC): the character tab under a title bar, laid out a little lower, drawn from the User it
// keeps (the console fills it with the server's answer). HP and MP in the common font with a
// shadow, exp in percent at the right, a white name (no karma color) with the title before it
// in one string, the crest looked up by its texture name.
//
// Desktop (NCConsole) +0x120 is this window.
class NCGMDetailStatusWnd : public NCDetailStatusWnd
{
    NW_DECLARE_CLASS(NCGMDetailStatusWnd, NCDetailStatusWnd, 0, 0x101B40C8)

    User Target;                     // +0x11C (0x1F8 in C2) the GM's answer; Engine's constructor builds it

    NCGMDetailStatusWnd();                                           // 0x10074a30
    // ~NCGMDetailStatusWnd                                          // 0x10074ab0

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x10090b60 (folded)
    virtual INT OnCreate();                                          // 36 0x10074b00
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x10074bb0

    void SetTarget(User InTarget);                                   // 0x100730d0 (name ours) the
                                                                     //   console's, by value
};
NW_ASSERT_OFFSET(NCGMDetailStatusWnd, Target, 0x11C);
NW_ASSERT_SIZE(NCGMDetailStatusWnd, 0x314);

// NWM_COMMAND id of the GM window's title bar
enum { NID_GMDETAIL_FRAME = 0xF0F118 };

#pragma pack(pop)