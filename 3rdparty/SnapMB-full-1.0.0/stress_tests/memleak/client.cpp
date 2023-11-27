#include "client.h"

#define regs_amount 32768

int Port = 5020;
char Address[16] = "127.0.0.1"; 

PConnThread Thread_1;
PDataThread Thread_2;
word HoldingRegisters[regs_amount];


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


int main(int argc, char* argv[])
{

    if (argc > 1)
        strcpy(Address, argv[1]);

    Thread_1 = new TConnThread(Address, Port);
    Thread_2 = new TDataThread(Address, Port);

    Thread_1->Start();
    SysSleep(11);
    Thread_2->Start();

    getchar();

    Thread_1->Terminate();
    Thread_2->Terminate();

    if (Thread_1->WaitFor(3000) != WAIT_OBJECT_0)
        Thread_1->Kill();
    if (Thread_2->WaitFor(3000) != WAIT_OBJECT_0)
        Thread_2->Kill();

    return 0;
}
//------------------------------------------------------------------------------
TConnThread::TConnThread(const char* Address, int Port)
{
    strncpy(FAddress, Address, 16);
    Client = new TSnapMBBroker(ProtoTCP, Address, Port);
    FreeOnTerminate = true;
}
//------------------------------------------------------------------------------
void TConnThread::Execute()
{
    int cnt = 0;
    while (!Terminated)
    {
        Client->Connect();
        SysSleep(100);
        Client->Disconnect();
        if (!Terminated)
            SysSleep(100);
    }
    printf("closing\n");
    delete Client;
}
//------------------------------------------------------------------------------
TDataThread::TDataThread(const char* Address, int Port)
{
    strncpy(FAddress, Address, 16);
    Client = new TSnapMBBroker(ProtoTCP, Address, Port);
    FreeOnTerminate = true;
}
//------------------------------------------------------------------------------
void TDataThread::Execute()
{
    int cnt = 0;
    word szRead = 0;
    while (!Terminated)
    {
        Client->ReadHoldingRegisters(1, 1, 64, &HoldingRegisters);
        if ((cnt++ % 20) == 0)
        {
            HoldingRegisters[0] = 0x0205;
            Client->RawRequest(1, &HoldingRegisters, 200, &HoldingRegisters, szRead);
        }

        if (!Terminated)
            SysSleep(47);
    }
    printf("closing\n");
    delete Client;
}
