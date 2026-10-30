#pragma once
// NCItemWnd - grid of item icons (UClass "CItemWnd", 0x118), base of the item windows:
// inventory, pet inventory, shops, warehouse, private stores, skills, quest items, trade
// (NCTradeItemWnd's UClass names NCWnd as its super, but its constructor calls NCItemWnd's
// and its vtable inherits these bodies). The items are NItemEntry records the window owns
// (Clear frees them), shown as Columns x VisibleRows icons of CellSize pixels, 3 apart,
// scrolled by whole rows. A click selects an item and picks it up (bDragging: the cursor
// becomes its icon); the tool tip over an item is its name, or for the selected item a box
// whose text the subclass draws (DrawItemToolTip).

#include "nwindow/NCScrollWnd.h"
#include "nwindow/NConsoleWnd.h"
#include "nwindow/NItemInfo.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

// An item of an item window (name ours; 0x1A28 bytes in C2, 0x1CAC in C1; created with new from an
// NItemInfo by the subclasses' AddItem and deleted by NCItemWnd::Clear): the NItemInfo fields in
// another order, a display name (sub_10019590, name ours) and the selection.
//
// C2 layout: from the copy of an NItemInfo into an entry (0x1001A9A0, field by field), Reset
// (0x100196F0), the getters (0x10019580..0x100196E0) and the allocation (`new` of 0x1A28 bytes in
// NCInvenItemWnd::AddItem). The strings are the NItemInfo's, shifted by 0xC; the two tool tip strings
// of C1 are gone (C2 keeps an object at +0x19B0 that the tool tip code, 0x10028B20, fills).
struct NItemEntry
{
    UTexture* Icon;            // +0x000
    INT Unknown004;            // +0x004 second icon (GetIcon2: the paperdoll's two-slot items)
    INT Unknown008;            // +0x008 third icon (GetIcon3)
    TCHAR Name[256];           // +0x00C (C1: 128 characters)
    TCHAR AdditionalName[256]; // +0x20C new in C2 (NItemInfo +0x200): shown after the name
    TCHAR IconName[128];       // +0x40C (C1 +0x10C)
    TCHAR Unknown20C[128];     // +0x50C texture of the second icon (C1 +0x20C)
    TCHAR Unknown30C[128];     // +0x60C texture of the third icon (C1 +0x30C)
    TCHAR Description[2048];   // +0x70C (C1 +0x40C, 1024 characters)
    TCHAR DisplayName[256];    // +0x170C new in C2 (name ours): "+N Name AdditionalName", built by sub_10019590
    INT Type;                  // +0x190C (-1) NItemInfo::Param; 0..3: the enchant shows (C1 +0xC0C)
    INT Unknown1910;           // +0x1910 (-1) new in C2 (NItemInfo +0x1710): the kind of an etc item (Type 5): 7 pet collar (its level is Enchant), 0xC / 0xD henna-like (the tool tip lists classes); the display name's "+N" is left out for 7, 0xC, 0xD
    INT ObjectID;              // +0x1914 the server's id of the item (C1 +0xC10)
    INT ID;                    // +0x1918 (C1 +0xC14)
    INT UnknownC18;            // +0x191C (-1) NItemInfo +0x1724 (C1 +0xC18): a skill's level (the tool tip: NToolTipInfo::Level, NConsoleWnd::GetSkillTypeText(ID, this))
    INT Count;                 // +0x1920 (C1 +0xC1C)
    INT UnknownC20;            // +0x1924 NItemInfo +0x1720 (C1 +0xC20): the price (NToolTipInfo::Fill's NTTO_Price line, formatted with commas)
    INT BodyPart;              // +0x1928 (NConsoleWnd::GetItemSlotText) (C1 +0xC24)
    INT Weight;                // +0x192C (C1 +0xC28)
    INT UnknownC2C;            // +0x1930 (-1) (C1 +0xC2C)
    INT UnknownC30;            // +0x1934 (-1) weapon type (NConsoleWnd::GetWeaponTypeName; 6: bow) (C1 +0xC30)
    INT UnknownC34;            // +0x1938 (-1) NItemInfo +0x1798 (C1 +0xC34): the amount wanted (the private store's buy list, NTT_Wanted); a recipe's first number
    INT Enchant;               // +0x193C (C1 +0xC38)
    SWORD UnknownC3C;          // +0x1940 NItemInfo +0x178C: a word in C2 (C1: an INT at +0xC3C)
    SWORD _Unknown1942;        // +0x1942 padding (Reset leaves it)
    UBOOL bStackable;          // +0x1944 the count shows: "Name(Count)" (C1 +0xC40)
    UBOOL bEquipped;           // +0x1948 (NItemInfo +0x1794) (C1 +0xC44)
    INT TradeCount;            // +0x194C only the constructors clear it; the trade grid's amount
                               //   offered (C1 +0xC48)
    UBOOL bSelected;           // +0x1950 (C1 +0x1C54)
    INT Unknown1C58;           // +0x1954 NItemInfo +0x1750 (C1 +0x1C58)
    INT CrystalGrade;          // +0x1958 (NItemInfo +0x1754) (C1 +0x1C5C)
    INT Unknown1C60;           // +0x195C (C1 +0x1C60)
    INT Unknown1C64;           // +0x1960 (C1 +0x1C64)
    INT Unknown1C68;           // +0x1964 NItemInfo +0x1760 (C1 +0x1C68)
    INT Unknown1C6C;           // +0x1968 attack speed (NConsoleWnd::GetAttackSpeedText) (C1 +0x1C6C)
    INT Unknown1C70;           // +0x196C (C1 +0x1C70)
    INT SubType;               // +0x1970 (-1) (NConsoleWnd::GetItemSlotText) (C1 +0x1C74)
    INT Unknown1C78;           // +0x1974 a shield's evasion (C1 +0x1C78)
    INT Unknown1C7C;           // +0x1978 Copy leaves it (C1 +0x1C7C)
    INT Unknown1C80;           // +0x197C NItemInfo +0x1738 P.Atk (C1 +0x1C80)
    INT Unknown1C84;           // +0x1980 M.Atk (C1 +0x1C84)
    INT Unknown1C88;           // +0x1984 P.Def (C1 +0x1C88)
    INT Unknown1C8C;           // +0x1988 M.Def (C1 +0x1C8C)
    INT Unknown1C90;           // +0x198C a shield's P.Def (C1 +0x1C90)
    INT Unknown1C94;           // +0x1990 a shield's rate (C1 +0x1C94)
    INT Unknown1C98;           // +0x1994 NItemInfo +0x179C (the paperdoll: the item is not taken
                               //   off by a click or a drag; a bow is drawn in one slot) (C1 +0x1C98)
    INT Unknown1998;           // +0x1998 new in C2 (NItemInfo +0x17A0)
    INT Unknown1C9C;           // +0x199C NItemInfo +0x177C MP bonus (robes) (C1 +0x1C9C)
    INT Unknown1CA0;           // +0x19A0 soulshots used (C1 +0x1CA0)
    INT Unknown1CA4;           // +0x19A4 spiritshots used (C1 +0x1CA4)
    INT Unknown1CA8;           // +0x19A8 NItemInfo +0x17A4; Reset leaves it (C1 +0x1CA8)
    INT Unknown19AC;           // +0x19AC new in C2 (NItemInfo +0x1788; read by sub_100196E0)
    NToolTipInfo ToolTip;      // +0x19B0 new in C2: the tool tip's data (0x78 bytes, constructed by 0x10002230, copied by 0x1001ACA0,
                               //   destroyed by 0x100022D0 through ~NItemEntry 0x1007FAF0, filled by NToolTipInfo::Fill 0x10028B20(Item, NTT_Item, 0));
                               //   the two 1024-character strings of C1 (ToolTipName, ToolTipText) are gone

    NItemEntry();                                                    // 0x1001A850
    NItemEntry(NItemInfo* Src);                                      // 0x1001A9A0
    __declspec(noinline) ~NItemEntry();                                                   // 0x1007FAF0 (destroys ToolTip)

    // The stock code calls these through getters folded with other classes' (0x10033ac0,
    // 0x10033ad0, 0x10033a40, 0x10033a50).
    UTexture* GetIcon() { return Icon; }
    INT GetUnknown004() { return Unknown004; }
    INT GetUnknown008() { return Unknown008; }
    // Unknown004 / Unknown008 are the second and third icons of an item drawn over two slots
    // (from Unknown20C / Unknown30C: NCEquipItemWnd)
    UTexture* GetIcon2() { return (UTexture*)(INT_PTR)Unknown004; }
    UTexture* GetIcon3() { return (UTexture*)(INT_PTR)Unknown008; }

    // C1's tool tip strings: C2 has none (see ToolTip above). The members stay, never defined, so that
    // NItemToolTip.h (C1 code, no C2 counterpart yet) still parses: a function that uses them does not link
    static TCHAR ToolTipName[1024];
    static TCHAR ToolTipText[1024];

    // C2: the stock functions at 0x10019580 / 0x10019630 / 0x10019650 are three getters of one shape (NULL
    // when the string is empty): the name (+0xC), the additional name (+0x20C) and the description (+0x70C)
    // (C1's GetToolTipName / GetToolTipText sat at these addresses: the fmap paired them)
    __declspec(noinline) const TCHAR* GetName();                                          // 0x10019580 NULL when empty
    __declspec(noinline) const TCHAR* GetAdditionalName();                                // 0x10019630 NULL when empty (name ours)
    __declspec(noinline) const TCHAR* GetDescription();                                   // 0x10019650 NULL when empty
    // 0x10019590 (name ours): "+NName-AdditionalName" into DisplayName and returns it (the "+N" only when
    // Enchant > 0 and Unknown1910 is not 7, 0xC, 0xD)
    __declspec(noinline) const TCHAR* MakeDisplayName();
    // names ours
    __declspec(noinline) INT GetUnknownC20();                                             // 0x10019670
    __declspec(noinline) INT GetUnknownC34();                                             // 0x10019680
    __declspec(noinline) void SetUnknownC34(INT Value);                                   // 0x10019690
    __declspec(noinline) void Copy(NItemEntry* Src);                                      // 0x1001AF90
    __declspec(noinline) void Select();                                                   // 0x100196A0
    __declspec(noinline) UBOOL IsSelected();                                              // 0x100196B0
    __declspec(noinline) void Unselect();                                                 // 0x100196C0
    __declspec(noinline) UBOOL IsStackable();                                             // 0x100196D0
    __declspec(noinline) void Reset();                                                    // 0x100196F0
    // 0x10028B20 is NToolTipInfo::Fill (NToolTip.cpp): ToolTip.Fill(this, Flags, Extra). Flags is NTT_* (2, NTT_Item, is the item
    // windows' usual call; the callers also pass 1, 4, 8, 0x10, 0x20, 0x40, 0x80, 0x802), Extra the NTTO_* options (0, or
    // 4 / 8 / 7 for the shops' price lines). Name ours.
    void FillToolTip(INT Flags, INT Extra);
    __declspec(noinline) INT GetUnknown19AC();                                            // 0x100196E0
};
NW_ASSERT_OFFSET(NItemEntry, Name, 0xC);
NW_ASSERT_OFFSET(NItemEntry, AdditionalName, 0x20C);
NW_ASSERT_OFFSET(NItemEntry, IconName, 0x40C);
NW_ASSERT_OFFSET(NItemEntry, Unknown20C, 0x50C);
NW_ASSERT_OFFSET(NItemEntry, Unknown30C, 0x60C);
NW_ASSERT_OFFSET(NItemEntry, Description, 0x70C);
NW_ASSERT_OFFSET(NItemEntry, DisplayName, 0x170C);
NW_ASSERT_OFFSET(NItemEntry, Type, 0x190C);
NW_ASSERT_OFFSET(NItemEntry, ObjectID, 0x1914);
NW_ASSERT_OFFSET(NItemEntry, Count, 0x1920);
NW_ASSERT_OFFSET(NItemEntry, Enchant, 0x193C);
NW_ASSERT_OFFSET(NItemEntry, bStackable, 0x1944);
NW_ASSERT_OFFSET(NItemEntry, TradeCount, 0x194C);
NW_ASSERT_OFFSET(NItemEntry, bSelected, 0x1950);
NW_ASSERT_OFFSET(NItemEntry, SubType, 0x1970);
NW_ASSERT_OFFSET(NItemEntry, Unknown1C98, 0x1994);
NW_ASSERT_OFFSET(NItemEntry, Unknown19AC, 0x19AC);
NW_ASSERT_OFFSET(NItemEntry, ToolTip, 0x19B0);
NW_ASSERT_SIZE(NItemEntry, 0x1A28);

// An item as the item requests to the server take it, by value (name ours): the engine's
// ItemInfo (UE2Engine.h) with NWindow's constructor (Type and Unknown2C -1, Unknown02 not
// set, the rest 0); the destroy and drop requests fill ObjectID and Count.
struct NItemRequest : public ItemInfo
{
    NItemRequest();                                                  // 0x10064290
};
NW_ASSERT_OFFSET(NItemRequest, ObjectID, 0x4);
NW_ASSERT_OFFSET(NItemRequest, Count, 0xC);
NW_ASSERT_SIZE(NItemRequest, 0x30);

// The console's requests that take an NItemRequest by value
inline void NRequestDestroyItem(NItemRequest Request)
{
    GConsole->RequestDestroyItem(Request);
}
inline void NRequestDropItem(NItemRequest Request, FVector Location)
{
    GConsole->RequestDropItem(Request, Location);
}

class NCItemWnd : public NCScrollWnd
{
    NW_DECLARE_CLASS(NCItemWnd, NCScrollWnd, 0, 0x101A2778)

    // C2 keeps three textures (OnCreate 0x10019E90); C1's TipTopTex / TipMidTex / TipBottomTex went with the
    // tool tip boxes, which the window's skin (NCWnd::ToolTipSkin) draws now
    UTexture* BackTex;               // +0xD0 "NWindow.BlackTexture" (the subclasses draw it)
    UTexture* SelectTex;             // +0xD4 "l2ui.nwindow.item_click" around the selected item
    UTexture* ToolTipTex;            // +0xD8 "L2UI.tooltip.mini-tool01" (not used by this class any more)
    TArray<NItemEntry*> Items;       // +0xDC
    UBOOL bDragging;                 // +0xE8 the selected item is being carried
    UBOOL bNoDrag;                   // +0xEC the next click selects without picking up
    INT Columns;                     // +0xF0 (5)
    INT VisibleRows;                 // +0xF4 (5)
    INT TopRow;                      // +0xF8 first row shown
    INT NumRows;                     // +0xFC rows of items (UpdateScrollRange)
    INT MaxItems;                    // +0x100 (25) AddItem refuses more
    INT CellSize;                    // +0x104 (32) icon size
    INT Selected;                    // +0x108 -1 = none
    INT HoverIndex;                  // +0x10C item under the mouse, -1 = none

    NCItemWnd();                                                     // 0x1001ad50
    NCItemWnd(INT InColumns, INT InVisibleRows, INT InMaxItems, INT InCellSize); // 0x1001ae70
    // ~NCItemWnd                                                    // 0x1001ae10

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x10019810
    virtual INT OnCreate();                                          // 36 0x10019e90
    virtual INT OnDestroy();                                         // 37 0x10019830
    virtual void HideWindow();                                       // 44 0x10019a60
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x10019ee0
    virtual INT OnDrawToolTip(NCGDevice* Device);                    // 56 0x1001b2d0
    virtual INT OnMouseMove(WPARAM wParam, LPARAM lParam);           // 60 0x1001a1a0
    virtual INT OnLButtonDown(WPARAM wParam, LPARAM lParam);         // 61 0x1001a090
    virtual INT OnLButtonUp(WPARAM wParam, LPARAM lParam) { return 1; } // 62 0x100d2e70 (folded)
    virtual INT OnMouseWheel(WPARAM wParam, LPARAM lParam);          // 66 0x10019960
    // by rows instead of NCScrollWnd's pixels
    virtual INT ScrollUp();                                          // 67 0x10019bb0
    virtual INT ScrollDown();                                        // 68 0x10019bf0
    virtual INT OnScrollBar(INT Value);                              // 69 0x10019b90
    // names ours
    virtual void Clear();                                            // 70 0x1001af00 deletes the items
    virtual void UnselectAll();                                      // 71 0x1001a240
    // text of the selected item's tool tip box, from (X, Y); nothing here
    virtual void DrawItemToolTip(NCGDevice* Device, NItemEntry* Item, INT X, INT Y); // 72 0x10019c40 (folded)

    // names ours
    __declspec(noinline) INT HitTest(INT X, INT Y);                                       // 0x100198b0 item index, -1 outside
    __declspec(noinline) INT GetSelected();                                               // 0x10019930 -1: none
    __declspec(noinline) INT GetItemCount();                                              // 0x10019950
    __declspec(noinline) void InitScrollBar();                                            // 0x10019ae0
    __declspec(noinline) void UpdateScrollRange();                                        // 0x10019b20 after adding items
};
NW_ASSERT_OFFSET(NCItemWnd, BackTex, 0xD0);
NW_ASSERT_OFFSET(NCItemWnd, ToolTipTex, 0xD8);
NW_ASSERT_OFFSET(NCItemWnd, Items, 0xDC);
NW_ASSERT_OFFSET(NCItemWnd, Columns, 0xF0);
NW_ASSERT_OFFSET(NCItemWnd, MaxItems, 0x100);
NW_ASSERT_OFFSET(NCItemWnd, HoverIndex, 0x10C);
NW_ASSERT_SIZE(NCItemWnd, 0x110);

#pragma pack(pop)