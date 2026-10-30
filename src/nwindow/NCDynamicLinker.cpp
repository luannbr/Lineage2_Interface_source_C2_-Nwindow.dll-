/*=============================================================================
    NCDynamicLinker.cpp - the link broken across lines (C2 only). Stock: 0x1003a440 .. 0x1003a4b1,
    0x1003ad20 .. 0x1003ae3f (constructor, destructors) and 0x1003af30 .. 0x1003b1d1 (OnCreate,
    OnDestroy).
=============================================================================*/
#include "nwindow/NCDynamicLinker.h"
#include "nwindow/NCActionLinker.h"
#include "nwindow/NCGDevice.h"

enum { NID_DYNAMICLINK_PIECE = (INT)0xFF00FF04 };   // the pieces' id (name ours)

// 0x1003ad20 - NControl's constructor is inlined in the stock code
NCDynamicLinker::NCDynamicLinker()
{
    Unknown1A4 = 0;
    Unknown1A8 = 0;
    Unknown1AC = 0;
    Unknown1B0 = 0;
    Unknown1B4 = 0;
    Unknown1B8 = 0;
    Unknown1BC = 0;
    Unknown1C0 = 0;
    Unknown1C4 = 0;
    Unknown1C8 = 0;
    Action = 0;
    Message = NULL;
    Links[0] = NULL;
    Links[1] = NULL;
    Links[2] = NULL;
    LinkCount = 0;
}
NW_HOOK_CTOR(0x1003ad20, NCDynamicLinker)

// 0x1003adf0 - the destructor (its scalar deleting destructor, 0x1003adc0, is slot 3 and calls it; the C1 to C2 map had
// put this hook on NCHtmlButton, whose destructor is 0x1003deb0)
NW_HOOK_DTOR(0x1003adf0, NCDynamicLinker)

// 0x1003ae40 (ret 0x30) - the HTML viewer's: the geometry and the code come as arguments
NCDynamicLinker::NCDynamicLinker(INT A1, INT A2, INT A3, INT A4, INT A5, INT A6, INT A7, INT A8, INT A9, INT A10,
                                 INT InAction, const TCHAR* InMessage)
{
    Unknown1A8 = 0;
    Unknown1A4 = 0;
    Unknown1B0 = 0;
    Unknown1AC = 0;
    Unknown1B8 = 0;
    Unknown1B4 = 0;
    Unknown1A4 = A1;
    Unknown1A8 = A2;
    Unknown1AC = A3;
    Unknown1B0 = A4;
    Unknown1B4 = A9;
    Unknown1B8 = A10;
    Unknown1BC = A5;
    Unknown1C0 = A6;
    Unknown1C4 = A7;
    Unknown1C8 = A8;
    Links[0] = NULL;
    Links[1] = NULL;
    Links[2] = NULL;
    LinkCount = 0;
    Action = InAction;
    Message = InMessage;
}
NW_HOOK_CTOR_ARGS(0x1003ae40, NCDynamicLinker,
                  (INT A1, INT A2, INT A3, INT A4, INT A5, INT A6, INT A7, INT A8, INT A9, INT A10, INT InAction, const TCHAR* InMessage),
                  (A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, InAction, InMessage))

// 0x1003a440 (vslot 25)
INT NCDynamicLinker::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (NControl::WindowProc(Msg, wParam, lParam))
        return 1;
    if (Msg == NWM_COMMAND && (INT)wParam == NID_DYNAMICLINK_PIECE)
        return OnLink(lParam) != 0;
    return 0;
}
NW_HOOK_METHOD(0x1003a440, NCDynamicLinker, WindowProc)

// 0x1003a4a0 (vslot 68) - a piece's code goes to the parent
INT NCDynamicLinker::OnLink(INT Code)
{
    NotifyParent(Code);
    return 1;
}
NW_HOOK_METHOD(0x1003a4a0, NCDynamicLinker, OnLink)

// 0x1003af30 (vslot 36) - one piece when the link's height is up to a line (a line is the height of an
// 'A' plus 3), two up to two lines, else three; the pieces cover the first line from the link's left,
// the lines between, and the last line (placement from the geometry fields, whose names are not known)
INT NCDynamicLinker::OnCreate()
{
    if (Action)
    {
        SIZE Size;
        NCGDevice::GetCharSize(&Size, 'A', 0);
        INT Step = Size.cy + 3;
        UINT Count;
        if (Unknown1C0 <= Step * 2)
            Count = (Step < Unknown1C0) + 1;
        else
            Count = 3;
        INT i = 0;
        if (Count != 0)
        {
            do
            {
                Links[i] = new NCActionLinker(Action);
                Links[i]->ID = NID_DYNAMICLINK_PIECE;
                INT H = Size.cy;
                if (i == 0)
                {
                    if (Count == 1)
                        Links[i]->SetWindowRect(0, 0, (Unknown1BC - Unknown1A4) + Unknown1B4 + Unknown1AC, H);
                    else if (Count >= 2)
                        Links[i]->SetWindowRect(0, 0, (Unknown1C4 - Unknown1A4) + Unknown1B4 + Unknown1AC, H);
                }
                else if (i == 1)
                {
                    INT W;
                    if (Count == 2)
                        W = Unknown1C8;
                    else if (Count == 3)
                    {
                        H = Unknown1C0 + Step * -2;
                        W = Unknown1C4;
                    }
                    if (Count == 2 || Count == 3)
                        Links[i]->SetWindowRect(Unknown1B4 - Unknown1A4, Size.cy + 3, W, H);
                }
                else if (i == 2)
                    Links[i]->SetWindowRect(Unknown1B4 - Unknown1A4, Unknown1C0 - Size.cy, Unknown1C8, H);
                CreateChild(Links[i], 0x12);
                if (Message)
                    appStrcpy(Links[i]->Message, Message);
                i++;
                LinkCount++;
            } while (i < (INT)Count);
        }
    }
    return 1;
}
NW_HOOK_METHOD(0x1003af30, NCDynamicLinker, OnCreate)

// 0x1003b150 (vslot 37) - guarded in C2; the pieces go, the base's OnDestroy is not called
INT NCDynamicLinker::OnDestroy()
{
    guard(NCDynamicLinker::OnDestroy);
    for (INT i = 0; i < LinkCount; i++)
    {
        if (Links[i])
        {
            RemoveChildWindow(Links[i]->Entry);
            delete Links[i];
            Links[i] = NULL;
        }
    }
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x1003b150, NCDynamicLinker, OnDestroy)

NW_CHECK_VTABLE(NCDynamicLinker, 0x10110a70, 69)
