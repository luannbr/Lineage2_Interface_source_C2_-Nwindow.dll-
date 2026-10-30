#pragma once
// NCMatchListCtrl - the party matching list (UClass "CMatchListCtrl", 0x210): class icon,
// level icon, name, range and party status per player (NCMatchWnd fills it). Class and level
// sort by the key the window stored in the cell's Param1; a click selects the player and asks
// the server for the details the tool tip shows (level, class and memo, when open).
//
// Row data (NCMatchWnd): Cells[0].Param2 class ID, Cells[1].Param2 level, Cells[2].Param1 the
// player's object ID; row Param1 / Param2: level / class open, row Text: the memo.

#include "nwindow/NCListCtrl.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCMatchListCtrl : public NCListCtrl
{
    NW_DECLARE_CLASS(NCMatchListCtrl, NCListCtrl, 0, 0x101C2338)

    INT Unknown20C;                  // +0x210 not used   // C2: offset derived, no evidence

    NCMatchListCtrl();                                               // 0x100960d0
    NCMatchListCtrl(INT InMaxRows, INT InVisibleRows, INT InMaxColumns, INT Unused); // 0x10096170
    // ~NCMatchListCtrl                                              // 0x10096120

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x10094660
    virtual INT OnLButtonDown(WPARAM wParam, LPARAM lParam);         // 61 0x100952b0
    virtual INT SortByColumn(INT Column);                            // 73 0x10095aa0
    void DrawRowToolTip(NCGDevice* Device, NListRow* Row, INT X, INT Y); // C2: slot gone (was 69)

    // names ours
    void BuildToolTip(NListRow* Row);                                // 0x10073e30 (C1 only: no C2 match)
    INT GetRowPlayerID(INT Index);                                   // 0x10095210 -1: none
    INT GetSelectedPlayerID();                                       // 0x10095240 -1: none
    const TCHAR* GetSelectedName();                                  // 0x10095270 NULL: none
};
NW_ASSERT_OFFSET(NCMatchListCtrl, Unknown20C, 0x210);
NW_ASSERT_SIZE(NCMatchListCtrl, 0x214);

#pragma pack(pop)