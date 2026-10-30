/*=============================================================================
    NCRecipeShopWnd.cpp - the dwarven recipe shop's manage window (new in C2).
    Stock: 0x100c0ec0 .. 0x100c1035 (WindowProc, the cancel / quit handlers), 0x100c13d0 ..
    0x100c1a00 (constructor, paint, destroy, start, add product, message), 0x100c32b0 /
    0x100c32c0 / 0x100c32d0 and 0x100c3c50 (OnCreate).
=============================================================================*/
#include "nwindow/NCRecipeShopWnd.h"
#include "nwindow/NCRecipeShopItemWnd.h"
#include "nwindow/NCConsole.h"
#include "nwindow/NCButton.h"
#include "nwindow/NCFrameCtrl.h"
#include "nwindow/NCGDevice.h"
#include "nwindow/NConsoleWnd.h"
#include "nwindow/NDialogInfo.h"
#include "nwindow/NClasses.gen.h"

enum { DIALOG_RECIPE_PRICE = 0x23 };    // NDialogInfo::ID (see NCRecipeShopItemWnd.cpp)
enum { DIALOG_RECIPE_MESSAGE = 0x24 };  // NDialogInfo::ID (name ours): the shop's message (text); the dialog box answers it
enum { MSG_PriceToBuy = 0x142 };
enum { MSG_ShopMessage = 0x14e };       // systemmsg line the message box shows

// sysstring labels (the numbers are the stock code's GL2GameData + 0x4B4E8 + 12 * index)
enum
{
    STR_RecipeList = 643,
    STR_ProductionList = 661,
    STR_StartButton = 0x1AC,
    STR_MessageButton = 0x180,
    STR_QuitButton = 0x181,
};

// a child window goes (repeated inline in the stock OnDestroy)
template <class T> static FORCEINLINE void DestroyChild(NCWnd* Parent, T*& Child)
{
    if (Child)
    {
        Parent->RemoveChildWindow(Child->Entry);
        delete Child;
        Child = NULL;
    }
}

// 0x100c13d0
NCRecipeShopWnd::NCRecipeShopWnd()
{
    RecipeItems = NULL;
    ProductionItems = NULL;
    Unknown0A0 = 0;
    BackTex = NULL;
    Unknown0A8 = 0;
    UpButton = NULL;
    DownButton = NULL;
    StartButton = NULL;
    MessageButton = NULL;
    QuitButton = NULL;
    UnknownButtonC0 = NULL;
    Unknown0C4 = 0;
}
NW_HOOK_CTOR(0x100c13d0, NCRecipeShopWnd)

// 0x100c1460
NW_HOOK_DTOR(0x100c1460, NCRecipeShopWnd)

// 0x100c0fb0 - both grids emptied (the stock code tail-calls the second Clear)
__declspec(noinline) void NCRecipeShopWnd::ClearLists()
{
    RecipeItems->Clear();
    ProductionItems->Clear();
}
NW_HOOK_METHOD(0x100c0fb0, NCRecipeShopWnd, ClearLists)

// 0x100c0ec0 (vslot 25) - the buttons' handlers
INT NCRecipeShopWnd::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (NCWnd::WindowProc(Msg, wParam, lParam))
        return 1;
    if (Msg == NWM_COMMAND)
    {
        if (wParam == NID_RECIPESHOP_START)
            return OnStart() != 0;
        if (wParam == NID_RECIPESHOP_UP)
            return OnRemoveProduct() != 0;
        if (wParam == NID_RECIPESHOP_DOWN)
            return OnAddProduct() != 0;
        if (wParam == NID_RECIPESHOP_MESSAGE)
            return OnMessage() != 0;
        if (wParam == NID_RECIPESHOP_QUIT)
            return OnQuit() != 0;
        if (wParam == NID_RECIPESHOP_CANCEL)
            return OnCancel() != 0;
    }
    return 0;
}
NW_HOOK_METHOD(0x100c0ec0, NCRecipeShopWnd, WindowProc)

// 0x100c0fd0 (vslot 44) - closed with the grids emptied
void NCRecipeShopWnd::HideWindow()
{
    ClearLists();
    NCWnd::HideWindow();
}
NW_HOOK_METHOD(0x100c0fd0, NCRecipeShopWnd, HideWindow)

// 0x100c0fe0 (vslot 70) - the server is told the setup is cancelled, the window closes emptied
INT NCRecipeShopWnd::OnCancel()
{
    GConsole->RequestRecipeShopManageCancel();
    ClearLists();
    HideWindow();
    return 1;
}
NW_HOOK_METHOD(0x100c0fe0, NCRecipeShopWnd, OnCancel)

// 0x100c1010 (vslot 71) - the server is told the manager is left, the window closes emptied
INT NCRecipeShopWnd::OnQuit()
{
    GConsole->RequestRecipeShopManageQuit();
    ClearLists();
    HideWindow();
    return 1;
}
NW_HOOK_METHOD(0x100c1010, NCRecipeShopWnd, OnQuit)

// 0x100c14b0 (vslot 55) - the background and the two grid labels
INT NCRecipeShopWnd::OnPaint(NCGDevice* Device)
{
    Device->BeginPaint(ScreenX, ScreenY);
    Device->SetClip(ScreenX, ScreenY, Width, Height);
    if (BackTex)
        Device->DrawTexture(0, 21, 240, 344, 0, 0, 240, 344, BackTex);
    Device->DrawText(15, 25, 0xFFFFFFFF, *SysString(STR_RecipeList), 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
    Device->DrawText(15, 200, 0xFFFFFFFF, *SysString(STR_ProductionList), 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0);
    Device->EndPaint();
    Device->ReleaseClip();
    return 1;
}
NW_HOOK_METHOD(0x100c14b0, NCRecipeShopWnd, OnPaint)

// 0x100c15b0 (vslot 37)
INT NCRecipeShopWnd::OnDestroy()
{
    DestroyChild(this, RecipeItems);
    DestroyChild(this, ProductionItems);
    DestroyChild(this, UpButton);
    DestroyChild(this, DownButton);
    DestroyChild(this, StartButton);
    DestroyChild(this, UnknownButtonC0);
    DestroyChild(this, QuitButton);
    DestroyChild(this, MessageButton);
    NCWnd::OnDestroy();
    return 1;
}
NW_HOOK_METHOD(0x100c15b0, NCRecipeShopWnd, OnDestroy)

// 0x100c1730 (vslot 69) - Start: the shop's list goes to the server (the number of entries, then the recipe's object id
// and the price of each; the parameter stack is made for three values an entry), the window closes emptied
INT NCRecipeShopWnd::OnStart()
{
    INT Num = ProductionItems->Items.Num();
    L2ParamStack Params(Num * 3 + 1);
    Params.PushBack((void*)(INT_PTR)Num);
    for (INT i = 0; i < ProductionItems->Items.Num(); i++)
    {
        Params.PushBack((void*)(INT_PTR)ProductionItems->Items(i)->ObjectID);
        Params.PushBack((void*)(INT_PTR)ProductionItems->Items(i)->UnknownC20);
    }
    GConsole->RequestRecipeShopListSet(Params);
    ClearLists();
    HideWindow();
    return 1;
}
NW_HOOK_METHOD(0x100c1730, NCRecipeShopWnd, OnStart)

// 0x100c1820 (vslot 68) - the down arrow: the price of the selected recipe is asked (the answer is the upper grid's
// OnDialogResult, which moves the recipe to the lower grid)
INT NCRecipeShopWnd::OnAddProduct()
{
    INT Sel = RecipeItems->GetSelected();
    if (Sel >= 0 && Sel < RecipeItems->Items.Num())
    {
        NItemEntry* Item = RecipeItems->Items(Sel);
        if (Item)
        {
            TCHAR Name[64];
            appSprintf(Name, TEXT("%s"), Item->GetName());
            NDialogInfo Info;
            Info.ID = DIALOG_RECIPE_PRICE;
            Info.Type = 3;
            Info.Owner = RecipeItems->Entry;
            Info.bMainBox = 1;
            Info.bInput = 1;
            Info.SetSystemMessage(MSG_PriceToBuy);
            Info.bLargeAmount = 1;
            GConsole->ShowDialog(&Info);
        }
    }
    return 1;
}
NW_HOOK_METHOD(0x100c1820, NCRecipeShopWnd, OnAddProduct)

// 0x100c1930 (vslot 72) - Message: a text box with the shop's message so far (the player's pawn +0x1494), no owner: the
// dialog box answers it by its ID
INT NCRecipeShopWnd::OnMessage()
{
    NDialogInfo Info;
    Info.ID = DIALOG_RECIPE_MESSAGE;
    Info.bMainBox = 1;
    Info.bInput = 1;
    Info.SetSystemMessage(MSG_ShopMessage);
    Info.bLargeAmount = 0;
    Info.Type = 0;
    const TCHAR* Message = NULL;
    User* Me = GConsole->GetMyUser();
    if (Me)
    {
        APawn* MyPawn = Me->Pawn;
        if (MyPawn)
            Message = *nw::Field<FString>(MyPawn, 0x1494);   // (name ours) the shop message
    }
    Info.SetText60(Message);
    GConsole->ShowDialog(&Info);
    return 1;
}
NW_HOOK_METHOD(0x100c1930, NCRecipeShopWnd, OnMessage)

// 0x100c32d0 (vslot 67) - the up arrow: the selected entry of the lower grid is taken off it
INT NCRecipeShopWnd::OnRemoveProduct()
{
    INT Sel = ProductionItems->GetSelected();
    if (Sel >= 0 && Sel < ProductionItems->Items.Num())
        ProductionItems->Items.Remove(Sel, 1);
    return 1;
}
NW_HOOK_METHOD(0x100c32d0, NCRecipeShopWnd, OnRemoveProduct)

// 0x100c32c0 - what the console forwards (stock: the upper grid's AddItem as a tail jump)
UBOOL NCRecipeShopWnd::AddRecipeItem(NItemInfo* Info)
{
    return RecipeItems->AddItem(Info);
}
NW_HOOK_METHOD(0x100c32c0, NCRecipeShopWnd, AddRecipeItem)

// 0x100c32b0 - the same for the lower grid
UBOOL NCRecipeShopWnd::AddProductionItem(NItemInfo* Info)
{
    return ProductionItems->AddItem(Info);
}
NW_HOOK_METHOD(0x100c32b0, NCRecipeShopWnd, AddProductionItem)

// 0x100c3c50 (vslot 36) - title bar, the two grids (6 x 3 icons below, 6 x 4 above, 160 recipes at most), the arrows
// between them, then Start, Message and Quit
INT NCRecipeShopWnd::OnCreate()
{
    NCWnd::OnCreate();
    BackTex = FindTexture(TEXT("L2UI.RecipeWnd.RecipePersonalMakingBack"));

    FrameCtrl = new NCFrameCtrl(1);
    FrameID = NID_RECIPESHOP_FRAME;
    FrameCtrl->ID = NID_RECIPESHOP_FRAME;
    FrameCtrl->SetWindowRect(0, 0, 240, 21);
    CreateChild(FrameCtrl, NWS_VISIBLE);

    ProductionItems = new NCRecipeShopItemWnd(6, 3, 160, 32);
    ProductionItems->SetWindowRect(16, 225, 223, 104);
    CreateChild(ProductionItems, NWS_VISIBLE | 0x20);   // 0x20: meaning unknown
    ProductionItems->SetMode(0);

    RecipeItems = new NCRecipeShopItemWnd(6, 4, 160, 32);
    RecipeItems->SetWindowRect(16, 50, 223, 139);
    CreateChild(RecipeItems, NWS_VISIBLE | 0x20);
    RecipeItems->SetMode(1);
    ProductionItems->SetTarget(RecipeItems);
    RecipeItems->SetTarget(ProductionItems);

    // the up arrow's pushed texture is its normal one, as in the stock code
    UpButton = new NCButton(TEXT("L2UI.StoreWnd.StoreUpBtn1"), TEXT("L2UI.StoreWnd.StoreUpBtn1"), 0);
    UpButton->ID = NID_RECIPESHOP_UP;
    UpButton->SetWindowRect(194, 200, 14, 14);
    CreateChild(UpButton, NWS_VISIBLE);
    DownButton = new NCButton(TEXT("L2UI.StoreWnd.StoreDownBtn1"), TEXT("L2UI.StoreWnd.StoreDownBtn2"), 0);
    DownButton->ID = NID_RECIPESHOP_DOWN;
    DownButton->SetWindowRect(210, 200, 14, 14);
    CreateChild(DownButton, NWS_VISIBLE);

    StartButton = new NCButton(TEXT("L2UI.Control.DefaultButton"), TEXT("L2UI.Control.DefaultButton_click"), 0);
    StartButton->ID = NID_RECIPESHOP_START;
    StartButton->SetWindowRect(164, 335, 67, 19);
    CreateChild(StartButton, NWS_VISIBLE);
    StartButton->SetTextID(STR_StartButton);
    MessageButton = new NCButton(TEXT("L2UI.Control.DefaultButton"), TEXT("L2UI.Control.DefaultButton_click"), 0);
    MessageButton->ID = NID_RECIPESHOP_MESSAGE;
    MessageButton->SetWindowRect(87, 335, 67, 19);
    CreateChild(MessageButton, NWS_VISIBLE);
    MessageButton->SetTextID(STR_MessageButton);
    QuitButton = new NCButton(TEXT("L2UI.Control.DefaultButton"), TEXT("L2UI.Control.DefaultButton_click"), 0);
    QuitButton->ID = NID_RECIPESHOP_QUIT;
    QuitButton->SetWindowRect(10, 335, 67, 19);
    CreateChild(QuitButton, NWS_VISIBLE);
    QuitButton->SetTextID(STR_QuitButton);
    return 1;
}
NW_HOOK_METHOD(0x100c3c50, NCRecipeShopWnd, OnCreate)

NW_CHECK_VTABLE(NCRecipeShopWnd, 0x10123798, 73)
