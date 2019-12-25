#ifndef _FS_FILTER1_INCLUDED_
#define _FS_FILTER1_INCLUDED_

#include "FilterStatus.h"
#include <fltKernel.h>

typedef struct _GLOBAL_DATA
{
    PFLT_FILTER FilterHandle;
    struct
    {
        PFLT_PORT ServerPort;
        PFLT_PORT ClientPort;
    }Communication;
}GLOBAL_DATA, *PGLOBAL_DATA;


extern GLOBAL_DATA gDrv;


typedef struct  _FILE_EVENT {
    UCHAR majorFileEventType;
    UCHAR minorFileEventType;
    NTSTATUS status;
    ULONG_PTR information;
    ULONG flags;
    HANDLE processId;
    UNICODE_STRING filePath;
} FILE_EVENT, *PFILE_EVENT;

#endif