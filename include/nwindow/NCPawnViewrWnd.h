#pragma once
// PawnView - the developers' pawn viewer (not in the normal game: NConsoleWnd's
// InitializePawnViewer makes it, the console's PawnViewerWnd[0]). It dresses and animates the
// pawn the player controls:
//
// NCPawnViewrWnd - the main window (UClass "CPawnViewrWnd", 0x27D0): three pages over the
// pawn (combined animations, hair meshes, a crowd simulator), the item table by category as
// icon grids or as text lists (the "GUI" check box picks), a search of the item names, and the
// weapon of a weapon type for the animations ("Use"). Each item worn goes into the record of
// the default character chosen in NCPawnView_CharacterSelectWnd (Characters, 18 races /
// genders / class types), which dresses the pawn (User::SetPawnResource).
//
// NCPawnView_CharacterSelectWnd - a top-level window (UClass "CPawnView_CharacterSelectWnd",
// 0xBC): the default character in a combo box (the player's pawn is replaced by a new one of
// its class) and its mesh's animation sequences in NCPawnView_CharacterSelect_List (a double
// click plays one in a loop, a second one stops it), sorted by weapon or by action.
//
// NCPawnView_InvenItemWnd / NCPawnView_InvenTextWnd - a category's items as icons (a right or
// double click puts the item on or takes it off) and as text lines (a click does).
//
// NCPawnViewr_MixAnimWnd - three animations taken from the list and a speed: "Start" plays
// them as the three steps of a skill (APawn::SetMagicInfo / InitSkillProcess).
//
// NCPawnViewr_HairChangeWnd - hair style and color, then the two hair meshes (a / b) found for
// them in LineageWarriors, and their textures; "OK" puts them on the pawn.
//
// NCPawnViewr_Simulator - spawns N random characters around the pawn that attack each other,
// cast, cancel and get replaced, by the percentages typed (its per-frame part never runs:
// nothing sets bRunning).

#include "nwindow/NCDebugWnd.h"
#include "nwindow/NCInvenItemWnd.h"
#include "nwindow/NConsoleWnd.h"
#include "nwindow/NViewerData.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class APawn;
class NCButton;
class NCCheckBox;
class NCComboBox;
class NCEditBox;
class NCTabControl;
class NCPawnViewrWnd;

class NCPawnView_InvenItemWnd : public NCInvenItemWnd
{
    NW_DECLARE_CLASS(NCPawnView_InvenItemWnd, NCInvenItemWnd, 0, 0x101CB898)

    NCPawnView_InvenItemWnd();                                       // 0x100affe0
    NCPawnView_InvenItemWnd(INT InColumns, INT InVisibleRows, INT InMaxItems, INT InCellSize); // 0x100b0080
    // ~NCPawnView_InvenItemWnd                                      // 0x100b0030

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x100aa450 (shared)
    virtual INT OnMouseMove(WPARAM wParam, LPARAM lParam);           // 60 0x100abbc0
    virtual INT OnLButtonUp(WPARAM wParam, LPARAM lParam);           // 62 0x100abcb0 (C2: an item let go outside drops into the world)
    virtual INT OnRButtonDown(WPARAM wParam, LPARAM lParam);         // 63 0x100abe60
    virtual INT OnLButtonDblClick(WPARAM wParam, LPARAM lParam);     // 73 0x100aa4b0
};
NW_ASSERT_SIZE(NCPawnView_InvenItemWnd, 0x124);

class NCPawnView_InvenTextWnd : public NCWnd
{
    NW_DECLARE_CLASS(NCPawnView_InvenTextWnd, NCWnd, 0, 0x101CA920)

    NCListBox* List;                 // +0x98 item names; a line's param is its NItemEntry

    NCPawnView_InvenTextWnd();                                       // 0x100abb20
    // ~NCPawnView_InvenTextWnd                                      // 0x100abb70

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x100ab580
    virtual INT OnCreate();                                          // 36 0x100ad0c0
    virtual void ShowWindow() {}                                     // 43 0x100ab5e0 (folded)
    virtual INT OnInvenItemList(INT Index);                          // 67 0x100acf60
};
NW_ASSERT_SIZE(NCPawnView_InvenTextWnd, 0x9C);

class NCPawnView_CharacterSelect_List : public NCDebugWnd
{
    NW_DECLARE_CLASS(NCPawnView_CharacterSelect_List, NCDebugWnd, 0, 0x101CAE40)

    INT PlayingIndex;                // +0x1EC line whose animation plays, -1 = none

    NCPawnView_CharacterSelect_List();                               // 0x100b07b0
    NCPawnView_CharacterSelect_List(INT InMaxLines, INT InVisibleLines); // 0x100b0850
    // ~NCPawnView_CharacterSelect_List                              // 0x100b0800

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x1009c940 (shared)
    virtual INT OnCreate();                                          // 36 0x100ab7c0
    virtual INT OnDestroy();                                         // 37 0x100ab8c0
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x100ab840
    virtual INT OnRButtonDown(WPARAM wParam, LPARAM lParam);         // 63 0x100afe90 (C2: plays the animation once)
    virtual INT OnMouseWheel(WPARAM wParam, LPARAM lParam);          // 66 0x1009cb20 (shared)
    virtual INT OnLButtonDblClick(WPARAM wParam, LPARAM lParam);     // 71 0x100afe80
    virtual INT ScrollUp();                                          // 72 0x1009cae0 (shared)
    virtual INT ScrollDown();                                        // 73 0x1009caf0 (shared)
    virtual INT SetTopIndex(INT Index);                              // 74 0x1009cb00 (shared)

    INT PlayAnim(INT bLoop);                                         // 0x100adb00 (name ours): the animation of the selected line plays on the
                                                                     //   pawn (looped or once); on the line already playing it stops
};
NW_ASSERT_OFFSET(NCPawnView_CharacterSelect_List, PlayingIndex, 0x1EC);
NW_ASSERT_SIZE(NCPawnView_CharacterSelect_List, 0x1F0);

class NCPawnView_CharacterSelectWnd : public NCWnd
{
    NW_DECLARE_CLASS(NCPawnView_CharacterSelectWnd, NCWnd, 0, 0x101CB368)

    NCComboBox* NameCombo;           // +0x98 the default characters   // C2: offset derived, no evidence
    NCPawnView_CharacterSelect_List* AnimList; // +0x9C the pawn mesh's animations
    NCCheckBox* ByActionCheck;       // +0xA0 sort the animations by action (else by weapon)
    UTexture* TitleTex;              // +0xA4 "sek.cbui168"
    UTexture* BackTex;               // +0xA8 "sek.cbui144"
    UTexture* UnknownA8;             // +0xAC "sek.cbui67" (not drawn)
    UTexture* UnknownAC;             // +0xB0 "sek.cbui68" (not drawn)
    UTexture* UnknownB0;             // +0xB4 "sek.cbui69" (not drawn)
    INT UnknownB4;                   // +0xB8 (0)
    INT Selected;                    // +0xBC default character shown, -1 = none

    NCPawnView_CharacterSelectWnd();                                 // 0x100acd20
    // ~NCPawnView_CharacterSelectWnd                                // 0x100acda0

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x100ab4d0
    virtual INT OnCreate();                                          // 36 0x100b10b0
    virtual INT OnDestroy();                                         // 37 0x100acef0 (shared)
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x100acdf0
    virtual INT OnNameComboBox(INT Index);                           // 67 0x100b0440
    virtual INT OnAniNameComboBox(INT Index) { return 1; }           // 68 0x100ab570 (folded)
    virtual INT OnByActionCheck();                                   // 69 0x100b07a0 (name ours)

    void UpdateAnimList();                                           // 0x100afb20 (name ours)
};
NW_ASSERT_OFFSET(NCPawnView_CharacterSelectWnd, TitleTex, 0xA4);
NW_ASSERT_OFFSET(NCPawnView_CharacterSelectWnd, Selected, 0xBC);
NW_ASSERT_SIZE(NCPawnView_CharacterSelectWnd, 0xC0);

class NCPawnViewr_MixAnimWnd : public NCWnd
{
    NW_DECLARE_CLASS(NCPawnViewr_MixAnimWnd, NCWnd, 0, 0x101CC800)

    NCButton* TakeButtons[3];        // +0x98 "Use": the list's animation into the edit line
    NCEditBox* AnimEdits[3];         // +0xA4 the skill's three animations (disabled: filled
                                     //   by the buttons only)
    NCEditBox* SpeedEdit;            // +0xB0 ("1.9")
    NCButton* StartButton;           // +0xB4 "Start"

    NCPawnViewr_MixAnimWnd();                                        // 0x100ad200
    // ~NCPawnViewr_MixAnimWnd                                       // 0x100ad260

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x100ab5f0
    virtual INT OnCreate();                                          // 36 0x100ad2b0
    virtual INT OnDestroy();                                         // 37 0x100ad9b0
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x100ad8a0
    // names ours
    virtual INT OnTake1();                                           // 67 0x100ab6a0
    virtual INT OnTake2();                                           // 68 0x100ab700
    virtual INT OnTake3();                                           // 69 0x100ab760
    virtual INT OnStart();                                           // 70 0x100ad700
};
NW_ASSERT_OFFSET(NCPawnViewr_MixAnimWnd, AnimEdits, 0xA4);
NW_ASSERT_SIZE(NCPawnViewr_MixAnimWnd, 0xB8);

class NCPawnViewr_HairChangeWnd : public NCWnd
{
    NW_DECLARE_CLASS(NCPawnViewr_HairChangeWnd, NCWnd, 0, 0x101CC2E0)

    // C2 layout from OnCreate (0x100ADD80), the constructor (0x100ADC50) and the handlers; the window was redesigned:
    // the "a" and "b" hair meshes of a hair number, found for a style suffix and a color, and a face list; names ours
    NCComboBox* HairStyleCombo;      // +0x98 id 0x104, 0..4: the "_m0S" suffix of the mesh names (the handler puts it back to 0)
    NCComboBox* HairColorCombo;      // +0x9C id 0x112, 0..9: the "_t0C" color of the textures
    NCComboBox* MeshACombo;          // +0xA0 id 0x105, the "a" hair meshes found
    NCComboBox* MeshBCombo;          // +0xA4 id 0x106, the "b" hair meshes found
    NCComboBox* FaceCombo;           // +0xA8 id 0x109, the faces found
    NCCheckBox* TogetherCheck;       // +0xAC id 0x110 "AH & BH TOGETHER", checked: a mesh picked picks the other with its number
    NCButton* ResetButton;           // +0xB0 id 0x108
    INT HairColor;                   // +0xB4 as picked (the character record gets it)
    INT HairNumber;                  // +0xB8 the mesh number picked (the character record HairStyle)
    INT HairStyle;                   // +0xBC the suffix (HairStyleCombo)
    INT Face;                        // +0xC0
    INT MeshANumbers[10];            // +0xC4 the mesh number of each MeshACombo line, then 10 for "No Mesh"
    INT MeshBNumbers[10];            // +0xEC the same for MeshBCombo
    INT FaceNumbers[10];             // +0x114 the face number of each FaceCombo line
    TCHAR MeshATexture[256];         // +0x13C "<race>.<race>_m00N_t0C_m0S_ah"
    TCHAR MeshBTexture[256];         // +0x33C "..._bh"

    NCPawnViewr_HairChangeWnd();                                     // 0x100adc50
    // ~NCPawnViewr_HairChangeWnd                                    // 0x100add30

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x100ab940
    virtual INT OnCreate();                                          // 36 0x100add80
    virtual INT OnDestroy();                                         // 37 0x100af150
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x100aed60
    // names ours
    virtual INT OnReset();                                           // 67 0x100ae8c0 button 0x108: everything back to 0
    virtual INT OnStyleCombo(INT Index);                             // 68 0x100ae970 the style combo goes back to 0
    virtual INT OnColorCombo(INT Index);                             // 69 0x100ae9b0 a color picked: the picked meshes get it
    virtual INT OnMeshA(INT Index);                                  // 70 0x100aea20
    virtual INT OnMeshB(INT Index);                                  // 71 0x100aeb50
    virtual INT OnFace(INT Index);                                   // 72 0x100aec80

    void OnOK();                                                     // 0x100ae6d0 (name ours): the picked meshes and textures on the pawn
    void FillFaceList();                                             // 0x100ae260 (name ours): the faces that exist
    void FillMeshLists();                                            // 0x100ae3d0 (name ours): the meshes that exist
    INT FindMeshA(INT Number);                                       // 0x100aba40 (name ours): the line of a mesh number
    INT FindMeshB(INT Number);                                       // 0x100aba70 (name ours)
    void SyncFromCharacter();                                        // 0x100afea0 (name ours): the values of the chosen default character
};
NW_ASSERT_OFFSET(NCPawnViewr_HairChangeWnd, HairColor, 0xB4);
NW_ASSERT_OFFSET(NCPawnViewr_HairChangeWnd, MeshANumbers, 0xC4);
NW_ASSERT_OFFSET(NCPawnViewr_HairChangeWnd, MeshBNumbers, 0xEC);
NW_ASSERT_OFFSET(NCPawnViewr_HairChangeWnd, FaceNumbers, 0x114);
NW_ASSERT_OFFSET(NCPawnViewr_HairChangeWnd, MeshATexture, 0x13C);
NW_ASSERT_OFFSET(NCPawnViewr_HairChangeWnd, MeshBTexture, 0x33C);
NW_ASSERT_SIZE(NCPawnViewr_HairChangeWnd, 0x53C);

// a pawn of the simulator (name ours; 8 bytes)
struct NSimPawn
{
    APawn* Pawn;
    INT Kind;                        // 1: attacks and casts the skill table's skills, 2: the
                                     //   bow share (casts a few fixed skills), 0: none
};

// the stock's std::vector<NSimPawn> (MSVC 7.1 layout: the empty allocator, then first, last and
// end of storage). The simulator only reserves it and writes its elements past the size.
struct NSimPawnVector
{
    INT Allocator;                   // +0x0
    NSimPawn* First;                 // +0x4
    NSimPawn* Last;                  // +0x8
    NSimPawn* End;                   // +0xC

    // <vector>'s reserve, _Xlen and _Ucopy's _Uninitialized_copy (NStdLib.cpp; names ours)
    void Reserve(DWORD Count);                                       // 0x1008dd30
    void Xlen() const;                                               // 0x1008dcb0
    static NSimPawn* UninitializedCopy(NSimPawn* First, NSimPawn* Last, NSimPawn* Dest,
                                       void* Al, INT Tag);           // 0x1008c720

    // the vector's destructor, inline in the simulator's
    ~NSimPawnVector()
    {
        if (First)
            appFree(First);
        First = NULL;
        Last = NULL;
        End = NULL;
    }
};
NW_ASSERT_SIZE(NSimPawnVector, 0x10);

// 4-byte packing: LastTime sits at +0xC4
#pragma pack(push, 4)
class NCPawnViewr_Simulator : public NCWnd
{
    NW_DECLARE_CLASS(NCPawnViewr_Simulator, NCWnd, 0, 0x101CBDB8)

    NCEditBox* CountEdit;            // +0x98 how many pawns
    NCEditBox* SkillEdit;            // +0x9C % a second to use a skill or attack
    NCEditBox* CancelEdit;           // +0xA0 % a second to cancel
    NCEditBox* ReplaceEdit;          // +0xA4 % a second to be replaced by a new pawn
    NCEditBox* BowEdit;              // +0xA8 % of the new pawns of the bow kind
    NCButton* StartButton;           // +0xAC "Start"
    UBOOL bRunning;                  // +0xB0 never set
    INT NumPawns;                    // +0xB4
    FLOAT SkillRate;                 // +0xB8 the percentages typed
    FLOAT CancelRate;                // +0xBC
    FLOAT ReplaceRate;               // +0xC0
    FLOAT BowRate;                   // +0xC4
    DOUBLE LastTime;                 // +0xC8 appSeconds
    DOUBLE DeltaTime;                // +0xD0 seconds since the last frame
    NSimPawnVector Pawns;            // +0xD8
    INT SkillIDs[2000];              // +0xE8 active skills with an animation (OnCreate)
    INT NumSkills;                   // +0x2028

    NCPawnViewr_Simulator();                                         // 0x100b1860
    // ~NCPawnViewr_Simulator                                        // 0x100b1900

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x100abaa0
    virtual INT OnCreate();                                          // 36 0x100b0890
    virtual INT OnDestroy();                                         // 37 0x100af8c0 (shared)
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x100af240
    // names ours
    virtual INT OnStart();                                           // 67 0x100b15b0
    virtual NSimPawn SpawnPawn();                                    // 68 0x100af570
    virtual INT TryAction(INT Index);                                // 69 0x100b0ce0
    virtual INT TryCancel(INT Index);                                // 70 0x100b0f80
    virtual INT TryReplace(INT Index);                               // 71 0x100b0fd0
};
NW_ASSERT_OFFSET(NCPawnViewr_Simulator, bRunning, 0xB0);
NW_ASSERT_OFFSET(NCPawnViewr_Simulator, LastTime, 0xC8);
NW_ASSERT_OFFSET(NCPawnViewr_Simulator, Pawns, 0xD8);
NW_ASSERT_OFFSET(NCPawnViewr_Simulator, SkillIDs, 0xE8);
NW_ASSERT_SIZE(NCPawnViewr_Simulator, 0x202C);
#pragma pack(pop)

class NCPawnViewrWnd : public NCWnd
{
    NW_DECLARE_CLASS(NCPawnViewrWnd, NCWnd, 0, 0x101CCD28)

    NCPawnView_InvenItemWnd* ItemWnds[8]; // +0x98 a grid per category (6 made: chest, legs,
                                     //   gloves, feet, headgear, etc)
    NCPawnView_InvenTextWnd* TextWnds[8]; // +0xB8 the same as text lists
    NCPawnView_CharacterSelectWnd* CharSelectWnd; // +0xD8 top-level (not a child)
    NCPawnViewr_MixAnimWnd* MixAnimWnd; // +0xDC the pages of PageTabs
    NCPawnViewr_HairChangeWnd* HairChangeWnd; // +0xE0
    NCPawnViewr_Simulator* Simulator; // +0xE4
    // C2 layout from OnCreate (0x100b19a0) and OnDestroy (0x100ac9b0); the viewer has a third row of controls
    // (an item id range, three more buttons): names ours, after the control ids
    UTexture* GridBackTex;           // +0xE8 "L2UI.QuestWnd.questBack3" (under the grids)
    UTexture* BackTex;               // +0xEC "L2UI.etcWnd.OptionBack"
    NCEditBox* SearchEdit;           // +0xF0 id 0x116
    NCEditBox* RangeEditA;           // +0xF4 (new in C2) id 0x11A, digits only (type 3), 23 x 16 at (25, 560)
    NCEditBox* RangeEditB;           // +0xF8 (new in C2) id 0x11B, digits only, at (60, 560)
    NCWnd* UnknownF8;                // +0xFC destroyed by OnDestroy, never created
    NCButton* SearchButton;          // +0x100 "Search" id 0x102
    NCButton* ListButton;            // +0x104 "View List" id 0x103
    NCButton* Button108;             // +0x108 (new in C2) id 0x117, 28 x 16 at (25, 500)
    NCButton* Button10C;             // +0x10C (new in C2) id 0x118, 50 x 16 at (70, 500)
    NCButton* Button110;             // +0x110 (new in C2) id 0x119, 42 x 16 at (110, 560)
    NCTabControl* CategoryTabs;      // +0x114 no pages: OnCategoryTab shows the grid or list
    NCTabControl* PageTabs;          // +0x118
    NCCheckBox* GUICheck;            // +0x11C "GUI": the grids, else the text lists
    User Characters[19];             // +0x120 the 18 default characters (race, gender, class
                                     //   from the creation's table; the 19th reads past it)
    ULevel* Level;                   // +0x2688 the engine's level when created (not read)
    INT CurrentPage;                 // +0x268C category shown
    NCComboBox* WeaponTypeCombo;     // +0x2690 "0(HAND)" .. "7(DUALFIST)"
    NCButton* UseButton;             // +0x2694 "Use": the weapon of that type on the character
    INT DefaultWeapons[8];           // +0x2698 first item id found of each weapon type, -1 none

    NCPawnViewrWnd();                                                // 0x100abfe0
    // ~NCPawnViewrWnd                                               // 0x100ac0b0

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x100aa540
    virtual INT OnCreate();                                          // 36 0x100b19a0
    virtual INT OnDestroy();                                         // 37 0x100ac9b0
    virtual void ShowWindow();                                       // 43 0x100acc10
    virtual void HideWindow();                                       // 44 0x100ab320
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x100ac760
    // names ours
    virtual INT vfn062() { return 1; }                               // 67 0x10005b80 (folded) id 0xF3
    virtual INT OnCategoryTab(INT Index);                            // 68 0x100ac100
    virtual INT OnGUICheck();                                        // 69 0x100ac940
    virtual INT OnSearch();                                          // 70 0x100b01d0
    virtual INT OnViewList();                                        // 71 0x100b03b0
    virtual INT OnUseWeapon();                                       // 72 0x100afa30
    // C2 only (names ours): the edit lines' Enter keys and the three buttons of the third row
    virtual INT OnSearchEdit(INT Code);                              // 73 0x100ab490 Enter in SearchEdit (id 0x116): search
    virtual INT OnRangeEdit(INT Code);                               // 74 0x100ab4b0 Enter in RangeEditB (id 0x11B): go
    virtual INT OnSpawnCharacter();                                  // 75 0x100ac1a0 button 0x117
    virtual INT OnToggleFlag();                                      // 76 0x100ac590 button 0x118 (flips a controller bit)
    virtual INT OnGoToMap();                                         // 77 0x100ac5c0 button 0x119 (teleport / open <x>_<y>.unr)

    void HidePages();                                                // 0x100aa6e0 (name ours)
    void AddAllCharacter();                                          // 0x100aa720
    INT GetItemCategory(NViewItemData* Data);                        // 0x100aaf50 (name ours; C2: one argument, ret 4)
    void FillItemInfo(NViewItemData* Data, NItemInfo* Info);         // 0x100aafe0 (name ours): the record into an NItemInfo
    void Clear();                                                    // 0x100ab260
    INT AddItem(INT Page, NItemInfo* Info);                          // 0x100ab3b0
    void AddAllItem();                                               // 0x100b00b0
};
NW_ASSERT_OFFSET(NCPawnViewrWnd, TextWnds, 0xB8);
NW_ASSERT_OFFSET(NCPawnViewrWnd, CharSelectWnd, 0xD8);
NW_ASSERT_OFFSET(NCPawnViewrWnd, GridBackTex, 0xE8);
NW_ASSERT_OFFSET(NCPawnViewrWnd, SearchEdit, 0xF0);
NW_ASSERT_OFFSET(NCPawnViewrWnd, CategoryTabs, 0x114);
NW_ASSERT_OFFSET(NCPawnViewrWnd, Characters, 0x120);
NW_ASSERT_OFFSET(NCPawnViewrWnd, Level, 0x2688);
NW_ASSERT_OFFSET(NCPawnViewrWnd, DefaultWeapons, 0x2698);
NW_ASSERT_SIZE(NCPawnViewrWnd, 0x26B8);

/*-----------------------------------------------------------------------------
    The viewer's helpers (inline in the stock code, names ours).
-----------------------------------------------------------------------------*/
namespace NPawnView
{
    // the console's PawnViewerWnd[0]
    FORCEINLINE NCPawnViewrWnd* Viewer()
    {
        return static_cast<NCPawnViewrWnd*>(GConsole->PawnViewerWnd[0]);
    }
    // the pawn the player's controller drives
    FORCEINLINE APawn* PlayerPawn() { return ViewActor()->Pawn; }
}

#pragma pack(pop)