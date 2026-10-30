/*=============================================================================
    NCGMSnoopWnd.cpp - the GM's snoop windows (C2 only), and the manager that holds four of them.
    Stock: 0x100b68a0 .. 0x100b6976 (WindowProc, Reset, OnClose, the manager's ShowWindow),
    0x100b6b70 .. 0x100b6bc9, 0x100b70c0 .. 0x100b7194, 0x100b7370 .. 0x100b74d3, OnCreate at
    0x100b8960, and the manager's constructor and destructor at 0x1004c6a0 .. 0x1004c73f.
=============================================================================*/
#include "nwindow/NCGMSnoopWnd.h"
#include "nwindow/NCGMSnoopManager.h"
#include "nwindow/NCButton.h"
#include "nwindow/NCCheckBox.h"
#include "nwindow/NCFrameCtrl.h"
#include "nwindow/NCGDevice.h"
#include "nwindow/NCPetitionListBox.h"
#include "nwindow/NConsoleWnd.h"
#include "nwindow/NDialogInfo.h"

// ids (names ours)
enum
{
    NID_GMSNOOP_LIST = 0xFCA0B1,
    NID_GMSNOOP_CLOSE = 0xFCA0B4,
    NID_GMSNOOP_FRAME = 0xFCA0B5,
    NID_GMSNOOP_FILTER = (INT)0xF0F01450,   // + the filter's number
};

// the seven check boxes: system message of the label and place (names ours)
struct NSnoopFilterDesc
{
    INT LabelID;
    INT X;
    INT Y;
};
static const NSnoopFilterDesc GSnoopFilters[7] = {
    { 0x16A, 15, 240 }, { 0x168, 84, 240 }, { 0x169, 153, 240 }, { 0x167, 222, 240 },
    { 0x16B, 15, 254 }, { 0x17F, 84, 254 }, { 0x250, 153, 254 },
};

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

/*-----------------------------------------------------------------------------
    NCGMSnoopWnd
-----------------------------------------------------------------------------*/

// 0x100b6b70 - the members are left as allocated
NCGMSnoopWnd::NCGMSnoopWnd()
{
}
NW_HOOK_CTOR(0x100b6b70, NCGMSnoopWnd)

// 0x100b6bc0
NW_HOOK_DTOR(0x100b6bc0, NCGMSnoopWnd)

// 0x100b68a0 (vslot 25)
INT NCGMSnoopWnd::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (NCWnd::WindowProc(Msg, wParam, lParam))
        return 1;
    if (Msg == NWM_COMMAND && wParam == NID_GMSNOOP_CLOSE)
        return OnClose() != 0;
    return 0;
}
NW_HOOK_METHOD(0x100b68a0, NCGMSnoopWnd, WindowProc)

// 0x100b6900 - the conversation emptied (NCListBox::Clear, vslot 70)
void NCGMSnoopWnd::Reset()
{
    List->Clear();
    SnoopID = -1;
}
NW_HOOK_METHOD(0x100b6900, NCGMSnoopWnd, Reset)

// 0x100b6920 (vslot 67) - closing: the window goes, the server is told (the stock calls the
// console's 0x10055c10, which is NetHandler->GMSnoopEnd) and the list is emptied
INT NCGMSnoopWnd::OnClose()
{
    HideWindow();
    GConsole->Engine->NetHandler->GMSnoopEnd(SnoopID);
    Reset();
    return 1;
}
NW_HOOK_METHOD(0x100b6920, NCGMSnoopWnd, OnClose)

// 0x100b7260 (name ours) - a line of the conversation goes into the list when the check box of its chat kind is
// checked: type 8 / 4 / 3 / 1 / 2 / 0 / 9 are the filters 0 .. 6 (label ids 0x16A, 0x168, 0x169, 0x167, 0x16B, 0x17F,
// 0x250); the other types (5 .. 7, and above 9) are dropped. The fourth value is not used (the console passes 0)
void NCGMSnoopWnd::AddLine(const TCHAR* Text, DWORD Color, INT Type, INT Unused)
{
    INT Filter;
    switch (Type)
    {
    case 0: Filter = 5; break;
    case 1: Filter = 3; break;
    case 2: Filter = 4; break;
    case 3: Filter = 2; break;
    case 4: Filter = 1; break;
    case 8: Filter = 0; break;
    case 9: Filter = 6; break;
    default: return;
    }
    if (Filters[Filter]->bChecked)
        List->AddString(Text, Color, -1);
}
NW_HOOK_METHOD(0x100b7260, NCGMSnoopWnd, AddLine)

// 0x100b8960 (vslot 36) - title bar, the conversation, the close button and seven filters
INT NCGMSnoopWnd::OnCreate()
{
    NCWnd::OnCreate();
    BackTex = FindTexture(TEXT("L2UI.etcwnd.Petitionback"));

    FrameCtrl = new NCFrameCtrl(1);
    FrameCtrl->ID = NID_GMSNOOP_FRAME;
    FrameCtrl->SetWindowRect(0, 0, 300, 21);
    CreateChild(FrameCtrl, NWS_VISIBLE);

    // 16 lines seen, 200 kept, a bare list (NWS_NOHITTEST, not NWS_VISIBLE) that follows the last line
    List = new NCPetitionListBox(200, 16, 12);
    List->ID = NID_GMSNOOP_LIST;
    List->SetWindowRect(15, 38, 282, 195);
    CreateChild(List, NWS_NOHITTEST);
    List->bAutoScroll = 1;
    List->LineGap = 0;

    CloseButton = new NCButton(TEXT("L2UI.control.DefaultButton"), TEXT("L2UI.control.DefaultButton_click"), 0);
    CloseButton->ID = NID_GMSNOOP_CLOSE;
    CloseButton->SetWindowRect(110, 270, 64, 19);
    CreateChild(CloseButton, NWS_VISIBLE);
    CloseButton->SetTextID(141);

    for (INT i = 0; i < 7; i++)
    {
        Filters[i] = new NCCheckBox(GSnoopFilters[i].LabelID, 0, 0);
        Filters[i]->ID = NID_GMSNOOP_FILTER + i;
        Filters[i]->SetWindowRect(GSnoopFilters[i].X, GSnoopFilters[i].Y, 64, 12);
        CreateChild(Filters[i], NWS_VISIBLE);
    }
    for (INT i = 0; i < 7; i++)
        Filters[i]->bChecked = 1;
    SnoopID = -1;
    return 1;
}
NW_HOOK_METHOD(0x100b8960, NCGMSnoopWnd, OnCreate)

// 0x100b70c0 (vslot 37) - guarded in C2
INT NCGMSnoopWnd::OnDestroy()
{
    guard(NCGMSnoopWnd::OnDestroy);
    DestroyChild(this, List);
    DestroyChild(this, CloseButton);
    for (INT i = 0; i < 7; i++)
        DestroyChild(this, Filters[i]);
    NCWnd::OnDestroy();
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x100b70c0, NCGMSnoopWnd, OnDestroy)

// 0x100b71d0 (vslot 55) - the stock body is NCPetitionWnd's, for several windows (hooked there):
// the panel under the 21-pixel title bar; the texture is not checked
INT NCGMSnoopWnd::OnPaint(NCGDevice* Device)
{
    Device->BeginPaint(ScreenX, ScreenY);
    Device->SetClip(ScreenX, ScreenY, Width, Height);
    Device->DrawTexture(0, 21, 300, 280, 0, 0, 300, 280, BackTex);
    Device->ReleaseClip();
    Device->EndPaint();
    return 1;
}

/*-----------------------------------------------------------------------------
    NCGMSnoopManager
-----------------------------------------------------------------------------*/

// 0x1004c6a0 - the windows come with OnCreate
NCGMSnoopManager::NCGMSnoopManager()
{
}
NW_HOOK_CTOR(0x1004c6a0, NCGMSnoopManager)

// 0x1004c6f0
NW_HOOK_DTOR(0x1004c6f0, NCGMSnoopManager)

// 0x100b6950 (vslot 43) - shown, but its four windows stay hidden
void NCGMSnoopManager::ShowWindow()
{
    NCWnd::ShowWindow();
    for (INT i = 0; i < 4; i++)
        Snoops[i]->HideWindow();
}
NW_HOOK_METHOD(0x100b6950, NCGMSnoopManager, ShowWindow)

// 0x100b7370 (vslot 36) - the four snoop windows (NWS_NOCLOSE | NWS_NOHITTEST), hidden
INT NCGMSnoopManager::OnCreate()
{
    NCWnd::OnCreate();
    for (INT i = 0; i < 4; i++)
    {
        Snoops[i] = new NCGMSnoopWnd();
        Snoops[i]->SetWindowRect(15, 38, 300, 301);
        CreateChild(Snoops[i], NWS_NOCLOSE | NWS_NOHITTEST);
        Snoops[i]->HideWindow();
    }
    return 1;
}
NW_HOOK_METHOD(0x100b7370, NCGMSnoopManager, OnCreate)

// 0x100b7430 (vslot 37) - guarded in C2
INT NCGMSnoopManager::OnDestroy()
{
    guard(NCGMSnoopManager::OnDestroy);
    for (INT i = 0; i < 4; i++)
        DestroyChild(this, Snoops[i]);
    NCWnd::OnDestroy();
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x100b7430, NCGMSnoopManager, OnDestroy)

// 0x100b74f0 (name ours; the console's Snoop calls it with five values) - a line for the windows that listen to
// SnoopID: each shows it and shows itself
void NCGMSnoopManager::AddLine(INT SnoopID, const TCHAR* Text, DWORD Color, INT Type, INT Unused)
{
    for (INT i = 0; i < 4; i++)
    {
        if (Snoops[i]->SnoopID == SnoopID)
        {
            Snoops[i]->AddLine(Text, Color, Type, Unused);
            Snoops[i]->ShowWindow();
        }
    }
}
NW_HOOK_METHOD(0x100b74f0, NCGMSnoopManager, AddLine)

// 0x100b7550 (name ours; the console's Snoop calls it) - the window of a snoop: the one that has it already (then
// nothing changes), else the first free one, titled "<system string 693> - <name>"; none free: the dialog 0x1F
// (system message 0x322) says so
void NCGMSnoopManager::OpenSnoop(INT SnoopID, const TCHAR* TargetName)
{
    for (INT i = 0; i < 4; i++)
        if (Snoops[i]->SnoopID == SnoopID)
            return;
    for (INT i = 0; i < 4; i++)
    {
        if (Snoops[i]->SnoopID == -1)
        {
            Snoops[i]->SnoopID = SnoopID;
            TCHAR Title[128];
            appSprintf(Title, TEXT("%s - %s"), *NCObject::SysString(693), TargetName);
            Snoops[i]->SetTitle(Title);
            return;
        }
    }
    NDialogInfo Info;
    Info.ID = 0x1F;
    Info.bMainBox = 1;
    Info.bInput = 0;
    Info.SetSystemMessage(0x322);
    GConsole->ShowDialog(&Info);
}
NW_HOOK_METHOD(0x100b7550, NCGMSnoopManager, OpenSnoop)

NW_CHECK_VTABLE(NCGMSnoopWnd, 0x10122718, 68)
NW_CHECK_VTABLE(NCGMSnoopManager, 0x101131d8, 67)
