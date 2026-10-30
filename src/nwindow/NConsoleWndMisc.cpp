/*=============================================================================
    NConsoleWndMisc.cpp - the small services of the console that the windows call: the player's
    pawn and place, the macros the macro window keeps (the list, a macro by its id), the tattoo
    slots by the class, the additional name of an item, the party window's names, a few tail calls
    into the window manager and the network handler. Every one of them was an anonymous function of
    the C2 image (names ours).
=============================================================================*/
#include "nwindow/NConsoleWnd.h"
#include "nwindow/NCConsole.h"
#include "nwindow/NCEquipItemWnd.h"
#include "nwindow/NCInventoryWnd.h"
#include "nwindow/NCMacroListWnd.h"
#include "nwindow/NCPartyWnd.h"
#include "nwindow/NCShortCutWnd.h"
#include "nwindow/NCStatusWnd.h"
#include "nwindow/NCVirtualWndMain.h"
#include "nwindow/NGameGuard.h"
#include "nwindow/NMacroEntry.h"

// the nProtect GameGuard library's answer to a good start (StartGameGuard): 0x755, read-only data of the stock image
// (name ours); the standalone build defines it in src/standalone/NwGlobals.cpp
#if NW_HOOK_MODE
#define GNPGameMonSuccess NW_GLOBAL(const INT, 0x10129364)
#else
extern const INT GNPGameMonSuccess;
#endif

// the player's windows of the console that the macro functions go through
static FORCEINLINE NCMacroListWnd* MacroWnd(NConsoleWnd* Console)
{
    return static_cast<NCMacroListWnd*>(static_cast<NCConsole*>(Console->Desktop)->NewWndF8);
}

// 0x10060af0 - the player's pawn: the first viewport's controller's pawn. The stock reads through without a check
// (the macro executor asks it only once a viewport exists)
APawn* NConsoleWnd::GetMyPawn()
{
    return Engine->Client->Viewports(0)->Actor->Pawn;
}
NW_HOOK_METHOD(0x10060af0, NConsoleWnd, GetMyPawn)

// 0x10057dd0 - where the player's pawn stands: (0, 0, 0) without a controller or a pawn. (C2: anonymous; its C1 twin,
// GetMyLocation, was 0x10038540)
FVector NConsoleWnd::GetMyLocation()
{
    FVector Result(0.f, 0.f, 0.f);
    APlayerController* Controller = Engine->Client->Viewports(0)->Actor;
    if (Controller && Controller->Pawn)
        Result = Controller->Pawn->Location;
    return Result;
}
NW_HOOK_METHOD(0x10057dd0, NConsoleWnd, GetMyLocation)

/*-----------------------------------------------------------------------------
    The macros (the macro window's list: TArray<NMacroEntry*>, +0xC4 of the window).
-----------------------------------------------------------------------------*/

// 0x10056a20 - how many macros the macro window holds; 0 without the desktop or the window
INT NConsoleWnd::GetMacroCount()
{
    if (Desktop && static_cast<NCConsole*>(Desktop)->NewWndF8)
        return MacroWnd(this)->Macros.Num();
    return 0;
}
NW_HOOK_METHOD(0x10056a20, NConsoleWnd, GetMacroCount)

// 0x10060ce0 - the macro at Index; NULL out of range
NMacroEntry* NConsoleWnd::GetMacro(INT Index)
{
    if (GetMacroCount() > Index && Index >= 0)
        return MacroWnd(this)->Macros(Index);
    return NULL;
}
NW_HOOK_METHOD(0x10060ce0, NConsoleWnd, GetMacro)

// 0x10060d10 - whether a macro of the list is running (HookShortCutKey: a key press stops it first)
UBOOL NConsoleWnd::IsAnyMacroRunning()
{
    INT i = 0;
    if (GetMacroCount() > 0)
    {
        do
        {
            if (GetMacro(i) && GetMacro(i)->bRunning)
                break;
            i++;
        } while (i < GetMacroCount());
    }
    return i < GetMacroCount();
}
NW_HOOK_METHOD(0x10060d10, NConsoleWnd, IsAnyMacroRunning)

// 0x10060d70 - the name of the macro with the server's id MacroID, NULL when the list has none
TCHAR* NConsoleWnd::GetMacroName(INT MacroID)
{
    if (GetMacroCount())
    {
        INT i = 0;
        if (GetMacroCount() > 0)
        {
            do
            {
                if (GetMacro(i) && GetMacro(i)->ID == MacroID)
                    return GetMacro(i)->Name;
                i++;
            } while (i < GetMacroCount());
        }
    }
    return NULL;
}
NW_HOOK_METHOD(0x10060d70, NConsoleWnd, GetMacroName)

// 0x10060dd0 - the icon number of the macro with the id, -1 when the list has none
INT NConsoleWnd::GetMacroIconType(INT MacroID)
{
    if (GetMacroCount())
    {
        INT i = 0;
        if (GetMacroCount() > 0)
        {
            do
            {
                if (GetMacro(i) && GetMacro(i)->ID == MacroID)
                    return GetMacro(i)->IconType;
                i++;
            } while (i < GetMacroCount());
        }
    }
    return -1;
}
NW_HOOK_METHOD(0x10060dd0, NConsoleWnd, GetMacroIconType)

// 0x10061cc0 - a macro slot of the shortcut bar is used: the macro with that id starts running (its flag; the tick
// runs its lines)
void NConsoleWnd::StartMacro(INT MacroID)
{
    INT i = 0;
    if (GetMacroCount() > 0)
    {
        do
        {
            if (GetMacro(i) && GetMacro(i)->ID == MacroID)
            {
                GetMacro(i)->bRunning = 1;
                return;
            }
            i++;
        } while (i < GetMacroCount());
    }
}
NW_HOOK_METHOD(0x10061cc0, NConsoleWnd, StartMacro)

// 0x100569e0 - a macro object (made by ReceiveMacroListInfo) goes into the macro window's list; 0 when there is no
// object or no window
UBOOL NConsoleWnd::AddMacro(NMacroEntry* Macro)
{
    if (Macro && Desktop && static_cast<NCConsole*>(Desktop)->NewWndF8)
    {
        MacroWnd(this)->AddMacro(Macro);
        return 1;
    }
    return 0;
}
NW_HOOK_METHOD(0x100569e0, NConsoleWnd, AddMacro)

/*-----------------------------------------------------------------------------
    The player's class.
-----------------------------------------------------------------------------*/

// 0x10060e90 - how many tattoo slots the player has: by the profession tier of its class (the 58 classes: the
// base class 1, the first profession 2, the second 3); -1 without a player or for an id past the table
INT NConsoleWnd::GetHennaSlotCount()
{
    User* Me = GetMyUser();
    if (Me && (UINT)Me->ClassID <= 0x39)
    {
        switch (Me->ClassID)
        {
        case 0: case 10: case 18: case 25: case 31: case 38: case 44: case 49: case 53:
            return 1;
        case 1: case 4: case 7: case 11: case 15: case 19: case 22: case 26: case 29: case 32:
        case 35: case 39: case 42: case 45: case 47: case 50: case 54: case 56:
            return 2;
        case 2: case 3: case 5: case 6: case 8: case 9: case 12: case 13: case 14: case 16: case 17:
        case 20: case 21: case 23: case 24: case 27: case 28: case 30: case 33: case 34: case 36:
        case 37: case 40: case 41: case 43: case 46: case 48: case 51: case 52: case 55: case 57:
            return 3;
        }
    }
    return -1;
}
NW_HOOK_METHOD(0x10060e90, NConsoleWnd, GetHennaSlotCount)

/*-----------------------------------------------------------------------------
    Items, the party and the window manager.
-----------------------------------------------------------------------------*/

// 0x10057730 - the item has an additional name (the data holds "NAME_None" when it has not)
UBOOL NConsoleWnd::HasItemAdditionalName(INT ItemID)
{
    TCHAR* Name = GL2GameData.GetItemAdditionalName(ItemID);
    if (Name && appStrcmp(Name, TEXT("NAME_None")))
        return 1;
    return 0;
}
NW_HOOK_METHOD(0x10057730, NConsoleWnd, HasItemAdditionalName)

// 0x100578a0 - the item's additional name. The stock compares it with "NAME_None" and drops the result: it returns the
// name either way
const TCHAR* NConsoleWnd::GetItemAdditionalName(INT ItemID)
{
    TCHAR* Name = GL2GameData.GetItemAdditionalName(ItemID);
    if (Name)
        appStrcmp(Name, TEXT("NAME_None"));
    return Name;
}
NW_HOOK_METHOD(0x100578a0, NConsoleWnd, GetItemAdditionalName)

// 0x10052c50 - the name of the Index-th member of the party window (the %party1.. words of a /command, ExpandToken);
// NULL out of range or without the window
TCHAR* NConsoleWnd::GetPartyMemberName(INT Index)
{
    if (Desktop)
    {
        NCPartyWnd* Party = static_cast<NCConsole*>(Desktop)->PartyWnd;
        if (Party && Party->Members)
        {
            if (Index < Party->NumMembers && Index >= 0)
                return Party->Members[Index]->Name;
        }
    }
    return NULL;
}
NW_HOOK_METHOD(0x10052c50, NConsoleWnd, GetPartyMemberName)

// 0x10056130 - the mouse position of the game window, from the window manager (a tail call into
// NCVirtualWndMain::GetMousePos, 0x1002e390; NCWnd::Snap asks it)
UBOOL NConsoleWnd::GetMousePos(INT* X, INT* Y)
{
    return WndMain->GetMousePos(X, Y);
}
NW_HOOK_METHOD(0x10056130, NConsoleWnd, GetMousePos)

// 0x10055c10 - the GM's snoop window was closed: NetHandler slot 251 (a tail call)
void NConsoleWnd::RequestGMSnoopEnd(INT SnoopID)
{
    Engine->NetHandler->GMSnoopEnd(SnoopID);
}
NW_HOOK_METHOD(0x10055c10, NConsoleWnd, RequestGMSnoopEnd)

// 0x100560f0 - a line into the desktop's chat window, with sub type 5 and the given color (the /setcl help)
void NConsoleWnd::ShowChatText(const TCHAR* Text, DWORD Color)
{
    if (Desktop)
        static_cast<NCConsole*>(Desktop)->AddSystemText(Text, 5, Color);
}
NW_HOOK_METHOD(0x100560f0, NConsoleWnd, ShowChatText)

// 0x10055e20 - the text of system message MsgID with one number as its parameter, written to Out instead of shown
// (the security card prompt of the login box): the number is added as a parameter of type 1, then the message is
// ended with the text asked for
void NConsoleWnd::GetNumberMessage(INT MsgID, INT Number, TCHAR* Out)
{
    L2ParamStack Params(10);
    Params.PushBack((void*)1);
    Params.PushBack((void*)(INT_PTR)Number);
    AddSystemMessageParam(Params);
    L2ParamStack Message(10);
    Message.PushBack((void*)(INT_PTR)MsgID);
    EndSystemMessageParam(Message, 1, (INT)(INT_PTR)Out);
}
NW_HOOK_METHOD(0x10055e20, NConsoleWnd, GetNumberMessage)

// 0x10052110 (guard name) - the card key for the login (NetHandler slot 68, RequestSCCheck)
UBOOL NConsoleWnd::RequestAuthCardKeyLogin(INT CardID, INT Key)
{
    guard(NConsoleWnd::RequestAuthCardKeyLogin);
    return Engine->NetHandler->RequestSCCheck(CardID, Key) != 0;
    unguard;
}
NW_HOOK_METHOD(0x10052110, NConsoleWnd, RequestAuthCardKeyLogin)

// 0x10066fd0 - the "/useshortcut" kind of command 0x50: the item of the shortcut slot, when it is a weapon or armor that
// the player does not wear yet, is put on (RequestUseItem). The slot Index counts the pages
void NConsoleWnd::UseShortCutEquip(INT Index)
{
    NShortCutItem* Item = static_cast<NCConsole*>(Desktop)->ShortCutWnd->ItemAt(Index);
    if (Item && Item->Type == SHORTCUT_Item)
    {
        NCEquipItemWnd* Equip = static_cast<NCConsole*>(Desktop)->InventoryWnd->EquipWnd;
        for (INT i = 0; i < Equip->Items.Num(); i++)
            if (Equip->Items(i) && Equip->Items(i)->ID == Item->ID)
                return;
        NViewItemData* Data = NItemTable::Find(&Item->ID);
        if (Data && (Data->Class == ITEMCLASS_Weapon || Data->Class == ITEMCLASS_Armor))
            RequestUseItem(Item->ObjectID);
    }
}
NW_HOOK_METHOD(0x10066fd0, NConsoleWnd, UseShortCutEquip)

// 0x1004c5c0 - Cast<ALineagePlayerController>, out of line
ALineagePlayerController* CastToLineagePlayerController(UObject* Object)
{
    if (Object && Object->IsA(ALineagePlayerController::StaticClass()))
        return (ALineagePlayerController*)Object;
    return NULL;
}
NW_HOOK_FUNC(0x1004c5c0, CastToLineagePlayerController)

// 0x100574e0 (name ours; the stock's GIsL2NetLogin branch of NConsoleWnd::Initialize, which the C2 binary patch jumps
// over, so it does not run in the patched client) - the game's side of the nProtect GameGuard start-up. The library
// (NGameGuard.h: statically linked into the stock image, which hook mode calls in place; the standalone build has none) is
// asked to start with NPGameLibApi::InitNPGameMon (0x100DFF20); its answer 0x755 (the library's success code, kept at
// 0x10129364) or 0x73 hands the game window to SetHwndToGameMon (0x100DFF40); any other answer is shown in the warning
// dialog as "<text> (ErrorCode=<answer>)", the text a system message chosen by the answer (765 .. 771; 1077 for the
// answers not listed)
void NConsoleWnd::StartGameGuard(INT hWnd)
{
    TCHAR Message[512];
    INT Code = NGameGuard::Init();
    INT MsgID;
    if (Code == GNPGameMonSuccess)
    {
        NGameGuard::SetHwnd(hWnd);
        return;
    }
    switch (Code)
    {
    case 0x73:
        NGameGuard::SetHwnd(hWnd);
        return;
    case 0x6E: MsgID = 765; break;
    case 0x72: MsgID = 766; break;
    case 0x78: case 0x7C: case 0x96: case 0x99: case 0xAA:
        MsgID = 767;
        break;
    case 0x9B: MsgID = 768; break;
    case 0xC8: MsgID = 769; break;
    case 0x154: case 0x15E: case 0x17C:
        MsgID = 770;
        break;
    case 0x168: MsgID = 771; break;
    default: MsgID = 1077; break;
    }
    appSprintf(Message, TEXT("%s (ErrorCode=%d)"), *GL2GameData.SystemMsg[MsgID].Message, Code);
    ShowWarningDialog((INT)(INT_PTR)Message, 1);
}
NW_HOOK_METHOD(0x100574e0, NConsoleWnd, StartGameGuard)

// 0x10026550 - the allocator's Free (slot 302, a virtual of the console that does nothing else; the stock linker folded the
// global operator delete into it, so NString and the other stock code call it as a function): GMalloc->Free(Ptr)
void NConsoleWnd::Free(void* Ptr)
{
    appFree(Ptr);
}
NW_HOOK_METHOD(0x10026550, NConsoleWnd, Free)
