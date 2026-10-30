#pragma once
// NCButton and its subclasses (UClass "CButton" 0x21C, "CExButton", "CPushButton",
// "CRadioButton").
//
// A button paints NormalTex, or PushedTex while bReleased is 0, stretched over its rectangle,
// with its label centered (a SysString index, or Text). Clicks play interface sound 0 (2 when
// disabled) and report NWM_COMMAND(ID, NotifyID) to the parent on release inside the button.
// The tool tip (ToolTipID, a SysString index) is drawn above the button, or below it, while
// the mouse is over it.
//   NCExButton     reports 1 on press and 0 on release instead.
//   NCPushButton   reports on press and, while held, again every RepeatDelay ms (timer).
//   NCRadioButton  stays down once clicked; reports Value.

#include "nwindow/NControl.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCButton : public NControl
{
    NW_DECLARE_CLASS(NCButton, NControl, 0, 0x10197BC0)

    UBOOL bReleased;           // +0x1A4 0 while pressed (paints PushedTex)
    UBOOL bMouseOver;          // +0x1A8 set by OnMouseMove, shows the tool tip
    UBOOL bToolTipAbove;       // +0x1AC 1 = tool tip above the button, 0 = below
    UTexture* NormalTex;       // +0x1B0
    UTexture* PushedTex;       // +0x1B4
    UTexture* ToolTipTex;      // +0x1B8 "L2UI.tooltip.mini-tool01", stretched under the tip
    INT NormalTexH;            // +0x1BC VSize of NormalTex
    INT NormalTexW;            // +0x1C0 USize
    INT PushedTexH;            // +0x1C4
    INT PushedTexW;            // +0x1C8
    UBOOL bAcceptKey;          // +0x1CC 1: Enter / Space click it while it has the focus
    NToolTipInfo ToolTip;      // +0x1D0 new in C2 (name ours): the tool tip's data, filled by SetToolTip; OnDrawToolTip
                               //   hands a copy of it to the window's tool tip skin (NCWnd::ToolTipSkin)
    FString Text;              // +0x248 label when TextID < 0 (C1: TCHAR Text[32])
    INT TextID;                // +0x254 SysString index of the label, -1 = Text
    INT ToolTipID;             // +0x258 SysString index of the tool tip, -1 = none
    INT TextOffsetX;           // +0x25C label offset from the center
    INT TextOffsetY;           // +0x260

    NCButton();                                                                 // 0x10002340
    NCButton(const TCHAR* NormalTexName, const TCHAR* PushedTexName, UBOOL bInAcceptKey); // 0x10002440
    // ~NCButton                                                                // 0x10002650

    virtual __declspec(noinline) INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam); // 25 0x10090b60
    virtual INT OnDestroy();                                                    // 37 0x10001140 (folded)
    virtual INT OnPaint(NCGDevice* Device);                                     // 55 0x10001ca0
    virtual INT OnDrawToolTip(NCGDevice* Device);                               // 56 0x10002710
    virtual INT OnKeyDown(WPARAM wParam, LPARAM lParam);                        // 58 0x100011d0
    virtual INT OnMouseMove(WPARAM wParam, LPARAM lParam);                      // 60 0x100010b0
    virtual INT OnLButtonDown(WPARAM wParam, LPARAM lParam);                    // 61 0x10001000
    virtual INT OnLButtonUp(WPARAM wParam, LPARAM lParam);                      // 62 0x10001060

    // names ours
    void SetTextID(INT ID);                                                     // 0x10001220
    void SetText(const TCHAR* InText);                                          // 0x10001250
    UBOOL SetNormalTex(const TCHAR* Name);                                      // 0x10001280
    UBOOL SetPushedTex(const TCHAR* Name);                                      // 0x100012d0
    void SetToolTip(INT ID, UBOOL bAbove);                                      // 0x10001320
};
NW_ASSERT_OFFSET(NCButton, bReleased, 0x1A4);
NW_ASSERT_OFFSET(NCButton, NormalTex, 0x1B0);
NW_ASSERT_OFFSET(NCButton, bAcceptKey, 0x1CC);
NW_ASSERT_OFFSET(NCButton, ToolTip, 0x1D0);
NW_ASSERT_OFFSET(NCButton, Text, 0x248);
NW_ASSERT_OFFSET(NCButton, TextID, 0x254);
NW_ASSERT_OFFSET(NCButton, TextOffsetY, 0x260);
NW_ASSERT_SIZE(NCButton, 0x264);

class NCExButton : public NCButton
{
    NW_DECLARE_CLASS(NCExButton, NCButton, 0, 0x101980E0)

    NCExButton();                                                               // 0x10002830
    NCExButton(const TCHAR* NormalTexName, const TCHAR* PushedTexName, UBOOL bInAcceptKey); // 0x100028d0
    // ~NCExButton                                                              // 0x10002880

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);             // 25 0x1003a790 (folded)
    virtual INT OnLButtonDown(WPARAM wParam, LPARAM lParam);                    // 61 0x10001390
    virtual INT OnLButtonUp(WPARAM wParam, LPARAM lParam);                      // 62 0x100013d0
};
NW_ASSERT_SIZE(NCExButton, 0x264);

class NCPushButton : public NCButton
{
    NW_DECLARE_CLASS(NCPushButton, NCButton, 0, 0x101976A0)

    UBOOL bHeld;               // +0x264 pressed and still inside
    INT RepeatDelay;           // +0x268 ms between repeats (100)
    UBOOL bAutoRepeat;         // +0x26C (1)

    NCPushButton();                                                             // 0x100029d0
    NCPushButton(const TCHAR* NormalTexName, const TCHAR* PushedTexName);       // 0x10002a80
    // ~NCPushButton                                                            // 0x10002a30

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);             // 25 0x1003a790 (folded)
    virtual INT OnTimer(INT TimerID);                                           // 35 0x10001560
    virtual INT OnDestroy();                                                    // 37 0x10001590
    virtual INT OnMouseMove(WPARAM wParam, LPARAM lParam);                      // 60 0x100014f0
    virtual INT OnLButtonDown(WPARAM wParam, LPARAM lParam);                    // 61 0x10001440
    virtual INT OnLButtonUp(WPARAM wParam, LPARAM lParam);                      // 62 0x100014b0
};
NW_ASSERT_OFFSET(NCPushButton, bHeld, 0x264);
NW_ASSERT_SIZE(NCPushButton, 0x270);

class NCRadioButton : public NCButton
{
    NW_DECLARE_CLASS(NCRadioButton, NCButton, 0, 0x10198B30)

    INT Value;                 // +0x264 sent to the parent when selected (-1)
    INT CreateWidth;           // +0x268 size at OnCreate
    INT CreateHeight;          // +0x26C
    UBOOL bToggle;             // +0x270 a second click releases it; never reports (the constructors leave it as
                               //   allocated: zero)

    NCRadioButton();                                                            // 0x10002ae0
    NCRadioButton(const TCHAR* NormalTexName, const TCHAR* PushedTexName);      // 0x10002ba0
    // ~NCRadioButton                                                           // 0x10002b50

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);             // 25 0x1003a790 (folded)
    virtual INT OnCreate();                                                     // 36 0x10001750
    virtual INT OnDestroy();                                                    // 37 0x10001780 (folded)
    virtual INT OnDrawToolTip(NCGDevice* Device);                               // 56 0x10001f90
    virtual INT OnMouseMove(WPARAM wParam, LPARAM lParam);                      // 60 0x100016f0
    virtual INT OnLButtonDown(WPARAM wParam, LPARAM lParam);                    // 61 0x10001610
    virtual INT OnLButtonUp(WPARAM wParam, LPARAM lParam);                      // 62 0x100016c0
};
NW_ASSERT_OFFSET(NCRadioButton, Value, 0x264);
NW_ASSERT_OFFSET(NCRadioButton, bToggle, 0x270);
NW_ASSERT_SIZE(NCRadioButton, 0x274);

#pragma pack(pop)