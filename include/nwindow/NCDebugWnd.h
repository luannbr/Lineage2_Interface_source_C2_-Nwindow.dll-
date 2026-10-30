#pragma once
// NCDebugWnd - the debug log list (UClass "CDebugWnd", 0x1E8), fed by NCObject::Debugf.
// Lines are prefixed with their category; the categories come from .\L2Debug.ini
// ([DebugInfo] count, then Library%d: name%d, color%d = R=,G=,B=, IsEnable%d).

#include "nwindow/NCListBox.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

// one debug category (names ours)
struct NDebugCategory
{
    TCHAR Name[32];            // +0x00 (entry 0: "NWindow")
    UBOOL bEnabled;            // +0x40
    DWORD Color;               // +0x44
};
NW_ASSERT_SIZE(NDebugCategory, 0x48);

#if NW_HOOK_MODE
#define GDebugCategories (reinterpret_cast<NDebugCategory*>(::nw::Addr(0x1016bc90)))
#define GNumDebugCategories NW_GLOBAL(INT, 0x101b21d0)
#else
extern NDebugCategory GDebugCategories[];   // 20 in the stock image
extern INT GNumDebugCategories;
#endif

class NCDebugWnd : public NCListBox
{
    NW_DECLARE_CLASS(NCDebugWnd, NCListBox, 0, 0x101B21D8)

    NCDebugWnd();                                                    // 0x1006ecb0 (50, 10)
    NCDebugWnd(INT InMaxLines, INT InVisibleLines);                  // 0x1006ed50
    // ~NCDebugWnd                                                   // 0x1006ed00

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x100dc140
    virtual INT OnCreate();                                          // 36 0x1006e930
    virtual INT OnDestroy();                                         // 37 0x1006ebb0
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x1006e940

    // names ours
    void SetBackTexture(const TCHAR* Name);                          // 0x1006e960
    void AddLog(INT Category, const TCHAR* Text, INT Unused);        // 0x1006e990
    void LoadCategories();                                           // 0x1006ea10
};
NW_ASSERT_SIZE(NCDebugWnd, 0x1EC);

#pragma pack(pop)