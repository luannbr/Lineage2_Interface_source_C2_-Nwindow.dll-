#pragma once
// NCTree - the entries of a tree view (UClass "CTree", 0x1BC; C2 only): the root NCTVItem and, under
// it, the others, each a child window that the paint puts in place (DrawItem lays them out one
// under the other, the children of an expanded entry indented, and returns the height used).
// NCTreeCtrl holds one inside a scrolling window.

#include "nwindow/NCTVItem.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCTree : public NControl
{
    NW_DECLARE_CLASS(NCTree, NControl, 0, 0x101A6340)

    NCTVItem* Root;                  // +0x1A4 NULL until the first AddItem without a parent (name ours)
    BYTE _Unknown1A8[0x1B4 - 0x1A8]; // +0x1A8 never touched
    INT ContentHeight;               // +0x1B4 what DrawItem returned in the last paint (name ours)
    INT ScrollOffset;                // +0x1B8 how far NCTreeCtrl moved it up (name ours)

    NCTree();                                                        // 0x1002a660
    // ~NCTree                                                       // 0x1002a6e0

    virtual INT OnDestroy();                                         // 37 0x1002a580
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x1002a7a0
    virtual NCTVItem* AddItem(NTVItemData* Data, NCTVItem* Parent);  // 68 0x1002b300 (name ours) the root when Parent is
                                                                     //   NULL (only the first), else under Parent; NULL: not added
    virtual void DeleteItem(NCTVItem** Item);                        // 69 0x1002a730 (name ours) the entry and what is under it
    virtual void Clear();                                            // 70 0x1002a570 (name ours) everything
    virtual INT DrawItem(NCGDevice* Device, NCTVItem* Item, INT X, INT Y);  // 71 0x1002a830 (name ours)
};
NW_ASSERT_OFFSET(NCTree, Root, 0x1A4);
NW_ASSERT_OFFSET(NCTree, ContentHeight, 0x1B4);
NW_ASSERT_OFFSET(NCTree, ScrollOffset, 0x1B8);
NW_ASSERT_SIZE(NCTree, 0x1BC);

#pragma pack(pop)
