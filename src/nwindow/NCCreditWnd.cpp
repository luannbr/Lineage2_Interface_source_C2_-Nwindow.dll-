/*=============================================================================
    NCCreditWnd.cpp - the credits (C2 only). Stock: 0x1006e020 .. 0x1006e92a.
=============================================================================*/
#include <intrin.h>
#include "nwindow/NCCreditWnd.h"
#include "nwindow/NCHtmlViewer.h"
#include "nwindow/NCGDevice.h"
#include "nwindow/NConsoleWnd.h"
#include "nwindow/NCLoginWnd.h"
#include "nwindow/NCMessageWnd.h"

#if NW_HOOK_MODE
// the [Credit] key names, TCHAR[5][255] ("CreditSound1".."CreditSound5") in the stock image
#define GCreditSoundKeys (reinterpret_cast<const TCHAR (*)[255]>(::nw::Addr(0x1016A4F0)))
#else
extern const TCHAR GCreditSoundKeys[5][255];
#endif

// 0x1006e070 - NControl's constructor is inlined in the stock code; the sound names are
// left as allocated
NCCreditWnd::NCCreditWnd()
{
    BackTex = NULL;
    Viewer = NULL;
    Elapsed = 0;
    Page = 0;
    MusicHandle = 0;
    SavedVolume = 0;
    PageSeconds = 0;
}
NW_HOOK_CTOR(0x1006e070, NCCreditWnd)

// 0x1006e100
NW_HOOK_DTOR(0x1006e100, NCCreditWnd)

// the table of pages (FL2GameData +0x4CC0 holds a pointer to it)
#define GCreditTable ((NCreditTable*)GL2GameData.CreditTable)

// 0x1006e300 (ret 4) - TMap::Find: the pair of the key's hash chain with the key
NCreditPage** NCreditTable::Find(const UINT* Key)
{
    INT Index = Hash[(HashCount - 1) & *Key];
    if (Index != -1)
    {
        do
        {
            if (Pairs[Index].Key == *Key)
                return &Pairs[Index].Value;
            Index = Pairs[Index].HashNext;
        } while (Index != -1);
    }
    return NULL;
}
NW_HOOK_METHOD(0x1006e300, NCreditTable, Find)

// 0x1006e020 - the credits' own font goes (a member that ignores its `this`)
void NCCreditWnd::FreeFont()
{
    NCGDevice::DestroyFont(2);
}
NW_HOOK_METHOD(0x1006e020, NCCreditWnd, FreeFont)

// 0x1006de70 (ret 4) - the text's rectangle (400 x 550) by the page's Align: 1..9 are the nine places of a
// three by three grid over the screen (30 pixels from the edges; the middle ones centered); any other value
// is the top left
void NCCreditWnd::PlaceViewer(INT Align)
{
    INT ScreenW = GSEKScreenX;
    INT ScreenH = GSEKScreenY;
    switch (Align)
    {
    default:
        Viewer->SetWindowRect(30, 30, 400, 550);
        return;
    case 2:
        Viewer->SetWindowRect(ScreenW / 2 - 200, 30, 400, 550);
        return;
    case 3:
        Viewer->SetWindowRect(ScreenW - 430, 30, 400, 550);
        return;
    case 4:
        Viewer->SetWindowRect(30, ScreenH / 2 - 275, 400, 550);
        return;
    case 5:
        Viewer->SetWindowRect(ScreenW / 2 - 200, ScreenH / 2 - 275, 400, 550);
        return;
    case 6:
        Viewer->SetWindowRect(ScreenW - 430, ScreenH / 2 - 275, 400, 550);
        return;
    case 7:
        Viewer->SetWindowRect(30, ScreenH - 580, 400, 550);
        return;
    case 8:
        Viewer->SetWindowRect(ScreenW / 2 - 200, ScreenH - 580, 400, 550);
        return;
    case 9:
        Viewer->SetWindowRect(ScreenW - 430, ScreenH - 580, 400, 550);
        return;
    }
}
NW_HOOK_METHOD(0x1006de70, NCCreditWnd, PlaceViewer)

// 0x1006e360 - the next page: its picture, its text file and its place; past the last one the credits
// end (the login screen comes back, the intro music again and the volume), as on Escape
void NCCreditWnd::NextPage()
{
    Elapsed = 0;
    Page++;
    FString Path;
    if (Page < PageCount)
    {
        NCreditPage* Record = *GCreditTable->Find((const UINT*)&Page);
        PageSeconds = Record->Seconds;
        BackTex = FindTexture(*Record->Texture);
        Path = FString(TEXT("..\\L2Text\\")) + Record->File;
        Viewer->LoadHtmlFile((TCHAR*)*Path);
        PlaceViewer(Record->Align);
    }
    else
    {
        GConsole->LoginWnd->ShowWindow();
        GConsole->MessageWnd->ShowWindow();
        HideWindow();
        if (MusicHandle && GConsole && GConsole->Engine && GConsole->Engine->Audio)
        {
            GConsole->Engine->Audio->StopMusic(MusicHandle, 2.0f);
            GConsole->PlayIntroMusic();
            GConsole->Engine->Audio->SetSoundVolume((FLOAT)SavedVolume);
        }
        KillFocus();
        Page = 0;
        Elapsed = 0;
        PageSeconds = 0;
        FreeFont();
    }
}
NW_HOOK_METHOD(0x1006e360, NCCreditWnd, NextPage)

// 0x1006e560 (ret 4) - time goes on; while the credits are shown and have the focus and no music plays, one
// of the five tracks is chosen at random (the old one stopped); a page that outlasted its seconds is replaced
void NCCreditWnd::Tick(FLOAT DeltaTime)
{
    Elapsed = DeltaTime + Elapsed;
    if (GConsole && GConsole->Engine && GConsole->Engine->Audio)
    {
        UAudioSubsystem* Audio = GConsole->Engine->Audio;
        if (!Audio->IsMusicPlaying() && IsShowWindow() && IsFocused())
        {
            if (MusicHandle)
                Audio->StopMusic(MusicHandle, 0.0f);
            INT Pick = appRand() % 5;
            MusicHandle = GConsole->Engine->Audio->PlayMusic(FString(SoundName[Pick]), 1.5f, 0);
        }
    }
    if ((FLOAT)PageSeconds < Elapsed)
        NextPage();
}
NW_HOOK_METHOD(0x1006e560, NCCreditWnd, Tick)

// 0x1006e650 - the credits' font, the first page (number 0) and no sound (the music comes with Tick)
void NCCreditWnd::Start()
{
    NCGDevice::LoadFont(2, TEXT("L2UI.CreditWnd.CreditFont"), TEXT("CreditFont.gly"));
    UINT Key = 0;
    NCreditPage** Slot = GCreditTable->Find(&Key);
    if (Slot)
    {
        FString Path;
        NCreditPage* Record = *Slot;
        PlaceViewer(Record->Align);
        PageSeconds = Record->Seconds;
        BackTex = FindTexture(*Record->Texture);
        Path = FString(TEXT("..\\L2Text\\")) + Record->File;
        Viewer->LoadHtmlFile((TCHAR*)*Path);
        Viewer->ShowBars(0);   // 0x100254D0, NCScrollWnd: the scroll bar's parts hidden
        GConsole->Engine->Audio->SetSoundVolume(0.0f);
    }
}
NW_HOOK_METHOD(0x1006e650, NCCreditWnd, Start)

// 0x1006e030 (vslot 43) - the volume is kept (truncated to a number) to be given back at the end
void NCCreditWnd::ShowWindow()
{
    if (GConsole && GConsole->Engine && GConsole->Engine->Audio)
        SavedVolume = (INT)GConsole->Engine->Audio->GetSoundVolume();
    NCWnd::ShowWindow();
}
NW_HOOK_METHOD(0x1006e030, NCCreditWnd, ShowWindow)

// 0x1006e150 (vslot 37) - not guarded; NControl's OnDestroy is not called
INT NCCreditWnd::OnDestroy()
{
    if (Viewer)
    {
        RemoveChildWindow(Viewer->Entry);
        delete Viewer;
        Viewer = NULL;
    }
    return 1;
}
NW_HOOK_METHOD(0x1006e150, NCCreditWnd, OnDestroy)

// 0x1006e190 (vslot 55) - the page's picture over the whole window (a 1024 x 768 source)
INT NCCreditWnd::OnPaint(NCGDevice* Device)
{
    Device->SetClip(ScreenX, ScreenY, Width, Height);
    Device->BeginPaint(ScreenX, ScreenY);
    if (BackTex)
        Device->DrawStretchedTex(0, 0, Width, Height, 0, 0, 1024, 768, BackTex);
    Device->EndPaint();
    Device->ReleaseClip();
    return 1;
}
NW_HOOK_METHOD(0x1006e190, NCCreditWnd, OnPaint)

// 0x1006e220 (vslot 58) - Escape ends the credits: the login screen comes back, the intro
// music starts again and the volume is given back; every other key goes to NCWnd
INT NCCreditWnd::OnKeyDown(WPARAM wParam, LPARAM lParam)
{
    if (wParam == VK_ESCAPE)
    {
        GConsole->LoginWnd->ShowWindow();
        GConsole->MessageWnd->ShowWindow();
        HideWindow();
        KillFocus();
        Page = 0;
        Elapsed = 0;
        PageSeconds = 0;
        if (MusicHandle && GConsole && GConsole->Engine && GConsole->Engine->Audio)
        {
            GConsole->Engine->Audio->StopMusic(MusicHandle, 2.0f);
            GConsole->PlayIntroMusic();
            GConsole->Engine->Audio->SetSoundVolume((FLOAT)SavedVolume);
        }
        FreeFont();
    }
    NCWnd::OnKeyDown(wParam, lParam);
    return 1;
}
NW_HOOK_METHOD(0x1006e220, NCCreditWnd, OnKeyDown)

// 0x1006e540 (vslot 61) - a click goes to the next page
INT NCCreditWnd::OnLButtonDown(WPARAM wParam, LPARAM lParam)
{
    if (IsShowWindow())
        NextPage();
    return 1;
}
NW_HOOK_METHOD(0x1006e540, NCCreditWnd, OnLButtonDown)

// 0x1006e7a0 (vslot 36) - the HTML viewer (the stock constructor with the argument 2), the
// music names and the number of pages; NControl's OnCreate is not called
INT NCCreditWnd::OnCreate()
{
    Viewer = new NCHtmlViewer(2);   // 0x100331A0
    Viewer->SetWindowRect(30, 30, 400, 550);
    CreateChild(Viewer, NWS_VISIBLE);
    Viewer->ShowWindow();
    Viewer->ShowBars(0);   // 0x100254D0, NCScrollWnd: the scroll bar's parts hidden

    appRandInit((INT)(DWORD)__rdtsc());
    for (INT i = 0; i < 5; i++)
    {
        if (!GConfig->GetString(TEXT("Credit"), GCreditSoundKeys[i], SoundName[i], 256, TEXT("ALAudio.int")))
            appStrcpy(SoundName[i], TEXT("intro"));
    }
    PageCount = 0;
    NCreditTable* Table = GCreditTable;
    for (INT i = 0; i < Table->Num; i++)
        PageCount++;
    return 1;
}
NW_HOOK_METHOD(0x1006e7a0, NCCreditWnd, OnCreate)

NW_CHECK_VTABLE(NCCreditWnd, 0x10115bc0, 68)
