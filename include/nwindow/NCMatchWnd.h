#pragma once
// NCMatchWnd - party matching (UClass "CMatchWnd", 0xB4): the list of players looking for a
// party (NCMatchListCtrl) and Invite, Options, Whisper and Targeting on the selected one. The
// console fills it: ClearList starts a new list (remembering the selection), AddPlayer adds
// each player the server sends, UpdatePlayerInfo stores the details asked for on a click.

#include "nwindow/NCWnd.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCMatchListCtrl;
class NCButton;

// NWM_COMMAND ids of the buttons (names ours)
enum
{
    NID_MATCH_INVITE = 0xF2,
    NID_MATCH_OPTIONS = 0xF3,
    NID_MATCH_WHISPER = 0xF4,
    NID_MATCH_TARGETING = 0xF5,
};

class NCMatchWnd : public NCWnd
{
    NW_DECLARE_CLASS(NCMatchWnd, NCWnd, 0, 0x101C2858)

    INT Unknown94;                   // +0x98 not used   // C2: offset derived, no evidence
    UTexture* BackTex;               // +0x9C "L2UI.MatchWnd.MatchingBack"   // C2: OnCreate / OnPaint (0x100961a0, 0x10095520)
    NCMatchListCtrl* MatchList;      // +0xA0
    NCButton* InviteButton;          // +0xA4
    NCButton* OptionsButton;         // +0xA8
    NCButton* WhisperButton;         // +0xAC
    NCButton* TargetingButton;       // +0xB0
    INT SelectedID;                  // +0xB4 object ID selected when the list was cleared,
                                     //   selected again when it comes back

    NCMatchWnd();                                                    // 0x10095310
    // ~NCMatchWnd                                                   // 0x10095380

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x100946c0
    virtual INT OnCreate();                                          // 36 0x100961a0
    virtual INT OnDestroy();                                         // 37 0x100953d0
    virtual void ShowWindow();                                       // 43 0x1008c9c0 (folded)
    virtual void HideWindow();                                       // 44 0x1007b470 (folded)
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x10095520
    // the buttons (names ours)
    virtual INT OnInvite();                                          // 67 0x100955d0
    virtual INT OnOptions();                                         // 68 0x10094770
    virtual INT OnWhisper();                                         // 69 0x10095680
    virtual INT OnTargeting();                                       // 70 0x100957a0

    // the console's calls (names ours)
    void AddPlayer(L2ParamStack& Params);                            // 0x10095cb0
    void ClearList();                                                // 0x100955b0
    void UpdatePlayerInfo(L2ParamStack& Params);                     // 0x10096820
};
NW_ASSERT_OFFSET(NCMatchWnd, BackTex, 0x9C);
NW_ASSERT_OFFSET(NCMatchWnd, MatchList, 0xA0);
NW_ASSERT_OFFSET(NCMatchWnd, SelectedID, 0xB4);
NW_ASSERT_SIZE(NCMatchWnd, 0xB8);

#pragma pack(pop)