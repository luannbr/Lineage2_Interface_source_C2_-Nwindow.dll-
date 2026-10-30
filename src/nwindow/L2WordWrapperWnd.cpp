/*=============================================================================
    L2WordWrapperWnd.cpp - the developers' word wrap converter window (C2 only). Stock:
    0x1003f030 .. 0x1003fff0 (ConnectServer, OnDestroy, OnCreate, OnPaint, SendRequest,
    WriteOutputFile, the constructor and destructor, WrapText, ConvertDocument, LoadInputFile),
    0x1003fff0 .. 0x1004012c (OnGenerate) and 0x10040130 .. 0x10040182 (WindowProc).
=============================================================================*/
#include <winsock2.h>
#include "nwindow/L2WordWrapperWnd.h"
#include "nwindow/NCButton.h"
#include "nwindow/NCConsole.h"
#include "nwindow/NCGDevice.h"
#include "nwindow/NCMessageWnd.h"
#include "nwindow/NCNPCHtmlViewer.h"
#include "nwindow/NConsoleWnd.h"

#include <string.h>

enum { NID_WORDWRAP_GENERATE = (INT)0xFF330001 };   // the button's id (name ours)

#if NW_HOOK_MODE
// ws2_32 is not linked into the hook DLL: the calls go through the stock image's own import thunks
// (each one a `jmp [IAT]`), whose addresses are the stock ones
#define Ws_WSAStartup    ((int (WSAAPI*)(WORD, LPWSADATA))::nw::Addr(0x100e7216))
#define Ws_socket        ((SOCKET (WSAAPI*)(int, int, int))::nw::Addr(0x100e7210))
#define Ws_inet_addr     ((unsigned long (WSAAPI*)(const char*))::nw::Addr(0x100e720a))
#define Ws_gethostbyname ((struct hostent* (WSAAPI*)(const char*))::nw::Addr(0x100e7204))
#define Ws_htons         ((u_short (WSAAPI*)(u_short))::nw::Addr(0x100e71fe))
#define Ws_connect       ((int (WSAAPI*)(SOCKET, const struct sockaddr*, int))::nw::Addr(0x100e71f8))
#define Ws_WSAGetLastError ((int (WSAAPI*)())::nw::Addr(0x100e71f2))
#define Ws_send          ((int (WSAAPI*)(SOCKET, const char*, int, int))::nw::Addr(0x100e7222))
#define Ws_recv          ((int (WSAAPI*)(SOCKET, char*, int, int))::nw::Addr(0x100e721c))
#else
#define Ws_WSAStartup    WSAStartup
#define Ws_socket        socket
#define Ws_inet_addr     inet_addr
#define Ws_gethostbyname gethostbyname
#define Ws_htons         htons
#define Ws_connect       connect
#define Ws_WSAGetLastError WSAGetLastError
#define Ws_send          send
#define Ws_recv          recv
#endif

// the line break the server's wrapping is marked with in the page (name ours)
#define WRAP_BREAK TEXT("<br1>")

// 0x1003f470 - the std::wstring is empty (inline in the stock code, as in ours); the count and
// the button are left to OnCreate
L2WordWrapperWnd::L2WordWrapperWnd()
{
    GenerateButton = NULL;
    ConvertedCount = 0;
    WrapCursor = 0;
}
NW_HOOK_CTOR(0x1003f470, L2WordWrapperWnd)

// 0x1003f500
NW_HOOK_DTOR(0x1003f500, L2WordWrapperWnd)

// 0x1003f030 - the socket is kept in Socket; a name is looked up when the text is not an address.
// A failed socket() leaves -1 in Socket; the sockaddr is not cleared (sin_zero is never read)
BYTE L2WordWrapperWnd::ConnectServer(const char* Host, INT Port)
{
    WSADATA Data;
    Ws_WSAStartup(0x101, &Data);
    Socket = Ws_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (Socket == (UINT_PTR)-1)
        return 0;
    sockaddr_in Address;
    unsigned long Ip = Ws_inet_addr(Host);
    if (Ip != INADDR_NONE)
        Address.sin_addr.s_addr = Ip;
    else
    {
        hostent* Entry = Ws_gethostbyname(Host);
        if (!Entry)
            return 0;
        memcpy(&Address.sin_addr, Entry->h_addr_list[0], Entry->h_length);
    }
    Address.sin_family = AF_INET;
    Address.sin_port = Ws_htons((u_short)Port);
    if (Ws_connect(Socket, (const sockaddr*)&Address, sizeof(Address)) != 0)
    {
        Ws_WSAGetLastError();
        return 0;
    }
    return 1;
}
NW_HOOK_METHOD(0x1003f030, L2WordWrapperWnd, ConnectServer)

// 0x1003f100 - Len bytes of Request to the server and its answer into Reply (a 4 KB buffer): the first dword
// of the answer is the number of characters that follow, so it is complete at count * 2 + 4 bytes. The next
// recv starts at the number of bytes the last one gave (not at the total: the stock does the same), and a
// closed connection is not noticed. A failed send says so in the message window.
BYTE L2WordWrapperWnd::SendRequest(INT Len, const void* Request, void* Reply)
{
    if (!Request || !Reply || !Socket)
        return 0;
    if (Ws_send(Socket, (const char*)Request, Len, 0) <= 0)
    {
        GConsole->MessageWnd->SetText(TEXT("Disconnect!!!!"));
        return 0;
    }
    INT Offset = 0;
    INT Total = 0;
    do
    {
        Offset = Ws_recv(Socket, (char*)Reply + Offset, 0x1000 - Offset, 0);
        Total += Offset;
        if (*(INT*)Reply * 2 + 4 == Total)
            break;
    } while (Total < 0x1000);
    return 1;
}
NW_HOOK_METHOD(0x1003f100, L2WordWrapperWnd, SendRequest)

// 0x10040130 (vslot 25)
INT L2WordWrapperWnd::WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (NCWnd::WindowProc(Msg, wParam, lParam))
        return 1;
    if (Msg == NWM_COMMAND && (INT)wParam == NID_WORDWRAP_GENERATE)
        return OnGenerate() != 0;
    return 0;
}
NW_HOOK_METHOD(0x10040130, L2WordWrapperWnd, WindowProc)

// 0x1003f1a0 (vslot 37) - not guarded; NCWnd's OnDestroy first, then the button
INT L2WordWrapperWnd::OnDestroy()
{
    NCWnd::OnDestroy();
    if (GenerateButton)
    {
        RemoveChildWindow(GenerateButton->Entry);
        delete GenerateButton;
        GenerateButton = NULL;
    }
    return 1;
}
NW_HOOK_METHOD(0x1003f1a0, L2WordWrapperWnd, OnDestroy)

// 0x1003f1e0 (vslot 36) - the button, then the connection to the converter on this machine; the
// message window says how it went
INT L2WordWrapperWnd::OnCreate()
{
    NCWnd::OnCreate();
    GenerateButton = new NCButton(TEXT("L2UI.control.DefaultButton"), TEXT("L2UI.control.DefaultButton_click"), 0);
    GenerateButton->ID = NID_WORDWRAP_GENERATE;
    GenerateButton->SetWindowRect(0, 0, 67, 19);
    CreateChild(GenerateButton, NWS_VISIBLE);
    GenerateButton->SetText(TEXT("Generate!"));
    Socket = 0;
    if (!ConnectServer("localhost", 7778))
    {
        Socket = 0;
        GConsole->MessageWnd->SetText(TEXT("Connection Failed to WordWrap Server!!!!!"));
        return 1;
    }
    GConsole->MessageWnd->SetText(TEXT("Connection O~~~K~~~~ to WordWrap Server!!!!!"));
    return 1;
}
NW_HOOK_METHOD(0x1003f1e0, L2WordWrapperWnd, OnCreate)

// 0x1003f300 (vslot 55) - the number of files converted; the origin goes on before the clip
INT L2WordWrapperWnd::OnPaint(NCGDevice* Device)
{
    TCHAR Text[256];
    Device->BeginPaint(ScreenX, ScreenY);
    Device->SetClip(ScreenX, ScreenY, Width, Height);
    appSprintf(Text, TEXT("Converted File Num : %d"), ConvertedCount);
    Device->DrawText(0, 20, 0xFF000000, Text, 0, 0, 0, L2FONT_Normal, 0, 0, 0, 0, 0, 0, 0);
    Device->EndPaint();
    Device->ReleaseClip();
    return 1;
}
NW_HOOK_METHOD(0x1003f300, L2WordWrapperWnd, OnPaint)

// 0x1003f3b0 (name ours; w3shortcut's names.csv line calls it ClearUnknownC0) - WrapCursor back to the start of the page
// (the stock's one-line body is not shared with NActionItem::Select: the fmap pairing was wrong)
__declspec(noinline) void L2WordWrapperWnd::ClearWrapCursor()
{
    WrapCursor = 0;
}
NW_HOOK_METHOD(0x1003f3b0, L2WordWrapperWnd, ClearWrapCursor)

// 0x1003f3c0 - the page (Document) as UTF-16 into ..\OUTPUT_THAI_HTML\<Name>, through a stack copy of it
void L2WordWrapperWnd::WriteOutputFile(const TCHAR* Name)
{
    TCHAR Path[256];
    TCHAR Text[0x2800];
    appSprintf(Path, TEXT("..\\OUTPUT_THAI_HTML\\%s"), Name);
    FArchive* Writer = GFileManager->CreateFileWriter(Path, FILEWRITE_EvenIfReadOnly | FILEWRITE_Unbuffered, GNull);
    if (Writer)
    {
        appStrcpy(Text, Document.CStr());
        Writer->Serialize(Text, appStrlen(Text) * 2);
        Writer->Flush();
        delete Writer;
    }
}
NW_HOOK_METHOD(0x1003f3c0, L2WordWrapperWnd, WriteOutputFile)

// 0x1003f5a0 - the server wraps Text (Width pixels, the HTML layout's maximum) and answers it with "<br1>" at the
// line breaks; each piece of the answer is looked for in Document from WrapCursor on, the tags of the page
// (<a ..>, <font ..>, </a>, </f..>) skipped inside the match, and a "<br1>" goes into Document after it. The stock
// quirks stay: the count of characters matched is not cleared when a match fails half way, a piece of one
// character is never marked, and the end of a tag is searched without a limit. Frame: three 4 KB request /
// answer buffers, the text of the answer (8 KB), a copy of the page and the piece
void L2WordWrapperWnd::WrapText(TCHAR* Text, INT Width)
{
    TCHAR Piece[512];
    BYTE Request[0x1000];
    BYTE Reply[0x1000];
    TCHAR Answer[0x1000];
    TCHAR PageCopy[0x2800];
    memset(Answer, 0, 0x1000);
    memset(Request, 0, 0x1000);
    memset(Reply, 0, 0x1000);
    INT Len = appStrlen(Text);
    ((INT*)Request)[0] = Width;
    ((INT*)Request)[1] = Len;
    appMemcpy(Request + 8, Text, Len * 2);
    if (!SendRequest(Len * 2 + 8, Request, Reply))
        return;
    INT Count = ((INT*)Reply)[0];
    if ((BYTE)Count == 0)
        return;
    memcpy(Answer, Reply + 4, Count * 2);
    NStlWString Wrapped;
    Wrapped.Assign(Answer, wcslen(Answer));
    for (;;)
    {
        UINT Pos = Wrapped.Find(WRAP_BREAK, 0, wcslen(WRAP_BREAK));
        if (Pos > Wrapped.Size)
            break;
        {
            NStlWString Line;
            {
                NStlWString Part;
                Wrapped.Substr(&Part, 0, Pos);
                Line.Assign(Part, 0, NStlWString::NPos);
            }
            Wrapped.Erase(0, Pos + 5);
            appStrcpy(Piece, Line.CStr());
        }
        appStrcpy(PageCopy, Document.CStr());
        INT Matched = 0;
        INT J;
        if (WrapCursor < Document.Size)
        {
            for (;;)
            {
                if (Piece[0] != PageCopy[WrapCursor])
                    Matched = 0;
                else
                {
                    Matched++;
                    J = 1;
                    if (appStrlen(Piece) > J)
                    {
                        for (;;)
                        {
                            UINT At = WrapCursor + Matched;
                            TCHAR C = PageCopy[At];
                            if (C == '<' &&
                                ((PageCopy[At + 1] == 'a' && PageCopy[At + 2] == ' ') ||
                                 (PageCopy[At + 1] == 'f' && PageCopy[At + 2] == 'o' && PageCopy[At + 3] == 'n' &&
                                  PageCopy[At + 4] == 't') ||
                                 (PageCopy[At + 1] == '/' && PageCopy[At + 2] == 'a') ||
                                 (PageCopy[At + 1] == '/' && PageCopy[At + 2] == 'f')))
                            {
                                // inside a tag of the page: on to its '>'
                                if (PageCopy[At] != '>')
                                {
                                    TCHAR* P = &PageCopy[At];
                                    do
                                    {
                                        P++;
                                        Matched++;
                                    } while (*P != '>');
                                }
                                Matched++;
                            }
                            if (Piece[J] != PageCopy[WrapCursor + Matched])
                            {
                                J = 0;
                                break;
                            }
                            Matched++;
                            if (J == appStrlen(Piece) - 1)
                                goto Found;
                            J++;
                            if (!(J < appStrlen(Piece)))
                                goto Found;
                        }
                    }
                    else
                        J = 0;
                }
                WrapCursor++;
                if (!(WrapCursor < Document.Size))
                    break;
            }
            continue;
        Found:
            {
                UINT BreakAt = WrapCursor + Matched;
                WrapCursor = BreakAt + 5;
                Document.Insert(BreakAt, WRAP_BREAK, wcslen(WRAP_BREAK));
            }
        }
    }
}
NW_HOOK_METHOD(0x1003f5a0, L2WordWrapperWnd, WrapText)

// 0x1003f9b0 - the pages' fonts and links across the breaks: from each "<font ..>" (or "<a ..>") whose element has a
// break before its end tag, the element is closed before the first break and opened again after it. The scan runs on a
// copy of the page and counts into the real one by hand, so only the first break of an element is dealt with in one
// pass; five passes over the page for the fonts, five for the links. Nothing is looked for after an element that has
// no break behind its start tag
void L2WordWrapperWnd::ConvertDocument()
{
    NStlWString Work;
    NStlWString Tag;
    for (INT Pass = 5; Pass > 0; Pass--)
    {
        {
            NStlWString Copy;
            Copy.Assign(Document, 0, NStlWString::NPos);
            Work.Assign(Copy, 0, NStlWString::NPos);
        }
        UINT Off = 0;
        for (;;)
        {
            UINT Pos = Work.Find(TEXT("<font "), 0, wcslen(TEXT("<font ")));
            if (Pos > Work.Size)
                break;
            Work.Erase(0, Pos);
            Off += Pos;
            UINT Close = Work.Find(TEXT(">"), 0, wcslen(TEXT(">")));
            {
                NStlWString Part;
                Work.Substr(&Part, 0, Close + 1);
                Tag.Assign(Part, 0, NStlWString::NPos);
            }
            Work.Erase(0, Close + 1);
            Off += Close + 1;
            UINT End = Work.Find(TEXT("</font>"), 0, wcslen(TEXT("</font>")));
            UINT Break = Work.Find(WRAP_BREAK, 0, wcslen(WRAP_BREAK));
            if (Break > Work.Size)
            {
                Work.Erase(0, End + 7);
                break;
            }
            if (Break < End)
            {
                Work.Erase(0, Break);
                Off += Break;
                Document.Insert(Off, TEXT("</font>"), wcslen(TEXT("</font>")));
                Document.Insert(Off + 12, Tag, 0, NStlWString::NPos);
                Off += Tag.Size + 7;
            }
            else
            {
                Work.Erase(0, End + 7);
                Off += End + 7;
            }
        }
    }
    for (INT Pass = 5; Pass > 0; Pass--)
    {
        {
            NStlWString Copy;
            Copy.Assign(Document, 0, NStlWString::NPos);
            Work.Assign(Copy, 0, NStlWString::NPos);
        }
        UINT Off = 0;
        for (;;)
        {
            UINT Pos = Work.Find(TEXT("<a "), 0, wcslen(TEXT("<a ")));
            if (Pos > Work.Size)
                break;
            Work.Erase(0, Pos);
            Off += Pos;
            UINT Close = Work.Find(TEXT(">"), 0, wcslen(TEXT(">")));
            {
                NStlWString Part;
                Work.Substr(&Part, 0, Close + 1);
                Tag.Assign(Part, 0, NStlWString::NPos);
            }
            Work.Erase(0, Close + 1);
            Off += Close + 1;
            UINT End = Work.Find(TEXT("</a>"), 0, wcslen(TEXT("</a>")));
            UINT Break = Work.Find(WRAP_BREAK, 0, wcslen(WRAP_BREAK));
            if (Break > Work.Size)
            {
                Work.Erase(0, End + 4);
                break;
            }
            if (Break < End)
            {
                Work.Erase(0, Break);
                Off += Break;
                Document.Insert(Off, TEXT("</a>"), wcslen(TEXT("</a>")));
                Document.Insert(Off + 9, Tag, 0, NStlWString::NPos);
                Off += Tag.Size + 4;
            }
            else
            {
                Work.Erase(0, End + 4);
                Off += End + 4;
            }
        }
    }
}
NW_HOOK_METHOD(0x1003f9b0, L2WordWrapperWnd, ConvertDocument)

// 0x1003fe70 - the file's bytes as UTF-16 text into Document, every double space made a single one. The buffer
// read is not freed (the stock leaves it); an empty file or none leaves 0 / a Document of "" as it is
INT L2WordWrapperWnd::LoadInputFile(const TCHAR* Path)
{
    INT Size = GFileManager->FileSize(Path);
    FArchive* Reader = GFileManager->CreateFileReader(Path, 0, GNull);
    if (!Reader)
        return 0;
    TCHAR* Buffer = (TCHAR*)appMalloc(Size + 2, TEXT("ThaiHtmlConvertor"));
    appMemset(Buffer, 0, Size + 2);
    if (Buffer)
    {
        Reader->Serialize(Buffer, Size);
        {
            NStlWString Text;
            Text.Assign(Buffer, wcslen(Buffer));
            Document.Assign(Text, 0, NStlWString::NPos);
        }
        for (;;)
        {
            UINT Pos = Document.Find(TEXT("  "), 0, wcslen(TEXT("  ")));
            if (Pos >= Document.Size)
                break;
            Document.Replace(Pos, 2, TEXT(" "), wcslen(TEXT(" ")));
        }
    }
    delete Reader;
    return 1;
}
NW_HOOK_METHOD(0x1003fe70, L2WordWrapperWnd, LoadInputFile)

// 0x1003fff0 - the Generate! button: every file of ..\INPUT_THAI_HTML (the "." and ".." entries too, which fail to
// load) is loaded, shown in the NPC dialog, converted and written to ..\OUTPUT_THAI_HTML; the log says where it is.
// The search handle is never closed
INT L2WordWrapperWnd::OnGenerate()
{
    WIN32_FIND_DATAW Find;
    TCHAR Path[256];
    ConvertedCount = 0;
    HANDLE Handle = FindFirstFileW(TEXT("..\\INPUT_THAI_HTML\\*.*"), &Find);
    if (Handle == INVALID_HANDLE_VALUE)
        return 1;
    do
    {
        appSprintf(Path, TEXT("..\\INPUT_THAI_HTML\\%s"), Find.cFileName);
        if (GLog)
        {
            GLog->Logf2(TEXT("%s parsing begin....."), Find.cFileName);
            GLog->Flush();
        }
        if (LoadInputFile(Path))
        {
            GConsole->GetDesktop()->NpcHtmlWnd->ShowPage((TCHAR*)Document.CStr());
            ConvertDocument();
            WriteOutputFile(Find.cFileName);
            ClearWrapCursor();
            ConvertedCount++;
        }
        if (GLog)
        {
            GLog->Logf2(TEXT("%s parsing SUCCESS!!"), Find.cFileName);
            GLog->Flush();
        }
    } while (FindNextFileW(Handle, &Find));
    return 1;
}
NW_HOOK_METHOD(0x1003fff0, L2WordWrapperWnd, OnGenerate)

NW_CHECK_VTABLE(L2WordWrapperWnd, 0x10111878, 67)
