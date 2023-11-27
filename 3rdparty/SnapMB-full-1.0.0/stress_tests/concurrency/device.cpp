#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <inttypes.h>
#include "snapmb.h"


#define regs_amount 16
#define Port_1 5021
#define Port_2 5022
#define AllZero 0
#define AllOne 0xFFFFFFFFFFFFFFFF

#define CopyModeRead  0
#define CopyModeWrite 1

PSnapMBDevice Device_1;
PSnapMBDevice Device_2;

uint64_t Regs = 0;  // Simulate 4 * 16bit registers
uint64_t Copy = 0;  // Simulate 4 * 16bit registers

char Address[16] = "127.0.0.1"; 

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

char* Now(char* buf)
{
    time_t my_time = time(NULL);
    sprintf(buf, ctime(&my_time));
    buf[strlen(buf) - 1] = 0;
    return buf;
}


int main(int argc, char* argv[])
{
    int Result;
    int Cnt = 0;
    char tb[80];

    Device_1 = new TSnapMBDevice(ProtoTCP, 1, Address, Port_1);
    Device_2 = new TSnapMBDevice(ProtoTCP, 2, Address, Port_2);

    // to create the shared lock it is sufficient to register the area only in one device
    Device_1->RegisterArea(mbAreaHoldingRegisters, &Regs, sizeof(Regs));

    Result = Device_1->Start();
    if (Result == 0)
    {
        printf("Device 1 started succesfully @%s:%d\n", Address, Port_1);
        Result = Device_2->Start();
        if (Result == 0)
            printf("Device 2 started succesfully @%s:%d\n", Address, Port_2);
        else
            printf("Device_2 failed to start\n");
    }
    else
        printf("Device_1 failed to start\n");

    if (Result != 0)
    {
        delete Device_1;
        delete Device_2;
        return 1;
    }
    printf("Press ctrl-c to break\n");
    printf("Started in : %s\n", Now(tb));

    // The concept is:
    // Two external clients will asynchronously write 4 registers, 
    // one time all filled with 0xFFFF, another with 0x0000.
    // Here, two devices will read and write these registers.
    // The check is that we will **never** read a value different from
    // 0x0000000000000000 or 0xFFFFFFFFFFFFFFFF

    while (true)
    {
        Copy = Regs;
        //Device_1->CopyArea(mbAreaHoldingRegisters, 1, 4, &Copy, CopyModeRead);

        if (Copy != AllZero && Copy != AllOne)
        {
            printf("Error : %I64x" PRIx64 "Found !!!\n", Copy);
        }
        else 
            printf("%10d - %s\r", Cnt++, Now(tb));

        SysSleep(23);
    }

    delete Device_1;
    delete Device_2;
}

