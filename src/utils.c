#include "ft_traceroute.h"

uint16_t calculate_checksum(void *addr, int len)
{
    uint32_t sum = 0;
    uint16_t *ptr = addr;

    while (len > 1) {
	sum += *ptr++;
	len -= 2;
    }

    if (len == 1) {
	sum += *(uint8_t *)ptr;
    }

    while (sum >> 16) {
	sum = (sum & 0xffff) + (sum >> 16);
    }

    return (uint16_t)~sum;
}

double get_time_now(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + (tv.tv_usec / 1000000.0);
}
    