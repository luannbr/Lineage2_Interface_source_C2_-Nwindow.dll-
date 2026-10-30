#pragma once
// NCListCtrl - list with columns (UClass "CListCtrl", 0x20C): a header button per column
// (NCHeaderCtrl; a click sorts the rows by that column, the next click reverses the order),
// rows of cells under them (text, color, optional icon), scroll buttons and a slider on the
// right. The owner builds each row (NListRow, one NListCell per column) and hands it over
// with AddRow; from then on the list owns it. A double click reports the row (NWM_COMMAND,
// lParam = index); the selected row shows its tool tip while the mouse is over it.
// The server list of the login screen, the clan members, party matching and the siege
// lists derive from it (NCServerSelectListCtrl, NCMatchListCtrl, NCSiegeInfoListCtrl).

#include "nwindow/NControl.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCPushButton;
class NCSliderBar;

// NWM_COMMAND id of the header buttons (NotifyID = column index)
enum { NID_LISTCTRL_HEADER = 0xF5 };

// A cell (name ours; 0x1820 bytes in C2, 0xA1C in C1: the texts got longer). What the list draws: Icon
// centered in the column, then Text after IconW pixels, in Color.
struct NListCell
{
    TCHAR Text[2048];          // +0x0000 (C1: 1024)
    UTexture* Icon;            // +0x1000
    DWORD Color;               // +0x1004 (-1: white)
    TCHAR Text2[1024];         // +0x1008 not drawn by the list (the owners keep data here) (C1: 256)
    INT Param1;                // +0x1808 (-1) for the owner
    INT Param2;                // +0x180C (-1)
    INT Param3;                // +0x1810 new in C2 (SetParams' third argument; the constructor leaves it)
    INT UnknownA10;            // +0x1814 (0)
    INT IconW;                 // +0x1818
    INT IconH;                 // +0x181C

    // inlined in NListRow's constructor
    NListCell()
    {
        Text[0] = 0;
        Icon = NULL;
        Color = 0xFFFFFFFF;
        Text2[0] = 0;
        Param1 = -1;
        Param2 = -1;
        UnknownA10 = 0;
        IconW = 0;
        IconH = 0;
    }
    void SetText(const TCHAR* InText);                   // 0x1001c2e0
    void SetNumber(INT Value);                           // 0x1001c310 "%d"
    void SetIcon(UTexture* InIcon, INT W, INT H);        // 0x1001c330 (NULL keeps the old one)
    void SetText2(const TCHAR* InText);                  // 0x1001c360
    void SetParams(INT InParam1, INT InParam2, INT InParam3 = 0); // 0x1001c390 (C1: two arguments)
    void SetColor(DWORD InColor);                        // 0x1001c3c0
};
NW_ASSERT_OFFSET(NListCell, Icon, 0x1000);
NW_ASSERT_OFFSET(NListCell, Text2, 0x1008);
NW_ASSERT_OFFSET(NListCell, Param1, 0x1808);
NW_ASSERT_OFFSET(NListCell, IconW, 0x1818);
NW_ASSERT_SIZE(NListCell, 0x1820);

// A row (name ours; 0x888 bytes in C2, created by the owner with new).
struct NListRow
{
    NListCell* Cells;          // +0x000 one per column
    TCHAR Text[1024];          // +0x004 not drawn by the list
    INT Param1;                // +0x804 (-1) for the owner
    INT Param2;                // +0x808 (-1)
    INT Param3;                // +0x80C new in C2 (SetParams' third argument)
    NToolTipInfo ToolTip;      // +0x810 shown while the selected row is under the mouse (C1: TCHAR[1024]); the
                               //   constructor / destructor are NToolTipInfo's (0x10002230 / 0x100022d0)

    NListRow(INT NumCells);                              // 0x1001d720
    ~NListRow();                                         // 0x1001d7f0 (C2: only frees Cells and the tool tip)
    void SetText(const TCHAR* InText);                   // 0x1001c4c0
    void SetParams(INT InParam1, INT InParam2, INT InParam3 = 0); // 0x1001c4f0 (C1: two arguments)
    const TCHAR* GetToolTip();                           // C2: no stock function (C1: NULL when the text is empty)
};
NW_ASSERT_OFFSET(NListRow, Text, 0x4);
NW_ASSERT_OFFSET(NListRow, Param1, 0x804);
NW_ASSERT_OFFSET(NListRow, ToolTip, 0x810);
NW_ASSERT_SIZE(NListRow, 0x888);

// What a column is made of (name ours; 0x1814 bytes, built on the owner's stack and handed
// to NCListCtrl::AddColumn).
struct NListColumnInfo
{
    INT Width;                 // +0x000
    INT Height;                // +0x004 of the header; the rows start under it
    TCHAR Title[1024];         // +0x008
    UBOOL bClickable;          // +0x808 (1) a click on the header sorts
    UBOOL bNumeric;            // +0x80C (0) sort by value (_wtol) instead of by text
    INT Unknown810;            // +0x810 (left as it was on the stack)
    TCHAR NormalTex[1024];     // +0x814 header textures, "NWindow.BlackTexture" /
    TCHAR PushedTex[1024];     // +0x1014  "NWindow.WhiteTexture" when empty

    // inlined in the owners
    NListColumnInfo()
    {
        Title[0] = 0;
        bClickable = 1;
        bNumeric = 0;
        NormalTex[0] = 0;
        PushedTex[0] = 0;
    }
    void SetTitleID(INT TextID);                         // 0x100187d0 sysstring line
    void SetTitle(const TCHAR* InTitle);                 // 0x10018830
    void SetNormalTex(const TCHAR* Name);                // 0x10018860
    void SetPushedTex(const TCHAR* Name);                // 0x10018890
};
NW_ASSERT_OFFSET(NListColumnInfo, Title, 0x8);
NW_ASSERT_OFFSET(NListColumnInfo, bClickable, 0x808);
NW_ASSERT_OFFSET(NListColumnInfo, NormalTex, 0x814);
NW_ASSERT_OFFSET(NListColumnInfo, PushedTex, 0x1014);
NW_ASSERT_SIZE(NListColumnInfo, 0x1814);

// NCHeaderCtrl - a column's header button (UClass "CHeaderCtrl", 0x9C0): its texture
// stretched, the title centered, pushed while the mouse button is down. Released over it,
// it tells the list (NID_LISTCTRL_HEADER, lParam = NotifyID = column index).
class NCHeaderCtrl : public NControl
{
    NW_DECLARE_CLASS(NCHeaderCtrl, NControl, 0, 0x101A31E0)

    UTexture* NormalTex;             // +0x1A4
    UTexture* PushedTex;             // +0x1A8
    TCHAR Title[1024];               // +0x1AC
    UBOOL bReleased;                 // +0x9AC (1)
    UBOOL bClickable;                // +0x9B0 (1)
    UBOOL bNumeric;                  // +0x9B4 (0) the column sorts by value
    UBOOL bAscending;                // +0x9B8 (1) order of the next sort; each sort flips it
    INT ColumnWidth;                 // +0x9BC
    INT ColumnHeight;                // +0x9C0

    NCHeaderCtrl();                                                  // 0x1001cbf0
    // ~NCHeaderCtrl                                                 // 0x1001cc90

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x10090b60 (folded)
    virtual INT OnDestroy();                                         // 37 0x1001c5d0 (folded)
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x1001cce0
    virtual INT OnLButtonDown(WPARAM wParam, LPARAM lParam);         // 61 0x1001c520
    virtual INT OnLButtonUp(WPARAM wParam, LPARAM lParam);           // 62 0x1001c580

    void SetColumnInfo(NListColumnInfo* Info);                       // 0x1001c650 (name ours)
};
NW_ASSERT_OFFSET(NCHeaderCtrl, Title, 0x1AC);
NW_ASSERT_OFFSET(NCHeaderCtrl, bReleased, 0x9AC);
NW_ASSERT_OFFSET(NCHeaderCtrl, ColumnWidth, 0x9BC);
NW_ASSERT_SIZE(NCHeaderCtrl, 0x9C4);

class NCListCtrl : public NControl
{
    NW_DECLARE_CLASS(NCListCtrl, NControl, 0, 0x101A3708)

    UTexture* BackTex;               // +0x1A4 set by the owner
    UTexture* HighlightTex;          // +0x1A8 "NWindow.MenuHighlight" (not drawn here)
    UTexture* ToolTipTex;            // +0x1AC "L2UI.tooltip.mini-tool01" (not drawn here)
    UTexture* TipTopTex;             // +0x1B0 "L2UI.Tooltip.Tool02"  the row tool tip: top,
    UTexture* TipMidTex;             // +0x1B4 "L2UI.Tooltip.Tool01"  a strip per text line,
    UTexture* TipBottomTex;          // +0x1B8 "L2UI.Tooltip.Tool03"  bottom
    TArray<NListRow*> Rows;          // +0x1BC
    TArray<NCHeaderCtrl*> Columns;   // +0x1C8
    NCPushButton* ScrollUpButton;    // +0x1D4 the scroll controls come with the first column
    NCPushButton* ScrollDownButton;  // +0x1D8
    NCSliderBar* ScrollBar;          // +0x1DC
    INT VisibleRows;                 // +0x1E0 (10)
    INT MaxRows;                     // +0x1E4 (100) AddRow refuses more
    INT MaxColumns;                  // +0x1E8 (3)
    INT TopRow;                      // +0x1EC first row shown
    INT NumRows;                     // +0x1F0 rows when the scroll range was last set
    INT Selected;                    // +0x1F4 -1 = none
    INT HoverRow;                    // +0x1F8 row under the mouse, -1 = none
    INT Unknown1F8;                  // +0x1FC (16; 12 from the sized constructor), never read
    INT HeaderHeight;                // +0x200 the rows start under the headers (AddColumn)
    INT TextX;                       // +0x204 where the cells start
    INT TextY;                       // +0x208
    INT LineGap;                     // +0x20C (1) rows are 12 + LineGap high

    NCListCtrl();                                                    // 0x1001d820
    NCListCtrl(INT InMaxRows, INT InVisibleRows, INT InMaxColumns, INT Unused); // 0x1001d9a0
    // ~NCListCtrl                                                   // 0x1001d930

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x1001c700
    virtual INT OnCreate();                                          // 36 0x1001c7e0
    virtual INT OnDestroy();                                         // 37 0x1001dd10
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x1001ce50
    virtual INT OnDrawToolTip(NCGDevice* Device);                    // 56 0x1001ded0
    virtual INT OnMouseMove(WPARAM wParam, LPARAM lParam);           // 60 0x1001c970
    virtual INT OnLButtonDown(WPARAM wParam, LPARAM lParam);         // 61 0x1001c930
    virtual INT OnMouseWheel(WPARAM wParam, LPARAM lParam);          // 66 0x1001c9f0
    // names ours
    virtual void Clear();                                            // 68 0x1001dc30 rows and columns
    virtual INT OnLButtonDblClick(WPARAM wParam, LPARAM lParam);     // 69 0x1001c9a0
    virtual INT ScrollUp();                                          // 70 0x1001caa0
    virtual INT ScrollDown();                                        // 71 0x1001cae0
    virtual INT SetTopRow(INT Row);                                  // 72 0x1001cb30
    virtual INT SortByColumn(INT Column);                            // 73 0x1001d240
    // text of the row tool tip, drawn at (X, Y) in its box (the subclasses draw it)
    void DrawRowToolTip(NCGDevice* Device, NListRow* Row, INT X, INT Y) {} // C2: slot gone (was 69) (folded)

    // names ours
    INT HitTest(INT X, INT Y);                                       // 0x1001c850 row, -1 = none
    TArray<NListRow*>* GetRows();                                    // 0x1001cb50
    INT GetRowCount();                                               // 0x1001cb60
    void SetSelected(INT Index);                                     // 0x1001cb70
    void SetTextOffset(INT X, INT Y);                                // 0x1001cb90
    void UpdateScrollRange();                                        // 0x1001cbb0
    NListRow* GetSelectedRow();                                      // 0x1001d120
    void SetRepeatDelay(INT Delay);                                  // 0x1001d140
    void InitScrollBar();                                            // 0x10019840 (C1 only: no C2 match)
    INT AddRow(NListRow* Row);                                       // 0x1001d420
    void ClearRows();                                                // 0x10019ca0 (C1 only: no C2 match)
    void CreateScrollControls();                                     // 0x1001d470
    INT AddColumn(INT Index, NListColumnInfo* Info);                 // 0x1001da90
};
NW_ASSERT_OFFSET(NCListCtrl, Rows, 0x1BC);
NW_ASSERT_OFFSET(NCListCtrl, Columns, 0x1C8);
NW_ASSERT_OFFSET(NCListCtrl, ScrollBar, 0x1DC);
NW_ASSERT_OFFSET(NCListCtrl, TopRow, 0x1EC);
NW_ASSERT_OFFSET(NCListCtrl, Selected, 0x1F4);
NW_ASSERT_OFFSET(NCListCtrl, HeaderHeight, 0x200);
NW_ASSERT_OFFSET(NCListCtrl, LineGap, 0x20C);
NW_ASSERT_SIZE(NCListCtrl, 0x210);

#pragma pack(pop)