/*=============================================================================
    NCTVItem.cpp - the entry of a tree view (C2 only). Stock: 0x1002a4b0 .. 0x1002a65f (OnCreate,
    OnLButtonDown, OnPaint), 0x1002b0b0 .. 0x1002b0dd (AddChild), 0x1002b0e0 .. 0x1002b16f (the
    children array's destructor), 0x1002b170 .. 0x1002b2ff (constructor, destructor).
=============================================================================*/
#include "nwindow/NCTVItem.h"
#include "nwindow/NCGDevice.h"

// 0x1002b0e0 - ~TArray<NCTVItem*> (the template code, out of line in the stock module)
typedef TArray<NCTVItem*> TTVItemArray;
NW_HOOK_DTOR(0x1002b0e0, TTVItemArray)

// 0x1002b170 - NControl's constructor is inlined in the stock code; the data record starts
// empty, the hit box and the sizes zero, Level is the argument
NCTVItem::NCTVItem(INT InLevel)
{
    Data.Icon = NULL;
    Data.Unknown04 = -1;
    Data.Unknown08 = 0;
    Data.Unknown0C = 0;
    Data.Text[0] = 0;
    Data.Text2[0] = 0;
    Data.Width = 0;
    Data.Height = 0;
    Data.IconW = 0;
    Data.IconH = 0;
    Unknown19DC = 0;
    Unknown19E0 = 0;
    Unknown19E4 = 0;
    Unknown19E8 = 0;
    Level = InLevel;
    bExpanded = 0;
    Unknown19F4 = 0;
    Unknown19F8 = 0;
    Unknown19FC = 0;
    Unknown1A00 = 0;
    HitX = 0;
    HitY = 0;
    HitW = 0;
    HitH = 0;
    Unknown1A14 = 0;
    Unknown1A18 = 0;
    CopyW = 0;
    CopyH = 0;
}
NW_HOOK_CTOR_ARGS(0x1002b170, NCTVItem, (INT A), (A))

// 0x1002b2a0
NW_HOOK_DTOR(0x1002b2a0, NCTVItem)

// 0x1002b0b0 (ret 4) - Child at the end of the children
NCTVItem* NCTVItem::AddChild(NCTVItem* Child)
{
    if (Child)
    {
        INT Index = Children.Add(1);
        Children(Index) = Child;
        return Child;
    }
    return NULL;
}
NW_HOOK_METHOD(0x1002b0b0, NCTVItem, AddChild)

// 0x1002a4b0 (vslot 36) - the size the window starts with is kept (NControl's OnCreate runs first)
INT NCTVItem::OnCreate()
{
    NControl::OnCreate();
    CopyW = Width;
    CopyH = Height;
    return 1;
}
NW_HOOK_METHOD(0x1002a4b0, NCTVItem, OnCreate)

// 0x1002a4e0 (vslot 61) - with entries under it a click turns bExpanded over, and a click inside the
// hit box turns it back (16-bit coordinates relative to the item)
INT NCTVItem::OnLButtonDown(WPARAM wParam, LPARAM lParam)
{
    if (Children.Num() > 0)
    {
        UBOOL bToggled = (bExpanded == 0);
        UINT X = (WORD)((SHORT)lParam - (SHORT)ScreenX);
        bExpanded = bToggled;
        if (HitX <= (INT)X && X <= (UINT)(HitW + HitX))
        {
            UINT Y = (WORD)((SHORT)((DWORD)lParam >> 16) - (SHORT)ScreenY);
            if (HitY <= (INT)Y && Y <= (UINT)(HitH + HitY))
            {
                bExpanded = (bToggled == 0);
                return 1;
            }
        }
    }
    return 1;
}
NW_HOOK_METHOD(0x1002a4e0, NCTVItem, OnLButtonDown)

// 0x1002a600 (vslot 55) - nothing is drawn (the tree draws the entries)
INT NCTVItem::OnPaint(NCGDevice* Device)
{
    Device->BeginPaint(ScreenX, ScreenY);
    Device->SetClip(ScreenX, ScreenY, Width, Height);
    Device->ReleaseClip();
    Device->EndPaint();
    return 1;
}
NW_HOOK_METHOD(0x1002a600, NCTVItem, OnPaint)

NW_CHECK_VTABLE(NCTVItem, 0x1010ea20, 68)
