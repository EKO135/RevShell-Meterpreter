#include <string>
#include <cstring>
#include <cstdio>

// windows
#define _WINSOCKAPI_
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "server.hpp"
#define BUFSIZE 4096
//std::string find_local_ip();
#define NUMBER_OF_THREADS = 2
#define JOB_NUMBER = {1, 2}


auto main(int argc, char* argv[]) -> int
{
	ShowWindow(GetConsoleWindow(), SW_SHOW);
	
	const char* lport;
	if (argc == 2) { lport = argv[1]; } 
	else { lport = "999"; }
	
	// below is just for testing
	Server serv(lport);
	Server* server = &serv;
	while (true) {
		server->initialize();
		server->create_socket();
		server->bind_socket();
		server->accept_connections();
		server->start_handler();
	}
	return 0;
}

//TODO: create threads
void create_threads() { return; }
template<typename T>
void work(T server) { return; }
void create_jobs() { return; }

/*
std::string find_local_ip()
{
	STARTUPINFO sinfo = { 0 };
	PROCESS_INFORMATION pinfo = { 0 };
	HANDLE output_Rd = NULL;
	HANDLE output_Wr = NULL;
	BOOL bsuccess;

	SECURITY_ATTRIBUTES sa;
	printf("\n->Start of parent execution.\n");
	// Set the bInheritHandle flag so pipe handles are inherited. 
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.bInheritHandle = TRUE;
	sa.lpSecurityDescriptor = NULL;

	// Create a pipe for the child process's STDOUT. 
	if (!CreatePipe(&output_Rd, &output_Wr, &sa, 0)) { return "failed"; }
	if (!SetHandleInformation(output_Rd, HANDLE_FLAG_INHERIT, 0)) { return "failed"; }

	sinfo.cb = sizeof(sinfo);
	sinfo.dwFlags = (STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW);
	sinfo.hStdOutput = sinfo.hStdError = output_Wr;

	std::wstring Process = L"powershell.exe ";
	Process.append(L"\"Test-Connection -ComputerName(hostname) -Count 1 | ");
	Process.append(L"Select -ExpandProperty IPV4Address | Select -ExpandProperty IPAddressToString\" ");

	printf("\nrunning command\n");
	bsuccess = CreateProcess (
		NULL, (LPWSTR)Process.c_str(),
		NULL, NULL, TRUE,
		NORMAL_PRIORITY_CLASS | CREATE_NO_WINDOW,
		NULL, NULL, &sinfo, &pinfo
	);
	WaitForSingleObject(pinfo.hProcess, INFINITE);
	CloseHandle(pinfo.hProcess);
	CloseHandle(pinfo.hThread);
	CloseHandle(output_Wr);
	
	if (!bsuccess) { return "failed"; }
	else
	{
		DWORD dwRead;
		CHAR chBuf[BUFSIZE];
		bool bSuccess = FALSE;
		std::string OUTPUT = "";
		for (;;) {
			bSuccess = ReadFile(output_Rd, chBuf, BUFSIZE, &dwRead, NULL);
			if (!bSuccess || dwRead == 0) { break; }
			std::string s(chBuf, dwRead);
			OUTPUT += s;
		}
		return OUTPUT;
	}
}
*/
