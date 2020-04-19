#include "argtable3/argtable3.h"
#include "esp_console.h"
#include "lwip/sockets.h"
#include "lwip/inet.h"
#include "lwip/netdb.h"
#include "lwip/sockets.h"
#include "lwip/prot/icmp.h"
#include "lwip/prot/ip.h"

#include "cmd_traceroute.h"
#include "net_util.h"
#include "util.h"

static struct {
//    struct arg_dbl *timeout;
//    struct arg_int *max_hops;
    struct arg_str *host;
//    struct arg_int *data_size;
    struct arg_end *end;
} traceroute_args;

static int do_traceroute(int argc, char **argv) {
    int nerrors = arg_parse(argc, argv, (void **)&traceroute_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, traceroute_args.end, argv[0]);
        return 1;
    }

    const char *host = traceroute_args.host->sval[0];
    size_t data_size = 23;

    struct addrinfo hints = {0}, *result = NULL;
    hints.ai_family = AF_INET; // TODO v4 / v6 switch

    int m = getaddrinfo(host, NULL, &hints, &result);
    if (m != 0) {
        printf("Could not getaddrinfo() for host %s", host);
        freeaddrinfo(result);
        return m;
    }

    if (result->ai_next != NULL) {
        printf("Warn: Host %s has multiple IP addresses!", host);
    }

    size_t icmp_pkt_size = sizeof(struct icmp_echo_hdr) + data_size;
    struct icmp_echo_hdr *packet_hdr = mem_calloc(1, icmp_pkt_size);
    packet_hdr->type = ICMP_ECHO;
    packet_hdr->code = 0;
    packet_hdr->id = 0x2342;

    // fill the additional data buffer with some data
    char *data = (char*)(packet_hdr) + sizeof(struct icmp_echo_hdr);
    for (unsigned int i=0; i<data_size; i++) {
        data[i] = 'A';
    }

    int sock = socket(result->ai_family, SOCK_RAW, IP_PROTO_ICMP);

    // set socket timeout
    struct timeval timeout;
    timeout.tv_sec = 2;
    timeout.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    // set TTL
    for (int ttl = 1; ttl < 32; ttl++) {
        setsockopt(sock, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl));
        printf("%3d  ", ttl);

        for (int count=0; count<3; count++) {
            struct timeval start_time, end_time;
            gettimeofday(&start_time, NULL);
            int sent = sendto(sock, packet_hdr, icmp_pkt_size, 0, result->ai_addr, result->ai_addrlen);
            if (sent <= 0) {
                printf(" * ");
                continue;
            }

            char buf[64];
            int len = 0;
            struct sockaddr_storage from;
            socklen_t from_len = sizeof(from);

            if ((len = recvfrom(sock, buf, sizeof(buf), 0, (struct sockaddr *)&from, &from_len)) > 0) {
                gettimeofday(&end_time, NULL);

                if (count == 0) {
                    printf_sockaddr(from.ss_family, (struct sockaddr *)&from);
                    printf(" ");
                }
                printf(" %d ms ", TIME_DIFF_MS(end_time, start_time));
            } else {
                printf(" * ");
            }
        }
        printf("\n");
    }

    return 0;
}

esp_err_t register_traceroute_cmd(void) {
    traceroute_args.host = arg_str1(NULL, NULL, "<host>", "Host to traceroute");
    traceroute_args.end = arg_end(1);
    const esp_console_cmd_t traceroute_cmd = {
        .command = "traceroute",
        .help = "Perform a traceroute",
        .func = &do_traceroute,
        .argtable = &traceroute_args,
    };

    return esp_console_cmd_register(&traceroute_cmd);
}
