/*=============================================================================
    NCGDevice.cpp - paint device (UCanvas wrapper with window origin).
    Stock: 0x1000c700 .. 0x1000d25b, 0x1000d6a0 .. 0x1000d806, 0x1000dd30 .. 0x1000dd5b (the
    origins' deque is NStdLib.cpp's).
=============================================================================*/
#include "nwindow/NCGDevice.h"
#include "nwindow/NL2Font.h"
#include "nwindow/NCWnd.h"
#include "nwindow/NConsoleWnd.h"

#include <wchar.h>

/*-----------------------------------------------------------------------------
    Construction / destruction
-----------------------------------------------------------------------------*/

// 0x100108c0 - the first measure ("c") sets LineHeight; C2 adds the Thai array at +0x60 (zeroed)
NCGDevice::NCGDevice()
    : Canvas(NULL), OriginX(0), OriginY(0)
{
    SIZE Size;
    GetTextExtent(&Size, TEXT("c"), 0, 0, 0, 0, 0, L2FONT_Normal);
    LineHeight = Size.cy;
}
NW_HOOK_CTOR(0x100108c0, NCGDevice)

// 0x10010980 - ConditionalDestroy, the Thai array's destructor, the origins' Tidy, ~NCObject
// (NW_DECLARE_CLASS + members)
NW_HOOK_DTOR(0x10010980, NCGDevice)

// 0x1000fca0 (vslot 10) - the text align parser is shared by the whole package; C2 clears it
// before the delete
void NCGDevice::Destroy()
{
    Super::Destroy();
    if (GTextAlignParser)
    {
        GTextAlignParser->Clear();
        delete GTextAlignParser;
        GTextAlignParser = NULL;
    }
}
NW_HOOK_METHOD(0x1000fca0, NCGDevice, Destroy)

// 0x1000fc00 - C2 makes the parser only for GLanguageType 1, and without looking at the old one
void NCGDevice::Init()
{
    SmallDigits = FindTexture(TEXT("SEK.cbui13"));
    LargeDigits = FindTexture(TEXT("L2UI.NWindow.Number"));
    if (GLanguageType == 1)
        GTextAlignParser = new FL2TextAlignParser;
}
NW_HOOK_METHOD(0x1000fc00, NCGDevice, Init)

// 0x1000f410 - NCVirtualWndMain::SetGDevice, each frame
UBOOL NCGDevice::SetCanvas(UCanvas* InCanvas)
{
    Canvas = InCanvas;
    return 1;
}
NW_HOOK_METHOD(0x1000f410, NCGDevice, SetCanvas)

// 0x1000f420 - NCVirtualWndMain::ReleaseGDevice
void NCGDevice::ClearCanvas()
{
    Canvas = NULL;
}
NW_HOOK_METHOD(0x1000f420, NCGDevice, ClearCanvas)

/*-----------------------------------------------------------------------------
    Text
-----------------------------------------------------------------------------*/

// 0x1000f380 - one character of the font FontType; Size is zeroed first and stays so for a font or a
// glyph that does not exist (C1: the normal font only, Size untouched)
void NCGDevice::GetCharSize(SIZE* Size, TCHAR C, INT FontType)
{
    UL2Font* Font = GCanvas->L2Fonts[FontType];
    Size->cx = 0;
    Size->cy = 0;
    if (Font)
    {
        FL2FontChar* Char = Font->GetChar(C);
        if (Char)
        {
            Size->cx = Char->Width;
            Size->cy = Char->Height;
        }
    }
}
NW_HOOK_FUNC(0x1000f380, NCGDevice::GetCharSize)

// 0x1000f3d0 - measures with the viewport canvas, not with a device. C2: the font is the 8th
// argument (C1 always measured with the normal one) and P5 is not turned into 0/1
DWORD NCGDevice::GetTextExtent(SIZE* Size, const TCHAR* Text, DWORD P3, DWORD P4, INT P5, INT P7, DWORD P8,
                               L2FontType FontType)
{
    return GCanvas->GetTextExtent(Text, Size, P3, P4, P5, FontType, P7, P8);
}
NW_HOOK_FUNC(0x1000f3d0, NCGDevice::GetTextExtent)

// 0x1000f430 - DrawText's (see there) with the ranges and the three arguments the canvas's
// colored text takes (1, the origin's move since BeginPaint, 1). The 13th argument is not read.
DWORD NCGDevice::DrawColoredLine(INT X, INT Y, DWORD Color, const TCHAR* Text, DWORD P5, DWORD P6,
                                 DWORD P7, L2FontType FontType, INT P9, INT P10, DWORD P11,
                                 TArray<FL2ColorFontInfo*>* ColorInfo, INT P13)
{
    if (!Canvas)
        return 0;
    if (!Text)
        return 0;
    SIZE Size;
    GetTextExtent(&Size, Text, P5, P6, X, 0, 0, L2FONT_Normal);  // X really is the 5th argument, as in DrawText
    LineHeight = Size.cy;
    Touch(OriginY + Y, Size.cy);
    return Canvas->DrawTextToCanvas(OriginX + X, OriginY + Y, Color, Text, 0, P5, P6, 0.f, P7, FontType, P9,
                                    P10, P11, 1, OriginX - PaintOriginX, 1, ColorInfo, 0,
                                    (EFontExceptionType)0);
}
NW_HOOK_METHOD(0x1000f430, NCGDevice, DrawColoredLine)

/*-----------------------------------------------------------------------------
    The Thai text path (new in C2) and the line wrapper.
-----------------------------------------------------------------------------*/

// 0x1000b150 (name ours) - how many characters the cluster holds
INT ThaiCharacter::CharCount() const
{
    INT Count = 1;
    if (W[1] != 0)
    {
        if (W[2] != 0)
            Count = 2;
        if (W[3] != 0)
            Count++;
        if (W[0] != 0)
            Count++;
        if (W[4] != 0)
            Count++;
        if (W[6] != 0)
            Count++;
    }
    return Count;
}
NW_HOOK_METHOD(0x1000b150, ThaiCharacter, CharCount)

// 0x1000fba0 (name ours) - GCanvas->LoadFont (vslot 34)
void NCGDevice::LoadFont(INT Index, TCHAR* P2, TCHAR* P3)
{
    GCanvas->LoadFont(Index, P2, P3);
}
NW_HOOK_FUNC(0x1000fba0, NCGDevice::LoadFont)

// 0x1000fbc0 (name ours) - GCanvas->DestroyFont (vslot 35)
void NCGDevice::DestroyFont(INT Index)
{
    GCanvas->DestroyFont(Index);
}
NW_HOOK_FUNC(0x1000fbc0, NCGDevice::DestroyFont)

// 0x1000fbe0 (name ours) - GCanvas->MakeThaiCharArray: Text split into clusters, Buffer (as long as Text) gets
// the characters in the order the clusters give them
void NCGDevice::MakeThaiCharArray(const TCHAR* Text, TArray<ThaiCharacter>& Chars, TCHAR* Buffer)
{
    GCanvas->MakeThaiCharArray(Text, Chars, Buffer);
}
NW_HOOK_FUNC(0x1000fbe0, NCGDevice::MakeThaiCharArray)

// width of one glyph of Font added to Width (the Thai measuring in both wrappers)
static FORCEINLINE void AddGlyphWidth(UL2Font* Font, TCHAR C, DWORD& Width)
{
    FL2FontChar* Char = Font->GetChar(C);
    if (Char)
        Width += Char->Width;
}

// the width of one cluster is that of its glyphs: the mark at +0xE alone when there is one, else the base at +2 and
// the one at +0, else the one at +0 or the one at +0xA
static FORCEINLINE void AddClusterWidth(UL2Font* Font, const ThaiCharacter& Cluster, DWORD& Width)
{
    if (Cluster.W[7] != 0)
        AddGlyphWidth(Font, Cluster.W[7], Width);
    else if (Cluster.W[1] != 0)
    {
        AddGlyphWidth(Font, Cluster.W[1], Width);
        if (Cluster.W[0] != 0)
            AddGlyphWidth(Font, Cluster.W[0], Width);
    }
    else if (Cluster.W[0] != 0)
        AddGlyphWidth(Font, Cluster.W[0], Width);
    else if (Cluster.W[5] != 0)
        AddGlyphWidth(Font, Cluster.W[5], Width);
}

// 0x1000fed0 - the first line of Text that fits MaxWidth pixels (NULL when none does). English: the text align
// parser; Thai (GLanguageType 5): clusters; else one character at a time (a line break or a control character is
// skipped). The line is built in GWrapLine, the text copied to GWrapText, GWrapPos is how far it was taken.
// C2 adds the Thai branch.
TCHAR* NCGDevice::FirstWrappedLine(const TCHAR* Text, DWORD MaxWidth, INT FontType)
{
    DWORD LineWidth = 0;
    if (!Text || !GCanvas)
        return NULL;
    UL2Font* Font = GCanvas->L2Fonts[FontType];
    if (Font)
    {
        if (GLanguageType == 1 && GTextAlignParser)
        {
            INT LineStart = 0;   // GetLineBuffer's out value: the offset in Text where the line starts (not a length)
            GTextAlignParser->Clear();
            GTextAlignParser->SetParserParam((TCHAR*)Text, MaxWidth, Font, 0, 0, 1);
            return GTextAlignParser->GetLineBuffer(LineStart);
        }
        if (GLanguageType != 5)
        {
            wcscpy(GWrapText, Text);
            GWrapLen = (INT)wcslen(GWrapText);
            GWrapPos = 0;
            GWrapLine[0] = 0;
            INT Count = 0;
            if (GWrapText[0])
            {
                const TCHAR* p = GWrapText;
                do
                {
                    TCHAR C = *p++;
                    if (GWrapPos >= GWrapLen)
                        break;
                    if (C == 0x0A || C < ' ')
                        GWrapPos++;
                    else
                    {
                        FL2FontChar* Char = Font->GetChar(C);
                        if (Char)
                        {
                            LineWidth = (DWORD)((DOUBLE)Char->Width + (DOUBLE)LineWidth);
                            if (LineWidth > MaxWidth)
                                break;
                        }
                        GWrapPos++;
                        GWrapLine[Count++] = C;
                    }
                } while (*p);
            }
            GWrapLine[Count] = 0;
            return Count ? GWrapLine : NULL;
        }

        // Thai
        TArray<ThaiCharacter> Chars;
        TCHAR* Buffer = new TCHAR[appStrlen(Text) + 1];
        MakeThaiCharArray(Text, Chars, Buffer);
        wcscpy(GWrapText, Buffer);
        GWrapLen = (INT)wcslen(GWrapText);
        GWrapLine[0] = 0;
        UL2Font* ThaiFont = GCanvas->L2Fonts[FontType];
        GWrapPos = 0;
        INT Taken = 0;
        if (ThaiFont)
        {
            for (INT i = 0; i < Chars.Num(); i++)
            {
                ThaiCharacter Cluster = Chars(i);
                AddClusterWidth(ThaiFont, Cluster, LineWidth);
                if (MaxWidth < LineWidth)
                    break;
                Taken++;
                GWrapPos += Cluster.CharCount();
            }
            INT Total = 0;
            for (INT i = 0; i < Taken; i++)
                Total += Chars(i).CharCount();
            if (Total > 0 && Buffer)
            {
                appStrncpy(GWrapLine, Buffer, Total + 1);
                GWrapLine[Total + 1] = 0;   // one past the terminator appStrncpy wrote, as in the stock code
                delete[] Buffer;
                return GWrapLine;
            }
        }
    }
    return NULL;
}
NW_HOOK_FUNC(0x1000fed0, NCGDevice::FirstWrappedLine)

// 0x100102b0 - continues where the previous line stopped (GWrapPos)
TCHAR* NCGDevice::NextWrappedLine(DWORD MaxWidth, INT FontType)
{
    DWORD LineWidth = 0;
    if (!GCanvas)
        return NULL;
    if (GLanguageType == 1 && GTextAlignParser)
    {
        INT LineStart = 0;   // GetLineBuffer's out value: the offset in Text where the line starts (not a length)
        GTextAlignParser->SetWidthParam(MaxWidth);
        return GTextAlignParser->GetLineBuffer(LineStart);
    }
    if (GLanguageType == 5)
    {
        if (GWrapPos < GWrapLen)
        {
            TCHAR* Rest = &GWrapText[GWrapPos];
            TArray<ThaiCharacter> Chars;
            TCHAR* Buffer = new TCHAR[appStrlen(Rest) + 1];
            MakeThaiCharArray(Rest, Chars, Buffer);
            GWrapLine[0] = 0;
            UL2Font* Font = GCanvas->L2Fonts[FontType];
            INT Taken = 0;
            if (Font)
            {
                for (INT i = 0; i < Chars.Num(); i++)
                {
                    ThaiCharacter Cluster = Chars(i);
                    AddClusterWidth(Font, Cluster, LineWidth);
                    if (MaxWidth < LineWidth)
                        break;
                    Taken++;
                    GWrapPos += Cluster.CharCount();
                }
                INT Total = 0;
                for (INT i = 0; i < Taken; i++)
                    Total += Chars(i).CharCount();
                if (Total > 0 && Buffer)
                {
                    appStrncpy(GWrapLine, Buffer, Total + 1);
                    GWrapLine[Total + 1] = 0;   // one past the terminator appStrncpy wrote, as in the stock code
                    delete[] Buffer;
                    return GWrapLine;
                }
            }
        }
    }
    else if (GWrapPos < GWrapLen)
    {
        GWrapLine[0] = 0;
        UL2Font* Font = GCanvas->L2Fonts[FontType];
        INT Count = 0;
        const TCHAR* p = &GWrapText[GWrapPos];
        if (Font)
        {
            for (TCHAR C = *p; C; C = *p)
            {
                p++;
                if (GWrapPos >= GWrapLen)
                    break;
                if (C == 0x0A || C < ' ')
                    GWrapPos++;
                else
                {
                    FL2FontChar* Char = Font->GetChar(C);
                    if (Char)
                    {
                        LineWidth = (DWORD)((DOUBLE)Char->Width + (DOUBLE)LineWidth);
                        if (LineWidth > MaxWidth)
                            break;
                    }
                    GWrapPos++;
                    GWrapLine[Count++] = C;
                }
            }
            GWrapLine[Count] = 0;
            return Count ? GWrapLine : NULL;
        }
    }
    return NULL;
}
NW_HOOK_FUNC(0x100102b0, NCGDevice::NextWrappedLine)

// 0x1000f510 - text at (X, Y) of the device origin; LineHeight and the dirty range follow the
// measured text. C2: the canvas's DrawText is DrawTextToCanvas (vslot 46) with two more
// arguments (0, 0), the measure takes the font (0), and the 15th argument is not read.
DWORD NCGDevice::DrawText(INT X, INT Y, DWORD Color, const TCHAR* Text, DWORD P5, DWORD P6, DWORD P7,
                          L2FontType FontType, INT P9, INT P10, DWORD P11, INT P12, INT P13, INT P14,
                          INT P15)
{
    if (!Canvas)
        return 0;
    if (!Text)
        return 0;
    SIZE Size;
    GetTextExtent(&Size, Text, P5, P6, X, 0, 0, L2FONT_Normal);  // X really is the 5th argument in the stock code
    LineHeight = Size.cy;
    Touch(OriginY + Y, Size.cy);
    return Canvas->DrawTextToCanvas(OriginX + X, OriginY + Y, Color, Text, 0, P5, P6, 0.f, P7, FontType, P9,
                                    P10, P11, P12, P13, P14, NULL, 0, (EFontExceptionType)0);
}
NW_HOOK_METHOD(0x1000f510, NCGDevice, DrawText)

// 0x1000f950
void NCGDevice::Draw3DCoordText(FVector Loc, DWORD Color, TCHAR* Text, UTexture* Tex, INT P5, INT P6,
                                L2FontType FontType)
{
    guard(NCGDevice::Draw3DCoordText);
    Canvas->Draw3DCoordText(Loc, Color, Text, Tex, P5, P6, FontType, 0);
    unguard;
}
NW_HOOK_METHOD(0x1000f950, NCGDevice, Draw3DCoordText)

// 0x1000fb40 - digits from a texture strip: 5 px wide small set or 8 px wide large set
DWORD NCGDevice::DrawSpecialDigit(UBOOL bLarge, INT X, INT Y, DWORD Color, TCHAR* Text)
{
    if (!Text || !Canvas)
        return 0;
    if (!bLarge)
        return Canvas->DrawSpecialDigit(5, OriginX + X, OriginY + Y, Color, Text, SmallDigits);
    return Canvas->DrawSpecialDigit(8, OriginX + X, OriginY + Y, Color, Text, LargeDigits);
}
NW_HOOK_METHOD(0x1000fb40, NCGDevice, DrawSpecialDigit)

/*-----------------------------------------------------------------------------
    Shapes and textures
-----------------------------------------------------------------------------*/

// 0x1000f5f0
void NCGDevice::DrawRect(INT X, INT Y, INT XL, INT YL, DWORD Color)
{
    UTexture* White = FindTexture(TEXT("NWindow.WhiteTexture"));
    Canvas->DrawRect(OriginX + X, OriginY + Y, XL, YL, White, Color);
    Touch(OriginY + Y, YL);
}
NW_HOOK_METHOD(0x1000f5f0, NCGDevice, DrawRect)

// 0x1000f660
void NCGDevice::DrawStretchedTex(INT X, INT Y, INT XL, INT YL, UTexture* Tex)
{
    if (Tex && (Tex->USize || Tex->VSize))
    {
        Canvas->DrawStretchedTex(OriginX + X, OriginY + Y, XL, YL, Tex);
        Touch(OriginY + Y, YL);
    }
}
NW_HOOK_METHOD_SIG(0x1000f660, NCGDevice, DrawStretchedTex, void (NCGDevice::*)(INT, INT, INT, INT, UTexture*))

// 0x1000f6d0
void NCGDevice::DrawStretchedTex(INT X, INT Y, INT XL, INT YL, INT SrcX, INT SrcY, INT SrcXL, INT SrcYL,
                                 UTexture* Tex)
{
    if (Tex && (Tex->USize || Tex->VSize))
    {
        Canvas->DrawStretchedTex(OriginX + X, OriginY + Y, XL, YL, SrcX, SrcY, SrcXL, SrcYL, Tex);
        Touch(OriginY + Y, YL);
    }
}
NW_HOOK_METHOD_SIG(0x1000f6d0, NCGDevice, DrawStretchedTex,
                   void (NCGDevice::*)(INT, INT, INT, INT, INT, INT, INT, INT, UTexture*))

// 0x1000f750
void NCGDevice::DrawTexture(INT X, INT Y, INT XL, INT YL, INT U, INT V, INT UL, INT VL, UTexture* Tex)
{
    if (Tex && (Tex->USize || Tex->VSize))
    {
        Canvas->DrawTexture(OriginX + X, OriginY + Y, XL, YL, (FLOAT)U, (FLOAT)V, (FLOAT)UL, (FLOAT)VL, Tex);
        Touch(OriginY + Y, YL);
    }
}
NW_HOOK_METHOD(0x1000f750, NCGDevice, DrawTexture)

// 0x1000f7e0
void NCGDevice::DrawBrightenTexture(INT X, INT Y, INT XL, INT YL, INT U, INT V, INT UL, INT VL, UTexture* Tex,
                                    DWORD Brightness)
{
    if (Tex && (Tex->USize || Tex->VSize))
    {
        Canvas->DrawBrightenTexture(OriginX + X, OriginY + Y, XL, YL, (FLOAT)U, (FLOAT)V, (FLOAT)UL, (FLOAT)VL,
                                    Tex, Brightness);
        Touch(OriginY + Y, YL);
    }
}
NW_HOOK_METHOD(0x1000f7e0, NCGDevice, DrawBrightenTexture)

// 0x1000f8d0
void NCGDevice::DrawLine(INT X1, INT Y1, INT X2, INT Y2, DWORD Color, INT Width)
{
    Canvas->DrawLine(OriginX + X1, OriginY + Y1, OriginX + X2, OriginY + Y2, Color, Width);
    Touch(OriginY + Y1, 0);
    Touch(OriginY + Y2, 0);
}
NW_HOOK_METHOD(0x1000f8d0, NCGDevice, DrawLine)

// 0x1000fa00 - clan crest, 16x12 taken at (0,4) of the crest texture
UBOOL NCGDevice::DrawPledgeCrest(INT X, INT Y, INT CrestID)
{
    if (CrestID <= 0)
        return 0;
    UTexture* Crest = GConsole->GetPledgeCrestTexture(CrestID);
    if (!Crest || (!Crest->USize && !Crest->VSize))
        return 0;
    Canvas->DrawTexture(OriginX + X, OriginY + Y, 16, 12, 0.f, 4.f, 16.f, 12.f, Crest);
    Touch(OriginY + Y, 12);
    return 1;
}
NW_HOOK_METHOD(0x1000fa00, NCGDevice, DrawPledgeCrest)

// 0x1000fa90 - alliance crest, 8x12
UBOOL NCGDevice::DrawAllyCrest(INT X, INT Y, INT CrestID)
{
    if (CrestID <= 0)
        return 0;
    UTexture* Crest = GConsole->GetAllyCrestTexture(CrestID);
    if (!Crest || (!Crest->USize && !Crest->VSize))
        return 0;
    Canvas->DrawTexture(OriginX + X, OriginY + Y, 8, 12, 0.f, 4.f, 8.f, 12.f, Crest);
    Touch(OriginY + Y, 12);
    return 1;
}
NW_HOOK_METHOD(0x1000fa90, NCGDevice, DrawAllyCrest)

/*-----------------------------------------------------------------------------
    Canvas passthroughs
-----------------------------------------------------------------------------*/

// 0x1000f880
void NCGDevice::DrawMinimapLoc(INT X, INT Y, UTexture* Tex)
{
    Canvas->DrawMinimapLoc(OriginX + X, OriginY + Y, Tex);
}
NW_HOOK_METHOD(0x1000f880, NCGDevice, DrawMinimapLoc)

// 0x1000f8b0
void NCGDevice::DrawRadar()
{
    Canvas->DrawRadar();
}
NW_HOOK_METHOD(0x1000f8b0, NCGDevice, DrawRadar)

// 0x1000f8c0 - radar coordinates are absolute (no origin)
void NCGDevice::DrawRadarTarget(INT X, INT Y)
{
    Canvas->DrawRadarTarget(X, Y);
}
NW_HOOK_METHOD(0x1000f8c0, NCGDevice, DrawRadarTarget)

// 0x1000fb20 - clip rectangles are absolute (no origin)
void NCGDevice::SetClip(INT X, INT Y, INT XL, INT YL)
{
    Canvas->SetClip(X, Y, XL, YL);
}
NW_HOOK_METHOD(0x1000fb20, NCGDevice, SetClip)

// 0x1000fb30
void NCGDevice::ReleaseClip()
{
    Canvas->ReleaseClip();
}
NW_HOOK_METHOD(0x1000fb30, NCGDevice, ReleaseClip)

/*-----------------------------------------------------------------------------
    Tool tip box (no stock function: inlined in NCButton::OnDrawToolTip 0x10001dd0
    and NCIcon::OnDrawToolTip 0x10015910).
-----------------------------------------------------------------------------*/

// Clip, background and text are placed relative to the device origin (the window): above it
// (Y = -height) or below it (Y = Wnd->Height), moved left when the box would leave the screen.
void NCGDevice::DrawToolTipBox(NCWnd* Wnd, const TCHAR* Text, UTexture* Back, UBOOL bAbove)
{
    SIZE Size;
    GetTextExtent(&Size, Text, 0, 0, 0, 0, 0, L2FONT_Normal);
    INT ScreenW = GConsole->Desktop->LocalW;
    INT X = 0;
    INT ClipX = Wnd->ScreenX;
    if (Wnd->ScreenX + Size.cx > ScreenW)
    {
        X = ScreenW - Size.cx - Wnd->ScreenX;
        ClipX = ScreenW - Size.cx;
    }
    INT Y = bAbove ? -Size.cy : Wnd->Height;
    SetClip(ClipX, Wnd->ScreenY + Y, Size.cx, Size.cy);
    if (Back)
        DrawStretchedTex(X, Y, Size.cx, Size.cy, Back);
    DrawText(X, Y, 0xFFFFFFFF, Text, 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0, 0);
    ReleaseClip();
}

/*-----------------------------------------------------------------------------
    Origin stack
-----------------------------------------------------------------------------*/

// 0x10010f70
void NCGDevice::PushOrigin(INT DX, INT DY)
{
    POINT Move;
    Move.x = DX;
    Move.y = DY;
    Origins.PushBack(Move);
    OriginX += Move.x;
    OriginY += Move.y;
}
NW_HOOK_METHOD(0x10010f70, NCGDevice, PushOrigin)

// 0x100109f0 - the last move undone (nothing checks that there is one)
void NCGDevice::PopOrigin()
{
    POINT& Move = Origins.Back();
    OriginX -= Move.x;
    OriginY -= Move.y;
    Origins.PopBack();
}
NW_HOOK_METHOD(0x100109f0, NCGDevice, PopOrigin)

NW_CHECK_VTABLE(NCGDevice, 0x1010aa50, 25)
