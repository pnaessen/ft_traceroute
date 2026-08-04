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

bool receive_single_packet(t_traceroute *tr, int target_seq, double timeout_sec, t_probe_result *res)
{
    char buffer[512];
    struct sockaddr_in from;
    socklen_t from_len;
    struct timeval tv;
    double start_time = get_time_now();

    memset(res, 0, sizeof(t_probe_result));

    while (true) {
	double elapsed = get_time_now() - start_time;
	if (elapsed >= timeout_sec) {
	    return false;
	}

	double remaining = timeout_sec - elapsed;
	tv.tv_sec = (time_t)remaining;
	tv.tv_usec = (suseconds_t)((remaining - tv.tv_sec) * 1000000.0);

	fd_set readfds;
	FD_ZERO(&readfds);
	FD_SET(tr->recv_sock, &readfds);

	int ready = select(tr->recv_sock + 1, &readfds, NULL, NULL, &tv);
	if (ready <= 0) {
	    if (ready < 0 && errno == EINTR)
		continue;
	    return false;
	}

	from_len = sizeof(from);
	ssize_t ret = recvfrom(tr->recv_sock, buffer, sizeof(buffer), 0, (struct sockaddr *)&from, &from_len);
	if (ret < 0) {
	    if (errno == EINTR)
		continue;
	    return false;
	}

	if (ret < (ssize_t)sizeof(struct iphdr))
	    continue;

	struct iphdr *ip = (struct iphdr *)buffer;
	int ip_hlen = ip->ihl * 4;

	if (ret < ip_hlen + (ssize_t)sizeof(struct icmphdr))
	    continue;

	struct icmphdr *icmp = (struct icmphdr *)(buffer + ip_hlen);
	int type = icmp->type;
	int code = icmp->code;
	int recv_seq = -1;
	bool is_final = false;

	if (type == ICMP_ECHOREPLY) {
	    if (ntohs(icmp->un.echo.id) == tr->pid) {
		recv_seq = ntohs(icmp->un.echo.sequence);
		is_final = true;
	    }
	} else if (type == ICMP_TIME_EXCEEDED || type == ICMP_DEST_UNREACH) {
	    if (ret >= ip_hlen + 8 + (ssize_t)sizeof(struct iphdr)) {
		struct iphdr *inner_ip = (struct iphdr *)(buffer + ip_hlen + 8);
		int inner_ip_hlen = inner_ip->ihl * 4;

		if (inner_ip->daddr == tr->dest_addr.sin_addr.s_addr) {
		    if (tr->use_icmp && inner_ip->protocol == IPPROTO_ICMP) {
			if (ret >= ip_hlen + 8 + inner_ip_hlen + (ssize_t)sizeof(struct icmphdr)) {
			    struct icmphdr *inner_icmp = (struct icmphdr *)((char *)inner_ip + inner_ip_hlen);
			    if (ntohs(inner_icmp->un.echo.id) == tr->pid) {
				recv_seq = ntohs(inner_icmp->un.echo.sequence);
			    }
			}
		    } else if (!tr->use_icmp && inner_ip->protocol == IPPROTO_UDP) {
			if (ret >= ip_hlen + 8 + inner_ip_hlen + 4) {
			    uint16_t *inner_udp_ports = (uint16_t *)((char *)inner_ip + inner_ip_hlen);
			    uint16_t inner_dst_port = ntohs(inner_udp_ports[1]);
			    recv_seq = (int)(inner_dst_port - tr->port_base) + 1;
			}
		    }
		}
	    }
	    if (type == ICMP_DEST_UNREACH)
		is_final = true;
	}

	if (recv_seq == target_seq) {
	    res->got_reply = true;
	    res->type = type;
	    res->code = code;
	    res->rtt = (get_time_now() - start_time) * 1000.0;
	    res->is_final = is_final;
	    resolve_info(tr, &from, res);
	    return true;
	}
    }
}
