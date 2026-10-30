/*=============================================================================
    NCXMasSealWnd.cpp - the Christmas seal window (C2 only). Stock: 0x100df890 .. 0x100dfb3f.
=============================================================================*/
#include "nwindow/NCXMasSealWnd.h"
#include "nwindow/NCFrameCtrl.h"
#include "nwindow/NCGDevice.h"
#include "nwindow/NGameData.h"

enum { NID_XMASSEAL_FRAME = 0xB000FA };   // the title bar's id (name ours)

// 0x100df910 - the first and the last member only; the back texture comes with OnCreate
NCXMasSealWnd::NCXMasSealWnd()
{
    Unknown98 = 0;
    SealTex = NULL;
}
NW_HOOK_CTOR(0x100df910, NCXMasSealWnd)

// 0x100df960
NW_HOOK_DTOR(0x100df960, NCXMasSealWnd)

// 0x100df9b0 (vslot 36) - a 334 x 21 title bar of its own, FrameID is not set
INT NCXMasSealWnd::OnCreate()
{
    NCWnd::OnCreate();
    BackTex = FindTexture(TEXT("L2UI.MinimapWnd.MinimapBack"));

    FrameCtrl = new NCFrameCtrl(1);
    FrameCtrl->ID = NID_XMASSEAL_FRAME;
    FrameCtrl->SetWindowRect(0, 0, 334, 21);
    CreateChild(FrameCtrl, NWS_VISIBLE);
    return 1;
}
NW_HOOK_METHOD(0x100df9b0, NCXMasSealWnd, OnCreate)

// 0x100df890 (vslot 37) - guarded in C2; nothing of its own to free
INT NCXMasSealWnd::OnDestroy()
{
    guard(NCXMasSealWnd::OnDestroy);
    NCWnd::OnDestroy();
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x100df890, NCXMasSealWnd, OnDestroy)

// 0x100dfa70 (vslot 55) - the panel below the title bar, the seal inside it
INT NCXMasSealWnd::OnPaint(NCGDevice* Device)
{
    Device->SetClip(ScreenX, ScreenY, Width, Height);
    Device->BeginPaint(ScreenX, ScreenY);
    if (BackTex)
        Device->DrawTexture(0, 21, 334, 332, 0, 0, 334, 332, BackTex);
    if (SealTex)
        Device->DrawTexture(3, 22, 328, 329, 0, 0, 328, 329, SealTex);
    Device->EndPaint();
    Device->ReleaseClip();
    return 1;
}
NW_HOOK_METHOD(0x100dfa70, NCXMasSealWnd, OnPaint)

// 0x100dfb40 (name ours) - the item the console shows as the seal (UnknownA98 keeps its id): the picture is the texture of
// the name at +0x7C of its item record (the stock reads the record without a check)
void NCXMasSealWnd::SetItem(INT ItemID)
{
    Unknown98 = ItemID;
    NViewItemData* Data = GL2GameData.ItemTable->FindRef(Unknown98);
    SealTex = FindTexture(*nw::Field<L2FName>(Data, 0x7C));
}
NW_HOOK_METHOD(0x100dfb40, NCXMasSealWnd, SetItem)

NW_CHECK_VTABLE(NCXMasSealWnd, 0x10129078, 67)
