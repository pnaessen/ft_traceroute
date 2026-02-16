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

/*
** STRUCTURES
*/

typedef struct s_traceroute {

} t_traceroute;

/*
** PROTOTYPES
*/

#endif
