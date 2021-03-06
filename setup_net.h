#include "udp_ip.h"

int setupNet(void * defconptr)
{
    int client_connected;
    struct udp_pkg * tmp;
    char msg[16];

    tmp = malloc(sizeof(udp_pkg) + 128 /*slack*/);

    client_connected =  -1;

    while (client_connected < 0)
    {
        memset(tmp, 0, sizeof(udp_pkg));
        debugPrint("waiting for Ps2PaD++CLT-PING");
        udp_recvfrom(tmp, msg, 861);

        client_connected = !strncmp(msg, "Ps2PaD++CLT-PING", 16);
    }
    debugPrint("i've got client_ping");
    memcpy(defconptr, tmp, 42);
    return 1;
}
