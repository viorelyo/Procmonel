#include "Communication.h"
#include "GlobalData.h"
#include "Shared.h"

#define MSG_DELAY_ONE_MICROSECOND               (-10)
#define MSG_DELAY_ONE_MILLISECOND               (MSG_DELAY_ONE_MICROSECOND*1000)
#define MSG_DELAY_ONE_SECOND                    (MSG_DELAY_ONE_MILLISECOND*1000)

NTSTATUS
CbConnectNotify(
    _In_ PFLT_PORT ClientPort,
    _In_opt_ PVOID ServerPortCookie,
    _In_reads_bytes_opt_(SizeOfContext) PVOID Context,
    _In_ ULONG SizeOfContext,
    _Outptr_result_maybenull_ PVOID *ConnectionPortCookie
)
{
    UNREFERENCED_PARAMETER(ServerPortCookie);
    UNREFERENCED_PARAMETER(Context);
    UNREFERENCED_PARAMETER(ConnectionPortCookie);
    UNREFERENCED_PARAMETER(SizeOfContext);

    gDrv.Communication.ClientPort = ClientPort;
    *ConnectionPortCookie = 0;
    return STATUS_SUCCESS;
}

VOID
CbDisconnectNotify(
    _In_opt_ PVOID ConnectionCookie
)
{
    UNREFERENCED_PARAMETER(ConnectionCookie);
    FltCloseClientPort(gDrv.FilterHandle, &gDrv.Communication.ClientPort);
    gDrv.Communication.ClientPort = NULL;
}


NTSTATUS
CbMessageNotify(
    _In_opt_ PVOID PortCookie,
    _In_reads_bytes_opt_(InputBufferLength) PVOID InputBuffer,
    _In_ ULONG InputBufferLength,
    _Out_writes_bytes_to_opt_(OutputBufferLength, *ReturnOutputBufferLength) PVOID OutputBuffer,
    _In_ ULONG OutputBufferLength,
    _Out_ PULONG ReturnOutputBufferLength
)
{
    UNREFERENCED_PARAMETER(PortCookie);
    UNREFERENCED_PARAMETER(InputBuffer);
    UNREFERENCED_PARAMETER(InputBufferLength);
    UNREFERENCED_PARAMETER(OutputBuffer);
    UNREFERENCED_PARAMETER(OutputBufferLength);

    DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "!!!! MESSAGE !!!!");
    *ReturnOutputBufferLength = 0;

    return STATUS_SUCCESS;
}

FILTERSTATUS
CommInitializeCommunication()
{
    NTSTATUS      status = STATUS_UNSUCCESSFUL;
    UNICODE_STRING ustrPortName;
    OBJECT_ATTRIBUTES objAttr;
    PSECURITY_DESCRIPTOR pSecurityDescriptor = NULL;
    BOOLEAN        bFreeSecurityDescriptor = FALSE;

    RtlInitUnicodeString(&ustrPortName, MY_FILTER_PORT_NAME);

    __try
    {
        status = FltBuildDefaultSecurityDescriptor(&pSecurityDescriptor, FLT_PORT_ALL_ACCESS);
        if (!NT_SUCCESS(status))
        {
            __leave;
        }
        bFreeSecurityDescriptor = TRUE;

        InitializeObjectAttributes(&objAttr,
            &ustrPortName,
            OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
            NULL,
            pSecurityDescriptor);
        status = FltCreateCommunicationPort(gDrv.FilterHandle,
            &gDrv.Communication.ServerPort,
            &objAttr,
            NULL,    // currently not using a communication cookie
            CbConnectNotify,
            CbDisconnectNotify,
            CbMessageNotify,
            1       // using only one connection
        );
        if (!NT_SUCCESS(status))
        {
            __leave;
        }

        status = STATUS_SUCCESS;
    }
    __finally
    {
        if (bFreeSecurityDescriptor)
        {
            FltFreeSecurityDescriptor(pSecurityDescriptor);
        }
    }

    return status;
}


VOID CommUninitializeCommunication()
{
    FltCloseClientPort(gDrv.FilterHandle, &gDrv.Communication.ClientPort);
    gDrv.Communication.ClientPort = NULL;
    FltCloseCommunicationPort(gDrv.Communication.ServerPort);
}

NTSTATUS
CommSendMessage(
    _In_ PVOID InputBuffer,
    _In_ ULONG InputBufferSize,
    _Out_ PVOID OutputBuffer,
    _Inout_ PULONG OutputBufferSize
)
{
    NTSTATUS status = STATUS_UNSUCCESSFUL;
    LARGE_INTEGER timeout;
    timeout.QuadPart = 60 * MSG_DELAY_ONE_SECOND; // 60 seconds

    __try
    {
        status = FltSendMessage(gDrv.FilterHandle,
            &gDrv.Communication.ClientPort,
            InputBuffer,
            InputBufferSize,
            OutputBuffer,
            OutputBufferSize,
            &timeout);
        if (!NT_SUCCESS(status))
        {
            DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "FltSendMessage failed with status 0x%X", status);
            // not success
            __leave;
        }

        if (status == STATUS_TIMEOUT)
        {
            status = STATUS_MESSAGE_LOST;
            __leave;
        }

        status = STATUS_SUCCESS;
    }
    __finally
    {
    }

    return status;
}

