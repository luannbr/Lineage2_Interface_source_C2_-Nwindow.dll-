#pragma once
// NItemInfo - what the console and the item windows pass around about one icon of an item,
// skill or action (name ours; 0x17A8 bytes in C2, 0xC9C in C1; built on the stack). The inventory
// fills most of it (NConsoleWnd::AddInventoryItem...); the action window only the names, the icon
// and two numbers. The item windows keep a copy of it in each NItemEntry (NCItemWnd.h), where the
// fields' use shows. Field meanings beyond those are not known yet.
//
// C2 layout: derived from the constructor (0x10040800: which dwords it stores and the -1s), the copy
// into an NItemEntry (0x1001A9A0: field by field) and the readers (NCInvenItemWnd::AddItem,
// NCEquipItemWnd::AddItem, NCInventoryWnd::AddItem). The strings are bigger and the Description
// is four times as long; the ints start at +0x1700 (C1: +0xC00), with two new fields in the middle
// and one at the end of the string part.

#include "nwindow/NWindow.h"

struct NItemInfo
{
    TCHAR Name[256];           // +0x000 (C1: 128 characters)
    TCHAR AdditionalName[256]; // +0x200 new in C2 (FL2GameData::GetItemAdditionalName: NItemEntry shows it after the name)
    TCHAR IconName[128];       // +0x400 texture of the icon (the windows look it up) (C1 +0x100)
    TCHAR Unknown200[128];     // +0x500 the second icon's texture (C1 +0x200)
    TCHAR Unknown300[128];     // +0x600 the third icon's (C1 +0x300)
    TCHAR Description[2048];   // +0x700 (C1 +0x400, 1024 characters)
    UTexture* Icon;            // +0x1700 (C1 +0xC00)
    INT UnknownC04;            // +0x1704 the second icon (C1 +0xC04)
    INT UnknownC08;            // +0x1708 the third icon (C1 +0xC08)
    INT Param;                 // +0x170C (-1) actions: FL2ActionData::Param; items: the type
                               //   (0..3: the enchant shows, NCQuestItemWnd)
    INT Unknown1710;           // +0x1710 (-1) new in C2: NItemEntry +0x1910, which the entry's display name looks at
                               //   (7, 0xC, 0xD: no "+N" before the name)
    INT ObjectID;              // +0x1714 items: the server's id of this one (C1 +0xC10)
    INT ID;                    // +0x1718 actions: FL2ActionData::ActionID (C1 +0xC14)
    INT Count;                 // +0x171C (C1 +0xC18)
    INT UnknownC1C;            // +0x1720 (C1 +0xC1C)
    INT UnknownC20;            // +0x1724 (-1) (C1 +0xC20)
    INT BodyPart;              // +0x1728 items: slot mask (NConsoleWnd::GetItemSlotText) (C1 +0xC24)
    INT UnknownC28;            // +0x172C (C1 +0xC28)
    INT UnknownC2C;            // +0x1730 (-1) (C1 +0xC2C)
    INT UnknownC30;            // +0x1734 (-1) (C1 +0xC30)
    INT UnknownC34[13];        // +0x1738 (C1 +0xC34)
    INT SubType;               // +0x176C (-1) items: (NConsoleWnd::GetItemSlotText) (C1 +0xC68)
    INT UnknownC6C;            // +0x1770 (C1 +0xC6C)
    INT UnknownC70;            // +0x1774 (C1 +0xC70)
    INT Enchant;               // +0x1778 (C1 +0xC74)
    INT UnknownC78;            // +0x177C (C1 +0xC78)
    INT UnknownC7C;            // +0x1780 (C1 +0xC7C)
    INT UnknownC80;            // +0x1784 (C1 +0xC80)
    INT Unknown1788;           // +0x1788 new in C2 (the constructor clears it last; NItemEntry +0x19AC)
    SWORD UnknownC84;          // +0x178C a word in C2 (C1: an INT at +0xC84; the engine's ItemInfo has the same word)
    SWORD _Unknown178E;        // +0x178E padding: the constructor does not touch it
    UBOOL bStackable;          // +0x1790 the count shows next to the name (C1 +0xC88)
    INT UnknownC8C;            // +0x1794 items: worn (NCInventoryWnd::AddItem sends it to the equipment window) (C1 +0xC8C)
    INT UnknownC90;            // +0x1798 (-1) (C1 +0xC90)
    INT UnknownC94;            // +0x179C (C1 +0xC94)
    INT Unknown17A0;           // +0x17A0 new in C2 (NItemEntry +0x1998)
    INT UnknownC98;            // +0x17A4 (C1 +0xC98)

    NItemInfo();                                                     // 0x10040800
};
NW_ASSERT_OFFSET(NItemInfo, AdditionalName, 0x200);
NW_ASSERT_OFFSET(NItemInfo, IconName, 0x400);
NW_ASSERT_OFFSET(NItemInfo, Unknown200, 0x500);
NW_ASSERT_OFFSET(NItemInfo, Unknown300, 0x600);
NW_ASSERT_OFFSET(NItemInfo, Description, 0x700);
NW_ASSERT_OFFSET(NItemInfo, Icon, 0x1700);
NW_ASSERT_OFFSET(NItemInfo, Param, 0x170C);
NW_ASSERT_OFFSET(NItemInfo, Unknown1710, 0x1710);
NW_ASSERT_OFFSET(NItemInfo, ID, 0x1718);
NW_ASSERT_OFFSET(NItemInfo, BodyPart, 0x1728);
NW_ASSERT_OFFSET(NItemInfo, UnknownC34, 0x1738);
NW_ASSERT_OFFSET(NItemInfo, SubType, 0x176C);
NW_ASSERT_OFFSET(NItemInfo, Enchant, 0x1778);
NW_ASSERT_OFFSET(NItemInfo, Unknown1788, 0x1788);
NW_ASSERT_OFFSET(NItemInfo, UnknownC84, 0x178C);
NW_ASSERT_OFFSET(NItemInfo, bStackable, 0x1790);
NW_ASSERT_OFFSET(NItemInfo, UnknownC90, 0x1798);
NW_ASSERT_OFFSET(NItemInfo, Unknown17A0, 0x17A0);
NW_ASSERT_SIZE(NItemInfo, 0x17A8);
