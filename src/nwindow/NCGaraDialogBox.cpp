/*=============================================================================
    NCGaraDialogBox.cpp - the desktop's message box (C2 only). Stock: 0x100065f0 .. 0x100067cf,
    0x10006860 .. 0x10006bcf (Cancel), 0x10006c50 .. 0x10006d0d, 0x10006f90 .. 0x10007b1f,
    0x10007b20 .. 0x10008e77 (OK), 0x10008fa0 .. 0x10009091 and 0x10009500 (ShowWindow);
    NDialogInfo keeps 0x100064b0 .. 0x100065c0, 0x10006e60 and 0x100067d0.
=============================================================================*/
#include "nwindow/NCGaraDialogBox.h"
#include "nwindow/NDialogInfo.h"
#include "nwindow/NCButton.h"
#include "nwindow/NCConsole.h"
#include "nwindow/NCDialogBox.h"
#include "nwindow/NCEditBox.h"
#include "nwindow/NCGDevice.h"
#include "nwindow/NCItemWnd.h"
#include "nwindow/NCProgressCtrl.h"
#include "nwindow/NConsoleWnd.h"
#include "nwindow/NCrt.h"

enum { SYSMSG_ItemFlying = 179 };           // "cannot be dropped here" (name ours): SystemMsg[179]
enum { COLOR_Warning = 0xFFB09B79 };        // the system message color
enum { SOUND_TrashBasket = 4 };             // the table's 4 (name ours)

/*-----------------------------------------------------------------------------
    Construction.
-----------------------------------------------------------------------------*/

// 0x10006f90 - NControl's constructor is inlined in the stock code; the keypad, the edit line, Type,
// bLargeAmount and the place are left as allocated
NCGaraDialogBox::NCGaraDialogBox()
{
    BackTex = NULL;
    InputBackTex = NULL;
    OKButton = NULL;
    CancelButton = NULL;
    Progress = NULL;
    Owner = NULL;
    bProgress = 0;
    bInput = 0;
    Param1 = 0;
    Param2 = 0;
    DialogID = -1;
}
NW_HOOK_CTOR(0x10006f90, NCGaraDialogBox)

// 0x10007090
NW_HOOK_DTOR(0x10007090, NCGaraDialogBox)

/*-----------------------------------------------------------------------------
    The small virtuals.
-----------------------------------------------------------------------------*/

// 0x100065f0 (vslot 25)
INT NCGaraDialogBox::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (NControl::WindowProc(Msg, wParam, lParam))
        return 1;
    if (Msg == NWM_COMMAND)
    {
        if ((INT)wParam == NID_DIALOG_OK)
            return OnOK() != 0;
        if ((INT)wParam == NID_DIALOG_CANCEL)
            return OnCancel() != 0;
        if ((INT)wParam == NID_DIALOG_KEYPAD)
            return OnKeypad(lParam) != 0;
        if ((INT)wParam == NID_DIALOG_EDIT)
            return OnEditNotify(lParam) != 0;
        if ((INT)wParam == NID_DIALOG_PROGRESS)
            return OnProgressEnd() != 0;
    }
    return 0;
}
NW_HOOK_METHOD(0x100065f0, NCGaraDialogBox, WindowProc)

// 0x100066d0 (vslot 29)
void NCGaraDialogBox::SetFocus()
{
    NCWnd::SetFocus();
    if (bInput)
        EditBox->SetFocus();
}
NW_HOOK_METHOD(0x100066d0, NCGaraDialogBox, SetFocus)

// 0x100066f0 (vslot 58) - Enter answers yes only where something was typed or the box is a notice
INT NCGaraDialogBox::OnKeyDown(WPARAM wParam, LPARAM lParam)
{
    if (wParam == VK_RETURN)
    {
        if (bInput == 0 && DialogID != 0x1D && DialogID != 0x1F && DialogID != 0x10)
        {
            OnCancel();
            return 1;
        }
        OnOK();
    }
    return 1;
}
NW_HOOK_METHOD(0x100066f0, NCGaraDialogBox, OnKeyDown)

// 0x10006790 (vslot 44)
void NCGaraDialogBox::HideWindow()
{
    Reset();
    NCWnd::HideWindow();
}
NW_HOOK_METHOD(0x10006790, NCGaraDialogBox, HideWindow)

// 0x10009500 (vslot 43)
void NCGaraDialogBox::ShowWindow()
{
    NCWnd::ShowWindow();
    UpdateLayout();
}
NW_HOOK_METHOD(0x10009500, NCGaraDialogBox, ShowWindow)

// 0x100067a0 (vslot 70) - Enter in the edit line (1), or on an empty one (0)
INT NCGaraDialogBox::OnEditNotify(INT Code)
{
    if (Code == 1)
    {
        OnOK();
        return 1;
    }
    if (Code == 0)
        OnCancel();
    return 1;
}
NW_HOOK_METHOD(0x100067a0, NCGaraDialogBox, OnEditNotify)

// 0x10006d00 (vslot 72) - time is up
INT NCGaraDialogBox::OnProgressEnd()
{
    OnCancel();
    return 1;
}
NW_HOOK_METHOD(0x10006d00, NCGaraDialogBox, OnProgressEnd)

// 0x10008fa0 (vslot 71) - a key of the keypad: the digits are typed into the edit line, key 10 is the
// backspace, key 11 types the number of Unknown238 (the line emptied first), key 12 a zero
INT NCGaraDialogBox::OnKeypad(INT Key)
{
    TCHAR Text[128];
    if (Key >= 0 && Key < 9)
    {
        appSprintf(Text, TEXT("%d"), Key + 1);
        EditBox->InsertText(Text, 1);
        return 1;
    }
    if (Key == 9)
    {
        appSprintf(Text, TEXT("%d"), 0);
        EditBox->InsertText(Text, 1);
        return 1;
    }
    if (Key == 10)
    {
        SendTo(EditBox->Entry, WM_KEYDOWN, VK_BACK, 0);
        return 1;
    }
    if (Key == 11)
    {
        appSprintf(Text, TEXT("%d"), Unknown238);
        EditBox->ClearText();
        EditBox->InsertText(Text, 1);
        return 1;
    }
    if (Key == 12)
    {
        EditBox->ClearText();
        EditBox->InsertText(TEXT("0"), 1);
    }
    return 1;
}
NW_HOOK_METHOD(0x10008fa0, NCGaraDialogBox, OnKeypad)

/*-----------------------------------------------------------------------------
    Creation, destruction and paint.
-----------------------------------------------------------------------------*/

// 0x10007110 (vslot 36) - the edit line, the keypad right of the box (1..9 in rows of three, then
// 0, a backspace, a wide "00" and a clear), the progress bar and the two buttons; no OnCreate of the
// base is called
INT NCGaraDialogBox::OnCreate()
{
    BackTex = FindTexture(TEXT("L2UI.EtcWnd.Popup"));
    InputBackTex = FindTexture(TEXT("L2UI.EtcWnd.Popup"));
    KeypadTex = FindTexture(TEXT("L2ui.numpad.numpad_back"));

    EditBox = new NCEditBox(3, NULL, 1, 150, 0);
    EditBox->ID = NID_DIALOG_EDIT;
    EditBox->SetWindowRect(44, 41, 116, 16);
    CreateChild(EditBox, NWS_VISIBLE);

    INT X = 6;
    INT Y = 8;
    INT W = 16;
    INT H = 16;
    for (INT i = 0; i < 13; i++)
    {
        if (i >= 0 && i < 9)
        {
            if (i % 3 == 0)
            {
                X = 6;
                if (i != 0)
                    Y += 18;
            }
            else
                X += 18;
        }
        else if (i == 9)
        {
            X = 6;
            Y += 18;
            W = 16;
        }
        else if (i == 10)
        {
            Y = 8;
            X = 60;
            H = 34;
            W = 16;
        }
        else if (i == 11)
        {
            X = 24;
            Y = 62;
            W = 34;
            H = 16;
        }
        else if (i == 12)
        {
            Y = 44;
            X = 60;
            H = 34;
            W = 16;
        }
        Keypad[i] = new NCButton(GKeypadTextures[i][0], GKeypadTextures[i][1], 0);
        Keypad[i]->ID = NID_DIALOG_KEYPAD;
        Keypad[i]->NotifyID = i;
        Keypad[i]->SetWindowRect(X + 205, Y, W, H);
        CreateChild(Keypad[i], NWS_VISIBLE);
    }

    Progress = new NCProgressCtrl();
    Progress->ID = NID_DIALOG_PROGRESS;
    Progress->SetWindowRect(20, 3, 168, 6);
    CreateChild(Progress, NWS_VISIBLE);
    Progress->SetMax(10000);
    Progress->Reset();

    OKButton = new NCButton(TEXT("L2UI.Control.DefaultButton"), TEXT("L2UI.Control.DefaultButton_click"), 0);
    OKButton->ID = NID_DIALOG_OK;
    OKButton->SetWindowRect(20, 65, 67, 19);
    CreateChild(OKButton, NWS_VISIBLE);

    CancelButton = new NCButton(TEXT("L2UI.Control.DefaultButton"), TEXT("L2UI.Control.DefaultButton_click"), 0);
    CancelButton->ID = NID_DIALOG_CANCEL;
    CancelButton->SetWindowRect(121, 65, 67, 19);
    CreateChild(CancelButton, NWS_VISIBLE);
    return 1;
}
NW_HOOK_METHOD(0x10007110, NCGaraDialogBox, OnCreate)

// 0x100076f0 (vslot 37) - guarded in C2; NCWnd's OnDestroy at the end (NControl's is not called)
INT NCGaraDialogBox::OnDestroy()
{
    guard(NCGaraDialogBox::OnDestroy);
    for (INT i = 0; i < 13; i++)
    {
        if (Keypad[i])
        {
            RemoveChildWindow(Keypad[i]->Entry);
            delete Keypad[i];
            Keypad[i] = NULL;
        }
    }
    if (OKButton)
    {
        RemoveChildWindow(OKButton->Entry);
        delete OKButton;
        OKButton = NULL;
    }
    if (CancelButton)
    {
        RemoveChildWindow(CancelButton->Entry);
        delete CancelButton;
        CancelButton = NULL;
    }
    if (EditBox)
    {
        RemoveChildWindow(EditBox->Entry);
        delete EditBox;
        EditBox = NULL;
    }
    if (Progress)
    {
        RemoveChildWindow(Progress->Entry);
        delete Progress;
        Progress = NULL;
    }
    NCWnd::OnDestroy();
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x100076f0, NCGaraDialogBox, OnDestroy)

// 0x10007560 (vslot 55) - the box (205 wide; with an edit line the keypad's panel, 82 wide, beside
// it) and the message at (28, 12), its escaped newlines made real; the clip goes on before the origin
INT NCGaraDialogBox::OnPaint(NCGDevice* Device)
{
    guard(NCGaraDialogBox::OnPaint);
    Device->SetClip(ScreenX, ScreenY, Width, Height);
    Device->BeginPaint(ScreenX, ScreenY);
    if (bInput == 0)
    {
        if (BackTex)
            Device->DrawTexture(0, 0, 205, Height, 0, 0, 205, Height, BackTex);
    }
    else
    {
        if (InputBackTex)
            Device->DrawTexture(0, 0, 205, Height, 0, 0, 205, Height, InputBackTex);
        if (KeypadTex)
            Device->DrawTexture(205, 0, 82, Height, 0, 0, 82, Height, KeypadTex);
    }
    FString Text;
    GConsole->ConvertNewLines((TCHAR*)*Message, Text);
    Device->DrawText(28, 12, 0xFFFFFFFF, *Text, 168, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0, 0);
    Device->EndPaint();
    Device->ReleaseClip();
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x10007560, NCGaraDialogBox, OnPaint)

/*-----------------------------------------------------------------------------
    The state: Reset, SetInfo, the buttons' texts, the layout.
-----------------------------------------------------------------------------*/

// 0x10006740 - ParamText and the params are kept
void NCGaraDialogBox::Reset()
{
    DialogID = -1;
    bInput = 0;
    Owner = NULL;
    Message.Empty();
    Unknown238 = 0;
    bProgress = 0;
    EditBox->ClearText();
}
NW_HOOK_METHOD(0x10006740, NCGaraDialogBox, Reset)

// 0x10006c50
void NCGaraDialogBox::SetButtonTexts(INT OKTextID, INT CancelTextID)
{
    if (OKButton)
    {
        if (*SysString(OKTextID) != NULL)
            OKButton->SetTextID(OKTextID);
    }
    if (CancelButton)
    {
        if (*SysString(CancelTextID) != NULL)
            CancelButton->SetTextID(CancelTextID);
    }
}
NW_HOOK_METHOD(0x10006c50, NCGaraDialogBox, SetButtonTexts)

// 0x10006cf0
NWndEntry* NCGaraDialogBox::GetOwner()
{
    return Owner;
}
NW_HOOK_METHOD(0x10006cf0, NCGaraDialogBox, GetOwner)

// 0x10006cc0 (ret 4)
void NCGaraDialogBox::SetEditText(const TCHAR* Text)
{
    if (bInput && Text)
        EditBox->InsertText(Text, 1);
}
NW_HOOK_METHOD(0x10006cc0, NCGaraDialogBox, SetEditText)

// 0x10007860 - 203 x 87 (the box with a digits line: 287 wide, with the keypad); the buttons at (27, 60)
// and (114, 60), a notice's OK alone and centered
void NCGaraDialogBox::UpdateLayout()
{
    SetSize(203, 87);
    // the stock code fills { LocalX, LocalY, LocalW, LocalH }; SetScreenPos reads x and y
    INT Origin[4];
    Origin[0] = LocalX;
    Origin[1] = LocalY;
    Origin[2] = LocalW;
    Origin[3] = LocalH;
    OKButton->SetScreenPos(27, 60, (const POINT*)Origin);
    CancelButton->SetScreenPos(114, 60, (const POINT*)Origin);
    if (bInput == 0)
    {
        SetSize(203, 87);
        EditBox->HideWindow();
        for (INT i = 0; i < 13; i++)
            Keypad[i]->HideWindow();
    }
    else if (Type == 3)
        SetSize(287, 87);
    else
    {
        SetSize(203, 87);
        for (INT i = 0; i < 13; i++)
            Keypad[i]->HideWindow();
    }
    if (DialogID == 0x1D || DialogID == 0x1F || DialogID == 0x10)
    {
        CancelButton->HideWindow();
        OKButton->SetScreenPos(((DWORD)Width >> 1) - 27, 60, (const POINT*)Origin);
    }
    if (bProgress == 0)
        Progress->HideWindow();
    else
    {
        Progress->ShowWindow();
        if (DialogID != 0x13 && DialogID != 0x34)
            Progress->SetMax(10000);
        else
            Progress->SetMax(900000);
    }
}
NW_HOOK_METHOD(0x10007860, NCGaraDialogBox, UpdateLayout)

// 0x10007a00 - Text2 wins over Text; an empty ParamText or a zero param keeps the box's; the 12 bytes
// after the NDialogInfo's own 0x78 (and its +0x74) come along
void NCGaraDialogBox::SetInfo(NDialogInfo* Info)
{
    Reset();
    DialogID = Info->ID;
    Type = Info->Type;
    EditBox->SetType(Info->Type);
    bInput = Info->bInput;
    bLargeAmount = Info->bLargeAmount;
    Owner = Info->Owner;
    bProgress = Info->bProgress;
    if (bProgress == 0)
        Progress->HideWindow();
    else
    {
        Progress->ShowWindow();
        Progress->Reset();
        Progress->Start();
    }
    if (Info->Text2.Num() == 0)
    {
        if (Info->Text.Num() != 0)
            Message = Info->Text;
    }
    else
        Message = Info->Text2;
    if (Info->ParamText.Num() != 0)
        ParamText = Info->ParamText;
    if (Info->Param1)
        Param1 = Info->Param1;
    if (Info->Param2)
        Param2 = Info->Param2;
    Unknown238 = Info->Unknown74;
    Unknown22C[0] = nw::Field<INT>(Info, 0x78);
    Unknown22C[1] = nw::Field<INT>(Info, 0x7C);
    Unknown22C[2] = nw::Field<INT>(Info, 0x80);
    if (bInput && Info->Text60.Num() != 0)
        SetEditText(*Info->Text60);
}
NW_HOOK_METHOD(0x10007a00, NCGaraDialogBox, SetInfo)

/*-----------------------------------------------------------------------------
    The answers. OnOK: the amount typed is read first (0 .. 1,000,000, or 100,000,000 with
    bLargeAmount; 1 without an edit line or a text), then DialogID picks what the box does. The ids
    are C2's (C1's from 0x24 on moved up by four, with new ones in between).
-----------------------------------------------------------------------------*/

// the amount typed, as the stock code reads it at the top of OnOK and again in some cases
#define GARA_AMOUNT(Amount)                                                                    \
    {                                                                                          \
        Amount = NWtol(EditBox->CommitText());                                                 \
        INT AmountMax = (bLargeAmount ? 99000000 : 0) + 1000000;                               \
        if (Amount < 0)                                                                        \
            Amount = 0;                                                                        \
        else if (Amount > AmountMax)                                                           \
            Amount = AmountMax;                                                                \
    }

// a request with one argument, the text typed
#define TEXT_REQUEST(Request)                                                                  \
    {                                                                                          \
        L2ParamStack Params(10);                                                               \
        Params.PushBack((void*)EditBox->CommitText());                                         \
        GConsole->Request(Params);                                                             \
    }
// a request with one number
#define VALUE_REQUEST(Request, Value)                                                          \
    {                                                                                          \
        L2ParamStack Params(10);                                                               \
        Params.PushBack((void*)(INT_PTR)(Value));                                              \
        GConsole->Request(Params);                                                             \
    }
// a request with ParamText and a number
#define NAMED_REQUEST(Request, Value)                                                          \
    {                                                                                          \
        L2ParamStack Params(10);                                                               \
        Params.PushBack((void*)*ParamText);                                                    \
        Params.PushBack((void*)(INT_PTR)(Value));                                              \
        GConsole->Request(Params);                                                             \
    }
// a request with Param1 and two flags
#define SIEGE_REQUEST(Request, A, B, C)                                                        \
    {                                                                                          \
        L2ParamStack Params(10);                                                               \
        Params.PushBack((void*)(INT_PTR)(A));                                                  \
        Params.PushBack((void*)(INT_PTR)(B));                                                  \
        Params.PushBack((void*)(INT_PTR)(C));                                                  \
        GConsole->Request(Params);                                                             \
    }

// the name typed as the stock code measures it: bytes in the ANSI code page
static FORCEINLINE INT AnsiLength(const TCHAR* Text)
{
    ANSICHAR Ansi[512];
    WideCharToMultiByte(CP_ACP, 0, Text, -1, Ansi, 512, NULL, NULL);
    return (INT)strlen(Ansi);
}

// 0x10007b20 (vslot 68) - OK. The box closes afterwards unless noted.
INT NCGaraDialogBox::OnOK()
{
    if (bProgress)
        Progress->Stop();
    INT Amount = 1;
    if (bInput && EditBox->CommitText())
        GARA_AMOUNT(Amount)
    switch (DialogID)
    {
    case 0: // yes to the owner
    case 1:
        if (Owner)
            SendTo(Owner, NWM_DIALOG_RESULT, 1, 0);
        break;
    case 2: // amounts for the owner (items, adena...); without an edit line, yes; the id comes along
    case 3:
    case 8:
    case 0x1E:
    case 0x23:
        if (Owner)
        {
            if (bInput == 0)
                SendTo(Owner, NWM_DIALOG_RESULT, 1, DialogID);
            else if (EditBox->CommitText())
            {
                INT Typed;
                GARA_AMOUNT(Typed)
                SendTo(Owner, NWM_DIALOG_RESULT, Typed, DialogID);
            }
        }
        break;
    case 4: // drop an item where the pawn stands (unless that is forbidden)
    {
        NItemRequest Request;
        Request.ObjectID = Param1;
        Request.Count = Amount;
        if ((GConsole->GetMyUser()->Pawn->Unknown690 & 1) == 0)
            GConsole->RequestDropItem(Request, *(FVector*)Unknown22C);
        else
            GConsole->GetDesktop()->AddSystemText(*GL2GameData.SystemMsg[SYSMSG_ItemFlying].Message, 0, (DWORD)-1);
        break;
    }
    case 5: // destroy an item
    {
        NItemRequest Request;
        Request.ObjectID = Param1;
        Request.Count = Amount;
        GConsole->RequestDestroyItem(Request);
        GConsole->PlayInterfaceSound(SOUND_TrashBasket);
        break;
    }
    case 6: // crystallize an item
        GConsole->RequestCrystallizeItem(Param1, Amount);
        GConsole->PlayInterfaceSound(SOUND_TrashBasket);
        break;
    case 7: // trade request
        VALUE_REQUEST(AnswerTradeRequest, 1)
        break;
    case 0xA: // party invitation
        VALUE_REQUEST(RequestAnswerJoinParty, 1)
        break;
    case 0xB:
        TEXT_REQUEST(RequestNet103)
        break;
    case 0xC:
        GConsole->RequestNet106();
        break;
    case 0xE: // clan invitation
        VALUE_REQUEST(RequestAnswerJoinPledge, 1)
        break;
    case 0xF: // title for the selected clan member
    {
        User* Target = GConsole->GetSelectedActor();
        if (!Target || Target->Unknown8)
            break;
        INT Len = AnsiLength(EditBox->CommitText());
        if (!GConsole->HasInvalidChars((TCHAR*)EditBox->CommitText()) &&
            GConsole->IsValidName((TCHAR*)EditBox->CommitText()))
        {
            if (Len > 16)
            {
                GConsole->ShowSystemMessage(80, COLOR_Warning);
                EditBox->SetFocus();
                return 1;
            }
            L2ParamStack Params(10);
            Params.PushBack(Target->GetName());
            Params.PushBack((void*)EditBox->CommitText());
            GConsole->RequestGiveNickName(Params);
            break;
        }
        GConsole->ShowSystemMessage(204, COLOR_Warning);
        EditBox->ClearText();
        EditBox->SetFocus();
        return 1;
    }
    case 0x10: // clan crest notice: then the file name (dialog 0x11)
    {
        NDialogInfo Info;
        Info.Owner = Entry;
        Info.ID = 0x11;
        Info.bMainBox = 1;
        Info.bInput = 1;
        Info.Type = 0;
        Info.SetSystemMessage(208);
        GConsole->ShowDialog(&Info);
        return 1;
    }
    case 0x11:
        TEXT_REQUEST(RegisterPledgeCrest)
        break;
    case 0x12: // clan war: declare, answer, stop, answer, surrender
        TEXT_REQUEST(RequestStartPledgeWar)
        break;
    case 0x13:
        NAMED_REQUEST(RequestReplyStartPledgeWar, 1)
        break;
    case 0x14:
        TEXT_REQUEST(RequestStopPledgeWar)
        break;
    case 0x15:
        NAMED_REQUEST(RequestReplyStopPledgeWar, 1)
        break;
    case 0x17:
        TEXT_REQUEST(RequestSurrenderPledgeWar)
        break;
    case 0x18: // expel a member
    {
        L2ParamStack Params(10);
        Params.PushBack((void*)*ParamText);
        GConsole->RequestOustPledgeMember(Params);
        break;
    }
    case 0x19:
        TEXT_REQUEST(RequestSurrenderPersonally)
        break;
    case 0x1A: // leave the clan
        GConsole->RequestWithdrawalPledge();
        break;
    case 0x1B: // restart / quit (the system menu)
        if (Owner)
            SendTo(Owner, NWM_DIALOG_RESULT, 0x1B, 0);
        break;
    case 0x1C:
        if (Owner)
            SendTo(Owner, NWM_DIALOG_RESULT, 0x1C, 0);
        break;
    case 0x1D: // disconnected: the game ends
        GConsole->QuitGame();
        break;
    case 0x20: // revival
        GConsole->AnswerReviveRequest(1, Param1, Param2);
        break;
    case 0x21: // use an item
        GConsole->RequestUseItem(Param1);
        break;
    case 0x22: // delete a recipe
        GConsole->RequestRecipeItemDelete(Param1);
        break;
    case 0x24: // the recipe shop's message
        if (EditBox->CommitText())
        {
            const TCHAR* Text = EditBox->CommitText();
            if (Text && *Text)
                GConsole->RequestRecipeShopMessageSet((TCHAR*)Text);
        }
        break;
    case 0x25: // private store (sell) amounts; the box stays when the owner refuses
        if (Owner)
        {
            if (bInput == 0)
                SendTo(Owner, 0xF0000006, 1, 0);
            else if (EditBox->CommitText())
            {
                INT Typed;
                GARA_AMOUNT(Typed)
                if (!SendTo(Owner, 0xF0000006, Typed, 0))
                    return 1;
                Reset();
                HideWindow();
                return 1;
            }
        }
        break;
    case 0x26:
        if (Owner)
        {
            if (bInput == 0)
                SendTo(Owner, 0xF0000007, 1, 0);
            else if (EditBox->CommitText())
            {
                INT Typed;
                GARA_AMOUNT(Typed)
                SendTo(Owner, 0xF0000007, Typed, 0);
            }
        }
        break;
    case 0x27: // private store message
        if (EditBox->CommitText())
        {
            const TCHAR* Text = EditBox->CommitText();
            if (Text && *Text)
                GConsole->SetPrivateStoreMsg((TCHAR*)Text);
        }
        break;
    case 0x28:
        if (Owner)
        {
            if (SendTo(Owner, 0xF0000008, 0, 0))
            {
                Reset();
                HideWindow();
            }
            return 1;
        }
        break;
    case 0x29:
        if (Owner)
            SendTo(Owner, 0xF000000A, 0, 0);
        break;
    case 0x2A: // private store (buy) amounts
        if (Owner)
        {
            if (bInput == 0)
                SendTo(Owner, 0xF000000B, 1, 0);
            else if (EditBox->CommitText())
            {
                INT Typed;
                GARA_AMOUNT(Typed)
                if (!SendTo(Owner, 0xF000000B, Typed, 0))
                    return 1;
                Reset();
                HideWindow();
                return 1;
            }
        }
        break;
    case 0x2B:
        if (Owner)
        {
            if (bInput == 0)
                SendTo(Owner, 0xF000000C, 1, 0);
            else if (EditBox->CommitText())
            {
                INT Typed;
                GARA_AMOUNT(Typed)
                SendTo(Owner, 0xF000000C, Typed, 0);
            }
        }
        break;
    case 0x2C: // private store (buy) message
        if (EditBox->CommitText())
        {
            const TCHAR* Text = EditBox->CommitText();
            if (Text && *Text)
                GConsole->SetPrivateStoreBuyMsg((TCHAR*)Text);
        }
        break;
    case 0x2D:
        if (Owner)
        {
            if (!SendTo(Owner, 0xF000000D, 0, 0))
                return 1;
            Reset();
            HideWindow();
            return 1;
        }
        break;
    case 0x2E:
        if (Owner)
            SendTo(Owner, 0xF000000F, 0, 0);
        break;
    case 0x2F: // alliance: crest, dismiss, leave, expel a clan
        if (EditBox->CommitText())
            TEXT_REQUEST(RegisterAllyCrest)
        break;
    case 0x30:
        GConsole->RequestDismissAlly();
        break;
    case 0x31:
        GConsole->RequestWithdrawAlly();
        break;
    case 0x32:
        if (EditBox->CommitText())
            TEXT_REQUEST(RequestOustAlly)
        break;
    case 0x33: // alliance invitation
        VALUE_REQUEST(RequestAnswerJoinAlly, 1)
        break;
    case 0x34: // alliance war: start / stop asked by the other side
        NAMED_REQUEST(RequestReplyStartAllianceWar, 1)
        break;
    case 0x35:
        NAMED_REQUEST(RequestReplyStopAllianceWar, 1)
        break;
    case 0x36: // give an item to the pet / take one back
    {
        L2ParamStack Params(10);
        Params.PushBack((void*)(INT_PTR)Param1);
        Params.PushBack((void*)(INT_PTR)Amount);
        GConsole->RequestGiveItemToPet(Params);
        break;
    }
    case 0x37:
    {
        L2ParamStack Params(10);
        Params.PushBack((void*)(INT_PTR)Param1);
        Params.PushBack((void*)(INT_PTR)Amount);
        Params.PushBack((void*)0);
        GConsole->RequestGetItemFromPet(Params);
        break;
    }
    case 0x38: // drop an item where the cursor points (unless that is forbidden)
    {
        NItemRequest Request;
        Request.ObjectID = Param1;
        Request.Count = Amount;
        if ((GConsole->GetMyUser()->Pawn->Unknown690 & 1) == 0)
            GConsole->RequestDropItem(Request, ((UGameEngine*)GConsole->Engine)->MouseHitLocation);
        else
            GConsole->GetDesktop()->AddSystemText(*GL2GameData.SystemMsg[SYSMSG_ItemFlying].Message, 0, (DWORD)-1);
        break;
    }
    case 0x39: // pet name
    {
        INT Len = AnsiLength(EditBox->CommitText());
        if (!GConsole->HasInvalidChars((TCHAR*)EditBox->CommitText()) &&
            GConsole->IsValidName((TCHAR*)EditBox->CommitText()))
        {
            if (Len > 16)
            {
                GConsole->ShowSystemMessage(80, COLOR_Warning);
                EditBox->SetFocus();
                return 1;
            }
            GConsole->RequestChangePetName((TCHAR*)EditBox->CommitText());
            break;
        }
        GConsole->ShowSystemMessage(591, COLOR_Warning);
        EditBox->ClearText();
        EditBox->SetFocus();
        return 1;
    }
    case 0x3A: // action 20, with the modifier keys held
    {
        L2ParamStack Params(10);
        Params.PushBack((void*)20);
        Params.PushBack((void*)(INT_PTR)GConsole->IsCtrlDown());
        Params.PushBack((void*)(INT_PTR)GConsole->Engine->Client->Viewports(0)->Input->KeyDown(IK_Shift));
        GConsole->RequestActionUse(Params);
        break;
    }
    case 0x3B: // friend request
        VALUE_REQUEST(RequestAnswerFriendInvite, 1)
        break;
    case 0x3C:
        if (Owner)
            SendTo(Owner, 0xF0000010, 0, 0);
        break;
    case 0x3D:
        if (Owner)
            SendTo(Owner, 0xF0000011, 0, 0);
        break;
    case 0x3E: // siege: register / cancel as attacker / defender (castle in Param1)
        SIEGE_REQUEST(RequestJoinSiege, Param1, 1, 1)
        break;
    case 0x3F:
        SIEGE_REQUEST(RequestJoinSiege, Param1, 1, 0)
        break;
    case 0x40:
        SIEGE_REQUEST(RequestJoinSiege, Param1, 0, 1)
        break;
    case 0x41:
        SIEGE_REQUEST(RequestJoinSiege, Param1, 0, 0)
        break;
    case 0x42: // siege: approve / refuse a defending clan
        SIEGE_REQUEST(RequestConfirmSiegeWaitingList, Param1, Param2, 1)
        break;
    case 0x43:
        SIEGE_REQUEST(RequestConfirmSiegeWaitingList, Param1, Param2, 0)
        break;
    case 0x44:
        if (Owner)
            SendTo(Owner, NWM_DIALOG_RESULT, 0, 0);
        break;
    case 0x45:
        GConsole->RequestUseItem(Param1);
        break;
    }
    Reset();
    HideWindow();
    if (EditBox)
        EditBox->KillFocus();
    KillFocus();
    return 1;
}
NW_HOOK_METHOD(0x10007b20, NCGaraDialogBox, OnOK)

// 0x10006860 (vslot 69) - Cancel: the refusals; the box closes
INT NCGaraDialogBox::OnCancel()
{
    if (bProgress)
        Progress->Stop();
    if (EditBox)
        EditBox->KillFocus();
    switch (DialogID)
    {
    case 1:
        if (Owner)
            SendTo(Owner, NWM_DIALOG_RESULT, 0, 0);
        break;
    case 7:
        VALUE_REQUEST(AnswerTradeRequest, 0)
        break;
    case 0xA:
        VALUE_REQUEST(RequestAnswerJoinParty, 0)
        break;
    case 0xE:
        VALUE_REQUEST(RequestAnswerJoinPledge, 0)
        break;
    case 0x13:
        NAMED_REQUEST(RequestReplyStartPledgeWar, 0)
        break;
    case 0x15:
        VALUE_REQUEST(RequestReplyStopPledgeWar, 0)   // no clan name, unlike OK
        break;
    case 0x20:
        GConsole->AnswerReviveRequest(0, Param1, Param2);
        break;
    case 0x29:
        if (Owner)
            Owner->Wnd->HideWindow();
        break;
    case 0x33:
        VALUE_REQUEST(RequestAnswerJoinAlly, 0)
        break;
    case 0x34:
        NAMED_REQUEST(RequestReplyStartAllianceWar, 0)
        break;
    case 0x35:
        NAMED_REQUEST(RequestReplyStopAllianceWar, 0)
        break;
    case 0x3B:
        VALUE_REQUEST(RequestAnswerFriendInvite, 0)
        // falls through: the owner is told too
    case 0x28:
        if (Owner)
            SendTo(Owner, 0xF0000009, 0, 0);
        break;
    }
    Reset();
    HideWindow();
    KillFocus();
    return 1;
}
NW_HOOK_METHOD(0x10006860, NCGaraDialogBox, OnCancel)

#undef GARA_AMOUNT
#undef TEXT_REQUEST
#undef VALUE_REQUEST
#undef NAMED_REQUEST
#undef SIEGE_REQUEST

NW_CHECK_VTABLE(NCGaraDialogBox, 0x10109df0, 73)
