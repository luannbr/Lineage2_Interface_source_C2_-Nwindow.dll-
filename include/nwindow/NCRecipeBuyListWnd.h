#pragma once
// NCRecipeBuyListWnd - what a dwarf's recipe shop offers, seen by the customer (UClass "CRecipeBuyListWnd", 0xB8; new
// in C2; the desktop's NCConsole +0x144). The console opens it when the server sends the shop's list: the list box of
// the recipes (NCRecipeListBox: name, price, success rate), the dwarf's MP bar ("Creator MP") and the player's
// adena. Clicking a recipe asks the server for its ingredients (RequestRecipeShopMakeInfo: the shop owner and the
// recipe), which opens the manufacture window (NCRecipeBuyManufactureWnd, the desktop's +0x148) over this one.

#include "nwindow/NCWnd.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCRecipeListBox;
struct NRecipeRow;

enum { NID_RECIPEBUYLIST_FRAME = 0xFF, NID_RECIPEBUYLIST_LIST = 0x1A05 };   // NWM_COMMAND ids (names ours)

class NCRecipeBuyListWnd : public NCWnd
{
    NW_DECLARE_CLASS(NCRecipeBuyListWnd, NCWnd, 0, 0x101D4380)

    NCRecipeListBox* RecipeList;     // +0x98 (name ours) id 0x1A05
    UTexture* BackTex;               // +0x9C "L2UI.RecipeWnd.RecipePersonalListBack"
    UTexture* MPTex;                 // +0xA0 "L2UI.PlayerStatusWnd.MP" (the bar's texture)
    UTexture* AdenaTex;              // +0xA4 "L2UI.WindowIcon.WindowIconAdena"
    INT ShopOwnerID;                 // +0xA8 (-1) (name ours) whose shop it is (stock 0x100c11c0, which the console calls)
    INT MP;                          // +0xAC (name ours) the dwarf's MP (the console writes it)
    INT MaxMP;                       // +0xB0 (the console writes it)
    INT Adena;                       // +0xB4 the player's adena (stock 0x100c11d0, which the console calls)

    NCRecipeBuyListWnd();                                            // 0x100c1fd0
    // ~NCRecipeBuyListWnd                                           // 0x100c2040

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x100c1150
    virtual INT OnCreate();                                          // 36 0x100c42f0
    virtual INT OnDestroy();                                         // 37 0x100c2090
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x100c20d0
    // name ours
    virtual INT OnSelectRecipe(LPARAM Index);                        // 67 0x100c2320 a recipe of the list was clicked
                                                                     //   (the list box's NWM_COMMAND code; not read)

    // names ours (the console calls them)
    void ClearList();                                                // 0x100c11b0 the list box's Clear
    void AddRecipe(const NRecipeRow* Src);                           // 0x100c3510 the list box's AddEntry (a tail call)
    void SetShopOwner(INT InOwnerID);                                // 0x100c11c0
    void SetAdena(INT InAdena);                                      // 0x100c11d0
};
NW_ASSERT_OFFSET(NCRecipeBuyListWnd, RecipeList, 0x98);
NW_ASSERT_OFFSET(NCRecipeBuyListWnd, ShopOwnerID, 0xA8);
NW_ASSERT_OFFSET(NCRecipeBuyListWnd, Adena, 0xB4);
NW_ASSERT_SIZE(NCRecipeBuyListWnd, 0xB8);

#pragma pack(pop)
