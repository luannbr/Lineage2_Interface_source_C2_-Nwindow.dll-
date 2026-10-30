#pragma once
// NCGDevice - the paint device handed to every OnPaint: a UCanvas plus the window origin
// and the vertical range touched since the last reset (UClass "CGDevice", sizeof 0x6C in C2).
// Each Draw* forwards to the UCanvas method of the same name with the origin applied.
//
// ---------------------------------------------------------------------------------------------
// C2 SIGNATURES (read from the stock code, checked against the C2 callers; C1 -> C2 changes):
//
//   static DWORD GetTextExtent(SIZE* Size, const TCHAR* Text, DWORD P3, DWORD P4, INT P5,
//                              INT P7, DWORD P8, L2FontType FontType);         // cdecl, 8 args
//       C1 had 7 (no FontType): every C2 caller pushes one more word FIRST (the last argument),
//       the caller pops 0x20 instead of 0x1C. The word is the font the text is measured with
//       (UCanvas::L2Fonts index, 0 = L2FONT_Normal); it reaches UCanvas::GetTextExtent (vslot 51)
//       as its 6th argument. P5 is passed through as is (the C1 hook turned it into 0/1).
//       The stock NCGDevice::DrawText / DrawColoredLine call it as (&Size, Text, P5, P6, X, 0, 0, 0)
//       (X as the 5th argument is in the original).
//   DWORD DrawText(INT X, INT Y, DWORD Color, const TCHAR* Text, DWORD P5, DWORD P6, DWORD P7,
//                  L2FontType FontType, INT P9, INT P10, DWORD P11, INT P12, INT P13, INT P14,
//                  INT P15);                                                     // thiscall, ret 0x3C
//       C1: 14 arguments, ret 0x38. The new last argument is never read by the stock function (the
//       callers push a 0): UCanvas::DrawText of C1 became UCanvas::DrawTextToCanvas (vslot 46) with
//       two more arguments, and the device passes NULL (ColorInfo), 0 and 0 for them.
//       DrawText now also passes FontType as UCanvas's 10th argument unchanged, as in C1.
//   DWORD DrawColoredLine(INT X, INT Y, DWORD Color, const TCHAR* Text, DWORD P5, DWORD P6,
//                         DWORD P7, L2FontType FontType, INT P9, INT P10, DWORD P11,
//                         TArray<FL2ColorFontInfo*>* ColorInfo, INT P13);       // ret 0x34
//       C1: 12 arguments, ret 0x30, ColorInfo a TArray<FL2ColorFontInfo>*. One more unused last
//       argument in C2; ColorInfo is the engine's TArray of POINTERS now.
//   static void GetCharSize(SIZE* Size, TCHAR C, INT FontType);                   // cdecl, 3 args
//       C1: (Size, C), always the normal font. C2 zeroes *Size first, then fills it from the
//       UL2Font of L2Fonts[FontType] (nothing when the font or the glyph is missing).
//   DrawLine, DrawSpecialDigit, PopOrigin: same arguments as C1 (their canvas slots moved).
//   Everything else (DrawRect, DrawStretchedTex x2, DrawTexture, DrawBrightenTexture, the
//   minimap/radar/crest/clip functions, Draw3DCoordText, PushOrigin) keeps its C1 arguments.
//
// The new arguments carry DEFAULT values here (0 / L2FONT_Normal) only so that callers that were
// copied from C1 keep compiling: a caller that is ported from the C2 code must pass what the stock
// caller pushes (look at the `push` that comes first before the call: it is the new argument).
// ---------------------------------------------------------------------------------------------

#include "nwindow/NCObject.h"
#include "nwindow/NStdLib.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCWnd;

// A glyph cluster of the Thai text path (UCanvas::MakeThaiCharArray fills TArray<ThaiCharacter>;
// the engine header only declares the name). 0x14 bytes, ten WORDs: the layout is only known from
// the sizes the stock code uses (NCGDevice's array: Remove(.., 0x14); FirstWrappedLine copies one
// with rep movsd ecx=5), the fields are not named yet.
struct ThaiCharacter
{
    WORD W[10];

    INT CharCount() const;   // 0x1000b150 (name ours): the characters the cluster holds, 1 to 6
    // 0x1000b190 (name ours, defined in NCEditBox.cpp): the cluster's width in px with the font FontType of
    // Canvas: the base glyph (W[7] when set, else W[0], else W[5]) or, with a top mark W[1], that glyph plus
    // the mark's (sara am W[0] == 0xE33 counts as sara aa 0xE32)
    INT GetWidth(UCanvas* Canvas, INT FontType) const;
};
NW_ASSERT_SIZE(ThaiCharacter, 0x14);

// state of the non-English line wrapper (names ours)
typedef TCHAR NWrapBuffer[2048];
#if NW_HOOK_MODE
// C2 addresses: the wrap state sits right below NCGDevice's UClass (0x1019f870) as in C1
// (0x1011e4c8): GWrapPos 0x1019f86c, GWrapLen 0x1019f868, GWrapLine 0x1019e868, GWrapText
// 0x1019d868 (confirmed in FirstWrappedLine 0x1000fed0: wcscpy into 0x1019d868, the line is built
// at 0x1019e868, 0x1019f868 holds the length and 0x1019f86c the position)
#define GWrapText NW_GLOBAL(NWrapBuffer, 0x1019d868)   // copy of the text being wrapped
#define GWrapLine NW_GLOBAL(NWrapBuffer, 0x1019e868)   // the line returned
#define GWrapLen  NW_GLOBAL(INT, 0x1019f868)
#define GWrapPos  NW_GLOBAL(INT, 0x1019f86c)
#else
// references into one object that keeps the stock order (src/standalone/NwGlobals.cpp): an
// overlong text runs from GWrapText into GWrapLine, GWrapLen and GWrapPos (FirstWrappedLine's
// wcscpy), as in the stock image
extern NWrapBuffer& GWrapText;
extern NWrapBuffer& GWrapLine;
extern INT& GWrapLen;
extern INT& GWrapPos;
#endif

class NCGDevice : public NCObject
{
    NW_DECLARE_CLASS(NCGDevice, NCObject, 0, 0x1019F870)

    UCanvas* Canvas;            // +0x28
    INT OriginX;                // +0x2C
    INT OriginY;                // +0x30
    NPointDeque Origins;        // +0x34 the moves of PushOrigin, undone by PopOrigin
    INT DirtyTop;               // +0x48 smallest screen Y drawn
    INT DirtyBottom;            // +0x4C largest screen Y drawn
    INT LineHeight;             // +0x50 height of the last text measured by DrawText
    UTexture* SmallDigits;      // +0x54 "SEK.cbui13"
    UTexture* LargeDigits;      // +0x58 "L2UI.NWindow.Number"
    INT PaintOriginX;           // +0x5C OriginX as BeginPaint left it (name ours): DrawColoredLine
                                //   passes OriginX - PaintOriginX, the origin's move since BeginPaint
    TArray<ThaiCharacter> ThaiChars; // +0x60 new in C2 (name ours): only constructed (zeroed) and
                                //   destroyed by the device; nothing in NCGDevice's code uses it

    NCGDevice();                // 0x100108c0
    // ~NCGDevice               // 0x10010980

    virtual void Destroy();     // 0x1000fca0 (vslot 10)

    void Init();                                                             // 0x1000fc00 (name ours)
    UBOOL SetCanvas(UCanvas* InCanvas);                                      // 0x1000f410 (name ours)
    void ClearCanvas();                                                      // 0x1000f420 (name ours)
    static void GetCharSize(SIZE* Size, TCHAR C, INT FontType = 0);         // 0x1000f380 (name ours)
    // out of line on purpose: the stock callers call 0x1000f3d0, they do not inline the canvas call
    static __declspec(noinline) DWORD GetTextExtent(SIZE* Size, const TCHAR* Text, DWORD P3, DWORD P4,
                                                    INT P5, INT P7, DWORD P8,
                                                    L2FontType FontType = L2FONT_Normal); // 0x1000f3d0 (name ours)
    // line wrapping: the first line of Text that fits MaxWidth pixels, then the next ones
    // (names ours); NULL when there is nothing left
    static TCHAR* FirstWrappedLine(const TCHAR* Text, DWORD MaxWidth, INT FontType); // 0x1000fed0
    static TCHAR* NextWrappedLine(DWORD MaxWidth, INT FontType);            // 0x100102b0
    // new in C2, thin cdecl wrappers over the canvas (names ours): LoadFont / DestroyFont are UCanvas
    // vslots 34 / 35, MakeThaiCharArray the canvas's Thai text splitter (every caller of the Thai path uses it)
    static __declspec(noinline) void LoadFont(INT Index, TCHAR* P2, TCHAR* P3);              // 0x1000fba0
    static __declspec(noinline) void DestroyFont(INT Index);                                  // 0x1000fbc0
    static __declspec(noinline) void MakeThaiCharArray(const TCHAR* Text, TArray<ThaiCharacter>& Chars,
                                                       TCHAR* Buffer);                       // 0x1000fbe0

    DWORD DrawText(INT X, INT Y, DWORD Color, const TCHAR* Text, DWORD P5, DWORD P6, DWORD P7,
                   L2FontType FontType, INT P9, INT P10, DWORD P11, INT P12, INT P13,
                   INT P14, INT P15 = 0);                                    // 0x1000f510
    void DrawRect(INT X, INT Y, INT XL, INT YL, DWORD Color);                // 0x1000f5f0
    void DrawStretchedTex(INT X, INT Y, INT XL, INT YL, UTexture* Tex);      // 0x1000f660
    void DrawStretchedTex(INT X, INT Y, INT XL, INT YL, INT SrcX, INT SrcY, INT SrcXL, INT SrcYL,
                          UTexture* Tex);                                    // 0x1000f6d0
    void DrawTexture(INT X, INT Y, INT XL, INT YL, INT U, INT V, INT UL, INT VL,
                     UTexture* Tex);                                         // 0x1000f750
    void DrawBrightenTexture(INT X, INT Y, INT XL, INT YL, INT U, INT V, INT UL, INT VL,
                             UTexture* Tex, DWORD Brightness);               // 0x1000f7e0
    void DrawMinimapLoc(INT X, INT Y, UTexture* Tex);                        // 0x1000f880
    void DrawRadar();                                                        // 0x1000f8b0
    void DrawRadarTarget(INT X, INT Y);                                      // 0x1000f8c0
    void DrawLine(INT X1, INT Y1, INT X2, INT Y2, DWORD Color, INT Width);   // 0x1000f8d0
    void Draw3DCoordText(FVector Loc, DWORD Color, TCHAR* Text, UTexture* Tex, INT P5, INT P6,
                         L2FontType FontType);                               // 0x1000f950
    UBOOL DrawPledgeCrest(INT X, INT Y, INT CrestID);                        // 0x1000fa00 (name ours)
    UBOOL DrawAllyCrest(INT X, INT Y, INT CrestID);                          // 0x1000fa90 (name ours)
    void SetClip(INT X, INT Y, INT XL, INT YL);                              // 0x1000fb20
    void ReleaseClip();                                                      // 0x1000fb30
    DWORD DrawSpecialDigit(UBOOL bLarge, INT X, INT Y, DWORD Color, TCHAR* Text); // 0x1000fb40
    // names ours: the origin moved by (DX, DY) (a POINT by value in the stock code) and back
    void PushOrigin(INT DX, INT DY);                                         // 0x10010f70
    void PopOrigin();                                                        // 0x100109f0
    // DrawText with the canvas's color ranges (the HTML viewer's colored words; name ours)
    DWORD DrawColoredLine(INT X, INT Y, DWORD Color, const TCHAR* Text, DWORD P5, DWORD P6,
                          DWORD P7, L2FontType FontType, INT P9, INT P10, DWORD P11,
                          TArray<FL2ColorFontInfo*>* ColorInfo, INT P13 = 0); // 0x1000f430

    // Every OnPaint / OnDrawToolTip opens and closes with these (inlined in the stock code,
    // names ours): from BeginPaint on, Draw* coordinates are relative to (X, Y).
    void BeginPaint(INT X, INT Y)
    {
        OriginY = Y;
        OriginX = X;
        PaintOriginX = X;
        DirtyTop = OriginY;
        DirtyBottom = 0;
    }
    void EndPaint()
    {
        OriginX = 0;
        OriginY = 0;
    }

    // Tool tip box of buttons and icons (inlined in each OnDrawToolTip of the stock code,
    // name ours): Text on Back, above or below Wnd, pushed left to stay on the screen.
    void DrawToolTipBox(NCWnd* Wnd, const TCHAR* Text, UTexture* Back, UBOOL bAbove);

private:
    // the dirty-range update every Draw* repeats inline in the stock code
    void Touch(INT ScreenY, INT Height)
    {
        if (ScreenY + Height > DirtyBottom)
            DirtyBottom = ScreenY + Height;
        if (ScreenY < DirtyTop)
            DirtyTop = ScreenY;
    }
};
NW_ASSERT_OFFSET(NCGDevice, Canvas, 0x28);
NW_ASSERT_OFFSET(NCGDevice, Origins, 0x34);
NW_ASSERT_OFFSET(NCGDevice, DirtyTop, 0x48);
NW_ASSERT_OFFSET(NCGDevice, LineHeight, 0x50);
NW_ASSERT_OFFSET(NCGDevice, LargeDigits, 0x58);
NW_ASSERT_OFFSET(NCGDevice, PaintOriginX, 0x5C);
NW_ASSERT_OFFSET(NCGDevice, ThaiChars, 0x60);
NW_ASSERT_SIZE(NCGDevice, 0x6C);

#pragma pack(pop)
