#define _WINSOCKAPI_
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "thread_mngr.hpp"

auto main(int argc, char* argv[]) -> int
{
	ShowWindow(GetConsoleWindow(), SW_SHOW);
	const char* lport;
	if (argc == 2) { lport = argv[1]; }
	else { lport = "999"; }

	Thread_Manager TM(lport);
	Thread_Manager* tmngr = &TM;
	tmngr->Tmain();

	return 0;
}
