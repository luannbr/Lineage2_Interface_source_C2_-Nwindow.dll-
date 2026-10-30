/*=============================================================================
    NCRecipeTree.cpp - the ingredient tree of the recipe information window (C2 only).
    Stock: 0x100c4830 (OnCreate), 0x100c5960 .. 0x100c59b0 (constructor, destructor), 0x100c5a00 (OnPaint), 0x100c5a90 (the
    entry drawing, vslot 71).
    The constructor calls NCTree's (0x1002a660, w2newui's): it is hooked only when that group's header is in the tree
    (NW_HAVE_NCTREE_H, see NCRecipeTree.h); until then the stock constructor runs (NCRecipeTreeCtrl::OnCreate makes the tree
    through it: StockCall).
=============================================================================*/
#include "nwindow/NCRecipeTree.h"
#include "nwindow/NCGDevice.h"
#include "nwindow/NClasses.gen.h"

// what the recipe code reads of an entry (NCTVItem's data record names the third and fourth dwords after nothing: here they
// are the number the recipe needs and the number the player has; +0x1A1C / +0x1A20 are the entry's place, see NCRecipeTree.h)
static FORCEINLINE INT& EntryNeed(NCTVItem* Item) { return Item->Data.Unknown08; }
static FORCEINLINE INT& EntryHave(NCTVItem* Item) { return Item->Data.Unknown0C; }

/*-----------------------------------------------------------------------------
    NCRecipeTree
-----------------------------------------------------------------------------*/

#ifdef NW_HAVE_NCTREE_H
// 0x100c5960 - NCTree's constructor and the vtable
NCRecipeTree::NCRecipeTree()
{
}
NW_HOOK_CTOR(0x100c5960, NCRecipeTree)
#endif

// 0x100c59b0
NW_HOOK_DTOR(0x100c59b0, NCRecipeTree)

// 0x100c4830 (vslot 36) - the five entry textures
INT NCRecipeTree::OnCreate()
{
    NControl::OnCreate();
    FrameTex = FindTexture(TEXT("L2UI.RecipeWnd.RecipeTreeIconBack"));
    FrameOpenTex = FindTexture(TEXT("L2UI.RecipeWnd.RecipeTreeIconBack_click"));
    FrameLeafTex = FindTexture(TEXT("L2UI.RecipeWnd.RecipeTreeIconDisableBack"));
    PlusTex = FindTexture(TEXT("L2UI.RecipeWnd.TreePlus"));
    MinusTex = FindTexture(TEXT("L2UI.RecipeWnd.TreeMinus"));
    return 1;
}
NW_HOOK_METHOD(0x100c4830, NCRecipeTree, OnCreate)

// 0x100c5a00 (vslot 55) - the clip is the window's top moved by the view offset, 246 high (the frame's inner height); the
// tree is drawn from the root at (15, 5) and the control takes the height that came out (the scroll frame reads it)
INT NCRecipeTree::OnPaint(NCGDevice* Device)
{
    Device->BeginPaint(ScreenX, ScreenY);
    Device->SetClip(ScreenX, ScrollOffset + ScreenY, Width, 246);
    ContentHeight = DrawItem(Device, Root, 15, 5);
    SetSize(Width, ContentHeight);
    Device->ReleaseClip();
    Device->EndPaint();
    return 1;
}
NW_HOOK_METHOD(0x100c5a00, NCRecipeTree, OnPaint)

// 0x100c5a90 (vslot 71, ret 0x10) - the entry is placed at (X, Y) (its window rectangle: wider by 16 with children; the place is
// kept in +0x1A1C / +0x1A20 for NCRecipeTreeCtrl's visibility test), its frame and icon drawn: expanded, the open frame and a
// minus sign at (X - 14, Y + 10); collapsed with children, the frame and a plus sign; a leaf, the disabled frame. Then the name
// at (icon width + X + 5, Y + 4) and below it "(have/need)" with a dimming square over the icon when the player has less than the
// recipe needs (the root has neither). The children follow: expanded, each one drawn under the previous (their column starts
// half an icon to the right; the heights add up), and the height used is returned; collapsed, they are hidden. An entry without
// children returns its own height (icon + 5); no entry, 0.
INT NCRecipeTree::DrawItem(NCGDevice* Device, NCTVItem* Item, INT X, INT Y)
{
    TCHAR Text[32];
    if (!Item)
        return 0;
    Item->CopyW = X;
    Item->CopyH = Y;
    if (Item->Children.Num() > 0)
        Item->SetWindowRect(X, Y, Item->Data.Width + 16, Item->Data.Height);
    else
        Item->SetWindowRect(X, Y, Item->Data.Width, Item->Data.Height);
    Device->DrawTexture(X, Y, Item->Data.IconW, Item->Data.IconH, 0, 0, Item->Data.IconW, Item->Data.IconH, Item->Data.Icon);
    if (Item->bExpanded)
    {
        Device->DrawTexture(X, Y, 32, 32, 0, 0, 32, 32, FrameOpenTex);
        Device->DrawTexture(X - 14, Y + 10, 12, 12, 0, 0, 12, 12, MinusTex);
    }
    else if (Item->Children.Num() > 0)
    {
        Device->DrawTexture(X, Y, 32, 32, 0, 0, 32, 32, FrameTex);
        Device->DrawTexture(X - 14, Y + 10, 12, 12, 0, 0, 12, 12, PlusTex);
    }
    else
        Device->DrawTexture(X, Y, 32, 32, 0, 0, 32, 32, FrameLeafTex);
    Device->DrawText(Item->Data.IconW + X + 5, Y + 4, 0xFFFFFFFF, Item->Data.Text, 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
    if (Item != Root)
    {
        appSprintf(Text, TEXT("(%d/%d)"), EntryHave(Item), EntryNeed(Item));
        Device->DrawText(Item->Data.IconW + X + 5, Y + 18, 0xFFFFFFFF, Text, 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
        if (EntryHave(Item) < EntryNeed(Item))
            Device->DrawTexture(X, Y, 32, 32, 0, 0, 32, 32, FindTexture(TEXT("NWindow.ChatBack")));
    }
    INT Used = Item->Data.IconH + 5;
    if (Item->Children.Num() > 0)
    {
        if (Item->bExpanded)
        {
            for (INT i = 0; i < Item->Children.Num(); i++)
                Used += DrawItem(Device, Item->Children(i), Item->Data.IconW / 2 + X, Used + Y);
            return Used;
        }
        for (INT i = 0; i < Item->Children.Num(); i++)
            Item->Children(i)->HideWindow();
    }
    return Used;
}
NW_HOOK_METHOD(0x100c5a90, NCRecipeTree, DrawItem)

NW_CHECK_VTABLE(NCRecipeTree, 0x101242b0, 72)
