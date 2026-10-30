/*=============================================================================
    NConsoleWnd.cpp - the in-game console (the NWindow root): its construction and vtable,
    and the system message lines. The requests to the server are in NConsoleWndNet.cpp, the
    data getters in NConsoleWndData.cpp, the engine's callbacks in NConsoleWndEvents.cpp /
    NConsoleWndItems.cpp / NConsoleWndLogin.cpp, the windows in NConsoleWndWindows.cpp, the
    frame and the input in NConsoleWndInput.cpp, the text services in NConsoleWndText.cpp
    and the key actions in NConsoleWndActions.cpp.
=============================================================================*/
#include "nwindow/NConsoleWnd.h"
#include "nwindow/NCItemWnd.h"
#include "nwindow/NCConsole.h"
#include "nwindow/NDialogInfo.h"

// 0x1006b6a0 - the object is zeroed on allocation; these the stock clears again (C2: WarningDialogBox,
// Unknown357C = 1, Unknown3580, KeyCrypt and bNoChatOnEnter are new)
NConsoleWnd::NConsoleWnd()
{
    bInitialized = 0;
    WndMain = NULL;
    LoginWnd = NULL;
    LobbyWnd = NULL;
    Desktop = NULL;
    DebugWnd = NULL;
    UnusedWnd = NULL;
    MouseWheelDelta = 0;
    DeltaTime = 0;
    bSuspended = 0;
    bWheelHandled = 0;
    KeyCrypt = NULL;
    Unknown3580 = 0;
    bCtrlDown = 0;
    bCtrlDownAlt = 0;
    TutorialEvents = 0;
    bNoChatOnEnter = 0;
    WarningDialogBox = NULL;
    Unknown357C = 1;
    LoadingScreenID = 10001;
}
NW_HOOK_CTOR(0x1006b6a0, NConsoleWnd)
// 0x1006b7c0 - the vtable, ConditionalDestroy, the timers' and the system message parameters' arrays, UL2ConsoleWnd's
NW_HOOK_DTOR(0x1006b7c0, NConsoleWnd)
NW_CHECK_VTABLE_FOR(NConsoleWnd, UObject, 0x10115438, 335)

// the dialog box of a system message (NDialogInfo::ID): OK only
enum { DIALOG_SYSTEM_MESSAGE = 0x1F };

// 0x100578d0 - the interface sound of a name (name ours): none for no name or "None" (C2: the
// sounds are named in the systemmsg lines now, not numbered)
void NConsoleWnd::PlaySoundName(const TCHAR* Name)
{
    if (Name && *Name && appStrcmp(Name, TEXT("None")))
        Engine->OnInterfacePlaySound((TCHAR*)Name);
}
NW_HOOK_METHOD(0x100578d0, NConsoleWnd, PlaySoundName)

// 0x10061d20 - the interface sound of a number (name ours): the stock keeps the names in a table at
// 0x1016a0a0 (the callers push the number) and jumps to PlaySoundName
void NConsoleWnd::PlayInterfaceSound(INT SoundID)
{
    static const TCHAR* const SoundNames[11] =
    {
        TEXT("itemsound.click_1"), TEXT("itemsound.click_2"), TEXT("itemsound.click_failed"),
        TEXT("itemsound.pickup"), TEXT("itemsound.trash_basket"), TEXT("itemsound.window_open"),
        TEXT("itemsound.window_close"), TEXT("itemsound.quest_tutorial"),
        TEXT("itemSound.itemdrop_etc_paper"), TEXT("itemSound.cooltime_end"), TEXT("itemSound.petition")
    };
    PlaySoundName(SoundNames[SoundID]);
}
NW_HOOK_METHOD(0x10061d20, NConsoleWnd, PlayInterfaceSound)

// 0x1006a8c0 - a system message into the chat window (with the sound of its line), or in a dialog
// box for the ones of type 4. The line's validity is read before the id is checked. The color the
// caller passes is not read: C2 takes it from the line (Msg.Unknown14; C1 used the argument).
void NConsoleWnd::ShowSystemMessage(INT ID, DWORD Color)
{
    FL2SystemMsgData& Msg = GL2GameData.SystemMsg[ID];
    if (!Msg.Unknown0 || ID < 0 || ID >= 3000)
        return;
    if (Msg.Type == 4)
    {
        NDialogInfo Info;
        Info.ID = DIALOG_SYSTEM_MESSAGE;
        Info.bMainBox = 1;
        Info.bInput = 0;
        Info.Owner = NULL;
        Info.SetSystemMessage(ID);
        ShowDialog(&Info);
    }
    else
    {
        GetDesktop()->AddSystemMessage(ID, Msg.Unknown14);
        PlaySoundName(*Msg.Message2);
    }
}
NW_HOOK_METHOD(0x1006a8c0, NConsoleWnd, ShowSystemMessage)
