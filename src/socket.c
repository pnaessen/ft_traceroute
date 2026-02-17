#include "ft_traceroute.h"

int create_socket(t_traceroute *tr)
{
    struct timeval tv = {1, 0};

    tr->sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (tr->sockfd < 0) {
	perror("socket");
	return -1;
    }

    if (setsockopt(tr->sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
	perror("setsockopt");
	close(tr->sockfd);
	return -1;
    }

    return 0;
}
