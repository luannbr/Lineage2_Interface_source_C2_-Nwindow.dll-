#pragma once
// NCProgressCtrl - count-down bar (UClass "CProgressCtrl", 0x1C0). While running, each paint
// adds the frame time (ms) to Pos; the bar shrinks from full to empty, and at Max the control
// stops and reports NWM_COMMAND(ID, 0) to the parent.

#include "nwindow/NControl.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCProgressCtrl : public NControl
{
    NW_DECLARE_CLASS(NCProgressCtrl, NControl, 0, 0x1019B468)

    UTexture* BarTex;          // +0x1A4 "NWindow.WhiteTexture" by default
    UTexture* BackTex;         // +0x1A8 "NWindow.BlackTexture" by default
    INT Unknown1A8;            // +0x1AC (1)
    INT Border;                // +0x1B0 bar inset (2)
    UBOOL bStopped;            // +0x1B4 (1: created stopped)
    DWORD Pos;                 // +0x1B8 ms elapsed
    DWORD Max;                 // +0x1BC ms
    INT Unknown1BC;            // +0x1C0   // C2: offset derived, no evidence

    NCProgressCtrl();                                                        // 0x10006130
    NCProgressCtrl(const TCHAR* BackTexName, const TCHAR* BarTexName, INT InBorder); // 0x10006270
    // ~NCProgressCtrl                                                       // 0x10006220

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);          // 25 0x10090b60 (folded)
    virtual INT OnCreate() { return 1; }                                     // 36 0x10005b80 (folded)
    virtual INT OnDestroy() { return 1; }                                    // 37 0x10005b80 (folded)
    virtual INT OnPaint(NCGDevice* Device);                                  // 55 0x10006360

    // names ours
    void SetRemaining(DWORD Remaining);                                      // 0x10005b90
    void SetMax(DWORD InMax);                                                // 0x10005bb0
    void Reset();                                                            // 0x10005bc0
    void Stop();                                                             // 0x10005be0
    void Start();                                                            // 0x10005bf0
};
NW_ASSERT_OFFSET(NCProgressCtrl, Border, 0x1B0);
NW_ASSERT_OFFSET(NCProgressCtrl, Max, 0x1BC);
NW_ASSERT_SIZE(NCProgressCtrl, 0x1C4);

#pragma pack(pop)