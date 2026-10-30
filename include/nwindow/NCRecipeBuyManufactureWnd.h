#pragma once
// NCRecipeBuyManufactureWnd - "Item Creation" for an offer of a recipe shop (UClass "CRecipeBuyManufactureWnd", 0x1B04):
// NCRecipeManufactureWnd's window (same product, ingredients, creator MP bar, outcome line and buttons; see that header for
// the common fields) for a customer of a recipe shop: Create asks the shop's owner to make the item
// (RequestRecipeShopMakeDo(ShopID, RecipeID)) and <PREV goes back to the shop's list (RequestRecipeShopSellList(ShopID)).
// The console opens it (NConsoleWnd::vfn131 0x10059330, the answer of RequestRecipeShopMakeInfo): the buy list hidden,
// Reset, the shop id, the creator's MP, the outcome, SetRecipe, the window placed where the list was, shown and focused.
//
// Differences from NCRecipeManufactureWnd (layout from the constructor 0x100c44a0 and the diff of the two OnCreate /
// OnPaint / SetRecipe bodies, which are identical but for these offsets): two new fields (+0x1AE8 ShopID, +0x1AEC) push the
// outcome to +0x1AF0 and the four buttons to +0x1AF4.., and the close button's id is 0xF8, not 0xFD.
//
// Desktop (NCConsole) +0x148.

#include "nwindow/NCRecipeManufactureItemWnd.h"
#include "nwindow/NCRecipeManufactureWnd.h"
#include "nwindow/NRecipeData.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCButton;

enum { NID_RECIPEBUYMAKE_CLOSE = 0xF8 };   // the close button (NCRecipeManufactureWnd's is 0xFD)

class NCRecipeBuyManufactureWnd : public NCWnd
{
    NW_DECLARE_CLASS(NCRecipeBuyManufactureWnd, NCWnd, 0, 0x101D52F0)

    NCRecipeManufactureItemWnd* IngredientWnd;  // +0x98
    UTexture* BackTex;               // +0x9C
    UTexture* MPTex;                 // +0xA0
    NItemEntry Product;              // +0xA4
    INT RecipeID;                    // +0x1ACC
    INT ProductID;                   // +0x1AD0
    INT ProductCount;                // +0x1AD4
    INT MPCost;                      // +0x1AD8
    INT SuccessRate;                 // +0x1ADC
    INT CreatorMP;                   // +0x1AE0
    INT CreatorMaxMP;                // +0x1AE4
    INT ShopID;                      // +0x1AE8 the shop's owner (the console writes it; the requests take it first)
    INT Unknown1AEC;                 // +0x1AEC (name ours) zeroed by the constructor and Reset, never read by this class
    INT Result;                      // +0x1AF0 (-1) the last try: 0 failed, 1 succeeded
    NCButton* TreeButton;            // +0x1AF4 id 0xFE
    NCButton* MakeButton;            // +0x1AF8 id 0xF9
    NCButton* PrevButton;            // +0x1AFC id 0xFA
    NCButton* CloseButton;           // +0x1B00 id 0xF8

    NCRecipeBuyManufactureWnd();                                     // 0x100c44a0
    // ~NCRecipeBuyManufactureWnd                                    // 0x100c4580

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x100c11e0
    virtual INT OnCreate();                                          // 36 0x100c23c0
    virtual INT OnDestroy();                                         // 37 0x100c27e0
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x100c28d0
    // names ours
    virtual INT OnPrev();                                            // 67 0x100c12e0 hides the window, asks for the shop's list
    virtual INT OnClose();                                           // 68 0x100a0830 (NCOptionWnd::OnCancel's body)
    virtual INT OnShowTree();                                        // 69 0x100c1310 (hooked here; NCRecipeManufactureWnd shares it)
    virtual INT OnMake();                                            // 70 0x100c13b0 RequestRecipeShopMakeDo(ShopID, RecipeID)

    // names ours
    void Reset();                                                    // 0x100c1290 the numbers zeroed (not the outcome), the grid emptied
    void SetRecipe(NRecipeData* Rec);                                // 0x100c3520 (NCRecipeManufactureWnd::SetRecipe's code)
    UBOOL AddIngredient(NItemEntry* Item);                           // 0x100c7dc0 (NCRecipeManufactureWnd's)
};
NW_ASSERT_OFFSET(NCRecipeBuyManufactureWnd, Product, 0xA4);
NW_ASSERT_OFFSET(NCRecipeBuyManufactureWnd, ShopID, 0x1AE8);
NW_ASSERT_OFFSET(NCRecipeBuyManufactureWnd, Result, 0x1AF0);
NW_ASSERT_OFFSET(NCRecipeBuyManufactureWnd, TreeButton, 0x1AF4);
NW_ASSERT_OFFSET(NCRecipeBuyManufactureWnd, CloseButton, 0x1B00);
NW_ASSERT_SIZE(NCRecipeBuyManufactureWnd, 0x1B04);

#pragma pack(pop)
