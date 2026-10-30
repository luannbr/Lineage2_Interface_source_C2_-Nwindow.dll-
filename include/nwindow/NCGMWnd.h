#pragma once
// The GM's windows. NCGMWnd (UClass "CGMWnd", 0xC0; desktop +0x114, //gmwnd): a player's name
// and six buttons that ask the server about that player and open the matching view window of
// the desktop (NCConsole::ToggleGM*): Status (NCGMDetailStatusWnd), Inventory, Skills
// (NCGMMagicSkillWnd), Quest, Warehouse (NCGMStoreWnd) and Clan; the seventh button closes the
// view windows.
//
// The view windows are the player's windows under a title bar, filled by the console with the
// server's answer and made read-only: nothing is dragged out of their grids (the item windows'
// release and right button only put the carried item down), the quest window's Cancel is
// disabled, the clan page's buttons are all disabled.
//   NCGMInventoryWnd (0xB0; desktop +0x118): the inventory's paperdoll and two pages with the
//     adena, no weight bar, no trash or crystallize icons.
//   NCGMQuestWnd (0xCC; desktop +0x124), NCGMActionWnd (0x9C; desktop +0x12C, the clan page
//     NCGMPledgeWnd only).

#include "nwindow/NCEquipItemWnd.h"
#include "nwindow/NCInvenItemWnd.h"
#include "nwindow/NCPledgeWnd.h"
#include "nwindow/NCQuestWnd.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCButton;
class NCEditBox;
class NCTabControl;
struct NItemInfo;

// NWM_COMMAND ids (names ours)
enum
{
    NID_GM_NAME = 0x1505,
    NID_GM_INVENTORY = 0xF0F006,
    NID_GM_STATUS = 0xF0F007,
    NID_GM_SKILLS = 0xF0F008,
    NID_GM_QUEST = 0xF0F00A,
    NID_GM_CLAN = 0xF0F00B,
    NID_GM_WAREHOUSE = 0xF0F00C,
    NID_GM_FRAME = 0xF0F015,
    NID_GM_CLOSE = 0xF0F016,
    NID_GM_NEW1 = 0xF0F00D,           // new in C2
    NID_GM_NEW2 = 0xF0F00E,
    NID_GM_NEW3 = 0xF0F00F,
    NID_GM_NEW4 = 0xF0F01C,
    NID_GM_NEW5 = 0xF0F02C,
    NID_GM_NEW6 = 0xF0F03C,
    NID_GMINVENTORY_FRAME = 0xF0F00D,
    NID_GMINVENTORY_TABS = 0xF0F00E,
    NID_GMACTION_FRAME = 0xF0F018,
    NID_GMQUEST_FRAME = 0xF0F119,
    NID_GMQUEST_TABS = 0xF0F120,
    NID_GMQUEST_CANCEL = 0xF0F121,     // disabled, and the window procedure ignores it
    NID_GMQUEST_LIST = 0xF0F122,       // not handled: a quest clicked shows nothing
};

class NCGMWnd : public NCWnd
{
    NW_DECLARE_CLASS(NCGMWnd, NCWnd, 0, 0x101B5F90)

    UTexture* TopTex;                // +0x98 "sek.ui02"
    UTexture* MidTex;                // +0x9C "sek.cbui157"
    UTexture* BottomTex;             // +0xA0 "sek.ui03"
    NCEditBox* NameEdit;             // +0xA4 the player's name (ToggleGMWnd focuses it)
    NCButton* InventoryButton;       // +0xA8
    NCButton* SkillsButton;          // +0xAC
    NCButton* StatusButton;          // +0xB0
    NCButton* QuestButton;           // +0xB4
    NCButton* WarehouseButton;       // +0xB8
    NCButton* ClanButton;            // +0xBC
    NCButton* CloseButton;           // +0xC0
    NCWnd* NewChild[6];              // +0xC4 .. +0xD8 new in C2: OnCreate makes them, OnDestroy deletes them

    NCGMWnd();                                                       // 0x10076fd0
    // ~NCGMWnd                                                      // 0x10077050

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x100732f0
    virtual INT OnCreate();                                          // 36 0x100770a0
    virtual INT OnDestroy();                                         // 37 0x10077ac0
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x10077da0
    // the buttons (names ours)
    virtual INT OnInventory();                                       // 67 0x100734c0
    virtual INT OnSkills();                                          // 68 0x10073520
    virtual INT OnStatus();                                          // 69 0x10073580
    virtual INT OnQuest();                                           // 70 0x100735e0
    virtual INT OnWarehouse();                                       // 71 0x10073640
    virtual INT OnClan();                                            // 72 0x100736a0
    virtual INT OnClose();                                           // 73 0x10073700
    // the six new buttons of C2 (names ours): the builder commands "teleportto", "recall" and "snoop" for the name typed
    // (1..3; the third has no button: OnCreate does not make it), and the searches of an NPC, an item and a skill by name
    // (4..6: the match's id and name in a dialog box)
    virtual INT OnNewButton1();                                      // 74 0x10073860 (id 0xF0F00D)
    virtual INT OnNewButton2();                                      // 75 0x10073920 (id 0xF0F00E)
    virtual INT OnNewButton3();                                      // 76 0x100739e0 (id 0xF0F00F)
    virtual INT OnNewButton4();                                      // 77 0x10078bf0 (id 0xF0F01C) NPC table (GL2GameData +0x27E8)
    virtual INT OnNewButton5();                                      // 78 0x10078d60 (id 0xF0F02C) item table (+0x27E4)
    virtual INT OnNewButton6();                                      // 79 0x10078ec0 (id 0xF0F03C) skill table (+0x4CB8)
};
NW_ASSERT_OFFSET(NCGMWnd, NameEdit, 0xA4);
NW_ASSERT_OFFSET(NCGMWnd, CloseButton, 0xC0);
NW_ASSERT_SIZE(NCGMWnd, 0xDC);

// the GM inventory's pages (UClass "CGMInvenItemWnd", 0x13C)
class NCGMInvenItemWnd : public NCInvenItemWnd
{
    NW_DECLARE_CLASS(NCGMInvenItemWnd, NCInvenItemWnd, 0, 0x101B4B08)

    NCGMInvenItemWnd();                                              // 0x100784a0
    NCGMInvenItemWnd(INT InColumns, INT InVisibleRows, INT InMaxItems, INT InCellSize); // 0x10078540
    // ~NCGMInvenItemWnd                                             // 0x100784f0

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x10072f70
    // NCGMSkillItemWnd::OnLButtonUp's body (0x1005bdc0, hooked there): unhooked copies
    virtual INT OnLButtonUp(WPARAM wParam, LPARAM lParam);           // 62 0x10072f90 (folded)
    virtual INT OnRButtonDown(WPARAM wParam, LPARAM lParam);         // 63 0x10072f90 (folded)
    virtual INT OnLButtonDblClick(WPARAM wParam, LPARAM lParam);     // 73 0x10072f90 (folded)
};
NW_ASSERT_SIZE(NCGMInvenItemWnd, 0x124);

// the GM inventory's paperdoll (UClass "CGMEquipItemWnd", 0x254)
class NCGMEquipItemWnd : public NCEquipItemWnd
{
    NW_DECLARE_CLASS(NCGMEquipItemWnd, NCEquipItemWnd, 0, 0x101B5A70)
    BYTE Unknown250;                      // +0x250 (new in C2; 0 from the constructor and from NCGMInventoryWnd::ClearItems)
    BYTE Unknown251;                      // +0x251 (the same)
    BYTE _UnknownTail252[0x254 - 0x252];  // +0x252 padding up to sizeof

    NCGMEquipItemWnd(INT InCellSize = 32);                                // 0x100783f0
    // ~NCGMEquipItemWnd                                             // 0x10078450

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x10072e40
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x10073cd0
    virtual INT OnLButtonUp(WPARAM wParam, LPARAM lParam);           // 62 0x10072e60
    virtual INT OnRButtonDown(WPARAM wParam, LPARAM lParam);         // 63 0x10072e60 (folded)
    virtual INT GetEquipSlot(INT BodyPart, INT ObjectID);            // 67 0x10072e80 the slot by the body part bits alone
    virtual INT OnLButtonDblClick(WPARAM wParam, LPARAM lParam);     // 68 0x10072e60 (folded)

    // names ours (the console's adds go through NCGMInventoryWnd::AddItem)
    UBOOL AddItem(NItemInfo* Info);                                  // 0x10077f50 NCEquipItemWnd's, but a second ring (earring)
                                                                     //   is shown on the other slot
};
NW_ASSERT_SIZE(NCGMEquipItemWnd, 0x254);

class NCGMInventoryWnd : public NCWnd
{
    NW_DECLARE_CLASS(NCGMInventoryWnd, NCWnd, 0, 0x101B7958)

    UTexture* BackTex;               // +0x98 "L2UI.InventoryWnd.InventoryBack"
    UTexture* AdenaTex;              // +0x9C "L2UI.WindowIcon.WindowIconAdena"
    INT Unknown9C;                   // +0xA0   // C2: offset derived, no evidence
    NCGMInvenItemWnd* ItemWnd[2];    // +0xA4 the pages: 0 "Item", 1 "Quest"
    NCGMEquipItemWnd* EquipWnd;      // +0xAC
    NCTabControl* TabCtrl;           // +0xB0

    NCGMInventoryWnd();                                              // 0x100741a0
    // ~NCGMInventoryWnd                                             // 0x10074200

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x10043ef0 (folded)
    virtual INT OnCreate();                                          // 36 0x10078570
    virtual INT OnDestroy();                                         // 37 0x100743f0
    virtual void ShowWindow();                                       // 43 0x10073000
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x10074250

    // the console's calls (names ours)
    void ClearItems();                                               // 0x10072fb0
    UBOOL AddItem(NItemInfo* Info);                                  // 0x10078160
    void AddHenna(INT HennaID, INT bUsable);                         // 0x10073020 (name ours) the paperdoll's (a tail jump)
    void ClearHennas();                                              // 0x10073030 (name ours) the paperdoll's (a tail jump)
};
NW_ASSERT_OFFSET(NCGMInventoryWnd, ItemWnd, 0xA4);
NW_ASSERT_OFFSET(NCGMInventoryWnd, TabCtrl, 0xB0);
NW_ASSERT_SIZE(NCGMInventoryWnd, 0xB4);

class NCGMQuestWnd : public NCQuestWnd
{
    NW_DECLARE_CLASS(NCGMQuestWnd, NCQuestWnd, 0, 0x101B69E0)

    NCGMQuestWnd();                                                  // 0x10076370
    // ~NCGMQuestWnd                                                 // 0x100763c0

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x100730f0
    virtual INT OnCreate();                                          // 36 0x10076410
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x100764d0
    virtual UBOOL IsInDropZone(INT X, INT Y) { return 0; }           // 67 0x10001ac0 (folded)
};
NW_ASSERT_SIZE(NCGMQuestWnd, 0xD8);

// the GM's clan page (UClass "CGMPledgeWnd", 0x3B0): drawn 17 pixels lower than the player's
class NCGMPledgeWnd : public NCPledgeWnd
{
    NW_DECLARE_CLASS(NCGMPledgeWnd, NCPledgeWnd, 0, 0x101B7438)

    NCGMPledgeWnd();                                                 // 0x10076760
    // ~NCGMPledgeWnd                                                // 0x100767b0

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x10073180
    virtual INT OnDestroy();                                         // 37 0x100731a0
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x10076800
};
NW_ASSERT_SIZE(NCGMPledgeWnd, 0x8FC);

class NCGMActionWnd : public NCWnd
{
    NW_DECLARE_CLASS(NCGMActionWnd, NCWnd, 0, 0x101B64B8)

    UTexture* BackTex;               // +0x98 "L2UI.AcitonWnd.PledgeBack" (sic; never drawn)
    NCGMPledgeWnd* PledgeWnd;        // +0x9C

    NCGMActionWnd();                                                 // 0x10076bd0
    // ~NCGMActionWnd                                                // 0x10076c20

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x10043ef0 (folded)
    virtual INT OnCreate();                                          // 36 0x10076c70
    virtual INT OnDestroy();                                         // 37 0x10076f20 (shared)
    virtual void ShowWindow();                                       // 43 0x10073220
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x10076ed0 (shared)
};
NW_ASSERT_OFFSET(NCGMActionWnd, PledgeWnd, 0x9C);
NW_ASSERT_SIZE(NCGMActionWnd, 0xA0);

#pragma pack(pop)