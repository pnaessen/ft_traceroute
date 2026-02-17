#ifndef CORE_H
#define CORE_H

#include "traceroute_types.h"

int parse_args(int argc, char **argv, t_traceroute *tr);
int resolve_dns(const char *host, struct sockaddr_in *dest);
int create_socket(t_traceroute *tr);
void run_traceroute(t_traceroute *tr);
void send_icmp_packet(t_traceroute *tr, int seq);
void receive_packet(t_traceroute *tr, t_probe_result *res, int expected_seq);

#endif
