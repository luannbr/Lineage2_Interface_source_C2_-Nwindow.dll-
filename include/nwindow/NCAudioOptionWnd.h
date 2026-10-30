#pragma once
// NCAudioOptionWnd - the audio page of the options window (UClass "CAudioOptionWnd", 0xA8):
// sound effect and music volume, each a combo box of six levels ("5" .. "1", Off = sysstring
// 228). Without an audio subsystem both boxes are disabled. Apply sets the volumes and saves
// them (Option.ini [Audio] SoundVolume / MusicVolume); music turned back on from Off restarts.

#include "nwindow/NCWnd.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCComboBox;

// NWM_COMMAND ids of the boxes
enum
{
    NID_AUDIO_SOUND = 0x1F12,
    NID_AUDIO_MUSIC = 0x1F16,
};

class NCAudioOptionWnd : public NCWnd
{
    NW_DECLARE_CLASS(NCAudioOptionWnd, NCWnd, 0, 0x101C8F60)

    NCComboBox* SoundCombo;                // +0x98 level 0 (1.0) .. 5 (Off)
    NCComboBox* MusicCombo;                // +0x9C
    NCWnd* Unused9C[3];                    // +0xA0 destroyed, never created

    NCAudioOptionWnd();                                              // 0x100a2ae0
    // ~NCAudioOptionWnd                                             // 0x100a2b40

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x10043ef0 (folded)
    virtual INT OnCreate();                                          // 36 0x100a2b90
    virtual INT OnDestroy();                                         // 37 0x100a2e50
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x100a2ce0

    // names ours
    void ClearItems();                                               // 0x100a0310 (C2 only; before InitVolumes)
    void InitVolumes();                                              // 0x100a05d0
    void SelectVolumes();                                            // 0x100a0330 current levels
    void Apply();                                                    // 0x100a2fa0
};
NW_ASSERT_OFFSET(NCAudioOptionWnd, SoundCombo, 0x98);
NW_ASSERT_OFFSET(NCAudioOptionWnd, Unused9C, 0xA0);
NW_ASSERT_SIZE(NCAudioOptionWnd, 0xAC);

#pragma pack(pop)