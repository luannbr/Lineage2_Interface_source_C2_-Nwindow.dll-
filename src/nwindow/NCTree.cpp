/*=============================================================================
    NCTree.cpp - the entries of a tree view (C2 only). Stock: 0x1002a570 .. 0x1002a5cf (Clear,
    OnDestroy), 0x1002a660 .. 0x1002a6af and 0x1002a6e0 .. 0x1002a99f (constructor, destructor,
    DeleteItem, OnPaint, DrawItem) and 0x1002b300 .. 0x1002b4e0 (AddItem).
=============================================================================*/
#include "nwindow/NCTree.h"
#include "nwindow/NCGDevice.h"

enum { NID_TVITEM = 0xA00000 };   // every entry's id (name ours)

// 0x1002a660 - NControl's constructor is inlined in the stock code
NCTree::NCTree()
{
    Root = NULL;
    ScrollOffset = 0;
    ContentHeight = 0;
}
NW_HOOK_CTOR(0x1002a660, NCTree)

// 0x1002a6e0
NW_HOOK_DTOR(0x1002a6e0, NCTree)

// 0x1002a570 (vslot 70) - everything goes
void NCTree::Clear()
{
    DeleteItem(&Root);
}
NW_HOOK_METHOD(0x1002a570, NCTree, Clear)

// 0x1002a580 (vslot 37) - guarded in C2; the entries go, NControl's OnDestroy is not called
INT NCTree::OnDestroy()
{
    guard(NCTree::OnDestroy);
    Clear();
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x1002a580, NCTree, OnDestroy)

// 0x1002a730 (vslot 69) - the children first (each by its address in the array: the count is read
// again after each one), then the entry
void NCTree::DeleteItem(NCTVItem** Item)
{
    if (*Item)
    {
        INT Count = (*Item)->Children.Num();
        if (Count > 0)
        {
            for (INT i = 0; i < (*Item)->Children.Num(); i++)
                DeleteItem(&(*Item)->Children(i));
        }
        if (*Item)
        {
            RemoveChildWindow((*Item)->Entry);
            delete *Item;
            *Item = NULL;
        }
    }
}
NW_HOOK_METHOD(0x1002a730, NCTree, DeleteItem)

// 0x1002a830 (vslot 71) - Item at (X, Y): its window sized to its picture and text (16 wider with
// entries under it), the picture and the text (blue) drawn, then the children indented by the
// picture's width + 5 under it when it is expanded (hidden when not). The height used goes back.
INT NCTree::DrawItem(NCGDevice* Device, NCTVItem* Item, INT X, INT Y)
{
    if (!Item)
        return 0;
    Item->ShowWindow();
    if (Item->Children.Num() < 1)
        Item->SetWindowRect(X, Y, Item->Data.Width, Item->Data.Height);
    else
        Item->SetWindowRect(X, Y, Item->Data.Width + 16, Item->Data.Height);
    Device->DrawTexture(X, Y, Item->Data.IconW, Item->Data.IconH, 0, 0, Item->Data.IconW, Item->Data.IconH,
                        Item->Data.Icon);
    Device->DrawText(Item->Data.IconW + 5 + X, Y, 0xFF0000FF, Item->Data.Text, 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0, 0);
    INT NextY = Item->Data.IconH + 5;
    if (Item->Children.Num() > 0)
    {
        INT i = 0;
        if (Item->bExpanded == 0)
        {
            if (Item->Children.Num() > 0)
            {
                do
                {
                    Item->Children(i)->HideWindow();
                    i++;
                } while (i < Item->Children.Num());
            }
        }
        else if (Item->Children.Num() > 0)
        {
            do
            {
                INT Used = DrawItem(Device, Item->Children(i), Item->Data.IconW + 5 + X, NextY + Y);
                NextY += Used;
                i++;
            } while (i < Item->Children.Num());
            return NextY;
        }
    }
    return NextY;
}
NW_HOOK_METHOD(0x1002a830, NCTree, DrawItem)

// 0x1002a7a0 (vslot 55) - the entries laid out, moved up by ScrollOffset; the window is as tall as they are
INT NCTree::OnPaint(NCGDevice* Device)
{
    Device->BeginPaint(ScreenX, ScreenY);
    Device->SetClip(ScreenX, ScreenY + ScrollOffset, Width, Height);
    ContentHeight = DrawItem(Device, Root, 0, 0);
    SetSize(Width, ContentHeight);
    Device->ReleaseClip();
    Device->EndPaint();
    return 1;
}
NW_HOOK_METHOD(0x1002a7a0, NCTree, OnPaint)

// 0x1002b300 (vslot 68) - a copy of Data is made into a new entry (the stock allocates it with the
// level -1) as wide as its picture and its text; without a parent it becomes the root (once),
// else a child of Parent
NCTVItem* NCTree::AddItem(NTVItemData* Data, NCTVItem* Parent)
{
    if (!Data)
        return NULL;
    if (Parent)
    {
        NCTVItem* Item = new NCTVItem(-1);
        Item->Data = *Data;
        SIZE Size;
        NCGDevice::GetTextExtent(&Size, Data->Text, 0, 0, 0, 0, 0);
        Item->Data.Width = Data->IconW + Size.cx + 2;
        Item->Data.Height = Data->IconH;
        Item->ID = NID_TVITEM;
        CreateChild(Item, NWS_VISIBLE);
        return Parent->AddChild(Item);
    }
    if (Root == NULL)
    {
        Root = new NCTVItem(-1);
        Root->Data = *Data;
        SIZE Size;
        NCGDevice::GetTextExtent(&Size, Data->Text, 0, 0, 0, 0, 0);
        Root->Data.Width = Data->IconW + Size.cx + 2;
        Root->Data.Height = Data->IconH;
        Root->ID = NID_TVITEM;
        NWndEntry* E = AddChildWindow(Root, NWS_VISIBLE);
        Root->AttachEntry(E);
        return Root;
    }
    return NULL;
}
NW_HOOK_METHOD(0x1002b300, NCTree, AddItem)

NW_CHECK_VTABLE(NCTree, 0x1010e7b0, 72)
