#include "ComDriver.h"
#include "Shared.h"
#include <fltUser.h>
#include <stdio.h>

#define COMM_NUMBER_OF_LISTENER_THREADS 10

HANDLE gDriverPort;
HANDLE gListenerThreads[COMM_NUMBER_OF_LISTENER_THREADS];

HANDLE gTerminationEvent = NULL;


VOID
CommpMessageListner(
    _In_ PVOID Context
)
{
    printf("[Thread] started: TID = 0x%X\n", GetCurrentThreadId());

    DWORD dwInputBufferSize = COMM_MAX_MESSAGE_SIZE;
    PVOID pInputBuffer = malloc(dwInputBufferSize);
    if (!pInputBuffer)
    {
        printf("[Thread] ERROR, no resources: TID = 0x%X\n", GetCurrentThreadId());
        return;
    }
    DWORD dwBytesWritten = 0;

    OVERLAPPED overlapped;
    overlapped.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    overlapped.Offset = 0;
    overlapped.OffsetHigh = 0;

    HANDLE hWaitArray[2] = { gTerminationEvent, overlapped.hEvent };

    BOOL terminationEventSignaled = FALSE;
    while (!terminationEventSignaled)
    {
        BOOL processMessageDirectly = FALSE;
        BOOL weWaitedForKernel = FALSE;
        NTSTATUS status = WaitForSingleObject(gTerminationEvent, 0);
        if (WAIT_OBJECT_0 == status)
        {
            // termination signaled
            terminationEventSignaled = TRUE;
            continue;
        }

        HRESULT result = FilterGetMessage(gDriverPort,
            (PFILTER_MESSAGE_HEADER)pInputBuffer,
            dwInputBufferSize,
            &overlapped
        );
        if (S_OK == result)
        {
            // we managed to get a message
            processMessageDirectly = TRUE;
        }
        else if (ERROR_IO_PENDING == (result & 0x7FFF))
        {
            // we have to wait for messages
            status = WaitForMultipleObjects(2, hWaitArray, FALSE, INFINITE);
            weWaitedForKernel = TRUE;
        }
        else if (ERROR_INVALID_HANDLE == (result & 0x7FFF))
        {
            // The communication port was closed
            // This should never happen while the service is active
            // terminating the thread
            wprintf(L"[Error] FilterGetMessage returned ERROR_INVALID_HANDLE.\n");
            goto exit;
        }
        else
        {
            // unexpected status returned
            wprintf(L"[Error] FilterGetMessage returned unexpected result. Status = 0x%X\n", status);
            continue;
        }

        if (processMessageDirectly)
        {
            // pInputBuffer contains the message
        }
        else if (weWaitedForKernel)
        {
            switch (status)
            {
            case WAIT_OBJECT_0:
                terminationEventSignaled = TRUE;
                continue;

            case WAIT_OBJECT_0 + 1:
                // message from kernel trough OVERLAPPED IO
                if (!GetOverlappedResult(gDriverPort, &overlapped, &dwBytesWritten, FALSE))
                {
                    wprintf(L"[Error] GetOverlappedResult failed with error = 0x%X\n", GetLastError());
                    goto exit;
                }
                break;
            default:
                printf("[Thread] ERROR: TID = 0x%X, WaitForMultipleObjects failed... status = 0x%X\n", GetCurrentThreadId(), status);
                goto exit;
            }
        }

        // 
        // HANDLE the message
        //
        PFILTER_MESSAGE_HEADER header = (PFILTER_MESSAGE_HEADER)pInputBuffer;
        PMY_MSG_HEADER ourMessageHeader = (PMY_MSG_HEADER)(header + 1);

        switch (ourMessageHeader->MessageId)
        {
        case idProcessCreate:
            // HANDLE CREATE
            break;
            /// etc
        default:
            break;
        }

    }

exit:
    CancelIo(gDriverPort);
    printf("[Thread] stopped: TID = 0x%X\n", GetCurrentThreadId());
    free(pInputBuffer);
    CloseHandle(overlapped.hEvent);
}


BOOL
InitCommunication()
{
    //
    // Connect to the communication port
    //
    HRESULT result = FilterConnectCommunicationPort(MY_FILTER_PORT_NAME,
        0,
        NULL,
        0,
        NULL,
        &gDriverPort
    );
    if (S_OK != result)
    {
        wprintf(L"[Error] FilterConnectCommunicationPort failed with status 0x%X\n", result);
        gDriverPort = INVALID_HANDLE_VALUE;
        return FALSE;
    }

    gTerminationEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (!gTerminationEvent)
    {
        wprintf(L"CreateEvent failed with error = 0x%X\n", GetLastError());
        return FALSE;
    }

    //
    // Start the listener threads - this must be the last thing we do
    //
    for (int i = 0; i < COMM_NUMBER_OF_LISTENER_THREADS; i++)
    {
        gListenerThreads[i] = CreateThread(NULL,
            0,
            (LPTHREAD_START_ROUTINE)CommpMessageListner,
            NULL,
            0,
            NULL
        );
        if (NULL == gListenerThreads[i])
        {
            wprintf(L"CreateThread failed with error = 0x%X\n", GetLastError());
            return FALSE;
        }
    }


    return TRUE;
}

void UninitCommunication()
{
    CloseHandle(gDriverPort);
    gDriverPort = NULL;

    SetEvent(gTerminationEvent); // tell all the threads to stop

    for (int i = 0; i < COMM_NUMBER_OF_LISTENER_THREADS; i++)
    {
        if (gListenerThreads[i])
        {
            WaitForSingleObject(gListenerThreads[i], INFINITE);
            CloseHandle(gListenerThreads[i]);
            gListenerThreads[i] = NULL;
        }
    }

}


BOOL
CommSendMessage(
    _In_reads_bytes_(dwInBufferSize) LPVOID lpInBuffer,
    _In_ DWORD dwInBufferSize,
    _Out_writes_bytes_to_opt_(dwOutBufferSize, *lpBytesReturned) LPVOID lpOutBuffer,
    _In_ DWORD dwOutBufferSize,
    _Out_ LPDWORD lpBytesReturned
)
{
    HRESULT result = FilterSendMessage(gDriverPort,
        lpInBuffer,
        dwInBufferSize,
        lpOutBuffer,           // use same buffer for output
        dwOutBufferSize,
        lpBytesReturned
    );
    if (S_OK != result)
    {
        wprintf(L"FilterSendMessage failed with status = 0x%X\n", result);
        return FALSE;
    }

    return TRUE;
}