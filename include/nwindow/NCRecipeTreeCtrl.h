#pragma once
// NCRecipeTreeCtrl - the scrolling frame of the recipe tree (UClass "CRecipeTreeCtrl", 0xE8): an NCTreeCtrl (a scroll window
// with a tree in it) that keeps its own NCRecipeTree in +0xE4 instead of NCTreeCtrl's +0xD0 (the C2 source duplicated the
// base class' code with the new tree; every override here is NCTreeCtrl's body on the field at +0xE4). The tree is made
// as wide as the frame less its slider (16 pixels); the scroll bar moves the tree window up and down (MoveWindow) and the
// tree keeps how far in its +0x1B8 (ScrollOffset). Paint resizes the frame's scroll range from what the tree measured
// (+0x1B4 + 21) and asks for the entries to be shown or hidden by their position (vslot 71).
//
// NCTreeCtrl belongs to another group (w2newui: nwindow/NCTreeCtrl.h): until that header is in the tree a stand-in with the same
// layout and names declares it (see NCRecipeTree.h for the same arrangement); __has_include picks the real one.

#include "nwindow/NCScrollWnd.h"
#include "nwindow/NCRecipeTree.h"

#if defined(__has_include)
#if __has_include("nwindow/NCTreeCtrl.h")
#define NW_HAVE_NCTREECTRL_H 1
#endif
#endif

#ifdef NW_HAVE_NCTREECTRL_H
#include "nwindow/NCTreeCtrl.h"
#else
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets
// ---- stand-in for nwindow/NCTreeCtrl.h (w2newui) ----
class NCTreeCtrl : public NCScrollWnd
{
    NW_DECLARE_CLASS(NCTreeCtrl, NCScrollWnd, 0, 0x101A6D80)

    NCTree* Tree;                    // +0xD0
    BYTE _UnknownD4[0xE4 - 0xD4];    // +0xD4

    virtual INT OnCreate() = 0;                                      // 36 0x1002aa40
    virtual INT OnDestroy() = 0;                                     // 37 0x1002ab00
    virtual INT OnPaint(NCGDevice* Device) = 0;                      // 55 0x1002abb0
    virtual INT OnMouseWheel(WPARAM wParam, LPARAM lParam) = 0;      // 66 0x1002aef0
    virtual INT ScrollUp() = 0;                                      // 67 0x1002ace0
    virtual INT ScrollDown() = 0;                                    // 68 0x1002ada0
    virtual INT OnScrollBar(INT Value) = 0;                          // 69 0x1002ae70
    virtual void Clear() = 0;                                        // 70 0x1002b020
    virtual void ShowVisible(NCTVItem* Item) = 0;                    // 71 0x1002ac40
};
NW_ASSERT_OFFSET(NCTreeCtrl, Tree, 0xD0);
NW_ASSERT_SIZE(NCTreeCtrl, 0xE4);
#pragma pack(pop)
#endif

#pragma pack(push, 4)

class NCRecipeTreeCtrl : public NCTreeCtrl
{
    NW_DECLARE_CLASS(NCRecipeTreeCtrl, NCTreeCtrl, 0, 0x101D71C0)

    NCRecipeTree* RecipeTree;        // +0xE4 the tree control, id 0xA00, (0, 0) with the frame's width less 16 (name ours; NCTreeCtrl's own
                                     //   Tree at +0xD0 stays NULL)

    NCRecipeTreeCtrl();                                              // 0x100c5d70 (NCTreeCtrl's constructor 0x1002a9a0, the vtable, +0xCC = 16)
    // ~NCRecipeTreeCtrl                                             // 0x100c5dc0

    virtual INT OnCreate();                                          // 36 0x100c5e10
    virtual INT OnDestroy();                                         // 37 0x100c5ed0
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x100c5f10
    virtual INT OnMouseWheel(WPARAM wParam, LPARAM lParam);          // 66 0x100c62c0
    virtual INT ScrollUp();                                          // 67 0x100c60b0
    virtual INT ScrollDown();                                        // 68 0x100c6170
    virtual INT OnScrollBar(INT Value);                              // 69 0x100c6240
    virtual void Clear();                                            // 70 0x100c5fa0 the tree emptied and the view back at the top
    virtual void ShowVisible(NCTVItem* Item);                        // 71 0x100c6010 an entry shown when it is inside the frame, hidden when not; its children too

    // 0x100c6af0 (ret 0x10; name ours): the recipe that makes ItemID at SuccessRate percent, as entries under Parent: the item's
    // entry (icon, name, description, how many are needed and how many the first inventory page holds) and, for each
    // ingredient, the same for the recipe that makes it at 100 percent (down to the items nobody makes). Without a
    // recipe (or with an empty record) the entry is the item's own record and a leaf.
    void AddRecipe(INT ItemID, INT SuccessRate, INT Need, NCTVItem* Parent);
};
NW_ASSERT_OFFSET(NCRecipeTreeCtrl, RecipeTree, 0xE4);
NW_ASSERT_SIZE(NCRecipeTreeCtrl, 0xE8);

#pragma pack(pop)
