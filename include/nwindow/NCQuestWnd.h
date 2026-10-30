#pragma once
// NCQuestWnd - the quest window (UClass "CQuestWnd", 0xCC, 240 x 300; NCMainWnd creates it,
// NCGMQuestWnd derives from it): the accepted quests in a list and, below, two tab pages: the
// selected quest's description (NCQuestInfoWnd) and the quest items (NCQuestItemWnd). Opening
// it asks the server for the quests again; "Cancel" gives up the selected quest after asking.

#include "nwindow/NCWnd.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCButton;
class NCListBox;
class NCQuestInfoWnd;
class NCQuestItemWnd;
class NCTabControl;
struct NItemInfo;

class NCQuestWnd : public NCWnd
{
    NW_DECLARE_CLASS(NCQuestWnd, NCWnd, 0, 0x101D3930)

    UTexture* TopTex;                // +0x98 "L2UI.QuestWnd.questBack1" the top 115 pixels
    UTexture* InfoBackTex;           // +0x9C "L2UI.QuestWnd.questBack2" the rest, page 0
    UTexture* ItemBackTex;           // +0xA0 "L2UI.QuestWnd.questBack3" the rest, page 1
    UTexture* ItemPageIconTex;       // +0xA4 "sek.cbui303" 14 x 14 at (220, 127) on page 1
    NCListBox* QuestList;            // +0xA8 the quest names, the quest ids as parameters
    NCQuestInfoWnd* InfoWnd;         // +0xAC page 0 "Description"
    NCQuestItemWnd* ItemWnd;         // +0xB0 page 1 "Item"
    NCTabControl* TabCtrl;           // +0xB4
    NCButton* CancelButton;          // +0xB8 page 0 only
    NCWnd* NewChild[2];              // +0xBC, +0xC0 new in C2 (NCPushButtons): [0] the arrow back (command 0xF9), [1] the arrow
                                     //   forward (0xF8) of the description page; shown with it, hidden on the item page (OnTab)
    INT DropX;                       // +0xC4 (160) the rectangle where a quest item dropped
    INT DropY;                       // +0xC8 (420)   asks to be destroyed (IsInDropZone): below
    INT DropW;                       // +0xCC (27)    the window, so a drop inside never hits it
    INT DropH;                       // +0xD0 (27)
    INT CurTab;                      // +0xD4 page shown

    NCQuestWnd();                                                    // 0x100bf930
    // ~NCQuestWnd                                                   // 0x10073c50

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x100becb0
    virtual INT OnCreate();                                          // 36 0x100c0480
    virtual INT OnDestroy();                                         // 37 0x100bf9a0
    virtual void ShowWindow();                                       // 43 0x100c0ad0
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x100bfb50
    // names ours
    virtual UBOOL IsInDropZone(INT X, INT Y);                        // 67 0x100bee50
    virtual INT OnDialogResult(WPARAM wParam, LPARAM lParam);        // 68 0x100bfd40
    virtual INT OnCancel();                                          // 69 0x100bfd70
    virtual INT OnSelectQuest(INT Index);                            // 70 0x100bfe70
    virtual INT OnTab(INT Page);                                     // 71 0x100bedb0
    virtual INT OnNewButton1();                                      // 72 0x100bfe90 (C2 only; name ours) the command 0xF8: next step
    virtual INT OnNewButton2();                                      // 73 0x100bfeb0 (C2 only; name ours) the command 0xF9: previous step

    // names ours: the console fills the window with these
    void Clear();                                                    // 0x100c0aa0
    INT AddQuest(INT QuestID, const TCHAR* Name, INT Step);          // 0x100c01a0 (Step: the server's bit set or count)
    INT AddQuestItem(NItemInfo* Info);                               // 0x100c01e0
};
NW_ASSERT_OFFSET(NCQuestWnd, TopTex, 0x98);
NW_ASSERT_OFFSET(NCQuestWnd, QuestList, 0xA8);
NW_ASSERT_OFFSET(NCQuestWnd, DropX, 0xC4);
NW_ASSERT_OFFSET(NCQuestWnd, CurTab, 0xD4);
NW_ASSERT_SIZE(NCQuestWnd, 0xD8);

#pragma pack(pop)