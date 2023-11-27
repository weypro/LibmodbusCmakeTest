#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <inttypes.h>
#include "snapmb.h"

#define regs_amount 32768
#define bits_amount 65536
#define Port 5020

PSnapMBDevice Device;

word InputRegisters[regs_amount];
word HoldingRegisters[regs_amount];
byte Coils[bits_amount];
byte DiscreteInputs[bits_amount];

char Address[16] = "127.0.0.1"; 
char Text[256] = "";
int cnt = 0;
int err_cnt = 0;

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

char* TimeStr(char* buf, time_t the_time)
{
    sprintf(buf, ctime(&the_time));
    buf[strlen(buf) - 1] = 0;
    return buf;
}

void SNAP_API EventCallBack(void* usrPtr, void* Event, int Size)
{
    if (PSrvEvent(Event)->EvtRetCode != 0 || PSrvEvent(Event)->EvtCode == 0x00004000)
        err_cnt++;

    printf("%d Events triggered, %d Error handled\r", cnt++, err_cnt);
}

int main(int argc, char* argv[])
{
    int Result = 0;
    int Cnt = 0;
    char tb[80];
    time_t time_start;

    if (argc > 1)
        strcpy(Address, argv[1]);

    Device = new TSnapMBDevice(ProtoTCP, 1, Address, Port);

    // to create the shared lock it is sufficient to register the area only in one device
    Device->RegisterArea(mbAreaHoldingRegisters, &HoldingRegisters, regs_amount);
    Device->RegisterCallback(cbkDeviceEvent, (void*)EventCallBack, NULL);
    printf("Device created, press any key to terminate\n");

    Result = Device->Start();

    if (Result == 0)
    {
        time_start = time(NULL);
        getchar();
        Device->Stop();
        printf("Started in : %s\n", TimeStr(tb, time_start));
        printf("Ended in   : %s\n", TimeStr(tb, time(NULL)));
        SysSleep(1000);
    }
    else
        printf("Device failed to start\n");

    delete Device;
}

