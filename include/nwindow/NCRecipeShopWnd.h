#pragma once
// NCRecipeShopWnd - the dwarven recipe shop's manage window (UClass "CRecipeShopWnd", 0xCC; new in C2; the
// desktop's NCConsole +0x140). The player sets up a private workshop here: the upper grid ("Recipe List")
// holds the recipe book, the lower one ("Production List") the recipes put on sale; moving a recipe down asks its
// price (NCRecipeShopItemWnd.h), the up arrow takes the selected one off the lower grid. Start sends the list
// (RequestRecipeShopListSet: count, then recipe object id and price of each), Message asks the shop's text (the
// dialog box answers with NDialogInfo ID 0x24), and closing the window tells the server to quit (Quit) or to cancel
// the setup (Cancel).
//
// NWM_COMMAND ids (names ours): the buttons were made in OnCreate, the ids WindowProc dispatches are the buttons'.

#include "nwindow/NCWnd.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCButton;
class NCRecipeShopItemWnd;
struct NItemInfo;

enum
{
    NID_RECIPESHOP_FRAME = 0xF3,     // the title bar
    NID_RECIPESHOP_START = 0xF4,     // sends the list (OnStart)
    NID_RECIPESHOP_UP = 0xF6,        // takes the selected product off the lower grid (OnRemoveProduct)
    NID_RECIPESHOP_DOWN = 0xF7,      // asks the price of the selected recipe (OnAddProduct)
    NID_RECIPESHOP_CANCEL = 0xF8,    // no control creates it here (OnCancel)
    NID_RECIPESHOP_MESSAGE = 0xF9,   // asks the shop's message (OnMessage)
    NID_RECIPESHOP_QUIT = 0xF0,      // (OnQuit)
};

class NCRecipeShopWnd : public NCWnd
{
    NW_DECLARE_CLASS(NCRecipeShopWnd, NCWnd, 0, 0x101D5818)

    NCRecipeShopItemWnd* RecipeItems;     // +0x98 (name ours) upper grid, Mode 1: the recipe book
    NCRecipeShopItemWnd* ProductionItems; // +0x9C (name ours) lower grid, Mode 0: what is put on sale
    INT Unknown0A0;                       // +0xA0 (name ours) the constructor clears it, nothing else touches it
    UTexture* BackTex;                    // +0xA4 "L2UI.RecipeWnd.RecipePersonalMakingBack"
    INT Unknown0A8;                       // +0xA8 (cleared by the constructor, never used here)
    NCButton* UpButton;                   // +0xAC (name ours) StoreUpBtn1, id 0xF6
    NCButton* DownButton;                 // +0xB0 StoreDownBtn1 / 2, id 0xF7
    NCButton* StartButton;                // +0xB4 DefaultButton, id 0xF4, label sysstring 0x1AC
    NCButton* MessageButton;              // +0xB8 id 0xF9, label 0x180
    NCButton* QuitButton;                 // +0xBC id 0xF0, label 0x181
    NCButton* UnknownButtonC0;            // +0xC0 OnDestroy frees it, OnCreate does not make one (id 0xF8's)
    INT Unknown0C4;                       // +0xC4 (name ours) the console's recipe shop message (NConsoleWnd vfn126, 0x10059220)
                                      //   stores two numbers here and at +0xC8 before it shows the window; nothing here reads them
    INT Unknown0C8;                       // +0xC8 (name ours) see +0xC4 (the constructor leaves it as allocated)

    NCRecipeShopWnd();                                               // 0x100c13d0
    // ~NCRecipeShopWnd                                              // 0x100c1460

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x100c0ec0
    virtual INT OnCreate();                                          // 36 0x100c3c50
    virtual INT OnDestroy();                                         // 37 0x100c15b0
    virtual void HideWindow();                                       // 44 0x100c0fd0
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x100c14b0
    // names ours: the buttons' handlers
    virtual INT OnRemoveProduct();                                   // 67 0x100c32d0 up arrow
    virtual INT OnAddProduct();                                      // 68 0x100c1820 down arrow
    virtual INT OnStart();                                           // 69 0x100c1730
    virtual INT OnCancel();                                          // 70 0x100c0fe0
    virtual INT OnQuit();                                            // 71 0x100c1010
    virtual INT OnMessage();                                         // 72 0x100c1930

    // names ours (the console calls them)
    __declspec(noinline) void ClearLists();                          // 0x100c0fb0 both grids emptied
    UBOOL AddRecipeItem(NItemInfo* Info);                            // 0x100c32c0 into the upper grid
    UBOOL AddProductionItem(NItemInfo* Info);                        // 0x100c32b0 into the lower grid
};
NW_ASSERT_OFFSET(NCRecipeShopWnd, RecipeItems, 0x98);
NW_ASSERT_OFFSET(NCRecipeShopWnd, BackTex, 0xA4);
NW_ASSERT_OFFSET(NCRecipeShopWnd, UpButton, 0xAC);
NW_ASSERT_OFFSET(NCRecipeShopWnd, Unknown0C4, 0xC4);
NW_ASSERT_SIZE(NCRecipeShopWnd, 0xCC);

#pragma pack(pop)
