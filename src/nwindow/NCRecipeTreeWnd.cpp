/*=============================================================================
    NCRecipeTreeWnd.cpp - "Recipe Information", the ingredient tree of a recipe (C2 only).
    Stock: 0x100c48a0 (WindowProc), 0x100c4900 (the Close button), 0x100c4930 (Reset), 0x100c63f0 .. 0x100c6660 (OnCreate,
    OnDestroy, OnPaint), 0x100c6e20 (SetRecipe), 0x100c77b0 .. 0x100c7850 (constructor, destructor).
=============================================================================*/
#include "nwindow/NCRecipeTreeWnd.h"
#include "nwindow/NCRecipeTreeCtrl.h"
#include "nwindow/NCConsole.h"
#include "nwindow/NCButton.h"
#include "nwindow/NCFrameCtrl.h"
#include "nwindow/NCGDevice.h"
#include "nwindow/NConsoleWnd.h"
#include "nwindow/NCRecipeConsoleOps.h"
#include "nwindow/NRecipeData.h"
#include "nwindow/NClasses.gen.h"

// sysstring labels (the C1 project's F6_Craft.md names the same texts)
enum
{
    STR_MPCost = 320,         // "MP cost"
    STR_SuccessRate = 642,    // "Success Rate"
    STR_Close = 646,
};

/*-----------------------------------------------------------------------------
    NCRecipeTreeWnd
-----------------------------------------------------------------------------*/

// 0x100c77b0
NCRecipeTreeWnd::NCRecipeTreeWnd()
{
    TreeCtrl = NULL;
    CloseButton = NULL;
    MPCost = 0;
    SuccessRate = 0;
    Level = 0;
}
NW_HOOK_CTOR(0x100c77b0, NCRecipeTreeWnd)

// 0x100c7850 - the product's tool tip object goes with it
NW_HOOK_DTOR(0x100c7850, NCRecipeTreeWnd)

// 0x100c48a0 (vslot 25) - the Close button
INT NCRecipeTreeWnd::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (NCWnd::WindowProc(Msg, wParam, lParam))
        return 1;
    if (Msg == NWM_COMMAND && wParam == NID_RECIPETREE_CLOSE)
        return OnClose() != 0;
    return 0;
}
NW_HOOK_METHOD(0x100c48a0, NCRecipeTreeWnd, WindowProc)

// 0x100c4900 (vslot 67) - the tree control emptied, the window hidden
INT NCRecipeTreeWnd::OnClose()
{
    TreeCtrl->Clear();
    HideWindow();
    return 1;
}
NW_HOOK_METHOD(0x100c4900, NCRecipeTreeWnd, OnClose)

// 0x100c4930 - the tree emptied and the numbers zeroed (the recipe windows call it before they fill the tree again)
void NCRecipeTreeWnd::Reset()
{
    TreeCtrl->Clear();
    MPCost = 0;
    SuccessRate = 0;
    Level = 0;
}
NW_HOOK_METHOD(0x100c4930, NCRecipeTreeWnd, Reset)

// 0x100c63f0 (vslot 36) - the title bar (id 0xFF), the background, the tree control and the Close button
INT NCRecipeTreeWnd::OnCreate()
{
    NCWnd::OnCreate();

    FrameCtrl = new NCFrameCtrl(1);
    FrameCtrl->ID = NID_RECIPETREE_FRAME;
    FrameCtrl->SetWindowRect(0, 0, 240, 21);
    CreateChild(FrameCtrl, NWS_VISIBLE);

    BackTex = FindTexture(TEXT("L2UI.RecipeWnd.RecipeTreeBack"));

    NCWnd* Ctrl = new NCRecipeTreeCtrl;   // constructor 0x100c5d70
    TreeCtrl = (NCRecipeTreeCtrl*)Ctrl;
    Ctrl->SetWindowRect(2, 82, 236, 246);
    CreateChild(Ctrl, NWS_VISIBLE);

    CloseButton = new NCButton(TEXT("L2UI.Control.DefaultButton"), TEXT("L2UI.Control.DefaultButton_click"), 0);
    CloseButton->ID = NID_RECIPETREE_CLOSE;
    CloseButton->SetWindowRect(87, 331, 67, 19);
    CreateChild(CloseButton, NWS_VISIBLE);
    CloseButton->SetTextID(STR_Close);
    return 1;
}
NW_HOOK_METHOD(0x100c63f0, NCRecipeTreeWnd, OnCreate)

// 0x100c65f0 (vslot 37) - the tree control and the button go
INT NCRecipeTreeWnd::OnDestroy()
{
    if (TreeCtrl)
    {
        RemoveChildWindow(TreeCtrl->Entry);
        delete TreeCtrl;
        TreeCtrl = NULL;
    }
    if (CloseButton)
    {
        RemoveChildWindow(CloseButton->Entry);
        delete CloseButton;
        CloseButton = NULL;
    }
    NCWnd::OnDestroy();
    return 1;
}
NW_HOOK_METHOD(0x100c65f0, NCRecipeTreeWnd, OnDestroy)

// 0x100c6660 (vslot 55) - the background, the product's icon (16, 35), its name and grade, then the three lines: "MP cost"
// and "Success Rate" as label : value (the label grey, the value tan), "Lv.N" at the right of the second
INT NCRecipeTreeWnd::OnPaint(NCGDevice* Device)
{
    SIZE Size;
    TCHAR Text[64];
    Device->BeginPaint(ScreenX, ScreenY);
    Device->SetClip(ScreenX, ScreenY, Width, Height);
    if (BackTex)
        Device->DrawTexture(0, 21, 240, 344, 0, 0, 240, 344, BackTex);
    if (Product.Icon)
        Device->DrawTexture(16, 35, 32, 32, 0, 0, 32, 32, Product.Icon);

    NCGDevice::GetTextExtent(&Size, Product.Name, 0, 0, 0, 0, 0);
    Device->DrawText(60, 31, 0xFFFFFFFF, Product.Name, 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
    if (GConsole->GetGradeText(Product.CrystalGrade))
    {
        appSprintf(Text, TEXT("`%s`"), GConsole->GetGradeText(Product.CrystalGrade));
        Device->DrawText(Size.cx + 62, 31, 0xFFFFFFFF, Text, 0, 0, 0, L2FONT_Normal, 1, 0, 0, 0, 0, 0);
    }

    NCGDevice::GetTextExtent(&Size, *SysString(STR_MPCost), 0, 0, 0, 0, 0);
    Device->DrawText(60, 45, 0xFFA3A3A3, *SysString(STR_MPCost), 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
    INT SepW = Device->DrawText(Size.cx + 60, 45, 0xFFFFFFFF, TEXT(" : "), 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
    Device->DrawText(Size.cx + 60 + SepW, 45, 0xFFB09B79, appItoa(MPCost), 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);

    NCGDevice::GetTextExtent(&Size, *SysString(STR_SuccessRate), 0, 0, 0, 0, 0);
    Device->DrawText(60, 59, 0xFFA3A3A3, *SysString(STR_SuccessRate), 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
    SepW = Device->DrawText(Size.cx + 60, 59, 0xFFFFFFFF, TEXT(" : "), 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
    appSprintf(Text, TEXT("%d%%"), SuccessRate);
    Device->DrawText(Size.cx + 60 + SepW, 59, 0xFFB09B79, Text, 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);

    appSprintf(Text, TEXT("Lv.%d"), Level);
    Device->DrawText(207, 59, 0xFFFFFFFF, Text, 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
    Device->ReleaseClip();
    Device->EndPaint();
    return 1;
}
NW_HOOK_METHOD(0x100c6660, NCRecipeTreeWnd, OnPaint)

// 0x100c6e20 (ret 8) - the recipe's numbers, its product's name (the item's, with its additional name when it has one:
// NFormatItemName, the console's 0x100570d0), icon, description and grade, and the tree control filled by NCRecipeTreeCtrl's 0x100c6af0 with
// (product, success rate, 0, 0). Nothing happens when the recipe is unknown or its record is empty.
void NCRecipeTreeWnd::SetRecipe(INT RecipeIndex, INT Unused)
{
    NRecipeData* Rec = NGetRecipeByIndex(RecipeIndex);
    if (Rec && Rec->ProductRec)
    {
        MPCost = Rec->MPCost;
        SuccessRate = Rec->SuccessRate;
        Level = Rec->Level;
        appStrcpy(Product.IconName, *Rec->ProductRec->IconName);
        Product.Icon = FindTexture(Product.IconName);
        if (!Product.Icon)
            Product.Icon = FindTexture(TEXT("NWindow.BlackTexture"));
        NFormatItemName(Product.Name, Rec->ProductID);   // the console's: the name with its additional name
        appStrcpy(Product.Description, *NItemRecordDescription(Rec->ProductRec));
        NItemRecordGrade(Rec->ProductRec, &Product.CrystalGrade);
        if (TreeCtrl)
            TreeCtrl->AddRecipe(Rec->ProductID, Rec->SuccessRate, 0, NULL);
    }
}
NW_HOOK_METHOD(0x100c6e20, NCRecipeTreeWnd, SetRecipe)

NW_CHECK_VTABLE(NCRecipeTreeWnd, 0x101248e8, 68)
