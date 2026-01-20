
#define RL_BASE_NO_ENTRYPOINT 1
#include "base/base.h"
#include "base/base.c"

#include "sys/socket.h"
#include <netinet/in.h>


int main(int ArgsCount, char **Args)
{
    Log("Hello, world!\n");
    
    int MaxConnections = 64;
    u16 Port = 2600;
    
    s32 ServerHandle;
    // Start listening on the socket
    {
        s32 Result;
        ServerHandle = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        Assert(ServerHandle > 2);
        
        {
            u32 On = 1;
            Result = setsockopt(ServerHandle, SOL_SOCKET, SO_REUSEADDR, (u8*)&On, sizeof(On));
            Assert(!Result);
        }
        
        struct sockaddr_in Address = {0};
        Address.sin_family = AF_INET;
        Address.sin_port = htons(Port);
        
        Result = bind(ServerHandle, (const struct sockaddr *)&Address, sizeof(Address));
        Assert(!Result);
        
        Result = listen(ServerHandle, MaxConnections);
        Assert(!Result);
        Log("Listening on :%u\n", Port);
    }
    
    struct sockaddr_in ClientAddress;
    socklen_t ClientAddressLength;
    s32 ClientHandle;
    ClientHandle = accept(ServerHandle, (struct sockaddr *)&ClientAddress, &ClientAddressLength);
    
    u16 ClientPort = ntohs(ClientAddress.sin_port);
    
    Trap();
    
    return 0;
}