#pragma once
// NCRecipeBookItemWnd - the recipe book's icon grid (UClass "CRecipeBookItemWnd", 0x110; new in C2). A plain
// NCItemWnd (6 x 7 icons, 160 recipes at most, a name line for the tool tip) whose entries are the recipes the
// character knows. Dragging a recipe onto the book's trash icon (NCRecipeBookWnd::HitTrash) asks to delete it
// (NDialogInfo ID 0x22, answered by the console: RequestRecipeItemDelete); a double click asks the server for the
// recipe's ingredients (RequestRecipeItemMakeInfo) and closes the grid.
//
// WindowProc is the stock body shared with NCInvenItemWnd (0x100c4610).

#include "nwindow/NCItemWnd.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCRecipeBookItemWnd : public NCItemWnd
{
    NW_DECLARE_CLASS(NCRecipeBookItemWnd, NCItemWnd, 0, 0x101D7C18)

    NCRecipeBookItemWnd();                                           // 0x100c6f40
    NCRecipeBookItemWnd(INT InColumns, INT InVisibleRows, INT InMaxItems, INT InCellSize); // 0x100c6fe0
    // ~NCRecipeBookItemWnd                                          // 0x100c6f90

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x100c4610 (shared with NCInvenItemWnd)
    virtual INT OnMouseMove(WPARAM wParam, LPARAM lParam);           // 60 0x100c49d0
    virtual INT OnLButtonDown(WPARAM wParam, LPARAM lParam);         // 61 0x100c4960
    virtual INT OnLButtonUp(WPARAM wParam, LPARAM lParam);           // 62 0x100c4ad0
    // names ours
    virtual INT OnLButtonDblClick(WPARAM wParam, LPARAM lParam);     // 73 0x100c4c70 WM_LBUTTONDBLCLK

    // name ours (the console calls it through NCRecipeBookWnd::AddRecipe)
    __declspec(noinline) UBOOL AddItem(NItemInfo* Info);             // 0x100c69e0 0 when full
};
NW_ASSERT_SIZE(NCRecipeBookItemWnd, 0x110);

#pragma pack(pop)
