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

void receive_packet(t_traceroute *tr, t_probe_result *res, int expected_seq, double start_time)
{
    char buffer[512];
    struct sockaddr_in from;
    socklen_t from_len = sizeof(from);
    ssize_t ret;
    struct iphdr *ip;
    struct icmphdr *icmp;

    fd_set readfds;

    struct timeval tv;

    double timeout_sec = 1.0;
    res->got_reply = false;

    while (true) {
	double elapsed = get_time_now() - start_time;
	if (elapsed >= timeout_sec) {
	    return;
	}

	double remaining = timeout_sec - elapsed;
	tv.tv_sec = (time_t)remaining;
	tv.tv_usec = (suseconds_t)((remaining - tv.tv_sec) * 1000000.0);

	FD_ZERO(&readfds);
	FD_SET(tr->recv_sock, &readfds);

	int ready = select(tr->recv_sock + 1, &readfds, NULL, NULL, &tv);

	if (ready < 0) {
	    if (errno == EINTR)
		continue;
	    perror("select");
	    return;
	} else if (ready == 0) {
	    return;
	}

	ret =
	    recvfrom(tr->recv_sock, buffer, sizeof(buffer), 0, (struct sockaddr *)&from, &from_len);

	if (ret < 0)
	    return;

	ip = (struct iphdr *)buffer;
	icmp = (struct icmphdr *)(buffer + (ip->ihl * 4));

	if (icmp->type == ICMP_ECHOREPLY) {
	    if (ntohs(icmp->un.echo.id) == tr->pid &&
		ntohs(icmp->un.echo.sequence) == expected_seq) {
		res->got_reply = true;
		res->type = icmp->type;
		res->code = icmp->code;
		resolve_info(tr, &from, res);
		return;
	    }
	} else if (icmp->type == ICMP_TIME_EXCEEDED || icmp->type == ICMP_DEST_UNREACH) {
	    struct iphdr *inner_ip = (struct iphdr *)(buffer + (ip->ihl * 4) + 8);
	    int inner_ip_hlen = inner_ip->ihl * 4;

	    if (tr->use_icmp && inner_ip->protocol == IPPROTO_ICMP) {
		struct icmphdr *inner_icmp = (struct icmphdr *)((char *)inner_ip + inner_ip_hlen);

		if (ntohs(inner_icmp->un.echo.id) == tr->pid &&
		    ntohs(inner_icmp->un.echo.sequence) == expected_seq) {

		    res->got_reply = true;
		    res->type = icmp->type;
		    res->code = icmp->code;
		    resolve_info(tr, &from, res);
		    return;
		}
	    } else if (!tr->use_icmp && inner_ip->protocol == IPPROTO_UDP) {
		uint16_t *inner_udp_ports = (uint16_t *)((char *)inner_ip + inner_ip_hlen);
		uint16_t inner_dst_port = ntohs(inner_udp_ports[1]);

		if (inner_dst_port == (tr->port_base + expected_seq)) {
		    res->got_reply = true;
		    res->type = icmp->type;
		    res->code = icmp->code;
		    resolve_info(tr, &from, res);
		    return;
		}
	    }
	}
    }
}
