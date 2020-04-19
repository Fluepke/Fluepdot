#include "net_util.h"

void* get_sin_addr(int af, struct sockaddr *addr)
{
    switch (af)
    {
        case AF_INET:
            return &(((struct sockaddr_in*)addr)->sin_addr);

        case AF_INET6:
            return &(((struct sockaddr_in6*)addr)->sin6_addr);
    }
    return NULL;
}

void printf_sockaddr(int af, struct sockaddr *addr) {
    char addr_str[INET6_ADDRSTRLEN] = {};

    if (inet_ntop(af, get_sin_addr(af, addr), addr_str, sizeof(addr_str))) {
        printf("%s", addr_str);
    } else {
        printf("<inet_ntop_error>");
    }
}
