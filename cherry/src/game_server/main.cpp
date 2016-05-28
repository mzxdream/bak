#include <net/net_manager.h>
#include <iostream>
#include <cstring>

int main(int argc, char *argv[])
{
    NetManager net;
    if (!net.Init(4))
    {
        return 0;
    }
    if (!net.AddListener("127.0.0.1", 3232))
    {
        return 0;
    }
    char tmp[1024];
    while (std::cin.getline(tmp, 1024))
    {
        net.WriteAll(tmp, strlen(tmp));
    }
    return 0;
}
