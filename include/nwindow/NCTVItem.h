#pragma once
// NCTVItem - one entry of a tree view (UClass "CTVItem", 0x1A24; C2 only): a picture and a text
// whose data record (NTVItemData, 0x1828 bytes) the tree copies in whole, the entries under it
// (Children) and whether they are shown (bExpanded). It paints nothing itself: NCTree::DrawItem
// puts it in place and draws it (the picture and the text), the click only toggles bExpanded.
// NCRecipeTree (the recipe windows' tree) is built on this family.

#include "nwindow/NControl.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

// The record a tree is given for an entry (name ours; 0x1828 bytes, from NCTree::AddItem's copy
// into the item at +0x1A4 and the item's constructor: the fields only AddItem sets are marked)
struct NTVItemData
{
    UTexture* Icon;                  // +0x000 the picture drawn left of the text
    INT Unknown04;                   // +0x004 (-1 in a new item)
    INT Unknown08;                   // +0x008
    INT Unknown0C;                   // +0x00C
    TCHAR Text[1024];                // +0x010 the entry's text (the extent is measured with it)
    TCHAR Text2[2048];               // +0x810 a second text (never read by the tree)
    INT Width;                       // +0x1810 set by AddItem: the icon's width + 2 + the text's
    INT Height;                      // +0x1814 set by AddItem: the icon's height
    INT IconW;                       // +0x1818 the picture's size
    INT IconH;                       // +0x181C
    INT Unknown1820;                 // +0x1820
    INT Unknown1824;                 // +0x1824
};
NW_ASSERT_SIZE(NTVItemData, 0x1828);

class NCTVItem : public NControl
{
    NW_DECLARE_CLASS(NCTVItem, NControl, 0, 0x101A6860)

    NTVItemData Data;                // +0x1A4 the copy of the record
    INT Unknown19CC;                 // +0x19CC
    TArray<NCTVItem*> Children;      // +0x19D0 the entries under this one (name ours)
    INT Unknown19DC;                 // +0x19DC (0)
    INT Unknown19E0;                 // +0x19E0 (0)
    INT Unknown19E4;                 // +0x19E4 (0)
    INT Unknown19E8;                 // +0x19E8 (0)
    INT Level;                       // +0x19EC the constructor's argument (name ours)
    UBOOL bExpanded;                 // +0x19F0 the children are shown (name ours)
    INT Unknown19F4;                 // +0x19F4 (0)
    INT Unknown19F8;                 // +0x19F8 (0)
    INT Unknown19FC;                 // +0x19FC (0)
    INT Unknown1A00;                 // +0x1A00 (0)
    INT HitX;                        // +0x1A04 the box a click toggles inside / outside of (name ours)
    INT HitY;                        // +0x1A08
    INT HitW;                        // +0x1A0C
    INT HitH;                        // +0x1A10
    INT Unknown1A14;                 // +0x1A14 (0)
    INT Unknown1A18;                 // +0x1A18 (0)
    INT CopyW;                       // +0x1A1C the window's size at OnCreate (name ours)
    INT CopyH;                       // +0x1A20

    NCTVItem(INT InLevel = -1);                                        // 0x1002b170
    // ~NCTVItem                                                     // 0x1002b2a0 (the children's array freed in line)

    virtual INT OnCreate();                                          // 36 0x1002a4b0
    virtual INT OnDestroy() { return 1; }                            // 37 0x10005b80 (folded)
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x1002a600
    virtual INT OnMouseMove(WPARAM wParam, LPARAM lParam) { return 1; }   // 60 0x100d2e70 (folded)
    virtual INT OnLButtonDown(WPARAM wParam, LPARAM lParam);         // 61 0x1002a4e0
    virtual INT OnLButtonUp(WPARAM wParam, LPARAM lParam) { return 1; }   // 62 0x100d2e70 (folded)

    // name ours: Child goes to the end of Children and is handed back (NULL: nothing)
    NCTVItem* AddChild(NCTVItem* Child);                             // 0x1002b0b0 (ret 4)
};
NW_ASSERT_OFFSET(NCTVItem, Data, 0x1A4);
NW_ASSERT_OFFSET(NCTVItem, Children, 0x19D0);
NW_ASSERT_OFFSET(NCTVItem, Level, 0x19EC);
NW_ASSERT_OFFSET(NCTVItem, bExpanded, 0x19F0);
NW_ASSERT_OFFSET(NCTVItem, HitX, 0x1A04);
NW_ASSERT_OFFSET(NCTVItem, CopyW, 0x1A1C);
NW_ASSERT_SIZE(NCTVItem, 0x1A24);

#pragma pack(pop)
