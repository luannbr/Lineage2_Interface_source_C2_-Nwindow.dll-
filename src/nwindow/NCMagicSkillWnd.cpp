/*=============================================================================
    NCMagicSkillWnd.cpp - the skill window and its GM copy. Stock: 0x100729a0 .. 0x100729aa,
    0x100732f0 .. 0x1007374e (WindowProc is NControl's body, OnDestroy NCPetActionWnd's),
    NCGMMagicSkillWnd's 0x1005be40 .. 0x1005be4a, 0x1005cb10 .. 0x1005cdaf and 0x100bd0e0 ..
    0x100bd11e.
=============================================================================*/
#include "nwindow/NCMagicSkillWnd.h"
#include "nwindow/NCSkillWnd.h"
#include "nwindow/NCSkillItemWnd.h"
#include "nwindow/NCFrameCtrl.h"
#include "nwindow/NCGDevice.h"
#include "nwindow/NConsoleWnd.h"

#include <wchar.h>

/*-----------------------------------------------------------------------------
    NCMagicSkillWnd
-----------------------------------------------------------------------------*/

// 0x10091620 - the fields are left as allocated
NCMagicSkillWnd::NCMagicSkillWnd()
{
}
NW_HOOK_CTOR(0x10091620, NCMagicSkillWnd)

// 0x10091670
NW_HOOK_DTOR(0x10091670, NCMagicSkillWnd)

// stock body shared with NControl's (0x10004e90)
INT NCMagicSkillWnd::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (Msg != WM_PAINT && !IsEnabled())
        return 0;
    return NCWnd::WindowProc(Msg, wParam, lParam) != 0;
}

// 0x100916c0 (vslot 36) - the background and the skill list
INT NCMagicSkillWnd::OnCreate()
{
    NCWnd::OnCreate();
    BackTex = FindTexture(TEXT("L2UI.SkillWnd.skillBack"));
    SkillWnd = new NCSkillWnd;
    SkillWnd->ID = NID_MAGICSKILL_LIST;
    SkillWnd->SetWindowRect(0, 0, 240, 300);
    CreateChild(SkillWnd, NWS_VISIBLE | 0x20);
    return 1;
}
NW_HOOK_METHOD(0x100916c0, NCMagicSkillWnd, OnCreate)

// 0x10091790 (vslot 37) - the skill list goes (same body as NCPetActionWnd's, its own copy); BackTex is left
INT NCMagicSkillWnd::OnDestroy()
{
    guard(NCMagicSkillWnd::OnDestroy);
    if (SkillWnd)
    {
        RemoveChildWindow(SkillWnd->Entry);
        delete SkillWnd;
        SkillWnd = NULL;
    }
    NCWnd::OnDestroy();
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x10091790, NCMagicSkillWnd, OnDestroy)

// 0x100918d0 (vslot 43) - only when hidden: the lists empty, the server is asked for them
void NCMagicSkillWnd::ShowWindow()
{
    if (!IsShowWindow())
    {
        ClearSkills();
        L2ParamStack Params(10);
        GConsole->RequestSkillList(Params);
        NCWnd::ShowWindow();
    }
}
NW_HOOK_METHOD(0x100918d0, NCMagicSkillWnd, ShowWindow)

// 0x10091950 (vslot 44)
void NCMagicSkillWnd::HideWindow()
{
    ClearSkills();
    NCWnd::HideWindow();
}
NW_HOOK_METHOD(0x10091950, NCMagicSkillWnd, HideWindow)

// 0x10091840 (vslot 55) - the clip one pixel taller than the window, the background under it
INT NCMagicSkillWnd::OnPaint(NCGDevice* Device)
{
    Device->SetClip(ScreenX, ScreenY, Width, Height + 1);
    Device->BeginPaint(ScreenX, ScreenY);
    if (BackTex)
        Device->DrawTexture(0, 0, 240, 301, 0, 0, 240, 301, BackTex);
    Device->EndPaint();
    Device->ReleaseClip();
    return 1;
}
NW_HOOK_METHOD(0x10091840, NCMagicSkillWnd, OnPaint)

// 0x10090ba0
void NCMagicSkillWnd::ClearSkills()
{
    SkillWnd->ClearLists();
}
NW_HOOK_METHOD(0x10090ba0, NCMagicSkillWnd, ClearSkills)

// a list's skills take the new name and description and make their tool tip again (repeated
// inline in the stock ReloadSkillTexts); the level is NItemEntry +0xC18
static FORCEINLINE void ReloadListTexts(NCSkillItemWnd* List)
{
    for (INT i = 0; i < List->Items.Num(); i++)
    {
        NItemEntry* Item = List->Items(i);
        if (!Item)
            continue;
        if (GConsole->GetSkillName(Item->ID, Item->UnknownC18))
            wcscpy(Item->Name, GConsole->GetSkillName(Item->ID, Item->UnknownC18));
        if (GConsole->GetSkillDesc(Item->ID, Item->UnknownC18))
            wcscpy(Item->Description, GConsole->GetSkillDesc(Item->ID, Item->UnknownC18));
        Item->FillToolTip(4, 0);
    }
}

// 0x10091960 - after the language changed
void NCMagicSkillWnd::ReloadSkillTexts()
{
    ReloadListTexts(SkillWnd->ActiveList);
    ReloadListTexts(SkillWnd->PassiveList);
}
NW_HOOK_METHOD(0x10091960, NCMagicSkillWnd, ReloadSkillTexts)

NW_CHECK_VTABLE(NCMagicSkillWnd, 0x1011b970, 67)

/*-----------------------------------------------------------------------------
    NCGMMagicSkillWnd
-----------------------------------------------------------------------------*/

// 0x100746e0 - the fields are left as allocated
NCGMMagicSkillWnd::NCGMMagicSkillWnd()
{
}
NW_HOOK_CTOR(0x100746e0, NCGMMagicSkillWnd)

// 0x10074730
NW_HOOK_DTOR(0x10074730, NCGMMagicSkillWnd)

// stock body shared with NControl's (0x10004e90)
INT NCGMMagicSkillWnd::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (Msg != WM_PAINT && !IsEnabled())
        return 0;
    return NCWnd::WindowProc(Msg, wParam, lParam) != 0;
}

// 0x10074780 (vslot 36) - the title bar (no text) over the skill list
INT NCGMMagicSkillWnd::OnCreate()
{
    NCWnd::OnCreate();
    FrameCtrl = new NCFrameCtrl(1);
    FrameCtrl->ID = NID_GMMAGICSKILL_FRAME;
    FrameCtrl->SetWindowRect(0, 0, 240, 21);
    CreateChild(FrameCtrl, NWS_VISIBLE);
    SkillWnd = new NCGMSkillWnd;
    SkillWnd->ID = NID_MAGICSKILL_LIST;
    SkillWnd->SetWindowRect(0, 21, 240, 300);
    CreateChild(SkillWnd, NWS_VISIBLE | 0x20);
    BackTex = FindTexture(TEXT("L2UI.SkillWnd.skillBack"));
    return 1;
}
NW_HOOK_METHOD(0x10074780, NCGMMagicSkillWnd, OnCreate)

// 0x100748e0 (vslot 37)
INT NCGMMagicSkillWnd::OnDestroy()
{
    guard(NCGMMagicSkillWnd::OnDestroy);
    if (SkillWnd)
    {
        RemoveChildWindow(SkillWnd->Entry);
        delete SkillWnd;
        SkillWnd = NULL;
    }
    NCWnd::OnDestroy();
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x100748e0, NCGMMagicSkillWnd, OnDestroy)

// stock body shared with NCEditBox's (0x100a5d70)
void NCGMMagicSkillWnd::ShowWindow()
{
    NCWnd::ShowWindow();
}

// 0x10074a20 (vslot 44)
void NCGMMagicSkillWnd::HideWindow()
{
    ClearSkills();
    NCWnd::HideWindow();
}
NW_HOOK_METHOD(0x10074a20, NCGMMagicSkillWnd, HideWindow)

// 0x10074990 (vslot 55) - the clip 8 pixels taller than the window, the background under the
// title bar
INT NCGMMagicSkillWnd::OnPaint(NCGDevice* Device)
{
    Device->SetClip(ScreenX, ScreenY, Width, Height + 8);
    Device->BeginPaint(ScreenX, ScreenY);
    if (BackTex)
        Device->DrawTexture(0, 21, 240, 300, 0, 0, 240, 300, BackTex);
    Device->EndPaint();
    Device->ReleaseClip();
    return 1;
}
NW_HOOK_METHOD(0x10074990, NCGMMagicSkillWnd, OnPaint)

// 0x100730c0
void NCGMMagicSkillWnd::ClearSkills()
{
    SkillWnd->ClearLists();
}
NW_HOOK_METHOD(0x100730c0, NCGMMagicSkillWnd, ClearSkills)

NW_CHECK_VTABLE(NCGMMagicSkillWnd, 0x10116f50, 67)
