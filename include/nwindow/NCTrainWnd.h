#pragma once
// NCTrainWnd - the skill trainer window (UClass "CTrainWnd", 0x9C; desktop +0xE8): a title
// bar ("Skill Trainer", sysstring 477) over two pages in the same place, the skill list
// (NCTrainListWnd) and the details of one skill (NCTrainInfoWnd). The console drives it: a new
// list empties both pages and shows the list (vslot 151), each row comes with 153; a row
// picked asks the server for the skill, whose answer (152) fills and shows the info page and
// its required items (154). "Learn" asks the server to learn it, "List" goes back.

#include "nwindow/NCWnd.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCButton;
class NCTrainListBox;
struct NItemEntry;
struct NItemInfo;
struct NTrainEntry;

// NWM_COMMAND ids of the pages' controls
enum
{
    NID_TRAIN_FRAME = 4,
    NID_TRAIN_LEARN = 2,
    NID_TRAIN_LIST = 3,
    NID_TRAIN_LISTBOX = 0x1A04,
};

// A skill as the info page shows it (names ours; 0x4C0 bytes in C2, 0x2C0 in C1: Name and IconName are
// 256 characters). The console fills one from the
// server's answer and hands it to SetInfo; the page keeps its own (NCTrainInfoWnd::Info), whose
// Items are its required item entries.
struct NTrainSkillInfo
{
    TCHAR Name[256];           // +0x000
    TCHAR IconName[256];       // +0x200
    FString Desc;              // +0x400
    TCHAR Unknown20C[64];      // +0x40C drawn after the level
    UTexture* Icon;            // +0x48C
    NItemEntry* Items[4];      // +0x490 required items: the page's are made by its OnCreate
                               //   (NItemEntry::UnknownC34 1: shown) and freed here
    INT SkillID;               // +0x4A0 sent with Level by "Learn"
    INT Level;                 // +0x4A4
    INT Unknown2A8;            // +0x4A8
    INT SP;                    // +0x4AC required SP
    INT MpCost;                // +0x4B0
    INT Range;                 // +0x4B4 not shown when negative
    INT Unknown2B8;            // +0x4B8
    INT Unknown2BC;            // +0x4BC

    NTrainSkillInfo();                                               // 0x10047260
    ~NTrainSkillInfo();                                              // 0x1004ddf0 frees Items
};
NW_ASSERT_OFFSET(NTrainSkillInfo, Desc, 0x400);
NW_ASSERT_OFFSET(NTrainSkillInfo, Icon, 0x48C);
NW_ASSERT_OFFSET(NTrainSkillInfo, SkillID, 0x4A0);
NW_ASSERT_OFFSET(NTrainSkillInfo, Unknown2BC, 0x4BC);
NW_ASSERT_SIZE(NTrainSkillInfo, 0x4C0);

// NCTrainListWnd - the list page (UClass "CTrainListWnd", 0x9C): "Skill list" (sysstring 364)
// over the NCTrainListBox, the player's SP at the bottom right.
class NCTrainListWnd : public NCWnd
{
    NW_DECLARE_CLASS(NCTrainListWnd, NCWnd, 0, 0x101DF280)

    UTexture* BackTex;                     // +0x98 "L2UI.SkillTrain.TrainerBack1"
    NCTrainListBox* ListBox;               // +0x9C

    NCTrainListWnd();                                                // 0x100dcb20
    // ~NCTrainListWnd                                               // 0x100dcb70

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x100dc190
    virtual INT OnCreate();                                          // 36 0x100de000
    virtual INT OnDestroy();                                         // 37 0x100dcbc0 (NCPetActionWnd's)
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x100dcc00
    // name ours
    virtual INT OnListCommand(LPARAM lParam);                        // 67 0x100dcd90 a row picked

    // names ours
    void Clear();                                                    // 0x100b2b90 (the list box's Clear, vslot 70)
    void AddEntry(const NTrainEntry* Entry);                         // 0x100dde40
};
NW_ASSERT_OFFSET(NCTrainListWnd, ListBox, 0x9C);
NW_ASSERT_SIZE(NCTrainListWnd, 0xA0);

// NCTrainInfoWnd - the skill page (UClass "CTrainInfoWnd", 0x360): "Skill explanation" (366)
// with the icon, name, level, MP cost, range and description; "Learning condition" (367) with
// the required SP and up to four required items; the player's SP; "Learn" and "List".
class NCTrainInfoWnd : public NCWnd
{
    NW_DECLARE_CLASS(NCTrainInfoWnd, NCWnd, 0, 0x101E01F0)

    UTexture* BackTex;                     // +0x98 "L2UI.SkillTrain.TrainerBack3"
    NTrainSkillInfo Info;                  // +0x9C the skill shown (0x4C0 bytes in C2; C1 +0x9C, 0x2C0)
    NCButton* LearnButton;                 // +0x55C "Learn" (sysstring 368)
    NCButton* ListButton;                  // +0x560 "List" (sysstring 369)

    NCTrainInfoWnd();                                                // 0x100de0e0
    // ~NCTrainInfoWnd                                               // 0x100de170

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x100dc1f0
    virtual INT OnCreate();                                          // 36 0x100dce90
    virtual INT OnDestroy();                                         // 37 0x100dd0a0
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x100dd110
    // names ours
    virtual INT OnLearn();                                           // 67 0x100dc3f0
    virtual INT OnList();                                            // 68 0x100dc480

    // names ours
    void SetInfo(NTrainSkillInfo* Src);                              // 0x100dc260
    void Reset();                                                    // 0x100dc380
    void AddRequiredItem(NItemInfo* Item);                           // 0x100dd9b0 into a free row
};
NW_ASSERT_OFFSET(NCTrainInfoWnd, Info, 0x9C);
NW_ASSERT_OFFSET(NCTrainInfoWnd, LearnButton, 0x55C);
NW_ASSERT_SIZE(NCTrainInfoWnd, 0x564);

class NCTrainWnd : public NCWnd
{
    NW_DECLARE_CLASS(NCTrainWnd, NCWnd, 0, 0x101DFCD0)

    NCTrainListWnd* ListWnd;               // +0x98
    NCTrainInfoWnd* InfoWnd;               // +0x9C

    NCTrainWnd();                                                    // 0x100ddaa0
    // ~NCTrainWnd                                                   // 0x100ddaf0

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x10043ef0 (folded)
    virtual INT OnCreate();                                          // 36 0x100de1d0
    virtual INT OnDestroy();                                         // 37 0x100acef0
    virtual void ShowWindow();                                       // 43 0x1007b460 (folded)
    virtual void HideWindow();                                       // 44 0x1007b470 (folded)
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x100ddb40

    // the console's (names ours)
    void Reset();                                                    // 0x100dc520 both pages
    void ShowList();                                                 // 0x100dc4d0
    void ShowInfo();                                                 // 0x100dc4f0
    void SetSkillInfo(NTrainSkillInfo* Info);                        // 0x100dc510
    void AddRequiredItem(NItemInfo* Item);                           // 0x100ddbf0
    void AddEntry(const NTrainEntry* Entry);                         // 0x100bca90 (C1 only: no C2 match)
};
NW_ASSERT_OFFSET(NCTrainWnd, ListWnd, 0x98);
NW_ASSERT_SIZE(NCTrainWnd, 0xA0);

#pragma pack(pop)