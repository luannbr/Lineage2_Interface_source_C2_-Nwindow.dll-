#pragma once
// L2WordWrapperWnd - the developers' word wrap converter (UClass "L2WordWrapperWnd", 0xC4; C2
// only; NConsoleWnd::Initialize makes it, at NConsoleWnd +0x3564): a "Generate!" button and a
// count of the files converted. It talks to a "WordWrap Server" on localhost:7778 (the Thai text
// is wrapped by the server) and tells the message window whether the connection worked. The
// Generate button converts the Thai HTML pages of ..\INPUT_THAI_HTML into ..\OUTPUT_THAI_HTML
// (the line breaks of the server's wrapping marked "<br1>", font and link tags closed and
// opened again around them); in Thai mode the HTML layout (NHtmlBlock) asks WrapText for the same
// breaks of every line of a page.

#include "nwindow/NCWnd.h"
#include "nwindow/NStl.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class NCButton;

class L2WordWrapperWnd : public NCWnd
{
    NW_DECLARE_CLASS(L2WordWrapperWnd, NCWnd, 0, 0x101ADDC8)

    NCButton* GenerateButton;        // +0x98 id 0xFF330001, text "Generate!" (name ours)
    INT ConvertedCount;              // +0x9C "Converted File Num : %d" (name ours)
    UINT_PTR Socket;                 // +0xA0 the connection to the server (name ours; 0 = none, -1 after a failed socket())
    NStlWString Document;            // +0xA4 a std::wstring: the page being converted (name ours)
    UINT WrapCursor;                 // +0xC0 where WrapText goes on in Document (name ours; 0 at the start of a page)

    L2WordWrapperWnd();                                              // 0x1003f470
    // ~L2WordWrapperWnd                                             // 0x1003f500 (the string freed in line)

    virtual INT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam);  // 25 0x10040130
    virtual INT OnCreate();                                          // 36 0x1003f1e0
    virtual INT OnDestroy();                                         // 37 0x1003f1a0
    virtual INT OnPaint(NCGDevice* Device);                          // 55 0x1003f300

    // names ours
    BYTE ConnectServer(const char* Host, INT Port);                  // 0x1003f030 1 = connected
    BYTE SendRequest(INT Len, const void* Request, void* Reply);     // 0x1003f100 1 = the answer is in (a 4 KB buffer)
    void ClearWrapCursor();                                          // 0x1003f3b0 WrapCursor = 0
    void WriteOutputFile(const TCHAR* Name);                         // 0x1003f3c0 Document into ..\OUTPUT_THAI_HTML\Name
    void WrapText(TCHAR* Text, INT Width);                           // 0x1003f5a0 the server's line breaks of Text into Document
    void ConvertDocument();                                          // 0x1003f9b0 close and open the tags around the breaks
    INT LoadInputFile(const TCHAR* Path);                            // 0x1003fe70 1 = Document is the file's text
    INT OnGenerate();                                                // 0x1003fff0 the button: converts the files
};
NW_ASSERT_OFFSET(L2WordWrapperWnd, GenerateButton, 0x98);
NW_ASSERT_OFFSET(L2WordWrapperWnd, ConvertedCount, 0x9C);
NW_ASSERT_OFFSET(L2WordWrapperWnd, Socket, 0xA0);
NW_ASSERT_OFFSET(L2WordWrapperWnd, Document, 0xA4);
NW_ASSERT_OFFSET(L2WordWrapperWnd, WrapCursor, 0xC0);
NW_ASSERT_SIZE(L2WordWrapperWnd, 0xC4);

#pragma pack(pop)
