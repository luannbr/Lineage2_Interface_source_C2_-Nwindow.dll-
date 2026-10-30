#pragma once
// NCRecipeTreeWnd - "Recipe Information" (UClass "CRecipeTreeWnd", 0x1AD8): the tree of what a recipe's product needs, opened
// by the tree button of the recipe windows (NCRecipeManufactureWnd / NCRecipeBuyManufactureWnd::OnShowTree): the product's
// icon, name and grade, its "MP cost", "Success Rate" and "Lv.N", then a tree control (NCRecipeTreeCtrl) with one node
// per ingredient, each opening to the recipe that makes it (sub_100c6af0, the recursion through
// FL2GameData::GetRecipeDataBy2Condition(item, 100)), and a Close button.
//
// Desktop (NCConsole) +0x138 is this window (NewWnd138 there; recipe book +0x134, manufacture +0x13C, shop +0x140,
// buy list +0x144, buy-manufacture +0x148).
//
// Layout from the constructor 0x100c77b0 (the NItemEntry at +0xA0, the zeroed numbers), OnCreate 0x100c63f0 (the
// texture, the tree control, the button), OnDestroy 0x100c65f0 and the readers (OnPaint 0x100c6660, SetRecipe 0x100c6e20).
// The product is an NItemEntry used only for its strings, icon and grade: Name (+0xAC), IconName (+0x4AC), Description
// (+0x7AC), CrystalGrade (+0x19F8), Icon (+0xA0).

#include "nwindow/NCItemWnd.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCButton;
class NCRecipeTreeCtrl;

// NWM_COMMAND ids (names ours)
enum
{
    NID_RECIPETREE_FRAME = 0xFF,   // the title bar (NControl::ID; NCWnd::FrameID is not set here)
    NID_RECIPETREE_CLOSE = 0xFD,
};

class NCRecipeTreeWnd : public NCWnd
{
    NW_DECLARE_CLASS(NCRecipeTreeWnd, NCWnd, 0, 0x101D5D38)

    UTexture* BackTex;               // +0x98 "L2UI.RecipeWnd.RecipeTreeBack" (the constructor leaves it)
    NCRecipeTreeCtrl* TreeCtrl;      // +0x9C the scrolling tree, at (2, 0x52) 0xEC x 0xF6
    NItemEntry Product;              // +0xA0 the recipe's product (name, grade, icon, description)
    INT MPCost;                      // +0x1AC8 "MP cost" (NRecipeData::MPCost)
    INT SuccessRate;                 // +0x1ACC "Success Rate" (percent)
    INT Level;                       // +0x1AD0 "Lv.N"
    NCButton* CloseButton;           // +0x1AD4 id 0xFD, label sysstring 646

    NCRecipeTreeWnd();                                               // 0x100c77b0
    // ~NCRecipeTreeWnd                                              // 0x100c7850

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x100c48a0
    virtual INT OnCreate();                                          // 36 0x100c63f0
    virtual INT OnDestroy();                                         // 37 0x100c65f0
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x100c6660
    // name ours: the Close button: the tree is emptied and the window hidden
    virtual INT OnClose();                                           // 67 0x100c4900

    // names ours
    void Reset();                                                    // 0x100c4930 the tree emptied, the three numbers zeroed
    // 0x100c6e20 (ret 8): shows the recipe RecipeIndex (GetRecipeDataByIndex): the numbers, the product's strings and icon
    // and the tree filled by NCRecipeTreeCtrl 0x100c6af0; the second argument is not read
    void SetRecipe(INT RecipeIndex, INT Unused);
};
NW_ASSERT_OFFSET(NCRecipeTreeWnd, TreeCtrl, 0x9C);
NW_ASSERT_OFFSET(NCRecipeTreeWnd, Product, 0xA0);
NW_ASSERT_OFFSET(NCRecipeTreeWnd, MPCost, 0x1AC8);
NW_ASSERT_OFFSET(NCRecipeTreeWnd, CloseButton, 0x1AD4);
NW_ASSERT_SIZE(NCRecipeTreeWnd, 0x1AD8);

#pragma pack(pop)
