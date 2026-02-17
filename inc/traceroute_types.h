#ifndef TRACEROUTE_TYPES_H
#define TRACEROUTE_TYPES_H

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
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#define DEF_MAX_HOPS 30
#define DEF_PROBES_PER_HOP 3
#define DEF_PORT 33434

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

#define ERR_DNS -1
#define ERR_SOCKET -2
#define ERR_ARGS -3

typedef struct s_probe_result {
    double rtt;
    char ip[INET_ADDRSTRLEN];
    char hostname[NI_MAXHOST];
    int type;
    int code;
    bool got_reply;
    bool is_final;
} t_probe_result;

typedef struct s_traceroute {
    char *target;
    int max_hops;
    int probes_per_hop;
    bool resolve_dns;

    struct sockaddr_in dest_addr;
    int sockfd;
    uint16_t pid;
    uint16_t port_base;
} t_traceroute;

#endif
