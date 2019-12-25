#ifndef _COMMSHARED_H_INCLUDED_
#define _COMMSHARED_H_INCLUDED_


#define MY_FILTER_PORT_NAME L"\\MY_DRIVERCommPort"

#define COMM_MAX_MESSAGE_SIZE  4*1024 // 4Kb = 1 page

typedef enum _MY_MESSAGE_ID
{
    idProcessCreate = 0,
    idProcessTerminate = 1
}MY_MESSAGE_ID;

typedef struct _MY_MSG_HEADER
{
    MY_MESSAGE_ID MessageId;
}MY_MSG_HEADER, *PMY_MSG_HEADER;

/*
typedef struct _MY_FIRST_MESSAGE
{
MY_MSG_HEADER Header;
// /// // // //
//
//
//
//  BODY for first
//
//
};

typedef struct _MY_SECOND_MESSAGE
{
MY_MSG_HEADER Header;
// /// // // //
//
//
// BODY for second
//
//
//
};

*/

#endif