#ifndef _COM_DRV_H_INCLUDED_
#define _COM_DRV_H_INCLUDED_
#include <Windows.h>

BOOL InitCommunication();


void UninitCommunication();

BOOL
CommSendMessage(
    _In_reads_bytes_(dwInBufferSize) LPVOID lpInBuffer,
    _In_ DWORD dwInBufferSize,
    _Out_writes_bytes_to_opt_(dwOutBufferSize, *lpBytesReturned) LPVOID lpOutBuffer,
    _In_ DWORD dwOutBufferSize,
    _Out_ LPDWORD lpBytesReturned
);


#endif
