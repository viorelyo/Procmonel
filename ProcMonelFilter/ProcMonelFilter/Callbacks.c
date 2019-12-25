#include "Callbacks.h"
#include <ntstrsafe.h>


#define REGISTRY_POOL_TAG '1gaT'


void ProcessCreated(
    HANDLE ProcessId,
    PPS_CREATE_NOTIFY_INFO CreateInfo)
{
    LARGE_INTEGER Now = { 0 };
    LARGE_INTEGER LocalNow = { 0 };
    TIME_FIELDS NowTF = { 0 };

    KeQuerySystemTimePrecise(&Now);
    ExSystemTimeToLocalTime(&Now, &LocalNow);
    RtlTimeToTimeFields(&LocalNow, &NowTF);

    DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "ProcMonel: [Time]: {%hu:%hu:%hu:%hu} | [Operation]: Process Created |[PID]: 0x%p | [Path]: %wZ | [Result]: %s | [Details]: |\n",
        NowTF.Hour,
        NowTF.Minute,
        NowTF.Second,
        NowTF.Milliseconds,
        (PVOID)ProcessId,
        CreateInfo->ImageFileName,
        "Success"
    );
}


void ProcessTerminated(HANDLE ProcessId)
{
    LARGE_INTEGER Now = { 0 };
    LARGE_INTEGER LocalNow = { 0 };
    TIME_FIELDS NowTF = { 0 };

    KeQuerySystemTimePrecise(&Now);
    ExSystemTimeToLocalTime(&Now, &LocalNow);
    RtlTimeToTimeFields(&LocalNow, &NowTF);

    DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "ProcMonel: [Time]: (%hu:%hu:%hu.%hu) | [Operation]: Process Terminated | [PID]: 0x%p | [Result]: %s | [Details]: |\n",
        NowTF.Hour,
        NowTF.Minute,
        NowTF.Second,
        NowTF.Milliseconds,
        (PVOID)ProcessId,
        "Success"
    );
}


void ThreadCreated(HANDLE ProcessId, HANDLE ThreadId)
{
    LARGE_INTEGER Now = { 0 };
    LARGE_INTEGER LocalNow = { 0 };
    TIME_FIELDS NowTF = { 0 };

    KeQuerySystemTimePrecise(&Now);
    ExSystemTimeToLocalTime(&Now, &LocalNow);
    RtlTimeToTimeFields(&LocalNow, &NowTF);

    DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "ProcMonel: [Time]: (%hu:%hu:%hu.%hu) | [Operation]: Thread Created | [PID]: 0x%p | [ThreadID]: 0x%p | [Result]: %s | [Details]: |\n",
        NowTF.Hour,
        NowTF.Minute,
        NowTF.Second,
        NowTF.Milliseconds,
        (PVOID)ProcessId,
        (PVOID)ThreadId,
        "Success"
    );
}


void ThreadTerminated(HANDLE ProcessId, HANDLE ThreadId)
{
    LARGE_INTEGER Now = { 0 };
    LARGE_INTEGER LocalNow = { 0 };
    TIME_FIELDS NowTF = { 0 };

    KeQuerySystemTimePrecise(&Now);
    ExSystemTimeToLocalTime(&Now, &LocalNow);
    RtlTimeToTimeFields(&LocalNow, &NowTF);

    DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "ProcMonel: [Time]: (%hu:%hu:%hu.%hu) | [Operation]: Thread Terminated | [PID]: 0x%p | [ThreadID]: 0x%p | [Result]: %s | [Details]: |\n",
        NowTF.Hour,
        NowTF.Minute,
        NowTF.Second,
        NowTF.Milliseconds,
        (PVOID)ProcessId,
        (PVOID)ThreadId,
        "Success"
    );
}


void ImageLoaded(HANDLE ProcessId, PUNICODE_STRING FullImageName, PIMAGE_INFO ImageInfo)
{
    LARGE_INTEGER Now = { 0 };
    LARGE_INTEGER LocalNow = { 0 };
    TIME_FIELDS NowTF = { 0 };

    UNREFERENCED_PARAMETER(ImageInfo);

    KeQuerySystemTimePrecise(&Now);
    ExSystemTimeToLocalTime(&Now, &LocalNow);
    RtlTimeToTimeFields(&LocalNow, &NowTF);

	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "ProcMonel: [Time]: (%hu:%hu:%hu.%hu) | [Operation]: Image Loaded | [PID]: 0x%p | [Path]: %wZ | [Result]: %s | [Details]: |\n",
		NowTF.Hour,
		NowTF.Minute,
		NowTF.Second,
		NowTF.Milliseconds,
		(PVOID)ProcessId,
		FullImageName,
        "Succes"
	);
}


/*
* Utils function
* Retrieves RegistryKey Name from (PVOID)Object
*/
void GetRegistryObjectCompleteName(PUNICODE_STRING RegistryPath, PVOID RegistryObject)
{
	NTSTATUS status = { 0 };
	ULONG returnedLength = 0;
	PUNICODE_STRING	ObjectName = NULL;

	status = ObQueryNameString(RegistryObject, (POBJECT_NAME_INFORMATION)ObjectName, 0, &returnedLength);
	if (STATUS_INFO_LENGTH_MISMATCH == status)
	{
		ObjectName = ExAllocatePoolWithTag(NonPagedPool, returnedLength, REGISTRY_POOL_TAG);
		status = ObQueryNameString(RegistryObject, (POBJECT_NAME_INFORMATION)ObjectName, returnedLength, &returnedLength);
		if (NT_SUCCESS(status))
		{
			RtlUnicodeStringCopy(RegistryPath, ObjectName);
		}
		ExFreePoolWithTag(ObjectName, REGISTRY_POOL_TAG);
	}
}


void RegistryKeyCreated(PREG_POST_OPERATION_INFORMATION CallbackData)
{
    LARGE_INTEGER Now = { 0 };
    LARGE_INTEGER LocalNow = { 0 };
    TIME_FIELDS NowTF = { 0 };
    HANDLE processId = { 0 };
	PREG_CREATE_KEY_INFORMATION_V1 PreCreateInfo = NULL;

	KeQuerySystemTimePrecise(&Now);
	ExSystemTimeToLocalTime(&Now, &LocalNow);
	RtlTimeToTimeFields(&LocalNow, &NowTF);

	processId = PsGetCurrentProcessId();
	PreCreateInfo = (PREG_CREATE_KEY_INFORMATION_V1)CallbackData->PreInformation;
    
	if (!NT_SUCCESS(CallbackData->Status))
	{
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "ProcMonel: [Time]: (%hu:%hu:%hu.%hu) | [Operation]: RegCreateKey | [PID]: 0x%p | [Path]: %wZ | [Result]: 0x%p | [Details]: |\n",
			NowTF.Hour,
			NowTF.Minute,
			NowTF.Second,
			NowTF.Milliseconds,
			(PVOID)processId,
			PreCreateInfo->CompleteName,
            (PVOID)CallbackData->Status
			);
		return;
	}

	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "ProcMonel: [Time]: (%hu:%hu:%hu.%hu) | [Operation]: RegCreateKey | [PID]: 0x%p | [Path]: %wZ | [Result]: 0x%p | [Details]: |\n",
		NowTF.Hour,
		NowTF.Minute,
		NowTF.Second,
		NowTF.Milliseconds,
		(PVOID)processId,
		PreCreateInfo->CompleteName,
        (PVOID)CallbackData->Status
	);
}


void RegistryValueKeySet(PREG_POST_OPERATION_INFORMATION CallbackData)
{
	LARGE_INTEGER Now = { 0 };
	LARGE_INTEGER LocalNow = { 0 };
	TIME_FIELDS NowTF = { 0 };
	HANDLE processId = { 0 };
	PREG_SET_VALUE_KEY_INFORMATION PreSetValueInfo = NULL;
	UNICODE_STRING registryPath = { 0 };

	KeQuerySystemTimePrecise(&Now);
	ExSystemTimeToLocalTime(&Now, &LocalNow);
	RtlTimeToTimeFields(&LocalNow, &NowTF);

	processId = PsGetCurrentProcessId();
	PreSetValueInfo = (PREG_SET_VALUE_KEY_INFORMATION)CallbackData->PreInformation;

	if (!NT_SUCCESS(CallbackData->Status))
	{
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "ProcMonel: [Time]: (%hu:%hu:%hu.%hu) | [Operation]: RegSetValue | [PID]: 0x%p | [Path]: %wZ | [Result]: 0x%p | [Details]: |\n",
			NowTF.Hour,
			NowTF.Minute,
			NowTF.Second,
			NowTF.Milliseconds,
			(PVOID)processId,
			PreSetValueInfo->ValueName,
            (PVOID)CallbackData->Status
		);
		return;
	}

	registryPath.Length = 0;
	registryPath.MaximumLength = NTSTRSAFE_UNICODE_STRING_MAX_CCH * sizeof(WCHAR);
	registryPath.Buffer = ExAllocatePoolWithTag(NonPagedPool, registryPath.MaximumLength, REGISTRY_POOL_TAG);

	GetRegistryObjectCompleteName(&registryPath, CallbackData->Object);

	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "ProcMonel: [Time]: (%hu:%hu:%hu.%hu) | [Operation]: RegSetValue | [PID]: 0x%p | [Path]: %wZ\\%wZ | [Result]: 0x%p | [Details]: |\n",
		NowTF.Hour,
		NowTF.Minute,
		NowTF.Second,
		NowTF.Milliseconds,
		(PVOID)processId,
		registryPath,
		PreSetValueInfo->ValueName,
        (PVOID)CallbackData->Status
	);

	ExFreePoolWithTag(registryPath.Buffer, REGISTRY_POOL_TAG);
}


void RegistryKeyDeleted(PREG_POST_OPERATION_INFORMATION CallbackData)
{
	LARGE_INTEGER Now = { 0 };
	LARGE_INTEGER LocalNow = { 0 };
	TIME_FIELDS NowTF = { 0 };
	HANDLE processId = { 0 };
	UNICODE_STRING registryPath = { 0 };

	KeQuerySystemTimePrecise(&Now);
	ExSystemTimeToLocalTime(&Now, &LocalNow);
	RtlTimeToTimeFields(&LocalNow, &NowTF);

	processId = PsGetCurrentProcessId();

	if (!NT_SUCCESS(CallbackData->Status))
	{
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "ProcMonel: [Time]: (%hu:%hu:%hu.%hu) | [Operation]: RegDeleteKey | [PID]: 0x%p | [Path]: %wZ | [Result]: 0x%p | [Details]: |\n",
			NowTF.Hour,
			NowTF.Minute,
			NowTF.Second,
			NowTF.Milliseconds,
			(PVOID)processId,
			L"",
            (PVOID)CallbackData->Status
		);
		return;
	}

	registryPath.Length = 0;
	registryPath.MaximumLength = NTSTRSAFE_UNICODE_STRING_MAX_CCH * sizeof(WCHAR);
	registryPath.Buffer = ExAllocatePoolWithTag(NonPagedPool, registryPath.MaximumLength, REGISTRY_POOL_TAG);

	GetRegistryObjectCompleteName(&registryPath, CallbackData->Object);

	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "ProcMonel: [Time]: (%hu:%hu:%hu.%hu) | [Operation]: RegDeleteKey | [PID]: 0x%p | [Path]: %wZ | [Result]: 0x%p | [Details]: |\n",
		NowTF.Hour,
		NowTF.Minute,
		NowTF.Second,
		NowTF.Milliseconds,
		(PVOID)processId,
		registryPath,
        (PVOID)CallbackData->Status
	);

	ExFreePoolWithTag(registryPath.Buffer, REGISTRY_POOL_TAG);
}


void RegistryValueKeyDeleted(PREG_POST_OPERATION_INFORMATION CallbackData)
{
	LARGE_INTEGER Now = { 0 };
	LARGE_INTEGER LocalNow = { 0 };
	TIME_FIELDS NowTF = { 0 };
	HANDLE processId = { 0 };
	PREG_DELETE_VALUE_KEY_INFORMATION PreDeleteValueInfo = NULL;
    UNICODE_STRING registryPath = { 0 };

	KeQuerySystemTimePrecise(&Now);
	ExSystemTimeToLocalTime(&Now, &LocalNow);
	RtlTimeToTimeFields(&LocalNow, &NowTF);

	processId = PsGetCurrentProcessId();
	PreDeleteValueInfo = (PREG_DELETE_VALUE_KEY_INFORMATION)CallbackData->PreInformation;

	if (!NT_SUCCESS(CallbackData->Status))
	{
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "ProcMonel: [Time]: (%hu:%hu:%hu.%hu) | [Operation]: RegDeleteValue | [PID]: 0x%p | [Path]: %wZ | [Result]: 0x%p | [Details]: |\n",
			NowTF.Hour,
			NowTF.Minute,
			NowTF.Second,
			NowTF.Milliseconds,
			(PVOID)processId,
			PreDeleteValueInfo->ValueName,
            (PVOID)CallbackData->Status
		);
		return;
	}

    registryPath.Length = 0;
    registryPath.MaximumLength = NTSTRSAFE_UNICODE_STRING_MAX_CCH * sizeof(WCHAR);
    registryPath.Buffer = ExAllocatePoolWithTag(NonPagedPool, registryPath.MaximumLength, REGISTRY_POOL_TAG);

    GetRegistryObjectCompleteName(&registryPath, CallbackData->Object);

	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "ProcMonel: [Time]: (%hu:%hu:%hu.%hu) | [Operation]: RegDeleteValue | [PID]: 0x%p | [Path]: %wZ\\%wZ | [Result]: 0x%p | [Details]: |\n",
		NowTF.Hour,
		NowTF.Minute,
		NowTF.Second,
		NowTF.Milliseconds,
		(PVOID)processId,
        registryPath,
		PreDeleteValueInfo->ValueName,
        (PVOID)CallbackData->Status
	);

    ExFreePoolWithTag(registryPath.Buffer, REGISTRY_POOL_TAG);
}


void FileCreated(PFILE_EVENT FileEvent)
{
    LARGE_INTEGER Now = { 0 };
    LARGE_INTEGER LocalNow = { 0 };
    TIME_FIELDS NowTF = { 0 };

    KeQuerySystemTimePrecise(&Now);
    ExSystemTimeToLocalTime(&Now, &LocalNow);
    RtlTimeToTimeFields(&LocalNow, &NowTF);

    DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "ProcMonel: [Time]: (%hu:%hu:%hu.%hu) | [Operation]: FileCreated | [PID]: 0x%p | [Path]: %wZ | [Result]: 0x%p | [Details]: |\n",
        NowTF.Hour,
        NowTF.Minute,
        NowTF.Second,
        NowTF.Milliseconds,
        (PVOID)FileEvent->processId,
        FileEvent->filePath,
        (PVOID)FileEvent->status
    );
}

void FileRead(PFILE_EVENT FileEvent)
{
    LARGE_INTEGER Now = { 0 };
    LARGE_INTEGER LocalNow = { 0 };
    TIME_FIELDS NowTF = { 0 };

    KeQuerySystemTimePrecise(&Now);
    ExSystemTimeToLocalTime(&Now, &LocalNow);
    RtlTimeToTimeFields(&LocalNow, &NowTF);

    DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "ProcMonel: [Time]: (%hu:%hu:%hu.%hu) | [Operation]: FileRead | [PID]: 0x%p | [Path]: %wZ | [Result]: 0x%p | [Details]: |\n",
        NowTF.Hour,
        NowTF.Minute,
        NowTF.Second,
        NowTF.Milliseconds,
        (PVOID)FileEvent->processId,
        FileEvent->filePath,
        (PVOID)FileEvent->status
    );
}

void FileWritten(PFILE_EVENT FileEvent)
{
    LARGE_INTEGER Now = { 0 };
    LARGE_INTEGER LocalNow = { 0 };
    TIME_FIELDS NowTF = { 0 };

    KeQuerySystemTimePrecise(&Now);
    ExSystemTimeToLocalTime(&Now, &LocalNow);
    RtlTimeToTimeFields(&LocalNow, &NowTF);

    DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "ProcMonel: [Time]: (%hu:%hu:%hu.%hu) | [Operation]: FileWrite | [PID]: 0x%p | [Path]: %wZ | [Result]: 0x%p | [Details]: |\n",
        NowTF.Hour,
        NowTF.Minute,
        NowTF.Second,
        NowTF.Milliseconds,
        (PVOID)FileEvent->processId,
        FileEvent->filePath,
        (PVOID)FileEvent->status
    );
}

void FileCleanedup(PFILE_EVENT FileEvent)
{
    LARGE_INTEGER Now = { 0 };
    LARGE_INTEGER LocalNow = { 0 };
    TIME_FIELDS NowTF = { 0 };

    KeQuerySystemTimePrecise(&Now);
    ExSystemTimeToLocalTime(&Now, &LocalNow);
    RtlTimeToTimeFields(&LocalNow, &NowTF);

    DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "ProcMonel: [Time]: (%hu:%hu:%hu.%hu) | [Operation]: FileCleanup | [PID]: 0x%p | [Path]: %wZ | [Result]: 0x%p | [Details]: |\n",
        NowTF.Hour,
        NowTF.Minute,
        NowTF.Second,
        NowTF.Milliseconds,
        (PVOID)FileEvent->processId,
        FileEvent->filePath,
        (PVOID)FileEvent->status
    );
}

void FileClosed(PFILE_EVENT FileEvent)
{
    LARGE_INTEGER Now = { 0 };
    LARGE_INTEGER LocalNow = { 0 };
    TIME_FIELDS NowTF = { 0 };

    KeQuerySystemTimePrecise(&Now);
    ExSystemTimeToLocalTime(&Now, &LocalNow);
    RtlTimeToTimeFields(&LocalNow, &NowTF);

    DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "ProcMonel: [Time]: (%hu:%hu:%hu.%hu) | [Operation]: FileClose | [PID]: 0x%p | [Path]: %wZ | [Result]: 0x%p | [Details]: |\n",
        NowTF.Hour,
        NowTF.Minute,
        NowTF.Second,
        NowTF.Milliseconds,
        (PVOID)FileEvent->processId,
        FileEvent->filePath,
        (PVOID)FileEvent->status
    );
}


void FileSetAttributes(PFILE_EVENT FileEvent)
{
    LARGE_INTEGER Now = { 0 };
    LARGE_INTEGER LocalNow = { 0 };
    TIME_FIELDS NowTF = { 0 };

    KeQuerySystemTimePrecise(&Now);
    ExSystemTimeToLocalTime(&Now, &LocalNow);
    RtlTimeToTimeFields(&LocalNow, &NowTF);

    DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "ProcMonel: [Time]: (%hu:%hu:%hu.%hu) | [Operation]: FileSetAttributes | [PID]: 0x%p | [Path]: %wZ | [Result]: 0x%p | [Details]: |\n",
        NowTF.Hour,
        NowTF.Minute,
        NowTF.Second,
        NowTF.Milliseconds,
        (PVOID)FileEvent->processId,
        FileEvent->filePath,
        (PVOID)FileEvent->status
    );
}