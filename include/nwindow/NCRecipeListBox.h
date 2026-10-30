#pragma once
// NCRecipeListBox - the list of recipes a dwarf's shop offers (UClass "CRecipeListBox", 0x1F8; new in C2), the list
// of NCRecipeBuyListWnd. Each row is 43 pixels high (plus LineGap, 4): the recipe's icon, its name, the price in
// adena ("Price : 1,234 Adena", the amount colored by its size) and the success rate ("Rate : 60%"). Same shape as
// NCVIPShopItemListBox / NCTrainListBox (their code is shared where identical).
//
// WindowProc (0x100dc140), OnCreate (0x100dc160), OnDestroy (0x100ddc20) and OnLButtonDown (0x10079050) are the stock
// bodies shared with NCChatListBox / NCTrainListBox (hooked there); HitTest (0x100c1040) and Clear (0x100c3310) the
// ones shared with the henna list (NCHennaListBox, another group's).

#include "nwindow/NCListBox.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

// one row (name ours; 0xA20 bytes, allocated by AddEntry, freed with appFree by OnDestroy / Clear). The console builds
// one on its stack for every recipe the server lists (NConsoleWnd vfn130, 0x10066c30) and AddEntry copies it.
struct NRecipeRow
{
    TCHAR Name[128];           // +0x000 the recipe's (the item it makes) name
    TCHAR IconName[128];       // +0x100 texture of the icon (AddEntry looks it up)
    TCHAR Description[1024];   // +0x200 never drawn here
    UTexture* Icon;            // +0xA00 AddEntry finds it by IconName ("NWindow.BlackTexture" when missing)
    INT RecipeID;              // +0xA04 (-1) what RequestRecipeShopMakeInfo names (the recipe's index)
    INT Unknown0A08;           // +0xA08 (-1) (the recipe record's +0x20)
    INT SuccessRate;           // +0xA0C percent, the "Rate" line
    INT Price;                 // +0xA10 adena, the "Price" line
    INT Unknown0A14;           // +0xA14 AddEntry leaves it as constructed (0); the console stores the item record's
                               //   +0xBC / +0x214 here
    INT Unknown0A18;           // +0xA18 (the console's second argument)
    INT Unknown0A1C;           // +0xA1C (-1) AddEntry leaves it as constructed

    // as AddEntry initializes a fresh row
    NRecipeRow()
    {
        Name[0] = 0;
        IconName[0] = 0;
        Description[0] = 0;
        Icon = NULL;
        RecipeID = -1;
        Unknown0A08 = -1;
        SuccessRate = 0;
        Unknown0A14 = 0;
        Unknown0A18 = 0;
        Unknown0A1C = -1;
        Price = 0;
    }
};
NW_ASSERT_OFFSET(NRecipeRow, IconName, 0x100);
NW_ASSERT_OFFSET(NRecipeRow, Description, 0x200);
NW_ASSERT_OFFSET(NRecipeRow, Icon, 0xA00);
NW_ASSERT_OFFSET(NRecipeRow, SuccessRate, 0xA0C);
NW_ASSERT_OFFSET(NRecipeRow, Price, 0xA10);
NW_ASSERT_SIZE(NRecipeRow, 0xA20);

// the amount with a comma between thousands (name ours; stdcall free function at 0x10057150, only the recipe windows
// call it): Out gets at most 32 characters for any INT
void __stdcall NFormatThousands(TCHAR* Out, INT Value);       // 0x10057150

// the color an amount's text is drawn in (name ours; stdcall free function at 0x1002bb70, NCEditBox::OnPaint calls it
// too): white up to four digits, then orange, yellow, green and cyan in turn
DWORD __stdcall NAmountColor(const TCHAR* Text);              // 0x1002bb70

class NCRecipeListBox : public NCListBox
{
    NW_DECLARE_CLASS(NCRecipeListBox, NCListBox, 0, 0x101D4DD0)

    TArray<NRecipeRow*> Entries;     // +0x1EC (name ours)

    NCRecipeListBox();                                               // 0x100c41e0
    NCRecipeListBox(INT InMaxLines, INT InVisibleLines, INT Unused); // 0x100c42a0
    // ~NCRecipeListBox                                              // 0x100c4240

    // stock bodies shared with other list boxes (hooked there)
    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x100dc140 (NCChatListBox's)
    virtual INT OnCreate();                                          // 36 0x100dc160 (NCTrainListBox's)
    virtual INT OnDestroy();                                         // 37 0x100ddc20 (NCTrainListBox's)
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x100c1a00
    virtual INT OnLButtonDown(WPARAM wParam, LPARAM lParam);         // 61 0x10079050 (NCTrainListBox's)
    virtual INT HitTest(INT X, INT Y);                               // 68 0x100c1040 (shared with NCHennaListBox)
    virtual void Clear();                                            // 70 0x100c3310 (shared with NCHennaListBox)

    // names ours
    __declspec(noinline) void UpdateEntryScrollBar();                // 0x100c1110 (the same body serves NCVIPShopItemListBox,
                                                                     //   NCTrainListBox and the henna list)
    __declspec(noinline) UBOOL GetSelectedEntry(NRecipeRow* Out);    // 0x100c1ef0 the selected row copied into Out
    __declspec(noinline) void AddEntry(const NRecipeRow* Src);       // 0x100c3390 a copy of Src
};
NW_ASSERT_OFFSET(NCRecipeListBox, Entries, 0x1EC);
NW_ASSERT_SIZE(NCRecipeListBox, 0x1F8);

#pragma pack(pop)
