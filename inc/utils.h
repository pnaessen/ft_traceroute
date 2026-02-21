#ifndef UTILS_H
#define UTILS_H

int parse_args(int argc, char **argv, t_traceroute *tr);
uint16_t calculate_checksum(void *addr, int len);
double get_time_now(void);

#endif
