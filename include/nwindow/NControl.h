#pragma once
// NControl - base of every control (buttons, edit boxes, lists...) (UClass "Control", 0x1A0).
// A control reports to its parent with NWM_COMMAND (wParam = the control's ID, lParam = a code
// that depends on the control: NotifyID for buttons, 1/0 for press/release...).

#include "nwindow/NCWnd.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NControl : public NCWnd
{
    NW_DECLARE_CLASS(NControl, NCWnd, 0, 0x1019AA18)

    INT ID;                    // +0x98 set by the parent that creates the control
    UTexture* DisableTexture;  // +0x9C "NWindow.Disable" (OnCreate)
    INT NotifyID;              // +0xA0 id sent to the parent, -1 = ID (name ours)
    TCHAR Caption[128];        // +0xA4 (name ours)

    NControl();                                                     // 0x10001c30
    // ~NControl                                                    // 0x10001be0

    virtual __declspec(noinline) INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam); // 25 0x10043ef0 (folded with
                                                                    //    identical overrides)
    virtual INT OnCreate();                                         // 36 0x10005940
    virtual INT OnDestroy();                                        // 37 0x10005970
    virtual const TCHAR* GetValue() { return NULL; }                // 67 0x10001ab0 (folded) (name
                                                                    //    ours) what an HTML form
                                                                    //    control holds

    // talking to the parent window (names ours)
    UBOOL NotifyParent(LPARAM Code);                                // 0x10005820 NWM_COMMAND
    void ReleaseFocus();                                            // 0x10005860
    void HideParent();                                              // 0x10005890
    UBOOL SendTo(NWndEntry* Target, UINT Msg, WPARAM wParam, LPARAM lParam); // 0x100058c0
    UBOOL SendToParent(UINT Msg, WPARAM wParam, LPARAM lParam);    // 0x10005900
    void SetCaption(const TCHAR* Text);                             // 0x100059f0
    const TCHAR* GetCaption();                                      // 0x10005a10

protected:
    NCWnd* ParentWnd() { return Entry->Parent ? Entry->Parent->Wnd : NULL; }
};
NW_ASSERT_OFFSET(NControl, ID, 0x98);
NW_ASSERT_OFFSET(NControl, NotifyID, 0xA0);
NW_ASSERT_OFFSET(NControl, Caption, 0xA4);
NW_ASSERT_SIZE(NControl, 0x1A4);

#pragma pack(pop)