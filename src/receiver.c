#include "ft_traceroute.h"

static void resolve_reverse_dns(struct sockaddr_in *addr, char *hostname)
{
    if (getnameinfo((struct sockaddr *)addr, sizeof(*addr), hostname, NI_MAXHOST, NULL, 0,
		    NI_NUMERICHOST) != 0)
	strncpy(hostname, inet_ntoa(addr->sin_addr), NI_MAXHOST);
}

t_response receive_packet(t_traceroute *tr)
{
    t_response res;
    char buffer[512];
    struct sockaddr_in from;
    socklen_t from_len = sizeof(from);

    memset(&res, 0, sizeof(res));

    ssize_t ret =
	recvfrom(tr->sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&from, &from_len);
    if (ret < 0)
	return res;

    struct iphdr *ip = (struct iphdr *)buffer;
    struct icmphdr *icmp = (struct icmphdr *)(buffer + (ip->ihl * 4));

    res.type = icmp->type;
    res.code = icmp->code;
    res.got_reply = 0;

    if (res.type == ICMP_ECHOREPLY) {
	if (ntohs(icmp->un.echo.id) == tr->pid) {
	    res.got_reply = 1;
	    res.seq = ntohs(icmp->un.echo.sequence);
	}
    } else if (res.type == ICMP_TIME_EXCEEDED) {
	struct icmphdr *inner_icmp =
	    (struct icmphdr *)(buffer + (ip->ihl * 4) + 8 + sizeof(struct iphdr));

	if (ntohs(inner_icmp->un.echo.id) == tr->pid) {
	    res.got_reply = 1;
	    res.seq = ntohs(inner_icmp->un.echo.sequence);
	}
    }

    if (res.got_reply) {
	strncpy(res.ip_str, inet_ntoa(from.sin_addr), INET_ADDRSTRLEN);
	resolve_reverse_dns(&from, res.hostname);
    }

    return res;
}
