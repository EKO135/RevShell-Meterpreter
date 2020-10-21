#include <thread>
#include <chrono>
// this define stops windows.h including winsock.h
#define _WINSOCKAPI_
// stops windows.h including random shit
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "client.hpp"

auto wmain(int argc, wchar_t* argv[], wchar_t* envp[]) -> int
{
	ShowWindow(GetConsoleWindow(), SW_SHOW);

	std::wstring chost;
	unsigned short int cport;
	if (argc == 3) {
		chost = argv[1];
		cport = atoi(LPCSTR(argv[2]));
	}
	else {
		chost = L"127.0.0.1";
		cport = 999;
	}

	Client client(chost, cport);
	Client* cli = &client;
	while (1) {
		cli->create_socket();
		while (1) {
			if (!cli->connect_socket()) {
				std::this_thread::sleep_for(std::chrono::seconds(5));
				continue;
			}
			else { break; }
		}
		try {
			cli->receive_commands();
		}
		catch (const std::exception& e) {
			printf("Error in \"receive_commands\": %s", e);
		}
		cli->close_connection();
	}
	return 0;
}
