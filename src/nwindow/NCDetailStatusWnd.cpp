/*=============================================================================
    NCDetailStatusWnd.cpp - the character tab of the status window (Alt+T) and the GM's view
    of another player's status.
    Stock: 0x1005be50 .. 0x1005be67, 0x1005c5b0 .. 0x1005c5ff, 0x1005cdb0 .. 0x1005cdfd,
    0x1005ce30 .. 0x1005e30b, 0x100aea90 .. 0x100aeb64, 0x100b1be0 .. 0x100b32fa.
=============================================================================*/
#include "nwindow/NCDetailStatusWnd.h"
#include "nwindow/NCFrameCtrl.h"
#include "nwindow/NCGDevice.h"
#include "nwindow/NConsoleWnd.h"

#include <stdio.h>
#include <wchar.h>

// exp at the start of each level (index = level), a table of the stock image, and the GM
// window's own copy of it, from level 1 (index = level - 1); same values
#if NW_HOOK_MODE
#define GExpTable (reinterpret_cast<const INT*>(::nw::Addr(0x10125f44)))
#define GGMExpTable (reinterpret_cast<const INT*>(::nw::Addr(0x10116750)))
#else
extern const INT GExpTable[];
extern const INT GGMExpTable[];
#endif

// every text of the window goes out with the same trailing arguments
static FORCEINLINE void DrawString(NCGDevice* Device, INT X, INT Y, DWORD Color, const TCHAR* Text)
{
    Device->DrawText(X, Y, Color, Text, 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
}

// a number centered on CenterX in the values' color (spelled out at each use in the stock code)
static FORCEINLINE void DrawNumber(NCGDevice* Device, TCHAR* Buf, INT CenterX, INT Y, INT Value)
{
    SIZE Size;
    appSprintf(Buf, TEXT("%d"), Value);
    NCGDevice::GetTextExtent(&Size, Buf, 0, 0, 0, 0, 0);
    DrawString(Device, CenterX - Size.cx / 2, Y, 0xFFB09B79, Buf);
}

// 0x100d6750 (no C1 match) - the textures other than the first are left as allocated
NCDetailStatusWnd::NCDetailStatusWnd()
{
    BackTex = NULL;
    for (INT i = 0; i < 17; i++)
        UnknownCC[i] = 0;
}
NW_HOOK_CTOR(0x100d6750, NCDetailStatusWnd)

// 0x10073b50
NW_HOOK_DTOR(0x10073b50, NCDetailStatusWnd)

// stock body shared with NControl's (0x10004e90)
INT NCDetailStatusWnd::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (Msg != WM_PAINT && !IsEnabled())
        return 0;
    return NCWnd::WindowProc(Msg, wParam, lParam) != 0;
}

// 0x100d35e0 (vslot 36)
INT NCDetailStatusWnd::OnCreate()
{
    NCWnd::OnCreate();
    BackTex = FindTexture(TEXT("L2UI.StatusWnd.statusback"));
    HPTex = FindTexture(TEXT("L2UI.StatusWnd.statusHP1"));
    MPTex = FindTexture(TEXT("L2UI.StatusWnd.statusMP1"));
    EXPTex = FindTexture(TEXT("L2UI.StatusWnd.statusEXP1"));
    WeightTex[0] = FindTexture(TEXT("L2UI.StatusWnd.statusWt1"));
    WeightTex[1] = FindTexture(TEXT("L2UI.StatusWnd.statusWt2"));
    WeightTex[2] = FindTexture(TEXT("L2UI.StatusWnd.statusWt3"));
    WeightTex[3] = FindTexture(TEXT("L2UI.StatusWnd.statusWt4"));
    SPTex = FindTexture(TEXT("L2UI.StatusWnd.statusSP"));
    NameLeftTex = FindTexture(TEXT("L2UI.StatusWnd.statusName"));
    NameRightTex = FindTexture(TEXT("L2UI.StatusWnd.statusName-1"));
    return 1;
}
NW_HOOK_METHOD(0x100d35e0, NCDetailStatusWnd, OnCreate)

// 0x100d36c0 (vslot 37)
INT NCDetailStatusWnd::OnDestroy()
{
    guard(NCDetailStatusWnd::OnDestroy);
    NCWnd::OnDestroy();
    return 1;
    unguard;
}
NW_HOOK_METHOD(0x100d36c0, NCDetailStatusWnd, OnDestroy)

// stock body shared with NCEditBox's (0x100a5d70)
void NCDetailStatusWnd::ShowWindow()
{
    NCWnd::ShowWindow();
}

// a stat and its bonus (the window's, not the player's) centered on CenterX: "N" without a bonus, "N(-B)" with a
// negative one and the format the caller gives ("%d(+%d)", "%d (+%d)") with a positive one
static FORCEINLINE void DrawStat(NCGDevice* Device, TCHAR* Buf, INT CenterX, INT Y, INT Value, INT Bonus,
                                 const TCHAR* PlusFormat)
{
    SIZE Size;
    if (Bonus > 0)
        appSprintf(Buf, PlusFormat, Value, Bonus);
    else if (Bonus < 0)
        appSprintf(Buf, TEXT("%d(%d)"), Value, Bonus);
    else
        appSprintf(Buf, TEXT("%d"), Value);
    NCGDevice::GetTextExtent(&Size, Buf, 0, 0, 0, 0, 0);
    DrawString(Device, CenterX - Size.cx / 2, Y, 0xFFB09B79, Buf);
}

// 0x100d67d0 (vslot 55) - C2: the labels, bars and values sit higher (the clan at 27, the rows 15 apart), a row of
// two more values (the words at +0x1F4 and +0x1F2 of the player) is at the bottom, the six stats show their
// bonuses ("N(+B)"), and the speed is the swimming one in water. The clip opens before BeginPaint; bars 84 (HP, MP,
// weight) and 189 (exp) pixels wide; title and name centered over the window (each cut to 9 characters and ".."
// when both are wider than 220 pixels together), the name reddened by the karma.
INT NCDetailStatusWnd::OnPaint(NCGDevice* Device)
{
    Device->SetClip(ScreenX, ScreenY, Width, Height);
    Device->BeginPaint(ScreenX, ScreenY);
    if (BackTex)
        Device->DrawTexture(0, 0, 240, 300, 0, 0, 240, 300, BackTex);

    DrawString(Device, 17, 27, 0xFFA3A3A3, *NCObject::SysString(430));    // "Clan"
    DrawString(Device, 17, 42, 0xFFA3A3A3, *NCObject::SysString(88));     // "Lv"
    DrawString(Device, 17, 102, 0xFFFFFFFF, *NCObject::SysString(93));    // "Combat"
    DrawString(Device, 17, 118, 0xFFA3A3A3, *NCObject::SysString(94));    // "P.Atk."
    DrawString(Device, 17, 133, 0xFFA3A3A3, *NCObject::SysString(95));    // "P.Def."
    DrawString(Device, 17, 148, 0xFFA3A3A3, *NCObject::SysString(96));    // "Accuracy"
    DrawString(Device, 17, 163, 0xFFA3A3A3, *NCObject::SysString(113));   // "Critical"
    DrawString(Device, 17, 178, 0xFFA3A3A3, *NCObject::SysString(111));   // "Atk. Spd."
    DrawString(Device, 17, 196, 0xFFFFFFFF, *NCObject::SysString(103));   // "Basic"
    DrawString(Device, 122, 118, 0xFFA3A3A3, *NCObject::SysString(98));   // "M.Atk."
    DrawString(Device, 122, 134, 0xFFA3A3A3, *NCObject::SysString(99));   // "M.Def."
    DrawString(Device, 122, 149, 0xFFA3A3A3, *NCObject::SysString(97));   // "Evasion"
    DrawString(Device, 122, 164, 0xFFA3A3A3, *NCObject::SysString(432));  // "Speed"
    DrawString(Device, 122, 179, 0xFFA3A3A3, *NCObject::SysString(112));  // "Casting Spd."
    DrawString(Device, 17, 213, 0xFFA3A3A3, *NCObject::SysString(104));   // "STR"
    DrawString(Device, 93, 213, 0xFFA3A3A3, *NCObject::SysString(105));   // "DEX"
    DrawString(Device, 163, 213, 0xFFA3A3A3, *NCObject::SysString(106));  // "CON"
    DrawString(Device, 17, 229, 0xFFA3A3A3, *NCObject::SysString(107));   // "INT"
    DrawString(Device, 93, 229, 0xFFA3A3A3, *NCObject::SysString(108));   // "WIT"
    DrawString(Device, 163, 229, 0xFFA3A3A3, *NCObject::SysString(109));  // "MEN"
    DrawString(Device, 17, 248, 0xFFFFFFFF, *NCObject::SysString(100));   // "Social"
    DrawString(Device, 17, 267, 0xFFA3A3A3, *NCObject::SysString(101));   // "Karma"
    DrawString(Device, 122, 267, 0xFFA3A3A3, *NCObject::SysString(102));  // "PvP"
    DrawString(Device, 17, 283, 0xFFA3A3A3, *NCObject::SysString(709));   // new in C2
    DrawString(Device, 122, 283, 0xFFA3A3A3, *NCObject::SysString(710));  // new in C2

    Player = GConsole->GetMyUser();
    if (Player)
    {
        INT HPW = 0;
        INT MPW = 0;
        INT WeightW = 0;
        INT EXPW = 0;
        if (Player->MaxHP)
            HPW = (INT)(Player->HP * 84.0 / Player->MaxHP);
        if (Player->MaxMP)
            MPW = (INT)(Player->MP * 84.0 / Player->MaxMP);
        if (Player->MaxLoad)
            WeightW = (INT)(Player->Load * 84.0 / Player->MaxLoad);
        if (GExpTable[Player->Level + 1])
            EXPW = (INT)((DOUBLE)(Player->Exp - GExpTable[Player->Level]) * 189.0 /
                         (GExpTable[Player->Level + 1] - GExpTable[Player->Level]));

        if (HPW > 0 && HPW <= 84)
            Device->DrawTexture(34, 71, HPW, 10, 0, 0, HPW, 10, HPTex);
        else if (HPW > 84)
            Device->DrawTexture(34, 71, 84, 10, 0, 0, 84, 10, HPTex);
        if (MPW > 0 && MPW <= 84)
            Device->DrawTexture(139, 71, MPW, 10, 0, 0, MPW, 10, MPTex);
        else if (MPW > 84)
            Device->DrawTexture(139, 71, 84, 10, 0, 0, 84, 10, MPTex);
        if (EXPW > 0 && EXPW <= 189)
            Device->DrawTexture(34, 57, EXPW, 10, 0, 0, EXPW, 10, EXPTex);
        else if (EXPW > 189)
            Device->DrawTexture(34, 57, 189, 10, 0, 0, 189, 10, EXPTex);

        FLOAT WeightPct = 0.f;
        if (WeightW > 0)
        {
            // the first test uses the unrounded quotient, the others the stored float
            DOUBLE Pct = (DOUBLE)Player->Load / Player->MaxLoad * 100.0;
            WeightPct = (FLOAT)Pct;
            if (Pct <= 50.0)
                Device->DrawTexture(139, 85, WeightW, 10, 0, 0, WeightW, 10, WeightTex[0]);
            else if (WeightPct <= 66.66)
                Device->DrawTexture(139, 85, WeightW, 10, 0, 0, WeightW, 10, WeightTex[1]);
            else if (WeightPct > 66.66 && WeightPct <= 80.f)
                Device->DrawTexture(139, 85, WeightW, 10, 0, 0, WeightW, 10, WeightTex[2]);
            else if (WeightPct > 80.f)
                Device->DrawTexture(139, 85, WeightW, 10, 0, 0, WeightW, 10, WeightTex[3]);
        }

        TCHAR Buf[64];
        SIZE Size;
        INT LevelExp = GExpTable[Player->Level];
        swprintf(Buf, ARRAY_COUNT(Buf), TEXT("%.2f%s"),
                 (DOUBLE)(Player->Exp - LevelExp) * 100.0 / (GExpTable[Player->Level + 1] - LevelExp),
                 TEXT("%"));
        NCGDevice::GetTextExtent(&Size, Buf, 0, 0, 0, 0, 0);
        Device->DrawSpecialDigit(1, 128 - Size.cx / 2, 58, 0xFFB4B4B4, Buf);

        // "now / max" around a slash centered at 76 (HP) and 181 (MP), in the large digits
        appSprintf(Buf, TEXT("%s"), TEXT("/"));
        NCGDevice::GetTextExtent(&Size, Buf, 0, 0, 0, 0, 0);
        Device->DrawSpecialDigit(1, 76 - Size.cx / 2, 72, 0xFFFFFFFF, Buf);
        appSprintf(Buf, TEXT("%d"), Player->HP);
        NCGDevice::GetTextExtent(&Size, Buf, 0, 0, 0, 0, 0);
        Device->DrawSpecialDigit(1, 66 - Size.cx, 72, 0xFFFFFFFF, Buf);
        appSprintf(Buf, TEXT("%d"), Player->MaxHP);
        NCGDevice::GetTextExtent(&Size, Buf, 0, 0, 0, 0, 0);
        Device->DrawSpecialDigit(1, 82, 72, 0xFFFFFFFF, Buf);
        appSprintf(Buf, TEXT("%s"), TEXT("/"));
        NCGDevice::GetTextExtent(&Size, Buf, 0, 0, 0, 0, 0);
        Device->DrawSpecialDigit(1, 181 - Size.cx / 2, 72, 0xFFFFFFFF, Buf);
        appSprintf(Buf, TEXT("%d"), Player->MP);
        NCGDevice::GetTextExtent(&Size, Buf, 0, 0, 0, 0, 0);
        Device->DrawSpecialDigit(1, 171 - Size.cx, 72, 0xFFFFFFFF, Buf);
        appSprintf(Buf, TEXT("%d"), Player->MaxMP);
        NCGDevice::GetTextExtent(&Size, Buf, 0, 0, 0, 0, 0);
        Device->DrawSpecialDigit(1, 187, 72, 0xFFFFFFFF, Buf);

        DrawNumber(Device, Buf, 76, 85, Player->SP);
        appSprintf(Buf, TEXT("%.2f%s"), WeightPct, TEXT("%"));
        NCGDevice::GetTextExtent(&Size, Buf, 0, 0, 0, 0, 0);
        Device->DrawSpecialDigit(1, 181 - Size.cx / 2, 86, 0xFFFFFFFF, Buf);

        // the name is white without karma; with it, green and blue go down by the karma (at
        // least by 100 + karma / 16, at most by 255: red)
        INT Red = Player->Karma;
        if (Red > 255)
            Red = 255;
        if (Red > 0)
        {
            INT MinRed = Red / 16 + 100;
            if (Red < MinRed)
                Red = MinRed;
            else if (Red >= 255)
                Red = 255;
        }
        DWORD NameColor = 0xFFFF0000 | ((255 - Red) << 8) | (255 - Red);

        // the title first, when there is one; the name between two end pieces
        TCHAR Name[64];
        TCHAR Title[64];
        SIZE NameSize;
        SIZE TitleSize;
        if (Player->GetNickName() && *Player->GetNickName())
        {
            appSprintf(Title, TEXT("%s"), Player->GetNickName());
            NCGDevice::GetTextExtent(&TitleSize, Title, 0, 0, 0, 0, 0);
            appSprintf(Name, TEXT("%s"), Player->GetName());
            NCGDevice::GetTextExtent(&NameSize, Name, 0, 0, 0, 0, 0);
            if (TitleSize.cx + NameSize.cx > 220)
            {
                if (TitleSize.cx > 109)
                {
                    Title[9] = '.';
                    Title[10] = '.';
                    Title[11] = 0;
                    NCGDevice::GetTextExtent(&TitleSize, Title, 0, 0, 0, 0, 0);
                }
                if (NameSize.cx > 109)
                {
                    Name[9] = '.';
                    Name[10] = '.';
                    Name[11] = 0;
                    NCGDevice::GetTextExtent(&NameSize, Name, 0, 0, 0, 0, 0);
                }
            }
            INT X = 120 - (TitleSize.cx + NameSize.cx) / 2;
            DrawString(Device, X, 5, 0xFFA2F9EC, Title);
            DrawString(Device, X + TitleSize.cx + 6, 5, NameColor, Name);
            Device->DrawTexture(X - 6, 3, 6, 16, 0, 0, 6, 16, NameLeftTex);
            Device->DrawTexture(X + NameSize.cx + TitleSize.cx + 6, 3, 6, 16, 0, 0, 6, 16,
                                NameRightTex);
        }
        else
        {
            appSprintf(Name, TEXT("%s"), Player->GetName());
            NCGDevice::GetTextExtent(&NameSize, Name, 0, 0, 0, 0, 0);
            INT X = 120 - NameSize.cx / 2;
            DrawString(Device, X, 5, NameColor, Name);
            Device->DrawTexture(X - 6, 3, 6, 16, 0, 0, 6, 16, NameLeftTex);
            Device->DrawTexture(X + NameSize.cx, 3, 6, 16, 0, 0, 6, 16, NameRightTex);
        }

        // the clan: crest and name, else SysString 431 ("No Clan")
        PledgeInfo* Pledge = GConsole->GetPledgeInfo(Player->PledgeID);
        if (Pledge)
        {
            if (Pledge->CrestTex)
                Device->DrawTexture(73, 27, 16, 12, 0, 4, 16, 12, Pledge->CrestTex);
            const TCHAR* PledgeName = Pledge->Name;
            if (PledgeName && *PledgeName)
            {
                NCGDevice::GetTextExtent(&Size, PledgeName, 0, 0, 0, 0, 0);
                DrawString(Device, 93, 27, 0xFFB09B79, PledgeName);
            }
        }
        else
            DrawString(Device, 73, 27, 0xFFFFFFFF, *NCObject::SysString(431));

        appSprintf(Buf, TEXT("%d %s"), Player->Level,
                   GConsole->GetClassNameString(Player->ClassID));
        NCGDevice::GetTextExtent(&Size, Buf, 0, 0, 0, 0, 0);
        DrawString(Device, 35, 42, 0xFFB09B79, Buf);

        DrawNumber(Device, Buf, 89, 118, Player->PAtk);
        DrawNumber(Device, Buf, 89, 133, Player->PDef);
        DrawNumber(Device, Buf, 89, 148, Player->Accuracy);
        DrawNumber(Device, Buf, 89, 163, Player->Critical);
        DrawNumber(Device, Buf, 89, 178, Player->PAtkSpeed);
        DrawNumber(Device, Buf, 206, 118, Player->MAtk);
        DrawNumber(Device, Buf, 206, 133, Player->MDef);
        DrawNumber(Device, Buf, 206, 148, Player->Evasion);
        // the speed as the player moves: running or walking (anything else: no speed); swimming (the pawn's +0x62C
        // is 1) takes the water speeds; times the speed rate (a double in C2)
        APawn* Pawn = Player->Pawn;
        BYTE MoveMode = Pawn->Controller->MoveMode;
        if (MoveMode == 1)
        {
            INT Speed = nw::Field<INT>(Pawn, 0x62C) == 1 ? Player->RunSpeedWater : Player->RunSpeed;
            DrawNumber(Device, Buf, 206, 163, (INT)(Speed * Player->MoveSpeedRate));
        }
        else if (MoveMode == 0)
        {
            INT Speed = nw::Field<INT>(Pawn, 0x62C) == 1 ? Player->WalkSpeedWater : Player->WalkSpeed;
            DrawNumber(Device, Buf, 206, 163, (INT)(Speed * Player->MoveSpeedRate));
        }
        DrawNumber(Device, Buf, 206, 178, Player->CastSpeed);
        DrawStat(Device, Buf, 65, 213, Player->Str, UnknownCC[BONUS_Str], TEXT("%d(+%d)"));
        DrawStat(Device, Buf, 138, 213, Player->Dex, UnknownCC[BONUS_Dex], TEXT("%d (+%d)"));
        DrawStat(Device, Buf, 212, 213, Player->Con, UnknownCC[BONUS_Con], TEXT("%d (+%d)"));
        DrawStat(Device, Buf, 65, 229, Player->Int, UnknownCC[BONUS_Int], TEXT("%d (+%d)"));
        DrawStat(Device, Buf, 138, 229, Player->Wit, UnknownCC[BONUS_Wit], TEXT("%d (+%d)"));
        DrawStat(Device, Buf, 212, 229, Player->Men, UnknownCC[BONUS_Men], TEXT("%d (+%d)"));

        // from 999999 on the karma gets a "+" (the value itself is still printed)
        if (Player->Karma >= 999999)
            appSprintf(Buf, TEXT("%d+"), Player->Karma);
        else
            appSprintf(Buf, TEXT("%d"), Player->Karma);
        NCGDevice::GetTextExtent(&Size, Buf, 0, 0, 0, 0, 0);
        DrawString(Device, 82 - Size.cx / 2, 267, 0xFFB09B79, Buf);
        appSprintf(Buf, TEXT("%d / %d"), Player->PvPCount, Player->PKCount);
        NCGDevice::GetTextExtent(&Size, Buf, 0, 0, 0, 0, 0);
        DrawString(Device, 202 - Size.cx / 2, 267, 0xFFB09B79, Buf);
        DrawNumber(Device, Buf, 82, 283, (INT)nw::Field<SWORD>(Player, 0x1F4));   // new in C2
        DrawNumber(Device, Buf, 202, 283, (INT)nw::Field<SWORD>(Player, 0x1F2));
    }
    Device->EndPaint();
    Device->ReleaseClip();
    return 1;
}
NW_HOOK_METHOD(0x100d67d0, NCDetailStatusWnd, OnPaint)

NW_CHECK_VTABLE(NCDetailStatusWnd, 0x10116908, 67)

/*-----------------------------------------------------------------------------
    NCGMDetailStatusWnd.
-----------------------------------------------------------------------------*/

// a number of the GM window: black one pixel lower right, then white
static FORCEINLINE void DrawShadowText(NCGDevice* Device, INT X, INT Y, const TCHAR* Text)
{
    DrawString(Device, X + 1, Y + 1, 0xFF000000, Text);
    DrawString(Device, X, Y, 0xFFFFFFFF, Text);
}

// the slash between two numbers of the GM window, in the large digits, with the same shadow
static FORCEINLINE void DrawShadowSlash(NCGDevice* Device, TCHAR* Buf, INT CenterX, INT Y)
{
    SIZE Size;
    appSprintf(Buf, TEXT("%s"), TEXT("/"));
    NCGDevice::GetTextExtent(&Size, Buf, 0, 0, 0, 0, 0);
    Device->DrawSpecialDigit(1, CenterX + 1 - Size.cx / 2, Y + 1, 0xFF000000, Buf);
    Device->DrawSpecialDigit(1, CenterX - Size.cx / 2, Y, 0xFFFFFFFF, Buf);
}

// 0x10074a30 - the User is built by Engine's constructor
NCGMDetailStatusWnd::NCGMDetailStatusWnd()
{
}
NW_HOOK_CTOR(0x10074a30, NCGMDetailStatusWnd)

// 0x10074ab0
NW_HOOK_DTOR(0x10074ab0, NCGMDetailStatusWnd)

// stock body shared with NCButton's (0x100051f0)
INT NCGMDetailStatusWnd::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (Msg != WM_PAINT && !IsEnabled())
        return 0;
    return NCDetailStatusWnd::WindowProc(Msg, wParam, lParam) != 0;
}

// 0x10074b00 (vslot 36) - the character tab's textures, and a title bar
INT NCGMDetailStatusWnd::OnCreate()
{
    NCDetailStatusWnd::OnCreate();
    FrameCtrl = new NCFrameCtrl(1);
    FrameCtrl->ID = NID_GMDETAIL_FRAME;
    FrameCtrl->SetWindowRect(0, 0, 240, 21);
    CreateChild(FrameCtrl, NWS_VISIBLE);
    return 1;
}
NW_HOOK_METHOD(0x10074b00, NCGMDetailStatusWnd, OnCreate)

// 0x10074bb0 (vslot 55) - the character tab of the player the GM asked about (Target): the same paint as the
// player's in C2 (the strings and values sit lower, 21 pixels under the title bar, each at its own height; HP and MP
// in the large digits without the C1 shadow, the clan from its info and not by the crest's name, the stats' bonuses
// and the two new value rows), the speed is the running one times the speed rate whatever the movement mode
INT NCGMDetailStatusWnd::OnPaint(NCGDevice* Device)
{
    Device->SetClip(ScreenX, ScreenY, Width, Height);
    Device->BeginPaint(ScreenX, ScreenY);
    if (BackTex)
        Device->DrawTexture(0, 21, 240, 300, 0, 0, 240, 300, BackTex);

    DrawString(Device, 17, 47, 0xFFA3A3A3, *NCObject::SysString(430));    // "Clan"
    DrawString(Device, 17, 62, 0xFFA3A3A3, *NCObject::SysString(88));     // "Lv"
    DrawString(Device, 17, 123, 0xFFFFFFFF, *NCObject::SysString(93));    // "Combat"
    DrawString(Device, 17, 140, 0xFFA3A3A3, *NCObject::SysString(94));    // "P.Atk."
    DrawString(Device, 17, 155, 0xFFA3A3A3, *NCObject::SysString(95));    // "P.Def."
    DrawString(Device, 17, 170, 0xFFA3A3A3, *NCObject::SysString(96));    // "Accuracy"
    DrawString(Device, 17, 185, 0xFFA3A3A3, *NCObject::SysString(113));   // "Critical"
    DrawString(Device, 17, 200, 0xFFA3A3A3, *NCObject::SysString(111));   // "Atk. Spd."
    DrawString(Device, 17, 218, 0xFFFFFFFF, *NCObject::SysString(103));   // "Basic"
    DrawString(Device, 122, 140, 0xFFA3A3A3, *NCObject::SysString(98));   // "M.Atk."
    DrawString(Device, 122, 156, 0xFFA3A3A3, *NCObject::SysString(99));   // "M.Def."
    DrawString(Device, 122, 171, 0xFFA3A3A3, *NCObject::SysString(97));   // "Evasion"
    DrawString(Device, 122, 186, 0xFFA3A3A3, *NCObject::SysString(432));  // "Speed"
    DrawString(Device, 122, 201, 0xFFA3A3A3, *NCObject::SysString(112));  // "Casting Spd."
    DrawString(Device, 17, 235, 0xFFA3A3A3, *NCObject::SysString(104));   // "STR"
    DrawString(Device, 93, 235, 0xFFA3A3A3, *NCObject::SysString(105));   // "DEX"
    DrawString(Device, 163, 235, 0xFFA3A3A3, *NCObject::SysString(106));  // "CON"
    DrawString(Device, 17, 251, 0xFFA3A3A3, *NCObject::SysString(107));   // "INT"
    DrawString(Device, 93, 251, 0xFFA3A3A3, *NCObject::SysString(108));   // "WIT"
    DrawString(Device, 163, 251, 0xFFA3A3A3, *NCObject::SysString(109));  // "MEN"
    DrawString(Device, 17, 270, 0xFFFFFFFF, *NCObject::SysString(100));   // "Social"
    DrawString(Device, 17, 289, 0xFFA3A3A3, *NCObject::SysString(101));   // "Karma"
    DrawString(Device, 122, 289, 0xFFA3A3A3, *NCObject::SysString(102));  // "PvP"
    DrawString(Device, 17, 305, 0xFFA3A3A3, *NCObject::SysString(709));   // new in C2
    DrawString(Device, 122, 305, 0xFFA3A3A3, *NCObject::SysString(710));  // new in C2

    // the GM's exp table starts at level 1 (index = level - 1)
    INT HPW = 0;
    INT MPW = 0;
    INT WeightW = 0;
    INT EXPW = 0;
    if (Target.MaxHP)
        HPW = (INT)(Target.HP * 84.0 / Target.MaxHP);
    if (Target.MaxMP)
        MPW = (INT)(Target.MP * 84.0 / Target.MaxMP);
    if (Target.MaxLoad)
        WeightW = (INT)(Target.Load * 84.0 / Target.MaxLoad);
    if (GGMExpTable[Target.Level])
        EXPW = (INT)((DOUBLE)(Target.Exp - GGMExpTable[Target.Level - 1]) * 189.0 /
                     (GGMExpTable[Target.Level] - GGMExpTable[Target.Level - 1]));

    if (HPW > 0 && HPW <= 84)
        Device->DrawTexture(34, 92, HPW, 10, 0, 0, HPW, 10, HPTex);
    else if (HPW > 84)
        Device->DrawTexture(34, 92, 84, 10, 0, 0, 84, 10, HPTex);
    if (MPW > 0 && MPW <= 84)
        Device->DrawTexture(139, 92, MPW, 10, 0, 0, MPW, 10, MPTex);
    else if (MPW > 84)
        Device->DrawTexture(139, 92, 84, 10, 0, 0, 84, 10, MPTex);
    if (EXPW > 0 && EXPW <= 189)
        Device->DrawTexture(34, 78, EXPW, 10, 0, 0, EXPW, 10, EXPTex);
    else if (EXPW > 189)
        Device->DrawTexture(34, 78, 189, 10, 0, 0, 189, 10, EXPTex);

    FLOAT WeightPct = 0.f;
    if (WeightW > 0)
    {
        DOUBLE Pct = (DOUBLE)Target.Load / Target.MaxLoad * 100.0;
        WeightPct = (FLOAT)Pct;
        if (Pct <= 50.0)
            Device->DrawTexture(139, 106, WeightW, 10, 0, 0, WeightW, 10, WeightTex[0]);
        else if (WeightPct <= 66.66)
            Device->DrawTexture(139, 106, WeightW, 10, 0, 0, WeightW, 10, WeightTex[1]);
        else if (WeightPct > 66.66 && WeightPct <= 80.f)
            Device->DrawTexture(139, 106, WeightW, 10, 0, 0, WeightW, 10, WeightTex[2]);
        else if (WeightPct > 80.f)
            Device->DrawTexture(139, 106, WeightW, 10, 0, 0, WeightW, 10, WeightTex[3]);
    }

    TCHAR Buf[64];
    SIZE Size;
    INT LevelExp = GGMExpTable[Target.Level - 1];
    swprintf(Buf, ARRAY_COUNT(Buf), TEXT("%.2f%s"),
             (DOUBLE)(Target.Exp - LevelExp) * 100.0 / (GGMExpTable[Target.Level] - LevelExp), TEXT("%"));
    NCGDevice::GetTextExtent(&Size, Buf, 0, 0, 0, 0, 0);
    Device->DrawSpecialDigit(1, 128 - Size.cx / 2, 79, 0xFFB4B4B4, Buf);

    // "now / max" around a slash centered at 76 (HP) and 181 (MP), in the large digits
    appSprintf(Buf, TEXT("%s"), TEXT("/"));
    NCGDevice::GetTextExtent(&Size, Buf, 0, 0, 0, 0, 0);
    Device->DrawSpecialDigit(1, 76 - Size.cx / 2, 93, 0xFFFFFFFF, Buf);
    appSprintf(Buf, TEXT("%d"), Target.HP);
    NCGDevice::GetTextExtent(&Size, Buf, 0, 0, 0, 0, 0);
    Device->DrawSpecialDigit(1, 66 - Size.cx, 93, 0xFFFFFFFF, Buf);
    appSprintf(Buf, TEXT("%d"), Target.MaxHP);
    NCGDevice::GetTextExtent(&Size, Buf, 0, 0, 0, 0, 0);
    Device->DrawSpecialDigit(1, 82, 93, 0xFFFFFFFF, Buf);
    appSprintf(Buf, TEXT("%s"), TEXT("/"));
    NCGDevice::GetTextExtent(&Size, Buf, 0, 0, 0, 0, 0);
    Device->DrawSpecialDigit(1, 181 - Size.cx / 2, 93, 0xFFFFFFFF, Buf);
    appSprintf(Buf, TEXT("%d"), Target.MP);
    NCGDevice::GetTextExtent(&Size, Buf, 0, 0, 0, 0, 0);
    Device->DrawSpecialDigit(1, 171 - Size.cx, 93, 0xFFFFFFFF, Buf);
    appSprintf(Buf, TEXT("%d"), Target.MaxMP);
    NCGDevice::GetTextExtent(&Size, Buf, 0, 0, 0, 0, 0);
    Device->DrawSpecialDigit(1, 187, 93, 0xFFFFFFFF, Buf);

    DrawNumber(Device, Buf, 76, 106, Target.SP);
    appSprintf(Buf, TEXT("%.2f%s"), WeightPct, TEXT("%"));
    NCGDevice::GetTextExtent(&Size, Buf, 0, 0, 0, 0, 0);
    Device->DrawSpecialDigit(1, 181 - Size.cx / 2, 107, 0xFFFFFFFF, Buf);

    // the name is white without karma; with it, green and blue go down by the karma (at least by
    // 100 + karma / 16, at most by 255: red)
    INT Red = Target.Karma;
    if (Red > 255)
        Red = 255;
    if (Red > 0)
    {
        INT MinRed = Red / 16 + 100;
        if (Red < MinRed)
            Red = MinRed;
        else if (Red >= 255)
            Red = 255;
    }
    DWORD NameColor = 0xFFFF0000 | ((255 - Red) << 8) | (255 - Red);

    // the title first, when there is one; the name between two end pieces
    TCHAR Name[64];
    TCHAR Title[64];
    SIZE NameSize;
    SIZE TitleSize;
    if (Target.GetNickName() && *Target.GetNickName())
    {
        appSprintf(Title, TEXT("%s"), Target.GetNickName());
        NCGDevice::GetTextExtent(&TitleSize, Title, 0, 0, 0, 0, 0);
        appSprintf(Name, TEXT("%s"), Target.GetName());
        NCGDevice::GetTextExtent(&NameSize, Name, 0, 0, 0, 0, 0);
        if (TitleSize.cx + NameSize.cx > 220)
        {
            if (TitleSize.cx > 109)
            {
                Title[9] = '.';
                Title[10] = '.';
                Title[11] = 0;
                NCGDevice::GetTextExtent(&TitleSize, Title, 0, 0, 0, 0, 0);
            }
            if (NameSize.cx > 109)
            {
                Name[9] = '.';
                Name[10] = '.';
                Name[11] = 0;
                NCGDevice::GetTextExtent(&NameSize, Name, 0, 0, 0, 0, 0);
            }
        }
        INT X = 120 - (TitleSize.cx + NameSize.cx) / 2;
        DrawString(Device, X, 26, 0xFFA2F9EC, Title);
        DrawString(Device, X + TitleSize.cx + 6, 26, NameColor, Name);
        Device->DrawTexture(X - 6, 24, 6, 16, 0, 0, 6, 16, NameLeftTex);
        Device->DrawTexture(X + NameSize.cx + TitleSize.cx + 6, 24, 6, 16, 0, 0, 6, 16, NameRightTex);
    }
    else
    {
        appSprintf(Name, TEXT("%s"), Target.GetName());
        NCGDevice::GetTextExtent(&NameSize, Name, 0, 0, 0, 0, 0);
        INT X = 120 - NameSize.cx / 2;
        DrawString(Device, X, 26, NameColor, Name);
        Device->DrawTexture(X - 6, 24, 6, 16, 0, 0, 6, 16, NameLeftTex);
        Device->DrawTexture(X + NameSize.cx, 24, 6, 16, 0, 0, 6, 16, NameRightTex);
    }

    // the clan: crest and name, else SysString 431 ("No Clan")
    PledgeInfo* Pledge = GConsole->GetPledgeInfo(Target.PledgeID);
    if (Pledge)
    {
        if (Pledge->CrestTex)
            Device->DrawTexture(73, 47, 16, 12, 0, 4, 16, 12, Pledge->CrestTex);
        const TCHAR* PledgeName = Pledge->Name;
        if (PledgeName && *PledgeName)
        {
            NCGDevice::GetTextExtent(&Size, PledgeName, 0, 0, 0, 0, 0);
            DrawString(Device, 93, 47, 0xFFB09B79, PledgeName);
        }
    }
    else
        DrawString(Device, 73, 47, 0xFFFFFFFF, *NCObject::SysString(431));

    appSprintf(Buf, TEXT("%d %s"), Target.Level, GConsole->GetClassNameString(Target.ClassID));
    NCGDevice::GetTextExtent(&Size, Buf, 0, 0, 0, 0, 0);
    DrawString(Device, 35, 62, 0xFFB09B79, Buf);

    DrawNumber(Device, Buf, 89, 140, Target.PAtk);
    DrawNumber(Device, Buf, 89, 155, Target.PDef);
    DrawNumber(Device, Buf, 89, 170, Target.Accuracy);
    DrawNumber(Device, Buf, 89, 185, Target.Critical);
    DrawNumber(Device, Buf, 89, 200, Target.PAtkSpeed);
    DrawNumber(Device, Buf, 206, 140, Target.MAtk);
    DrawNumber(Device, Buf, 206, 155, Target.MDef);
    DrawNumber(Device, Buf, 206, 170, Target.Evasion);
    DrawNumber(Device, Buf, 206, 185, (INT)(Target.RunSpeed * Target.MoveSpeedRate));
    DrawNumber(Device, Buf, 206, 200, Target.CastSpeed);
    DrawStat(Device, Buf, 65, 235, Target.Str, UnknownCC[BONUS_Str], TEXT("%d(+%d)"));
    DrawStat(Device, Buf, 138, 235, Target.Dex, UnknownCC[BONUS_Dex], TEXT("%d (+%d)"));
    DrawStat(Device, Buf, 212, 235, Target.Con, UnknownCC[BONUS_Con], TEXT("%d (+%d)"));
    DrawStat(Device, Buf, 65, 251, Target.Int, UnknownCC[BONUS_Int], TEXT("%d (+%d)"));
    DrawStat(Device, Buf, 138, 251, Target.Wit, UnknownCC[BONUS_Wit], TEXT("%d (+%d)"));
    DrawStat(Device, Buf, 212, 251, Target.Men, UnknownCC[BONUS_Men], TEXT("%d (+%d)"));

    // from 999999 on the karma gets a "+" (the value itself is still printed)
    if (Target.Karma >= 999999)
        appSprintf(Buf, TEXT("%d+"), Target.Karma);
    else
        appSprintf(Buf, TEXT("%d"), Target.Karma);
    NCGDevice::GetTextExtent(&Size, Buf, 0, 0, 0, 0, 0);
    DrawString(Device, 82 - Size.cx / 2, 289, 0xFFB09B79, Buf);
    appSprintf(Buf, TEXT("%d / %d"), Target.PvPCount, Target.PKCount);
    NCGDevice::GetTextExtent(&Size, Buf, 0, 0, 0, 0, 0);
    DrawString(Device, 202 - Size.cx / 2, 289, 0xFFB09B79, Buf);
    DrawNumber(Device, Buf, 82, 305, (INT)nw::Field<SWORD>(&Target, 0x1F4));   // new in C2
    DrawNumber(Device, Buf, 202, 305, (INT)nw::Field<SWORD>(&Target, 0x1F2));

    Device->EndPaint();
    Device->ReleaseClip();
    return 1;
}
NW_HOOK_METHOD(0x10074bb0, NCGMDetailStatusWnd, OnPaint)

// 0x100730d0 - the server's answer about the player (NConsoleWnd's virtual 202)
void NCGMDetailStatusWnd::SetTarget(User InTarget)
{
    Target = InTarget;
}
NW_HOOK_METHOD(0x100730d0, NCGMDetailStatusWnd, SetTarget)

NW_CHECK_VTABLE(NCGMDetailStatusWnd, 0x101170c8, 67)
