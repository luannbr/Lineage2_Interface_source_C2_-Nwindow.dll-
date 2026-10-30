#pragma once
// NCCountSpin - a number with two small buttons at its right that count it up and down between
// a minimum and a maximum (UClass "CCountSpin", 0x3E4). No window of NWindow.dll creates one;
// only the class's InternalConstructor builds it (0 .. 100, no label). A label is drawn first,
// the number after it; each step tells the parent (NWM_COMMAND, 1 up / 2 down), which takes it
// back by setting bCancelStep meanwhile.

#include "nwindow/NControl.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCPushButton;

class NCCountSpin : public NControl
{
    NW_DECLARE_CLASS(NCCountSpin, NControl, 0, 0x1019AF40)

    TCHAR ValueText[16];             // +0x1A4 the value in decimal
    FString Label;                   // +0x1C4 drawn before it (C2: an FString, C1: TCHAR Label[256]); the
                                     //   constructor's default is "Default"
    INT TextX;                       // +0x1D0 (5)
    INT TextY;                       // +0x1D4 (5)
    UTexture* BackTex;               // +0x1D8 "NWindow.EditBack", over the whole control
    INT Min;                         // +0x1DC
    INT Max;                         // +0x1E0
    INT Value;                       // +0x1E4
    NCPushButton* UpButton;          // +0x1E8
    NCPushButton* DownButton;        // +0x1EC
    UBOOL bCancelStep;               // +0x1F0 (name ours) set by the parent while it is told of
                                     //   a step: the step is undone

    NCCountSpin(INT InMin = 0, INT InMax = 100, const TCHAR* InLabel = NULL);         // 0x10005c00
    // ~NCCountSpin                                                  // 0x10005d20

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x10005a40
    virtual INT OnCreate();                                          // 36 0x10005ec0
    virtual INT OnDestroy();                                         // 37 0x10006060
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x10005d90
    // names ours
    virtual INT OnUp();                                              // 68 0x10005ac0
    virtual INT OnDown();                                            // 69 0x10005b20
};
NW_ASSERT_OFFSET(NCCountSpin, ValueText, 0x1A4);
NW_ASSERT_OFFSET(NCCountSpin, Label, 0x1C4);
NW_ASSERT_OFFSET(NCCountSpin, TextX, 0x1D0);
NW_ASSERT_OFFSET(NCCountSpin, Min, 0x1DC);
NW_ASSERT_OFFSET(NCCountSpin, UpButton, 0x1E8);
NW_ASSERT_SIZE(NCCountSpin, 0x1F4);

#pragma pack(pop)