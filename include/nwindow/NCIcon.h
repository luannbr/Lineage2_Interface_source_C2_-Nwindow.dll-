#pragma once
// NCIcon - the 32x32 icon a minimized window leaves on the screen (UClass "CIcon", 0x2C0).
// NCWnd::OnCreate creates one for NWS_MINIMIZABLE windows; NCWnd::Minimize picks its texture
// and tool tip. It can be dragged (WM_MOVE to itself, sticky like windows); a click without a
// drag restores the window.

#include "nwindow/NControl.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCIcon : public NControl
{
    NW_DECLARE_CLASS(NCIcon, NControl, 0, 0x101A2250)

    UTexture* IconTex;         // +0x1A4
    INT DragX;                 // +0x1A8 where the drag grabbed the icon (NCWnd::Snap)
    INT DragY;                 // +0x1AC
    FString Name;              // +0x1B0 the tool tip text (C1: TCHAR Name[128])
    BYTE bMouseOver;           // +0x1BC
    INT Unknown1C0;            // +0x1C0 (-1; only the constructor writes it; C1 name Unknown2B0)
    UBOOL bToolTipAbove;       // +0x1C4 (SetIcon)
    UTexture* ToolTipTex;      // +0x1C8 "NWindow.ChatBack"
    UBOOL bDragged;            // +0x1CC moved since the button went down

    NCIcon();                                                        // 0x100190a0
    // ~NCIcon                                                       // 0x10019180

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x10018e90
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x100191f0
    virtual INT OnDrawToolTip(NCGDevice* Device);                    // 56 0x100192c0
    virtual INT OnMouseMove(WPARAM wParam, LPARAM lParam);           // 60 0x10018f50
    virtual INT OnLButtonDown(WPARAM wParam, LPARAM lParam);         // 61 0x10019000
    virtual INT OnLButtonUp(WPARAM wParam, LPARAM lParam);           // 62 0x10019040
    // slot 68, new in C2 (name ours; WM_LBUTTONDBLCLK goes there): the stock body is `mov eax, 1; ret 8`
    // folded with the other classes' (0x100d2e70)
    virtual INT OnLButtonDblClick(WPARAM wParam, LPARAM lParam) { return 1; }  // 68 0x100d2e70 (folded)

    void SetIcon(const TCHAR* TexName, INT NameID, BYTE bAbove);     // 0x10018ef0 (name ours)
};
NW_ASSERT_OFFSET(NCIcon, Name, 0x1B0);
NW_ASSERT_OFFSET(NCIcon, bMouseOver, 0x1BC);
NW_ASSERT_OFFSET(NCIcon, bToolTipAbove, 0x1C4);
NW_ASSERT_OFFSET(NCIcon, bDragged, 0x1CC);
NW_ASSERT_SIZE(NCIcon, 0x1D0);

#pragma pack(pop)