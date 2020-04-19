#include "argtable3/argtable3.h"
#include "lwip/sockets.h"
#include "lwip/inet.h"
#include "lwip/netdb.h"
#include "lwip/sockets.h"
#include "esp_console.h"

#include "cmd_host.h"
#include "util.h"
#include "net_util.h"

static struct {
    struct arg_str *host;
    struct arg_end *end;
} host_cmd_args;

static int do_host_lookup(char* hostname, int ai_family) {
    struct addrinfo hints = { 0 }, *results, *result = NULL;
    hints.ai_family = ai_family;

    int m = getaddrinfo(hostname, NULL, &hints, &results);
    if (m != 0) {
        freeaddrinfo(results);
        return m;
    }

    for (result = results; result != NULL; result = result->ai_next) {
        char straddr[INET6_ADDRSTRLEN] = {};
        if (inet_ntop(result->ai_family, get_sin_addr(result->ai_family, result->ai_addr), straddr, sizeof(straddr))) {
            printf("Host %s has IP address %s\n", host_cmd_args.host->sval[0], straddr);
        } else {
            printf("inet_ntop error: %d\n", errno);
            freeaddrinfo(results);
            return errno;
        }
    }

    freeaddrinfo(results);
    return 0;
}

static int do_host_cmd(int argc, char**argv) {
    int nerrors = arg_parse(argc, argv, (void **)&host_cmd_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, host_cmd_args.end, argv[0]);
        return 1;
    }

    int ret_v6, ret_v4;
    ret_v6 = do_host_lookup(host_cmd_args.host->sval[0], AF_INET6);
    ret_v4 = do_host_lookup(host_cmd_args.host->sval[0], AF_INET);

    if (ret_v6 || ret_v4 == 0) {
        return 0;
    }
    return 1;
}

esp_err_t register_host_cmd(void) {
    host_cmd_args.host = arg_str1(NULL, NULL, "<host>", "Host to look up");
    host_cmd_args.end = arg_end(1);
    const esp_console_cmd_t host_cmd = {
        .command = "host",
        .help = "Perform a DNS lookup",
        .func = &do_host_cmd,
        .argtable = &host_cmd_args,
    };

    return esp_console_cmd_register(&host_cmd);
}
