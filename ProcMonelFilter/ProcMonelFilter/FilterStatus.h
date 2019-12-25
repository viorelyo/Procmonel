#ifndef _FILTER_STATUS_H_INCLUDED_
#define _FILTER_STATUS_H_INCLUDED_

//
// basic status type definition
//
#ifndef _NTDEF_ // FILTER uses statuses from <ntstatus.h> 
typedef _Return_type_success_(return >= 0) long NTSTATUS;
typedef NTSTATUS *PNTSTATUS;
#endif

typedef _Return_type_success_(return >= 0) NTSTATUS FILTERSTATUS;


//
// FILTER specific status bits and values
//
#define STATUS_CUSTOM                          0x20000000L      // define our statuses as custom nt statuses
#define STATUS_FACILITY_FILTER                 0x08040000L      // our facility is 804

#define STATUS_FILTER_SUCCESS_BITS             0x28040000L      // success status
#define STATUS_FILTER_INFORMATIONAL_BITS       0x68040000L      // informational status
#define STATUS_FILTER_WARNING_BITS             0xA8040000L      // warning status
#define STATUS_FILTER_ERROR_BITS               0xE8040000L      // error status

#define STATUS_FILTER_BSON_ERROR_BITS          0xE8048000L

//
// Status evaluation macros - Aliases for the macros from Microsoft
//

//
// Generic test for success on any status value (non-negative numbers
// indicate success).
//
#define FILTER_SUCCESS(Status)      (((FILTERSTATUS)(Status)) >= 0)        // same as NT_SUCCESS

//
// Generic test for information on any status value.
//
#define FILTER_INFORMATION(Status)  ((((unsigned long)(Status)) >> 30) == 1)   // same as NT_INFORMATION

//
// Generic test for warning on any status value.
//
#define FILTER_WARNING(Status)      ((((unsigned long)(Status)) >> 30) == 2)   // same as NT_WARNING

//
// Generic test for error on any status value.
//
#define FILTER_ERROR(Status)        ((((unsigned long)(Status)) >> 30) == 3)   // same as NT_ERROR


//
// FILTER specific status codes
//
#define STATUS_COMM_DEM0          ((FILTERSTATUS)STATUS_FILTER_BSON_ERROR_BITS|0x0001L)

#endif