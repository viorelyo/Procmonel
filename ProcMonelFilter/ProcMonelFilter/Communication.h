#ifndef _COMMUNICATION_H_INCLUDED_
#define _COMMUNICATION_H_INCLUDED_

#include <fltKernel.h>
#include "FilterStatus.h"

FILTERSTATUS
CommInitializeCommunication();

VOID
CommUninitializeCommunication();

_IRQL_requires_max_(APC_LEVEL)
NTSTATUS
CommSendMessage(
    _In_ PVOID InputBuffer,
    _In_ ULONG InputBufferSize,
    _Out_ PVOID OutputBuffer,
    _Inout_ PULONG OutputBufferSize
);

#endif