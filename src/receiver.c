#include "ft_traceroute.h"

static void resolve_info(t_traceroute *tr, struct sockaddr_in *addr, t_probe_result *res)
{
    if (inet_ntop(AF_INET, &addr->sin_addr, res->ip, INET_ADDRSTRLEN) == NULL) {
	strncpy(res->ip, "?", INET_ADDRSTRLEN);
    }

    if (tr->resolve_dns) {
	if (getnameinfo((struct sockaddr *)addr, sizeof(*addr), res->hostname, NI_MAXHOST, NULL, 0,
			0) != 0) {
	    strncpy(res->hostname, res->ip, NI_MAXHOST);
	}
    } else {
	strncpy(res->hostname, res->ip, NI_MAXHOST);
    }
}

void receive_packet(t_traceroute *tr, t_probe_result *res)
{
    char buffer[512];
    struct sockaddr_in from;
    socklen_t from_len = sizeof(from);
    ssize_t ret;
    struct iphdr *ip;
    struct icmphdr *icmp;

    res->got_reply = false;

    ret = recvfrom(tr->sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&from, &from_len);

    if (ret < 0) {
	return;
    }

    ip = (struct iphdr *)buffer;
    icmp = (struct icmphdr *)(buffer + (ip->ihl * 4));

    if (icmp->type == ICMP_ECHOREPLY) {
	if (ntohs(icmp->un.echo.id) == tr->pid && ntohs(icmp->un.echo.sequence) == res->code) {
	    if (ntohs(icmp->un.echo.id) == tr->pid) {
		res->got_reply = true;
		res->type = icmp->type;
		res->code = icmp->code;
	    }
	}
    } else if (icmp->type == ICMP_TIME_EXCEEDED) {
	struct iphdr *inner_ip = (struct iphdr *)(buffer + (ip->ihl * 4) + 8);
	struct icmphdr *inner_icmp = (struct icmphdr *)((char *)inner_ip + (inner_ip->ihl * 4));

	if (ntohs(inner_icmp->un.echo.id) == tr->pid) {
	    res->got_reply = true;
	    res->type = icmp->type;
	    res->code = icmp->code;
	}
    } else if (icmp->type == ICMP_DEST_UNREACH) {
	struct iphdr *inner_ip = (struct iphdr *)(buffer + (ip->ihl * 4) + 8);
	struct icmphdr *inner_icmp = (struct icmphdr *)((char *)inner_ip + (inner_ip->ihl * 4));

	if (ntohs(inner_icmp->un.echo.id) == tr->pid) {
	    res->got_reply = true;
	    res->type = icmp->type;
	    res->code = icmp->code;
	}
    }

    if (res->got_reply) {
	resolve_info(tr, &from, res);
    }
}
