#define WIN32_LEAN_AND_MEAN
#include <iostream>
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <ctime>
#include <thread>
#include "clientmsghandler.hpp"
#include "ToDoList.hpp"

using namespace std;

// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib




#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

toDoList toSend;
int inp(){
    while (true){
    string msg;
    std::getline(std::cin, msg);
    toSend.add(msg);
    }
    return 0;
}
DWORD WINAPI ReadingThread(LPVOID param)
{
    SOCKET ClientSocket = (SOCKET) param;
    char Buffer[512];
    int iResult;

    do {

        iResult = recv(ClientSocket, Buffer, 512, 0);
        if (iResult > 0) {
            Buffer[iResult] = '\0';
            thread t(&handlemsg,Buffer);
            t.detach();

        }
        else if (iResult == 0){}
            
        else  {
            printf("recv failed with error: %d\n", WSAGetLastError());
            closesocket(ClientSocket);
            WSACleanup();
            return 1;
        }

    } while (true);

    return 0;
}

int __cdecl main(int argc, char **argv) 
{   cout << "running on port: "<< DEFAULT_PORT << "\n";
    cout << "started executing main function\n";
    argc = 2;
    WSADATA wsaData;
    SOCKET ConnectSocket = INVALID_SOCKET;
    struct addrinfo *result = NULL,
                    *ptr = NULL,
                    hints;
    string sendbuf;
    char recvbuf[DEFAULT_BUFLEN];
    int iResult;
    int recvbuflen = DEFAULT_BUFLEN;
    HANDLE hThread;
    DWORD dwThreadID;
    
    // Validate the parameters
    if (argc != 2) {
        printf("usage: %s server-name\n", argv[0]);
        return 1;
    }

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    ZeroMemory( &hints, sizeof(hints) );
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Resolve the server address and port
    iResult = getaddrinfo(argv[1], DEFAULT_PORT, &hints, &result);
    if ( iResult != 0 ) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    // Attempt to connect to an address until one succeeds
    for(ptr=result; ptr != NULL ;ptr=ptr->ai_next) {

        // Create a SOCKET for connecting to server
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, 
            ptr->ai_protocol);
        if (ConnectSocket == INVALID_SOCKET) {
            printf("socket failed with error: %ld\n", WSAGetLastError());
            WSACleanup();
            return 1;
        }

        // Connect to server.
        iResult = connect( ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            continue;
        }
        break;
    }

    freeaddrinfo(result);

    if (ConnectSocket == INVALID_SOCKET) {
        printf("Unable to connect to server!\n");
        WSACleanup();
        return 1;
    }

    hThread = CreateThread(NULL, 0, &ReadingThread, (void*)ConnectSocket, 0, &dwThreadID);
    if (!hThread)
        {
            std::cout << "Error at CreateThread(): " << GetLastError() << std::endl;
            closesocket(ConnectSocket);
            WSACleanup();
            return 0;
        }
    // Send an initial buffer
    thread t(&inp);
    t.detach();
    do
        {
            if (toSend.size()> 0){
                std::string message = toSend.get();
                if (send(ConnectSocket, message.c_str(), message.length(), 0) == SOCKET_ERROR)
                {
                    std::cout << "Error at send(): " << WSAGetLastError() << std::endl;
                    break;
                }
            }
            if (!toSend.size()> 0){
                Sleep(50);
            }
        }
        while (true);



    // cleanup
    closesocket(ConnectSocket);
    WSACleanup();
    while (true){}
    return 0;
}