/*=============================================================================
    NCRecipeTreeCtrl.cpp - the scrolling frame of the recipe tree (C2 only).
    Stock: 0x100c5d70 .. 0x100c5dc0 (constructor, destructor), 0x100c5e10 .. 0x100c6240 (the overrides), 0x100c62c0
    (OnMouseWheel), 0x100c6af0 (the tree fill).
    The constructor calls NCTreeCtrl's (0x1002a9a0, w2newui's): it is hooked only when that group's header is in the tree
    (NW_HAVE_NCTREECTRL_H, see NCRecipeTreeCtrl.h); until then the stock constructor runs (NCRecipeTreeWnd::OnCreate makes the
    control through it: StockCall).
=============================================================================*/
#include "nwindow/NCRecipeTreeCtrl.h"
#include "nwindow/NCConsole.h"
#include "nwindow/NCGDevice.h"
#include "nwindow/NCInventoryWnd.h"
#include "nwindow/NCInvenItemWnd.h"
#include "nwindow/NCSliderBar.h"
#include "nwindow/NRecipeData.h"
#include "nwindow/NClasses.gen.h"

// NWM id of the tree control (names ours)
enum { NID_RECIPETREE_TREE = 0xA00 };
// the node width is the recipe tree's: a scroll bar of 16 pixels at the right of the frame
enum { RECIPETREE_SCROLLBAR_W = 16 };
// the node icon size and the sure recipe rate of the ingredients (names ours)
enum { RECIPETREE_ICON = 32, RECIPE_RATE_SURE = 100 };

// what the recipe code reads of an entry's data (see NCRecipeTree.cpp)
static FORCEINLINE INT& DataNeed(NTVItemData& Data) { return Data.Unknown08; }
static FORCEINLINE INT& DataHave(NTVItemData& Data) { return Data.Unknown0C; }

/*-----------------------------------------------------------------------------
    NCRecipeTreeCtrl
-----------------------------------------------------------------------------*/

#ifdef NW_HAVE_NCTREECTRL_H
// 0x100c5d70 - NCTreeCtrl's constructor, the vtable and the scroll window's +0xCC set to 16 (again)
NCRecipeTreeCtrl::NCRecipeTreeCtrl()
{
    Unknown0C8 = 16;
}
NW_HOOK_CTOR(0x100c5d70, NCRecipeTreeCtrl)
#endif

// 0x100c5dc0
NW_HOOK_DTOR(0x100c5dc0, NCRecipeTreeCtrl)

// 0x100c5e10 (vslot 36) - the tree, the frame's width less the slider's, its height
INT NCRecipeTreeCtrl::OnCreate()
{
    NCScrollWnd::OnCreate();
    NCWnd* Ctrl = new NCRecipeTree;   // constructor 0x100c5960
    RecipeTree = (NCRecipeTree*)Ctrl;
    RecipeTree->ID = NID_RECIPETREE_TREE;
    RecipeTree->SetWindowRect(0, 0, Width - RECIPETREE_SCROLLBAR_W, Height);
    CreateChild(RecipeTree, NWS_VISIBLE);
    return 1;
}
NW_HOOK_METHOD(0x100c5e10, NCRecipeTreeCtrl, OnCreate)

// 0x100c5ed0 (vslot 37) - the tree goes
INT NCRecipeTreeCtrl::OnDestroy()
{
    if (RecipeTree)
    {
        RemoveChildWindow(RecipeTree->Entry);
        delete RecipeTree;
        RecipeTree = NULL;
    }
    NCScrollWnd::OnDestroy();
    return 1;
}
NW_HOOK_METHOD(0x100c5ed0, NCRecipeTreeCtrl, OnDestroy)

// 0x100c5f10 (vslot 55) - the scroll range follows the tree (its measured height and a margin of 21) and the nodes are
// shown or hidden by where they are
INT NCRecipeTreeCtrl::OnPaint(NCGDevice* Device)
{
    Device->BeginPaint(ScreenX, ScreenY);
    Device->SetClip(ScreenX, ScreenY, Width, Height);
    SetContentHeight(RecipeTree->ContentHeight + 21);
    if (RecipeTree->Root)
        ShowVisible(RecipeTree->Root);
    Device->ReleaseClip();
    Device->EndPaint();
    return 1;
}
NW_HOOK_METHOD(0x100c5f10, NCRecipeTreeCtrl, OnPaint)

// 0x100c62c0 (vslot 66) - NCScrollWnd::OnMouseWheel (three lines a notch), and the tree window moved the same way
INT NCRecipeTreeCtrl::OnMouseWheel(WPARAM wParam, LPARAM lParam)
{
    SWORD Notches = (SWORD)((SWORD)HIWORD(wParam) / 120);
    DeltaX = ScrollX;
    DeltaY = ScrollY;
    if (Notches > 0)
    {
        if (ScrollY < 0)
        {
            ScrollY += LineStep * Notches * 3;
            if (ScrollY > 0)
                ScrollY = 0;
            ScrollBar->SetScrollPos(-ScrollY);
        }
    }
    else if (ScrollY > MinScrollY)
    {
        ScrollY += LineStep * Notches * 3;
        if (ScrollY < MinScrollY)
            ScrollY = MinScrollY;
        ScrollBar->SetScrollPos(-ScrollY);
    }
    DeltaX -= ScrollX;
    DeltaY -= ScrollY;
    RecipeTree->MoveWindow(0, -DeltaY);
    RecipeTree->ScrollOffset += DeltaY;
    return 1;
}
NW_HOOK_METHOD(0x100c62c0, NCRecipeTreeCtrl, OnMouseWheel)

// 0x100c60b0 (vslot 67) - a line up: the tree window goes down by the same line
INT NCRecipeTreeCtrl::ScrollUp()
{
    if (ScrollY < 0)
    {
        DeltaX = ScrollX;
        DeltaY = ScrollY;
        ScrollY += LineStep;
        if (ScrollY > 0)
            ScrollY = 0;
        ScrollBar->SetScrollPos(-ScrollY);
        DeltaX -= ScrollX;
        DeltaY -= ScrollY;
        RecipeTree->MoveWindow(0, LineStep);
        RecipeTree->ScrollOffset -= LineStep;
    }
    return 1;
}
NW_HOOK_METHOD(0x100c60b0, NCRecipeTreeCtrl, ScrollUp)

// 0x100c6170 (vslot 68) - a line down
INT NCRecipeTreeCtrl::ScrollDown()
{
    if (ScrollY > MinScrollY)
    {
        DeltaX = ScrollX;
        DeltaY = ScrollY;
        ScrollY -= LineStep;
        if (ScrollY < MinScrollY)
            ScrollY = MinScrollY;
        ScrollBar->SetScrollPos(-ScrollY);
        DeltaX -= ScrollX;
        DeltaY -= ScrollY;
        RecipeTree->MoveWindow(0, -LineStep);
        RecipeTree->ScrollOffset += LineStep;
    }
    return 1;
}
NW_HOOK_METHOD(0x100c6170, NCRecipeTreeCtrl, ScrollDown)

// 0x100c6240 (vslot 69) - the slider's value is the scroll position
INT NCRecipeTreeCtrl::OnScrollBar(INT Value)
{
    DeltaX = ScrollX;
    DeltaY = ScrollY;
    ScrollY = -Value;
    DeltaX -= ScrollX;
    DeltaY -= ScrollY;
    RecipeTree->MoveWindow(0, -DeltaY);
    RecipeTree->ScrollOffset += DeltaY;
    return 1;
}
NW_HOOK_METHOD(0x100c6240, NCRecipeTreeCtrl, OnScrollBar)

// 0x100c5fa0 (vslot 70) - the tree emptied and put back at the frame's origin, the scroll state zeroed (the slider's thumb too)
void NCRecipeTreeCtrl::Clear()
{
    RecipeTree->Clear();
    RecipeTree->MoveWindow(-RecipeTree->LocalX, -RecipeTree->LocalY);
    RecipeTree->ScrollOffset = 0;
    ScrollY = 0;
    ScrollX = 0;
    DeltaY = 0;
    DeltaX = 0;
    MinScrollY = 0;
    ScrollBar->ThumbPos = 0;
}
NW_HOOK_METHOD(0x100c5fa0, NCRecipeTreeCtrl, Clear)

// 0x100c6010 (vslot 71, ret 4) - an entry is shown when its place (kept in +0x1A1C / +0x1A20 by NCRecipeTree::DrawItem, moved by the
// scroll) is inside the tree window's size, else hidden; then the same for its children
void NCRecipeTreeCtrl::ShowVisible(NCTVItem* Item)
{
    if (Item)
    {
        INT ItemX = Item->CopyW + ScrollX;
        INT ItemY = Item->CopyH + ScrollY;
        if (ItemX < 0 || ItemX > RecipeTree->LocalW || ItemY < 0 || ItemY > RecipeTree->LocalH)
            Item->HideWindow();
        else
            Item->ShowWindow();
        for (INT i = 0; i < Item->Children.Num(); i++)
            ShowVisible(Item->Children(i));
    }
}
NW_HOOK_METHOD(0x100c6010, NCRecipeTreeCtrl, ShowVisible)

// 0x100c6af0 (ret 0x10) - see the header. The node info is 0x1828 bytes of which the stock frame fills the first 0x1820: the
// last two dwords go with whatever the frame held (the node copies all 0x1828).
void NCRecipeTreeCtrl::AddRecipe(INT ItemID, INT SuccessRate, INT Need, NCTVItem* Parent)
{
    NRecipeData* Rec = NGetRecipeBy2Condition(ItemID, SuccessRate);
    NViewItemData* Item;
    if (Rec && Rec->ProductRec)
        Item = Rec->ProductRec;
    else
        Item = NRecipeFindItem(&ItemID);
    if (!Item)
        return;

    NTVItemData Info;
    Info.Icon = NULL;
    Info.Unknown04 = -1;
    DataNeed(Info) = 0;
    DataHave(Info) = 0;
    Info.Text[0] = 0;
    Info.Text2[0] = 0;
    Info.Width = 0;
    Info.Height = 0;
    Info.IconW = 0;
    Info.IconH = 0;
    Info.Icon = FindTexture(*Item->IconName);
    if (!Info.Icon)
        Info.Icon = FindTexture(TEXT("NWindow.BlackTexture"));
    Info.IconH = RECIPETREE_ICON;
    Info.IconW = RECIPETREE_ICON;
    appStrcpy(Info.Text, *Item->Name);
    appStrcpy(Info.Text2, *NItemRecordDescription(Item));
    DataNeed(Info) = Need;
    NCInvenItemWnd* Page0 = GDesktop->InventoryWnd->ItemWnd[0];
    if (Page0)
    {
        for (INT i = 0; i < Page0->Items.Num(); i++)
            if (ItemID == Page0->Items(i)->ID)
                DataHave(Info) += Page0->Items(i)->Count;
    }
    NCTVItem* Node = RecipeTree->AddItem(&Info, Parent);
    if (Rec && Rec->ProductRec)
    {
        for (INT i = 0; i < Rec->Materials.Num(); i++)
        {
            NRecipeMaterial* Material = Rec->Materials(i);
            if (Material)
                AddRecipe(Material->ItemID, RECIPE_RATE_SURE, Material->Count, Node);
        }
    }
}
NW_HOOK_METHOD(0x100c6af0, NCRecipeTreeCtrl, AddRecipe)

NW_CHECK_VTABLE(NCRecipeTreeCtrl, 0x101243d0, 72)
