#pragma once
// NCMSViewrWnd - the skill viewer (UClass "CMSViewrWnd", 0x276C; titled "SKILLVIEWER"), a
// developer tool the console builds for its pawn viewer mode (NConsoleWnd::
// InitializePawnViewer, the console's PawnViewerWnd[2]): the level-1 skills of the skill table
// in three tabs (magic, active, passive) and the results of a search by name or by id in a
// fourth; a right click (or a double click) on a skill casts it with the player's pawn at its
// target. Beside it a top-level NCMSView_CharacterSelectWnd picks the character to cast with
// (race, sex and class: the pawn is spawned anew) and plays its animations.
//
// NCMSView_InvenItemWnd - the skill grids (UClass "CMSView_InvenItemWnd", 0x13C): nothing can
// be dragged.

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
    NID_MSVIEW_FRAME = 0xF2,
    NID_MSVIEW_NAMEEDIT = 0xF3,
    NID_MSVIEW_IDEDIT = 0xF4,
    NID_MSVIEW_SEARCHNAME = 0xF5,
    NID_MSVIEW_SEARCHID = 0xF6,
    NID_MSVIEW_TAB = 0xF7,
    NID_MSVIEW_SELECT_FRAME = (INT)0xAFF12011,
    NID_MSVIEW_SELECT_NAME = 0x1F04,
    NID_MSVIEW_SELECT_ANIM = 0x1F05,
};

class NCMSView_InvenItemWnd : public NCInvenItemWnd
{
    NW_DECLARE_CLASS(NCMSView_InvenItemWnd, NCInvenItemWnd, 0, 0x101BD600)

    NCMSView_InvenItemWnd();                                         // 0x1008a020
    NCMSView_InvenItemWnd(INT InColumns, INT InVisibleRows, INT InMaxItems, INT InCellSize); // 0x1008a0c0
    // ~NCMSView_InvenItemWnd                                        // 0x1008a070

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x100aa450 (hooked as
                                                                     //   NCNpcView_InvenItemWnd's)
    virtual INT OnMouseMove(WPARAM wParam, LPARAM lParam);           // 60 0x10088200
    virtual INT OnLButtonUp(WPARAM wParam, LPARAM lParam) { return 0; } // 62 0x10001ac0 (folded)
    virtual INT OnRButtonDown(WPARAM wParam, LPARAM lParam);         // 63 0x10089e90
    virtual INT OnLButtonDblClick(WPARAM wParam, LPARAM lParam);     // 73 0x10088290
};
NW_ASSERT_SIZE(NCMSView_InvenItemWnd, 0x124);

class NCMSView_CharacterSelectWnd : public NCWnd
{
    NW_DECLARE_CLASS(NCMSView_CharacterSelectWnd, NCWnd, 0, 0x101BE050)

    NCComboBox* NameCombo;               // +0x98 the characters: race, sex, class   // C2: offset derived, no evidence
    NCComboBox* AniCombo;                // +0x9C the pawn's animations
    UTexture* TitleTex;                  // +0xA0 "sek.cbui168"
    UTexture* BackTex;                   // +0xA4 "sek.cbui144"
    UTexture* Unknown67Tex;              // +0xA8 "sek.cbui67" (loaded, not drawn)
    UTexture* Unknown68Tex;              // +0xAC "sek.cbui68"
    UTexture* Unknown69Tex;              // +0xB0 "sek.cbui69"
    INT UnknownB0;                       // +0xB4
    INT Selected;                        // +0xB8 (-1) the character shown

    NCMSView_CharacterSelectWnd();                                   // 0x10089330
    // ~NCMSView_CharacterSelectWnd                                  // 0x100893b0

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x10088e50
    virtual INT OnCreate();                                          // 36 0x10089400
    virtual INT OnDestroy();                                         // 37 0x100898e0 (hooked as
                                                                     //   NCTrainWnd's)
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x100896a0
    // names from the guards
    virtual INT OnNameComboBox(INT Index);                           // 67 0x100899b0
    virtual INT OnAniNameComboBox(INT Index);                        // 68 0x10089d50
};
NW_ASSERT_OFFSET(NCMSView_CharacterSelectWnd, TitleTex, 0xA0);
NW_ASSERT_OFFSET(NCMSView_CharacterSelectWnd, Selected, 0xB8);
NW_ASSERT_SIZE(NCMSView_CharacterSelectWnd, 0xBC);

class NCMSViewrWnd : public NCWnd
{
    NW_DECLARE_CLASS(NCMSViewrWnd, NCWnd, 0, 0x101BDB28)

    NCMSView_InvenItemWnd* ItemWnd[4];   // +0x98 the tabs: magic, active, passive, the search's
    NCMSView_CharacterSelectWnd* CharSelectWnd; // +0xA8 top-level
    NCTabControl* TabCtrl;               // +0xAC
    UTexture* BackTex;                   // +0xB0 "L2UI.SkillWnd.SkillBack"
    UTexture* HtmlBackTex;               // +0xB4 "L2UI.etcwnd.NPCHtml_BACK" behind the search
    NCEditBox* NameEdit;                 // +0xB8
    NCEditBox* IDEdit;                   // +0xBC
    NCCheckBox* MagicCheck;              // +0xC0 the search lists the magic skills
    NCCheckBox* ActiveCheck;             // +0xC4 the active ones
    NCCheckBox* PassiveCheck;            // +0xC8 the passive ones
    NCButton* SearchNameButton;          // +0xCC
    NCButton* SearchIDButton;            // +0xD0
    User Characters[19];                 // +0xD4 the default characters (0x1F8 each in C2)
    ULevel* Level;                       // +0x263C the game's when created

    NCMSViewrWnd();                                                  // 0x10088ed0
    // ~NCMSViewrWnd                                                 // 0x10088fa0

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x10088350
    virtual INT OnCreate();                                          // 36 0x1008a8b0
    virtual INT OnDestroy();                                         // 37 0x100891a0
    virtual void ShowWindow();                                       // 43 0x10088d10
    virtual void HideWindow();                                       // 44 0x10088c80
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x10088ff0
    // names ours
    virtual INT OnSearchName();                                      // 67 0x1008a350
    virtual INT OnSearchID();                                        // 68 0x1008a610
    virtual INT OnIDEdit(INT Code);                                  // 69 0x10088c60 (new in C2: Enter in the ID line searches by ID)
    virtual INT OnNameEdit(INT Code);                                // 70 0x10088c40 (new in C2: Enter in the name line searches by name;
                                                                     //   the stock body is shared with NCAuthWnd::OnPasswordNotify)

    // names from the guards
    INT AddItem(INT Tab, NItemInfo* Info);                           // 0x10088db0
    void AddAllCharacter();                                          // 0x10088410
    void AddAllItem();                                               // 0x1008a0f0
};
NW_ASSERT_OFFSET(NCMSViewrWnd, CharSelectWnd, 0xA8);
NW_ASSERT_OFFSET(NCMSViewrWnd, Characters, 0xD4);
NW_ASSERT_OFFSET(NCMSViewrWnd, Level, 0x263C);
NW_ASSERT_SIZE(NCMSViewrWnd, 0x2640);

// the console's skill viewer (the third of its pawn viewer windows)
FORCEINLINE NCMSViewrWnd* MSViewer() { return static_cast<NCMSViewrWnd*>(GConsole->PawnViewerWnd[2]); }

#pragma pack(pop)