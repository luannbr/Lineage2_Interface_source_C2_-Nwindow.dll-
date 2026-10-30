#pragma once
// NCRecipeBookWnd - the recipe book (UClass "CRecipeBookWnd", 0xC0; new in C2): the grid of the recipes the
// character knows (NCRecipeBookItemWnd, "(n/50)" in the corner) and a trash icon to forget one. The console fills
// it when the server sends the book (RequestRecipeBookOpen asks for it) through AddRecipe.

#include "nwindow/NCWnd.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCRecipeBookItemWnd;
struct NItemInfo;

class NCRecipeBookWnd : public NCWnd
{
    NW_DECLARE_CLASS(NCRecipeBookWnd, NCWnd, 0, 0x101D6CA0)

    UTexture* BackTex;                // +0x98 "L2UI.RecipeWnd.RecipeBookBack"
    UTexture* TrashTex;               // +0x9C "L2UI.WindowIcon.DestroyItem"
    UTexture* TrashActiveTex;         // +0xA0 "L2UI.WindowIcon.DestroyItem_active" while the drag is over it
    NCRecipeBookItemWnd* Items;       // +0xA4 (name ours) the grid
    INT TrashX;                       // +0xA8 (name ours) the trash icon's place in the window (0xBE)
    INT TrashY;                       // +0xAC (0x13B)
    INT TrashW;                       // +0xB0 (32)
    INT TrashH;                       // +0xB4 (32)
    UBOOL bTrashActive;               // +0xB8 (name ours) the grid's OnMouseMove sets it: the cursor is over the icon
    INT Unknown0BC;                   // +0xBC (name ours) the constructor and Clear zero it, nothing here reads it

    NCRecipeBookWnd();                                               // 0x100c4cf0
    // ~NCRecipeBookWnd                                              // 0x100c4d80

    virtual INT OnCreate();                                          // 36 0x100c7010
    virtual INT OnDestroy();                                         // 37 0x100c4dd0
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x100c4e10

    // names ours (the console calls them: slots 123 and 124)
    void Clear();                                                    // 0x100c4670 the grid emptied
    UBOOL HitTrash(INT X, INT Y);                                    // 0x100c4690 the point is on the trash icon (the
                                                                     //   bounds are inclusive, the window's screen origin added)
    INT AddRecipe(NItemInfo* Info);                                  // 0x100c6ad0 into the grid, always 1
};
NW_ASSERT_OFFSET(NCRecipeBookWnd, BackTex, 0x98);
NW_ASSERT_OFFSET(NCRecipeBookWnd, Items, 0xA4);
NW_ASSERT_OFFSET(NCRecipeBookWnd, bTrashActive, 0xB8);
NW_ASSERT_OFFSET(NCRecipeBookWnd, Unknown0BC, 0xBC);
NW_ASSERT_SIZE(NCRecipeBookWnd, 0xC0);

#pragma pack(pop)
