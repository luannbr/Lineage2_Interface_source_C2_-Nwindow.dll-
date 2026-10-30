#pragma once
// NCChatListBox - the chat message list (UClass "CChatListBox", 0x210). Long messages are
// wrapped to the width (continuation lines indented 12 px); the scroll controls sit on the
// left and the visible line count follows the height (12 px lines). Each chat tab has one;
// its Filter says which message types it shows (IsChannelShown, NCChatWnd::AddChatMessage).

#include "nwindow/NCListBox.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

// The channel switches of one chat tab (names ours, after their chatfilter.ini keys; the
// message types are those of L2's Say2 packet). Passed by value to
// NCChatFilterWnd::SetFilter, hence a struct.
struct NChatFilter
{
    UBOOL bSystem;             // +0x00 "system" type 5 (system / GM messages)
    UBOOL bChat;               // +0x04 "chat"   master switch of the chat types below
    UBOOL bDamage;             // +0x08 "damage" type 5 subtype 3, needs bSystem too
    UBOOL bNormal;             // +0x0C "normal" type 0
    UBOOL bShout;              // +0x10 "shout"  type 1 (!)
    UBOOL bPledge;             // +0x14 "pledge" type 4 (@)
    UBOOL bParty;              // +0x18 "party"  type 3 (#)
    UBOOL bMarket;             // +0x1C "market" type 8 (+)
    UBOOL bTell;               // +0x20 "tell"   type 2 (")
    UBOOL bAlly;               // +0x24 "ally"   type 9 ($)
    UBOOL bUseItems;           // +0x28 "useitems" new in C2 (name ours: the ini key): the messages of items used;
                               //   a sub-switch of bSystem like bDamage (SetFilter greys both with it)
};
NW_ASSERT_SIZE(NChatFilter, 0x2C);

class NCChatListBox : public NCListBox
{
    NW_DECLARE_CLASS(NCChatListBox, NCListBox, 0, 0x101B02D8)

    NChatFilter Filter;        // +0x1EC (0x2C bytes in C2: LoadFilters reads the keys at +0x1EC .. +0x214)

    NCChatListBox();                                                 // 0x10048cf0
    NCChatListBox(INT InMaxLines, INT InVisibleLines, INT Unused);   // 0x10048d90
    // ~NCChatListBox                                                // 0x10048d40

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x100dc140 (shared with NCDebugWnd's)
    virtual INT OnCreate();                                          // 36 0x10045460
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x100454d0
    virtual void AddString(const TCHAR* Text, DWORD Color, INT Param); // 69 0x10047b70
    virtual UBOOL IsChannelShown(INT Type, INT SubType);             // 75 0x10044270 (name ours; the stock
                                                                     //   NCChatWnd::CaptureMessage calls +0x12C)
};
NW_ASSERT_OFFSET(NCChatListBox, Filter, 0x1EC);
NW_ASSERT_SIZE(NCChatListBox, 0x218);

#pragma pack(pop)