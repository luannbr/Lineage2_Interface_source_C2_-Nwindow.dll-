/*=============================================================================
    NTemplates.cpp - the template code the stock module keeps out of line: TArray methods
    for the element types of the windows (UnTemplate.h, with its check()s), UE2's Cast and
    TMap's Rehash, FString::Len. Each stock instance is hooked to ours through a typedef;
    instances that differ only in their exception tables (one per class that uses them)
    all come here. Stock: 0x10003a10, 0x10003bb0, 0x100039f0, 0x10004ce0, 0x10017620,
    0x10017eb0, 0x10018460, 0x10019880, 0x1001a010, 0x1001b380, 0x10041b20, 0x100424e0,
    0x1005fce0, 0x1006e700, 0x10070b40, 0x100745e0, 0x10075bc0, 0x10078bb0, 0x100792e0,
    0x1007a760, 0x100887e0, 0x10088800, 0x100a1660, 0x100ad570, 0x100b36e0, 0x100bcaa0,
    0x10022400, 0x10022980, 0x100259f0, 0x10028700, 0x10028a90, 0x10029fc0, 0x10036e70,
    0x1003cb40, and the console's 0x1004dd20, 0x1004dda0, 0x10055660, 0x100556f0, 0x10055780,
    0x10045e60.
=============================================================================*/
#include "nwindow/NCAbnormalStatusWnd.h"
#include "nwindow/NCCandidateBox.h"
#include "nwindow/NCComboBox.h"
#include "nwindow/NCChatWnd.h"
#include "nwindow/NCConsole.h"
#include "nwindow/NCGDevice.h"
#include "nwindow/NCItemWnd.h"
#include "nwindow/NCListBox.h"
#include "nwindow/NCListCtrl.h"
#include "nwindow/NCMinimapWnd.h"
#include "nwindow/NCQuestInfoWnd.h"
#include "nwindow/NCSiegeInfoMainWnd.h"
#include "nwindow/NCSkillItemWnd.h"
#include "nwindow/NCTrainListBox.h"
#include "nwindow/NCVIPShopWnd.h"
#include "nwindow/NGameData.h"
#include "nwindow/NCActionItemWnd.h"
#include "nwindow/NCButton.h"
#include "nwindow/NControl.h"

/*-----------------------------------------------------------------------------
    TArray of pointers: one out-of-line Remove, copy constructor and new(Array) for all of
    them; destructors per class.
-----------------------------------------------------------------------------*/

typedef TArray<void*> TPtrArray;
typedef TArray<NListRow*> TListRowArray;
typedef TArray<NItemEntry*> TItemEntryArray;
typedef TArray<NString*> TStringPtrArray;
typedef TArray<NCCoolTimeIcon*> TCoolTimeArray;
typedef TArray<NVIPNeededItem*> TVIPNeededItemArray;
typedef TArray<NSiegeClanInfo*> TSiegeClanArray;
typedef TArray<NTrainEntry*> TTrainEntryArray;
typedef TArray<AActor*> TActorArray;

// 0x1001d1b0
NW_HOOK_METHOD(0x1001d1b0, TPtrArray, Remove)
// 0x100959d0 - the copy (NCMatchWnd's rows)
NW_HOOK_CTOR_ARGS(0x100959d0, TPtrArray, (const TPtrArray& A), (A))
// 0x10094790 - new(Array): room for a new last element (the operator's instance for pointers)
static void* NewPtrArrayElement(size_t Size, TPtrArray& Array)
{
    return operator new(Size, Array);
}
NW_HOOK_FUNC(0x10094790, NewPtrArrayElement)

// 0x1001d690 - NCListCtrl's rows and columns, NCMatchWnd's copy of them
NW_HOOK_DTOR(0x1001d690, TListRowArray)
// 0x1001a740 - NCItemWnd's, NCEquipItemWnd's and NCShortCutWnd's items
NW_HOOK_DTOR(0x1001a740, TItemEntryArray)
// 0x10005670 - NCComboBox's items, NCChatWnd's tell names, NCPawnCreateWnd's (NCSelectCtrl)
NW_HOOK_DTOR(0x10005670, TStringPtrArray)
// 0x100781b0 - NCSkillItemWnd's cool times
NW_HOOK_DTOR(0x100781b0, TCoolTimeArray)
// 0x10087500 - the VIP shop's lists
NW_HOOK_DTOR(0x10087500, TVIPNeededItemArray)
// 0x100d2350 - NCSiegePartInfoWnd's clans
NW_HOOK_DTOR(0x100d2350, TSiegeClanArray)

// 0x10089e50 - the inventories' and the pawn viewer's actor lists
NW_HOOK_METHOD(0x10089e50, TActorArray, AddItem)

typedef TArray<NCRadioButton*> TRadioButtonArray;
typedef TArray<NWndEntry*> TWndEntryArray;
typedef TArray<NControl*> TControlArray;
typedef TArray<NActionItem*> TActionItemArray;

// 0x10026bf0, 0x10022980 - NCTabControl's tabs (the copy GetTabs returns)
NW_HOOK_CTOR_ARGS(0x10026bf0, TRadioButtonArray, (const TRadioButtonArray& A), (A))
NW_HOOK_DTOR(0x100271e0, TRadioButtonArray)
// 0x1002eff0 - NCVirtualWndMain's window lists
NW_HOOK_DTOR(0x1002eff0, TWndEntryArray)

// 0x1002d420 - Last(c) of those lists with its check (UnTemplate.h 408; the TArray of include/ue2 has none): the
// element c places from the end, by reference. DispatchWndMsg takes the top of the focus stack with it. The
// path is the C2 build's (NW_UNTEMPLATE_H of UE2Core.h still has the C1 one).
struct TWndEntryArrayLast : public TWndEntryArray
{
    NWndEntry*& LastChecked(INT c = 0)
    {
        NW_CHECK_AT(c<ArrayNum, "e:\\work\\localization\\2004_12_01\\warfare2110\\core\\inc\\UnTemplate.h", 408);
        return ((NWndEntry**)Data)[ArrayNum - c - 1];
    }
    // UnTemplate.h's RemoveItem (the stock keeps the instance out of line): every element equal to Item goes, the
    // count removed comes back
    INT RemoveItem(NWndEntry* const& Item)
    {
        INT Original = ArrayNum;
        for (INT Index = 0; Index < ArrayNum; Index++)
            if (((NWndEntry**)Data)[Index] == Item)
                Remove(Index--);
        return Original - ArrayNum;
    }
};
NW_HOOK_METHOD(0x1002d420, TWndEntryArrayLast, LastChecked)
// 0x1002e460 - the focus stack's PopFocusWindow takes the window out of it with it
NW_HOOK_METHOD(0x1002e460, TWndEntryArrayLast, RemoveItem)
// 0x100323d0 - NCHtmlViewer's controls and the page tree's children
NW_HOOK_DTOR(0x100323d0, TControlArray)
// 0x100433a0 - NCActionItemWnd's icons
NW_HOOK_DTOR(0x100433a0, TActionItemArray)
// 0x10048c80 - a jump to 0x10004ce0 (NCChatWnd's unwind code)
NW_HOOK_DTOR(0x10048c80, TStringPtrArray)
// 0x100c0200 - NCQuestInfoWnd's list. The stock instance removes with an element size of 4: an array of pointers
// (NCQuestInfoWnd.h still declares TArray<NQuestEntry>, the C1 layout, whose destructor makes the same calls)
NW_HOOK_DTOR(0x100c0200, TPtrArray)
// 0x100d84b0 - NCAbnormalStatusWnd's statuses, pointers to the records as well (the console's AddAbnormalStatus
// reads them so: NConsoleWndData.cpp)
NW_HOOK_DTOR(0x100d84b0, TPtrArray)

/*-----------------------------------------------------------------------------
    TArray of structures.
-----------------------------------------------------------------------------*/

typedef TArray<NListItem> TListItemArray;
typedef TArray<FMinimapTarget> TMinimapTargetArray;
typedef TArray<NQuestEntry> TQuestEntryArray;
typedef TArray<FAbnormalStatus> TAbnormalStatusArray;
typedef TArray<NImeText> TImeTextArray;
typedef TArray<NChatBalloon> TChatBalloonArray;
typedef TArray<FName> TNameArray;

// 0x1001ba40, 0x10018460 - NCListBox's lines (Remove: the chat and petition lists' AddString)
NW_HOOK_METHOD(0x1001ba40, TListItemArray, Remove)
NW_HOOK_DTOR(0x1001c050, TListItemArray)
// 0x10099e20, 0x100792e0 - the minimap's and the radar's targets
NW_HOOK_METHOD(0x10099e20, TMinimapTargetArray, Remove)
NW_HOOK_DTOR(0x1009a560, TMinimapTargetArray)
// 0x10004140, 0x10003a10, 0x10003bb0 - the IMEs' clauses and the candidate box's items
NW_HOOK_METHOD(0x10004140, TImeTextArray, Empty)
NW_HOOK_METHOD(0x10004160, TImeTextArray, AddItem)
NW_HOOK_DTOR(0x10004370, TImeTextArray)
// 0x10050ac0, 0x100424e0 - NCConsole's chat balloons (each one's text pointer cleared)
NW_HOOK_METHOD(0x10050ac0, TChatBalloonArray, Remove)
NW_HOOK_DTOR(0x100517d0, TChatBalloonArray)
// 0x100abb00 - the pawn viewer's animation steps
NW_HOOK_METHOD(0x100abb00, TNameArray, Empty)

/*-----------------------------------------------------------------------------
    TArray<INT>: the HTML alignment stacks.
-----------------------------------------------------------------------------*/

typedef TArray<INT> TIntArray;

// 0x100326d0 - a jump to the pointer arrays' destructor 0x10028700 (the stacks' atexit code)
NW_HOOK_DTOR(0x100326d0, TIntArray)
// 0x10033ea0 - Pop (the stock one calls its out-of-line Remove)
NW_HOOK_METHOD(0x10033ea0, TIntArray, Pop)
// 0x1001fb70 - the destructor instance of the arrays of 4-byte elements the windows keep (NCMultiEdit's IntList,
// NCInventoryWnd's, NCSiegeDateInfoWnd's, NToolTipInfo::Fill's and NCQuestInfoWnd's Steps)
NW_HOOK_DTOR(0x1001fb70, TIntArray)

/*-----------------------------------------------------------------------------
    More destructor instances: one per class that keeps the array (the stock compiler emitted each one again).
-----------------------------------------------------------------------------*/

typedef TArray<ThaiCharacter> TThaiCharArray;
typedef TArray<NChatLimiterItem> TChatLimiterItemArray;

// 0x1000d540 - the Thai clusters of the edit boxes and of the device (20-byte elements)
NW_HOOK_DTOR(0x1000d540, TThaiCharArray)
// 0x1008f7b0 - the macro window's, the macro grid's and the macro editor's lists of pointers
NW_HOOK_DTOR(0x1008f7b0, TPtrArray)
// 0x100c37c0 - NCRecipeListBox's entries
NW_HOOK_DTOR(0x100c37c0, TPtrArray)
// 0x1007af00 - NCHennaListBox's entries
NW_HOOK_DTOR(0x1007af00, TPtrArray)
// 0x100dde60 - NCTrainListBox's entries
NW_HOOK_DTOR(0x100dde60, TTrainEntryArray)
// 0x10048ba0 - the chat flood control's lines (8-byte elements)
NW_HOOK_DTOR(0x10048ba0, TChatLimiterItemArray)

/*-----------------------------------------------------------------------------
    Cast, TMap, FString.
-----------------------------------------------------------------------------*/



// 0x1001efd0 - the multi-line edit box's line lengths
NW_HOOK_METHOD(0x1001efd0, FString, Len)

/*-----------------------------------------------------------------------------
    The console's.
-----------------------------------------------------------------------------*/

typedef TArray<NTimer> TTimerArray;
typedef TArray<NSysMsgParam> TSysMsgParamArray;

// 0x10061d40 - Tick drops the killed timers
NW_HOOK_METHOD(0x10061d40, TTimerArray, Remove)
// 0x1006b5d0, 0x10055660 - the console's timers and system message parameters (for
// ~NConsoleWnd and the unwinding)
NW_HOOK_DTOR(0x1006b5d0, TTimerArray)
NW_HOOK_DTOR(0x1006b540, TSysMsgParamArray)

// TMap's iterator (UnTemplate.h): the map, its pairs, the index, the count removed while
// walking; with any removed, its destructor relaxes the hash (TMapBase::Relax, inline)
template <class T> struct NDataMapIterator
{
    NDataMap<T>& Map;
    TArray<typename NDataMap<T>::NPair>& Pairs;
    INT Index;
    INT Removed;

    NDataMapIterator(NDataMap<T>& InMap) : Map(InMap), Pairs(InMap.Pairs), Index(0), Removed(0) {}
    ~NDataMapIterator()
    {
        if (Removed)
        {
            while (Map.HashCount > Map.Pairs.Num() * 2 + 8)
                Map.HashCount /= 2;
            Map.Rehash();
        }
    }
};
typedef NDataMapIterator<NViewItemData> TViewItemMapIterator;

// 0x1006b660 - one instance for every map's iterator: NConsoleWnd::IsValidName calls it, the
// unwinding of the others jumps to it
NW_HOOK_DTOR(0x1006b660, TViewItemMapIterator)

typedef NDataMap<NViewItemData> TViewItemMap;

// 0x10061dc0 - TMapBase::FindRef of the item table: the value under the id, NULL when the chain of
// its hash slot has none (NConsoleWnd's item functions call it)
NW_HOOK_METHOD(0x10061dc0, TViewItemMap, FindRef)
