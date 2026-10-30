#pragma once
// NCLoadingWnd - the loading screen while entering the world (UClass "CLoadingWnd", 0x98): a
// 1024 x 768 picture stretched over the whole window. NConsoleWnd::RequestGameStart picks the
// picture: in Korean one per age rating of the chosen server, elsewhere the country's entry of
// FontInfo.ini.

#include "nwindow/NCWnd.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCLoadingWnd : public NCWnd
{
    NW_DECLARE_CLASS(NCLoadingWnd, NCWnd, 0, 0x101B9D60)

    UTexture* LoadingTex;            // +0x98

    NCLoadingWnd();                                                  // 0x100808c0
    // ~NCLoadingWnd                                                 // 0x10080910

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x10043ef0 (folded)
    virtual INT OnCreate() { return 1; }                             // 36 0x10005b80 (folded)
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x10080960

    void SetLoadingTexture(INT ScreenID);                            // 0x100807b0 (name ours)
};
NW_ASSERT_OFFSET(NCLoadingWnd, LoadingTex, 0x98);
NW_ASSERT_SIZE(NCLoadingWnd, 0x9C);

#pragma pack(pop)