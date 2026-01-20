
#define RL_BASE_NO_ENTRYPOINT 1
#include "base/base.h"
#include "base/base.c"

#include "sys/socket.h"
#include <netinet/in.h>

int main(int ArgsCount, char **Args)
{
    s32 ServerHandle;
    
    struct sockaddr_in Address = {0};
    Address.sin_family = AF_INET;
    Address.sin_port = htons(2600);
    
    ServerHandle = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    AssertErrno(ServerHandle != -1);
    
    s32 Error = connect(ServerHandle, (struct sockaddr *)&Address, sizeof(Address));
    AssertErrno(Error == 0);
    
    return 0;
}