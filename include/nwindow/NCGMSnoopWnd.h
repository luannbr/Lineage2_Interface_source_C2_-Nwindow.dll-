#pragma once
// NCGMSnoopWnd - one conversation a GM listens to (UClass "CGMSnoopWnd", 0xC4; C2 only): the
// petition window's look (title bar, the "Petitionback" panel, a conversation list) with seven
// check boxes under it (which kinds of chat are shown) and a close button that tells the
// server the snoop is over (NCGMSnoopManager keeps four of them).

#include "nwindow/NCWnd.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCButton;
class NCCheckBox;
class NCPetitionListBox;

class NCGMSnoopWnd : public NCWnd
{
    NW_DECLARE_CLASS(NCGMSnoopWnd, NCWnd, 0, 0x101D05B8)

    UTexture* BackTex;               // +0x98 "L2UI.etcwnd.Petitionback"
    NCPetitionListBox* List;         // +0x9C the conversation (name ours)
    NCButton* CloseButton;           // +0xA0 text 141 (name ours)
    NCCheckBox* Filters[7];          // +0xA4 the chat kinds shown, all checked (name ours)
    INT SnoopID;                     // +0xC0 what the server calls this snoop, -1 = none (name ours)

    NCGMSnoopWnd();                                                  // 0x100b6b70
    // ~NCGMSnoopWnd                                                 // 0x100b6bc0

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x100b68a0
    virtual INT OnCreate();                                          // 36 0x100b8960
    virtual INT OnDestroy();                                         // 37 0x100b70c0
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x100b71d0 (NCPetitionWnd's body)
    virtual INT OnClose();                                           // 67 0x100b6920 (name ours)

    // name ours
    __declspec(noinline) void Reset();                               // 0x100b6900 the list emptied, no snoop
    void AddLine(const TCHAR* Text, DWORD Color, INT Type, INT Unused); // 0x100b7260 a line of the chat, when its kind is shown
};
NW_ASSERT_OFFSET(NCGMSnoopWnd, BackTex, 0x98);
NW_ASSERT_OFFSET(NCGMSnoopWnd, List, 0x9C);
NW_ASSERT_OFFSET(NCGMSnoopWnd, Filters, 0xA4);
NW_ASSERT_OFFSET(NCGMSnoopWnd, SnoopID, 0xC0);
NW_ASSERT_SIZE(NCGMSnoopWnd, 0xC4);

#pragma pack(pop)
