#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "snapmb.h"

/*

           request     +------------+             +------------+
      ---------------->+            +------------>+            |    request relayed to serial line
           ETHERNET    |   Device   |   relay     |   Client   +<----------------+--------------------+-------....
      <----------------+            +<------------+            |                 |     RS232/485      |
                       +------------+             +------------+                 V                    V
                                                                        +--------+--------+  +--------+--------+
                                                                        | Serial Device 1 |  | Serial Device 2 |
                                                                        +-----------------+  +--------+--------+
*/
PSnapMBDevice DeviceHost;
PSnapMBBroker InnerClient;


char Text[256] = "";

void SNAP_API EventCallBack(void* usrPtr, void* Event, int Size)
{
    // print the event
    printf("%s\n", EventText(Event, Text, 255));
}

// here's where the magic happens
int SNAP_API PasstroughHandler(void* usrPtr, byte DeviceID, void* RxPDU, word RxPDUSize, void* TxPDU, word& TxPDUSize)
{
    // Relay the request through the client
    int rawResult = InnerClient->RawRequest(DeviceID, RxPDU, RxPDUSize, TxPDU, TxPDUSize, 0);

    // Converts Timeout and Hardware errors to be fully Modbus compliant
    if (rawResult != mbNoError)
    {
        // Timeout
        if ((rawResult & 0x0FFFFFFF) == 0x00050007)
            return errGatewayTargetFailed; // Target failed to respond
        // Other error (not Modbus error)
        if (TxPDUSize == 0)
            return  errSlaveDeviceFailure;
    }
    return 0;
}

int main(int argc, char* argv[])
{
    // The aim of this example is just to show how is simple to create a gateway so, 
    // to avoid distractions, here you will not find the command line parameters parsing

    // Device creation (server), here is a TCP Device, but you can create the type you need (UDP, RTU..)
    DeviceHost = new TSnapMBDevice(ProtoTCP, 1, "127.0.0.1", 502);
    // Inner Client creation, here is a serial client, here too you can create the type you need
    InnerClient = new TSnapMBBroker(FormatRTU, "COM5", 115200, 'E', 8, 1, 0);

    // From this point, and for the rest of the program there is no difference in 
    // the use of the device and the client, regardless of their type (Serial or Ethernet)

    // Set Passthrough mode
    DeviceHost->SetParam(par_DevicePassthrough, 1);
    DeviceHost->RegisterCallback(cbkPassthrough, PasstroughHandler, NULL);
    // Set events callback to see what's happening
    DeviceHost->RegisterCallback(cbkDeviceEvent, EventCallBack, NULL);

    // Start
    int Result = DeviceHost->Start();
    if (Result == 0)
    {
        if (InnerClient->Connect() != 0)
            printf("Warning, client connection failure\n");
        else
            printf("the system is fully operational\n");

        printf("enter a char to terminate\n");
        // Now the Device is running ... press a key (and Enter) to terminate
        getchar();
    }
    else
        printf("%s\n", ErrorText(Result, Text, 255));

    delete DeviceHost;
    delete InnerClient;
}
