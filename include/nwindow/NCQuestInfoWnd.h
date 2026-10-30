#pragma once
// NCQuestInfoWnd - the quest window's "Description" page (UClass "CQuestInfoWnd", 0xEC): the
// text of the selected quest at one of its steps (quest data, NConsoleWnd::AddQuestID), shown
// at the step the player is at; the arrows (NCQuestWnd's) page back to the earlier steps and
// forward again.

#include "nwindow/NCScrollWnd.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

// a quest of the list (name ours; 0x18 bytes in C2, allocated one by one: the window keeps pointers). The server
// sends the steps of a quest as a bit set (bit N: step N + 1; a count when bit 31 is clear), the window
// keeps the numbers (C1: 12 bytes, a quest had one step)
struct NQuestEntry
{
    INT QuestID;               // +0x0
    INT Step;                  // +0x4 index in Steps of the step the player is at (the last one)
    INT ShownStep;             // +0x8 index in Steps of the step shown (0 .. Step)
    TArray<INT> Steps;         // +0xC the step numbers (1 .. 30), in ascending order

    NQuestEntry() : Step(0), ShownStep(0) { Steps.Empty(); }      // 0x100bfed0 (the quest id is set by AddQuest); ~NQuestEntry 0x100c0090 (the steps' array)
};
NW_ASSERT_OFFSET(NQuestEntry, Steps, 0xC);
NW_ASSERT_SIZE(NQuestEntry, 0x18);

class NCQuestInfoWnd : public NCScrollWnd
{
    NW_DECLARE_CLASS(NCQuestInfoWnd, NCScrollWnd, 0, 0x101D3408)

    TArray<NQuestEntry*> Quests;     // +0xD0 in the order of NCQuestWnd's list (C1: TArray<NQuestEntry>)
    FString Text;                    // +0xDC the text shown (C1: TCHAR Text[1024] at +0xD8)
    INT Current;                     // +0xE8 index in Quests, -1 = none (C1 +0x8D8); the arrows (C1: two more
                                     //   fields) are NCQuestWnd's now

    NCQuestInfoWnd();                                                // 0x100c0290
    // ~NCQuestInfoWnd                                               // 0x100c0330

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x10019810
    virtual INT OnCreate();                                          // 36 0x1002ce20
    virtual INT OnDestroy();                                         // 37 0x100c0b60
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x100beea0
    virtual INT ScrollUp();                                          // 67 0x100bf000
    virtual INT ScrollDown();                                        // 68 0x100bf090

    // names ours (C2: plain functions; C1 had NextStep / PrevStep as virtuals)
    void ShowQuest(INT Index);                                       // 0x100bf290
    void NextStep();                                                 // 0x100bf320
    void PrevStep();                                                 // 0x100bf3c0
    void AddQuest(INT QuestID, INT Steps);                           // 0x100bff40
    void ClearQuests();                                              // 0x100c0a20
};
NW_ASSERT_OFFSET(NCQuestInfoWnd, Quests, 0xD0);
NW_ASSERT_OFFSET(NCQuestInfoWnd, Text, 0xDC);
NW_ASSERT_OFFSET(NCQuestInfoWnd, Current, 0xE8);
NW_ASSERT_SIZE(NCQuestInfoWnd, 0xEC);

#pragma pack(pop)
