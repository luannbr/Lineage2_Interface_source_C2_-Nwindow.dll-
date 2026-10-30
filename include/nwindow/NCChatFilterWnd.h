#pragma once
// NCChatFilterWnd - the chat filter panel that opens above the chat window (UClass
// "CChatFilterWnd", 0xC8): one check box per NChatFilter switch of the current tab, OK and
// Cancel. OK copies the boxes into the tab's list and saves them to chatfilter.ini, one
// section per tab (entire_tab, market_tab, party_tab, pledge_tab, ally_tab); LoadFilters reads
// the five sections back when the chat is created. The tab's own channel stays checked and
// grey (SetFilter).

#include "nwindow/NCWnd.h"
#include "nwindow/NCChatListBox.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCCheckBox;
class NCButton;

// Filters[] order (names ours) and the NWM_COMMAND ids of the panel's controls
enum
{
    CHATFILTER_System,
    CHATFILTER_Damage,
    CHATFILTER_Chat,
    CHATFILTER_Normal,
    CHATFILTER_Shout,
    CHATFILTER_Pledge,
    CHATFILTER_Party,
    CHATFILTER_Market,
    CHATFILTER_Tell,
    CHATFILTER_Ally,
    CHATFILTER_UseItems,   // new in C2 (name ours: the ini key "useitems")
    CHATFILTER_MAX
};
enum
{
    NID_CHATFILTER_SYSTEM = (INT)0xF0F01450,   // Filters[i]: 0xF0F01450 + i, the last 0xF0F01470
    NID_CHATFILTER_CHAT = (INT)0xF0F01452,
    NID_CHATFILTER_ALLY = (INT)0xF0F01470,   // the UseItems box has the same id in the stock code
    NID_CHATFILTER_OK = (INT)0xF0FF1458,
    NID_CHATFILTER_CANCEL = (INT)0xF0FF1459,
};

class NCChatFilterWnd : public NCWnd
{
    NW_DECLARE_CLASS(NCChatFilterWnd, NCWnd, 0, 0x101AFDB8)

    NCCheckBox* Filters[CHATFILTER_MAX];   // +0x98 CHATFILTER_* (11 in C2, the last at +0xC0)
    NCButton* OKButton;                    // +0xC4
    NCButton* CancelButton;                // +0xC8
    UTexture* BackTex;                     // +0xCC "L2UI.ChatWnd.chatoption"

    NCChatFilterWnd();                                               // 0x10045de0
    // ~NCChatFilterWnd                                              // 0x10045e30

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x100452f0
    virtual INT OnCreate();                                          // 36 0x10045e80
    virtual INT OnDestroy();                                         // 37 0x100466c0
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x100467d0
    // names ours
    virtual INT OnSystemClicked();                                   // 67 0x10046b10
    virtual INT OnChatClicked();                                     // 68 0x10046b60
    virtual INT OnOK();                                              // 69 0x10046c90
    virtual INT OnCancel();                                          // 70 0x100453a0

    void SetFilter(NChatFilter Filter);                              // 0x100468b0 (name ours)
    void LoadFilters();                                              // 0x10046f90 (name ours)

private:
    void GreyTabChannel();          // inlined twice in the stock code (name ours)
};
NW_ASSERT_OFFSET(NCChatFilterWnd, Filters, 0x98);
NW_ASSERT_OFFSET(NCChatFilterWnd, OKButton, 0xC4);
NW_ASSERT_OFFSET(NCChatFilterWnd, BackTex, 0xCC);
NW_ASSERT_SIZE(NCChatFilterWnd, 0xD0);

#pragma pack(pop)