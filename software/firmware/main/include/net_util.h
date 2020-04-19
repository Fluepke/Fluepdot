#include "lwip/inet.h"
#include "lwip/sockets.h"

void* get_sin_addr(int af, struct sockaddr *addr);
void printf_sockaddr(int af, struct sockaddr *addr);
