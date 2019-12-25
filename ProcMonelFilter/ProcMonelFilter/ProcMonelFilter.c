/*++

Module Name:

    ProcMonelFilter.c

Abstract:

    This is the main module of the ProcMonelFilter miniFilter driver.

Environment:

    Kernel mode

--*/

#include <dontuse.h>
#include <suppress.h>
#include "ProcMonel.h"
#include "GlobalData.h"
#include <ntstrsafe.h>


#define FILE_POOL_TAG 'cfm'


#pragma prefast(disable:__WARNING_ENCODE_MEMBER_FUNCTION_POINTER, "Not valid for kernel mode drivers")
/*************************************************************************
Globals
*************************************************************************/
GLOBAL_DATA gDrv;

ULONG_PTR OperationStatusCtx = 1;

#define PTDBG_TRACE_ROUTINES            0x00000001
#define PTDBG_TRACE_OPERATION_STATUS    0x00000002

ULONG gTraceFlags = PTDBG_TRACE_ROUTINES;


#define LogInfo( _dbgLevel, _string, ... )          \
    (FlagOn(gTraceFlags,(_dbgLevel)) ?              \
        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,  _string, __VA_ARGS__) :                          \
        ((int)0))

/*************************************************************************
    Prototypes
*************************************************************************/

EXTERN_C_START

DRIVER_INITIALIZE DriverEntry;
NTSTATUS
DriverEntry (
    _In_ PDRIVER_OBJECT DriverObject,
    _In_ PUNICODE_STRING RegistryPath
    );

NTSTATUS
ProcMonelFilterInstanceSetup (
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_SETUP_FLAGS Flags,
    _In_ DEVICE_TYPE VolumeDeviceType,
    _In_ FLT_FILESYSTEM_TYPE VolumeFilesystemType
    );

VOID
ProcMonelFilterInstanceTeardownStart (
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_TEARDOWN_FLAGS Flags
    );

VOID
ProcMonelFilterInstanceTeardownComplete (
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_TEARDOWN_FLAGS Flags
    );

NTSTATUS
ProcMonelFilterUnload (
    _In_ FLT_FILTER_UNLOAD_FLAGS Flags
    );

NTSTATUS
ProcMonelFilterInstanceQueryTeardown (
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_QUERY_TEARDOWN_FLAGS Flags
    );

FLT_PREOP_CALLBACK_STATUS
ProcMonelFilterPreOperation (
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _Flt_CompletionContext_Outptr_ PVOID *CompletionContext
    );

VOID
ProcMonelFilterOperationStatusCallback (
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ PFLT_IO_PARAMETER_BLOCK ParameterSnapshot,
    _In_ NTSTATUS OperationStatus,
    _In_ PVOID RequesterContext
    );

FLT_POSTOP_CALLBACK_STATUS
ProcMonelFilterPostOperation (
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_opt_ PVOID CompletionContext,
    _In_ FLT_POST_OPERATION_FLAGS Flags
    );

FLT_PREOP_CALLBACK_STATUS
ProcMonelFilterPreOperationNoPostOperation (
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _Flt_CompletionContext_Outptr_ PVOID *CompletionContext
    );

BOOLEAN
ProcMonelFilterDoRequestOperationStatus(
    _In_ PFLT_CALLBACK_DATA Data
    );

EXTERN_C_END

//
//  Assign text sections for each routine.
//

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(PAGE, ProcMonelFilterUnload)
#pragma alloc_text(PAGE, ProcMonelFilterInstanceQueryTeardown)
#pragma alloc_text(PAGE, ProcMonelFilterInstanceSetup)
#pragma alloc_text(PAGE, ProcMonelFilterInstanceTeardownStart)
#pragma alloc_text(PAGE, ProcMonelFilterInstanceTeardownComplete)
#endif

//
//  operation registration
//

CONST FLT_OPERATION_REGISTRATION Callbacks[] = {


    { IRP_MJ_CREATE,
      0,
      ProcMonelFilterPreOperation,
      ProcMonelFilterPostOperation },

    { IRP_MJ_CREATE_NAMED_PIPE,
      0,
      ProcMonelFilterPreOperation,
      ProcMonelFilterPostOperation },

    { IRP_MJ_CLOSE,
      0,
      ProcMonelFilterPreOperation,
      ProcMonelFilterPostOperation },

    { IRP_MJ_READ,
      0,
      ProcMonelFilterPreOperation,
      ProcMonelFilterPostOperation },

    { IRP_MJ_WRITE,
      0,
      ProcMonelFilterPreOperation,
      ProcMonelFilterPostOperation },

     { IRP_MJ_CLEANUP,
      0,
      ProcMonelFilterPreOperation,
      ProcMonelFilterPostOperation },

#if 0 // TODO - List all of the requests to filter.
    { IRP_MJ_QUERY_INFORMATION,
      0,
      ProcMonelFilterPreOperation,
      ProcMonelFilterPostOperation },

    { IRP_MJ_SET_INFORMATION,
      0,
      ProcMonelFilterPreOperation,
      ProcMonelFilterPostOperation },

    { IRP_MJ_QUERY_EA,
      0,
      ProcMonelFilterPreOperation,
      ProcMonelFilterPostOperation },

    { IRP_MJ_SET_EA,
      0,
      ProcMonelFilterPreOperation,
      ProcMonelFilterPostOperation },

    { IRP_MJ_FLUSH_BUFFERS,
      0,
      ProcMonelFilterPreOperation,
      ProcMonelFilterPostOperation },

    { IRP_MJ_QUERY_VOLUME_INFORMATION,
      0,
      ProcMonelFilterPreOperation,
      ProcMonelFilterPostOperation },

    { IRP_MJ_SET_VOLUME_INFORMATION,
      0,
      ProcMonelFilterPreOperation,
      ProcMonelFilterPostOperation },

    { IRP_MJ_DIRECTORY_CONTROL,
      0,
      ProcMonelFilterPreOperation,
      ProcMonelFilterPostOperation },

    { IRP_MJ_FILE_SYSTEM_CONTROL,
      0,
      ProcMonelFilterPreOperation,
      ProcMonelFilterPostOperation },

    { IRP_MJ_DEVICE_CONTROL,
      0,
      ProcMonelFilterPreOperation,
      ProcMonelFilterPostOperation },

    { IRP_MJ_INTERNAL_DEVICE_CONTROL,
      0,
      ProcMonelFilterPreOperation,
      ProcMonelFilterPostOperation },

    { IRP_MJ_SHUTDOWN,
      0,
      ProcMonelFilterPreOperationNoPostOperation,
      NULL },                               //post operations not supported

    { IRP_MJ_LOCK_CONTROL,
      0,
      ProcMonelFilterPreOperation,
      ProcMonelFilterPostOperation },

    { IRP_MJ_CLEANUP,
      0,
      ProcMonelFilterPreOperation,
      ProcMonelFilterPostOperation },

    { IRP_MJ_CREATE_MAILSLOT,
      0,
      ProcMonelFilterPreOperation,
      ProcMonelFilterPostOperation },

    { IRP_MJ_QUERY_SECURITY,
      0,
      ProcMonelFilterPreOperation,
      ProcMonelFilterPostOperation },

    { IRP_MJ_SET_SECURITY,
      0,
      ProcMonelFilterPreOperation,
      ProcMonelFilterPostOperation },

    { IRP_MJ_QUERY_QUOTA,
      0,
      ProcMonelFilterPreOperation,
      ProcMonelFilterPostOperation },

    { IRP_MJ_SET_QUOTA,
      0,
      ProcMonelFilterPreOperation,
      ProcMonelFilterPostOperation },

    { IRP_MJ_PNP,
      0,
      ProcMonelFilterPreOperation,
      ProcMonelFilterPostOperation },

    { IRP_MJ_ACQUIRE_FOR_SECTION_SYNCHRONIZATION,
      0,
      ProcMonelFilterPreOperation,
      ProcMonelFilterPostOperation },

    { IRP_MJ_RELEASE_FOR_SECTION_SYNCHRONIZATION,
      0,
      ProcMonelFilterPreOperation,
      ProcMonelFilterPostOperation },

    { IRP_MJ_ACQUIRE_FOR_MOD_WRITE,
      0,
      ProcMonelFilterPreOperation,
      ProcMonelFilterPostOperation },

    { IRP_MJ_RELEASE_FOR_MOD_WRITE,
      0,
      ProcMonelFilterPreOperation,
      ProcMonelFilterPostOperation },

    { IRP_MJ_ACQUIRE_FOR_CC_FLUSH,
      0,
      ProcMonelFilterPreOperation,
      ProcMonelFilterPostOperation },

    { IRP_MJ_RELEASE_FOR_CC_FLUSH,
      0,
      ProcMonelFilterPreOperation,
      ProcMonelFilterPostOperation },

    { IRP_MJ_FAST_IO_CHECK_IF_POSSIBLE,
      0,
      ProcMonelFilterPreOperation,
      ProcMonelFilterPostOperation },

    { IRP_MJ_NETWORK_QUERY_OPEN,
      0,
      ProcMonelFilterPreOperation,
      ProcMonelFilterPostOperation },

    { IRP_MJ_MDL_READ,
      0,
      ProcMonelFilterPreOperation,
      ProcMonelFilterPostOperation },

    { IRP_MJ_MDL_READ_COMPLETE,
      0,
      ProcMonelFilterPreOperation,
      ProcMonelFilterPostOperation },

    { IRP_MJ_PREPARE_MDL_WRITE,
      0,
      ProcMonelFilterPreOperation,
      ProcMonelFilterPostOperation },

    { IRP_MJ_MDL_WRITE_COMPLETE,
      0,
      ProcMonelFilterPreOperation,
      ProcMonelFilterPostOperation },

    { IRP_MJ_VOLUME_MOUNT,
      0,
      ProcMonelFilterPreOperation,
      ProcMonelFilterPostOperation },

    { IRP_MJ_VOLUME_DISMOUNT,
      0,
      ProcMonelFilterPreOperation,
      ProcMonelFilterPostOperation },

#endif // TODO

    { IRP_MJ_OPERATION_END }
};

//
//  This defines what we want to filter with FltMgr
//

CONST FLT_REGISTRATION FilterRegistration = {

    sizeof( FLT_REGISTRATION ),         //  Size
    FLT_REGISTRATION_VERSION,           //  Version
    0,                                  //  Flags

    NULL,                               //  Context
    Callbacks,                          //  Operation callbacks

    ProcMonelFilterUnload,                           //  MiniFilterUnload

    ProcMonelFilterInstanceSetup,                    //  InstanceSetup
    ProcMonelFilterInstanceQueryTeardown,            //  InstanceQueryTeardown
    ProcMonelFilterInstanceTeardownStart,            //  InstanceTeardownStart
    ProcMonelFilterInstanceTeardownComplete,         //  InstanceTeardownComplete

    NULL,                               //  GenerateFileName
    NULL,                               //  GenerateDestinationFileName
    NULL                                //  NormalizeNameComponent

};



NTSTATUS
ProcMonelFilterInstanceSetup (
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_SETUP_FLAGS Flags,
    _In_ DEVICE_TYPE VolumeDeviceType,
    _In_ FLT_FILESYSTEM_TYPE VolumeFilesystemType
    )
/*++

Routine Description:

    This routine is called whenever a new instance is created on a volume. This
    gives us a chance to decide if we need to attach to this volume or not.

    If this routine is not defined in the registration structure, automatic
    instances are always created.

Arguments:

    FltObjects - Pointer to the FLT_RELATED_OBJECTS data structure containing
        opaque handles to this filter, instance and its associated volume.

    Flags - Flags describing the reason for this attach request.

Return Value:

    STATUS_SUCCESS - attach
    STATUS_FLT_DO_NOT_ATTACH - do not attach

--*/
{
    UNREFERENCED_PARAMETER( FltObjects );
    UNREFERENCED_PARAMETER( Flags );
    UNREFERENCED_PARAMETER( VolumeDeviceType );
    UNREFERENCED_PARAMETER( VolumeFilesystemType );

    PAGED_CODE();

    LogInfo( PTDBG_TRACE_ROUTINES,
                  ("ProcMonelFilter!ProcMonelFilterInstanceSetup: Entered\n") );

    return STATUS_SUCCESS;
}


NTSTATUS
ProcMonelFilterInstanceQueryTeardown (
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_QUERY_TEARDOWN_FLAGS Flags
    )
/*++

Routine Description:

    This is called when an instance is being manually deleted by a
    call to FltDetachVolume or FilterDetach thereby giving us a
    chance to fail that detach request.

    If this routine is not defined in the registration structure, explicit
    detach requests via FltDetachVolume or FilterDetach will always be
    failed.

Arguments:

    FltObjects - Pointer to the FLT_RELATED_OBJECTS data structure containing
        opaque handles to this filter, instance and its associated volume.

    Flags - Indicating where this detach request came from.

Return Value:

    Returns the status of this operation.

--*/
{
    UNREFERENCED_PARAMETER( FltObjects );
    UNREFERENCED_PARAMETER( Flags );

    PAGED_CODE();

    LogInfo( PTDBG_TRACE_ROUTINES,
                  ("ProcMonelFilter!ProcMonelFilterInstanceQueryTeardown: Entered\n") );

    return STATUS_SUCCESS;
}


VOID
ProcMonelFilterInstanceTeardownStart (
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_TEARDOWN_FLAGS Flags
    )
/*++

Routine Description:

    This routine is called at the start of instance teardown.

Arguments:

    FltObjects - Pointer to the FLT_RELATED_OBJECTS data structure containing
        opaque handles to this filter, instance and its associated volume.

    Flags - Reason why this instance is being deleted.

Return Value:

    None.

--*/
{
    UNREFERENCED_PARAMETER( FltObjects );
    UNREFERENCED_PARAMETER( Flags );

    PAGED_CODE();

    LogInfo( PTDBG_TRACE_ROUTINES,
                  ("ProcMonelFilter!ProcMonelFilterInstanceTeardownStart: Entered\n") );
}


VOID
ProcMonelFilterInstanceTeardownComplete (
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_TEARDOWN_FLAGS Flags
    )
/*++

Routine Description:

    This routine is called at the end of instance teardown.

Arguments:

    FltObjects - Pointer to the FLT_RELATED_OBJECTS data structure containing
        opaque handles to this filter, instance and its associated volume.

    Flags - Reason why this instance is being deleted.

Return Value:

    None.

--*/
{
    UNREFERENCED_PARAMETER( FltObjects );
    UNREFERENCED_PARAMETER( Flags );

    PAGED_CODE();

    LogInfo( PTDBG_TRACE_ROUTINES,
                  ("ProcMonelFilter!ProcMonelFilterInstanceTeardownComplete: Entered\n") );
}


/*************************************************************************
    MiniFilter initialization and unload routines.
*************************************************************************/

NTSTATUS
DriverEntry (
    _In_ PDRIVER_OBJECT DriverObject,
    _In_ PUNICODE_STRING RegistryPath
    )
/*++

Routine Description:

    This is the initialization routine for this miniFilter driver.  This
    registers with FltMgr and initializes all global data structures.

Arguments:

    DriverObject - Pointer to driver object created by the system to
        represent this driver.

    RegistryPath - Unicode string identifying where the parameters for this
        driver are located in the registry.

Return Value:

    Routine can return non success error codes.

--*/
{
    NTSTATUS status;

    UNREFERENCED_PARAMETER( RegistryPath );

    LogInfo( PTDBG_TRACE_ROUTINES,
                  ("ProcMonelFilter!DriverEntry: Entered\n") );

    //
    //  Register with FltMgr to tell it our callback routines
    //

    status = FltRegisterFilter( DriverObject,
                                &FilterRegistration,
                                &gDrv.FilterHandle );

    FLT_ASSERT( NT_SUCCESS( status ) );

    if (NT_SUCCESS(status)) 
    {
        //
        //  Initialize Communication
        // 
        /*status = InitializeFilterCommunicationPort();
        if (!NT_SUCCESS(status)) {

            FltUnregisterFilter(gDrv.FilterHandle);
            return status;
        }*/

        //
        //  Initialize ProcMonFilter
        //
        status = ProcMonFltInitialize(DriverObject);
        if (!NT_SUCCESS(status))
        {
            //UninitializeFilterCommunicationPort();
            FltUnregisterFilter(gDrv.FilterHandle);
            return status;
        }

        //
        //  Start filtering i/o
        //
        status = FltStartFiltering(gDrv.FilterHandle);
        if (!NT_SUCCESS(status )) 
        {
            //UninitializeFilterCommunicationPort();
            ProcMonFltUninitialize();
            FltUnregisterFilter(gDrv.FilterHandle);
        }
    }



    return status;
}

NTSTATUS
ProcMonelFilterUnload (
    _In_ FLT_FILTER_UNLOAD_FLAGS Flags
    )
/*++

Routine Description:

    This is the unload routine for this miniFilter driver. This is called
    when the minifilter is about to be unloaded. We can fail this unload
    request if this is not a mandatory unload indicated by the Flags
    parameter.

Arguments:

    Flags - Indicating if this is a mandatory unload.

Return Value:

    Returns STATUS_SUCCESS.

--*/
{
    UNREFERENCED_PARAMETER( Flags );

    PAGED_CODE();

    LogInfo( PTDBG_TRACE_ROUTINES,
                  ("ProcMonelFilter!ProcMonelFilterUnload: Entered\n") );

    //UninitializeFilterCommunicationPort();
    ProcMonFltUninitialize();
    FltUnregisterFilter(gDrv.FilterHandle);

    return STATUS_SUCCESS;
}


/*************************************************************************
    MiniFilter callback routines.
*************************************************************************/
FLT_PREOP_CALLBACK_STATUS
ProcMonelFilterPreOperation (
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _Flt_CompletionContext_Outptr_ PVOID *CompletionContext
    )
{
    NTSTATUS status;
    PFILE_EVENT fileEvent = NULL;
    PFLT_FILE_NAME_INFORMATION fileNameInformation = NULL;
    UNICODE_STRING filePath = { 0 };
    BOOLEAN pathNameFound = FALSE;
    PEPROCESS process = NULL;

    UNREFERENCED_PARAMETER(CompletionContext);
    
    if (FLT_IS_FS_FILTER_OPERATION(Data))
    {
        return FLT_PREOP_SUCCESS_NO_CALLBACK;
    }

    filePath.Length = 0;
    filePath.MaximumLength = NTSTRSAFE_UNICODE_STRING_MAX_CCH * sizeof(WCHAR);          // maximum path name allowed in windows
    filePath.Buffer = ExAllocatePoolWithTag(NonPagedPool, filePath.MaximumLength, FILE_POOL_TAG);

    if (filePath.Buffer == NULL)
    {
        return FLT_PREOP_SUCCESS_NO_CALLBACK;
    }

    if ((NULL != FltObjects->FileObject) && (NULL != Data))
    {
        status = FltGetFileNameInformation(
            Data,
            FLT_FILE_NAME_NORMALIZED | FLT_FILE_NAME_QUERY_ALWAYS_ALLOW_CACHE_LOOKUP,
            &fileNameInformation);
        if (NT_SUCCESS(status))
        {
            if (fileNameInformation->Name.Length > 0)
            {
                RtlUnicodeStringCopy(&filePath, &fileNameInformation->Name);
                pathNameFound = TRUE;
            }
            
            // Release the file name information structure if it was used 
            if (fileNameInformation != NULL)
            {
                FltReleaseFileNameInformation(fileNameInformation);
            }
        }
        else
        {
            NTSTATUS lstatus;
            PFLT_FILE_NAME_INFORMATION lFileNameInformation;

            lstatus = FltGetFileNameInformation(
                Data,
                FLT_FILE_NAME_OPENED | FLT_FILE_NAME_QUERY_ALWAYS_ALLOW_CACHE_LOOKUP,
                &lFileNameInformation);
            if (NT_SUCCESS(lstatus))
            {
                if (lFileNameInformation->Name.Length > 0)
                {
                    RtlUnicodeStringCopy(&filePath, &lFileNameInformation->Name);
                    pathNameFound = TRUE;
                }

                if (lFileNameInformation != NULL)
                {
                    FltReleaseFileNameInformation(lFileNameInformation);
                }
            }
        }
    }

    if (!pathNameFound)
    {
        RtlUnicodeStringCatString(&filePath, L"UNKNOWN");
    }

    fileEvent = ExAllocatePoolWithTag(NonPagedPool, sizeof(FILE_EVENT) + filePath.Length + sizeof(WCHAR), FILE_POOL_TAG);
    if (fileEvent == NULL)
    {
        ExFreePoolWithTag(filePath.Buffer, FILE_POOL_TAG);
        return FLT_PREOP_SUCCESS_NO_CALLBACK;
    }
    
    fileEvent->filePath = filePath;
    fileEvent->majorFileEventType = Data->Iopb->MajorFunction;
    fileEvent->minorFileEventType = Data->Iopb->MinorFunction;

    if (FltObjects->FileObject != NULL)
    {
        fileEvent->flags = FltObjects->FileObject->Flags;
    }

    if (NULL != Data->Thread)
    {
        process = IoThreadToProcess(Data->Thread);
        fileEvent->processId = PsGetProcessId(process);
    }
    else
    {
        fileEvent->processId = PsGetCurrentProcessId();     // ProcessID may be incorrect
    }

    if (Data->Iopb->MajorFunction == IRP_MJ_SHUTDOWN)
    {
        ProcMonelFilterPostOperation(Data, FltObjects, fileEvent, 0);
        return FLT_PREOP_SUCCESS_NO_CALLBACK;
    }
    else
    {
        *CompletionContext = fileEvent;
        return FLT_PREOP_SUCCESS_WITH_CALLBACK;
    }
}



VOID
ProcMonelFilterOperationStatusCallback (
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ PFLT_IO_PARAMETER_BLOCK ParameterSnapshot,
    _In_ NTSTATUS OperationStatus,
    _In_ PVOID RequesterContext
    )
/*++

Routine Description:

    This routine is called when the given operation returns from the call
    to IoCallDriver.  This is useful for operations where STATUS_PENDING
    means the operation was successfully queued.  This is useful for OpLocks
    and directory change notification operations.

    This callback is called in the context of the originating thread and will
    never be called at DPC level.  The file object has been correctly
    referenced so that you can access it.  It will be automatically
    dereferenced upon return.

    This is non-pageable because it could be called on the paging path

Arguments:

    FltObjects - Pointer to the FLT_RELATED_OBJECTS data structure containing
        opaque handles to this filter, instance, its associated volume and
        file object.

    RequesterContext - The context for the completion routine for this
        operation.

    OperationStatus -

Return Value:

    The return value is the status of the operation.

--*/
{
    UNREFERENCED_PARAMETER( FltObjects );

    LogInfo( PTDBG_TRACE_ROUTINES,
                  ("ProcMonelFilter!ProcMonelFilterOperationStatusCallback: Entered\n") );

    LogInfo( PTDBG_TRACE_OPERATION_STATUS,
                  ("ProcMonelFilter!ProcMonelFilterOperationStatusCallback: Status=%08x ctx=%p IrpMj=%02x.%02x \"%s\"\n",
                   OperationStatus,
                   RequesterContext,
                   ParameterSnapshot->MajorFunction,
                   ParameterSnapshot->MinorFunction,
                   FltGetIrpName(ParameterSnapshot->MajorFunction)) );
}


FLT_POSTOP_CALLBACK_STATUS
ProcMonelFilterPostOperation (
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_opt_ PVOID CompletionContext,
    _In_ FLT_POST_OPERATION_FLAGS Flags
    )
/*++

Routine Description:

    This routine is the post-operation completion routine for this
    miniFilter.

    This is non-pageable because it may be called at DPC level.

Arguments:

    Data - Pointer to the filter callbackData that is passed to us.

    FltObjects - Pointer to the FLT_RELATED_OBJECTS data structure containing
        opaque handles to this filter, instance, its associated volume and
        file object.

    CompletionContext - The completion context set in the pre-operation routine.

    Flags - Denotes whether the completion is successful or is being drained.

Return Value:

    The return value is the status of the operation.

--*/
{
    UNREFERENCED_PARAMETER( FltObjects );
    UNREFERENCED_PARAMETER(Flags);
    PFILE_EVENT fileEvent = NULL;

    fileEvent = (PFILE_EVENT)CompletionContext;

    if (NULL == fileEvent)
    {
        return FLT_POSTOP_FINISHED_PROCESSING;
    }

    fileEvent->status = Data->IoStatus.Status;
    fileEvent->information = Data->IoStatus.Information;

    FileCallback(fileEvent);

    if (NULL != fileEvent)
    {
        ExFreePoolWithTag(fileEvent->filePath.Buffer, FILE_POOL_TAG);
        ExFreePoolWithTag(fileEvent, FILE_POOL_TAG);
        fileEvent = NULL;
    }

    return FLT_POSTOP_FINISHED_PROCESSING;
}


FLT_PREOP_CALLBACK_STATUS
ProcMonelFilterPreOperationNoPostOperation (
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _Flt_CompletionContext_Outptr_ PVOID *CompletionContext
    )
/*++

Routine Description:

    This routine is a pre-operation dispatch routine for this miniFilter.

    This is non-pageable because it could be called on the paging path

Arguments:

    Data - Pointer to the filter callbackData that is passed to us.

    FltObjects - Pointer to the FLT_RELATED_OBJECTS data structure containing
        opaque handles to this filter, instance, its associated volume and
        file object.

    CompletionContext - The context for the completion routine for this
        operation.

Return Value:

    The return value is the status of the operation.

--*/
{
    UNREFERENCED_PARAMETER( Data );
    UNREFERENCED_PARAMETER( FltObjects );
    UNREFERENCED_PARAMETER( CompletionContext );

    LogInfo( PTDBG_TRACE_ROUTINES,
                  ("ProcMonelFilter!ProcMonelFilterPreOperationNoPostOperation: Entered\n") );

    // This template code does not do anything with the callbackData, but
    // rather returns FLT_PREOP_SUCCESS_NO_CALLBACK.
    // This passes the request down to the next miniFilter in the chain.

    return FLT_PREOP_SUCCESS_NO_CALLBACK;
}


BOOLEAN
ProcMonelFilterDoRequestOperationStatus(
    _In_ PFLT_CALLBACK_DATA Data
    )
/*++

Routine Description:

    This identifies those operations we want the operation status for.  These
    are typically operations that return STATUS_PENDING as a normal completion
    status.

Arguments:

Return Value:

    TRUE - If we want the operation status
    FALSE - If we don't

--*/
{
    PFLT_IO_PARAMETER_BLOCK iopb = Data->Iopb;

    //
    //  return boolean state based on which operations we are interested in
    //

    return (BOOLEAN)

            //
            //  Check for oplock operations
            //

             (((iopb->MajorFunction == IRP_MJ_FILE_SYSTEM_CONTROL) &&
               ((iopb->Parameters.FileSystemControl.Common.FsControlCode == FSCTL_REQUEST_FILTER_OPLOCK)  ||
                (iopb->Parameters.FileSystemControl.Common.FsControlCode == FSCTL_REQUEST_BATCH_OPLOCK)   ||
                (iopb->Parameters.FileSystemControl.Common.FsControlCode == FSCTL_REQUEST_OPLOCK_LEVEL_1) ||
                (iopb->Parameters.FileSystemControl.Common.FsControlCode == FSCTL_REQUEST_OPLOCK_LEVEL_2)))

              ||

              //
              //    Check for directy change notification
              //

              ((iopb->MajorFunction == IRP_MJ_DIRECTORY_CONTROL) &&
               (iopb->MinorFunction == IRP_MN_NOTIFY_CHANGE_DIRECTORY))
             );
}
