#pragma once
// NCCmdTarget / NCWnd - base of every window and control (UClass "CCmdTarget", "CWnd").
//
// A window lives in a tree: NCVirtualWndMain::RegisterNWindow gives it an NWndEntry, its
// parent keeps the entries of its children in a linked list (NWndListNode), messages arrive
// through WindowProc (vslot 24) as Win32 message numbers. Coordinates: Local* are relative
// to the parent, Screen* absolute; SetWindowRect keeps both.

#include "nwindow/NCObject.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCWnd;
class NCVirtualWndMain;
class NCFrameCtrl;
class NCIcon;
class NCGDevice;

// Registration record of a window (NCVirtualWndMain::RegisterNWindow, 16 bytes) (name ours).
struct NWndEntry
{
    INT bValid;              // +0x00 1 while registered
    NCWnd* Wnd;              // +0x04
    NCVirtualWndMain* Main;  // +0x08 the manager that registered it
    NWndEntry* Parent;       // +0x0C entry of the parent window, NULL for top-level windows

    NWndEntry() : bValid(0), Wnd(NULL), Main(NULL), Parent(NULL) {}
};
NW_ASSERT_SIZE(NWndEntry, 0x10);

// Node of NCWnd's child list, allocated with new (12 bytes) (name ours).
struct NWndListNode
{
    NWndEntry* Entry;        // +0x00
    NWndListNode* Prev;      // +0x04
    NWndListNode* Next;      // +0x08

    NWndListNode() : Entry(NULL), Prev(NULL), Next(NULL) {}
};
NW_ASSERT_SIZE(NWndListNode, 0xC);

// NCWnd::Style bits (names ours)
enum
{
    NWS_VISIBLE = 0x0002,
    NWS_UNKNOWN20 = 0x0020,        // C2 (name ours): NCWnd::OnCreate sets it, with NWS_TOPMOST | NWS_VISIBLE
                                   //   (0x1022), on the icon of a minimizable window; its effect is not known
    NWS_NOHITTEST = 0x0008,        // skipped by NCVirtualWndMain hit tests
    NWS_NOTINPARENTLIST = 0x0040,  // RemoveChildWindow leaves the parent's list alone
    NWS_TITLEBAR = 0x0080,         // OnCreate adds an NCFrameCtrl title bar
    NWS_TOPMOST = 0x1000,          // NCVirtualWndMain keeps these in a separate array
    NWS_MINIMIZABLE = 0x2000,      // OnCreate adds an NCIcon shown while minimized; a
                                   //   decorated NCFrameCtrl adds a minimize button
    NWS_NOCLOSE = 0x4000,          // a decorated NCFrameCtrl leaves out its close button
    NWS_MODAL = 0x0100,            // C2 (name ours): SetModal / ClearModal (vslots 53, 54) set and clear it
    NWS_ITEMSELECT = 0x40000,      // C2 (name ours): the inventory is picking an item (vslots 51, 52 set and
                                   //   clear it: NConsoleWnd::SelectItemToEnchant, NCInventoryWnd::OnMouseMove)
};

// Sent by NCVirtualWndMain after each paint to the window under the cursor, wParam = the
// NCGDevice, handled by vslot 51 (tool tips) (name ours).
enum { NWM_COMMAND = 0x501, NWM_DRAWTOOLTIP = 0x505 };

// NCWnd statics (names ours)
#if NW_HOOK_MODE
// C2 addresses (db/amap.csv: C1 0x10123d54 / 0x10123d50 / 0x1010a998; the generator left the C1 ones here)
#define GFocusEntry  NW_GLOBAL(NWndEntry*, 0x101a72b4)  // entry of the window with the keyboard focus
#define GShiftDown   NW_GLOBAL(INT, 0x101a72b0)         // Shift held (set by OnKeyDown/OnKeyUp)
#define GNextFrameID NW_GLOBAL(INT, 0x1015bc70)         // next id for a title bar (NCWnd::OnCreate)
#else
extern NWndEntry* GFocusEntry;
extern INT GShiftDown;
extern INT GNextFrameID;
#endif

// The tool tips of the C2 windows (names ours). NCWnd::OnCreate makes an NToolTipSkin for every window (nine
// textures "L2UI.Tooltip.Tool01".."Tool09" and an NToolTipInfo). What a tool tip shows is an NToolTipInfo that
// its owner keeps (NCButton +0x1D0, NItemEntry +0x19B0, NListRow +0x810, a macro's first member): filled by SetText
// or one of the Fill* builders, which end by measuring it (Layout). The window's OnDrawToolTip hands the skin a
// copy of the info by value, the window's rectangle and the position of the thing the tip is about; the skin
// draws its box with the nine textures and the text on it: a small box with the title line (Draw, for the item
// under the mouse) or the full box with the lines and the description (DrawFull, for the selected item).
// Layouts from the stock constructor 0x10002230, copy constructor 0x10002560, copy assignment 0x1001aca0, the
// skin's destructor 0x10029f20, the builders and the draw routines (0x10027500 .. 0x1002a230).

class NConsoleWnd;
struct NItemEntry;
struct NListRow;

// NToolTipInfo::Style (+0x50): what the tool tip is of; the builders and the draw routines branch on it. The
// values are the ones the windows pass to NItemEntry::FillToolTip (names ours)
enum
{
    NTT_ItemName = 0x001,    // an item's name, additional name and grade, then its weight (a short tip)
    NTT_Item = 0x002,        // an item: the title, then a line per property of its type, the description
    NTT_Skill = 0x004,       // a skill: the title with its level, then type, MP cost and range
    NTT_Action = 0x008,      // an action (name only; Fill takes it with NTT_Name)
    NTT_SkillName = 0x010,   // a skill's name and level only (the abnormal status icons)
    NTT_ShortCut = 0x020,    // a shortcut slot (an item, a skill, an action or a macro: the title depends on Type)
    NTT_Recipe = 0x040,      // a recipe: name, additional name, grade, the two lines of its numbers
    NTT_Name = 0x080,        // the name only
    NTT_Text = 0x100,        // a name and a description (the macros: FillMacro)
    NTT_Desc = 0x200,        // a description under a few lines only (the party matching rows: FillMatchRow)
    NTT_Plain = 0x400,       // SetText: a text, the first line of the title
    NTT_Wanted = 0x800,      // with NTT_Item: the private store's buy list also shows the amount wanted
};

// NToolTipInfo::Options (+0x54): the second argument of Fill (names ours)
enum
{
    NTTO_NoEnchant = 0x01,   // the "+N" before the name is not shown
    NTTO_NoCount = 0x02,     // the " (count)" after the name is not shown
    NTTO_Price = 0x04,       // Fill adds a first line "Price : N Adena" (the shops'); drawn under the name
    NTTO_PriceEach = 0x08,   // the same with the "For Each" before the number (a stack's unit price)
    NTTO_Count = 0x20,       // Fill adds a first line with the count alone (the shortcut slot's amount)
};

// One line of the tool tip (0x414 bytes; the lines array's element): "label : value". The lines are built on the
// stack and copied into the array with the stock `rep movsd` of 0x105 dwords (name ours)
struct NToolTipLine
{
    TCHAR Label[256];       // +0x000
    TCHAR Value[256];       // +0x200
    DWORD LabelColor;       // +0x400 grey (0xFFA3A3A3); the draw routines use the constant, not this
    DWORD ValueColor;       // +0x404 0xFFB09B79; an amount of adena takes the color of its size (NumberColor)
    INT Unused408;          // +0x408 never written by the builders (the stack's garbage is copied)
    INT bLabel;             // +0x40C 1: the label is drawn before the value; 0: the value alone
    INT bColon;             // +0x410 1: the label is followed by " : ", 0: by a space

    // as the builders initialize their local line (the Label/Value start empty)
    NToolTipLine() : LabelColor(0xFFA3A3A3), ValueColor(0xFFB09B79), Unused408(0), bLabel(1), bColon(1)
    {
        Label[0] = 0;
        Value[0] = 0;
    }
};
NW_ASSERT_SIZE(NToolTipLine, 0x414);
typedef TArray<NToolTipLine> NToolTipLineArray;

// a window's screen rectangle as the skin's draw routines take it by value: x, y, width, height (name ours); also
// the two boxes of NToolTipInfo, whose X and Y nothing uses
struct NToolTipRect
{
    INT X, Y, W, H;
};

struct NToolTipInfo
{
    FString Name;           // +0x00 the item's / skill's name
    FString Name2;          // +0x0C its additional name (shown after the name, in yellow)
    INT Enchant;            // +0x18 the "+N" before the name (shown when > 0)
    INT Grade;              // +0x1C crystal grade (NConsoleWnd::GetGradeText: "`D`")
    INT Count;              // +0x20 the amount (the stack, or the trade's offer)
    INT Extra24;            // +0x24 a shortcut skill's second number: " (<sysstring 91>:N)" after its level (> 0)
    INT bShowCount;         // +0x28 the amount is shown (" (N)")
    INT Type;               // +0x2C the item's Type (NItemEntry +0x190C: 0 weapon, 1 armor, 2 jewel, 3.., 5 etc);
                            //   for a shortcut: 1 item, 2 skill, 3 action, 4 macro; -1 none
    INT Level;              // +0x30 a skill's level
    INT ItemID;             // +0x34 the item's id (NItemEntry +0x1918), only stored
    NToolTipLineArray Lines;     // +0x38 the property lines (elements of 0x414 bytes)
    FString Description;    // +0x44 the item's / skill's description (the newlines escaped)
    DWORD Style;            // +0x50 NTT_*
    DWORD Options;          // +0x54 NTTO_*
    NToolTipRect FullBox;   // +0x58 (X, Y unused) the full box's text size: W (+0x60) and H (+0x64), by Layout
    NToolTipRect TitleBox;  // +0x68 (X, Y unused) the title line's size: W (+0x70) and H (+0x74), by Layout

    // out of line like the stock ones (the callers call 0x10002230 / 0x10002560 / 0x100022d0 / 0x1001aca0)
    __declspec(noinline) NToolTipInfo();                        // 0x10002230
    __declspec(noinline) NToolTipInfo(const NToolTipInfo& Src); // 0x10002560 (the copy a by-value argument is made of)
    __declspec(noinline) ~NToolTipInfo();                       // 0x100022d0 (the members' destructors)
    __declspec(noinline) NToolTipInfo& operator=(const NToolTipInfo& Src);  // 0x1001aca0

    // 0x10027500: the sizes of the boxes for the Style, from the texts and the font (the builders end with it)
    __declspec(noinline) void Layout();
    // 0x10028ac0: back to an empty tip (Name, Name2, Description, the lines and the numbers; Style / Options kept
    // by the builders' own stores)
    __declspec(noinline) void Clear();
    // 0x10028aa0: the text and its Style (NCButton::SetToolTip passes NTT_Plain), then Layout
    __declspec(noinline) void SetText(const TCHAR* InText, INT InStyle);
    // 0x10028b20 (ret 0xC): the tip of an item entry (or the skill / action / shortcut / recipe the entry
    // stands for) for a Style and Options
    __declspec(noinline) void Fill(NItemEntry* Item, DWORD InStyle, DWORD InOptions);
    // 0x10029ba0 (ret 8): a macro object's tip: it starts with its NToolTipInfo (`this` is the macro) and keeps
    // its name at +0x9EC and its description at +0xA06
    __declspec(noinline) void FillMacro(NToolTipInfo* Macro, DWORD InStyle);
    // 0x10029bf0 (ret 8): the party matching row's tip: the level, the class and the party's purpose, and the
    // row's text as the description (`this` is the row's NToolTipInfo)
    __declspec(noinline) void FillMatchRow(NListRow* Row, DWORD InStyle);

    // 0x100273c0 (cdecl; name ours): Value with a comma between thousands, in a static buffer
    static const TCHAR* FormatNumber(INT Value);
    // 0x10027470 (cdecl; name ours): the color of a formatted amount: white up to four digits, then pink, yellow,
    // green and cyan in turn as it grows
    static DWORD NumberColor(const TCHAR* Text);
};
NW_ASSERT_OFFSET(NToolTipInfo, Enchant, 0x18);
NW_ASSERT_OFFSET(NToolTipInfo, Type, 0x2C);
NW_ASSERT_OFFSET(NToolTipInfo, ItemID, 0x34);
NW_ASSERT_OFFSET(NToolTipInfo, Lines, 0x38);
NW_ASSERT_OFFSET(NToolTipInfo, Description, 0x44);
NW_ASSERT_OFFSET(NToolTipInfo, Style, 0x50);
NW_ASSERT_OFFSET(NToolTipInfo, FullBox, 0x58);
NW_ASSERT_OFFSET(NToolTipInfo, TitleBox, 0x68);
NW_ASSERT_SIZE(NToolTipInfo, 0x78);

struct NToolTipSkin
{
    UTexture* Tex[9];       // +0x00 "L2UI.Tooltip.Tool01" .. "Tool09" (corners, edges and fill, row by row), set by NCWnd::OnCreate
    NToolTipInfo Info;      // +0x24 the copy of the tip being drawn (Draw / DrawFull store it first)

    __declspec(noinline) NToolTipSkin();     // 0x10029f00
    __declspec(noinline) ~NToolTipSkin();    // 0x10029f20: the arrays and strings are emptied by hand, then the
                                             //   members go (0x100022d0)

    // 0x1002a230 (ret 0x94): the box of the title line (TitleBox + 10) with the title text (DrawTitleText) at the
    // position the owner gives in OutX / OutY (relative to the device's origin, the window's corner), which the
    // routine moves to keep the box on the screen. Info is taken BY VALUE: the routine destroys its copy.
    void Draw(NCGDevice* Device, NToolTipInfo Info, INT* OutX, INT* OutY, NToolTipRect Wnd);
    // 0x10029fa0 (ret 0x94): the same for the full box (FullBox + 10) with DrawFullText
    void DrawFull(NCGDevice* Device, NToolTipInfo Info, INT* OutX, INT* OutY, NToolTipRect Wnd);
    // 0x10028760 (ret 0xC, name ours): the title line at X, Y: "+N" (orange), the name, the additional name
    // (yellow), the grade, " (count)", and the first property line when Options has NTTO_Price / NTTO_PriceEach
    void DrawTitleText(NCGDevice* Device, INT X, INT Y);
    // 0x10027eb0 (ret 0xC, name ours): the full box's text: the title by Style / Type, the lines, the description
    void DrawFullText(NCGDevice* Device, INT X, INT Y);

private:
    // the clamp to the screen and the nine-slice both draw routines inline (name ours)
    void DrawFrame(NCGDevice* Device, INT* OutX, INT* OutY, const NToolTipRect& Wnd, INT W, INT H);
};
NW_ASSERT_SIZE(NToolTipSkin, 0x9C);

class NCCmdTarget : public NCObject
{
    NW_DECLARE_ABSTRACT_CLASS(NCCmdTarget, NCObject, 0, 0x10199FD0)
    // ctor: NCObject's (0x10020ca0, identical code folded); ~NCCmdTarget 0x10001690
};
NW_ASSERT_SIZE(NCCmdTarget, 0x28);

class NCWnd : public NCCmdTarget
{
    NW_DECLARE_CLASS(NCWnd, NCCmdTarget, 0, 0x101A72C0)

    NWndListNode* ChildHead;   // +0x28
    NWndListNode* ChildTail;   // +0x2C
    INT LocalX;                // +0x30 relative to the parent
    INT LocalY;                // +0x34
    INT LocalW;                // +0x38
    INT LocalH;                // +0x3C
    UBOOL bFocus;              // +0x40
    INT ScreenX;               // +0x44 absolute
    INT ScreenY;               // +0x48
    INT Width;                 // +0x4C
    INT Height;                // +0x50
    DWORD Style;               // +0x54 NWS_*
    NWndEntry* Entry;          // +0x58 own registration
    UBOOL bEnabled;            // +0x5C
    NCFrameCtrl* FrameCtrl;    // +0x60 title bar (NWS_TITLEBAR)
    INT FrameID;               // +0x64 id given to FrameCtrl (GNextFrameID++)
    INT DragMouseX;            // +0x68 last mouse position of a drag (NCIcon)
    INT DragMouseY;            // +0x6C
    NToolTipSkin* ToolTipSkin; // +0x70 new in C2 (name ours): made by OnCreate for every window, freed by OnDestroy
                               //   (the constructor does not touch it)
    NWndEntry* NextTab;        // +0x74 TAB goes there
    NWndEntry* PrevTab;        // +0x78 Shift+TAB goes there
    INT Unknown78;             // +0x7C the window number (GNextWndNumber): the section of its saved place in
                               //   the windows INI (NConsoleWnd::SaveWindowPositions reads it at +0x7C). Named
                               //   after its C1 offset, others use the name
    NWndEntry* StuckTopTo;     // +0x80 Snap (sticky windows): window each edge is stuck
    NWndEntry* StuckLeftTo;    // +0x84   to, NULL = screen edge; meaningful only while
    NWndEntry* StuckRightTo;   // +0x88   the matching bStuck* is set. Explicit placement
    NWndEntry* StuckBottomTo;  // +0x8C   (SetWindowRect, SetSize...) clears everything.
    BYTE bStuckTop;            // +0x90
    BYTE bStuckLeft;           // +0x91
    BYTE bStuckRight;          // +0x92
    BYTE bStuckBottom;         // +0x93
    NCIcon* Icon;              // +0x94 shown instead of the window while minimized

    NCWnd();                                                     // 0x1002bc10
    // ~NCWnd                                                    // 0x10001b20

    virtual __declspec(noinline) INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam); // 25 0x1002b500
    virtual UBOOL IsEnabled() { return bEnabled; }               // 26 0x10001b70
    virtual UBOOL IsInWindow(INT X, INT Y);                      // 27 0x1002b8f0 (name ours)
    virtual UBOOL IsFocused() { return bFocus; }                 // 28 0x10001b80 (name ours)
    virtual void SetFocus();                                     // 29 0x1002b800
    virtual void KillFocus();                                    // 30 0x1002b850
    virtual void SetStyle(DWORD NewStyle) { Style = NewStyle; }  // 31 0x10001b90
    virtual DWORD GetStyle() { return Style; }                   // 32 0x10001ba0
    virtual INT vfn032() { return 0; }                           // 33 0x10001ab0 (folded)
    virtual INT vfn033() { return 0; }                           // 34 0x10001ab0 (folded)
    virtual INT OnTimer(INT TimerID) { return 0; }               // 35 0x10001ae0 (folded) WM_TIMER
    virtual INT OnCreate();                                      // 36 0x1002c9e0
    virtual INT OnDestroy();                                     // 37 0x1002cd20
    virtual void SetTabOrder(NWndEntry* Next, NWndEntry* Prev);  // 38 0x1002b6c0 (name ours)
    virtual void EnableWindow(UBOOL bEnable);                    // 39 0x1002b6e0 (name ours)
    virtual UBOOL IsShowWindow();                                // 40 0x1002b780 (name ours)
    virtual UBOOL PtInRect(INT X, INT Y, INT RectX, INT RectY, INT RectW, INT RectH); // 41 0x1002b8b0
    virtual void SetSize(INT W, INT H);                          // 42 0x1002c560 (name ours)
    virtual void ShowWindow();                                   // 43 0x1002b710
    virtual void HideWindow();                                   // 44 0x1002b750
    virtual void MoveWindow(INT DX, INT DY);                     // 45 0x1002c8b0 (name ours) snaps
    virtual void OffsetWindow(INT DX, INT DY);                   // 46 0x10045d90 (name ours)
    virtual void Restore();                                      // 47 0x1002cbf0 (name ours)
    virtual void Minimize();                                     // 48 0x1002c640 (name ours)
    virtual void CenterInParent();                               // 49 0x1002c5d0 (name ours)
    virtual void SetTitle(const TCHAR* Title);                   // 50 0x1002ba30 (name ours)
    // new in C2 (names ours; the evidence is in the callers): 51/52 set and clear NWS_ITEMSELECT, 53/54 tell the
    // window manager (NCVirtualWndMain vslots 30 / 31, which are not declared there yet) about the entry and
    // set / clear NWS_MODAL; the dialog boxes call them after their own ShowWindow / HideWindow (0x10006dc0 /
    // 0x10006da0 are `ShowWindow(); vfn053(Entry)` and `HideWindow(); vfn054(Entry)`).
    virtual void SetItemSelect();                                // 51 0x1002baa0 (name ours)
    virtual void ClearItemSelect();                              // 52 0x1002bac0 (name ours)
    virtual void SetModal(NWndEntry* E);                         // 53 0x1002ba60 (name ours)
    virtual void ClearModal(NWndEntry* E);                       // 54 0x1002ba80 (name ours)
    virtual INT OnPaint(NCGDevice* Device) { return 0; }         // 55 0x10001ae0 (folded) WM_PAINT
    virtual INT OnDrawToolTip(NCGDevice* Device) { return 0; }   // 56 0x10001ae0 (folded) (name ours)
    virtual INT OnChar(WPARAM wParam, LPARAM lParam) { return 0; }          // 57 0x10001ac0 (folded)
    virtual INT OnKeyDown(WPARAM wParam, LPARAM lParam);                    // 58 0x1002b980
    virtual INT OnKeyUp(WPARAM wParam, LPARAM lParam);                      // 59 0x1002ba00
    virtual INT OnMouseMove(WPARAM wParam, LPARAM lParam) { return 0; }     // 60 0x10001ac0 (folded)
    virtual INT OnLButtonDown(WPARAM wParam, LPARAM lParam) { return 0; }   // 61 0x10001ac0 (folded)
    virtual INT OnLButtonUp(WPARAM wParam, LPARAM lParam) { return 0; }     // 62 0x10001ac0 (folded)
    virtual INT OnRButtonDown(WPARAM wParam, LPARAM lParam) { return 0; }   // 63 0x10001ac0 (folded)
    virtual INT OnRButtonUp(WPARAM wParam, LPARAM lParam) { return 0; }     // 64 0x10001ac0 (folded)
    virtual INT OnMove(WPARAM wParam, LPARAM lParam);                       // 65 0x1002b950 WM_MOVE
    virtual INT OnMouseWheel(WPARAM wParam, LPARAM lParam) { return 0; }    // 66 0x10001ac0 (folded)

    // non-virtual
    // C2: this is NCCmdTarget::WindowProc (vslot 25 of NCCmdTarget's vtable): WM_CREATE, WM_DESTROY and WM_TIMER
    // go to OnCreate / OnDestroy / OnTimer. NCWnd::WindowProc calls it first (C1: an inline emitted with
    // NCComboBox, 0x10004270)
    __declspec(noinline) INT DefWindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);   // 0x10004bb0 (name ours)
    void BringChildToTop(NWndEntry* Child);                      // 0x1002b790 (name ours)
    void OffsetScreen(INT DX, INT DY);                           // 0x1002b870 (name ours)
    NWndEntry* FindWindowAt(INT X, INT Y);                       // 0x1002b940 (name ours)
    void FormatNumber(TCHAR* Out, const TCHAR* Digits);          // 0x1002bae0 (name ours)
    UBOOL AddChildNode(NWndEntry* Child);                        // 0x1002bc80 (name ours)
    void RemoveChildNode(NWndEntry* Child);                      // 0x1002bce0 (name ours)
    void Snap(POINT* Delta);                                     // 0x1002bd50 (name ours) sticky windows
    void SetScreenPos(INT X, INT Y, const POINT* Origin);        // 0x1002c4e0 (name ours)
    NWndEntry* AddChildWindow(NCWnd* Child, DWORD ChildStyle);   // 0x1002c820 (name ours)
    void RemoveChildWindow(NWndEntry* Child);                    // 0x1002c860 (name ours)
    void SetWindowRect(INT X, INT Y, INT W, INT H);              // 0x1002c910 (name ours)

    // The second half of CreateChild on the child (name ours): an out-of-line copy of the
    // inline code for the call sites where the stock compiler stopped inlining it.
    void AttachEntry(NWndEntry* E);                              // 0x100031b0

    // Registers a newly created child the way every OnCreate does (inlined in the stock
    // code, name ours): entry under this window, screen position made absolute, WM_CREATE.
    // Forced inline: inside a guard (/EHa) our compiler kept it out of line.
    FORCEINLINE NWndEntry* CreateChild(NCWnd* Child, DWORD ChildStyle)
    {
        NWndEntry* E = AddChildWindow(Child, ChildStyle);
        Child->Entry = E;
        if (E->Parent)
        {
            NCWnd* Parent = E->Parent->Wnd;
            Child->ScreenX += Parent->ScreenX;
            Child->ScreenY += Parent->ScreenY;
        }
        Child->WindowProc(WM_CREATE, 0, 0);
        return E;
    }

    void ResetSticky()
    {
        StuckBottomTo = NULL;
        StuckTopTo = NULL;
        StuckLeftTo = NULL;
        StuckRightTo = NULL;
        bStuckBottom = 0;
        bStuckLeft = 0;
        bStuckRight = 0;
        bStuckTop = 0;
    }
};
NW_ASSERT_OFFSET(NCWnd, ChildHead, 0x28);
NW_ASSERT_OFFSET(NCWnd, LocalX, 0x30);
NW_ASSERT_OFFSET(NCWnd, bFocus, 0x40);
NW_ASSERT_OFFSET(NCWnd, ScreenX, 0x44);
NW_ASSERT_OFFSET(NCWnd, Style, 0x54);
NW_ASSERT_OFFSET(NCWnd, Entry, 0x58);
NW_ASSERT_OFFSET(NCWnd, FrameCtrl, 0x60);
NW_ASSERT_OFFSET(NCWnd, ToolTipSkin, 0x70);
NW_ASSERT_OFFSET(NCWnd, NextTab, 0x74);
NW_ASSERT_OFFSET(NCWnd, StuckTopTo, 0x80);
NW_ASSERT_OFFSET(NCWnd, bStuckTop, 0x90);
NW_ASSERT_OFFSET(NCWnd, Icon, 0x94);
NW_ASSERT_SIZE(NCWnd, 0x98);

#pragma pack(pop)