#undef UNICODE

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include <ctime>
#include <thread>
#include "ToDoList.hpp"
#include "servermsghandler.hpp"

using namespace std;

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
// #pragma comment (lib, "Mswsock.lib")

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


int __cdecl server(void) 
{   cout << "started executing main function\n";
    cout << "running on port: "<< DEFAULT_PORT << "\n";
    WSADATA wsaData;
    int iResult;

    SOCKET ListenSocket = INVALID_SOCKET;
    SOCKET ClientSocket = INVALID_SOCKET;

    struct addrinfo *result = NULL;
    struct addrinfo hints;

    int iSendResult;
    char recvbuf[DEFAULT_BUFLEN];
    int recvbuflen = DEFAULT_BUFLEN;
    HANDLE hThread;
    DWORD dwThreadID;
    string sendbuf;

    cout << "initialised all variables\n";
    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }
    
    cout << "initilised Winsock\n";
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    // Resolve the server address and port
    iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
    if ( iResult != 0 ) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }
    cout << "Resolved the server address and port\n";
    // Create a SOCKET for the server to listen for client connections.
    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }
    cout << "Created a SOCKET for the server to listen for client connections.\n";
    // Setup the TCP listening socket
    iResult = bind( ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }
    cout << "Settedup the TCP listening socket\n";
    freeaddrinfo(result);

    iResult = listen(ListenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    // Accept a client socket
    ClientSocket = accept(ListenSocket, NULL, NULL);
    if (ClientSocket == INVALID_SOCKET) {
        printf("accept failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }
    cout << "Accepted a client socket\n";
    // No longer need server socket
    closesocket(ListenSocket);
    hThread = CreateThread(NULL, 0, &ReadingThread, (void*)ClientSocket, 0, &dwThreadID);
    if (!hThread)
        {
            std::cout << "Error at CreateThread(): " << GetLastError() << std::endl;
            closesocket(ClientSocket);
            WSACleanup();
            return 0;
        }
    thread t(&inp);
    t.detach();
    do
        {
            if (toSend.size()> 0){
                std::string message = toSend.get();
                if (send(ClientSocket, message.c_str(), message.length(), 0) == SOCKET_ERROR)
                {
                    std::cout << "Error at send(): " << WSAGetLastError() << std::endl;
                    break;
                }
                
            }
            if ( !toSend.size()> 0){
                Sleep(50);
            }
        }
        while (true);

    // Receive until the peer shuts down the connection
    

    // shutdown the connection since we're done
    iResult = shutdown(ClientSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(ClientSocket);
        WSACleanup();
        return 1;
    }

    // cleanup
    closesocket(ClientSocket);
    WSACleanup();
    return 0;

}

int main(){
    while (true){
    server();}
    return 0;
}

