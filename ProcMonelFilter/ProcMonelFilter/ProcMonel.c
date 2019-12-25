#include "ProcMonel.h"
#include "Callbacks.h"


/*************************************************************************
Globals
*************************************************************************/
LARGE_INTEGER Cookie = { 0 };



/*************************************************************************
Procedures
*************************************************************************/

/*
* A callback routine implemented by a driver to notify the caller when a process is created or exits.
*/
VOID CreateProcessNotifyRoutine(
    _Inout_ PEPROCESS Process,
    _In_ HANDLE ProcessId,
    _In_opt_ PPS_CREATE_NOTIFY_INFO CreateInfo)
{
    UNREFERENCED_PARAMETER(Process);

    if (NULL != CreateInfo)     // if process was created
    {
        ProcessCreated(ProcessId, CreateInfo);
    }
    else                        // if process was terminated
    {
        ProcessTerminated(ProcessId);
    }
}


/*
*   Callback routine for PsSetCreateThreadNotifyRoutineEx
*/
VOID CreateThreadNotifyRoutine(
    _In_ HANDLE ProcessId,
    _In_ HANDLE ThreadId,
    _In_ BOOLEAN Create
)
{
    if (TRUE == Create)            // if thread was created
    {
        ThreadCreated(ProcessId, ThreadId);
    }
    else                         // if thread was terminated
    {
        ThreadTerminated(ProcessId, ThreadId);
    }
}


/*
* Callback routine for PsSetLoadImageNotifyRoutine
*/
VOID LoadImageNotifyRoutine(
    _In_ PUNICODE_STRING FullImageName,
    _In_ HANDLE ProcessId,
    _In_ PIMAGE_INFO ImageInfo
)
{
    if (NULL == FullImageName)
    {
        return;     // system is unable to obtain the full name of the image at process creation time.
    }

    // Handle is zero if the new loaded image is a driver
    if (!ProcessId || ImageInfo->SystemModeImage)
        return;

    ImageLoaded(ProcessId, FullImageName, ImageInfo);
}


NTSTATUS RegistryCallback(
    _In_ PVOID CallbackContext, 
    _In_ PVOID Argument1, 
    _In_ PVOID Argument2)
{
    UNREFERENCED_PARAMETER(CallbackContext);

    REG_NOTIFY_CLASS Operation = (REG_NOTIFY_CLASS)(ULONG_PTR)Argument1;
    switch (Operation)
    {
    case RegNtPostCreateKeyEx:
        RegistryKeyCreated((PREG_POST_OPERATION_INFORMATION)Argument2);
        break;
    case RegNtPostSetValueKey:
        RegistryValueKeySet((PREG_POST_OPERATION_INFORMATION)Argument2);
        break;
    case RegNtPostDeleteKey:
        RegistryKeyDeleted((PREG_POST_OPERATION_INFORMATION)Argument2);
        break;
    case RegNtPostDeleteValueKey:
        RegistryValueKeyDeleted((PREG_POST_OPERATION_INFORMATION)Argument2);
        break;
    default:
        break;
    }

    return STATUS_SUCCESS;
}


/*
* Registers a callback for Registry Keys
*/
NTSTATUS CreateRegisterCallback(
    _In_ PDRIVER_OBJECT DriverObject
)
{
    NTSTATUS status = STATUS_SUCCESS;
    UNICODE_STRING altitudeString = RTL_CONSTANT_STRING(L"370030");

    status = CmRegisterCallbackEx(
        RegistryCallback,
        &altitudeString,
        DriverObject, 
        NULL,
        &Cookie,
        NULL);
    if (!NT_SUCCESS(status))
    {
        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "ProcMonel: CmRegisterCallbackEx returned 0x%x\n", status);
        goto Exit;
    }

Exit:
    return status;
}



NTSTATUS ProcMonFltInitialize(_In_ PDRIVER_OBJECT DriverObject)
{
    NTSTATUS status = STATUS_SUCCESS;

    UNREFERENCED_PARAMETER(DriverObject);
    status = PsSetCreateProcessNotifyRoutineEx(&CreateProcessNotifyRoutine, FALSE);
    if (!NT_SUCCESS(status))
    {
        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "ProcMonel: PsSetCreateProcessNotifyRoutineEx returned 0x%x\n", status);
        goto Exit;
    }

    status = PsSetCreateThreadNotifyRoutineEx(
        PsCreateThreadNotifyNonSystem,
        (PVOID)((size_t)(CreateThreadNotifyRoutine))
    );
    if (!NT_SUCCESS(status))
    {
        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "ProcMonel: PsSetCreateThreadNotifyRoutineEx returned 0x%x\n", status);
        goto Exit;
    }

    status = PsSetLoadImageNotifyRoutine(&LoadImageNotifyRoutine);
    if (!NT_SUCCESS(status))
    {
        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "ProcMonel: PsSetLoadImageNotifyRoutine returned 0x%x\n", status);
        goto Exit;
    }
    
    status = CreateRegisterCallback(DriverObject);
    if (!NT_SUCCESS(status))
    {
        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "ProcMonel: CreateRegisterCallback returned 0x%x\n", status);
        goto Exit;
    }

Exit:

    return status;
}


NTSTATUS ProcMonFltUninitialize()
{
    NTSTATUS status = STATUS_SUCCESS;
    
    status = CmUnRegisterCallback(Cookie);
    if (!NT_SUCCESS(status))
    {
        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "ProcMonel: CmUnRegisterCallback returned 0x%x\n", status);
        goto Exit;
    }

    status = PsRemoveLoadImageNotifyRoutine(&LoadImageNotifyRoutine);
    if (!NT_SUCCESS(status))
    {
        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "ProcMonel: PsRemoveLoadImageNotifyRoutine returned 0x%x\n", status);
        goto Exit;
    }

    status = PsRemoveCreateThreadNotifyRoutine(CreateThreadNotifyRoutine);
    if (!NT_SUCCESS(status))
    {
        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "ProcMonel: PsRemoveCreateThreadNotifyRoutine returned 0x%x\n", status);
        goto Exit;
    }

    status = PsSetCreateProcessNotifyRoutineEx(&CreateProcessNotifyRoutine, TRUE);
    if (!NT_SUCCESS(status))
    {
        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "ProcMonel: PsSetCreateProcessNotifyRoutineEx returned 0x%x\n", status);
        goto Exit;
    }

    
Exit:

    return status;
}


void FileCallback(PFILE_EVENT FileEvent)
{
    if (IRP_MJ_CREATE == FileEvent->majorFileEventType)
    {
        if ((FILE_CREATED == FileEvent->information) ||
            (FILE_OVERWRITTEN == FileEvent->information) ||
            (FILE_SUPERSEDED == FileEvent->information))
        {
            FileCreated(FileEvent);
        }
    }
    else if (IRP_MJ_READ == FileEvent->majorFileEventType)
    {
        FileRead(FileEvent);
    }
    else if (IRP_MJ_WRITE == FileEvent->majorFileEventType)
    {
        FileWritten(FileEvent);
    }
    else if (IRP_MJ_CLEANUP == FileEvent->majorFileEventType)
    {
        FileCleanedup(FileEvent);
    }
    else if (IRP_MJ_CLOSE == FileEvent->majorFileEventType)
    {
        FileClosed(FileEvent);
    }
    else if (IRP_MJ_SET_INFORMATION == FileEvent->majorFileEventType)
    {
        FileSetAttributes(FileEvent);
    }
}