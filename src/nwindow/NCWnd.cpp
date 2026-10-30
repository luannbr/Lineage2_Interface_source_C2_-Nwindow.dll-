/*=============================================================================
    NCWnd.cpp - base window: tree, geometry, focus, visibility, message routing.
    Stock (C2): 0x1002b500 .. 0x1002cdc9, plus the defaults folded at 0x100016e0.. and
    DefWindowProc (NCCmdTarget::WindowProc, 0x10004bb0).
=============================================================================*/
#include "nwindow/NCVirtualWndMain.h"
#include "nwindow/NControl.h"
#include "nwindow/NCFrameCtrl.h"
#include "nwindow/NCIcon.h"
#include "nwindow/NCConsole.h"
#include "nwindow/NCShopWnd.h"
#include "nwindow/NCStoreWnd.h"
#include "nwindow/NCTradeWnd.h"
#include "nwindow/NCDeliverWnd.h"
#include "nwindow/NClasses.gen.h"
#if NW_FEATURES
#include "features/WindowFix.h"   // phase 6: WindowFix
#endif

#include <wchar.h>

// The tool tip skin and info (NToolTipInfo / NToolTipSkin, 0x10002230 .. 0x1002a4a1) are in NToolTip.cpp.

/*-----------------------------------------------------------------------------
    NCCmdTarget
-----------------------------------------------------------------------------*/

// 0x10001a60 - NW_DECLARE_CLASS destructor
NW_HOOK_DTOR(0x10001a60, NCCmdTarget)

/*-----------------------------------------------------------------------------
    Construction
-----------------------------------------------------------------------------*/

// 0x1002bc10
NCWnd::NCWnd()
{
    ChildHead = NULL;
    ChildTail = NULL;
    LocalY = 0;
    LocalX = 0;
    LocalW = 0;
    LocalH = 0;
    bFocus = 0;
    ScreenY = 0;
    ScreenX = 0;
    Width = 0;
    Height = 0;
    Style = 0;
    Entry = NULL;
    bEnabled = 1;
    DragMouseY = 0;
    DragMouseX = 0;
    NextTab = NULL;
    PrevTab = NULL;
    bStuckTop = 0;
    bStuckBottom = 0;
    bStuckLeft = 0;
    bStuckRight = 0;
}
NW_HOOK_CTOR(0x1002bc10, NCWnd)

// 0x10001b20 - NW_DECLARE_CLASS destructor
NW_HOOK_DTOR(0x10001b20, NCWnd)

// Accessors and the defaults the linker folded (one stock body serves several slots/classes).
NW_HOOK_METHOD(0x10001b70, NCWnd, IsEnabled)
NW_HOOK_METHOD(0x10001b80, NCWnd, IsFocused)
NW_HOOK_METHOD(0x10001b90, NCWnd, SetStyle)
NW_HOOK_METHOD(0x10001ba0, NCWnd, GetStyle)
NW_HOOK_METHOD(0x10001ab0, NCWnd, vfn032)   // xor eax,eax; ret    - also vslot 33, NControl vslot 62
NW_HOOK_METHOD(0x10001ac0, NCWnd, OnChar)   // xor eax,eax; ret 8  - every default mouse/key handler
NW_HOOK_METHOD(0x10001ae0, NCWnd, OnTimer)  // xor eax,eax; ret 4  - also OnPaint, OnDrawToolTip

// 0x1002c9e0 - creates the title bar (NWS_TITLEBAR) and the minimized icon (NWS_MINIMIZABLE); C2 then makes
// the tool tip skin of every window: nine textures "L2UI.Tooltip.Tool01" .. "Tool09"
INT NCWnd::OnCreate()
{
    INT W = LocalW;
    FrameCtrl = NULL;
    Icon = NULL;
    if (Style & NWS_TITLEBAR)
    {
        FrameCtrl = new NCFrameCtrl(0);
        FrameID = GNextFrameID++;
        FrameCtrl->ID = FrameID;
        FrameCtrl->SetWindowRect(0, -15, W, 15);
        CreateChild(FrameCtrl, NWS_VISIBLE);
    }
    if (Style & NWS_MINIMIZABLE)
    {
        Icon = new NCIcon;
        Icon->ID = 15;
        Icon->SetWindowRect(0, 0, 32, 32);
        CreateChild(Icon, NWS_TOPMOST | NWS_UNKNOWN20 | NWS_VISIBLE);   // C2: 0x1022 (C1: 0x1002)
        Icon->HideWindow();
    }
    ToolTipSkin = new NToolTipSkin;
    for (INT i = 1; i < 10; i++)
    {
        TCHAR Name[32];
        appSprintf(Name, TEXT("L2UI.Tooltip.Tool0%d"), i);
        ToolTipSkin->Tex[i - 1] = FindTexture(Name);
    }
    return 1;
}
NW_HOOK_METHOD(0x1002c9e0, NCWnd, OnCreate)

// 0x1002cd20 - C2 guards it and frees the tool tip skin between the title bar and the icon
INT NCWnd::OnDestroy()
{
    guard(NCWnd::OnDestroy);
    if (FrameCtrl)
    {
        RemoveChildWindow(FrameCtrl->Entry);
        delete FrameCtrl;
        FrameCtrl = NULL;
    }
    if (ToolTipSkin)
    {
        delete ToolTipSkin;
        ToolTipSkin = NULL;
    }
    if (Icon)
    {
        RemoveChildWindow(Icon->Entry);
        delete Icon;
        Icon = NULL;
    }
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x1002cd20, NCWnd, OnDestroy)

/*-----------------------------------------------------------------------------
    Messages
-----------------------------------------------------------------------------*/

// 0x10004bb0 (NCCmdTarget::WindowProc, vslot 25 of NCCmdTarget) - the creation, destruction and timer messages
INT NCWnd::DefWindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (Msg == WM_CREATE)
        return OnCreate() != 0;
    if (Msg == WM_DESTROY)
        return OnDestroy() != 0;
    if (Msg == WM_TIMER)
        return OnTimer((INT)wParam) != 0;
    return 0;
}
NW_HOOK_METHOD(0x10004bb0, NCWnd, DefWindowProc)

// 0x1002b500 (vslot 25) - WM_PAINT carries the NCGDevice in wParam. C1 returned 0 first for a disabled window
// and any message but WM_PAINT: C2 does not (the test is gone from NControl::WindowProc too)
INT NCWnd::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (DefWindowProc(Msg, wParam, lParam))
        return 1;
    if (Msg == WM_PAINT)
    {
        if (Style & NWS_VISIBLE)
            OnPaint((NCGDevice*)wParam);
        return 1;
    }
    if (Msg == WM_CHAR)
        return OnChar(wParam, lParam) != 0;
    if (Msg == WM_KEYDOWN)
        return OnKeyDown(wParam, lParam) != 0;
    if (Msg == WM_KEYUP)
        return OnKeyUp(wParam, lParam) != 0;
    if (Msg == WM_MOUSEMOVE)
        return OnMouseMove(wParam, lParam) != 0;
    if (Msg == WM_LBUTTONDOWN)
        return OnLButtonDown(wParam, lParam) != 0;
    if (Msg == WM_RBUTTONDOWN)
        return OnRButtonDown(wParam, lParam) != 0;
    if (Msg == WM_LBUTTONUP)
        return OnLButtonUp(wParam, lParam) != 0;
    if (Msg == WM_RBUTTONUP)
        return OnRButtonUp(wParam, lParam) != 0;
    if (Msg == WM_MOVE)
        return OnMove(wParam, lParam) != 0;
    if (Msg == WM_MOUSEWHEEL)
        return OnMouseWheel(wParam, lParam) != 0;
    if (Msg == NWM_DRAWTOOLTIP)
    {
        if (Style & NWS_VISIBLE)
            OnDrawToolTip((NCGDevice*)wParam);
        return 1;
    }
    return 0;
}
NW_HOOK_METHOD(0x1002b500, NCWnd, WindowProc)

// 0x1002b980 (vslot 58) - TAB / Shift+TAB move the focus along NextTab / PrevTab
INT NCWnd::OnKeyDown(WPARAM wParam, LPARAM lParam)
{
    if (wParam == VK_TAB)
    {
        NWndEntry* Target;
        if (GShiftDown)
        {
            if (!PrevTab)
                return vfn032() != 0;
            KillFocus();
            Target = PrevTab;
        }
        else
        {
            if (!NextTab)
                return vfn032() != 0;
            KillFocus();
            Target = NextTab;
        }
        if (Target->Wnd)
            Target->Wnd->SetFocus();
    }
    else if (wParam == VK_SHIFT)
    {
        GShiftDown = 1;
    }
    return vfn032() != 0;
}
NW_HOOK_METHOD(0x1002b980, NCWnd, OnKeyDown)

// 0x1002ba00 (vslot 59)
INT NCWnd::OnKeyUp(WPARAM wParam, LPARAM lParam)
{
    if (wParam == VK_SHIFT)
        GShiftDown = 0;
    return vfn032() != 0;
}
NW_HOOK_METHOD(0x1002ba00, NCWnd, OnKeyUp)

// 0x1002b950 (vslot 65) - lParam holds a signed (dx, dy) pair
INT NCWnd::OnMove(WPARAM wParam, LPARAM lParam)
{
    MoveWindow((SWORD)(lParam & 0xFFFF), (SWORD)((DWORD)lParam >> 16));
    return 1;
}
NW_HOOK_METHOD(0x1002b950, NCWnd, OnMove)

/*-----------------------------------------------------------------------------
    Focus
-----------------------------------------------------------------------------*/

// 0x1002b800 (vslot 29)
void NCWnd::SetFocus()
{
    if (GFocusEntry == Entry)
        return;
    if (GFocusEntry)
    {
        GFocusEntry->Wnd->KillFocus();
        GFocusEntry = NULL;
    }
    bFocus = 1;
    GFocusEntry = Entry;
    if (Entry->Parent)
        Entry->Parent->Wnd->BringChildToTop(Entry);
}
NW_HOOK_METHOD(0x1002b800, NCWnd, SetFocus)

// 0x1002b850 (vslot 30)
void NCWnd::KillFocus()
{
    if (GFocusEntry == Entry)
        GFocusEntry = NULL;
    bFocus = 0;
}
NW_HOOK_METHOD(0x1002b850, NCWnd, KillFocus)

// 0x1002b6c0 (vslot 38)
void NCWnd::SetTabOrder(NWndEntry* Next, NWndEntry* Prev)
{
    if (Next)
        NextTab = Next;
    if (Prev)
        PrevTab = Prev;
}
NW_HOOK_METHOD(0x1002b6c0, NCWnd, SetTabOrder)

/*-----------------------------------------------------------------------------
    Visibility and state
-----------------------------------------------------------------------------*/

// 0x1002b6e0 (vslot 39)
void NCWnd::EnableWindow(UBOOL bEnable)
{
    bEnabled = bEnable;
    for (NWndListNode* Node = ChildHead; Node; Node = Node->Next)
        Node->Entry->Wnd->EnableWindow(bEnable);
}
NW_HOOK_METHOD(0x1002b6e0, NCWnd, EnableWindow)

// 0x1002b780 (vslot 40)
UBOOL NCWnd::IsShowWindow()
{
    return (Style & NWS_VISIBLE) ? 1 : 0;
}
NW_HOOK_METHOD(0x1002b780, NCWnd, IsShowWindow)

// 0x1002b710 (vslot 43) - a minimized window's icon goes away when the window shows
void NCWnd::ShowWindow()
{
    if (Style & NWS_MINIMIZABLE)
        Icon->HideWindow();
    Style |= NWS_VISIBLE;
    for (NWndListNode* Node = ChildHead; Node; Node = Node->Next)
        Node->Entry->Wnd->ShowWindow();
}
NW_HOOK_METHOD(0x1002b710, NCWnd, ShowWindow)

// 0x1002b750 (vslot 44)
void NCWnd::HideWindow()
{
    Style &= ~NWS_VISIBLE;
    for (NWndListNode* Node = ChildHead; Node; Node = Node->Next)
        Node->Entry->Wnd->HideWindow();
}
NW_HOOK_METHOD(0x1002b750, NCWnd, HideWindow)

// 0x1002cbf0 (vslot 47) - back from the icon, at the icon's place. C2: the inventory stays an icon while the
// NPC shop (bought from), the warehouse (depositing: mode 0), the trade or the freight window is open
void NCWnd::Restore()
{
    if (Entry->Wnd->IsA(NClass::NCInventoryWnd()))
    {
        NCConsole* Desk = (NCConsole*)GConsole->Desktop;
        if (Desk->ShopWnd->IsShowWindow() && Desk->ShopWnd->Mode == SHOPMODE_Sell)   // 0x100d8b70, the folded GetMode (+0xC0)
            return;
        if (Desk->StoreWnd->IsShowWindow() && Desk->StoreWnd->GetMode() == 0)
            return;
        if (Desk->TradeWnd->IsShowWindow())
            return;
        if (Desk->DeliverWnd->IsShowWindow())
            return;
    }
    SetWindowRect(Icon->ScreenX, Icon->ScreenY, Width, Height);
    Icon->SetWindowRect(0, 0, 0, 0);
    ShowWindow();
    Icon->HideWindow();
}
NW_HOOK_METHOD(0x1002cbf0, NCWnd, Restore)

// 0x1002c640 (vslot 48) - the icon of each minimizable window (texture, SysString id). C2 dropped the GM
// petition window and added the two macro windows
void NCWnd::Minimize()
{
    UObject* Wnd = Entry->Wnd;
    if (Wnd->IsA(NClass::NCInventoryWnd()))
        Icon->SetIcon(TEXT("L2UI.WindowIcon.MainInventory_large"), 0x8a, 1);
    else if (Wnd->IsA(NClass::NCMatchWnd()))
        Icon->SetIcon(TEXT("L2UI.WindowIcon.MainPartyMatching_large"), 0x185, 1);
    else if (Wnd->IsA(NClass::NCSystemMenuWnd()))
        Icon->SetIcon(TEXT("L2UI.WindowIcon.MainSystem_large"), 0xc1, 1);
    else if (Wnd->IsA(NClass::NCBoardWnd()))
        Icon->SetIcon(TEXT("L2UI.WindowIcon.MainCommunity_large"), 0x186, 1);
    else if (Wnd->IsA(NClass::NCMinimapWnd()))
        Icon->SetIcon(TEXT("L2UI.WindowIcon.Minimap_icon"), 0x1bf, 1);
    else if (Wnd->IsA(NClass::NCPetWnd()))
        Icon->SetIcon(TEXT("L2UI.WindowIcon.MainStatus_large"), 0x1fa, 1);
    else if (Wnd->IsA(NClass::NCSummonedWnd()))
        Icon->SetIcon(TEXT("L2UI.WindowIcon.MainStatus_large"), 0x1f9, 1);
    else if (Wnd->IsA(NClass::NCMacroListWnd()))
        Icon->SetIcon(TEXT("L2UI.MacroWnd.Macro_Icon1"), 0x28f, 1);
    else if (Wnd->IsA(NClass::NCMacroEditWnd()))
        Icon->SetIcon(TEXT("L2UI.MacroWnd.Macro_Icon2"), 0x290, 1);
    if (ScreenX < 0)
        Icon->OffsetWindow(-ScreenX, 0);
    HideWindow();
    Icon->ShowWindow();
}
NW_HOOK_METHOD(0x1002c640, NCWnd, Minimize)

// 0x1002ba30 (vslot 50)
void NCWnd::SetTitle(const TCHAR* Title)
{
    if (FrameCtrl && Title && *Title)
        FrameCtrl->SetTitleText(Title);
}
NW_HOOK_METHOD(0x1002ba30, NCWnd, SetTitle)

/*-----------------------------------------------------------------------------
    Geometry
-----------------------------------------------------------------------------*/

// 0x1002b8b0 (vslot 41) - edges included
UBOOL NCWnd::PtInRect(INT X, INT Y, INT RectX, INT RectY, INT RectW, INT RectH)
{
    if (X < RectX || X > RectX + RectW)
        return 0;
    if (Y < RectY || Y > RectY + RectH)
        return 0;
    return 1;
}
NW_HOOK_METHOD(0x1002b8b0, NCWnd, PtInRect)

// 0x1002b8f0 (vslot 27)
UBOOL NCWnd::IsInWindow(INT X, INT Y)
{
    if (!(Style & NWS_VISIBLE))
        return 0;
    return PtInRect(X, Y, ScreenX, ScreenY, Width, Height);
}
NW_HOOK_METHOD(0x1002b8f0, NCWnd, IsInWindow)

// 0x1002b940
NWndEntry* NCWnd::FindWindowAt(INT X, INT Y)
{
    return Entry->Main->FindNWindow(X, Y);
}
NW_HOOK_METHOD(0x1002b940, NCWnd, FindWindowAt)

// 0x1002b870 - screen position of the window and of its whole subtree
void NCWnd::OffsetScreen(INT DX, INT DY)
{
    for (NWndListNode* Node = ChildHead; Node; Node = Node->Next)
        Node->Entry->Wnd->OffsetScreen(DX, DY);
    ScreenX += DX;
    ScreenY += DY;
}
NW_HOOK_METHOD(0x1002b870, NCWnd, OffsetScreen)

// 0x10045d90 (vslot 46)
void NCWnd::OffsetWindow(INT DX, INT DY)
{
    LocalX += DX;
    LocalY += DY;
    OffsetScreen(DX, DY);
}
NW_HOOK_METHOD(0x10045d90, NCWnd, OffsetWindow)

// 0x1002c8b0 (vslot 45) - with the sticky-window option the move snaps to edges
void NCWnd::MoveWindow(INT DX, INT DY)
{
    if (!DX && !DY)
        return;
    POINT Delta;
    Delta.x = DX;
    Delta.y = DY;
    if (GIsStickyWindow)
        Snap(&Delta);
    LocalX += Delta.x;
    LocalY += Delta.y;
    OffsetScreen(Delta.x, Delta.y);
}
NW_HOOK_METHOD(0x1002c8b0, NCWnd, MoveWindow)

// abs() the way the stock code inlines it: (x ^ (x >> 31)) - (x >> 31)
static inline INT SnapAbs(INT X)
{
    return (X ^ (X >> 31)) - (X >> 31);
}

// 0x1002bd50 - sticky windows (GIsStickyWindow): while a NWS_TOPMOST window is dragged, its
// edges stick to the screen edges and to the edges of the other top-most windows closer than
// 10 px, and come loose once the cursor pulls 10 px away. bStuckTop..93 say which edge is stuck
// (top, left, right, bottom), StuckTopTo..8C to which window entry (NULL = screen edge).
// Delta is the move being applied, in and out. C2: the cursor comes from the console's mouse position
// (0x10056130, two INT*), not from GetCursorPos.
void NCWnd::Snap(POINT* Delta)
{
    if (!(GetStyle() & NWS_TOPMOST))
        return;
    const INT H = Height;
    const INT L = ScreenX;
    const INT T = ScreenY;
    const INT W = Width;

    // where the drag grabbed the window: the minimized icon's own grab point, else the
    // title bar's
    INT AnchorX, AnchorY;
    if (IsA(NCIcon::StaticClass()))
    {
        AnchorY = static_cast<NCIcon*>(this)->DragY;
        AnchorX = static_cast<NCIcon*>(this)->DragX;
    }
    else if (FrameCtrl)
    {
        AnchorX = FrameCtrl->GrabX;
        AnchorY = FrameCtrl->GrabY;
    }
    else
        return;

    POINT Cursor;
    GConsole->GetMousePos((INT*)&Cursor.x, (INT*)&Cursor.y);   // 0x10056130

    // stuck to a screen edge: keep still until the cursor is far enough
    if (bStuckLeft && !StuckLeftTo)
    {
        INT D = Cursor.x - AnchorX;
        if (SnapAbs(D) > 9)
        {
            Delta->x = D;
            bStuckLeft = 0;
            return;
        }
        Delta->x = 0;
    }
    if (bStuckTop && !StuckTopTo)
    {
        INT D = Cursor.y - AnchorY;
        if (SnapAbs(D) > 9)
        {
            Delta->y = D;
            bStuckTop = 0;
            return;
        }
        Delta->y = 0;
    }
    if (bStuckRight && !StuckRightTo)
    {
        INT D = Cursor.x - ScreenX - AnchorX;
        if (SnapAbs(D) > 9)
        {
            Delta->x = D;
            bStuckRight = 0;
            return;
        }
        Delta->x = 0;
    }
    if (bStuckBottom && !StuckBottomTo)
    {
        INT D = Cursor.y - ScreenY - AnchorY;
        if (SnapAbs(D) > 10)
        {
            Delta->y = D;
            bStuckBottom = 0;
            return;
        }
        Delta->y = 0;
    }

    // stick to a screen edge
    if (!bStuckLeft && L > -10 && L < 10)
    {
        Delta->x = -L;
        bStuckLeft = 1;
        StuckLeftTo = NULL;
    }
    if (!bStuckTop && T > -10 && T < 10)
    {
        Delta->y = -T;
        bStuckTop = 1;
        StuckTopTo = NULL;
    }
    if (!bStuckRight)
    {
        INT SW = GSEKScreenX;
        if (SW - 10 < W + L && W + L < SW + 10)
        {
            Delta->x = SW - W - L;
            bStuckRight = 1;
            StuckRightTo = NULL;
        }
    }
    if (!bStuckBottom)
    {
        INT SH = GSEKScreenY;
        if (SH - 10 < H + T && H + T < SH + 10)
        {
            Delta->y = SH - H - T;
            bStuckBottom = 1;
            StuckBottomTo = NULL;
        }
    }

    // the other top-most windows (the window itself included, as in the stock code)
    INT Count = Entry->Main->TopMost.Num();
    for (INT i = 0; i < Count; i++)
    {
        NWndEntry* E = Entry->Main->TopMost(i);
        NCWnd* O = E->Wnd;
        const INT OH = O->Height;
        const INT OW = O->Width;
        const INT OL = O->ScreenX;
        const INT OT = O->ScreenY;
        if (!O->IsShowWindow())
            continue;

        // stuck to this window: come loose when the cursor pulls away or the windows no
        // longer overlap along the other axis
        if (bStuckLeft && StuckLeftTo == E)
        {
            INT D = Cursor.x - OW - OL - AnchorX;
            if (SnapAbs(D) > 9 || OH + H < SnapAbs(OT - T + OH) || OH + H < SnapAbs(H - OT + T))
            {
                Delta->x = D;
                bStuckLeft = 0;
                continue;
            }
            Delta->x = 0;
        }
        if (bStuckTop && StuckTopTo == E)
        {
            INT D = Cursor.y - OT - OH - AnchorY;
            if (SnapAbs(D) > 9 || W + OW < SnapAbs(OW - L + OL) || W + OW < SnapAbs(W - OL + L))
            {
                Delta->y = D;
                bStuckTop = 0;
                continue;
            }
            Delta->y = 0;
        }
        if (bStuckRight && StuckRightTo == E)
        {
            INT D = Cursor.x - ScreenX - AnchorX;
            if (SnapAbs(D) > 9 || OH + H < SnapAbs(OT - T + OH) || OH + H < SnapAbs(H - OT + T))
            {
                Delta->x = D;
                bStuckRight = 0;
                continue;
            }
            Delta->x = 0;
        }
        if (bStuckBottom && StuckBottomTo == E)
        {
            INT D = Cursor.y - ScreenY - AnchorY;
            if (SnapAbs(D) > 9 || OW + W < SnapAbs(OW - L + OL) || OW + W < SnapAbs(W - OL + L))
            {
                Delta->y = D;
                bStuckBottom = 0;
                continue;
            }
            Delta->y = 0;
        }

        // stick: left edge to its right edge, top to its bottom, right to its left, bottom to its top
        if (!bStuckLeft && OW - 10 + OL < L && L < OW + 10 + OL && SnapAbs(OT - T + OH) <= H + OH &&
            SnapAbs(H - OT + T) <= H + OH)
        {
            Delta->x = OW - L + OL;
            bStuckLeft = 1;
            StuckLeftTo = E;
        }
        if (!bStuckTop && OT - 10 + OH < T && T < OT + 10 + OH && SnapAbs(OW - L + OL) <= OW + W &&
            SnapAbs(W - OL + L) <= OW + W)
        {
            Delta->y = OT - T + OH;
            bStuckTop = 1;
            StuckTopTo = E;
        }
        if (!bStuckRight && OL - 10 < Width + L && Width + L < OL + 10 && SnapAbs(OT - T + OH) <= OH + H &&
            SnapAbs(H - OT + T) <= OH + H)
        {
            Delta->x = OL - W - L;
            bStuckRight = 1;
            StuckRightTo = E;
        }
        if (!bStuckBottom && OT - 10 < H + T && H + T < OT + 10 && SnapAbs(OW - L + OL) <= OW + W &&
            SnapAbs(W - OL + L) <= OW + W)
        {
            Delta->y = OT - H - T;
            bStuckBottom = 1;
            StuckBottomTo = E;
        }
    }
}
NW_HOOK_METHOD(0x1002bd50, NCWnd, Snap)

// 0x1002c560 (vslot 42)
void NCWnd::SetSize(INT W, INT H)
{
    ResetSticky();
    if ((Style & NWS_TITLEBAR) && FrameCtrl)
        FrameCtrl->SetSize(W, FrameCtrl->LocalH);
    Width = LocalW = W;
    Height = LocalH = H;
}
NW_HOOK_METHOD(0x1002c560, NCWnd, SetSize)

// 0x1002c910 - X, Y relative to the parent; W or H 0 keeps the size. The children move by
// the difference between the new position and the current screen position.
void NCWnd::SetWindowRect(INT X, INT Y, INT W, INT H)
{
    ResetSticky();
    for (NWndListNode* Node = ChildHead; Node; Node = Node->Next)
        Node->Entry->Wnd->OffsetWindow(X - ScreenX, Y - ScreenY);
    ScreenX = X;
    ScreenY = Y;
    if (W && H)
    {
        Width = W;
        Height = H;
    }
    LocalX = ScreenX;
    LocalY = ScreenY;
    LocalW = Width;
    LocalH = Height;
    if (Entry && Entry->Parent)
    {
        NCWnd* Parent = Entry->Parent->Wnd;
        ScreenX += Parent->ScreenX;
        ScreenY += Parent->ScreenY;
    }
}
NW_HOOK_METHOD(0x1002c910, NCWnd, SetWindowRect)

// 0x100031b0 - CreateChild's tail, on the child
void NCWnd::AttachEntry(NWndEntry* E)
{
    Entry = E;
    if (E->Parent)
    {
        NCWnd* Parent = E->Parent->Wnd;
        ScreenX += Parent->ScreenX;
        ScreenY += Parent->ScreenY;
    }
    WindowProc(WM_CREATE, 0, 0);
}
NW_HOOK_METHOD(0x100031b0, NCWnd, AttachEntry)

// 0x1002c4e0 - screen position Origin + (X, Y)
void NCWnd::SetScreenPos(INT X, INT Y, const POINT* Origin)
{
    ResetSticky();
    INT DY = ScreenY - Origin->y;
    OffsetWindow(Origin->x - ScreenX + X, 0);
    OffsetWindow(0, Y - DY);
}
NW_HOOK_METHOD(0x1002c4e0, NCWnd, SetScreenPos)

// 0x1002c5d0 (vslot 49) - horizontally centered in the parent
void NCWnd::CenterInParent()
{
    NWndEntry* E = Entry;
    ResetSticky();
    if (E->Parent && E->Parent->Wnd)
        OffsetWindow((INT)(((DWORD)E->Parent->Wnd->LocalW >> 1) - ((DWORD)Width >> 1)) - LocalX, 0);
}
NW_HOOK_METHOD(0x1002c5d0, NCWnd, CenterInParent)

/*-----------------------------------------------------------------------------
    The four virtuals new in C2 (slots 51-54).
-----------------------------------------------------------------------------*/

// 0x1002baa0 (vslot 51)
void NCWnd::SetItemSelect()
{
    if (!(Style & NWS_ITEMSELECT))
        Style |= NWS_ITEMSELECT;
}
NW_HOOK_METHOD(0x1002baa0, NCWnd, SetItemSelect)

// 0x1002bac0 (vslot 52)
void NCWnd::ClearItemSelect()
{
    if (Style & NWS_ITEMSELECT)
        Style &= ~NWS_ITEMSELECT;
}
NW_HOOK_METHOD(0x1002bac0, NCWnd, ClearItemSelect)

// 0x1002ba60 (vslot 53) - NCVirtualWndMain's vslot 30 (not declared there), then the modal bit
void NCWnd::SetModal(NWndEntry* E)
{
    nw::VirtualCall<void>(Entry->Main, 30, E);
    Style |= NWS_MODAL;
}
NW_HOOK_METHOD(0x1002ba60, NCWnd, SetModal)

// 0x1002ba80 (vslot 54) - NCVirtualWndMain's vslot 31 (not declared there), then the modal bit goes
void NCWnd::ClearModal(NWndEntry* E)
{
    nw::VirtualCall<void>(Entry->Main, 31, E);
    Style &= ~NWS_MODAL;
}
NW_HOOK_METHOD(0x1002ba80, NCWnd, ClearModal)

/*-----------------------------------------------------------------------------
    Children
-----------------------------------------------------------------------------*/

// 0x1002bc80
UBOOL NCWnd::AddChildNode(NWndEntry* Child)
{
    NWndListNode* Node = new NWndListNode;
    Node->Entry = Child;
    if (!ChildTail && !ChildHead)
    {
        ChildHead = Node;
        ChildTail = Node;
        return 1;
    }
    ChildTail->Next = Node;
    Node->Prev = ChildTail;
    ChildTail = Node;
    return 1;
}
NW_HOOK_METHOD(0x1002bc80, NCWnd, AddChildNode)

// 0x1002bce0
void NCWnd::RemoveChildNode(NWndEntry* Child)
{
    NWndListNode* Node = ChildHead;
    while (Node && Node->Entry != Child)
        Node = Node->Next;
    if (!Node)
        return;
    if (Node->Prev)
        Node->Prev->Next = Node->Next;
    if (Node->Next)
        Node->Next->Prev = Node->Prev;
    if (ChildHead == Node)
        ChildHead = Node->Next;
    if (ChildTail == Node)
        ChildTail = Node->Prev;
    delete Node;
}
NW_HOOK_METHOD(0x1002bce0, NCWnd, RemoveChildNode)

// 0x1002b790 - moves the child to the front of this list, then this window to the front of
// its parent's, up to the top level. The list must not be empty (no check in the stock code).
void NCWnd::BringChildToTop(NWndEntry* Child)
{
    NWndListNode* Node = ChildHead;
    if (Node->Entry != Child)
    {
        for (Node = Node->Next; Node && Node->Entry != Child; Node = Node->Next)
        {
        }
        if (Node)
        {
            if (Node->Next)
            {
                Node->Next->Prev = Node->Prev;
                Node->Prev->Next = Node->Next;
            }
            else
            {
                ChildTail = Node->Prev;
                Node->Prev->Next = NULL;
            }
            ChildHead->Prev = Node;
            Node->Next = ChildHead;
            ChildHead = Node;
            Node->Prev = NULL;
        }
    }
    if (Entry->Parent)
        Entry->Parent->Wnd->BringChildToTop(Entry);
}
NW_HOOK_METHOD(0x1002b790, NCWnd, BringChildToTop)

// 0x1002c820
NWndEntry* NCWnd::AddChildWindow(NCWnd* Child, DWORD ChildStyle)
{
    if (!Entry)
        return NULL;
    NWndEntry* E = Entry->Main->RegisterNWindow(Child, ChildStyle, Entry);
    E->Parent = Entry;
    AddChildNode(E);
    return E;
}
NW_HOOK_METHOD(0x1002c820, NCWnd, AddChildWindow)

// 0x1002c860
void NCWnd::RemoveChildWindow(NWndEntry* Child)
{
    if (Child == GFocusEntry)
        GFocusEntry = NULL;
    if (!(Style & NWS_NOTINPARENTLIST))
        RemoveChildNode(Child);
    Entry->Main->UnRegisterNWindow(Child);
}
NW_HOOK_METHOD(0x1002c860, NCWnd, RemoveChildWindow)

/*-----------------------------------------------------------------------------
    Helpers
-----------------------------------------------------------------------------*/

// 0x1002bae0 - "1234567" -> "1,234,567" (does not use the window)
void NCWnd::FormatNumber(TCHAR* Out, const TCHAR* Digits)
{
    if (!Digits || !*Digits)
        return;
    INT Len = (INT)wcslen(Digits);
    INT i = Len - 1;
    INT OutLen = i / 3 + Len;
    Out[OutLen] = 0;
    TCHAR* Dst = &Out[OutLen - 1];
    for (INT n = 0; i > -1; i--, n++)
    {
        if (n % 3 == 0 && i < (INT)wcslen(Digits) - 1)
            *Dst-- = ',';
        *Dst-- = Digits[i];
    }
}
NW_HOOK_METHOD(0x1002bae0, NCWnd, FormatNumber)

// NCCmdTarget's own vtable is the one its destructor stores (its constructor is NCObject's,
// folded by the linker). The original's has 37 slots: 24..36 are declared in NCWnd here.
NW_CHECK_VTABLE(NCCmdTarget, 0x10108040, 38)
NW_CHECK_VTABLE(NCWnd, 0x101080d8, 67)
