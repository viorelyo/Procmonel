#pragma once
#include <fltKernel.h>
#include "GlobalData.h"

NTSTATUS ProcMonFltInitialize(PDRIVER_OBJECT DriverObject);
NTSTATUS ProcMonFltUninitialize();
void FileCallback(PFILE_EVENT FileEvent);