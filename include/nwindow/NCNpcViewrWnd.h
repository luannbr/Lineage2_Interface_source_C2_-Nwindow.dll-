#pragma once
// NCNpcViewrWnd - the NPC viewer (UClass "CNpcViewrWnd", 0x2750), a developer tool the console
// builds for its pawn viewer mode (NConsoleWnd::InitializePawnViewer, the console's
// PawnViewerWnd[1]): the item table in a grid, the items whose name holds the text searched in a second
// one; a right click (or a double click) on an item dresses the pawn shown with it. Beside it a
// top-level NCNpcView_CharacterSelectWnd lists the NPC table (NPCs and monsters, by name or by
// mesh): a double click turns the player's pawn into that NPC, "Summon" spawns it next to the
// player; the pawn's animations play from a combo box and two edit lines set its speed.
//
// NCNpcView_CharacterSelect_List - that NPC list (UClass "CNpcView_CharacterSelect_List",
// 0x1E8, a debug list): each line's parameter is its NViewNpcData.
//
// NCNpcView_InvenItemWnd - the item grids (UClass "CNpcView_InvenItemWnd", 0x13C): nothing
// can be dragged.

#include "nwindow/NCDebugWnd.h"
#include "nwindow/NCInvenItemWnd.h"
#include "nwindow/NViewerData.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCButton;
class NCCheckBox;
class NCComboBox;
class NCEditBox;
class NCTabControl;
struct NItemInfo;

// NWM_COMMAND ids of the viewer's controls (names ours)
enum
{
    NID_NPCVIEW_FRAME = 0xF2,
    NID_NPCVIEW_SEARCH = 0xFC,
    NID_NPCVIEW_SEARCHEDIT = 0xFD,         // new in C2
    NID_NPCVIEW_TAB = 0xFE001A,
    NID_NPCVIEW_SELECT_FRAME = 0xF3,
    NID_NPCVIEW_SELECT_SEARCHEDIT = 0xF4,
    NID_NPCVIEW_SELECT_SEARCH = 0xF5,
    NID_NPCVIEW_SELECT_RESET = 0xF6,
    NID_NPCVIEW_SELECT_RATE = 0xF7,
    NID_NPCVIEW_SELECT_SPEED = 0xF8,
    NID_NPCVIEW_SELECT_APPLY = 0xF9,
    NID_NPCVIEW_SELECT_SUMMON = 0xFA,
    NID_NPCVIEW_SELECT_MESHNAME = 0xFB,
    NID_NPCVIEW_SELECT_SPAWNTEST = 0xFF,   // new in C2
    NID_NPCVIEW_SELECT_ANIM = 0x1F05,
};

class NCNpcView_InvenItemWnd : public NCInvenItemWnd
{
    NW_DECLARE_CLASS(NCNpcView_InvenItemWnd, NCInvenItemWnd, 0, 0x101C7AC8)

    NCNpcView_InvenItemWnd();                                        // 0x1009e790
    NCNpcView_InvenItemWnd(INT InColumns, INT InVisibleRows, INT InMaxItems, INT InCellSize); // 0x1009e830
    // ~NCNpcView_InvenItemWnd                                       // 0x1009e7e0

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x100aa450 (also the
                                                                     //   skill viewer's grids')
    virtual INT OnMouseMove(WPARAM wParam, LPARAM lParam) { return 1; } // 60 0x1009c3f0 (folded)
    virtual INT OnLButtonUp(WPARAM wParam, LPARAM lParam) { return 0; } // 62 0x10001ac0 (folded)
    virtual INT OnRButtonDown(WPARAM wParam, LPARAM lParam);         // 63 0x1009cb50
    virtual INT OnLButtonDblClick(WPARAM wParam, LPARAM lParam);     // 73 0x1009c420
};
NW_ASSERT_SIZE(NCNpcView_InvenItemWnd, 0x124);

class NCNpcView_CharacterSelect_List : public NCDebugWnd
{
    NW_DECLARE_CLASS(NCNpcView_CharacterSelect_List, NCDebugWnd, 0, 0x101C7078)

    NCNpcView_CharacterSelect_List();                                // 0x1009edf0
    NCNpcView_CharacterSelect_List(INT InMaxLines, INT InVisibleLines); // 0x1009ee90
    // ~NCNpcView_CharacterSelect_List                               // 0x1009ee40

    // 24, 61, 67, 68 and 69 are also NCPawnView_CharacterSelect_List's
    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x1009c940
    virtual INT OnCreate();                                          // 36 0x1009c960
    virtual INT OnDestroy();                                         // 37 0x1009ca60
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x1009c9e0
    virtual INT OnMouseWheel(WPARAM wParam, LPARAM lParam);          // 66 0x1009cb20
    virtual INT OnLButtonDblClick(WPARAM wParam, LPARAM lParam);     // 71 0x1009d060
    virtual INT ScrollUp();                                          // 72 0x1009cae0
    virtual INT ScrollDown();                                        // 73 0x1009caf0
    virtual INT SetTopIndex(INT Index);                              // 74 0x1009cb00

    void ToggleNames(UBOOL bMeshNames);                              // 0x1009d650 (guard name)
    // name ours: the scroll bar's range (an inline method the stock compiler emitted)
    void SetScrollRange(INT Total, INT Page);                        // 0x1009cb40
};
NW_ASSERT_SIZE(NCNpcView_CharacterSelect_List, 0x1EC);

class NCNpcView_CharacterSelectWnd : public NCWnd
{
    NW_DECLARE_CLASS(NCNpcView_CharacterSelectWnd, NCWnd, 0, 0x101C7598)

    NCNpcView_CharacterSelect_List* List; // +0x98
    NCComboBox* AniCombo;                // +0x9C the pawn's animations
    NCEditBox* SearchEdit;               // +0xA0
    NCButton* SearchButton;              // +0xA4
    NCButton* ResetButton;               // +0xA8 the whole table again
    NCEditBox* RateEdit;                 // +0xAC the pawn's speed factor (Rate)
    NCEditBox* SpeedEdit;                // +0xB0 its speed (Speed)
    NCButton* ApplyButton;               // +0xB4 the two edit lines to the pawn
    NCCheckBox* MeshNameCheck;           // +0xB8 labeled "Mesh name" (in Korean): the list
                                         //   shows the meshes
    NCButton* SummonButton;              // +0xBC labeled "Summon" (in Korean)
    NCButton* SpawnTestButton;           // +0xC0 (new in C2) "TEST", id 0xFF: the stress test (OnSpawnTest)
    NCCheckBox* NpcCheck;                // +0xC4 the NPCs are listed
    NCCheckBox* MonsterCheck;            // +0xC8 the monsters are listed
    UTexture* TitleTex;                  // +0xCC "sek.cbui168"
    UTexture* BackTex;                   // +0xD0 "sek.cbui144"
    FLOAT Rate;                          // +0xD4 (1.0)
    INT Speed;                           // +0xD8

    NCNpcView_CharacterSelectWnd();                                  // 0x1009d710
    // ~NCNpcView_CharacterSelectWnd                                 // 0x1009d760

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x1009e650
    virtual INT OnCreate();                                          // 36 0x1009eec0
    virtual INT OnDestroy();                                         // 37 0x1009d8b0
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x1009dae0
    // names ours
    virtual INT OnSpawnTest();                                       // 67 0x1009e230 (C2 only, name ours) button 0xFF: one skeleton archer, OnSummon's body
                                                                     //   spawned around the player (a developer stress test)
    virtual INT OnSearch();                                          // 68 0x1009f8a0
    virtual INT OnReset();                                           // 69 0x1009fb20
    virtual INT OnApply();                                           // 70 0x1009dcd0
    virtual INT OnToggleNames();                                     // 71 0x1009ddf0
    virtual INT OnSummon();                                          // 72 0x1009de10
    virtual INT OnSearchEdit(INT Code);                              // 73 0x10088c60 (C2 only, name ours) Enter in the search line searches
                                                                     //   (the body is NCMSViewrWnd::OnIDEdit's, hooked there)

    INT OnAniNameCombo(INT Index);                                   // 0x1009d7b0 (guard name)
};
NW_ASSERT_OFFSET(NCNpcView_CharacterSelectWnd, List, 0x98);
NW_ASSERT_OFFSET(NCNpcView_CharacterSelectWnd, TitleTex, 0xCC);
NW_ASSERT_SIZE(NCNpcView_CharacterSelectWnd, 0xDC);

class NCNpcViewrWnd : public NCWnd
{
    NW_DECLARE_CLASS(NCNpcViewrWnd, NCWnd, 0, 0x101C6B50)

    NCNpcView_InvenItemWnd* ItemWnd[2];  // +0x98 the tabs: all the items, the search's
    NCNpcView_CharacterSelectWnd* CharSelectWnd; // +0xA0 top-level
    NCTabControl* TabCtrl;               // +0xA4
    NCEditBox* SearchEdit;               // +0xA8
    NCButton* SearchButton;              // +0xAC
    UTexture* BackTex;                   // +0xB0 "L2UI.QuestWnd.questBack3"
    UTexture* OptionBackTex;             // +0xB4 "L2UI.etcWnd.OptionBack"
    User Npcs[19];                       // +0xB8 the default NPC records (0x1F8 each in C2)
    ULevel* Level;                       // +0x2620 the game's when created

    NCNpcViewrWnd();                                                 // 0x1009cd10
    // ~NCNpcViewrWnd                                                // 0x1009cdd0

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x1009c4b0
    virtual INT OnCreate();                                          // 36 0x1009fb70
    virtual INT OnDestroy();                                         // 37 0x1009cf30
    virtual void ShowWindow();                                       // 43 0x1009c800
    virtual void HideWindow();                                       // 44 0x1009c770
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x1009ce20
    virtual INT OnSearchBtnClick();                                  // 67 0x1009eb90 (guard name)
    virtual INT OnSearchEdit(INT Code);                              // 68 0x10088c40 (C2 only, name ours) Enter in the search line clicks the
                                                                     //   button (the body is NCAuthWnd::OnPasswordNotify's, hooked there)

    // names from the guards
    INT AddItem(INT Tab, NItemInfo* Info);                           // 0x1009c8a0
    void AddAllCharacter();                                          // 0x1009e860
    void AddAllItem();                                               // 0x1009ea60
    void FillItemInfo(NViewItemData* Data, NItemInfo* Info);         // 0x1009c530 (name ours): the record into an NItemInfo
};
NW_ASSERT_OFFSET(NCNpcViewrWnd, CharSelectWnd, 0xA0);
NW_ASSERT_OFFSET(NCNpcViewrWnd, Npcs, 0xB8);
NW_ASSERT_OFFSET(NCNpcViewrWnd, Level, 0x2620);
NW_ASSERT_SIZE(NCNpcViewrWnd, 0x2624);

// the console's NPC viewer (the second of its pawn viewer windows)
FORCEINLINE NCNpcViewrWnd* NpcViewer() { return static_cast<NCNpcViewrWnd*>(GConsole->PawnViewerWnd[1]); }

#pragma pack(pop)