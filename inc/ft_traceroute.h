#define _GNU_SOURCE

#ifndef FT_TRACEROUTE_H
#define FT_TRACEROUTE_H
#include <arpa/inet.h>
#include <errno.h>
#include <float.h>
#include <limits.h>
#include <netdb.h>
#include <netinet/ip_icmp.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1
#define ERR_DNS -1
#define ERR_SOCKET -2

typedef struct s_probe {
    struct sockaddr_in addr;
    double             rtt;
    int                type;
    int                got_reply;
} t_probe;

typedef struct s_traceroute {
    int                sockfd;
    struct sockaddr_in dest_addr;
    char               *hostname;
    int                ttl;
    int                max_hops;
    int                probes_per_hop;
    uint16_t           pid;
} t_traceroute;

int resolve_dns(const char *host, struct sockaddr_in *dest);
int setup_socket(t_traceroute *tr);

#endif
