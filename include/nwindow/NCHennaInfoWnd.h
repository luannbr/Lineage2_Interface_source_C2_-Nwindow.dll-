#pragma once
// NCHennaInfoWnd - the confirmation of a tattoo (UClass "CHennaInfoWnd", 0x3530): the item (the dye) and the tattoo
// with their descriptions, the six stats before and after (the pairs "a > b" in two columns) and the player's adena;
// OK (id 0xF02) draws (Mode 0: RequestHennaEquip) or takes off (Mode 1: RequestHennaUnequip) the tattoo, Cancel
// (0xF03) goes back to the list the server sends again (RequestHennaItemList / RequestHennaUnequipList). The console
// fills it (NConsoleWnd's slots 135..144: ClearParams, the adena and the mode, SetHenna) and shows it.

#include "nwindow/NCItemWnd.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

// What SetHenna takes (name ours; 0x34 bytes, built on the console's stack: slots 135 and 140): the tattoo, its dye and
// the twelve stat values the window shows
struct NHennaParam
{
    INT HennaID;               // +0x00 (the tattoo in the game data: FL2GameData::GetHennaData)
    INT ItemID;                // +0x04 the dye (FL2GameData::GetItemData)
    INT Count;                 // +0x08 how many of it
    INT Price;                 // +0x0C
    INT bCanUse;               // +0x10 not read by SetHenna
    INT Stats[12];             // +0x14 copied to NCHennaInfoWnd::Stats
};
NW_ASSERT_OFFSET(NHennaParam, Stats, 0x14);
NW_ASSERT_SIZE(NHennaParam, 0x44);

// NWM_COMMAND ids (names ours)
enum
{
    NID_HENNAINFO_FRAME = 0xF01,
    NID_HENNAINFO_OK = 0xF02,
    NID_HENNAINFO_CANCEL = 0xF03,
};

class NCButton;

class NCHennaInfoWnd : public NCWnd
{
    NW_DECLARE_CLASS(NCHennaInfoWnd, NCWnd, 0, 0x101B88C8)

    UTexture* BackTex;               // +0x98 "L2UI.HennaWnd.HennaInfoBack"
    UTexture* AdenaTex;              // +0x9C "L2UI.WindowIcon.WindowIconAdena"
    NItemEntry Item;                 // +0xA0 the dye (SetHenna: name, icon and description from the item data, Count and
                                     //   UnknownC20 (the price) from the parameters)
    NItemEntry Henna;                // +0x1AC8 the tattoo (the name, description and icon of the game data)
    // the stats, in the order the parameters give them; the paint draws "Stats[2k] > Stats[2k+1]" in the cells
    //   0: right column, row 1 (SysString 107)   1: left, row 1 (104)   2: left, row 3 (106)
    //   3: right, row 3 (109)                    4: left, row 2 (105)   5: right, row 2 (108)
    INT Stats[12];                   // +0x34F0
    INT Mode;                        // +0x3520 0: to draw, 1: to take off (the console sets it)
    INT Adena;                       // +0x3524 the player's adena, shown at the bottom (the console sets it)
    NCButton* OKButton;              // +0x3528 id 0xF02
    NCButton* CancelButton;          // +0x352C id 0xF03

    NCHennaInfoWnd();                                                // 0x1007b230
    // ~NCHennaInfoWnd                                               // 0x1007b310

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x10079170
    virtual INT OnCreate();                                          // 36 0x10079c60
    virtual INT OnDestroy();                                         // 37 0x1007abb0
    virtual void ShowWindow();                                       // 43 0x100794b0
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x10079ea0
    // names ours
    virtual INT OnOK();                                              // 67 0x10079400
    virtual INT OnCancel();                                          // 68 0x10079460

    // names ours (the console calls them)
    void ClearParams();                                              // 0x100791f0 the stats zeroed
    void SetHenna(const NHennaParam* Param);                         // 0x10079240
};
NW_ASSERT_OFFSET(NCHennaInfoWnd, Item, 0xA0);
NW_ASSERT_OFFSET(NCHennaInfoWnd, Henna, 0x1AC8);
NW_ASSERT_OFFSET(NCHennaInfoWnd, Stats, 0x34F0);
NW_ASSERT_OFFSET(NCHennaInfoWnd, Mode, 0x3520);
NW_ASSERT_OFFSET(NCHennaInfoWnd, CancelButton, 0x352C);
NW_ASSERT_SIZE(NCHennaInfoWnd, 0x3530);

#pragma pack(pop)
