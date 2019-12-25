#pragma once
#include <fltKernel.h>
#include "GlobalData.h"


void ProcessCreated(HANDLE ProcessId, PPS_CREATE_NOTIFY_INFO CreateInfo);
void ProcessTerminated(HANDLE ProcessId);
void ThreadCreated(HANDLE ProcessId, HANDLE ThreadId);
void ThreadTerminated(HANDLE ProcessId, HANDLE ThreadId);
void ImageLoaded(HANDLE ProcessId, PUNICODE_STRING FullImageName, PIMAGE_INFO ImageInfo);
void RegistryKeyCreated(PREG_POST_OPERATION_INFORMATION CallbackData);
void RegistryValueKeySet(PREG_POST_OPERATION_INFORMATION CallbackData);
void RegistryKeyDeleted(PREG_POST_OPERATION_INFORMATION CallbackData);
void RegistryValueKeyDeleted(PREG_POST_OPERATION_INFORMATION CallbackData);
void FileClosed(PFILE_EVENT FileEvent);
void FileCleanedup(PFILE_EVENT FileEvent);
void FileWritten(PFILE_EVENT FileEvent);
void FileRead(PFILE_EVENT FileEvent);
void FileCreated(PFILE_EVENT FileEvent);
void FileSetAttributes(PFILE_EVENT FileEvent);
