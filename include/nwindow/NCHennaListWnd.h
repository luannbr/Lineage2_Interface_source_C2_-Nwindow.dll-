#pragma once
// NCHennaListWnd - the tattoo lists of the symbol maker (UClass "CHennaListWnd", 0xAC): the tattoos that can be drawn
// (Mode 0, the title "Select the symbol to add") or taken off (Mode 1), in an NCHennaListBox, and the player's adena
// at the bottom. A click on a row asks the server for its information (RequestHennaItemInfo / RequestHennaUnequipInfo):
// the answer fills NCHennaInfoWnd. The console fills it (NConsoleWnd's slots 135..144: it keeps this window at
// NCConsole +0x14C and the information window at +0x150).

#include "nwindow/NCHennaListBox.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

// NWM_COMMAND ids (names ours)
enum
{
    NID_HENNALIST_FRAME = 0xF01,
    NID_HENNALIST_LIST = 0x1A06,
};

class NCHennaListWnd : public NCWnd
{
    NW_DECLARE_CLASS(NCHennaListWnd, NCWnd, 0, 0x101B83A8)

    NCHennaListBox* ListBox;         // +0x98 100 rows kept, 5 seen
    UTexture* BackTex;               // +0x9C "L2UI.HennaWnd.HennaListBack"
    UTexture* AdenaTex;              // +0xA0 "L2UI.WindowIcon.WindowIconAdena"
    INT Mode;                        // +0xA4 0: to add, 1: to take off (set by the console; not by the constructor)
    INT Adena;                       // +0xA8 the player's adena, shown at the bottom (the console sets it)

    NCHennaListWnd();                                                // 0x100798a0
    // ~NCHennaListWnd                                               // 0x10079900

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x10079090
    virtual INT OnCreate();                                          // 36 0x1007b090
    virtual INT OnDestroy();                                         // 37 0x10079950
    virtual void ShowWindow();                                       // 43 0x10079110
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x10079a00 (the db names it NCRecipeListWnd's: copied code)
    // names ours
    virtual INT OnSelect(INT Index);                                 // 67 0x10079ba0 a row clicked

    // names ours (the console calls them)
    void Clear();                                                    // 0x100790f0 the rows and the adena
    void AddEntry(const NHennaListEntry* Src);                       // 0x1007aef0 (a tail jump into the list box's)
};
NW_ASSERT_OFFSET(NCHennaListWnd, BackTex, 0x9C);
NW_ASSERT_OFFSET(NCHennaListWnd, Mode, 0xA4);
NW_ASSERT_OFFSET(NCHennaListWnd, Adena, 0xA8);
NW_ASSERT_SIZE(NCHennaListWnd, 0xAC);

#pragma pack(pop)
