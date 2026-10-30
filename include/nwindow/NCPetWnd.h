#pragma once
// NCPetWnd - the pet window (UClass "CPetWnd", 0x184; opened from the pet's status bar or the
// action window): name and level, HP / MP / food / exp / weight bars, the Name button (a pet
// can be named once) and three pages under tabs: Information (NCPetDetailStatusWnd, the combat
// values), Action (NCPetActionWnd, the pet's actions) and Items (NCPetInventoryWnd).
// Everything is read again from the network handler at each paint.
//
// Desktop (NCConsole) +0x104 is this window.

#include "nwindow/NCWnd.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCActionItemWnd;
class NCButton;
class NCPetInventoryWnd;
class NCTabControl;
struct NItemInfo;

// NWM_COMMAND ids (names ours; the summon window uses the same tab id)
enum
{
    NID_PET_TABS = 0xFE001B,
    NID_PET_FRAME = 0xFE001C,
    NID_PET_NAME = 0xFE001D,          // the Name button
};

// The Information page (UClass "CPetDetailStatusWnd", 0x14C): P.Atk. ... Casting Spd.
class NCPetDetailStatusWnd : public NCWnd
{
    NW_DECLARE_CLASS(NCPetDetailStatusWnd, NCWnd, 0, 0x101CD768)

    UTexture* BackTex;               // +0x98 "L2UI.PetWnd.Pet02"
    PetInfo Pet;                     // +0x9C (0xAC in C2) the handler's record, read again by each paint

    NCPetDetailStatusWnd();                                          // 0x100b2d00
    // ~NCPetDetailStatusWnd                                         // 0x100b2d70

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x10043ef0 (folded)
    virtual INT OnCreate();                                          // 36 0x100b2ab0
    virtual INT OnDestroy();                                         // 37 0x100b2ae0 (folded)
    virtual INT OnPaint(NCGDevice* Device);                          // C2: slot gone (was 50)
};
NW_ASSERT_OFFSET(NCPetDetailStatusWnd, Pet, 0x9C);
NW_ASSERT_SIZE(NCPetDetailStatusWnd, 0x148);

// The Action page (UClass "CPetActionWnd", 0x9C): the pet actions of actionname-*.dat
// (category 4) in an icon grid.
class NCPetActionWnd : public NCWnd
{
    NW_DECLARE_CLASS(NCPetActionWnd, NCWnd, 0, 0x101CF128)

    UTexture* BackTex;               // +0x98 "L2UI.PetWnd.pet03"   // C2: offset derived, no evidence
    NCActionItemWnd* Items;          // +0x9C

    NCPetActionWnd();                                                // 0x100b2dc0
    // ~NCPetActionWnd                                               // 0x100b2e10

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x10043ef0 (folded)
    virtual INT OnCreate();                                          // 36 0x100b2e60
    virtual INT OnDestroy();                                         // 37 0x100b2f20
    virtual INT OnPaint(NCGDevice* Device);                          // C2: slot gone (was 50)

    void LoadActions();                                              // 0x10091700 (C1 only: no C2 match) (name ours)
};
NW_ASSERT_OFFSET(NCPetActionWnd, Items, 0x9C);
NW_ASSERT_SIZE(NCPetActionWnd, 0xA0);

class NCPetWnd : public NCWnd
{
    NW_DECLARE_CLASS(NCPetWnd, NCWnd, 0, 0x101CF650)

    UTexture* BackTex;               // +0x98 "L2UI.PetWnd.PetBack"
    UTexture* HPTex;                 // +0x9C "L2UI.StatusWnd.statusHP1"
    UTexture* MPTex;                 // +0xA0 "L2UI.StatusWnd.statusMP1"
    UTexture* EXPTex;                // +0xA4 "L2UI.StatusWnd.statusEXP1"
    UTexture* WeightTex[4];          // +0xA8 "L2UI.StatusWnd.statusWt1" .. 4 (as the status
                                     //   window: up to 50%, 66.66%, 80%, above)
    UTexture* FedTex;                // +0xB8 "L2UI.PetWnd.statusEat1"
    PetInfo Pet;                     // +0xBC (0xAC in C2) the handler's record, read again by each paint
    NCPetDetailStatusWnd* DetailWnd; // +0x168
    NCPetActionWnd* ActionWnd;       // +0x16C
    NCPetInventoryWnd* InventoryWnd; // +0x170
    NCTabControl* TabCtrl;           // +0x174
    NCButton* NameButton;            // +0x178
    UBOOL bCanName;                  // +0x17C (1) the console clears it (SetCanName)

    NCPetWnd();                                                      // 0x100b37e0
    // ~NCPetWnd                                                     // 0x100b3870

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x100b2ba0
    virtual INT OnCreate();                                          // 36 0x100b38c0
    virtual INT OnDestroy();                                         // 37 0x100b3d70
    virtual void ShowWindow();                                       // 43 0x100b45c0
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x100b3ec0
    virtual INT OnName();                                            // 67 0x100b4660 (name ours)

    // names ours (the console)
    void SetCanName(UBOOL bCan);                                     // 0x100b2c00
    void ClearInventory();                                           // 0x100b2c20
    UBOOL AddInventoryItem(NItemInfo* Info);                         // 0x100b6190 (C1 0x100940b0)
    UBOOL UpdateInventoryItem(INT Op, NItemInfo* Info);              // 0x100b6440 (C1 0x100940d0)
};
NW_ASSERT_OFFSET(NCPetWnd, WeightTex, 0xA8);
NW_ASSERT_OFFSET(NCPetWnd, Pet, 0xBC);
NW_ASSERT_OFFSET(NCPetWnd, DetailWnd, 0x168);
NW_ASSERT_OFFSET(NCPetWnd, bCanName, 0x17C);
NW_ASSERT_SIZE(NCPetWnd, 0x180);

#pragma pack(pop)