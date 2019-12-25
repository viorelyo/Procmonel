#include <stdio.h>
#include <Windows.h>
#include "ComDriver.h"


int main(
    int argc,
    char * argv[]
)
{
    char buffer[1000];

    if (!InitCommunication())
    {
        printf("Unable to connect to driver! Is it running? Are you admin?\n");
        return 0;
    }

    BOOLEAN terminate = FALSE;
    while (!terminate)
    {
        RtlZeroMemory(buffer, sizeof(buffer));

        printf("Please insert a command : \n");
        scanf_s("%999s", buffer, (unsigned)_countof(buffer));

        printf("[Command] : %s\n", buffer);

        if (0 == strcmp("exit", buffer))
        {
            terminate = TRUE;
            continue;
        }

        printf("Unrecognized command!\n");
    }

    UninitCommunication();
}