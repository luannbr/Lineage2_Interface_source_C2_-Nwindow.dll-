#pragma once
// NCAbstractMain / NCVirtualWndMain - owner of every window: registration, z-order lists,
// hit tests, focus, message dispatch, cursors (UClass "CAbstractMain", "CVirtualWndMain").
//
// Input path: the console calls DispatchWndMsg with the Win32 message; mouse coordinates are
// made screen-relative (ClientToScreenPacked), then the top-level windows are tried from the
// front (TopLevel is painted from index 0, so the last one is on top): clicks go to the deepest
// window hit (RouteClick, which also gives it the focus), mouse moves to the one under the
// cursor or to the focused one while the left button is held (RouteMouseMove), keyboard and IME
// messages to the focused window (SendToFocused). WM_PAINT paints each tree back to front and
// then lets the window under the cursor draw its tool tip.

#include "nwindow/NCWnd.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class UTexture;

class NCAbstractMain : public NCObject
{
    NW_DECLARE_ABSTRACT_CLASS(NCAbstractMain, NCObject, 0, 0x101A8230)
    // constructor: NCObject's (0x10020ca0, identical code folded); ~NCAbstractMain 0x10024690

    virtual NWndEntry* RegisterNWindow(NCWnd* Wnd, DWORD Style, NWndEntry* Parent) = 0;  // 25
    virtual void UnRegisterNWindow(NWndEntry* Entry) = 0;                                 // 26
    virtual NWndEntry* FindNWindow(INT X, INT Y) = 0;                                     // 27
    virtual void ClearFocus() = 0;                                                        // 28
    virtual void AddTopLevel(NWndEntry* Entry) = 0;                                       // 29 (name ours)
    // new in C2 (names ours; bodies 0x1002ef60 / 0x1002ef90 / 0x1002d410): a stack of windows that took the
    // focus for themselves (the array at +0x48 of NCVirtualWndMain): the last one gets the focus back when
    // another leaves
    virtual void PushFocusWindow(NWndEntry* Entry) = 0;                                   // 30
    virtual void PopFocusWindow(NWndEntry* Entry) = 0;                                    // 31
    virtual INT NumFocusWindows() = 0;                                                    // 32
};
NW_ASSERT_SIZE(NCAbstractMain, 0x28);

class NCVirtualWndMain : public NCAbstractMain
{
    NW_DECLARE_CLASS(NCVirtualWndMain, NCAbstractMain, 0, 0x101A7D10)

    NCGDevice* GDevice;              // +0x28 created by the constructor
    DWORD NumWindows;                // +0x2C registered windows (RegisterNWindow refuses 1000+)
    TArray<NWndEntry*> Windows;      // +0x30 every registered window
    TArray<NWndEntry*> TopLevel;     // +0x3C windows without a parent, back to front
    TArray<NWndEntry*> FocusWindows; // +0x48 new in C2 (name ours): the stack PushFocusWindow / PopFocusWindow keep
    TArray<NWndEntry*> TopMost;      // +0x54 windows registered with NWS_TOPMOST
    HCURSOR Cursors[7];              // +0x60 from this package's resources (LoadCursors)
    HCURSOR ArrowCursor;             // +0x7C IDC_ARROW
    INT CursorType;                  // +0x80 index in Cursors, 4 = color cursor, -1 = arrow
    UBOOL bCursorEnabled;            // +0x84
    HCURSOR ColorCursor;             // +0x88 built from a texture (CreateColorCursor)
    UBOOL bColorCursor;              // +0x8C ColorCursor is ours to destroy
    UBOOL bMouseMoveTaken;           // +0x90 a window already took this WM_MOUSEMOVE
    UBOOL bToolTipShown;             // +0x94 a window already drew its tool tip this frame

    NCVirtualWndMain();                                                    // 0x1002f080
    // ~NCVirtualWndMain                                                   // 0x1002f1c0

    virtual NWndEntry* RegisterNWindow(NCWnd* Wnd, DWORD Style, NWndEntry* Parent); // 25 0x1002e4d0
    virtual void UnRegisterNWindow(NWndEntry* Entry);                      // 26 0x1002e610
    virtual NWndEntry* FindNWindow(INT X, INT Y);                          // 27 0x1002dee0
    virtual void ClearFocus();                                             // 28 0x1002ddb0
    virtual void AddTopLevel(NWndEntry* Entry);                            // 29 0x1002e4a0 (name ours)
    virtual void PushFocusWindow(NWndEntry* Entry);                        // 30 0x1002ef60 (name ours)
    virtual void PopFocusWindow(NWndEntry* Entry);                         // 31 0x1002ef90 (name ours)
    virtual INT NumFocusWindows();                                         // 32 0x1002d410 (name ours)
    virtual NWndEntry* FindChildWindowAt(NCWnd* Wnd, INT X, INT Y);        // 33 0x1002de50 (name ours)

    // names ours unless marked
    UBOOL IsDispatchedMsg(UINT Msg);                                       // 0x1002d040
    void SetGDevice(UCanvas* Canvas);                                      // 0x1002d070 (guard name)
    void ReleaseGDevice();                                                 // 0x1002d0f0 (guard name)
    LPARAM ClientToScreenPacked(LPARAM Pos);                               // no C2 function: DispatchWndMsg inlines it (C1 0x100243c0)
    LPARAM CoordCompensate(LPARAM Pos);                                    // 0x1002d4e0 (guard name)
    INT DispatchToFocusWindow(NWndEntry* Entry, UINT Msg, WPARAM wParam, LPARAM lParam); // 0x1002d9f0 (name ours)
    UBOOL GetMousePos(INT* X, INT* Y);                                     // 0x1002e390 (name ours)
    INT SendToFocused(NWndEntry* Entry, UINT Msg, WPARAM wParam, LPARAM lParam); // 0x1002d170
    void PaintChildren(NWndEntry* Entry);                                  // 0x1002d2e0
    void LoadCursors();                                                    // 0x1002d320
    void DestroyCursors();                                                 // 0x1002d3a0
    void DeleteGDevice();                                                  // 0x1002d3f0
    UBOOL HitTestClick(NWndEntry* Entry, INT X, INT Y);                    // 0x1002d5b0
    INT RouteClick(NWndEntry* Entry, INT X, INT Y, UINT Msg, WPARAM wParam, LPARAM lParam); // 0x1002d710
    UBOOL HitTest(NWndEntry* Entry, LPARAM Pos);                           // 0x1002d860
    INT RouteMouseMove(NWndEntry* Entry, UINT Msg, WPARAM wParam, LPARAM lParam); // 0x1002d8f0
    void PaintToolTips(NWndEntry* Entry);                                  // 0x1002e790 (name ours)
    INT DispatchWndMsg(UINT Msg, WPARAM wParam, LPARAM lParam);            // 0x1002e8a0 (guard name)
    void CreateColorCursor(UTexture* Texture);                             // 0x1002dfd0
    void SetCursorType(INT Type, UTexture* Texture);                       // 0x1002e310
    UBOOL IsOverWindow(INT X, INT Y);                                      // 0x1002e3e0
};
NW_ASSERT_OFFSET(NCVirtualWndMain, Windows, 0x30);
NW_ASSERT_OFFSET(NCVirtualWndMain, TopLevel, 0x3C);
NW_ASSERT_OFFSET(NCVirtualWndMain, FocusWindows, 0x48);
NW_ASSERT_OFFSET(NCVirtualWndMain, TopMost, 0x54);
NW_ASSERT_OFFSET(NCVirtualWndMain, Cursors, 0x60);
NW_ASSERT_OFFSET(NCVirtualWndMain, ArrowCursor, 0x7C);
NW_ASSERT_OFFSET(NCVirtualWndMain, ColorCursor, 0x88);
NW_ASSERT_OFFSET(NCVirtualWndMain, bToolTipShown, 0x94);
NW_ASSERT_SIZE(NCVirtualWndMain, 0x98);

#pragma pack(pop)