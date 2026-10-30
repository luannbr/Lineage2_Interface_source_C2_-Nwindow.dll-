#pragma once
// NCFrameCtrl - title bar of a window (UClass "CFrameCtrl", 0x3D4) and NCResizeFrame, the
// edge that resizes a window (UClass "CResizeFrame", 0x3E8).
//
// Dragging the bar sends WM_MOVE(delta) to the parent, which moves (and snaps). A decorated
// frame paints the three title bar textures and the title, and adds a minimize button
// (parent NWS_MINIMIZABLE) and a close button (unless NWS_NOCLOSE); NCWnd::OnCreate makes
// undecorated ones: an invisible 15 px drag area above windows with NWS_TITLEBAR.

#include "nwindow/NControl.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCButton;

// NWM_COMMAND ids of the frame buttons
enum { NID_FRAME_MINIMIZE = 0xF0F0, NID_FRAME_CLOSE = 0xF0F1 };

class NCFrameCtrl : public NControl
{
    NW_DECLARE_CLASS(NCFrameCtrl, NControl, 0, 0x1019CE20)

    FString Title;             // +0x1A4 the window's title (C1: TCHAR Title[256], longer ones were ignored)
    UTexture* BlackTex;        // +0x1B0 "NWindow.BlackTexture" (not drawn)
    UTexture* TitleLeftTex;    // +0x1B4 "L2UI.NWindow.WindowTitlebar1", 24x21
    UTexture* TitleMidTex;     // +0x1B8 "L2UI.NWindow.WindowTitlebar2", stretched
    UTexture* TitleRightTex;   // +0x1BC "L2UI.NWindow.WindowTitlebar3", 8x21
    NCButton* CloseButton;     // +0x1C0
    NCButton* MinimizeButton;  // +0x1C4
    INT LastMouseX;            // +0x1C8 mouse position of the drag
    INT LastMouseY;            // +0x1CC
    INT TitleX;                // +0x1D0 (28)
    INT TitleY;                // +0x1D4 title centered vertically
    INT GrabX;                 // +0x1D8 where the drag grabbed the bar (NCWnd::Snap)
    INT GrabY;                 // +0x1DC
    UBOOL bDecorated;          // +0x1E0 textures, title and buttons

    NCFrameCtrl();                                                   // 0x1000ea80
    NCFrameCtrl(UBOOL bInDecorated);                                 // 0x1000eb80
    // ~NCFrameCtrl                                                  // 0x1000e9e0

    virtual __declspec(noinline) INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x1000e700
    virtual INT OnCreate();                                          // 36 0x1000ed90
    virtual INT OnDestroy();                                         // 37 0x1000ef40
    virtual void SetSize(INT W, INT H);                              // 42 0x1000e7a0
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x1000f010
    virtual INT OnMouseMove(WPARAM wParam, LPARAM lParam);           // 60 0x1000e7b0
    virtual INT OnLButtonDown(WPARAM wParam, LPARAM lParam);         // 61 0x1000e820
    virtual INT OnLButtonUp(WPARAM wParam, LPARAM lParam);           // 62 0x1000e880
    virtual INT OnClose();                                           // 68 0x1000ec80 (name ours)
    virtual INT OnMinimize();                                        // 69 0x1000e780 (name ours)

    void SetTitleText(const TCHAR* InTitle);                         // 0x1000e8a0 (name ours)
};
NW_ASSERT_OFFSET(NCFrameCtrl, Title, 0x1A4);
NW_ASSERT_OFFSET(NCFrameCtrl, BlackTex, 0x1B0);
NW_ASSERT_OFFSET(NCFrameCtrl, CloseButton, 0x1C0);
NW_ASSERT_OFFSET(NCFrameCtrl, GrabX, 0x1D8);
NW_ASSERT_OFFSET(NCFrameCtrl, bDecorated, 0x1E0);
NW_ASSERT_SIZE(NCFrameCtrl, 0x1E4);

// resize direction: which mouse delta goes to the parent (NWM_COMMAND, lParam = delta)
enum { RESIZE_Horizontal = 1, RESIZE_Vertical = 2 };

class NCResizeFrame : public NCFrameCtrl
{
    NW_DECLARE_CLASS(NCResizeFrame, NCFrameCtrl, 0, 0x1019D340)

    UBOOL bPressed;            // +0x1E4 cleared with bDragging, never read here
    INT Direction;             // +0x1E8 RESIZE_*
    INT Unknown3DC;            // +0x1EC
    UBOOL bDragging;           // +0x1F0 button went down inside
    UBOOL bCursorLocked;       // +0x1F4 resize cursor shown and locked

    NCResizeFrame(INT InDirection = 0);                              // 0x1000f170
    // ~NCResizeFrame                                                // 0x1000f1e0

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x1000e910
    virtual void KillFocus();                                        // 30 0x1000e9c0
    virtual INT OnMouseMove(WPARAM wParam, LPARAM lParam);           // 60 0x1000f230
    virtual INT OnLButtonDown(WPARAM wParam, LPARAM lParam);         // 61 0x1000e930
    virtual INT OnLButtonUp(WPARAM wParam, LPARAM lParam);           // 62 0x1000e9a0
};
NW_ASSERT_OFFSET(NCResizeFrame, Direction, 0x1E8);
NW_ASSERT_OFFSET(NCResizeFrame, bCursorLocked, 0x1F4);
NW_ASSERT_SIZE(NCResizeFrame, 0x1F8);

#pragma pack(pop)