#pragma once
// NCVideoOptionWnd - the video page of the options window (UClass "CVideoOptionWnd", 0xDC):
// display mode and refresh rate (from the render device), gamma, texture / model / animation
// detail, shadows, ground detail, terrain and PC/NPC ranges, PC/NPC limit, reflections, color
// cursor, texture filter and "Lower Detail" (which fixes the detail controls low). Apply sets
// the globals and the render device and saves Option.ini ([Video], [ClippingRange],
// [L2WaterEffect]).

#include "nwindow/NCWnd.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCComboBox;
class NCCheckBox;

// NWM_COMMAND ids of the controls
enum
{
    NID_VIDEO_RESOLUTION = 0x1F08,
    NID_VIDEO_GAMMA = 0x1F09,
    NID_VIDEO_TEXTURE = 0x1F0A,
    NID_VIDEO_TERRAINRANGE = 0x1F0D,
    NID_VIDEO_PAWNRANGE = 0x1F0E,
    NID_VIDEO_CURSOR = 0x1F0F,
    NID_VIDEO_REFLECTION = 0x1F11,
    NID_VIDEO_REFRESH = 0x1F14,
    NID_VIDEO_ACTORLIMIT = 0x1F17,
    NID_VIDEO_MODELING = 0x1F18,
    NID_VIDEO_ANIMATION = 0x1F19,
    NID_VIDEO_LOWERDETAIL = (INT)0xF0F01465,
    NID_VIDEO_SHADOW = (INT)0xF0F01467,
    NID_VIDEO_GROUNDDETAIL = (INT)0xF0F01468,
    NID_VIDEO_TEXTUREFILTER = (INT)0xF0F01469,
    NID_VIDEO_FULLSCREEN = (INT)0xF0F0146A,   // phase 6 (NW_FEATURES): not in the stock page
};

class NCVideoOptionWnd : public NCWnd
{
    NW_DECLARE_CLASS(NCVideoOptionWnd, NCWnd, 0, 0x101C8508)

    NCComboBox* ResolutionCombo;           // +0x98 "%d*%d %dbit", the device's display modes
    NCComboBox* RefreshCombo;              // +0x9C "%dHz", the rates of the selected mode
    NCComboBox* GammaCombo;                // +0xA0 "5" .. "1": gamma 1.2 .. 0.4
    NCComboBox* TextureCombo;              // +0xA4 High / Medium / Low (GL2TextureDetail)
    NCComboBox* ModelingCombo;             // +0xA8 High / Low (GL2UseReducePawnLOD)
    NCComboBox* AnimationCombo;            // +0xAC High / Low (GL2UseSkipAnim)
    NCCheckBox* ShadowBox;                 // +0xB0 GIsL2Projector
    NCCheckBox* GroundDetailBox;           // +0xB4 GL2RenderDeco
    NCComboBox* TerrainRangeCombo;         // +0xB8 "5" .. "1": terrain clipping 8 .. 4
    NCComboBox* PawnRangeCombo;            // +0xBC "5" .. "1": PC/NPC clipping 3 .. 1
    NCCheckBox* CursorBox;                 // +0xC0 GL2UseColorCursor
    NCComboBox* ReflectionCombo;           // +0xC4 None / All / Sky (water effect type)
    NCCheckBox* LowerDetailBox;            // +0xC8 GL2KeepMinFrameRate
    NCCheckBox* TextureFilterBox;          // +0xCC GL2UseTrilinear
    NCComboBox* ActorLimitCombo;           // +0xD0 None / "5" .. "1" (GL2RenderActorLimitOpt)
    NCWnd* UnusedD0[3];                    // +0xD4 destroyed, never created (phase 6, NW_FEATURES:
                                           //   [0] is the "Full Screen" box, an NCCheckBox)

    NCVideoOptionWnd();                                              // 0x100a0840
    // ~NCVideoOptionWnd                                             // 0x100a08a0

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x100a0100
    virtual INT OnCreate();                                          // 36 0x100a5300
    virtual INT OnDestroy();                                         // 37 0x100a1030
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x100a08f0
    // names ours
    virtual INT OnResolutionChanged(INT Index);                      // 67 0x100a1480
    virtual INT OnRefreshChanged(INT Index) { return 1; }            // 68 0x100ab570 (folded)
    virtual INT OnLowerDetailClicked();                              // 69 0x100a2990

    // names ours
    void InitModes();                                                // 0x100a01a0 (Apply)
    void LoadSettings();                                             // 0x100a15d0 current values
    void Apply();                                                    // 0x100a1b90
    void ClearItems();                                               // 0x100a13c0 (C2 only; before InitItems)
    void InitItems();                                                // 0x100a4db0

    // phase 6 (defined only with NW_FEATURES): the "Full Screen" box shows the mode the viewport
    // is in now (LoadSettings, and every time the options window opens)
    void SyncModeBox();
};
NW_ASSERT_OFFSET(NCVideoOptionWnd, ResolutionCombo, 0x98);
NW_ASSERT_OFFSET(NCVideoOptionWnd, ActorLimitCombo, 0xD0);
NW_ASSERT_SIZE(NCVideoOptionWnd, 0xE0);

#pragma pack(pop)