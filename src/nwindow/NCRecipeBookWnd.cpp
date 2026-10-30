/*=============================================================================
    NCRecipeBookWnd.cpp - the recipe book window (new in C2).
    Stock: 0x100c4670 .. 0x100c4f80 (Clear, the trash test, constructor, destroy, paint),
    0x100c6ad0 (AddRecipe), 0x100c7010 (OnCreate).
=============================================================================*/
#include "nwindow/NCRecipeBookWnd.h"
#include "nwindow/NCRecipeBookItemWnd.h"
#include "nwindow/NCFrameCtrl.h"
#include "nwindow/NCGDevice.h"
#include "nwindow/NClasses.gen.h"

enum { NID_RECIPEBOOK_FRAME = 0xFF };  // the title bar's id (name ours)
enum { RECIPE_BOOK_MAX = 50 };         // "(n/50)": the most recipes the book holds
enum { STR_RecipeBook = 649 };         // sysstring: the window's title line (GL2GameData + 0x4D354)

// a child window goes (repeated inline in the stock OnDestroy)
template <class T> static FORCEINLINE void DestroyChild(NCWnd* Parent, T*& Child)
{
    if (Child)
    {
        Parent->RemoveChildWindow(Child->Entry);
        delete Child;
        Child = NULL;
    }
}

// 0x100c4cf0 - the trash icon at (190, 315), 32 x 32
NCRecipeBookWnd::NCRecipeBookWnd()
{
    TrashY = 0;
    TrashX = 0;
    TrashW = 0;
    TrashH = 0;
    Unknown0BC = 0;
    TrashW = 32;
    TrashH = 32;
    TrashX = 190;
    TrashY = 315;
}
NW_HOOK_CTOR(0x100c4cf0, NCRecipeBookWnd)

// 0x100c4d80
NW_HOOK_DTOR(0x100c4d80, NCRecipeBookWnd)

// 0x100c4670 - the grid emptied
void NCRecipeBookWnd::Clear()
{
    Items->Clear();
    Unknown0BC = 0;
}
NW_HOOK_METHOD(0x100c4670, NCRecipeBookWnd, Clear)

// 0x100c4690 - the point (the window's own coordinates plus its screen origin) is on the trash icon, the edges included
UBOOL NCRecipeBookWnd::HitTrash(INT X, INT Y)
{
    INT DX = X - (TrashX + ScreenX);
    INT DY = Y - (TrashY + ScreenY);
    if (DX < 0 || DX > TrashW || DY < 0 || DY > TrashH)
        return 0;
    return 1;
}
NW_HOOK_METHOD(0x100c4690, NCRecipeBookWnd, HitTrash)

// 0x100c6ad0 - what the console forwards: a recipe into the grid
INT NCRecipeBookWnd::AddRecipe(NItemInfo* Info)
{
    if (Items)
        Items->AddItem(Info);
    return 1;
}
NW_HOOK_METHOD(0x100c6ad0, NCRecipeBookWnd, AddRecipe)

// 0x100c4dd0 (vslot 37)
INT NCRecipeBookWnd::OnDestroy()
{
    DestroyChild(this, Items);
    NCWnd::OnDestroy();
    return 1;
}
NW_HOOK_METHOD(0x100c4dd0, NCRecipeBookWnd, OnDestroy)

// 0x100c4e10 (vslot 55) - the background, the trash icon (lit while the drag is over it), the title and the count of
// recipes "(n/50)" right-aligned in orange
INT NCRecipeBookWnd::OnPaint(NCGDevice* Device)
{
    TCHAR Count[32];
    SIZE Size;
    Device->BeginPaint(ScreenX, ScreenY);
    Device->SetClip(ScreenX, ScreenY, Width, Height);
    if (BackTex)
        Device->DrawTexture(0, 21, 240, 344, 0, 0, 240, 344, BackTex);
    Device->DrawTexture(TrashX, TrashY, 32, 32, 0, 0, 32, 32, bTrashActive ? TrashActiveTex : TrashTex);
    Device->DrawText(12, 25, 0xFFFFFFFF, *SysString(STR_RecipeBook), 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
    if (Items)
    {
        appSprintf(Count, TEXT("(%d/%d)"), Items->Items.Num(), RECIPE_BOOK_MAX);
        NCGDevice::GetTextExtent(&Size, Count, 0, 0, 0, 0, 0);
        Device->DrawText((Width - Size.cx) - 10, 25, 0xFFB09B79, Count, 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
    }
    Device->EndPaint();
    Device->ReleaseClip();
    return 1;
}
NW_HOOK_METHOD(0x100c4e10, NCRecipeBookWnd, OnPaint)

// 0x100c7010 (vslot 36) - title bar (id 0xFF, the frame number is not set), the textures and the grid (6 x 7 icons, 160
// recipes at most)
INT NCRecipeBookWnd::OnCreate()
{
    NCWnd::OnCreate();
    FrameCtrl = new NCFrameCtrl(1);
    FrameCtrl->ID = NID_RECIPEBOOK_FRAME;
    FrameCtrl->SetWindowRect(0, 0, 240, 21);
    CreateChild(FrameCtrl, NWS_VISIBLE);

    BackTex = FindTexture(TEXT("L2UI.RecipeWnd.RecipeBookBack"));
    TrashTex = FindTexture(TEXT("L2UI.WindowIcon.DestroyItem"));
    TrashActiveTex = FindTexture(TEXT("L2UI.WindowIcon.DestroyItem_active"));

    Items = new NCRecipeBookItemWnd(6, 7, 160, 32);
    Items->SetWindowRect(16, 50, 223, 244);
    CreateChild(Items, NWS_VISIBLE | 0x20);   // 0x20: meaning unknown
    return 1;
}
NW_HOOK_METHOD(0x100c7010, NCRecipeBookWnd, OnCreate)

NW_CHECK_VTABLE(NCRecipeBookWnd, 0x10124170, 67)
