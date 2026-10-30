/*=============================================================================
    NCHennaListWnd.cpp - the tattoo lists of the symbol maker. Stock: 0x10079090 .. 0x10079109, 0x10079110 .. 0x10079168,
    0x100798a0 .. 0x10079a00 (constructor, destructor, OnDestroy), 0x10079a00 .. 0x10079b6b (paint), 0x10079ba0 .. 0x10079c5f,
    0x1007aef0, 0x1007b090 .. 0x1007b22f.
=============================================================================*/
#include "nwindow/NCHennaListWnd.h"
#include "nwindow/NCFrameCtrl.h"
#include "nwindow/NCGDevice.h"
#include "nwindow/NConsoleWnd.h"

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

// sysstring
enum
{
    STR_Price = 637,
    STR_AddSymbol = 651,        // the window's title when it lists the tattoos to draw
    STR_RemoveSymbol = 652,     // ... to take off
    STR_SelectToAdd = 659,      // the heading on the paper
    STR_SelectToRemove = 660,
};

// 0x100798a0 - the list box, the texture and the adena are cleared; AdenaTex and Mode are left as allocated
NCHennaListWnd::NCHennaListWnd()
{
    ListBox = NULL;
    BackTex = NULL;
    Adena = 0;
}
NW_HOOK_CTOR(0x100798a0, NCHennaListWnd)

// 0x10079900
NW_HOOK_DTOR(0x10079900, NCHennaListWnd)

// 0x10079090 (vslot 25) - a click on a row reports its index
INT NCHennaListWnd::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (NCWnd::WindowProc(Msg, wParam, lParam))
        return 1;
    if (Msg == NWM_COMMAND && wParam == NID_HENNALIST_LIST)
        return OnSelect((INT)lParam) != 0;
    return 0;
}
NW_HOOK_METHOD(0x10079090, NCHennaListWnd, WindowProc)

// 0x1007b090 (vslot 36) - the title bar, the textures and the list (100 rows kept, 5 seen, 5 pixels apart, from (15, 9))
INT NCHennaListWnd::OnCreate()
{
    NCWnd::OnCreate();
    FrameCtrl = new NCFrameCtrl(1);
    FrameCtrl->ID = NID_HENNALIST_FRAME;
    FrameCtrl->SetWindowRect(0, 0, 240, 21);
    CreateChild(FrameCtrl, NWS_VISIBLE);
    BackTex = FindTexture(TEXT("L2UI.HennaWnd.HennaListBack"));
    AdenaTex = FindTexture(TEXT("L2UI.WindowIcon.WindowIconAdena"));
    ListBox = new NCHennaListBox(100, 5, 16);
    ListBox->ID = NID_HENNALIST_LIST;
    ListBox->SetWindowRect(1, 39, 236, 252);
    CreateChild(ListBox, NWS_VISIBLE);
    ListBox->TextX = 15;
    ListBox->TextY = 9;
    ListBox->LineGap = 5;
    return 1;
}
NW_HOOK_METHOD(0x1007b090, NCHennaListWnd, OnCreate)

// 0x10079950 (vslot 37) - the list box goes, then the base
INT NCHennaListWnd::OnDestroy()
{
    guard(NCHennaListWnd::OnDestroy);
    DestroyChild(this, ListBox);
    NCWnd::OnDestroy();
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x10079950, NCHennaListWnd, OnDestroy)

// 0x10079110 (vslot 43) - the title by the mode
void NCHennaListWnd::ShowWindow()
{
    NCWnd::ShowWindow();
    if (Mode == 0)
        SetTitle(*SysString(STR_AddSymbol));
    else if (Mode == 1)
        SetTitle(*SysString(STR_RemoveSymbol));
}
NW_HOOK_METHOD(0x10079110, NCHennaListWnd, ShowWindow)

// 0x10079a00 (vslot 55) - the background, the heading by the mode, the adena icon and the adena right aligned to 222. The
// guard names NCRecipeListWnd::OnPaint (copied code; the db takes that for the owner of the address).
INT NCHennaListWnd::OnPaint(NCGDevice* Device)
{
    guard(NCRecipeListWnd::OnPaint);
    TCHAR Text[32];
    SIZE Size;
    Device->BeginPaint(ScreenX, ScreenY);
    Device->SetClip(ScreenX, ScreenY, Width, Height);
    if (BackTex)
        Device->DrawTexture(0, 21, 240, 344, 0, 0, 240, 344, BackTex);
    if (Mode == 0)
        Device->DrawText(15, 25, 0xFFFFFFFF, *SysString(STR_SelectToAdd), 0, 0, 0, L2FONT_Normal, 0, 0, 0,
                         0, 0, 0);
    else if (Mode == 1)
        Device->DrawText(15, 25, 0xFFFFFFFF, *SysString(STR_SelectToRemove), 0, 0, 0, L2FONT_Normal, 0, 0,
                         0, 0, 0, 0);
    if (AdenaTex)
        Device->DrawTexture(52, 313, 16, 12, 0, 0, 16, 12, AdenaTex);
    appSprintf(Text, TEXT("%d"), Adena);
    NCGDevice::GetTextExtent(&Size, Text, 0, 0, 0, 0, 0);
    Device->DrawText(222 - Size.cx, 315, 0xFFFFFFFF, Text, 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
    Device->EndPaint();
    Device->ReleaseClip();
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x10079a00, NCHennaListWnd, OnPaint)

// 0x10079ba0 (vslot 67) - a row clicked: the server is asked for the tattoo's information (to draw it, or to take it off)
INT NCHennaListWnd::OnSelect(INT Index)
{
    NHennaListEntry Entry;
    if (ListBox->GetSelectedEntry(&Entry))
    {
        if (Mode == 0)
        {
            GConsole->RequestHennaItemInfo(Entry.HennaID);
            return 1;
        }
        if (Mode == 1)
            GConsole->RequestHennaUnequipInfo(Entry.HennaID);
    }
    return 1;
}
NW_HOOK_METHOD(0x10079ba0, NCHennaListWnd, OnSelect)

// 0x100790f0 (name ours) - the rows and the adena cleared
void NCHennaListWnd::Clear()
{
    ListBox->Clear();
    Adena = 0;
}
NW_HOOK_METHOD(0x100790f0, NCHennaListWnd, Clear)

// 0x1007aef0 (name ours) - a row for the list (the stock code jumps into the list box's AddEntry)
void NCHennaListWnd::AddEntry(const NHennaListEntry* Src)
{
    ListBox->AddEntry(Src);
}
NW_HOOK_METHOD(0x1007aef0, NCHennaListWnd, AddEntry)

NW_CHECK_VTABLE(NCHennaListWnd, 0x10118270, 68)
