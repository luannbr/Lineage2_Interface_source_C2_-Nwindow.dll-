/*=============================================================================
    NCActionItemListWnd.cpp - the Basic page of the action window. Stock: 0x10034d10 ..
    0x10034f4e, 0x10036c70 .. 0x10036db3, 0x10037060 .. 0x1003724c.
=============================================================================*/
#include "nwindow/NCActionItemListWnd.h"
#include "nwindow/NCActionItemWnd.h"
#include "nwindow/NItemInfo.h"
#include "nwindow/NCGDevice.h"
#include "nwindow/NCConsole.h"
#include "nwindow/NConsoleWnd.h"

// 0x10040cd0 - only BackTex is cleared
NCActionItemListWnd::NCActionItemListWnd()
{
    BackTex = NULL;
}
NW_HOOK_CTOR(0x10040cd0, NCActionItemListWnd)

// 0x10040d20
NW_HOOK_DTOR(0x10040d20, NCActionItemListWnd)

// stock body shared with NControl's (0x10004e90)
INT NCActionItemListWnd::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (Msg != WM_PAINT && !IsEnabled())
        return 0;
    return NCWnd::WindowProc(Msg, wParam, lParam) != 0;
}

// one grid of the page (repeated inline in the stock OnCreate)
static FORCEINLINE NCActionItemWnd* CreateGrid(NCWnd* Parent, INT Rows, INT Y, INT H)
{
    NCActionItemWnd* Grid = new NCActionItemWnd(6, Rows, 15, 32);
    Grid->SetWindowRect(15, Y, 207, H);
    Parent->CreateChild(Grid, NWS_VISIBLE | 0x20);   // 0x20: meaning unknown
    return Grid;
}

// 0x10043720 (vslot 36) - the three grids; the actions are loaded when the action window shows (C1 loaded them here)
INT NCActionItemListWnd::OnCreate()
{
    NCWnd::OnCreate();
    BackTex = FindTexture(TEXT("L2UI.ActionWnd.ActionBack"));
    BasicGrid = CreateGrid(this, 2, 15, 70);
    PartyGrid = CreateGrid(this, 1, 109, 35);
    SocialGrid = CreateGrid(this, 3, 168, 100);
    return 1;
}
NW_HOOK_METHOD(0x10043720, NCActionItemListWnd, OnCreate)

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

// 0x10040e70 (vslot 37)
INT NCActionItemListWnd::OnDestroy()
{
    guard(NCActionItemListWnd::OnDestroy);
    DestroyChild(this, BasicGrid);
    DestroyChild(this, PartyGrid);
    DestroyChild(this, SocialGrid);
    NCWnd::OnDestroy();
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x10040e70, NCActionItemListWnd, OnDestroy)

// 0x10040d70 (vslot 55) - the background reaches 17 pixels above the page (behind the tabs); labels "Party" / "Social"
// (sysstring 130 / 204). The clip is set before the origin.
INT NCActionItemListWnd::OnPaint(NCGDevice* Device)
{
    Device->SetClip(ScreenX, ScreenY - 17, Width, Height + 17);
    Device->BeginPaint(ScreenX, ScreenY);
    if (BackTex)
        Device->DrawTexture(0, -17, 240, 301, 0, 0, 240, 301, BackTex);
    Device->DrawText(16, 90, 0xFFFFFFFF, *SysString(130), 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
    Device->DrawText(16, 150, 0xFFFFFFFF, *SysString(204), 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
    Device->EndPaint();
    Device->ReleaseClip();
    return 1;
}
NW_HOOK_METHOD(0x10040d70, NCActionItemListWnd, OnPaint)

// an action of actionname-*.dat the way LoadActions reads it (name ours; UE2Engine.h's FL2ActionEntry places only two
// strings, and by the walk below its Name and IconName are swapped: the name is at +1C, the icon's at +28)
struct NActionRow
{
    INT bValid;            // +0x00 0: no action here
    INT ActionID;          // +0x04
    INT Param;             // +0x08 the social action of the social ones
    INT Category;          // +0x0C 1 basic, 2 party, 3 social (4 pet, 5 summon: the pet windows)
    INT* Condition;        // +0x10 points at an int; -1: nothing to wait for (name ours, meaning not known)
    BYTE _Unknown14[8];    // +0x14
    FString Name;          // +0x1C
    FString IconName;      // +0x28
    FString Description;   // +0x34
};

// 0x10043910 (name ours) - every action of the table into the grid of its category (a missing icon is replaced by
// black); a full grid drops the rest. C2: the action 25 shows only when the player has the crystallize flag, and an
// action with a condition (the pointer at +10 pointing at -1) shows only with a pet or a summon: the pawn's field at
// +5F8, or the pet info with its flag at +A8 and an object id.
void NCActionItemListWnd::LoadActions()
{
    for (INT i = 0; i < GL2GameData.ActionTable.Num(); i++)
    {
        NActionRow* Row = (NActionRow*)GL2GameData.ActionTable(i);
        if (Row->bValid != 0 && Row->Category != 0
            && (Row->ActionID != 0x25 || (GConsole->GetMyUser() && GConsole->GetMyUser()->bCanCrystallize != 0)))
        {
            INT Category = Row->Category;
            NItemInfo Info;
            Info.ID = Row->ActionID;
            Info.Param = Row->Param;
            appStrcpy(Info.Name, *Row->Name);
            appStrcpy(Info.IconName, *Row->IconName);
            appStrcpy(Info.Description, *Row->Description);
            Info.Icon = FindTexture(Info.IconName);
            if (!Info.Icon)
                Info.Icon = FindTexture(TEXT("NWindow.BlackTexture"));
            PetInfo Pet = GConsole->GetPetInfo();
            if ((*Row->Condition != -1
                 || (GConsole->GetMyUser() && GConsole->GetMyUser()->Pawn
                     && (nw::Field<INT>(GConsole->GetMyUser()->Pawn, 0x5F8) != 0
                         || (Pet.UnknownA8 != 0 && Pet.ObjectID >= 0))))
                && (Category == 1 || Category == 2 || Category == 3))
            {
                switch (Category)
                {
                case 1:
                    BasicGrid->AddItem(&Info);
                    break;
                case 2:
                    PartyGrid->AddItem(&Info);
                    break;
                case 3:
                    SocialGrid->AddItem(&Info);
                    break;
                }
            }
        }
    }
}
NW_HOOK_METHOD(0x10043910, NCActionItemListWnd, LoadActions)

// 0x10043d40 (name ours) - the three grids' items are deleted (each: NCActionItemWnd::ClearItems, 0x100d8950)
void NCActionItemListWnd::ClearGrids()
{
    BasicGrid->ClearItems();
    PartyGrid->ClearItems();
    SocialGrid->ClearItems();
}
NW_HOOK_METHOD(0x10043d40, NCActionItemListWnd, ClearGrids)

NW_CHECK_VTABLE(NCActionItemListWnd, 0x10111ab8, 67)
