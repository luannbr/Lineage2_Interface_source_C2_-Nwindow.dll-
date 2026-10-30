#pragma once
// NCSummonedWnd - the summon window (UClass "CSummonedWnd", 0x160): the pet window without
// food, exp, weight, name button and items: name and level, HP / MP bars, and the pages
// Information (NCSummonedDetailStatusWnd) and Action (NCSummonedActionWnd, the summon actions
// of actionname-*.dat, category 5). Both pages share their code with the pet's.
//
// Desktop (NCConsole) +0x108 is this window.

#include "nwindow/NCWnd.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCActionItemWnd;
class NCTabControl;

enum { NID_SUMMON_FRAME = 0xFE001E };  // the tabs use NID_PET_TABS (0xFE001B)

// The Information page (UClass "CSummonedDetailStatusWnd", 0x14C)
class NCSummonedDetailStatusWnd : public NCWnd
{
    NW_DECLARE_CLASS(NCSummonedDetailStatusWnd, NCWnd, 0, 0x101CDC98)

    UTexture* BackTex;               // +0x98 "L2UI.PetWnd.Summon02"
    PetInfo Summon;                  // +0x9C (0xAC in C2) the handler's record, read again by each paint

    NCSummonedDetailStatusWnd();                                     // 0x100b4700
    // ~NCSummonedDetailStatusWnd                                    // 0x100b4770

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x10043ef0 (folded)
    virtual INT OnCreate();                                          // 36 0x100b2c30
    virtual INT OnDestroy();                                         // 37 0x100b2c60 (folded)
    virtual INT OnPaint(NCGDevice* Device);                          // C2: slot gone (was 50) (folded)
};
NW_ASSERT_OFFSET(NCSummonedDetailStatusWnd, Summon, 0x9C);
NW_ASSERT_SIZE(NCSummonedDetailStatusWnd, 0x148);

// The Action page (UClass "CSummonedActionWnd", 0x9C)
class NCSummonedActionWnd : public NCWnd
{
    NW_DECLARE_CLASS(NCSummonedActionWnd, NCWnd, 0, 0x101CD248)

    UTexture* BackTex;               // +0x98 "L2UI.PetWnd.Summon03"   // C2: offset derived, no evidence
    NCActionItemWnd* Items;          // +0x9C   // C2: offset derived, no evidence

    NCSummonedActionWnd();                                           // 0x100b4e40
    // ~NCSummonedActionWnd                                          // 0x100b4e90

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x10043ef0 (folded)
    virtual INT OnCreate();                                          // 36 0x100b4ee0
    virtual INT OnDestroy();                                         // 37 0x100b4fa0 (folded)
    virtual INT OnPaint(NCGDevice* Device);                          // C2: slot gone (was 50) (folded)

    void LoadActions();                                              // 0x100b50d0 (C1: 0x10092f50) (name ours)
};
NW_ASSERT_OFFSET(NCSummonedActionWnd, Items, 0x9C);
NW_ASSERT_SIZE(NCSummonedActionWnd, 0xA0);

class NCSummonedWnd : public NCWnd
{
    NW_DECLARE_CLASS(NCSummonedWnd, NCWnd, 0, 0x101CE6E0)

    UTexture* BackTex;               // +0x98 "L2UI.PetWnd.Summon01"
    UTexture* HPTex;                 // +0x9C "L2UI.StatusWnd.statusHP1"
    UTexture* MPTex;                 // +0xA0 "L2UI.StatusWnd.statusMP1"
    NCSummonedDetailStatusWnd* DetailWnd; // +0xA4
    NCSummonedActionWnd* ActionWnd;  // +0xA8
    NCTabControl* TabCtrl;           // +0xAC
    PetInfo Summon;                  // +0xB0 (0xAC in C2) the handler's record, read again by each paint

    NCSummonedWnd();                                                 // 0x100b52a0
    // ~NCSummonedWnd                                                // 0x100930f0 (C1 only: no C2 match)

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x10043ef0 (folded)
    virtual INT OnCreate();                                          // 36 0x100b5370
    virtual INT OnDestroy();                                         // 37 0x100b5670
    virtual void ShowWindow();                                       // 43 0x100b5b10
    virtual void HideWindow();                                       // 44 0x100b5b90 (new in C2)
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x100b5770

    void ClearActions();                                             // 0x100b2ce0 (name ours, C2 only): the page's icons dropped
};
NW_ASSERT_OFFSET(NCSummonedWnd, DetailWnd, 0xA4);
NW_ASSERT_OFFSET(NCSummonedWnd, Summon, 0xB0);
NW_ASSERT_SIZE(NCSummonedWnd, 0x15C);

#pragma pack(pop)