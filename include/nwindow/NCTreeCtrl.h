#pragma once
// NCTreeCtrl - a scrolling tree view (UClass "CTreeCtrl", 0xE4; C2 only): an NCScrollWnd with an
// NCTree (Tree) inside it, 16 pixels narrower than the window (the scroll bar's place). Scrolling
// moves the tree window and keeps its ScrollOffset; each paint hides the entries that are out
// of the view and shows the others. NCRecipeTreeCtrl is built on it.

#include "nwindow/NCScrollWnd.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCTree;
class NCTVItem;

class NCTreeCtrl : public NCScrollWnd
{
    NW_DECLARE_CLASS(NCTreeCtrl, NCScrollWnd, 0, 0x101A6D80)

    NCTree* Tree;                    // +0xD0 id 0xA00001 (name ours)
    BYTE _UnknownD4[0xE4 - 0xD4];    // +0xD4 never touched

    NCTreeCtrl();                                                    // 0x1002a9a0
    // ~NCTreeCtrl                                                   // 0x1002a9f0

    virtual INT OnCreate();                                          // 36 0x1002aa40
    virtual INT OnDestroy();                                         // 37 0x1002ab00
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x1002abb0
    virtual INT OnMouseWheel(WPARAM wParam, LPARAM lParam);          // 66 0x1002aef0
    virtual INT ScrollUp();                                          // 67 0x1002ace0
    virtual INT ScrollDown();                                        // 68 0x1002ada0
    virtual INT OnScrollBar(INT Value);                              // 69 0x1002ae70
    virtual void Clear();                                            // 70 0x1002b020 (name ours) the tree emptied, scrolled back
    virtual void ShowVisible(NCTVItem* Item);                        // 71 0x1002ac40 (name ours) Item and the entries under it shown
                                                                     //   or hidden by whether they are in the view
};
NW_ASSERT_OFFSET(NCTreeCtrl, Tree, 0xD0);
NW_ASSERT_SIZE(NCTreeCtrl, 0xE4);

#pragma pack(pop)
