#pragma once
// NCMagicSkillWnd - the skill window (UClass "CMagicSkillWnd", 0x9C; a page of the character
// window, NCMainWnd): the NCSkillWnd lists on "L2UI.SkillWnd.skillBack". Shown, it empties
// the lists and asks the server for the skill list; hidden, it empties them.

#include "nwindow/NCWnd.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCSkillWnd;
class NCGMSkillWnd;

// NWM_COMMAND ids: the skill list (both windows), the GM window's title bar
enum { NID_MAGICSKILL_LIST = 0x1F01, NID_GMMAGICSKILL_FRAME = 0xF0F117 };

class NCMagicSkillWnd : public NCWnd
{
    NW_DECLARE_CLASS(NCMagicSkillWnd, NCWnd, 0, 0x101C0980)

    UTexture* BackTex;                     // +0x98 "L2UI.SkillWnd.skillBack"   // C2: offset derived, no evidence
    NCSkillWnd* SkillWnd;                  // +0x9C

    NCMagicSkillWnd();                                               // 0x10091620
    // ~NCMagicSkillWnd                                              // 0x10091670

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x10043ef0 (folded)
    virtual INT OnCreate();                                          // 36 0x100916c0
    virtual INT OnDestroy();                                         // 37 0x10091790 (NCPetActionWnd's)
    virtual void ShowWindow();                                       // 43 0x100918d0
    virtual void HideWindow();                                       // 44 0x10091950
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x10091840

    // names ours
    void ClearSkills();                                              // 0x10090ba0
    void ReloadSkillTexts();                                         // 0x10091960 new language
};
NW_ASSERT_OFFSET(NCMagicSkillWnd, BackTex, 0x98);
NW_ASSERT_OFFSET(NCMagicSkillWnd, SkillWnd, 0x9C);
NW_ASSERT_SIZE(NCMagicSkillWnd, 0xA0);

// NCGMMagicSkillWnd - a GM's view of another player's skills (UClass "CGMMagicSkillWnd", 0x9C;
// desktop window): an NCGMSkillWnd under a title bar, on the same background. Hidden, it
// empties the lists; the console fills them.
class NCGMMagicSkillWnd : public NCWnd
{
    NW_DECLARE_CLASS(NCGMMagicSkillWnd, NCWnd, 0, 0x101B5548)

    NCGMSkillWnd* SkillWnd;                // +0x98
    UTexture* BackTex;                     // +0x9C "L2UI.SkillWnd.skillBack"

    NCGMMagicSkillWnd();                                             // 0x100746e0
    // ~NCGMMagicSkillWnd                                            // 0x10074730

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x10043ef0 (folded)
    virtual INT OnCreate();                                          // 36 0x10074780
    virtual INT OnDestroy();                                         // 37 0x100748e0
    virtual void ShowWindow();                                       // 43 0x1007b460 (folded)
    virtual void HideWindow();                                       // 44 0x10074a20
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x10074990

    // names ours
    void ClearSkills();                                              // 0x100730c0
};
NW_ASSERT_OFFSET(NCGMMagicSkillWnd, SkillWnd, 0x98);
NW_ASSERT_OFFSET(NCGMMagicSkillWnd, BackTex, 0x9C);
NW_ASSERT_SIZE(NCGMMagicSkillWnd, 0xA0);

#pragma pack(pop)