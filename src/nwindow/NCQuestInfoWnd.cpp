/*=============================================================================
    NCQuestInfoWnd.cpp - the quest window's description page. C2 stock: 0x100beea0 .. 0x100bf44f,
    0x100bfed0 .. 0x100c0083, 0x100c0090, 0x100c0200 (the quest array's destructor, NTemplates.cpp),
    0x100c0290 .. 0x100c0393, 0x100c0a20 .. 0x100c0a8d and 0x100c0b60 .. 0x100c0bac (the quest
    window and its item page sit in between).
=============================================================================*/
#include "nwindow/NCQuestInfoWnd.h"
#include "nwindow/NCButton.h"
#include "nwindow/NCGDevice.h"
#include "nwindow/NCSliderBar.h"
#include "nwindow/NConsoleWnd.h"

// how many characters the FString holds, as the stock tests it (the array's count, not the length)
static FORCEINLINE INT StringNum(const FString& S)
{
    return ((const TArray<TCHAR>*)&S)->Num();
}

// 0x100c0290
NCQuestInfoWnd::NCQuestInfoWnd()
{
    Current = -1;
}
NW_HOOK_CTOR(0x100c0290, NCQuestInfoWnd)

// 0x100c0330 - the text and the quest array go, then NCScrollWnd's
NW_HOOK_DTOR(0x100c0330, NCQuestInfoWnd)

// vslot 25: the stock slot is 0x10019810, the body the linker shared with NCItemWnd's (hooked there): the base's
// answer as a plain 0 / 1
INT NCQuestInfoWnd::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    return NCScrollWnd::WindowProc(Msg, wParam, lParam) != 0;
}

// vslot 36: the stock slot is 0x1002ce20, shared with NCNoticeInfoWnd's (hooked there)
INT NCQuestInfoWnd::OnCreate()
{
    NCScrollWnd::OnCreate();
    return 1;
}

// 0x100beea0 (vslot 55) - the text is drawn with the origin moved by the scroll position and
// measured after it, which sets the scroll range; the clip opens before BeginPaint
INT NCQuestInfoWnd::OnPaint(NCGDevice* Device)
{
    guard(NCQuestInfoWnd::OnPaint);
    Device->SetClip(ScreenX, ScreenY, Width, Height);
    Device->BeginPaint(ScreenX, ScreenY);
    if (Current >= 0 && Current < Quests.Num() && StringNum(Text))
    {
        Device->PushOrigin(ScrollX, ScrollY);
        Device->DrawText(2, 1, 0xFFFFFFFF, *Text, Width - 20, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
        SIZE Size;
        NCGDevice::GetTextExtent(&Size, *Text, Width - 20, 0, 0, 0, 0);
        SetContentHeight(Size.cy);
        Device->PopOrigin();
    }
    Device->EndPaint();
    Device->ReleaseClip();
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x100beea0, NCQuestInfoWnd, OnPaint)

// 0x100bf000 (vslot 67) - NCScrollWnd's, but DeltaY is kept when already at the top
INT NCQuestInfoWnd::ScrollUp()
{
    if (ScrollY < 0)
    {
        DeltaX = ScrollX;
        DeltaY = ScrollY;
        ScrollY += LineStep;
        if (ScrollY > 0)
            ScrollY = 0;
        ScrollBar->SetScrollPos(-ScrollY);
        DeltaX -= ScrollX;
        DeltaY -= ScrollY;
    }
    return 1;
}
NW_HOOK_METHOD(0x100bf000, NCQuestInfoWnd, ScrollUp)

// 0x100bf090 (vslot 68) - NCScrollWnd's, but DeltaY is kept when already at the bottom
INT NCQuestInfoWnd::ScrollDown()
{
    if (ScrollY > MinScrollY)
    {
        DeltaX = ScrollX;
        DeltaY = ScrollY;
        ScrollY -= LineStep;
        if (ScrollY < MinScrollY)
            ScrollY = MinScrollY;
        ScrollBar->SetScrollPos(-ScrollY);
        DeltaX -= ScrollX;
        DeltaY -= ScrollY;
    }
    return 1;
}
NW_HOOK_METHOD(0x100bf090, NCQuestInfoWnd, ScrollDown)

// 0x100bf290 - at the step the player is at; the text empties when the quest data has none
void NCQuestInfoWnd::ShowQuest(INT Index)
{
    Current = Index;
    ScrollBar->SetScrollPos(0);
    ScrollY = 0;
    if (Current >= 0 && Current < Quests.Num())
    {
        NQuestEntry* Quest = Quests(Current);
        Quest->ShownStep = Quest->Step;
        Quest = Quests(Current);
        const TCHAR* S = GConsole->GetQuestText(Quest->QuestID, Quest->Steps(Quest->Step));
        if (S)
        {
            Text = S;
            return;
        }
        Text.Empty();
    }
}
NW_HOOK_METHOD(0x100bf290, NCQuestInfoWnd, ShowQuest)

// 0x100bf320 - the arrow forward (NCQuestWnd's command 0xF8): the next step, when the player has reached it
void NCQuestInfoWnd::NextStep()
{
    if (Current >= 0 && Current < Quests.Num())
    {
        INT Shown = Quests(Current)->ShownStep;
        if (Shown < Quests(Current)->Step)
        {
            ScrollBar->SetScrollPos(0);
            ScrollY = 0;
            Quests(Current)->ShownStep += 1;
            NQuestEntry* Quest = Quests(Current);
            const TCHAR* S = GConsole->GetQuestText(Quest->QuestID, Quest->Steps(Shown + 1));
            if (!S)
            {
                Text.Empty();
                return;
            }
            Text = S;
        }
    }
}
NW_HOOK_METHOD(0x100bf320, NCQuestInfoWnd, NextStep)

// 0x100bf3c0 - the arrow back (command 0xF9): the step before the one shown
void NCQuestInfoWnd::PrevStep()
{
    if (Current >= 0 && Current < Quests.Num())
    {
        INT Shown = Quests(Current)->ShownStep;
        if (Shown > 0)
        {
            ScrollBar->SetScrollPos(0);
            ScrollY = 0;
            Quests(Current)->ShownStep -= 1;
            NQuestEntry* Quest = Quests(Current);
            const TCHAR* S = GConsole->GetQuestText(Quest->QuestID, Quest->Steps(Shown - 1));
            if (!S)
            {
                Text.Empty();
                return;
            }
            Text = S;
        }
    }
}
NW_HOOK_METHOD(0x100bf3c0, NCQuestInfoWnd, PrevStep)

// 2^Index as the stock computes it (pow, then _ftol: 2^31 overflows into the sign bit)
static FORCEINLINE INT QuestBit(INT Index)
{
    return Index < 31 ? (1 << Index) : (INT)0x80000000;
}

// 0x100bfed0, 0x100c0090 - the entry's constructor and destructor (the steps' array 0x1001fb70), out of line for AddQuest
// and ClearQuests
NW_HOOK_CTOR(0x100bfed0, NQuestEntry)
NW_HOOK_DTOR(0x100c0090, NQuestEntry)

// 0x100bff40 - Steps is the server's: a bit set (bit N: step N + 1, bits 0 .. 29; bit 31 set), or a
// plain count of steps (bit 31 clear) that stands for the first ones. The quest is at its last step
void NCQuestInfoWnd::AddQuest(INT QuestID, INT Steps)
{
    NQuestEntry* Quest = new NQuestEntry;
    Quest->QuestID = QuestID;
    if (!(Steps & QuestBit(31)))
    {
        INT Bits = 0;
        for (INT i = 0; i < Steps; i++)
            Bits |= QuestBit(i);
        Steps = Bits;
    }
    for (INT i = 0; i < 30; i++)
    {
        if (Steps & QuestBit(i))
            Quest->Steps.AddItem(i + 1);
    }
    Quest->Step = Quest->Steps.Num() - 1;
    Quest->ShownStep = Quest->Step;
    Quests.AddItem(Quest);
    ScrollBar->SetScrollPos(0);
}
NW_HOOK_METHOD(0x100bff40, NCQuestInfoWnd, AddQuest)

// 0x100c0a20
void NCQuestInfoWnd::ClearQuests()
{
    for (INT i = 0; i < Quests.Num(); i++)
    {
        if (Quests(i))
            delete Quests(i);
    }
    Quests.Empty();
    Current = -1;
    Text.Empty();
}
NW_HOOK_METHOD(0x100c0a20, NCQuestInfoWnd, ClearQuests)

// 0x100c0b60 (vslot 37)
INT NCQuestInfoWnd::OnDestroy()
{
    ClearQuests();
    NCScrollWnd::OnDestroy();
    return 1;
}
NW_HOOK_METHOD(0x100c0b60, NCQuestInfoWnd, OnDestroy)

NW_CHECK_VTABLE(NCQuestInfoWnd, 0x10123278, 70)
