#ifndef THREAD_MNGR_HPP
#define THREAD_MNGR_HPP

#define _WINSOCKAPI_
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tchar.h>
#include <strsafe.h>
#include <vector>

#include "server.hpp"

typedef struct thread_data {
    void* object;
    int  job_number;
} MYDATA, * PMYDATA;

#define MAX_THREADS 2
#define BUF_SIZE 255


class Thread_Manager : Server
{
private:
    PMYDATA pDataArray[MAX_THREADS] = {};
    DWORD   dwThreadIdArray[MAX_THREADS] = {};
    HANDLE  hThreadArray[MAX_THREADS] = {};

public:
    Thread_Manager(std::wstring port) { lport = port; }

    static DWORD WINAPI thread_start( LPVOID lpParam )
    {
        PMYDATA ppDataArray = (PMYDATA)lpParam;
        Thread_Manager* This = (Thread_Manager*)ppDataArray->object;
        return This->work(ppDataArray->job_number);
    }

    DWORD work(int THREAD_JOB)
    {
        // get StdOutput Handle
        HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
        if (hStdout == INVALID_HANDLE_VALUE) {
            printf("invalid handle");
            return 1;
        }
        while (true)
        {
            if (THREAD_JOB == 1) {
                initialize();
                create_socket();
                bind_socket();
                accept_connections();
            }
            if (THREAD_JOB == 2) {
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
                start_handler();
            }
        }
        return 0;
    }

    int Tmain()
    {
        for (int i = 0; i < MAX_THREADS; i++) {
            // Allocate memory for thread data.
            pDataArray[i] = (PMYDATA)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(MYDATA));
            if (pDataArray[i] == NULL)
            {
                // If the array allocation fails, the system is out of memory
                // so there is no point in trying to print an error message.
                // Just terminate execution.
                ExitProcess(2);
            }
            pDataArray[i]->object = this;
            pDataArray[i]->job_number = i + 1; // +1 otherwise itll be 0 and quit
            hThreadArray[i] = CreateThread ( NULL, 0, thread_start, pDataArray[i], 0, &dwThreadIdArray[i]);
            if (hThreadArray[i] == NULL)
            {
                ErrorHandler(TEXT("CreateThread"));
                ExitProcess(3);
            }
        }
        WaitForMultipleObjects(MAX_THREADS, hThreadArray, TRUE, INFINITE);
        for (int i = 0; i < MAX_THREADS; i++)
        {
            CloseHandle(hThreadArray[i]);
            if (pDataArray[i] != NULL)
            {
                HeapFree(GetProcessHeap(), 0, pDataArray[i]);
                pDataArray[i] = NULL;    // Ensure address is not reused.
            }
        }
        return 0;
    }

    void ErrorHandler(const wchar_t* lpszFunction)
    {
        // Retrieve the system error message for the last-error code.
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

        // Display the error message.
        lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,
            (lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR));
        StringCchPrintf((LPTSTR)lpDisplayBuf,
            LocalSize(lpDisplayBuf) / sizeof(TCHAR),
            TEXT("%s failed with error %d: %s"),
            lpszFunction, dw, lpMsgBuf);
        MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK);

        // Free error-handling buffer allocations.
        LocalFree(lpMsgBuf);
        LocalFree(lpDisplayBuf);
    }
};

#endif //THREAD_MNGR_HPP
