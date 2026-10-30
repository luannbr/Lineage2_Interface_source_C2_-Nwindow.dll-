/*=============================================================================
    NCVideoOptionWnd.cpp - the video page of the options. Stock: 0x1007d7e0 .. 0x1007d9f7,
    0x1007df40 .. 0x10080208, 0x10082140 .. 0x10082437, 0x100825b0 .. 0x10082f09 (slot 63 is
    a shared "return 1").
=============================================================================*/
#include "nwindow/NCVideoOptionWnd.h"
#include "nwindow/NCComboBox.h"
#include "nwindow/NCCheckBox.h"
#include "nwindow/NCGDevice.h"
#include "nwindow/NConsoleWnd.h"

#if NW_FEATURES
// phase 6: WindowFix (the "Full Screen" box, docs/notas/F6_WindowFix.md)
#include "features/WindowFix.h"
#endif

// 0x100a0840 - the controls are left as allocated, but for these
NCVideoOptionWnd::NCVideoOptionWnd()
{
    UnusedD0[0] = NULL;
    UnusedD0[1] = NULL;
    UnusedD0[2] = NULL;
}
NW_HOOK_CTOR(0x100a0840, NCVideoOptionWnd)

// 0x100a08a0
NW_HOOK_DTOR(0x100a08a0, NCVideoOptionWnd)

// 0x100a0100 (vslot 25)
INT NCVideoOptionWnd::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (NCWnd::WindowProc(Msg, wParam, lParam))
        return 1;
    if (Msg == NWM_COMMAND)
    {
        if (wParam == NID_VIDEO_RESOLUTION)
            return OnResolutionChanged(lParam) != 0;
        if (wParam == NID_VIDEO_REFRESH)
            return OnRefreshChanged(lParam) != 0;
        if (wParam == (WPARAM)NID_VIDEO_LOWERDETAIL)
            return OnLowerDetailClicked() != 0;
    }
    return 0;
}
NW_HOOK_METHOD(0x100a0100, NCVideoOptionWnd, WindowProc)

#if NW_FEATURES
// phase 6: the page with a "Full Screen" row on top (its box is created first, at y 2). The stock
// rows, which OnCreate and OnPaint still give at their stock y, go through RowY: there is no room
// for one more row (the page ends at 270, the clip at 280, the buttons start at 284), so every
// stock row moves down 15 px and the check boxes, drawn 12 high, pack 15 px apart instead of 18
// where a box is followed by another row. The last box then ends at 269.
static bool GModeRow = false;   // set by OnCreate: the page has the row (the fullscreen switch is in)

static INT RowY(INT StockY)
{
    static const INT Rows[][2] = {
        {3, 17},    // Resolution
        {21, 35},   // Refresh Rate
        {39, 53},   // Gamma
        {57, 71},   // Texture
        {75, 89},   // Modeling
        {93, 107},  // Animation
        {111, 125}, // Shadows (box)
        {129, 140}, // Ground Detail (box)
        {147, 155}, // Terrain Range
        {165, 173}, // PC/NPC Range
        {183, 191}, // PC/NPC Limit
        {201, 209}, // Reflections
        {219, 227}, // Graphic Cursor (box)
        {237, 242}, // Texture Filter (box)
        {255, 257}, // Lower Detail (box)
    };
    if (GModeRow)
        for (INT i = 0; i < (INT)(sizeof(Rows) / sizeof(Rows[0])); i++)
            if (Rows[i][0] == StockY)
                return Rows[i][1];
    return StockY;
}

// the rectangle of a check box: the stock 18 high, 14 where the rows are packed
static INT BoxRectHeight()
{
    return GModeRow ? 14 : 18;
}
#endif

// one combo box of the page (repeated inline in the stock OnCreate)
static FORCEINLINE NCComboBox* CreateOptionCombo(NCWnd* Parent, INT ID, INT Y, INT Flags = NWS_VISIBLE)
{
    NCComboBox* Combo = new NCComboBox;
    Combo->ID = ID;
#if NW_FEATURES
    Combo->SetWindowRect(100, RowY(Y), 112, 18);
#else
    Combo->SetWindowRect(100, Y, 112, 18);
#endif
    Parent->CreateChild(Combo, Flags);
    return Combo;
}

// one check box, unchecked, labelled "Use" (sysstring 443; repeated inline in the stock
// OnCreate)
static FORCEINLINE NCCheckBox* CreateOptionBox(NCWnd* Parent, INT ID, INT Y)
{
    NCCheckBox* Box = new NCCheckBox(*NCObject::SysString(443), 0, 0);
    Box->ID = ID;
#if NW_FEATURES
    Box->SetWindowRect(100, RowY(Y), 112, BoxRectHeight());
#else
    Box->SetWindowRect(100, Y, 112, 18);
#endif
    Parent->CreateChild(Box, NWS_VISIBLE);
    return Box;
}

// 0x100a5300 (vslot 36) - one control per line, their labels drawn by OnPaint
INT NCVideoOptionWnd::OnCreate()
{
    NCWnd::OnCreate();
#if NW_FEATURES
    // phase 6: the "Full Screen" row, first on the page; only when the fullscreen switch is in
    GModeRow = nwf::WindowFix_FullscreenSwitchAvailable();
    if (GModeRow)
    {
        NCCheckBox* ModeBox = new NCCheckBox(*SysString(443), 0, 0);
        ModeBox->ID = NID_VIDEO_FULLSCREEN;
        ModeBox->SetWindowRect(100, 2, 112, BoxRectHeight());
        CreateChild(ModeBox, NWS_VISIBLE);
        UnusedD0[0] = ModeBox;
    }
#endif
    ResolutionCombo = CreateOptionCombo(this, NID_VIDEO_RESOLUTION, 3, NWS_VISIBLE | 0x10);   // 0x10: meaning unknown
    RefreshCombo = CreateOptionCombo(this, NID_VIDEO_REFRESH, 21);
    GammaCombo = CreateOptionCombo(this, NID_VIDEO_GAMMA, 39);
    TextureCombo = CreateOptionCombo(this, NID_VIDEO_TEXTURE, 57);
    ModelingCombo = CreateOptionCombo(this, NID_VIDEO_MODELING, 75);
    AnimationCombo = CreateOptionCombo(this, NID_VIDEO_ANIMATION, 93);
    ShadowBox = CreateOptionBox(this, NID_VIDEO_SHADOW, 111);
    GroundDetailBox = CreateOptionBox(this, NID_VIDEO_GROUNDDETAIL, 129);
    TerrainRangeCombo = CreateOptionCombo(this, NID_VIDEO_TERRAINRANGE, 147);
    PawnRangeCombo = CreateOptionCombo(this, NID_VIDEO_PAWNRANGE, 165);
    ActorLimitCombo = CreateOptionCombo(this, NID_VIDEO_ACTORLIMIT, 183);
    ReflectionCombo = CreateOptionCombo(this, NID_VIDEO_REFLECTION, 201);
    CursorBox = CreateOptionBox(this, NID_VIDEO_CURSOR, 219);
    TextureFilterBox = CreateOptionBox(this, NID_VIDEO_TEXTUREFILTER, 237);
    LowerDetailBox = CreateOptionBox(this, NID_VIDEO_LOWERDETAIL, 255);
    InitItems();
    return 1;
}
NW_HOOK_METHOD(0x100a5300, NCVideoOptionWnd, OnCreate)

// a child window goes (repeated inline in the stock OnDestroy)
template <class T> static FORCEINLINE void DestroyChild(NCWnd* Parent, T*& Child)
{
    if (Child)
    {
        Parent->RemoveChildWindow(Child->Entry);
        delete Child;
        Child = NULL;
    }
}

// 0x100a1030 (vslot 37) - the combo boxes are enabled again first (no NULL check)
INT NCVideoOptionWnd::OnDestroy()
{
    guard(NCVideoOptionWnd::OnDestroy);
    DestroyChild(this, ResolutionCombo);
    DestroyChild(this, RefreshCombo);
    DestroyChild(this, GammaCombo);
    DestroyChild(this, TextureCombo);
    DestroyChild(this, ModelingCombo);
    DestroyChild(this, AnimationCombo);
    DestroyChild(this, ShadowBox);
    DestroyChild(this, GroundDetailBox);
    DestroyChild(this, TerrainRangeCombo);
    DestroyChild(this, PawnRangeCombo);
    DestroyChild(this, ActorLimitCombo);
    DestroyChild(this, CursorBox);
    DestroyChild(this, ReflectionCombo);
    DestroyChild(this, LowerDetailBox);
    DestroyChild(this, TextureFilterBox);
    DestroyChild(this, UnusedD0[0]);
    DestroyChild(this, UnusedD0[1]);
    DestroyChild(this, UnusedD0[2]);
    NCWnd::OnDestroy();
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x100a1030, NCVideoOptionWnd, OnDestroy)

// a label right-aligned on x 87 (repeated inline in the stock OnPaint); phase 6: a label follows its
// control through RowY
static FORCEINLINE void DrawLabel(NCGDevice* Device, INT TextID, INT Y)
{
    SIZE Size;
    NCGDevice::GetTextExtent(&Size, *NCObject::SysString(TextID), 0, 0, 0, 0, 0);
#if NW_FEATURES
    Device->DrawText(87 - Size.cx, RowY(Y), 0xFFFFFFFF, *NCObject::SysString(TextID), 0, 0, 0,
                     L2FONT_Normal, 0, 0, 0, 0, 0, 0);
#else
    Device->DrawText(87 - Size.cx, Y, 0xFFFFFFFF, *NCObject::SysString(TextID), 0, 0, 0,
                     L2FONT_Normal, 0, 0, 0, 0, 0, 0);
#endif
}

// the same, when the text pointer is not NULL (it never is)
static FORCEINLINE void DrawOptionLabel(NCGDevice* Device, INT TextID, INT Y)
{
    if (*NCObject::SysString(TextID))
        DrawLabel(Device, TextID, Y);
}

#if NW_FEATURES
// the same, for a text of ours (phase 6: no sysstring line says "Full Screen")
static void DrawTextLabel(NCGDevice* Device, const TCHAR* Text, INT Y)
{
    SIZE Size;
    NCGDevice::GetTextExtent(&Size, Text, 0, 0, 0, 0, 0);
    Device->DrawText(87 - Size.cx, Y, 0xFFFFFFFF, Text, 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
}
#endif

// 0x100a08f0 (vslot 55) - the clip reaches 10 pixels below the page. The animation line is a
// Korean literal ("Animation"), tested with the Texture line; Texture Filter is not tested.
INT NCVideoOptionWnd::OnPaint(NCGDevice* Device)
{
    guard(NCVideoOptionWnd::OnPaint);
    Device->SetClip(ScreenX, ScreenY, Width, Height + 10);
    Device->BeginPaint(ScreenX, ScreenY);
    DrawOptionLabel(Device, 209, 3);     // Resolution
    DrawOptionLabel(Device, 351, 21);    // Refresh Rate
    DrawOptionLabel(Device, 210, 39);    // Gamma
    DrawOptionLabel(Device, 211, 57);    // Texture
    DrawOptionLabel(Device, 594, 75);    // Modeling Detail
    DrawOptionLabel(Device, 607, 93);    // Animation (C2: a sysstring line; C1 had a Korean literal)
    DrawOptionLabel(Device, 212, 111);   // Shadows
    DrawOptionLabel(Device, 213, 129);   // Ground Detail
    DrawOptionLabel(Device, 214, 147);   // Terrain Range
    DrawOptionLabel(Device, 215, 165);   // PC/NPC Range
    DrawOptionLabel(Device, 540, 183);   // PC/NPC Limit
    DrawOptionLabel(Device, 352, 201);   // Reflections
    DrawOptionLabel(Device, 305, 219);   // Graphic Cursor
    DrawLabel(Device, 464, 237);         // Texture Filter
    DrawOptionLabel(Device, 370, 255);   // Lower Detail
#if NW_FEATURES
    if (UnusedD0[0])
        DrawTextLabel(Device, TEXT("Full Screen"), 3);   // phase 6: the row on top
#endif
    Device->EndPaint();
    Device->ReleaseClip();
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x100a08f0, NCVideoOptionWnd, OnPaint)

// 0x100a1480 (vslot 67) - the refresh box lists the new mode's rates and keeps the rate at the
// selected index of the new list (or the current rate), else selects the last one
INT NCVideoOptionWnd::OnResolutionChanged(INT Index)
{
    INT Widths[100], Heights[100], Bits[100], Rates[100];
    TCHAR Text[256];
    INT Rate = GRefreshRate;
    INT ModeCount = GConsole->Engine->GRenDev->GetRes(100, Widths, Heights, Bits);
    if (Index >= 0 && Index < ModeCount)
    {
        INT RateIndex = RefreshCombo->Selected;
        INT RateCount = GConsole->Engine->GRenDev->GetRefreshRate(100, Widths[Index],
                                                                  Heights[Index], Bits[Index],
                                                                  Rates);
        if (RateIndex >= 0 && RateIndex < RateCount)
            Rate = Rates[RateIndex];
        RefreshCombo->ClearItems();
        for (INT i = 0; i < RateCount; i++)
        {
            appSprintf(Text, TEXT("%dHz"), Rates[i]);
            RefreshCombo->AddItem(Text);
        }
        INT j;
        for (j = 0; j < RateCount; j++)
            if (Rates[j] == Rate)
            {
                RefreshCombo->SetSelected(j);
                break;
            }
        if (j == RateCount)
            RefreshCombo->SetSelected(RateCount - 1);
    }
    return 1;
}
NW_HOOK_METHOD(0x100a1480, NCVideoOptionWnd, OnResolutionChanged)

// "Lower Detail" fixes the detail controls; free again, reflections still need pixel shader
// 1.2 (repeated inline in LoadSettings and OnLowerDetailClicked)
static FORCEINLINE void EnableDetails(NCVideoOptionWnd* Wnd, UBOOL bEnable)
{
    Wnd->TextureCombo->EnableWindow(bEnable);
    Wnd->ModelingCombo->EnableWindow(bEnable);
    Wnd->AnimationCombo->EnableWindow(bEnable);
    Wnd->ShadowBox->bDisabled = !bEnable;
    Wnd->GroundDetailBox->bDisabled = !bEnable;
    Wnd->TerrainRangeCombo->EnableWindow(bEnable);
    Wnd->PawnRangeCombo->EnableWindow(bEnable);
    if (bEnable && GConsole->Engine->GRenDev->GetRenderCaps()->PixelShaderVersion >= 12)
        Wnd->ReflectionCombo->EnableWindow(1);
    else
    {
        Wnd->ReflectionCombo->EnableWindow(0);
        Wnd->ReflectionCombo->SetSelected(0);
    }
}

// 0x100a2990 (vslot 69)
INT NCVideoOptionWnd::OnLowerDetailClicked()
{
    if (LowerDetailBox->bChecked)
        EnableDetails(this, 0);
    else
        EnableDetails(this, 1);
    return 1;
}
NW_HOOK_METHOD(0x100a2990, NCVideoOptionWnd, OnLowerDetailClicked)

// 0x100a01a0 - both lists again, the current mode and rate selected (after Apply fell back
// to the default mode)
void NCVideoOptionWnd::InitModes()
{
    INT Widths[100], Heights[100], Bits[100], Rates[100];
    TCHAR Text[100];
    ResolutionCombo->ClearItems();
    RefreshCombo->ClearItems();
    INT Count = GConsole->Engine->GRenDev->GetRes(100, Widths, Heights, Bits);
    for (INT i = 0; i < Count; i++)
    {
        appSprintf(Text, TEXT("%d*%d %dbit"), Widths[i], Heights[i], Bits[i]);
        ResolutionCombo->AddItem(Text);
        if (GSEKScreenX == Widths[i] && GSEKScreenY == Heights[i] && GColorBits == Bits[i])
            ResolutionCombo->SetSelected(i);
    }
    Count = GConsole->Engine->GRenDev->GetRefreshRate(100, GSEKScreenX, GSEKScreenY, GColorBits,
                                                      Rates);
    for (INT j = 0; j < Count; j++)
    {
        appSprintf(Text, TEXT("%dHz"), Rates[j]);
        RefreshCombo->AddItem(Text);
        if (GRefreshRate == Rates[j])
            RefreshCombo->SetSelected(j);
    }
}
NW_HOOK_METHOD(0x100a01a0, NCVideoOptionWnd, InitModes)

// a check box follows its setting (repeated inline in the stock code; C1 left a disabled box alone)
static FORCEINLINE void SetBox(NCCheckBox* Box, UBOOL bValue)
{
    if (bValue)
        Box->bChecked = 1;
    else
        Box->bChecked = 0;
}

// 0x100a15d0 - the boxes show the current settings; the ranges and the gamma are placed
// between the steps Apply uses
void NCVideoOptionWnd::LoadSettings()
{
    INT Widths[100], Heights[100], Bits[100], Rates[100];
    INT Count = GConsole->Engine->GRenDev->GetRes(100, Widths, Heights, Bits);
    for (INT i = 0; i < Count; i++)
        if (GSEKScreenX == Widths[i] && GSEKScreenY == Heights[i] && GColorBits == Bits[i])
            ResolutionCombo->SetSelected(i);
    Count = GConsole->Engine->GRenDev->GetRefreshRate(100, GSEKScreenX, GSEKScreenY, GColorBits,
                                                      Rates);
    for (INT j = 0; j < Count; j++)
        if (GRefreshRate == Rates[j])
            RefreshCombo->SetSelected(j);

    UClient* Client = GConsole->Engine->Client;
    if (Client)
    {
        FLOAT Gamma = Client->Gamma;
        if (Gamma >= 1.2f)
            GammaCombo->SetSelected(0);
        else if (Gamma >= 1.0f && Gamma < 1.2f)
            GammaCombo->SetSelected(1);
        else if (Gamma >= 0.8f && Gamma < 1.0f)
            GammaCombo->SetSelected(2);
        else if (Gamma >= 0.6f && Gamma < 0.8f)
            GammaCombo->SetSelected(3);
        else if (Gamma < 0.6f)
            GammaCombo->SetSelected(4);
    }

    if (GL2OriginPawnCR < 1.5f)
        PawnRangeCombo->SetSelected(4);
    else if (GL2OriginPawnCR >= 1.5f && GL2OriginPawnCR < 2.0f)
        PawnRangeCombo->SetSelected(3);
    else if (GL2OriginPawnCR >= 2.0f && GL2OriginPawnCR < 2.5f)
        PawnRangeCombo->SetSelected(2);
    else if (GL2OriginPawnCR >= 2.5f && GL2OriginPawnCR < 3.0f)
        PawnRangeCombo->SetSelected(1);
    else if (GL2OriginPawnCR >= 3.0f)
        PawnRangeCombo->SetSelected(0);

    if (GL2OriginTerrainCR < 5.0f)
        TerrainRangeCombo->SetSelected(4);
    else if (GL2OriginTerrainCR >= 5.0f && GL2OriginTerrainCR < 6.0f)
        TerrainRangeCombo->SetSelected(3);
    else if (GL2OriginTerrainCR >= 6.0f && GL2OriginTerrainCR < 7.0f)
        TerrainRangeCombo->SetSelected(2);
    else if (GL2OriginTerrainCR >= 7.0f && GL2OriginTerrainCR < 8.0f)
        TerrainRangeCombo->SetSelected(1);
    else if (GL2OriginTerrainCR >= 8.0f)
        TerrainRangeCombo->SetSelected(0);

    SetBox(GroundDetailBox, GL2RenderDeco);
    SetBox(ShadowBox, GIsL2Projector);
    INT Texture = GL2TextureDetail;
    if (Texture >= 2)
        Texture = 2;
    else if (Texture <= 0)
        Texture = 0;
    TextureCombo->SetSelected(Texture);
    ModelingCombo->SetSelected(GL2UseReducePawnLOD != 0);
    AnimationCombo->SetSelected(GL2UseSkipAnim != 0);
    SetBox(CursorBox, GL2UseColorCursor);
    if (GIsL2WaterEffect && GIsL2WaterEffectType == 1)
        ReflectionCombo->SetSelected(1);
    else if (GIsL2WaterEffect && GIsL2WaterEffectType == 2)
        ReflectionCombo->SetSelected(2);
    else
        ReflectionCombo->SetSelected(0);
    SetBox(TextureFilterBox, GL2UseTrilinear);
    if (GConsole->Engine->GRenDev->GetRenderCaps()->PixelShaderVersion < 12)
    {
        ReflectionCombo->EnableWindow(0);
        ReflectionCombo->SetSelected(0);
    }
    else
        ReflectionCombo->EnableWindow(1);

    if (GL2RenderActorLimitOpt >= 6)
        ActorLimitCombo->SetSelected(0);
    else if (GL2RenderActorLimitOpt == 5)
        ActorLimitCombo->SetSelected(1);
    else if (GL2RenderActorLimitOpt == 4)
        ActorLimitCombo->SetSelected(2);
    else if (GL2RenderActorLimitOpt == 3)
        ActorLimitCombo->SetSelected(3);
    else if (GL2RenderActorLimitOpt == 2)
        ActorLimitCombo->SetSelected(4);
    else if (GL2RenderActorLimitOpt <= 1)
        ActorLimitCombo->SetSelected(5);

    if (GL2KeepMinFrameRate)
    {
        if (!LowerDetailBox->bDisabled)
            LowerDetailBox->bChecked = 1;
        EnableDetails(this, 0);
    }
    else
    {
        if (!LowerDetailBox->bDisabled)
            LowerDetailBox->bChecked = 0;
        EnableDetails(this, 1);
    }
#if NW_FEATURES
    SyncModeBox();   // phase 6
#endif
}
NW_HOOK_METHOD(0x100a15d0, NCVideoOptionWnd, LoadSettings)

#if NW_FEATURES
// phase 6 - the "Full Screen" box follows the viewport, not the last click: Alt+Enter can have
// switched the mode since the page was built, and OK would otherwise switch it back
void NCVideoOptionWnd::SyncModeBox()
{
    NCCheckBox* Box = static_cast<NCCheckBox*>(UnusedD0[0]);
    if (Box)
        Box->bChecked = nwf::WindowFix_IsFullscreen(Box->bChecked != 0);
}
#endif

// a setting saved as its number (repeated inline in the stock Apply)
static FORCEINLINE void SaveInt(const TCHAR* Section, const TCHAR* Key, INT Value)
{
    GConfig->SetString(Section, Key, appItoa(Value), TEXT("Option.ini"));
}

// a setting saved "%f" through the caller's buffer (repeated inline in the stock Apply)
static FORCEINLINE void SaveFloat(TCHAR* Text, const TCHAR* Section, const TCHAR* Key, FLOAT Value)
{
    appSprintf(Text, TEXT("%f"), Value);
    GConfig->SetString(Section, Key, Text, TEXT("Option.ini"));
}

// the fog of a terrain range step (env.int [FOG]), with its defaults (repeated inline in the
// stock Apply)
static FORCEINLINE void LoadFogRange(const TCHAR* StartKey, const TCHAR* EndKey, FLOAT DefaultStart,
                                     FLOAT DefaultEnd, FLOAT& Start, FLOAT& End)
{
    if (!GConfig->GetFloat(TEXT("FOG"), StartKey, Start, TEXT("env.int")))
        Start = DefaultStart;
    if (!GConfig->GetFloat(TEXT("FOG"), EndKey, End, TEXT("env.int")))
        End = DefaultEnd;
}

// 0x100a1b90 - the settings into the globals, the render device and Option.ini. A mode the device refuses falls
// back to 1024x768 32-bit 60 Hz (tried once, then the lists are rebuilt). "Lower Detail" forces the low textures and
// is saved twice. C2: the fullscreen flag comes from the viewport (not from l2.ini), the render device's SetRes is the
// 7-argument overload (slot 27), it leaves GSEKScreenX / Y to the engine, and the windows are no longer put back in
// their default places after a mode change.
void NCVideoOptionWnd::Apply()
{
    TCHAR Text[256];
    UBOOL bFullscreen = GConsole->Engine->Client->Viewports(0)->IsFullscreen();

    INT OldTextureDetail = GL2TextureDetail;
    if (GL2KeepMinFrameRate)
        OldTextureDetail = 2;
    if (!LowerDetailBox->bChecked)
    {
        GConfig->SetString(TEXT("Video"), TEXT("IsKeepMinFrameRate"), TEXT("false"), TEXT("Option.ini"));
        GL2KeepMinFrameRate = 0;
    }
    else
    {
        GConfig->SetString(TEXT("Video"), TEXT("IsKeepMinFrameRate"), TEXT("true"), TEXT("Option.ini"));
        GL2KeepMinFrameRate = 1;
    }

    UBOOL bOldTrilinear = GConsole->Engine->GRenDev->IsUseTrilinear();
    if (!TextureFilterBox->bChecked)
    {
        GL2UseTrilinear = 0;
        GConfig->SetString(TEXT("Video"), TEXT("UseTrilinear"), TEXT("false"), TEXT("Option.ini"));
    }
    else
    {
        GL2UseTrilinear = 1;
        GConfig->SetString(TEXT("Video"), TEXT("UseTrilinear"), TEXT("true"), TEXT("Option.ini"));
    }
    if (bOldTrilinear != GL2UseTrilinear)
        GConsole->Engine->GRenDev->SetUseTrilinear(GL2UseTrilinear);

    INT Texture = TextureCombo->Selected;
    if (Texture == 0)
        GL2TextureDetail = 0;
    else if (Texture == 1)
        GL2TextureDetail = 1;
    else
        GL2TextureDetail = 2;
    SaveInt(TEXT("Video"), TEXT("TextureDetail"), GL2TextureDetail);
    INT NewTextureDetail = GL2TextureDetail;
    if (GL2KeepMinFrameRate)
        NewTextureDetail = 2;
    if (OldTextureDetail != NewTextureDetail)
        GConsole->Engine->GRenDev->DeleteAllTexture();

    GL2UseReducePawnLOD = ModelingCombo->Selected != 0;
    SaveInt(TEXT("Video"), TEXT("ModelDetail"), GL2UseReducePawnLOD);
    GL2UseSkipAnim = AnimationCombo->Selected != 0;
    SaveInt(TEXT("Video"), TEXT("SkipAnim"), GL2UseSkipAnim);

    INT Widths[100], Heights[100], Bits[100], Rates[100];
    INT Mode = ResolutionCombo->Selected;
    INT ModeCount = GConsole->Engine->GRenDev->GetRes(100, Widths, Heights, Bits);
    if (Mode >= 0 && Mode < ModeCount)
    {
        INT RateIndex = RefreshCombo->Selected;
        INT NewX = Widths[Mode];
        INT NewY = Heights[Mode];
        INT NewBits = Bits[Mode];
        INT RateCount = GConsole->Engine->GRenDev->GetRefreshRate(100, NewX, NewY, NewBits, Rates);
        if (RateIndex >= 0 && RateIndex < RateCount)
        {
            INT Rate = Rates[RateIndex];
            if (GSEKScreenX != NewX || GSEKScreenY != NewY || GColorBits != NewBits ||
                GRefreshRate != Rate)
            {
                if (GConsole->Engine->GRenDev->SetRes(GConsole->Engine->Client->Viewports(0), NewX, NewY,
                                                      NewBits / 8, Rate, bFullscreen, 1))
                {
                    GColorBits = NewBits;
                    GRefreshRate = Rate;
                }
                else if (GConsole->Engine->GRenDev->SetRes(GConsole->Engine->Client->Viewports(0), 1024, 768,
                                                           4, 60, bFullscreen, 1))
                {
                    GColorBits = 32;
                    GRefreshRate = 60;
                    InitModes();
                }
            }
            SaveInt(TEXT("Video"), TEXT("GamePlayViewportX"), GSEKScreenX);
            SaveInt(TEXT("Video"), TEXT("GamePlayViewportY"), GSEKScreenY);
            SaveInt(TEXT("Video"), TEXT("ColorBits"), GColorBits);
            SaveInt(TEXT("Video"), TEXT("RefreshRate"), GRefreshRate);
        }
    }

#if NW_FEATURES
    // phase 6: the "Full Screen" box, after the resolution so the switch finds a mode of the list
    // (the hook refuses a fullscreen size the monitor does not have); the box then shows the mode
    // the viewport is really in
    if (NCCheckBox* ModeBox = static_cast<NCCheckBox*>(UnusedD0[0]))
    {
        if (ModeBox->bChecked != 0 && !nwf::WindowFix_IsFullscreen(true))
            nwf::WindowFix_SetFullscreen(true);
        else if (ModeBox->bChecked == 0 && nwf::WindowFix_IsFullscreen(false))
            nwf::WindowFix_SetFullscreen(false);
        SyncModeBox();
    }
#endif

    UClient* Client = GConsole->Engine->Client;
    if (Client)
    {
        FLOAT Gamma = Client->Gamma;
        switch (GammaCombo->Selected)
        {
        case 0: Gamma = 1.2f; break;
        case 1: Gamma = 1.0f; break;
        case 2: Gamma = 0.8f; break;
        case 3: Gamma = 0.6f; break;
        case 4: Gamma = 0.4f; break;
        }
        if (Gamma != Client->Gamma)
        {
            Client->Gamma = Gamma;
            SaveFloat(Text, TEXT("Video"), TEXT("Gamma"), Gamma);
            GConsole->Engine->UpdateGamma();
        }
    }

    switch (PawnRangeCombo->Selected)
    {
    case 0: GL2OriginPawnCR = 3.0f; break;
    case 1: GL2OriginPawnCR = 2.5f; break;
    case 2: GL2OriginPawnCR = 2.0f; break;
    case 3: GL2OriginPawnCR = 1.5f; break;
    case 4: GL2OriginPawnCR = 1.0f; break;
    }
    SaveFloat(Text, TEXT("ClippingRange"), TEXT("Pawn"), GL2OriginPawnCR);

    switch (TerrainRangeCombo->Selected)
    {
    case 0:
        GL2OriginTerrainCR = 8.0f;
        GL2OriginStaticMeshCR = 4.0f;
        GL2OriginStaticMeshLodCR = 6.0f;
        GL2ActorCR = 6.0f;
        break;
    case 1:
        GL2OriginTerrainCR = 7.0f;
        GL2OriginStaticMeshCR = 3.5f;
        GL2OriginStaticMeshLodCR = 5.5f;
        GL2ActorCR = 5.5f;
        break;
    case 2:
        GL2OriginTerrainCR = 6.0f;
        GL2OriginStaticMeshCR = 3.0f;
        GL2OriginStaticMeshLodCR = 5.0f;
        GL2ActorCR = 5.0f;
        break;
    case 3:
        GL2OriginTerrainCR = 5.0f;
        GL2OriginStaticMeshCR = 2.5f;
        GL2OriginStaticMeshLodCR = 4.5f;
        GL2ActorCR = 4.5f;
        break;
    case 4:
        GL2OriginTerrainCR = 4.0f;
        GL2OriginStaticMeshCR = 2.0f;
        GL2OriginStaticMeshLodCR = 4.0f;
        GL2ActorCR = 4.0f;
        break;
    }
    SaveFloat(Text, TEXT("ClippingRange"), TEXT("Terrain"), GL2OriginTerrainCR);
    SaveFloat(Text, TEXT("ClippingRange"), TEXT("Actor"), GL2ActorCR);
    SaveFloat(Text, TEXT("ClippingRange"), TEXT("StaticMesh"), GL2OriginStaticMeshCR);
    SaveFloat(Text, TEXT("ClippingRange"), TEXT("StaticMeshLod"), GL2OriginStaticMeshLodCR);

    if (!GroundDetailBox->bChecked)
    {
        GL2RenderDeco = 0;
        GConfig->SetString(TEXT("Video"), TEXT("RenderDeco"), TEXT("false"), TEXT("Option.ini"));
    }
    else
    {
        GL2RenderDeco = 1;
        GConfig->SetString(TEXT("Video"), TEXT("RenderDeco"), TEXT("true"), TEXT("Option.ini"));
    }
    if (!ShadowBox->bChecked)
    {
        GIsL2Projector = 0;
        GConfig->SetString(TEXT("Video"), TEXT("PawnShadow"), TEXT("false"), TEXT("Option.ini"));
    }
    else
    {
        GIsL2Projector = 1;
        GConfig->SetString(TEXT("Video"), TEXT("PawnShadow"), TEXT("true"), TEXT("Option.ini"));
    }
    if (!CursorBox->bChecked)
    {
        GL2UseColorCursor = 0;
        GConfig->SetString(TEXT("Video"), TEXT("UseColorCursor"), TEXT("false"), TEXT("Option.ini"));
        SetCursor(0, NULL);
        LockCursor();
    }
    else
    {
        GL2UseColorCursor = 1;
        GConfig->SetString(TEXT("Video"), TEXT("UseColorCursor"), TEXT("true"), TEXT("Option.ini"));
        UnlockCursor();
        SetCursor(0, NULL);
    }

    INT Reflection = ReflectionCombo->Selected;
    if (!Reflection)
    {
        GIsL2WaterEffect = 0;
        GConfig->SetString(TEXT("L2WaterEffect"), TEXT("IsUseEffect"), TEXT("false"),
                           TEXT("Option.ini"));
    }
    else
    {
        GIsL2WaterEffect = 1;
        GConfig->SetString(TEXT("L2WaterEffect"), TEXT("IsUseEffect"), TEXT("true"),
                           TEXT("Option.ini"));
    }
    GIsL2WaterEffectType = Reflection;
    SaveInt(TEXT("L2WaterEffect"), TEXT("EffectType"), Reflection);

    if (!LowerDetailBox->bChecked)
    {
        GConfig->SetString(TEXT("Video"), TEXT("IsKeepMinFrameRate"), TEXT("false"), TEXT("Option.ini"));
        GL2KeepMinFrameRate = 0;
    }
    else
    {
        GConfig->SetString(TEXT("Video"), TEXT("IsKeepMinFrameRate"), TEXT("true"), TEXT("Option.ini"));
        GL2KeepMinFrameRate = 1;
    }

    // the fog follows the terrain range; "Lower Detail" takes the nearest
    FLOAT FogStart = 1.5f;
    FLOAT FogEnd = 5.5f;
    if (GL2KeepMinFrameRate || GL2OriginTerrainCR < 5.0f)
        LoadFogRange(TEXT("StartRange1"), TEXT("EndRange1"), 0.5f, 3.5f, FogStart, FogEnd);
    else if (GL2OriginTerrainCR >= 5.0f && GL2OriginTerrainCR < 6.0f)
        LoadFogRange(TEXT("StartRange2"), TEXT("EndRange2"), 1.0f, 4.5f, FogStart, FogEnd);
    else if (GL2OriginTerrainCR >= 6.0f && GL2OriginTerrainCR < 7.0f)
        LoadFogRange(TEXT("StartRange3"), TEXT("EndRange3"), 1.5f, 5.5f, FogStart, FogEnd);
    else if (GL2OriginTerrainCR >= 7.0f && GL2OriginTerrainCR < 8.0f)
        LoadFogRange(TEXT("StartRange4"), TEXT("EndRange4"), 2.0f, 6.5f, FogStart, FogEnd);
    else if (GL2OriginTerrainCR >= 8.0f)
        LoadFogRange(TEXT("StartRange5"), TEXT("EndRange5"), 2.5f, 7.5f, FogStart, FogEnd);
    GL2FogStart = FogStart;
    GL2FogEnd = FogEnd;

    switch (ActorLimitCombo->Selected)
    {
    case 0: GL2RenderActorLimitOpt = 6; GL2RenderActorLimit = 10000; break;
    case 1: GL2RenderActorLimitOpt = 5; GL2RenderActorLimit = 400; break;
    case 2: GL2RenderActorLimitOpt = 4; GL2RenderActorLimit = 300; break;
    case 3: GL2RenderActorLimitOpt = 3; GL2RenderActorLimit = 200; break;
    case 4: GL2RenderActorLimitOpt = 2; GL2RenderActorLimit = 100; break;
    case 5: GL2RenderActorLimitOpt = 1; GL2RenderActorLimit = 50; break;
    }
    appSprintf(Text, TEXT("%d"), GL2RenderActorLimitOpt);
    GConfig->SetString(TEXT("Video"), TEXT("RenderActorLimited"), Text, TEXT("Option.ini"));
}
NW_HOOK_METHOD(0x100a1b90, NCVideoOptionWnd, Apply)

// 0x100a13c0 (name ours) - before the page is filled again (NCOptionWnd::ShowWindow): the lists are emptied and the
// check boxes cleared, in the order of the controls
void NCVideoOptionWnd::ClearItems()
{
    ResolutionCombo->ClearItems();
    RefreshCombo->ClearItems();
    GammaCombo->ClearItems();
    TextureCombo->ClearItems();
    ModelingCombo->ClearItems();
    AnimationCombo->ClearItems();
    ShadowBox->bChecked = 0;
    GroundDetailBox->bChecked = 0;
    TerrainRangeCombo->ClearItems();
    PawnRangeCombo->ClearItems();
    ActorLimitCombo->ClearItems();
    CursorBox->bChecked = 0;
    ReflectionCombo->ClearItems();
    LowerDetailBox->bChecked = 0;
    TextureFilterBox->bChecked = 0;
}
NW_HOOK_METHOD(0x100a13c0, NCVideoOptionWnd, ClearItems)

// the gamma, range and limit boxes list "5" .. "1" (repeated inline in the stock InitItems)
static FORCEINLINE void AddLevels(NCComboBox* Combo)
{
    Combo->AddItem(TEXT("5"));
    Combo->AddItem(TEXT("4"));
    Combo->AddItem(TEXT("3"));
    Combo->AddItem(TEXT("2"));
    Combo->AddItem(TEXT("1"));
}

// 0x100a4db0 - fills the boxes (High / Medium / Low, None, All, Sky: sysstring 224 .. 226,
// 27, 353, 354), then shows the current settings
void NCVideoOptionWnd::InitItems()
{
    INT Widths[100], Heights[100], Bits[100], Rates[100];
    TCHAR Text[100];
    INT Count = GConsole->Engine->GRenDev->GetRes(100, Widths, Heights, Bits);
    for (INT i = 0; i < Count; i++)
    {
        appSprintf(Text, TEXT("%d*%d %dbit"), Widths[i], Heights[i], Bits[i]);
        ResolutionCombo->AddItem(Text);
    }
    Count = GConsole->Engine->GRenDev->GetRefreshRate(100, GSEKScreenX, GSEKScreenY, GColorBits,
                                                      Rates);
    for (INT j = 0; j < Count; j++)
    {
        appSprintf(Text, TEXT("%dHz"), Rates[j]);
        RefreshCombo->AddItem(Text);
    }
    AddLevels(GammaCombo);
    TextureCombo->AddItem(0xE0);
    TextureCombo->AddItem(0xE1);
    TextureCombo->AddItem(0xE2);
    ModelingCombo->AddItem(0xE0);
    ModelingCombo->AddItem(0xE2);
    AnimationCombo->AddItem(0xE0);
    AnimationCombo->AddItem(0xE2);
    AddLevels(TerrainRangeCombo);
    AddLevels(PawnRangeCombo);
    ActorLimitCombo->AddItem(0x1B);
    AddLevels(ActorLimitCombo);
    ReflectionCombo->AddItem(0x1B);
    ReflectionCombo->AddItem(0x161);
    ReflectionCombo->AddItem(0x162);
    LoadSettings();
}
NW_HOOK_METHOD(0x100a4db0, NCVideoOptionWnd, InitItems)

NW_CHECK_VTABLE(NCVideoOptionWnd, 0x1011e978, 70)
