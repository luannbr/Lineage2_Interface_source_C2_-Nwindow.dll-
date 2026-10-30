/*=============================================================================
    NConsoleWndResize.cpp - the screen changed size: every window gets the place it has at the new size
    (NConsoleWnd::ResizeWindows, virtual 314, called by the viewport's ResizeViewport in WinDrv after it
    stored GSEKScreenX / GSEKScreenY).
=============================================================================*/
#include "nwindow/NConsoleWnd.h"
#include "nwindow/NCConsole.h"
#include "nwindow/NCWnd.h"
#include "nwindow/NCAbnormalStatusWnd.h"
#include "nwindow/NCIcon.h"

// 0x10056500 - a window of the desktop at W0 x H0 keeps its place in proportion when the desktop is W1 x H1: its distances
// to the left and the top edge (clamped to the old desktop) are kept in the ratio to the total slack on each axis (the
// distance to the edge and to the opposite edge), and the slack of the new desktop (new size minus the window's) is shared
// the same way. The slack of the new desktop is converted as an unsigned number (the stock does: a window larger than the
// new desktop gets a huge offset)
void NConsoleWnd::ScaleRect(INT W0, INT H0, INT W1, INT H1, NResizeRect* Rect)
{
    INT X = Rect->X;
    INT Y = Rect->Y;
    INT Right = W0 - Rect->W - X;
    INT Bottom = H0 - Rect->H - Y;
    if (X <= 0)
        X = 0;
    if (X >= W0)
        X = W0;
    if (Right <= 0)
        Right = 0;
    if (Right >= W0)
        Right = W0;
    if (Y <= 0)
        Y = 0;
    if (Y >= H0)
        Y = H0;
    if (Bottom <= 0)
        Bottom = 0;
    if (Bottom >= H0)
        Bottom = H0;
    // VC 7.0's _ftol (0x100e7998) converts to 64 bits and returns the low dword: the huge value of a negative difference
    // wraps back (ratio 1) instead of overflowing, so ours goes through __int64 too
    Rect->X = (INT)(__int64)((DOUBLE)X / (DOUBLE)(Right + X) * (DOUBLE)(UINT)(W1 - Rect->W));
    Rect->Y = (INT)(__int64)((DOUBLE)Y / (DOUBLE)(Bottom + Y) * (DOUBLE)(UINT)(H1 - Rect->H));
}
NW_HOOK_METHOD(0x10056500, NConsoleWnd, ScaleRect)

// 0x1005ddb0 (vslot 314, name ours) - the windows that were iconified come back (the stock restores them: the icon
// is shown while the window is minimized); the login screen, the message line, the option and restart windows, the
// character selection and creation and the loading and credit screens are placed for the new size (fixed distances
// from the edges, or the old place in proportion for the children of the lobby and the creation screen); the
// desktop gets the screen's size; then every window of the desktop keeps its place in proportion (0x10056500 maps the
// rectangle from the old desktop size to the new, SetScreenPos puts the window there) and the buffs window
// UpdateSize's. The order is the stock's
void NConsoleWnd::ResizeWindows()
{
    if (!Desktop || (!GSEKScreenX && !GSEKScreenY))
        return;
    // 1) the windows shown as their icon are restored
    static const DWORD Iconified[8] = { 0x98, 0x10C, 0xF0, 0x118, 0x114, 0x124, 0x128, 0xCC };
    for (INT i = 0; i < 8; i++)
    {
        NCWnd* Wnd = nw::Field<NCWnd*>(Desktop, Iconified[i]);
        if (Wnd->Icon->IsShowWindow())
            nw::Field<NCWnd*>(Desktop, Iconified[i])->Restore();
    }
    // the desktop's old size and place (the origin the windows' SetScreenPos is relative to)
    INT W0 = Desktop->LocalW;
    INT H0 = Desktop->LocalH;
    POINT Origin;
    Origin.x = Desktop->LocalX;
    Origin.y = Desktop->LocalY;
    // 2) the login screen
    NCWnd* Login = nw::Field<NCWnd*>(this, 0x3528);
    if (Login)
    {
        Login->SetSize(GSEKScreenX, GSEKScreenY - 0x34);
        Origin.x = Login->LocalX;
        Origin.y = Login->LocalY;
        nw::Field<NCWnd*>(Login, 0x98)->SetScreenPos(GSEKScreenX / 2 - 0x68, GSEKScreenY / 2 - 0x35, &Origin);
        nw::Field<NCWnd*>(Login, 0xBC)->SetScreenPos(GSEKScreenX - 0x84, GSEKScreenY - 0x94, &Origin);
        nw::Field<NCWnd*>(Login, 0xC0)->SetScreenPos(GSEKScreenX - 0x84, GSEKScreenY - 0x7C, &Origin);
        nw::Field<NCWnd*>(Login, 0xC4)->SetScreenPos(GSEKScreenX - 0x84, GSEKScreenY - 0x64, &Origin);
        nw::Field<NCWnd*>(Login, 0xC8)->SetScreenPos(GSEKScreenX - 0x84, GSEKScreenY - 0x4C, &Origin);
        nw::Field<NCWnd*>(Login, 0x9C)->SetScreenPos(GSEKScreenX / 2 - 0x10A, GSEKScreenY / 2 - 0xD5, &Origin);
        nw::Field<NCWnd*>(Login, 0xA4)->SetScreenPos(GSEKScreenX / 2 - 0x10A, GSEKScreenY / 2 - 0xD5, &Origin);
        nw::Field<NCWnd*>(Login, 0xA8)->SetScreenPos(GSEKScreenX / 2 - 0x10A, GSEKScreenY / 2 - 0xD5, &Origin);
        nw::Field<NCWnd*>(Login, 0xAC)->SetScreenPos(GSEKScreenX / 2 - 0x96, GSEKScreenY / 2 - 0x71, &Origin);
    }
    // 3) the message line, the option window and the restart menu
    NCWnd* Message = nw::Field<NCWnd*>(this, 0x353C);
    if (Message)
    {
        Message->SetSize(GSEKScreenX, 0x34);
        Message->SetScreenPos(0, GSEKScreenY - 0x34, &Origin);
    }
    NCWnd* Option = nw::Field<NCWnd*>(this, 0x3524);
    if (Option)
        Option->SetScreenPos(GSEKScreenX / 2 - 0x78, GSEKScreenY / 2 - 0xB6, &Origin);
    NCWnd* Restart = nw::Field<NCWnd*>(this, 0x3520);
    if (Restart)
        Restart->SetWindowRect(GSEKScreenX / 2 - 0x68, GSEKScreenY / 2 - 0x4C, 0xD0, 0x98);
    // 4) the character selection: its two big children keep their place in proportion (the old desktop size is the
    // divisor), the four buttons are a fixed distance from the corner
    NCWnd* Lobby = nw::Field<NCWnd*>(this, 0x3530);
    if (Lobby)
    {
        Lobby->SetSize(GSEKScreenX, GSEKScreenY - 0x34);
        FLOAT fH0 = (FLOAT)H0;
        FLOAT fW0 = (FLOAT)W0;
        Origin.x = Lobby->LocalX;
        Origin.y = Lobby->LocalY;
        NCWnd* Child = nw::Field<NCWnd*>(Lobby, 0x98);
        INT Y = (INT)((DOUBLE)Child->LocalY / fH0 * GSEKScreenY);
        INT X = (INT)((DOUBLE)Child->LocalX / fW0 * GSEKScreenX);
        Child->SetScreenPos(X, Y, &Origin);
        Child = nw::Field<NCWnd*>(Lobby, 0x9C);
        Y = (INT)((DOUBLE)Child->LocalY / fH0 * GSEKScreenY);
        X = (INT)((DOUBLE)Child->LocalX / fW0 * GSEKScreenX);
        Child->SetScreenPos(X, Y, &Origin);
        nw::Field<NCWnd*>(Lobby, 0xA8)->SetScreenPos(GSEKScreenX - 0x83, GSEKScreenY - 0xAC, &Origin);
        nw::Field<NCWnd*>(Lobby, 0xAC)->SetScreenPos(GSEKScreenX - 0x83, GSEKScreenY - 0x94, &Origin);
        nw::Field<NCWnd*>(Lobby, 0xA0)->SetScreenPos(GSEKScreenX - 0x83, GSEKScreenY - 0x7C, &Origin);
        nw::Field<NCWnd*>(Lobby, 0xA4)->SetScreenPos(GSEKScreenX - 0x83, GSEKScreenY - 0x64, &Origin);
    }
    // 5) the character creation: the same, with the proportions as floats of the first child's (the second is 244 lower)
    NCWnd* Create = nw::Field<NCWnd*>(this, 0x3534);
    if (Create)
    {
        Create->SetSize(GSEKScreenX, GSEKScreenY - 0x34);
        Origin.x = Create->LocalX;
        Origin.y = Create->LocalY;
        NCWnd* Child = nw::Field<NCWnd*>(Create, 0x98);
        FLOAT PropY = (FLOAT)((DOUBLE)Child->LocalY / H0);
        FLOAT PropX = (FLOAT)((DOUBLE)Child->LocalX / W0);
        INT Y = (INT)((DOUBLE)GSEKScreenY * PropY);
        INT X = (INT)((DOUBLE)GSEKScreenX * PropX);
        Child->SetScreenPos(X, Y, &Origin);
        Y = (INT)((DOUBLE)GSEKScreenY * PropY + 244.0f);
        X = (INT)((DOUBLE)GSEKScreenX * PropX);
        nw::Field<NCWnd*>(Create, 0x9C)->SetScreenPos(X, Y, &Origin);
        nw::Field<NCWnd*>(Create, 0xF0)->SetScreenPos(GSEKScreenX - 0x83, GSEKScreenY - 0x7C, &Origin);
        nw::Field<NCWnd*>(Create, 0xF4)->SetScreenPos(GSEKScreenX - 0x83, GSEKScreenY - 0x64, &Origin);
        nw::Field<NCWnd*>(Create, 0x100)->SetScreenPos(GSEKScreenX / 2 - 0x16, GSEKScreenY - 0x64, &Origin);
        nw::Field<NCWnd*>(Create, 0xF8)->SetScreenPos(GSEKScreenX / 2 - 0x58, GSEKScreenY - 0x64, &Origin);
        nw::Field<NCWnd*>(Create, 0xFC)->SetScreenPos(GSEKScreenX / 2 + 0x2C, GSEKScreenY - 0x64, &Origin);
        nw::Field<NCWnd*>(Create, 0x104)->SetScreenPos(GSEKScreenX / 2 - 0x16, GSEKScreenY - 0x64, &Origin);
    }
    // 6) the loading and credit screens and the desktop have the screen's size
    NCWnd* Loading = nw::Field<NCWnd*>(this, 0x3538);
    if (Loading)
        Loading->SetSize(GSEKScreenX, GSEKScreenY);
    NCWnd* Credit = nw::Field<NCWnd*>(this, 0x352C);
    if (Credit)
        Credit->SetSize(GSEKScreenX, GSEKScreenY);
    Desktop->SetSize(GSEKScreenX, GSEKScreenY);
    // 7) the windows of the desktop: the chat window at the bottom edge, the others where they were in proportion
    POINT DeskOrigin;
    DeskOrigin.x = Desktop->LocalX;
    DeskOrigin.y = Desktop->LocalY;
    nw::Field<NCWnd*>(Desktop, 0xA4)->SetScreenPos(0, GSEKScreenY - nw::Field<NCWnd*>(Desktop, 0xA4)->LocalH, &DeskOrigin);
#define NW_RESCALE(Window)                                                                                       \
    {                                                                                                            \
        NCWnd* Wnd = (Window);                                                                                   \
        NResizeRect Rect;                                                                                        \
        Rect.X = Wnd->ScreenX;                                                                                   \
        Rect.Y = Wnd->ScreenY;                                                                                   \
        Rect.W = Wnd->Width;                                                                                     \
        Rect.H = Wnd->Height;                                                                                    \
        ScaleRect(W0, H0, GSEKScreenX, GSEKScreenY, &Rect);                                                       \
        Wnd->SetScreenPos(Rect.X, Rect.Y, &DeskOrigin);                                                          \
    }
#define NW_RESCALE_DESK(Offset) NW_RESCALE(nw::Field<NCWnd*>(Desktop, Offset))
    NW_RESCALE_DESK(0xA8)
    NW_RESCALE_DESK(0xAC)
    NW_RESCALE_DESK(0x104)
    NW_RESCALE_DESK(0xCC)
    NW_RESCALE_DESK(0x98)
    NW_RESCALE_DESK(0xD4)
    NW_RESCALE_DESK(0xD0)
    NW_RESCALE_DESK(0xD8)
    nw::Field<NCAbnormalStatusWnd*>(Desktop, 0xD8)->UpdateSize();
    NW_RESCALE_DESK(0x170)
    NW_RESCALE_DESK(0x174)
    NW_RESCALE_DESK(0x114)
    NW_RESCALE_DESK(0x11C)
    NW_RESCALE_DESK(0x118)
    NW_RESCALE_DESK(0x10C)
    NW_RESCALE_DESK(0xDC)
    NW_RESCALE_DESK(0xE0)
    NW_RESCALE_DESK(0xF4)
    NW_RESCALE_DESK(0xEC)
    NW_RESCALE_DESK(0xF0)
    NW_RESCALE_DESK(0x188)
    NW_RESCALE_DESK(0x18C)
    NW_RESCALE_DESK(0xC0)
    NW_RESCALE_DESK(0xC4)
    NW_RESCALE_DESK(0xC8)
    NW_RESCALE_DESK(0x120)
    NW_RESCALE_DESK(0x108)
    NW_RESCALE(nw::Field<NCWnd*>(this, 0x3550))
    NW_RESCALE_DESK(0x124)
    NW_RESCALE_DESK(0x128)
    NW_RESCALE_DESK(0x154)
    NW_RESCALE_DESK(0x158)
    NW_RESCALE_DESK(0x15C)
    NW_RESCALE_DESK(0x160)
    NW_RESCALE_DESK(0x164)
    NW_RESCALE_DESK(0x168)
    NW_RESCALE_DESK(0x16C)
    NW_RESCALE_DESK(0xB8)
    NW_RESCALE_DESK(0x9C)
    NW_RESCALE_DESK(0xA0)
    NW_RESCALE_DESK(0xBC)
    NW_RESCALE_DESK(0x198)
    NW_RESCALE_DESK(0xFC)
    NW_RESCALE_DESK(0xF8)
    NW_RESCALE_DESK(0x100)
    NW_RESCALE_DESK(0x134)
    NW_RESCALE_DESK(0x144)
    NW_RESCALE_DESK(0x148)
    NW_RESCALE_DESK(0x13C)
    NW_RESCALE_DESK(0x140)
    NW_RESCALE_DESK(0x138)
    NW_RESCALE_DESK(0xE4)
#undef NW_RESCALE_DESK
#undef NW_RESCALE
}
NW_HOOK_METHOD(0x1005ddb0, NConsoleWnd, ResizeWindows)
