#pragma once
// NCRecipeManufactureWnd - "Item Creation" for the player's own recipe (UClass "CRecipeManufactureWnd", 0x1AFC): the
// product (icon, name, grade, "MP cost", "Success Rate", "Number of Resulting Items"), the ingredients in a grid below
// ("Ingredient(s)"; dimmed when the player lacks them), the creator's MP bar, the outcome of the last try and the count
// of the product the player keeps, with the buttons Create, <PREV (back to the recipe book) and Close, and a tree
// button that opens NCRecipeTreeWnd. The console opens it with the recipe's record, the player's MP and the outcome
// (NConsoleWnd::vfn125, the answer of RequestRecipeItemMakeInfo): Reset, SetRecipe, then the numbers.
//
// Desktop (NCConsole) +0x13C. NCRecipeBuyManufactureWnd (0x1B04) is the same window for a recipe shop's offer: the same
// code with two more fields (the shop's owner and one unnamed) and another close id; it is in NCRecipeBuyManufactureWnd.h
// and both live in src/nwindow/NCRecipeManufactureWnd.cpp.
//
// Layout from the constructor 0x100c7260, OnCreate 0x100c7390, OnDestroy 0x100c5150, OnPaint 0x100c5240 and the console's
// writers (vfn125 0x10059140: +0x1AE0, +0x1AE4, +0x1AE8). The product is an NItemEntry used for its strings, icon and
// grade only: Icon (+0xA4), Name (+0xB0), IconName (+0x4B0), Description (+0x7B0), CrystalGrade (+0x19FC).

#include "nwindow/NCRecipeManufactureItemWnd.h"
#include "nwindow/NRecipeData.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCButton;

// NWM_COMMAND ids of the recipe making windows (names ours; the buy window closes with 0xF8)
enum
{
    NID_RECIPEMAKE_FRAME = 0xFF,      // the title bar (NControl::ID)
    NID_RECIPEMAKE_CREATE = 0xF9,     // -> OnMake (vslot 70)
    NID_RECIPEMAKE_PREV = 0xFA,       // -> OnPrev (vslot 67)
    NID_RECIPEMAKE_CLOSE = 0xFD,      // -> OnClose (vslot 68); NCRecipeBuyManufactureWnd: 0xF8
    NID_RECIPEMAKE_TREE = 0xFE,       // -> OnShowTree (vslot 69)
};

class NCRecipeManufactureWnd : public NCWnd
{
    NW_DECLARE_CLASS(NCRecipeManufactureWnd, NCWnd, 0, 0x101D6780)

    NCRecipeManufactureItemWnd* IngredientWnd;  // +0x98 the ingredients, 6 x 3 cells at (16, 128) 0xDF x 0x68
    UTexture* BackTex;               // +0x9C "L2UI.RecipeWnd.RecipeMakingBack"
    UTexture* MPTex;                 // +0xA0 "L2UI.PlayerStatusWnd.MP" (the creator's MP bar)
    NItemEntry Product;              // +0xA4 the product: the icon, the strings and the grade (see above)
    INT RecipeID;                    // +0x1ACC what the requests take (NRecipeData::RecipeID)
    INT ProductID;                   // +0x1AD0 the item made: the count kept is the sum of the inventory's stacks of it
    INT ProductCount;                // +0x1AD4 "Number of Resulting Items"
    INT MPCost;                      // +0x1AD8 "MP cost"
    INT SuccessRate;                 // +0x1ADC percent
    INT CreatorMP;                   // +0x1AE0 the player's MP (the bar's numerator), set by the console
    INT CreatorMaxMP;                // +0x1AE4 its maximum (the bar is 152 pixels at most)
    INT Result;                      // +0x1AE8 (-1) the last try: 0 failed, 1 succeeded (OnPaint shows the system message
                                     //   960 / 959 with the product's name); the console sets it
    NCButton* TreeButton;            // +0x1AEC id 0xFE, "L2UI.RecipeWnd.RecipeTreeBtn" at (200, 43) 32 x 32
    NCButton* MakeButton;            // +0x1AF0 id 0xF9 "Create" (sysstring 645) at (10, 335)
    NCButton* PrevButton;            // +0x1AF4 id 0xFA "<PREV" (543) at (87, 335)
    NCButton* CloseButton;           // +0x1AF8 id 0xFD "Close" (646) at (164, 335)

    NCRecipeManufactureWnd();                                        // 0x100c7260
    // ~NCRecipeManufactureWnd                                       // 0x100c7330

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x100c4700
    virtual INT OnCreate();                                          // 36 0x100c7390
    virtual INT OnDestroy();                                         // 37 0x100c5150
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x100c5240
    // names ours
    virtual INT OnPrev();                                            // 67 0x100c47f0 hides the window and asks for the recipe book
    virtual INT OnClose();                                           // 68 0x100a0830 (NCOptionWnd::OnCancel's body)
    virtual INT OnShowTree();                                        // 69 0x100c1310 (the buy window's, shared)
    virtual INT OnMake();                                            // 70 0x100c4810 RequestRecipeItemMakeSelf(RecipeID)

    // names ours
    void Reset();                                                    // 0x100c47b0 the numbers zeroed, the grid emptied
    // 0x100c7de0: shows Rec (the same code as the buy window's 0x100c3520): the numbers, the product's strings and icon, and
    // an entry per ingredient with how many of it the player carries
    void SetRecipe(NRecipeData* Rec);
    // 0x100c7dc0 (shared with the buy window): the entry goes to the ingredient grid
    UBOOL AddIngredient(NItemEntry* Item);
};
NW_ASSERT_OFFSET(NCRecipeManufactureWnd, Product, 0xA4);
NW_ASSERT_OFFSET(NCRecipeManufactureWnd, RecipeID, 0x1ACC);
NW_ASSERT_OFFSET(NCRecipeManufactureWnd, CreatorMP, 0x1AE0);
NW_ASSERT_OFFSET(NCRecipeManufactureWnd, Result, 0x1AE8);
NW_ASSERT_OFFSET(NCRecipeManufactureWnd, TreeButton, 0x1AEC);
NW_ASSERT_OFFSET(NCRecipeManufactureWnd, CloseButton, 0x1AF8);
NW_ASSERT_SIZE(NCRecipeManufactureWnd, 0x1AFC);

#pragma pack(pop)
