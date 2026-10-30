/*=============================================================================
    NCPawnViewr_HairChangeWnd.cpp - the pawn viewer's hair page (C2: redesigned: a style suffix and a color, the "a" and
    "b" meshes of a hair number that exist, a face list). Stock: 0x100ab940 .. 0x100aba9a, 0x100adc50 .. 0x100af23e,
    0x100afea0 .. 0x100affa1.
=============================================================================*/
#include "nwindow/NCPawnViewrWnd.h"
#include "nwindow/NCButton.h"
#include "nwindow/NCCheckBox.h"
#include "nwindow/NCComboBox.h"
#include "nwindow/NCGDevice.h"
#include "nwindow/NConsoleWnd.h"

#include <string.h>

// NWM_COMMAND ids (names ours)
enum
{
    NID_HAIR_STYLE = 0x104,
    NID_HAIR_MESHA = 0x105,
    NID_HAIR_MESHB = 0x106,
    NID_HAIR_RESET = 0x108,
    NID_HAIR_FACE = 0x109,
    NID_HAIR_TOGETHER = 0x110,
    NID_HAIR_COLOR = 0x112,
};

// sysstring line of the button
enum { STR_Cancel = 141 };

// the mesh names' prefixes by the pawn's mesh type (APawn::MeshType); built on the stack from
// these literals by each stock user
static const TCHAR GHairMeshPrefixes[20][20] = {
    TEXT("MFighter"), TEXT("FFighter"), TEXT("MDarkElf"), TEXT("FDarkElf"), TEXT("MDwarf"),
    TEXT("FDwarf"),   TEXT("MElf"),     TEXT("FElf"),     TEXT("MMagic"),   TEXT("FMagic"),
    TEXT("MOrc"),     TEXT("FOrc"),     TEXT("MShaman"),  TEXT("FShaman")};

// the player's pawn: its hair meshes' prefix, and whether its hair can change
static FORCEINLINE const TCHAR* MeshPrefix(APawn* Pawn)
{
    return GHairMeshPrefixes[Pawn->MeshType];
}
static FORCEINLINE UBOOL HairLocked(APawn* Pawn) { return Pawn->Unknown678 & 4; }

// UObject::StaticFindObject, then StaticLoadObject when not loaded yet (inline each time)
static FORCEINLINE UObject* FindOrLoad(UClass* FindClass, UClass* LoadClass, const TCHAR* Name)
{
    UObject* Object = UObject::StaticFindObject(FindClass, ANY_PACKAGE, Name, 0);
    if (!Object)
        Object = UObject::StaticLoadObject(LoadClass, NULL, Name, NULL, 0, NULL);
    return Object;
}

// a child window goes (repeated inline in the stock OnDestroys)
template <class T> static FORCEINLINE void DestroyChild(NCWnd* Parent, T*& Child)
{
    if (Child)
    {
        Parent->RemoveChildWindow(Child->Entry);
        delete Child;
        Child = NULL;
    }
}

// 0x100adc50 - the combo boxes and the button are cleared (not the check box), the two textures emptied
NCPawnViewr_HairChangeWnd::NCPawnViewr_HairChangeWnd()
{
    HairStyleCombo = NULL;
    HairColorCombo = NULL;
    MeshACombo = NULL;
    MeshBCombo = NULL;
    FaceCombo = NULL;
    ResetButton = NULL;
    memset(MeshATexture, 0, sizeof(MeshATexture));
    memset(MeshBTexture, 0, sizeof(MeshBTexture));
}
NW_HOOK_CTOR(0x100adc50, NCPawnViewr_HairChangeWnd)

// 0x100add30
NW_HOOK_DTOR(0x100add30, NCPawnViewr_HairChangeWnd)

// 0x100ab940 (vslot 25) - C2 no longer refuses the messages of a disabled window here
INT NCPawnViewr_HairChangeWnd::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (NCWnd::WindowProc(Msg, wParam, lParam))
        return 1;
    if (Msg == NWM_COMMAND)
    {
        if (wParam == NID_HAIR_STYLE)
            return OnStyleCombo((INT)lParam) != 0;
        if (wParam == NID_HAIR_COLOR)
            return OnColorCombo((INT)lParam) != 0;
        if (wParam == NID_HAIR_MESHA)
            return OnMeshA((INT)lParam) != 0;
        if (wParam == NID_HAIR_MESHB)
            return OnMeshB((INT)lParam) != 0;
        if (wParam == NID_HAIR_FACE)
            return OnFace((INT)lParam) != 0;
        if (wParam == NID_HAIR_RESET)
            return OnReset() != 0;
    }
    return 0;
}
NW_HOOK_METHOD(0x100ab940, NCPawnViewr_HairChangeWnd, WindowProc)

// 0x100add80 (vslot 36) - the button, the style (0..4) and color (0..9) combos, the two mesh combos, the "together"
// check box and the face combo
INT NCPawnViewr_HairChangeWnd::OnCreate()
{
    TCHAR Text[64];
    NCWnd::OnCreate();
    ResetButton = new NCButton(TEXT("L2UI.Control.DefaultButton"),
                               TEXT("L2UI.Control.DefaultButton_click"), 0);
    ResetButton->ID = NID_HAIR_RESET;
    ResetButton->SetWindowRect(95, 200, 35, 16);
    CreateChild(ResetButton, NWS_VISIBLE);
    ResetButton->SetTextID(STR_Cancel);

    HairStyleCombo = new NCComboBox;
    HairStyleCombo->ID = NID_HAIR_STYLE;
    HairStyleCombo->SetWindowRect(55, 30, 35, 16);
    CreateChild(HairStyleCombo, NWS_VISIBLE);
    HairColorCombo = new NCComboBox;
    HairColorCombo->ID = NID_HAIR_COLOR;
    HairColorCombo->SetWindowRect(155, 30, 35, 16);
    CreateChild(HairColorCombo, NWS_VISIBLE);
    for (INT i = 0; i < 5; i++)
    {
        appSprintf(Text, TEXT("%d"), i);
        HairStyleCombo->AddItem(Text);
    }
    for (INT i = 0; i < 10; i++)
    {
        appSprintf(Text, TEXT("%d"), i);
        HairColorCombo->AddItem(Text);
    }

    MeshACombo = new NCComboBox;
    MeshACombo->ID = NID_HAIR_MESHA;
    MeshACombo->SetWindowRect(55, 55, 155, 16);
    CreateChild(MeshACombo, NWS_VISIBLE);
    MeshBCombo = new NCComboBox;
    MeshBCombo->ID = NID_HAIR_MESHB;
    MeshBCombo->SetWindowRect(55, 105, 155, 16);
    CreateChild(MeshBCombo, NWS_VISIBLE);

    TogetherCheck = new NCCheckBox(TEXT("AH & BH TOGETHER"), 0, 0);
    TogetherCheck->ID = NID_HAIR_TOGETHER;
    TogetherCheck->SetWindowRect(20, 12, 150, 16);
    CreateChild(TogetherCheck, NWS_VISIBLE);
    TogetherCheck->bChecked = 1;

    FaceCombo = new NCComboBox;
    FaceCombo->ID = NID_HAIR_FACE;
    FaceCombo->SetWindowRect(55, 155, 155, 16);
    CreateChild(FaceCombo, NWS_VISIBLE);
    return 1;
}
NW_HOOK_METHOD(0x100add80, NCPawnViewr_HairChangeWnd, OnCreate)

// 0x100af150 (vslot 37) - the color combo and the check box are left alone
INT NCPawnViewr_HairChangeWnd::OnDestroy()
{
    DestroyChild(this, HairStyleCombo);
    DestroyChild(this, MeshACombo);
    DestroyChild(this, MeshBCombo);
    DestroyChild(this, FaceCombo);
    DestroyChild(this, ResetButton);
    NCWnd::OnDestroy();
    return 1;
}
NW_HOOK_METHOD(0x100af150, NCPawnViewr_HairChangeWnd, OnDestroy)

// 0x100aed60 (vslot 55) - the labels, the number of meshes found, the textures chosen and the face label, each
// centered in its box; the origin goes on before the clip
INT NCPawnViewr_HairChangeWnd::OnPaint(NCGDevice* Device)
{
    guard(NCPawnViewr_HairChangeWnd::OnPaint);
    TCHAR Text[64];
    SIZE Size;
    Device->BeginPaint(ScreenX, ScreenY);
    Device->SetClip(ScreenX, ScreenY, Width, Height);
    NCGDevice::GetTextExtent(&Size, TEXT("HELM:"), 0, 0, 0, 0, 0);
    Device->DrawText((40 - Size.cx) / 2 + 15, (16 - Size.cy) / 2 + 30, 0xFFFFFFFF, TEXT("HELM:"),
                     0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
    NCGDevice::GetTextExtent(&Size, TEXT("COLOR:"), 0, 0, 0, 0, 0);
    Device->DrawText((55 - Size.cx) / 2 + 95, (16 - Size.cy) / 2 + 30, 0xFFFFFFFF,
                     TEXT("COLOR:"), 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
    appSprintf(Text, TEXT("AH(%d):"), MeshACombo->Items.Num());
    NCGDevice::GetTextExtent(&Size, Text, 0, 0, 0, 0, 0);
    Device->DrawText((40 - Size.cx) / 2 + 15, (16 - Size.cy) / 2 + 55, 0xFFFFFFFF, Text, 0, 0, 0,
                     L2FONT_Normal, 0, 0, 0, 0, 0, 0);
    appSprintf(Text, TEXT("BH(%d):"), MeshBCombo->Items.Num());
    NCGDevice::GetTextExtent(&Size, Text, 0, 0, 0, 0, 0);
    Device->DrawText((40 - Size.cx) / 2 + 15, (16 - Size.cy) / 2 + 105, 0xFFFFFFFF, Text, 0, 0, 0,
                     L2FONT_Normal, 0, 0, 0, 0, 0, 0);
    if (appStrlen(MeshATexture) > 0)
    {
        NCGDevice::GetTextExtent(&Size, MeshATexture, 0, 0, 0, 0, 0);
        Device->DrawText((220 - Size.cx) / 2 + 15, (16 - Size.cy) / 2 + 80, 0xFF808000,
                         MeshATexture, 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
    }
    if (appStrlen(MeshBTexture) > 0)
    {
        NCGDevice::GetTextExtent(&Size, MeshBTexture, 0, 0, 0, 0, 0);
        Device->DrawText((220 - Size.cx) / 2 + 15, (16 - Size.cy) / 2 + 130, 0xFF808000,
                         MeshBTexture, 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
    }
    appSprintf(Text, TEXT("FACE(%d):"), FaceCombo->Items.Num());
    NCGDevice::GetTextExtent(&Size, Text, 0, 0, 0, 0, 0);
    Device->DrawText((40 - Size.cx) / 2 + 10, (16 - Size.cy) / 2 + 155, 0xFFFFFFFF, Text, 0, 0, 0,
                     L2FONT_Normal, 0, 0, 0, 0, 0, 0);
    Device->EndPaint();
    Device->ReleaseClip();
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x100aed60, NCPawnViewr_HairChangeWnd, OnPaint)

// 0x100ae6d0 - the two meshes chosen and their textures on the player's pawn (skeletal meshes loaded when not found),
// and the hair number and color into the chosen default character's record. With no mesh line chosen the index is -1
// and the number read before MeshANumbers is the color
__declspec(noinline) void NCPawnViewr_HairChangeWnd::OnOK()
{
    TCHAR Name[256];
    APawn* Pawn = NPawnView::PlayerPawn();
    if (!Pawn)
        return;
    if (MeshANumbers[MeshACombo->Selected] < 0 || MeshBNumbers[MeshBCombo->Selected] < 0)
        return;
    appSprintf(Name, TEXT("LineageWarriors.%s"), MeshACombo->GetItemText(MeshACombo->Selected));
    Pawn->HairMeshes[1] =
        (UMesh*)FindOrLoad(UMesh::StaticClass(), USkeletalMesh::StaticClass(), Name);
    Pawn->HairMaterials[1] =
        (UMaterial*)FindOrLoad(UMaterial::StaticClass(), UMaterial::StaticClass(), MeshATexture);
    appSprintf(Name, TEXT("LineageWarriors.%s"), MeshBCombo->GetItemText(MeshBCombo->Selected));
    Pawn->HairMeshes[0] =
        (UMesh*)FindOrLoad(UMesh::StaticClass(), USkeletalMesh::StaticClass(), Name);
    Pawn->HairMaterials[0] =
        (UMaterial*)FindOrLoad(UMaterial::StaticClass(), UMaterial::StaticClass(), MeshBTexture);
    INT Index = NPawnView::Viewer()->CharSelectWnd->NameCombo->Selected;
    NPawnView::Viewer()->Characters[Index].HairStyle = HairNumber;
    NPawnView::Viewer()->Characters[Index].HairColor = HairColor;
}
NW_HOOK_METHOD(0x100ae6d0, NCPawnViewr_HairChangeWnd, OnOK)

// 0x100ae260 - the faces 0..9 of the pawn's mesh type that exist (loaded when needed) into the face combo; the combo
// goes to its first line when it had none
__declspec(noinline) void NCPawnViewr_HairChangeWnd::FillFaceList()
{
    TCHAR Path[100];
    TCHAR Name[100];
    if (HairLocked(NPawnView::PlayerPawn()))
        return;
    if (FaceCombo->Selected < 0)
        FaceCombo->SetSelected(0);
    FaceCombo->ClearItems();
    INT* Next = FaceNumbers;
    Face = FaceCombo->Selected;
    for (INT i = 0; i < 10; i++)
    {
        appSprintf(Name, TEXT("%s_m000_t0%d_f"), MeshPrefix(NPawnView::PlayerPawn()), i);
        appSprintf(Path, TEXT("%s.face.%s"), MeshPrefix(NPawnView::PlayerPawn()), Name);
        if (FindOrLoad(UMaterial::StaticClass(), UMaterial::StaticClass(), Path))
        {
            FaceCombo->AddItem(Name);
            *Next++ = i;
        }
    }
}
NW_HOOK_METHOD(0x100ae260, NCPawnViewr_HairChangeWnd, FillFaceList)

// 0x100ae3d0 - the "a" and "b" hair meshes 0..9 of the style suffix chosen that exist in LineageWarriors (loaded when
// needed) into the mesh combos, each list ended by "No Mesh" (number 10); the textures emptied
__declspec(noinline) void NCPawnViewr_HairChangeWnd::FillMeshLists()
{
    TCHAR Path[100];
    TCHAR Name[100];
    if (HairLocked(NPawnView::PlayerPawn()))
        return;
    INT NumA = 0;
    INT NumB = 0;
    if (HairStyleCombo->Selected < 0)
        HairStyleCombo->SetSelected(0);
    if (HairColorCombo->Selected < 0)
        HairColorCombo->SetSelected(0);
    MeshACombo->ClearItems();
    MeshBCombo->ClearItems();
    memset(MeshATexture, 0, sizeof(MeshATexture));
    memset(MeshBTexture, 0, sizeof(MeshBTexture));
    HairStyle = HairStyleCombo->Selected;
    HairColor = HairColorCombo->Selected;
    INT i;
    for (i = 0; i < 10; i++)
    {
        appSprintf(Name, TEXT("%s_m00%d_m0%d_ah"), MeshPrefix(NPawnView::PlayerPawn()), i, HairStyle);
        appSprintf(Path, TEXT("LineageWarriors.%s"), Name);
        if (FindOrLoad(UMesh::StaticClass(), UMesh::StaticClass(), Path))
        {
            MeshACombo->AddItem(Name);
            MeshANumbers[NumA++] = i;
        }
        appSprintf(Name, TEXT("%s_m00%d_m0%d_bh"), MeshPrefix(NPawnView::PlayerPawn()), i, HairStyle);
        appSprintf(Path, TEXT("LineageWarriors.%s"), Name);
        if (FindOrLoad(UMesh::StaticClass(), UMesh::StaticClass(), Path))
        {
            MeshBCombo->AddItem(Name);
            MeshBNumbers[NumB++] = i;
        }
    }
    MeshACombo->AddItem(TEXT("No Mesh"));
    MeshANumbers[NumA] = i;
    MeshBCombo->AddItem(TEXT("No Mesh"));
    MeshBNumbers[NumB] = i;
}
NW_HOOK_METHOD(0x100ae3d0, NCPawnViewr_HairChangeWnd, FillMeshLists)

// 0x100aba40 - the line of a mesh number in the "a" list (the line of "No Mesh" for 10), -1 when it is not there
__declspec(noinline) INT NCPawnViewr_HairChangeWnd::FindMeshA(INT Number)
{
    INT i = 0;
    for (;;)
    {
        if (MeshANumbers[i] == Number)
            return i;
        if (MeshANumbers[i] == 10)
            break;
        i++;
        if (i > 9)
            return -1;
    }
    return i;
}
NW_HOOK_METHOD(0x100aba40, NCPawnViewr_HairChangeWnd, FindMeshA)

// 0x100aba70 - the same in the "b" list
__declspec(noinline) INT NCPawnViewr_HairChangeWnd::FindMeshB(INT Number)
{
    INT i = 0;
    for (;;)
    {
        if (MeshBNumbers[i] == Number)
            return i;
        if (MeshBNumbers[i] == 10)
            break;
        i++;
        if (i > 9)
            return -1;
    }
    return i;
}
NW_HOOK_METHOD(0x100aba70, NCPawnViewr_HairChangeWnd, FindMeshB)

// 0x100ae8c0 (vslot 67) - the button: suffix, color, meshes and face all back to 0
INT NCPawnViewr_HairChangeWnd::OnReset()
{
    HairStyle = 0;
    HairStyleCombo->SetSelected(0);
    HairColor = 0;
    HairColorCombo->SetSelected(0);
    FillMeshLists();
    HairNumber = 0;
    MeshACombo->SetSelected(0);
    OnMeshA(HairNumber);
    MeshBCombo->SetSelected(HairNumber);
    OnMeshB(HairNumber);
    Face = 0;
    FaceCombo->SetSelected(0);
    OnFace(Face);
    return 1;
}
NW_HOOK_METHOD(0x100ae8c0, NCPawnViewr_HairChangeWnd, OnReset)

// 0x100ae970 (vslot 68) - the style combo goes back to its first line (the argument is not looked at)
INT NCPawnViewr_HairChangeWnd::OnStyleCombo(INT Index)
{
    if (!HairLocked(NPawnView::PlayerPawn()))
        HairStyleCombo->SetSelected(0);
    return 1;
}
NW_HOOK_METHOD(0x100ae970, NCPawnViewr_HairChangeWnd, OnStyleCombo)

// 0x100ae9b0 (vslot 69) - a color picked: the two meshes chosen get its textures
INT NCPawnViewr_HairChangeWnd::OnColorCombo(INT Index)
{
    if (!HairLocked(NPawnView::PlayerPawn()))
    {
        HairColor = Index;
        OnMeshA(MeshACombo->Selected);
        OnMeshB(MeshBCombo->Selected);
        OnOK();
    }
    return 1;
}
NW_HOOK_METHOD(0x100ae9b0, NCPawnViewr_HairChangeWnd, OnColorCombo)

// 0x100aea20 (vslot 70) - the "a" mesh's texture name, from the mesh number, color and style suffix ("No Mesh" empties
// it); the mesh goes on the pawn, and with the check box on the "b" mesh of the same number is picked too
INT NCPawnViewr_HairChangeWnd::OnMeshA(INT Index)
{
    if (!HairLocked(NPawnView::PlayerPawn()))
    {
        if (MeshANumbers[Index] == 10)
            memset(MeshATexture, 0, sizeof(MeshATexture));
        else
        {
            HairNumber = MeshANumbers[Index];
            const TCHAR* Prefix = MeshPrefix(NPawnView::PlayerPawn());
            appSprintf(MeshATexture, TEXT("%s.%s_m00%d_t0%d_m0%d_ah"), Prefix, Prefix,
                       MeshANumbers[Index], HairColor, HairStyle);
        }
        OnOK();
        if (TogetherCheck->bChecked && MeshANumbers[Index] != 10)
        {
            INT Other = FindMeshB(MeshANumbers[Index]);
            if (Other != MeshBCombo->Selected)
            {
                MeshBCombo->SetSelected(Other);
                OnMeshB(Other);
            }
        }
    }
    return 1;
}
NW_HOOK_METHOD(0x100aea20, NCPawnViewr_HairChangeWnd, OnMeshA)

// 0x100aeb50 (vslot 71)
INT NCPawnViewr_HairChangeWnd::OnMeshB(INT Index)
{
    if (!HairLocked(NPawnView::PlayerPawn()))
    {
        if (MeshBNumbers[Index] == 10)
            memset(MeshBTexture, 0, sizeof(MeshBTexture));
        else
        {
            HairNumber = MeshBNumbers[Index];
            const TCHAR* Prefix = MeshPrefix(NPawnView::PlayerPawn());
            appSprintf(MeshBTexture, TEXT("%s.%s_m00%d_t0%d_m0%d_bh"), Prefix, Prefix,
                       MeshBNumbers[Index], HairColor, HairStyle);
        }
        OnOK();
        if (TogetherCheck->bChecked && MeshBNumbers[Index] != 10)
        {
            INT Other = FindMeshA(MeshBNumbers[Index]);
            if (Other != MeshACombo->Selected)
            {
                MeshACombo->SetSelected(Other);
                OnMeshA(Other);
            }
        }
    }
    return 1;
}
NW_HOOK_METHOD(0x100aeb50, NCPawnViewr_HairChangeWnd, OnMeshB)

// 0x100aec80 (vslot 72) - a face picked: its number into the chosen default character's record, which dresses the
// player's pawn again
INT NCPawnViewr_HairChangeWnd::OnFace(INT Index)
{
    if (!HairLocked(NPawnView::PlayerPawn()))
    {
        Face = FaceNumbers[Index];
        if (Face < 0)
        {
            Face = 0;
            FaceCombo->SetSelected(0);
        }
        APawn* Pawn = NPawnView::PlayerPawn();
        if (Pawn)
        {
            INT Character = NPawnView::Viewer()->CharSelectWnd->NameCombo->Selected;
            NPawnView::Viewer()->Characters[Character].Pawn = Pawn;
            NPawnView::Viewer()->Characters[Character].Face = Face;
            NPawnView::Viewer()->Characters[Character].SetPawnResource();
        }
    }
    return 1;
}
NW_HOOK_METHOD(0x100aec80, NCPawnViewr_HairChangeWnd, OnFace)

// 0x100afea0 - the values of the character chosen in the character window into the hair controls: the suffix 0, the
// color, the hair number into both mesh combos, the face
__declspec(noinline) void NCPawnViewr_HairChangeWnd::SyncFromCharacter()
{
    INT Index = NPawnView::Viewer()->CharSelectWnd->NameCombo->Selected;
    HairStyle = 0;
    HairStyleCombo->SetSelected(0);
    HairColor = NPawnView::Viewer()->Characters[Index].HairColor;
    HairColorCombo->SetSelected(HairColor);
    FillMeshLists();
    HairNumber = NPawnView::Viewer()->Characters[Index].HairStyle;
    MeshACombo->SetSelected(HairNumber);
    OnMeshA(HairNumber);
    MeshBCombo->SetSelected(HairNumber);
    OnMeshB(HairNumber);
    FillFaceList();
    Face = NPawnView::Viewer()->Characters[Index].Face;
    FaceCombo->SetSelected(Face);
    OnFace(Face);
}
NW_HOOK_METHOD(0x100afea0, NCPawnViewr_HairChangeWnd, SyncFromCharacter)

NW_CHECK_VTABLE(NCPawnViewr_HairChangeWnd, 0x10120cc8, 73)
