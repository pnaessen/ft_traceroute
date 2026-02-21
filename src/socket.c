#include "ft_traceroute.h"

int create_socket(t_traceroute *tr)
{
    tr->recv_sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (tr->recv_sock < 0) {
	perror("socket recv");
	return -1;
    }

    if (tr->use_icmp) {
	tr->send_sock = tr->recv_sock;
    } else {
	tr->send_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (tr->send_sock < 0) {
	    perror("socket send");
	    return -1;
	}
    }

    return 0;
}
