#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "snapmb.h"

#define regs_amount 16
#define Port_1 5021
#define Port_2 5022
#define AllZero 0
#define AllOne 0xFFFFFFFFFFFFFFFF

int Port;
char Address[16] = "127.0.0.1"; 
uint64_t Regs = 0;  // Simulate 4 * 16bit registers

PSnapMBBroker Client;


void SysSleep(longword Delay_ms)
{
#ifdef SNAP_OS_WINDOWS
    Sleep(Delay_ms);
#else
    struct timespec ts;
    ts.tv_sec = (time_t)(Delay_ms / 1000);
    ts.tv_nsec = (long)((Delay_ms - ts.tv_sec) * 1000000);
    nanosleep(&ts, (struct timespec*)0);
#endif
}


int ParseCmdLine(int argc, char* argv[])
{
    if (argc > 1)
    {
        if (strcmp(argv[1], "1") == 0)
            return Port_1;
        if (strcmp(argv[1], "2") == 0)
            return Port_2;
    }
    return 0;
}


int main(int argc, char* argv[])
{
    bool toggle = false;
    int Cnt = 0;
    int Result = 0;
    int LastResult = 0;
    char Text[512];
    int Port = ParseCmdLine(argc, argv);
    if (Port == 0)
    {
        printf("Missing Port Number, should be 1 (%d) or 2 (%d)\n", Port_1, Port_2);
        return 1;
    }
    Client = new TSnapMBBroker(ProtoTCP, Address, Port);

    if (Client->Connect() != 0)
    {
        printf("Cound not connect to %s:%d\n", Address, Port);
        return 1;
    }

    printf("Connected to %s:%d\n", Address, Port);
    
    time_t my_time = time(NULL);

    while (true)
    {
        Regs = toggle ? AllOne : AllZero;
        toggle = !toggle;
        Result = Client->WriteMultipleRegisters(1, 1, 4, &Regs);

        if (Result != LastResult)
        {
            printf("\n");
            LastResult = Result;
        }

        if (Result == 0)
            printf("%d - OK\r", Cnt);
        else
            printf("%d - %s\r", Cnt, ErrorText(Result, Text, 511));
        Cnt++;
        SysSleep(23); // a prime number
    }

    delete Client;
    return 0;
}
