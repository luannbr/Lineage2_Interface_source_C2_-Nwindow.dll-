/*=============================================================================
    MSNSocket.cpp - the connection of the unused MSN messenger client (C2 only). Stock:
    0x100dfc40 .. 0x100dfdcf and 0x10055ab0 (the empty notification, folded with an empty
    two-argument function elsewhere).
=============================================================================*/
#include <winsock2.h>
#include <stdio.h>
#include "nwindow/MSNSocket.h"

enum { WM_MSN_SOCKET = 0x507 };   // the message the socket's events are posted as (name ours)

#if NW_HOOK_MODE
// ws2_32 is not linked into the hook DLL: the calls go through the stock image's own import thunks
// (each one a `jmp [IAT]`), whose addresses are the stock ones
#define Ws_send          ((int (WSAAPI*)(SOCKET, const char*, int, int))::nw::Addr(0x100e7222))
#define Ws_closesocket   ((int (WSAAPI*)(SOCKET))::nw::Addr(0x100e722e))
#define Ws_WSACleanup    ((int (WSAAPI*)())::nw::Addr(0x100e7228))
#define Ws_WSAGetLastError ((int (WSAAPI*)())::nw::Addr(0x100e71f2))
#define Ws_WSAAsyncSelect  ((int (WSAAPI*)(SOCKET, HWND, u_int, long))::nw::Addr(0x100e7234))
#else
#define Ws_send          send
#define Ws_closesocket   closesocket
#define Ws_WSACleanup    WSACleanup
#define Ws_WSAGetLastError WSAGetLastError
#define Ws_WSAAsyncSelect  WSAAsyncSelect
#endif

// 0x100dfc40
MSNSocket::MSNSocket()
{
    Socket = 0;
    hWnd = NULL;
}
NW_HOOK_CTOR(0x100dfc40, MSNSocket)

// 0x100dfd30
NW_HOOK_DTOR(0x100dfd30, MSNSocket)

// 0x100dfc60 - one line: the text and a CR LF (the stock code formats it with the C runtime
// into a 1 KB buffer; the version line already ends with a CR LF)
void MSNSocket::SendLine(const char* Text)
{
    char Buffer[1024];
    Buffer[0] = 0;
    sprintf(Buffer, "%s\r\n", Text);
    Ws_send(Socket, Buffer, (int)strlen(Buffer), 0);
}
NW_HOOK_METHOD(0x100dfc60, MSNSocket, SendLine)

// 0x100dfcc0
void MSNSocket::CloseSocket()
{
    if (Socket)
        Ws_closesocket(Socket);
    Socket = 0;
    Ws_WSACleanup();
}
NW_HOOK_METHOD(0x100dfcc0, MSNSocket, CloseSocket)

// 0x100dfce0 (vslot 10) - a thunk of CloseSocket in the stock code
void MSNSocket::Destroy()
{
    CloseSocket();
}
NW_HOOK_METHOD(0x100dfce0, MSNSocket, Destroy)

// 0x100dfd90 (vslot 25) - connected (no error in the high word of lParam): the socket's
// read, out-of-band and close events go to the window (0x25: the stock code asks for these three), then the protocol version is offered
void MSNSocket::OnConnect(WPARAM wParam, LPARAM lParam)
{
    if (((DWORD)lParam >> 16) == 0)
    {
        if (Ws_WSAAsyncSelect(Socket, hWnd, WM_MSN_SOCKET, FD_READ | FD_OOB | FD_CLOSE) != 0)
        {
            Ws_WSAGetLastError();
            return;
        }
        SendLine("VER 1 MSNP9 MSNP8 CVRO\r\n");
    }
}
NW_HOOK_METHOD(0x100dfd90, MSNSocket, OnConnect)

// 0x10055ab0 (vslot 26) - empty (the stock body is shared with other empty two-argument functions)
void MSNSocket::OnReceive(WPARAM wParam, LPARAM lParam)
{
}

NW_CHECK_VTABLE(MSNSocket, 0x101292c0, 27)
