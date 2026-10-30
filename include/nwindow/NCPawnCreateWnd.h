#pragma once
// NCPawnCreateWnd - the character creation screen (UClass "CPawnCreateWnd", 0x108; NConsoleWnd
// +0x36D0): the choices panel (NCPawnSetupWnd), the chosen class's description (NCPawnInfoWnd),
// "Create Character" and "Previous" in the lower right corner, and under the stage buttons that
// turn the chosen default character and zoom on its chest. The engine keeps 18 default
// characters, one per race / class type / gender (index (Race * 2 + ClassType) * 2 + Sex);
// choosing the race, class type or gender plays a lobby event that moves the camera
// (NCPawnSetupWnd::ExecCameraEvent).
//
// The four classes share one stretch of the stock code (0x10083c00 .. 0x10087410).

#include "nwindow/NControl.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

// C2: what the creation screen sends (UL2ConsoleWnd::RequestCharacterCreate, 0x10052480, by value, ret 0x60): the
// name is 24 characters now (C1: 20), so every field after it is 8 bytes further and the record is 0x60 bytes. It is
// the Engine's CharacterInfo (include/ue2/UE2Engine.h, the same C2 shape), so the console's request takes it as is
// (names ours, from NCPawnSetupWnd::GetCreateInfo 0x100a80a0 and NCPawnCreateWnd::OnCreateCharacter 0x100a8fd0)
typedef CharacterInfo NCharacterInfo;
NW_ASSERT_OFFSET(NCharacterInfo, Race, 0x30);
NW_ASSERT_OFFSET(NCharacterInfo, Stats, 0x3C);
NW_ASSERT_OFFSET(NCharacterInfo, HairStyle, 0x54);
NW_ASSERT_SIZE(NCharacterInfo, 0x60);

class NString;
class NCButton;
class NCExButton;
class NCPushButton;
class NCEditBox;
class NCPawnCreateWnd;

// NCSelectCtrl - a choice between SysString items, stepped through with a button on each side
// (UClass "CSelectCtrl", 0x1B8). A step wraps around and reports the new index to the parent
// (NWM_COMMAND, lParam = index).
class NCSelectCtrl : public NControl
{
    NW_DECLARE_CLASS(NCSelectCtrl, NControl, 0, 0x101CA3F8)

    NCPushButton* LeftButton;        // +0x1A4 "L2UI.LobbyWnd.LeftBtn"   // C2: offset derived, no evidence
    NCPushButton* RightButton;       // +0x1A8 "L2UI.LobbyWnd.rightBtn"   // C2: offset derived, no evidence
    INT Selected;                    // +0x1AC -1 = none
    TArray<NString*> Items;          // +0x1B0

    NCSelectCtrl();                                                  // 0x100a9e20
    // ~NCSelectCtrl                                                 // 0x100a9ee0

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x100a65f0
    virtual INT OnCreate();                                          // 36 0x100a6d00
    virtual INT OnDestroy();                                         // 37 0x100a9340
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x100a6e60
    // names ours
    virtual INT SelectPrev();                                        // 68 0x100a6670
    virtual INT SelectNext();                                        // 69 0x100a66c0

    // names ours
    void ClearItems();                                               // 0x100a92a0
    void AddItem(INT SysStringID);                                   // 0x100a9420

    // inlined in the stock code (name ours): an index past the end keeps the old one
    void SetSelected(INT Index)
    {
        if (Index < Items.Num())
            Selected = Index;
        if (Index < 0)
            Selected = -1;
    }
};
NW_ASSERT_OFFSET(NCSelectCtrl, LeftButton, 0x1A4);
NW_ASSERT_OFFSET(NCSelectCtrl, Selected, 0x1AC);
NW_ASSERT_OFFSET(NCSelectCtrl, Items, 0x1B0);
NW_ASSERT_SIZE(NCSelectCtrl, 0x1BC);

// NCPawnInfoWnd - the description of the chosen class under the choices (UClass
// "CPawnInfoWnd", 0x498): a classinfo text wrapped at 228 pixels.
class NCPawnInfoWnd : public NCWnd
{
    NW_DECLARE_CLASS(NCPawnInfoWnd, NCWnd, 0, 0x101C99A8)

    UTexture* BackTex;               // +0x98 "L2UI.LobbyWnd.MakingBack2"
    TCHAR Text[512];                 // +0x9C

    NCPawnInfoWnd();                                                 // 0x100a6f00
    // ~NCPawnInfoWnd                                                // 0x100a6f60

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x10043ef0 (shared)
    virtual INT OnCreate();                                          // 36 0x100a6fb0
    virtual INT OnDestroy();                                         // 37 0x100a6720 (shared)
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x100a6ff0

    // names ours
    void SetText(const TCHAR* InText);                               // 0x100a67b0
    void ClearText();                                                // 0x100a67a0
};
NW_ASSERT_OFFSET(NCPawnInfoWnd, Text, 0x9C);
NW_ASSERT_SIZE(NCPawnInfoWnd, 0x49C);

// NCPawnSetupWnd - the choices panel (UClass "CPawnSetupWnd", 0xD4): the name, then race, class
// type, gender, hair style, hair color and face, each an NCSelectCtrl. A choice clears the ones
// after it and enables the next one; hair and face are applied to the default character on the
// stage. Out of order choices get a system message.
class NCPawnSetupWnd : public NCWnd
{
    NW_DECLARE_CLASS(NCPawnSetupWnd, NCWnd, 0, 0x101C9EC8)

    NCSelectCtrl* RaceSelect;        // +0x98 sysstring 170..174
    NCSelectCtrl* ClassTypeSelect;   // +0x9C 175 warrior, 176 spellcaster (dwarves: warrior)
    NCSelectCtrl* SexSelect;         // +0xA0 177 male, 178 female
    NCSelectCtrl* HairStyleSelect;   // +0xA4 types A..D, men; A..F, women
    NCSelectCtrl* HairColorSelect;   // +0xA8 types A, B
    NCSelectCtrl* FaceSelect;        // +0xAC types A, B
    INT UnknownAC;                   // +0xB0 (0)
    UTexture* BackTex;               // +0xB4 "L2UI.LobbyWnd.MakingBack1"
    NCEditBox* NameEdit;             // +0xB8
    INT Race;                        // +0xBC the choices, -1 = none yet
    INT ClassType;                   // +0xC0
    INT Sex;                         // +0xC4
    INT HairStyle;                   // +0xC8
    INT HairColor;                   // +0xCC
    INT Face;                        // +0xD0
    INT CameraStep;                  // +0xD4 last camera view played (ExecCameraEvent)

    NCPawnSetupWnd();                                                // 0x100a70f0
    // ~NCPawnSetupWnd                                               // 0x100a7190

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x100a67e0
    virtual INT OnCreate();                                          // 36 0x100a9f40
    virtual INT OnDestroy();                                         // 37 0x100a73f0
    virtual void ShowWindow();                                       // 43 0x100a68e0
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x100a71e0
    // a selector changed (names ours)
    virtual INT OnRace(INT Index);                                   // 67 0x100a96a0
    virtual INT OnClassType(INT Index);                              // 68 0x100a9880
    virtual INT OnSex(INT Index);                                    // 69 0x100a9ae0
    virtual INT OnHairStyle(INT Index);                              // 70 0x100a76f0
    virtual INT OnHairColor(INT Index);                              // 71 0x100a7a00
    virtual INT OnFace(INT Index);                                   // 72 0x100a7d20

    // names ours
    INT GetCharacterType();                                          // 0x100a6900
    void ResetPreviews();                                            // 0x100a6920
    INT GetClassID();                                                // 0x100a69a0
    void SetMakingRace(INT InRace);                                  // 0x100a6a40
    void Reset();                                                    // 0x100a7600
    UBOOL GetCreateInfo(NCharacterInfo* Info);                        // 0x100a80a0
    void ExecCameraEvent(INT Step);                                  // 0x100a8280
    void InitSelectors();                                            // 0x100a94c0

private:
    NCPawnCreateWnd* CreateWnd();
};
NW_ASSERT_OFFSET(NCPawnSetupWnd, RaceSelect, 0x98);
NW_ASSERT_OFFSET(NCPawnSetupWnd, BackTex, 0xB4);
NW_ASSERT_OFFSET(NCPawnSetupWnd, Race, 0xBC);
NW_ASSERT_OFFSET(NCPawnSetupWnd, CameraStep, 0xD4);
NW_ASSERT_SIZE(NCPawnSetupWnd, 0xD8);

class NCPawnCreateWnd : public NCWnd
{
    NW_DECLARE_CLASS(NCPawnCreateWnd, NCWnd, 0, 0x101C9480)

    NCPawnSetupWnd* SetupWnd;        // +0x98
    NCPawnInfoWnd* InfoWnd;          // +0x9C
    RecommandedDataStr Recommended;  // +0xA0 the last class template the server sent
    NCButton* CreateButton;          // +0xF0
    NCButton* PreviousButton;        // +0xF4
    NCExButton* LeftButton;          // +0xF8 turn the character while held
    NCExButton* RightButton;         // +0xFC
    NCButton* ZoomInButton;          // +0x100 one of the two is shown; both have the same id
    NCButton* ZoomOutButton;         // +0x104
    UBOOL bZoomed;                   // +0x108

    NCPawnCreateWnd();                                               // 0x100a86b0
    // ~NCPawnCreateWnd                                              // 0x100a8710

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x100a6a50
    virtual INT OnCreate();                                          // 36 0x100a8760
    virtual INT OnDestroy();                                         // 37 0x100a8dd0
    virtual void ShowWindow();                                       // 43 0x100a6c60
    virtual void HideWindow();                                       // 44 0x100a6b70
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x100a8d20
    // names ours
    virtual INT OnPrevious();                                        // 67 0x100a6b50
    virtual INT OnCreateCharacter();                                 // 68 0x100a8fd0
    virtual INT OnTurnLeft(INT bPressed);                            // 69 0x100a6c80
    virtual INT OnTurnRight(INT bPressed);                           // 70 0x100a6cc0
    virtual INT OnZoom();                                            // 71 0x100a9100

    // names ours
    void SetRecommendedData(RecommandedDataStr Data);                // 0x100a6b30
    void EndRecommendedData() {}                                     // 0x100ab5e0 (empty, folded;
                                                                     //   the console calls it)
    void ShowViewButtons(UBOOL bShow);                               // 0x100a6b90
    void Reset();                                                    // 0x100a8fb0
};
NW_ASSERT_OFFSET(NCPawnCreateWnd, Recommended, 0xA0);
NW_ASSERT_OFFSET(NCPawnCreateWnd, CreateButton, 0xF0);
NW_ASSERT_OFFSET(NCPawnCreateWnd, ZoomOutButton, 0x104);
NW_ASSERT_SIZE(NCPawnCreateWnd, 0x10C);

#pragma pack(pop)