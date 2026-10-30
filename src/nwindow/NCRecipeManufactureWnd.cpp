/*=============================================================================
    NCRecipeManufactureWnd.cpp - the two "Item Creation" windows (C2 only): NCRecipeManufactureWnd (the player's own
    recipe) and NCRecipeBuyManufactureWnd (an offer of a recipe shop). The stock code is two copies of one source with
    the fields shifted (the buy window has two more, +0x1AE8 and +0x1AEC), so OnCreate, OnDestroy, OnPaint, SetRecipe and the
    tree button are written once as templates over the window type and each class' hook points at its own copy:
        NCRecipeManufactureWnd     0x100c4700 .. 0x100c4810, 0x100c5150, 0x100c5240, 0x100c7260 .. 0x100c7390, 0x100c7dc0, 0x100c7de0
        NCRecipeBuyManufactureWnd  0x100c11e0 .. 0x100c13b0, 0x100c23c0 .. 0x100c28d0, 0x100c3520, 0x100c44a0 .. 0x100c4580
    0x100c1310 (the tree button) is one function the two vtables share; it is hooked on the buy window and the other
    class' override calls the same code. 0x100c7dc0 (AddIngredient) is shared the same way, hooked on the manufacture window.
=============================================================================*/
#include "nwindow/NCRecipeManufactureWnd.h"
#include "nwindow/NCRecipeBuyManufactureWnd.h"
#include "nwindow/NCRecipeTreeWnd.h"
#include "nwindow/NCConsole.h"
#include "nwindow/NCButton.h"
#include "nwindow/NCFrameCtrl.h"
#include "nwindow/NCGDevice.h"
#include "nwindow/NCInventoryWnd.h"
#include "nwindow/NCInvenItemWnd.h"
#include "nwindow/NConsoleWnd.h"
#include "nwindow/NCRecipeConsoleOps.h"
#include "nwindow/NClasses.gen.h"

// sysstring labels (the C1 project's F6_Craft.md names the same texts)
enum
{
    STR_MPCost = 320,            // "MP cost"
    STR_Creator_MP = 644,        // "Creator MP"
    STR_Create = 645,
    STR_Close = 646,
    STR_ResultCount = 647,       // "Number of Resulting Items"
    STR_RetainedCount = 648,     // "Number of Retained Items"
    STR_SuccessRate = 642,       // "Success Rate"
    STR_Ingredients = 742,       // "Ingredient(s)"
    STR_Results = 743,           // "Result(s)"
    STR_Prev = 543,              // "<PREV"
};
// systemmsg lines of the outcome (name ours): "$s1 manufacturing failure." / "$s1 (*$s2) manufacturing success."
enum { MSG_MakeFailure = 960, MSG_MakeSuccess = 959 };

// the desktop's windows the recipe windows talk to (NCConsole.h keeps them untyped, names ours): +0x138 the tree window
static NCRecipeTreeWnd* RecipeTreeWnd()
{
    return (NCRecipeTreeWnd*)GDesktop->NewWnd138;
}

// a child window goes (repeated inline in the stock OnDestroys)
template <class T> static FORCEINLINE void DestroyChild(NCWnd* Parent, T*& Child)
{
    if (Child)
    {
        Parent->RemoveChildWindow(Child->Entry);
        delete Child;
        Child = NULL;
    }
}

// the player's count of an item: the stacks of Grid whose id is ItemID added to Entry's Count
static FORCEINLINE void AddOwned(NCItemWnd* Grid, NItemEntry* Entry)
{
    for (INT i = 0; i < Grid->Items.Num(); i++)
        if (Entry->ID == Grid->Items(i)->ID)
            Entry->Count += Grid->Items(i)->Count;
}

/*-----------------------------------------------------------------------------
    The code the two windows share, over the window type T (the same field names in both).
-----------------------------------------------------------------------------*/

// title bar (id 0xFF), the background and the MP bar's texture, the ingredient grid, the tree button, Create, <PREV and Close
template <class T> static FORCEINLINE INT MakingOnCreate(T* W, INT CloseID)
{
    W->NCWnd::OnCreate();

    W->FrameCtrl = new NCFrameCtrl(1);
    W->FrameCtrl->ID = NID_RECIPEMAKE_FRAME;
    W->FrameCtrl->SetWindowRect(0, 0, 240, 21);
    W->CreateChild(W->FrameCtrl, NWS_VISIBLE);

    W->BackTex = W->FindTexture(TEXT("L2UI.RecipeWnd.RecipeMakingBack"));
    W->MPTex = W->FindTexture(TEXT("L2UI.PlayerStatusWnd.MP"));

    W->IngredientWnd = new NCRecipeManufactureItemWnd(6, 3, 160, 32);
    W->IngredientWnd->SetWindowRect(16, 128, 223, 104);
    W->CreateChild(W->IngredientWnd, NWS_VISIBLE | NWS_UNKNOWN20);

    W->TreeButton = new NCButton(TEXT("L2UI.RecipeWnd.RecipeTreeBtn"), TEXT("L2UI.RecipeWnd.RecipeTreeBtn_click"), 0);
    W->TreeButton->ID = NID_RECIPEMAKE_TREE;
    W->TreeButton->SetWindowRect(200, 43, 32, 32);
    W->CreateChild(W->TreeButton, NWS_VISIBLE | NWS_UNKNOWN20);

    W->MakeButton = new NCButton(TEXT("L2UI.Control.DefaultButton"), TEXT("L2UI.Control.DefaultButton_click"), 0);
    W->MakeButton->ID = NID_RECIPEMAKE_CREATE;
    W->MakeButton->SetWindowRect(10, 335, 67, 19);
    W->CreateChild(W->MakeButton, NWS_VISIBLE | NWS_UNKNOWN20);
    W->MakeButton->SetTextID(STR_Create);

    W->PrevButton = new NCButton(TEXT("L2UI.Control.DefaultButton"), TEXT("L2UI.Control.DefaultButton_click"), 0);
    W->PrevButton->ID = NID_RECIPEMAKE_PREV;
    W->PrevButton->SetWindowRect(87, 335, 67, 19);
    W->CreateChild(W->PrevButton, NWS_VISIBLE | NWS_UNKNOWN20);
    W->PrevButton->SetTextID(STR_Prev);

    W->CloseButton = new NCButton(TEXT("L2UI.Control.DefaultButton"), TEXT("L2UI.Control.DefaultButton_click"), 0);
    W->CloseButton->ID = CloseID;
    W->CloseButton->SetWindowRect(164, 335, 67, 19);
    W->CreateChild(W->CloseButton, NWS_VISIBLE | NWS_UNKNOWN20);
    W->CloseButton->SetTextID(STR_Close);
    return 1;
}

// the grid and the four buttons go (the title bar is NCWnd::OnDestroy's)
template <class T> static FORCEINLINE INT MakingOnDestroy(T* W)
{
    DestroyChild(W, W->IngredientWnd);
    DestroyChild(W, W->TreeButton);
    DestroyChild(W, W->MakeButton);
    DestroyChild(W, W->PrevButton);
    DestroyChild(W, W->CloseButton);
    W->NCWnd::OnDestroy();
    return 1;
}

// the product's name, grade and numbers, the ingredient and result captions, the outcome line, the count of the product the
// player keeps and the creator's MP bar. Result is the window's outcome field (0 failed, 1 succeeded, else no line).
// NOTE the stock count loop: the second inventory page is searched by id but the count added is read from the FIRST page's
// entry at the same index (kept as it is).
template <class T> static FORCEINLINE INT MakingOnPaint(T* W, NCGDevice* Device, INT Result)
{
    SIZE Size;
    TCHAR Text[64];
    TCHAR Line[264];
    Device->BeginPaint(W->ScreenX, W->ScreenY);
    Device->SetClip(W->ScreenX, W->ScreenY, W->Width, W->Height);
    if (W->BackTex)
        Device->DrawTexture(0, 21, 240, 344, 0, 0, 240, 344, W->BackTex);
    if (W->Product.Icon)
        Device->DrawTexture(16, 43, 32, 32, 0, 0, 32, 32, W->Product.Icon);

    NCGDevice::GetTextExtent(&Size, W->Product.Name, 0, 0, 0, 0, 0);
    Device->DrawText(60, 32, 0xFFFFFFFF, W->Product.Name, 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
    if (GConsole->GetGradeText(W->Product.CrystalGrade))
    {
        appSprintf(Text, TEXT("`%s`"), GConsole->GetGradeText(W->Product.CrystalGrade));
        Device->DrawText(Size.cx + 62, 32, 0xFFFFFFFF, Text, 0, 0, 0, L2FONT_Normal, 1, 0, 0, 0, 0, 0);
    }

    // "MP cost : N"
    NCGDevice::GetTextExtent(&Size, *NCObject::SysString(STR_MPCost), 0, 0, 0, 0, 0);
    Device->DrawText(60, 46, 0xFFA3A3A3, *NCObject::SysString(STR_MPCost), 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
    INT SepW = Device->DrawText(Size.cx + 60, 46, 0xFFFFFFFF, TEXT(" : "), 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
    Device->DrawText(Size.cx + 60 + SepW, 46, 0xFFB09B79, appItoa(W->MPCost), 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);

    // "Success Rate : N%"
    NCGDevice::GetTextExtent(&Size, *NCObject::SysString(STR_SuccessRate), 0, 0, 0, 0, 0);
    Device->DrawText(60, 60, 0xFFA3A3A3, *NCObject::SysString(STR_SuccessRate), 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
    appSprintf(Text, TEXT("%d%%"), W->SuccessRate);
    SepW = Device->DrawText(Size.cx + 60, 60, 0xFFFFFFFF, TEXT(" : "), 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
    Device->DrawText(Size.cx + 60 + SepW, 60, 0xFFB09B79, Text, 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);

    // "Number of Resulting Items : N"
    NCGDevice::GetTextExtent(&Size, *NCObject::SysString(STR_ResultCount), 0, 0, 0, 0, 0);
    Device->DrawText(60, 74, 0xFFA3A3A3, *NCObject::SysString(STR_ResultCount), 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
    SepW = Device->DrawText(Size.cx + 60, 74, 0xFFFFFFFF, TEXT(" : "), 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
    Device->DrawText(Size.cx + 60 + SepW, 74, 0xFFB09B79, appItoa(W->ProductCount), 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);

    // the captions of the grid and of the result line
    Device->DrawText(15, 103, 0xFFFFFFFF, *NCObject::SysString(STR_Ingredients), 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
    Device->DrawText(15, 268, 0xFFFFFFFF, *NCObject::SysString(STR_Results), 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);

    // the outcome of the last try (the product's name, and the count on a success)
    if (Result == 0)
    {
        GConsole->FormatSystemMessage(Line, *GL2GameData.SystemMsg[MSG_MakeFailure].Message, W->Product.Name, NULL);
        Device->DrawText(15, 310, 0xFFB2BECF, Line, 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
    }
    else if (Result == 1)
    {
        const TCHAR* Count = appItoa(W->ProductCount);
        GConsole->FormatSystemMessage(Line, *GL2GameData.SystemMsg[MSG_MakeSuccess].Message, W->Product.Name, Count);
        Device->DrawText(15, 310, 0xFFB2BECF, Line, 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
    }

    // "Number of Retained Items : N": the player's stacks of the product, in the two inventory pages
    NCGDevice::GetTextExtent(&Size, *NCObject::SysString(STR_RetainedCount), 0, 0, 0, 0, 0);
    Device->DrawText(15, 292, 0xFFA3A3A3, *NCObject::SysString(STR_RetainedCount), 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
    NCInventoryWnd* Inventory = GDesktop->InventoryWnd;
    NCInvenItemWnd* Page0 = Inventory->ItemWnd[0];
    NCInvenItemWnd* Page1 = Inventory->ItemWnd[1];
    INT Total = 0;
    if (Page0)
    {
        for (INT i = 0; i < Page0->Items.Num(); i++)
            if (W->ProductID == Page0->Items(i)->ID)
                Total += Page0->Items(i)->Count;
    }
    if (Page1)
    {
        for (INT i = 0; i < Page1->Items.Num(); i++)
            if (W->ProductID == Page1->Items(i)->ID)
                Total += Page0->Items(i)->Count;
    }
    SepW = Device->DrawText(Size.cx + 17, 292, 0xFFFFFFFF, TEXT(" : "), 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
    Device->DrawText(Size.cx + 17 + SepW, 292, 0xFFB09B79, appItoa(Total), 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);

    // "Creator MP" and its bar: CreatorMP / CreatorMaxMP of 152 pixels
    Device->DrawText(15, 241, 0xFFFFFFFF, *NCObject::SysString(STR_Creator_MP), 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
    INT BarW = (INT)((DOUBLE)(W->CreatorMP * 152) / W->CreatorMaxMP);
    if (BarW > 0)
    {
        if (BarW > 152)
            BarW = 152;
        Device->DrawStretchedTex(72, 242, BarW, 10, 0, 0, 1, 10, W->MPTex);
    }
    Device->ReleaseClip();
    Device->EndPaint();
    return 1;
}

// the recipe's numbers, the product's strings, icon and grade, then an entry per ingredient (the item's own data by the
// console's MakeItemInfo, the player's count of it added from both inventory pages) into the grid. The entries made here
// are copied by the grid and never freed (as in the stock code).
template <class T> static FORCEINLINE void MakingSetRecipe(T* W, NRecipeData* Rec)
{
    if (Rec && Rec->ProductRec)
    {
        W->RecipeID = Rec->RecipeID;
        W->ProductID = Rec->ProductID;
        W->ProductCount = Rec->ProductCount;
        W->MPCost = Rec->MPCost;
        W->SuccessRate = Rec->SuccessRate;
        NViewItemData* Item = NRecipeFindItem(&Rec->ProductID);
        if (Item)
        {
            appStrcpy(W->Product.Name, *Item->Name);
            appStrcpy(W->Product.Description, *NItemRecordDescription(Item));
            appStrcpy(W->Product.IconName, *Item->IconName);
            W->Product.Icon = W->FindTexture(W->Product.IconName);
            if (!W->Product.Icon)
                W->Product.Icon = W->FindTexture(TEXT("NWindow.BlackTexture"));
            NItemRecordGrade(Item, &W->Product.CrystalGrade);
        }
        for (INT i = 0; i < Rec->Materials.Num(); i++)
        {
            NRecipeMaterial* Material = Rec->Materials(i);
            INT ID = Material->ItemID;
            INT Need = Material->Count;
            NItemEntry* Entry = new NItemEntry;
            Entry->ID = ID;
            Entry->UnknownC34 = Need;
            NMakeItemInfo(Entry);   // the console's MakeItemInfo: the entry's strings from the item table
            Entry->Icon = W->FindTexture(Entry->IconName);
            if (!Entry->Icon)
                Entry->Icon = W->FindTexture(TEXT("NWindow.BlackTexture"));
            NCInventoryWnd* Inventory = GDesktop->InventoryWnd;
            NCInvenItemWnd* Page0 = Inventory->ItemWnd[0];
            NCInvenItemWnd* Page1 = Inventory->ItemWnd[1];
            if (Page0)
                AddOwned(Page0, Entry);
            if (Page1)
                AddOwned(Page1, Entry);
            W->AddIngredient(Entry);
        }
    }
}

// 0x100c1310 - the tree button: the tree window hidden when it is shown, else filled with this recipe (its id and the success
// rate) and shown with the focus
template <class T> static FORCEINLINE INT MakingOnShowTree(T* W)
{
    if (!RecipeTreeWnd()->IsShowWindow())
    {
        RecipeTreeWnd()->Reset();
        RecipeTreeWnd()->SetRecipe(W->RecipeID, W->SuccessRate);
        RecipeTreeWnd()->ShowWindow();
        RecipeTreeWnd()->SetFocus();
        return 1;
    }
    RecipeTreeWnd()->HideWindow();
    return 1;
}

/*-----------------------------------------------------------------------------
    NCRecipeManufactureWnd
-----------------------------------------------------------------------------*/

// 0x100c7260 - nothing shown yet (Result -1)
NCRecipeManufactureWnd::NCRecipeManufactureWnd()
{
    IngredientWnd = NULL;
    TreeButton = NULL;
    MakeButton = NULL;
    PrevButton = NULL;
    CloseButton = NULL;
    RecipeID = 0;
    ProductID = 0;
    ProductCount = 0;
    MPCost = 0;
    SuccessRate = 0;
    CreatorMP = 0;
    CreatorMaxMP = 0;
    Result = -1;
}
NW_HOOK_CTOR(0x100c7260, NCRecipeManufactureWnd)

// 0x100c7330 - the product's tool tip object goes with it
NW_HOOK_DTOR(0x100c7330, NCRecipeManufactureWnd)

// 0x100c4700 (vslot 25) - the four buttons
INT NCRecipeManufactureWnd::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (NCWnd::WindowProc(Msg, wParam, lParam))
        return 1;
    if (Msg == NWM_COMMAND)
    {
        if (wParam == NID_RECIPEMAKE_CLOSE)
            return OnClose() != 0;
        if (wParam == NID_RECIPEMAKE_PREV)
            return OnPrev() != 0;
        if (wParam == NID_RECIPEMAKE_TREE)
            return OnShowTree() != 0;
        if (wParam == NID_RECIPEMAKE_CREATE)
            return OnMake() != 0;
    }
    return 0;
}
NW_HOOK_METHOD(0x100c4700, NCRecipeManufactureWnd, WindowProc)

// 0x100c7390 (vslot 36)
INT NCRecipeManufactureWnd::OnCreate()
{
    return MakingOnCreate(this, NID_RECIPEMAKE_CLOSE);
}
NW_HOOK_METHOD(0x100c7390, NCRecipeManufactureWnd, OnCreate)

// 0x100c5150 (vslot 37)
INT NCRecipeManufactureWnd::OnDestroy()
{
    return MakingOnDestroy(this);
}
NW_HOOK_METHOD(0x100c5150, NCRecipeManufactureWnd, OnDestroy)

// 0x100c5240 (vslot 55) - the outcome field is +0x1AE8
INT NCRecipeManufactureWnd::OnPaint(NCGDevice* Device)
{
    return MakingOnPaint(this, Device, Result);
}
NW_HOOK_METHOD(0x100c5240, NCRecipeManufactureWnd, OnPaint)

// 0x100c47f0 (vslot 67) - <PREV: the window closes and the recipe book is asked for again
INT NCRecipeManufactureWnd::OnPrev()
{
    HideWindow();
    GConsole->RequestRecipeBookOpen();
    return 1;
}
NW_HOOK_METHOD(0x100c47f0, NCRecipeManufactureWnd, OnPrev)

// vslot 68 - Close: stock body shared with NCOptionWnd::OnCancel (0x100a0830)
INT NCRecipeManufactureWnd::OnClose()
{
    HideWindow();
    return 1;
}

// vslot 69 - the tree button: 0x100c1310, the body the two recipe making windows share (hooked on the buy window)
INT NCRecipeManufactureWnd::OnShowTree()
{
    return MakingOnShowTree(this);
}

// 0x100c4810 (vslot 70) - Create: the server makes the recipe for the player
INT NCRecipeManufactureWnd::OnMake()
{
    GConsole->RequestRecipeItemMakeSelf(RecipeID);
    return 1;
}
NW_HOOK_METHOD(0x100c4810, NCRecipeManufactureWnd, OnMake)

// 0x100c47b0 - the numbers zeroed (not the outcome field, nor MP) and the grid emptied (NCItemWnd::Clear, vslot 70)
void NCRecipeManufactureWnd::Reset()
{
    RecipeID = 0;
    ProductID = 0;
    ProductCount = 0;
    MPCost = 0;
    SuccessRate = 0;
    CreatorMP = 0;
    CreatorMaxMP = 0;
    IngredientWnd->Clear();
}
NW_HOOK_METHOD(0x100c47b0, NCRecipeManufactureWnd, Reset)

// 0x100c7de0 (ret 4)
void NCRecipeManufactureWnd::SetRecipe(NRecipeData* Rec)
{
    MakingSetRecipe(this, Rec);
}
NW_HOOK_METHOD(0x100c7de0, NCRecipeManufactureWnd, SetRecipe)

// 0x100c7dc0 (ret 4) - the entry to the grid (shared with the buy window); always 1
UBOOL NCRecipeManufactureWnd::AddIngredient(NItemEntry* Item)
{
    if (IngredientWnd)
        IngredientWnd->AddIngredient(Item);
    return 1;
}
NW_HOOK_METHOD(0x100c7dc0, NCRecipeManufactureWnd, AddIngredient)

NW_CHECK_VTABLE(NCRecipeManufactureWnd, 0x101247c8, 71)

/*-----------------------------------------------------------------------------
    NCRecipeBuyManufactureWnd
-----------------------------------------------------------------------------*/

// 0x100c44a0 - nothing shown yet (Result -1)
NCRecipeBuyManufactureWnd::NCRecipeBuyManufactureWnd()
{
    IngredientWnd = NULL;
    TreeButton = NULL;
    MakeButton = NULL;
    PrevButton = NULL;
    CloseButton = NULL;
    RecipeID = 0;
    ProductID = 0;
    ProductCount = 0;
    MPCost = 0;
    SuccessRate = 0;
    CreatorMP = 0;
    CreatorMaxMP = 0;
    ShopID = 0;
    Unknown1AEC = 0;
    Result = -1;
}
NW_HOOK_CTOR(0x100c44a0, NCRecipeBuyManufactureWnd)

// 0x100c4580
NW_HOOK_DTOR(0x100c4580, NCRecipeBuyManufactureWnd)

// 0x100c11e0 (vslot 25) - the four buttons (Close is 0xF8 here)
INT NCRecipeBuyManufactureWnd::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (NCWnd::WindowProc(Msg, wParam, lParam))
        return 1;
    if (Msg == NWM_COMMAND)
    {
        if (wParam == NID_RECIPEBUYMAKE_CLOSE)
            return OnClose() != 0;
        if (wParam == NID_RECIPEMAKE_PREV)
            return OnPrev() != 0;
        if (wParam == NID_RECIPEMAKE_TREE)
            return OnShowTree() != 0;
        if (wParam == NID_RECIPEMAKE_CREATE)
            return OnMake() != 0;
    }
    return 0;
}
NW_HOOK_METHOD(0x100c11e0, NCRecipeBuyManufactureWnd, WindowProc)

// 0x100c23c0 (vslot 36)
INT NCRecipeBuyManufactureWnd::OnCreate()
{
    return MakingOnCreate(this, NID_RECIPEBUYMAKE_CLOSE);
}
NW_HOOK_METHOD(0x100c23c0, NCRecipeBuyManufactureWnd, OnCreate)

// 0x100c27e0 (vslot 37)
INT NCRecipeBuyManufactureWnd::OnDestroy()
{
    return MakingOnDestroy(this);
}
NW_HOOK_METHOD(0x100c27e0, NCRecipeBuyManufactureWnd, OnDestroy)

// 0x100c28d0 (vslot 55) - the outcome field is +0x1AF0
INT NCRecipeBuyManufactureWnd::OnPaint(NCGDevice* Device)
{
    return MakingOnPaint(this, Device, Result);
}
NW_HOOK_METHOD(0x100c28d0, NCRecipeBuyManufactureWnd, OnPaint)

// 0x100c12e0 (vslot 67) - <PREV: the window closes and the shop's list is asked for again
INT NCRecipeBuyManufactureWnd::OnPrev()
{
    HideWindow();
    GConsole->RequestRecipeShopSellList(ShopID);
    return 1;
}
NW_HOOK_METHOD(0x100c12e0, NCRecipeBuyManufactureWnd, OnPrev)

// vslot 68 - Close: stock body shared with NCOptionWnd::OnCancel (0x100a0830)
INT NCRecipeBuyManufactureWnd::OnClose()
{
    HideWindow();
    return 1;
}

// 0x100c1310 (vslot 69) - the tree button (the body both windows' vtables point at)
INT NCRecipeBuyManufactureWnd::OnShowTree()
{
    return MakingOnShowTree(this);
}
NW_HOOK_METHOD(0x100c1310, NCRecipeBuyManufactureWnd, OnShowTree)

// 0x100c13b0 (vslot 70) - Create: the shop's owner makes the recipe for the customer
INT NCRecipeBuyManufactureWnd::OnMake()
{
    GConsole->RequestRecipeShopMakeDo(ShopID, RecipeID);
    return 1;
}
NW_HOOK_METHOD(0x100c13b0, NCRecipeBuyManufactureWnd, OnMake)

// 0x100c1290 - the numbers and the shop zeroed (not the outcome field) and the grid emptied
void NCRecipeBuyManufactureWnd::Reset()
{
    RecipeID = 0;
    ProductID = 0;
    ProductCount = 0;
    MPCost = 0;
    SuccessRate = 0;
    CreatorMP = 0;
    CreatorMaxMP = 0;
    ShopID = 0;
    Unknown1AEC = 0;
    IngredientWnd->Clear();
}
NW_HOOK_METHOD(0x100c1290, NCRecipeBuyManufactureWnd, Reset)

// 0x100c3520 (ret 4)
void NCRecipeBuyManufactureWnd::SetRecipe(NRecipeData* Rec)
{
    MakingSetRecipe(this, Rec);
}
NW_HOOK_METHOD(0x100c3520, NCRecipeBuyManufactureWnd, SetRecipe)

// 0x100c7dc0 is NCRecipeManufactureWnd::AddIngredient's body (hooked there, one function for both windows)
UBOOL NCRecipeBuyManufactureWnd::AddIngredient(NItemEntry* Item)
{
    if (IngredientWnd)
        IngredientWnd->AddIngredient(Item);
    return 1;
}

NW_CHECK_VTABLE(NCRecipeBuyManufactureWnd, 0x10123e38, 71)
