#ifndef client_h
#define client_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "snap_threads.h"
#include "snapmb.h"

class TConnThread : public TSnapThread
{
private:
    char FAddress[16];
    PSnapMBBroker Client;
public:
    TConnThread(const char* Address, int Port);
    void Execute();
};
typedef TConnThread* PConnThread;


class TDataThread : public TSnapThread
{
private:
    char FAddress[16];
    PSnapMBBroker Client;
public:
    TDataThread(const char* Address, int Port);
    void Execute();
};
typedef TDataThread* PDataThread;


#endif
