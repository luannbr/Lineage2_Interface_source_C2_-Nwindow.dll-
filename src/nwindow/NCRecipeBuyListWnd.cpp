/*=============================================================================
    NCRecipeBuyListWnd.cpp - the recipe shop as the customer sees it (new in C2).
    Stock: 0x100c1150 (WindowProc), 0x100c1fd0 .. 0x100c2090 (constructor, destructor, OnDestroy), 0x100c20d0
    (OnPaint), 0x100c2320 (OnSelectRecipe), 0x100c42f0 (OnCreate), the three small bodies 0x100c11b0 / 0x100c11c0 /
    0x100c11d0 (the first one is folded with NCVIPShopWnd's list window).
=============================================================================*/
#include "nwindow/NCRecipeBuyListWnd.h"
#include "nwindow/NCRecipeListBox.h"
#include "nwindow/NCConsole.h"
#include "nwindow/NCFrameCtrl.h"
#include "nwindow/NCGDevice.h"
#include "nwindow/NConsoleWnd.h"
#include "nwindow/NClasses.gen.h"

// sysstring labels (GL2GameData + 0x4B4E8 + 12 * index)
enum
{
    STR_RecipeList = 643,    // 0x4D30C
    STR_CreatorMP = 644,     // 0x4D318
};

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

// 0x100c1fd0
NCRecipeBuyListWnd::NCRecipeBuyListWnd()
{
    RecipeList = NULL;
    BackTex = NULL;
    MP = 0;
    MaxMP = 0;
    Adena = 0;
    ShopOwnerID = -1;
}
NW_HOOK_CTOR(0x100c1fd0, NCRecipeBuyListWnd)

// 0x100c2040
NW_HOOK_DTOR(0x100c2040, NCRecipeBuyListWnd)

// 0x100c1150 (vslot 25) - the list box's click
INT NCRecipeBuyListWnd::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (NCWnd::WindowProc(Msg, wParam, lParam))
        return 1;
    if (Msg == NWM_COMMAND && wParam == NID_RECIPEBUYLIST_LIST)
        return OnSelectRecipe(lParam) != 0;
    return 0;
}
NW_HOOK_METHOD(0x100c1150, NCRecipeBuyListWnd, WindowProc)

// 0x100c11b0 - the list emptied (a body the compiler folded with NCVIPShopWnd's list window: the same code for any window
// whose child at +0x98 is a list; the hook is NCVIPShopItemListWnd::Clear's)
void NCRecipeBuyListWnd::ClearList()
{
    RecipeList->Clear();
}

// 0x100c11c0
void NCRecipeBuyListWnd::SetShopOwner(INT InOwnerID)
{
    ShopOwnerID = InOwnerID;
}
NW_HOOK_METHOD(0x100c11c0, NCRecipeBuyListWnd, SetShopOwner)

// 0x100c3510 - a recipe row into the list box (a tail call into NCRecipeListBox::AddEntry)
void NCRecipeBuyListWnd::AddRecipe(const NRecipeRow* Src)
{
    RecipeList->AddEntry(Src);
}
NW_HOOK_METHOD(0x100c3510, NCRecipeBuyListWnd, AddRecipe)

// 0x100c11d0
void NCRecipeBuyListWnd::SetAdena(INT InAdena)
{
    Adena = InAdena;
}
NW_HOOK_METHOD(0x100c11d0, NCRecipeBuyListWnd, SetAdena)

// 0x100c2090 (vslot 37)
INT NCRecipeBuyListWnd::OnDestroy()
{
    DestroyChild(this, RecipeList);
    NCWnd::OnDestroy();
    return 1;
}
NW_HOOK_METHOD(0x100c2090, NCRecipeBuyListWnd, OnDestroy)

// 0x100c2320 (vslot 67) - the clicked recipe: the server is asked what it takes (the shop owner and the recipe's index)
INT NCRecipeBuyListWnd::OnSelectRecipe(LPARAM Index)
{
    NRecipeRow Row;
    if (RecipeList->GetSelectedEntry(&Row))
        GConsole->RequestRecipeShopMakeInfo(ShopOwnerID, Row.RecipeID);
    return 1;
}
NW_HOOK_METHOD(0x100c2320, NCRecipeBuyListWnd, OnSelectRecipe)

// 0x100c20d0 (vslot 55) - the background, the list's label, "Creator MP" with the MP bar (152 pixels at most), the adena
// icon and the player's adena right-aligned
INT NCRecipeBuyListWnd::OnPaint(NCGDevice* Device)
{
    TCHAR AdenaText[132];
    SIZE Size;
    Device->BeginPaint(ScreenX, ScreenY);
    Device->SetClip(ScreenX, ScreenY, Width, Height);
    if (BackTex)
        Device->DrawTexture(0, 21, 240, 344, 0, 0, 240, 344, BackTex);
    Device->DrawText(15, 25, 0xFFFFFFFF, *SysString(STR_RecipeList), 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
    NCGDevice::GetTextExtent(&Size, *SysString(STR_CreatorMP), 0, 0, 0, 0, 0);
    Device->DrawText(70 - Size.cx, 299, 0xFFFFFFFF, *SysString(STR_CreatorMP), 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
    INT BarW = (INT)((DOUBLE)(MP * 152) / MaxMP);
    if (BarW > 0)
    {
        if (BarW > 152)
            BarW = 152;
        Device->DrawStretchedTex(72, 298, BarW, 10, 0, 0, 1, 10, MPTex);
    }
    if (AdenaTex)
        Device->DrawTexture(52, 313, 16, 12, 0, 0, 16, 12, AdenaTex);
    NFormatThousands(AdenaText, Adena);
    NCGDevice::GetTextExtent(&Size, AdenaText, 0, 0, 0, 0, 0);
    Device->DrawText(222 - Size.cx, 315, 0xFFFFFFFF, AdenaText, 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
    Device->EndPaint();
    Device->ReleaseClip();
    return 1;
}
NW_HOOK_METHOD(0x100c20d0, NCRecipeBuyListWnd, OnPaint)

// 0x100c42f0 (vslot 36) - title bar (id 0xFF, the frame number is not set), the textures and the recipe list (100 rows
// at most, 5 in view; text at 15, 9; rows 5 pixels apart)
INT NCRecipeBuyListWnd::OnCreate()
{
    NCWnd::OnCreate();
    FrameCtrl = new NCFrameCtrl(1);
    FrameCtrl->ID = NID_RECIPEBUYLIST_FRAME;
    FrameCtrl->SetWindowRect(0, 0, 240, 21);
    CreateChild(FrameCtrl, NWS_VISIBLE);

    BackTex = FindTexture(TEXT("L2UI.RecipeWnd.RecipePersonalListBack"));
    MPTex = FindTexture(TEXT("L2UI.PlayerStatusWnd.MP"));
    AdenaTex = FindTexture(TEXT("L2UI.WindowIcon.WindowIconAdena"));

    RecipeList = new NCRecipeListBox(100, 5, 16);
    RecipeList->ID = NID_RECIPEBUYLIST_LIST;
    RecipeList->SetWindowRect(1, 39, 236, 252);
    CreateChild(RecipeList, NWS_VISIBLE);
    RecipeList->TextX = 15;
    RecipeList->TextY = 9;
    RecipeList->LineGap = 5;
    return 1;
}
NW_HOOK_METHOD(0x100c42f0, NCRecipeBuyListWnd, OnCreate)

NW_CHECK_VTABLE(NCRecipeBuyListWnd, 0x10123900, 68)
