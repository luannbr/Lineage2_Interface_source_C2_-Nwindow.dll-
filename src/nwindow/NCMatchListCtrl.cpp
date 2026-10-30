/*=============================================================================
    NCMatchListCtrl.cpp - the party matching list. Stock: 0x10073dc0 .. 0x100744de,
    0x10075000 .. 0x10075092, 0x100750a0 .. 0x10075107, 0x10075c90 .. 0x10075e97,
    0x10075ea0 .. 0x10075f67.
=============================================================================*/
#include "nwindow/NCMatchListCtrl.h"
#include "nwindow/NCGDevice.h"
#include "nwindow/NConsoleWnd.h"

#include <wchar.h>

// sysstring lines
enum
{
    STR_Cls = 391,
    STR_Lv = 392,
    STR_NotOpen = 411,
};

enum { MATCH_TEXT_COLOR = 0xFFB09B79, MATCH_LABEL_COLOR = 0xFFA3A3A3, MATCH_MEMO_COLOR = 0xFFB2BECF };

// NWM_COMMAND id the window procedure sorts on; no control sends it (the headers report
// NID_LISTCTRL_HEADER)
enum { NID_MATCHLIST_SORT = 0xF7 };

// 0x100960d0
NCMatchListCtrl::NCMatchListCtrl()
{
}
NW_HOOK_CTOR(0x100960d0, NCMatchListCtrl)

// 0x10096170
NCMatchListCtrl::NCMatchListCtrl(INT InMaxRows, INT InVisibleRows, INT InMaxColumns, INT Unused)
    : NCListCtrl(InMaxRows, InVisibleRows, InMaxColumns, Unused)
{
}
NW_HOOK_CTOR_ARGS(0x10096170, NCMatchListCtrl, (INT A, INT B, INT C, INT D), (A, B, C, D))

// 0x10096120
NW_HOOK_DTOR(0x10096120, NCMatchListCtrl)

// 0x10094660 (vslot 25) - C2 does not test IsEnabled first
INT NCMatchListCtrl::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (NCListCtrl::WindowProc(Msg, wParam, lParam))
        return 1;
    if (Msg == NWM_COMMAND && (INT)wParam == NID_MATCHLIST_SORT)
        return SortByColumn((INT)lParam) != 0;
    return 0;
}
NW_HOOK_METHOD(0x10094660, NCMatchListCtrl, WindowProc)

// C2 CANDIDATE: the C2 function is not the same code as the C1 one; kept only while it matches the stock one (verify_data)
// 0x100952b0 (vslot 61) - the owner gets the row; without a tool tip yet, the server is
// asked for the player's details (HoverRow is left alone, unlike NCListCtrl's)
INT NCMatchListCtrl::OnLButtonDown(WPARAM wParam, LPARAM lParam)
{
    Selected = HitTest(LOWORD(lParam), HIWORD(lParam));
    if (Selected >= 0 && Selected < Rows.Num())
    {
        NotifyParent(Selected);
        NListRow* Row = Rows(Selected);
        if (Row)   // C2: the details are asked for at every click (C1: only without a tool tip yet)
            GConsole->RequestPartyMatchDetail(GetSelectedPlayerID());
    }
    return 1;
}
NW_HOOK_METHOD(0x100952b0, NCMatchListCtrl, OnLButtonDown)

// 0x10095aa0 (vslot 73) - NCListCtrl's bubble sort, but class and level (columns 0 and 1)
// sort by the key in Param1, compared unsigned, and rows without the icon stay in place
INT NCMatchListCtrl::SortByColumn(INT Column)
{
    NListRow* SelRow = GetSelectedRow();
    for (INT i = 0; i < Rows.Num(); i++)
    {
        for (INT j = 0; j < Rows.Num() - i && j < Rows.Num() - 1; j++)
        {
            UBOOL bSwap;
            if (Column == 0 || Column == 1)
            {
                NListCell* A = &Rows(j)->Cells[Column];
                if (!A->Icon)
                    continue;
                NListCell* B = &Rows(j + 1)->Cells[Column];
                if (!B->Icon)
                    continue;
                if (Columns(Column)->bAscending)
                    bSwap = (DWORD)A->Param1 > (DWORD)B->Param1;
                else
                    bSwap = (DWORD)A->Param1 < (DWORD)B->Param1;
            }
            else
            {
                const TCHAR* A = Rows(j)->Cells[Column].Text;
                if (!*A)
                    continue;
                const TCHAR* B = Rows(j + 1)->Cells[Column].Text;
                if (!*B)
                    continue;
                UBOOL bAscending = Columns(Column)->bAscending;
                INT Result = _wcsicmp(A, B);
                bSwap = bAscending ? Result > 0 : Result < 0;
            }
            if (bSwap)
            {
                NListRow* Temp = Rows(j);
                Rows(j) = Rows(j + 1);
                Rows(j + 1) = Temp;
            }
        }
    }
    Columns(Column)->bAscending = !Columns(Column)->bAscending;
    if (SelRow)
    {
        for (Selected = 0; Selected < Rows.Num(); Selected++)
        {
            if (Rows(Selected) == SelRow)
                break;
        }
    }
    return 1;
}
NW_HOOK_METHOD(0x10095aa0, NCMatchListCtrl, SortByColumn)

// 0x10073f90  NOT PORTED (no C2 counterpart)
void NCMatchListCtrl::DrawRowToolTip(NCGDevice* Device, NListRow* Row, INT X, INT Y)
{
    NW_UNPORTED_NORET();
}

// 0x10073e30  NOT PORTED (no C2 counterpart)
void NCMatchListCtrl::BuildToolTip(NListRow* Row)
{
    NW_UNPORTED_NORET();
}

// 0x10095210 - the object ID in the name cell
INT NCMatchListCtrl::GetRowPlayerID(INT Index)
{
    if (Index >= 0 && Index < Rows.Num() && Rows(Index))
    {
        INT ID = Rows(Index)->Cells[2].Param1;
        if (ID != 0)
            return ID;
    }
    return -1;
}
NW_HOOK_METHOD(0x10095210, NCMatchListCtrl, GetRowPlayerID)

// 0x10095240
INT NCMatchListCtrl::GetSelectedPlayerID()
{
    if (Selected >= 0 && Selected < Rows.Num() && Rows(Selected))
    {
        INT ID = Rows(Selected)->Cells[2].Param1;
        if (ID != 0)
            return ID;
    }
    return -1;
}
NW_HOOK_METHOD(0x10095240, NCMatchListCtrl, GetSelectedPlayerID)

// 0x10095270
const TCHAR* NCMatchListCtrl::GetSelectedName()
{
    if (Selected >= 0 && Selected < Rows.Num() && Rows(Selected))
    {
        const TCHAR* Name = Rows(Selected)->Cells[2].Text;
        if (Name && *Name)
            return Name;
    }
    return NULL;
}
NW_HOOK_METHOD(0x10095270, NCMatchListCtrl, GetSelectedName)

NW_CHECK_VTABLE(NCMatchListCtrl, 0x1011c3d0, 74)
