#define _WINSOCKAPI_
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "thread_mngr.hpp"

auto wmain(int argc, wchar_t* argv[], wchar_t* envp[]) -> int
{
	ShowWindow(GetConsoleWindow(), SW_SHOW);
	std::wstring lport;
	if (argc == 2) { lport = argv[1]; }
	else { lport = L"999"; }

	Thread_Manager TM(lport);
	Thread_Manager* tmngr = &TM;
	tmngr->Tmain();

	return 0;
}
