#pragma once
// NCRecipeTree - the ingredient tree of the recipe information window (UClass "CRecipeTree", 0x1D8): an NCTree (the tree view
// control of the C2 NWindow: its entries are NCTVItem child windows; NCTree keeps the root in +0x1A4, what the last paint
// measured in +0x1B4 and the view offset in +0x1B8) that draws each entry as a 32 x 32 frame with the ingredient's icon, its
// name and "(have/need)", a plus / minus sign for an entry with children and a dimming square for an ingredient the player
// lacks. NCRecipeTreeCtrl (the scrolling frame) owns it; NCRecipeTreeWnd shows both.
//
// NCTree and NCTVItem belong to another group (w2newui: nwindow/NCTree.h, NCTVItem.h). Until their headers are in this tree
// the stand-in below declares the same classes with their layouts and names (copied from that group's headers: do not
// change one without the other); __has_include picks the real headers as soon as they exist and the stand-in disappears.
// In the stand-in the virtuals the recipe tree inherits are pure so that the vtable can be emitted without them.
//
// What this class adds (OnCreate 0x100c4830): the five textures of the entries. The entry data the recipe code fills (NCTVItem's
// NTVItemData): Unknown08 is how many the recipe needs, Unknown0C how many the player carries (names after their use here).

#if defined(__has_include)
#if __has_include("nwindow/NCTree.h")
#define NW_HAVE_NCTREE_H 1
#endif
#endif

#ifdef NW_HAVE_NCTREE_H
#include "nwindow/NCTree.h"
#else
#include "nwindow/NControl.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

// ---- stand-in for nwindow/NCTVItem.h and nwindow/NCTree.h (w2newui) ----
struct NTVItemData
{
    UTexture* Icon;                  // +0x000
    INT Unknown04;                   // +0x004 (-1 in a new item)
    INT Unknown08;                   // +0x008
    INT Unknown0C;                   // +0x00C
    TCHAR Text[1024];                // +0x010
    TCHAR Text2[2048];               // +0x810
    INT Width;                       // +0x1810
    INT Height;                      // +0x1814
    INT IconW;                       // +0x1818
    INT IconH;                       // +0x181C
    INT Unknown1820;                 // +0x1820
    INT Unknown1824;                 // +0x1824
};
NW_ASSERT_SIZE(NTVItemData, 0x1828);

class NCTVItem : public NControl
{
    NW_DECLARE_CLASS(NCTVItem, NControl, 0, 0x101A6860)

    NTVItemData Data;                // +0x1A4
    INT Unknown19CC;                 // +0x19CC
    TArray<NCTVItem*> Children;      // +0x19D0
    INT Unknown19DC;                 // +0x19DC
    INT Unknown19E0;                 // +0x19E0
    INT Unknown19E4;                 // +0x19E4
    INT Unknown19E8;                 // +0x19E8
    INT Level;                       // +0x19EC
    UBOOL bExpanded;                 // +0x19F0
    INT Unknown19F4;                 // +0x19F4
    INT Unknown19F8;                 // +0x19F8
    INT Unknown19FC;                 // +0x19FC
    INT Unknown1A00;                 // +0x1A00
    INT HitX;                        // +0x1A04
    INT HitY;                        // +0x1A08
    INT HitW;                        // +0x1A0C
    INT HitH;                        // +0x1A10
    INT Unknown1A14;                 // +0x1A14
    INT Unknown1A18;                 // +0x1A18
    INT CopyW;                       // +0x1A1C (the recipe tree keeps the entry's x here, CopyH its y)
    INT CopyH;                       // +0x1A20
};
NW_ASSERT_OFFSET(NCTVItem, Data, 0x1A4);
NW_ASSERT_OFFSET(NCTVItem, Children, 0x19D0);
NW_ASSERT_OFFSET(NCTVItem, bExpanded, 0x19F0);
NW_ASSERT_OFFSET(NCTVItem, CopyW, 0x1A1C);
NW_ASSERT_SIZE(NCTVItem, 0x1A24);

class NCTree : public NControl
{
    NW_DECLARE_CLASS(NCTree, NControl, 0, 0x101A6340)

    NCTVItem* Root;                  // +0x1A4
    BYTE _Unknown1A8[0x1B4 - 0x1A8]; // +0x1A8
    INT ContentHeight;               // +0x1B4
    INT ScrollOffset;                // +0x1B8

    virtual INT OnDestroy() = 0;                                     // 37 0x1002a580
    virtual NCTVItem* AddItem(NTVItemData* Data, NCTVItem* Parent) = 0;   // 68 0x1002b300
    virtual void DeleteItem(NCTVItem** Item) = 0;                    // 69 0x1002a730
    virtual void Clear() = 0;                                        // 70 0x1002a570
    virtual INT DrawItem(NCGDevice* Device, NCTVItem* Item, INT X, INT Y) = 0;   // 71 0x1002a830
};
NW_ASSERT_OFFSET(NCTree, Root, 0x1A4);
NW_ASSERT_OFFSET(NCTree, ScrollOffset, 0x1B8);
NW_ASSERT_SIZE(NCTree, 0x1BC);
#pragma pack(pop)
#endif

#pragma pack(push, 4)

class NCRecipeTree : public NCTree
{
    NW_DECLARE_CLASS(NCRecipeTree, NCTree, 0, 0x101D6258)

    UTexture* FrameTex;              // +0x1BC "L2UI.RecipeWnd.RecipeTreeIconBack": an entry with children, collapsed
    UTexture* FrameOpenTex;          // +0x1C0 "L2UI.RecipeWnd.RecipeTreeIconBack_click": expanded
    UTexture* FrameLeafTex;          // +0x1C4 "L2UI.RecipeWnd.RecipeTreeIconDisableBack": a leaf
    UTexture* PlusTex;               // +0x1C8 "L2UI.RecipeWnd.TreePlus"
    UTexture* MinusTex;              // +0x1CC "L2UI.RecipeWnd.TreeMinus"
    INT Unknown1D0;                  // +0x1D0 (name ours) not touched by this class
    INT Unknown1D4;                  // +0x1D4 (name ours) not touched by this class

    NCRecipeTree();                                                  // 0x100c5960
    // ~NCRecipeTree                                                 // 0x100c59b0

    virtual INT OnCreate();                                          // 36 0x100c4830
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x100c5a00
    // 71 0x100c5a90 (ret 0x10): draws Item at (X, Y) and its children below / beside it and returns the height used
    virtual INT DrawItem(NCGDevice* Device, NCTVItem* Item, INT X, INT Y);
};
NW_ASSERT_OFFSET(NCRecipeTree, FrameTex, 0x1BC);
NW_ASSERT_OFFSET(NCRecipeTree, MinusTex, 0x1CC);
NW_ASSERT_SIZE(NCRecipeTree, 0x1D8);

#pragma pack(pop)
