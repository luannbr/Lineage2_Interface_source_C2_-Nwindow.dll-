/*=============================================================================
    NL2Font.cpp - L2 bitmap font. Stock: 0x10072220 .. 0x10072e39 (+ the engine templates the
    font code needs: UGlobalTempObjects::AddGlobalObject 0x10072850, ConstructObject<UFinalBlend>
    0x100727e0).
=============================================================================*/
#include "nwindow/NL2Font.h"

#include <string.h>

// (GFontBytesRead, GFontBlendA, GFontBlendB: src/standalone/NwGlobals.cpp in the standalone build)

/*-----------------------------------------------------------------------------
    FL2FontPage: the v1.0 pages (16-byte glyphs).
-----------------------------------------------------------------------------*/

// 0x10072460
FL2FontPage::FL2FontPage()
{
    First = 0;
    Count = 0;
    Chars = NULL;
}
NW_HOOK_CTOR(0x10072460, FL2FontPage)

// 0x100728e0 - the vtable, then Free (a tail jump). The vector destructor of Clear and the font's
// destructor reach it
FL2FontPage::~FL2FontPage()
{
    Free();
}
NW_HOOK_DTOR(0x100728e0, FL2FontPage)

// 0x10072490 (vslot 27) - the font's page texture (a 4-byte body that only the vtable refers to)
UTexture* NL2Font::GetTexture()
{
    return Texture;
}
NW_HOOK_METHOD(0x10072490, NL2Font, GetTexture)

// 0x10072220 - the glyph of C when it is on this page (the stock NL2Font::GetChar keeps ecx across this
// call: the callers are ours too)
FL2FontChar* FL2FontPage::Find(TCHAR C)
{
    INT Index = (_WORD)C;
    if (Index >= First && Index < First + Count)
        return &Chars[Index - First];
    return NULL;
}
NW_HOOK_METHOD(0x10072220, FL2FontPage, Find)

// 0x10072540 (vslot 0) - C2: only a missing file fails (C1 also refused an empty page); Count glyphs of 16
// bytes read one by one, counted in GFontBytesRead
UBOOL FL2FontPage::Load(INT InFirst, INT InCount, FILE* File)
{
    if (!File)
        return 0;
    First = InFirst;
    Count = InCount;
    Chars = (FL2FontChar*)appMalloc(InCount << 4, TEXT("new"));
    for (INT i = 0; i < Count; i++)
        GFontBytesRead += (INT)fread((BYTE*)Chars + i * 0x10, 1, 0x10, File);
    return 1;
}
NW_HOOK_METHOD(0x10072540, FL2FontPage, Load)

// 0x100725c0
void FL2FontPage::Free()
{
    if (Chars)
    {
        appFree(Chars);
        Chars = NULL;
    }
}
NW_HOOK_METHOD(0x100725c0, FL2FontPage, Free)

/*-----------------------------------------------------------------------------
    FL2FontPageEx: the v1.1 pages (20-byte glyphs and a table of 32 words).
-----------------------------------------------------------------------------*/

// 0x100728f0 - the vtable of this class and the base's three zeros (ExChars and Table are left alone)
FL2FontPageEx::FL2FontPageEx()
{
}
NW_HOOK_CTOR(0x100728f0, FL2FontPageEx)

// 0x10072910 - Free (of the base's Chars, which a v1.1 page never fills: ExChars leaks, as in the stock
// code), then the base destructor (Free again)
FL2FontPageEx::~FL2FontPageEx()
{
    Free();
}
NW_HOOK_DTOR(0x10072910, FL2FontPageEx)

// 0x100725f0 - like Load, with the page's 128-byte table (read by NL2Font::Load before the glyphs) and
// 20-byte glyphs; a missing file or table fails
UBOOL FL2FontPageEx::LoadEx(INT InFirst, INT InCount, FILE* File, const INT* InTable)
{
    if (!File)
        return 0;
    if (!InTable)
        return 0;
    First = InFirst;
    Count = InCount;
    ExChars = (FL2FontCharEx*)appMalloc(InCount * 0x14, TEXT("new"));
    for (INT i = 0; i < 32; i++)
        Table[i] = InTable[i];
    for (INT i = 0; i < Count; i++)
        GFontBytesRead += (INT)fread((BYTE*)ExChars + i * 0x14, 1, 0x14, File);
    return 1;
}
NW_HOOK_METHOD(0x100725f0, FL2FontPageEx, LoadEx)

// 0x10072250
FL2FontCharEx* FL2FontPageEx::FindEx(TCHAR C)
{
    INT Index = (_WORD)C;
    if (Index >= First && Index < First + Count)
        return &ExChars[Index - First];
    return NULL;
}
NW_HOOK_METHOD(0x10072250, FL2FontPageEx, FindEx)

// 0x10072280 - when C is on this page and its glyph has the variant Mask names (a single bit; the loop finds
// the highest set bit, 32 when there is none), the table's word of that bit; else -1
__declspec(noinline) INT FL2FontPageEx::GetMaskValue(TCHAR C, INT Mask)
{
    INT Index = (_WORD)C;
    if (Index >= First && Index < First + Count)
    {
        INT Bit = 0;
        do
        {
            if ((Mask >> Bit) == 1)
                break;
            Bit++;
        } while (Bit < 32);
        if (ExChars[Index - First].Mask & Mask)
            return Table[Bit];
    }
    return -1;
}
NW_HOOK_METHOD(0x10072280, FL2FontPageEx, GetMaskValue)

/*-----------------------------------------------------------------------------
    The engine templates the font code instantiates.
-----------------------------------------------------------------------------*/

// 0x10072850
void UGlobalTempObjects::AddGlobalObject(UObject** Object)
{
    guard(UGlobalTempObjects::AddGlobalObject);
    GlobalObjects(GlobalObjects.Add()) = Object;
    unguard;
}
NW_HOOK_METHOD(0x10072850, UGlobalTempObjects, AddGlobalObject)

// 0x100727e0 - UE2's ConstructObject<UFinalBlend> (UnObjBas.h 1194): the class must be a UFinalBlend (or a
// child of it); an Outer of -1 is the transient package
static __declspec(noinline) UFinalBlend* ConstructFinalBlend(UClass* Class, UObject* Outer, FName Name, DWORD SetFlags)
{
    UBOOL bChild = 0;
    for (UField* Test = reinterpret_cast<UField*>(Class); Test; Test = Test->SuperField)
    {
        if (Test == reinterpret_cast<UField*>(UFinalBlend::StaticClass()))
        {
            bChild = 1;
            break;
        }
    }
    NW_CHECK_AT(bChild, "e:\\work\\localization\\2004_12_01\\warfare2110\\core\\inc\\UnObjBas.h", 1194);
    if (Outer == (UObject*)-1)
        Outer = (UObject*)UObject::GetTransientPackage();
    return (UFinalBlend*)UObject::StaticConstructObject(Class, Outer, Name, SetFlags, NULL, GError, NULL);
}
NW_HOOK_FUNC(0x100727e0, ConstructFinalBlend)

/*-----------------------------------------------------------------------------
    NL2Font.
-----------------------------------------------------------------------------*/

// 0x100722d0
NL2Font::NL2Font()
{
    Texture = NULL;
    Header0 = 0;
    Header1 = 0;
    NumPages = 0;
    Pages = NULL;
    PagesEx = NULL;
    Kind = 0;
}
NW_HOOK_CTOR(0x100722d0, NL2Font)

// 0x100724e0 - ConditionalDestroy, then Engine's ~UL2Font
NW_HOOK_DTOR(0x100724e0, NL2Font)

// 0x10072960 (vslot 26) - 0 = loaded, -1 = no file or a failing page (the file stays open then, as in the
// stock code). C2: a version word ("v1.0" or "v1.1"; any other first four bytes are the first header int, the
// file is read again from the start as a v1.0 one) picks the page table, and the texture is looked up after
// the file is read
INT NL2Font::Load(const TCHAR* TextureName, const TCHAR* FontFile)
{
    FILE* File;
    if (GUnicodeOS)
        File = _wfopen(FontFile, L"rb");
    else
        File = fopen(TCHAR_TO_ANSI(FontFile), "rb");
    if (!File)
        return -1;

    char Version[8];
    fread(Version, 1, 4, File);
    Version[4] = 0;
    if (strcmp(Version, "v1.0") == 0)
    {
        Kind = 0;
        GFontBytesRead += 4;
    }
    else if (strcmp(Version, "v1.1") == 0)
    {
        GFontBytesRead += 4;
        Kind = 1;
    }
    else
    {
        Kind = 0;
        fseek(File, 0, SEEK_SET);
    }

    GFontBytesRead += (INT)fread(&Header0, 1, 4, File);
    GFontBytesRead += (INT)fread(&Header1, 1, 4, File);
    GFontBytesRead += (INT)fread(&NumPages, 1, 4, File);
    if (NumPages)
    {
        if (Kind == 0)
        {
            Pages = new FL2FontPage[NumPages];
            for (INT i = 0; i < NumPages; i++)
            {
                INT PageFirst, PageCount;
                GFontBytesRead += (INT)fread(&PageFirst, 1, 4, File);
                GFontBytesRead += (INT)fread(&PageCount, 1, 4, File);
                FL2FontPage* Page = &Pages[i];
                if (!Page->Load(PageFirst, PageCount, File))
                    return -1;
            }
        }
        else if (Kind == 1)
        {
            PagesEx = new FL2FontPageEx[NumPages];
            for (INT i = 0; i < NumPages; i++)
            {
                INT PageFirst, PageCount;
                INT Table[32];
                GFontBytesRead += (INT)fread(&PageFirst, 1, 4, File);
                GFontBytesRead += (INT)fread(&PageCount, 1, 4, File);
                GFontBytesRead += (INT)fread(Table, 1, 0x80, File);
                if (!PagesEx[i].LoadEx(PageFirst, PageCount, File, Table))
                    return -1;
            }
        }
    }
    fclose(File);

    Texture = (UTexture*)StaticFindObject(UTexture::StaticClass(), ANY_PACKAGE, TextureName, 0);
    if (!Texture)
        Texture = (UTexture*)StaticLoadObject(UTexture::StaticClass(), NULL, TextureName, NULL, 1, NULL);
    if (Texture)
        Texture->SetFlags(RF_Native);
    return 0;
}
NW_HOOK_METHOD(0x10072960, NL2Font, Load)

// 0x10072310 (vslot 28) - the first page holding a glyph with a size, in the table the font has
FL2FontChar* NL2Font::GetChar(TCHAR C)
{
    if (Kind == 0)
    {
        if (Pages)
        {
            for (INT i = 0; i < NumPages; i++)
            {
                FL2FontChar* Char = Pages[i].Find(C);
                if (Char && Char->Width > 0 && Char->Height > 0)
                    return Char;
            }
        }
    }
    else if (Kind == 1)
    {
        if (PagesEx)
        {
            for (INT i = 0; i < NumPages; i++)
            {
                FL2FontCharEx* Char = PagesEx[i].FindEx(C);
                if (Char && Char->Width > 0 && Char->Height > 0)
                    return Char;
            }
        }
    }
    return NULL;
}
NW_HOOK_METHOD(0x10072310, NL2Font, GetChar)

// 0x100723b0 (vslot 29) - the variant mask of the glyph in the v1.1 table; 0 without that table
INT NL2Font::GetCharMask(TCHAR C)
{
    if (!PagesEx)
        return 0;
    for (INT i = 0; i < NumPages; i++)
    {
        FL2FontCharEx* Char = PagesEx[i].FindEx(C);
        if (Char && Char->Width > 0 && Char->Height > 0)
            return Char->Mask;
    }
    return 0;
}
NW_HOOK_METHOD(0x100723b0, NL2Font, GetCharMask)

// 0x10072410 (vslot 33) - the first page's table word for the mask bit (the glyph on that page has it); 0
INT NL2Font::GetCharMaskValue(TCHAR C, INT Mask)
{
    if (!PagesEx)
        return 0;
    for (INT i = 0; i < NumPages; i++)
    {
        INT Value = PagesEx[i].GetMaskValue(C, Mask);
        if (Value >= 0)
            return Value;
    }
    return 0;
}
NW_HOOK_METHOD(0x10072410, NL2Font, GetCharMaskValue)

// 0x100724a0 (vslot 32)
UBOOL NL2Font::HasTexture()
{
    return Texture != NULL;
}
NW_HOOK_METHOD(0x100724a0, NL2Font, HasTexture)

// 0x100726a0 (vslot 31) - a UFinalBlend over the font texture: made the first time (kept in a global, listed
// in GGlobalTempObjects), refilled every time. Mode 1 and the others have the same code on two globals.
UMaterial* NL2Font::GetMaterial(INT Mode)
{
    if (!GetTexture())
        return NULL;
    UFinalBlend** Slot = (Mode == 1) ? &GFontBlendA : &GFontBlendB;
    if (!*Slot)
    {
        *Slot = ConstructFinalBlend(UFinalBlend::StaticClass(), (UObject*)-1, NAME_None, 0);
        GGlobalTempObjects->AddGlobalObject((UObject**)Slot);
    }
    // UFinalBlend (a UModifier): Material +0x38, FrameBufferBlending +0x3C (2: alpha blend), the bits at +0x40
    // (ZWrite 1, ZTest 2, AlphaTest 4, TwoSided 8), AlphaRef +0x44 (the class is only forward declared)
    nw::Field<UMaterial*>(*Slot, 0x38) = (UMaterial*)GetTexture();
    nw::Field<DWORD>(*Slot, 0x40) |= 8;
    nw::Field<BYTE>(*Slot, 0x3C) = 2;
    nw::Field<DWORD>(*Slot, 0x40) |= 1;
    nw::Field<DWORD>(*Slot, 0x40) |= 2;
    nw::Field<DWORD>(*Slot, 0x40) |= 4;
    nw::Field<BYTE>(*Slot, 0x44) = 0;
    return *Slot;
}
NW_HOOK_METHOD(0x100726a0, NL2Font, GetMaterial)

// 0x10072d70 (vslot 30) - the pages of both tables go, the counters and the texture too (the texture loses
// its RF_Native flag): C2 clears the pointers where C1 left them
void NL2Font::Clear()
{
    for (INT i = 0; i < NumPages; i++)
    {
        if (Pages)
            Pages[i].Free();
        if (PagesEx)
            PagesEx[i].Free();
    }
    NumPages = 0;
    Header0 = 0;
    Header1 = 0;
    if (Pages)
    {
        delete[] Pages;
        Pages = NULL;
    }
    if (PagesEx)
    {
        delete[] PagesEx;
        PagesEx = NULL;
    }
    if (Texture)
    {
        Texture->ClearFlags(RF_Native);
        Texture = NULL;
    }
}
NW_HOOK_METHOD(0x10072d70, NL2Font, Clear)

NW_CHECK_VTABLE_FOR(NL2Font, UObject, 0x101165f8, 34)
