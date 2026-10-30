#pragma once
// MSNSocket - the connection of the MSN messenger client (UClass "MSNSocket", 0x48, super
// UObject; C2 only). The C2 build has only its first steps and nothing creates it: a close
// (Destroy), the connected notification (it asks Windows to post the socket's events to the
// window and sends the protocol version line) and a line sender; the second notification is empty.

#include "nwindow/NWindow.h"
#pragma pack(push, 4)   // the C2 layout puts 64-bit members at 4-aligned offsets

class MSNSocket : public UObject
{
    NW_DECLARE_CLASS(MSNSocket, UObject, 0, 0x101E25F0)

    HWND hWnd;                       // +0x28 gets the socket's events (WM 0x507) (name ours)
    UINT_PTR Socket;                 // +0x2C (name ours; 0 = closed)
    BYTE _Unknown30[0x48 - 0x30];    // +0x30 never touched by the C2 code

    MSNSocket();                                                     // 0x100dfc40
    // ~MSNSocket                                                    // 0x100dfd30

    virtual void Destroy();                                          // 10 0x100dfce0 (the thunk of CloseSocket)
    virtual void OnConnect(WPARAM wParam, LPARAM lParam);            // 25 0x100dfd90 (name ours)
    virtual void OnReceive(WPARAM wParam, LPARAM lParam);            // 26 0x10055ab0 (name ours) empty

    // names ours
    void SendLine(const char* Text);                                 // 0x100dfc60 Text + "\r\n"
    void CloseSocket();                                              // 0x100dfcc0
};
NW_ASSERT_OFFSET(MSNSocket, hWnd, 0x28);
NW_ASSERT_OFFSET(MSNSocket, Socket, 0x2C);
NW_ASSERT_SIZE(MSNSocket, 0x48);

#pragma pack(pop)
