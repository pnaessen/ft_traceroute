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

void receive_hop_packets(t_traceroute *tr, t_probe_result *results, int *seqs, double *start_times, int num_probes)
{
    char buffer[512];
    struct sockaddr_in from;
    socklen_t from_len;
    ssize_t ret;
    fd_set readfds;
    struct timeval tv;
    double timeout_sec = 1.0;

    while (true) {
	double min_remaining = -1.0;
	double now = get_time_now();
	for (int i = 0; i < num_probes; i++) {
	    if (!results[i].got_reply) {
		double elapsed = now - start_times[i];
		if (elapsed < timeout_sec) {
		    double remaining = timeout_sec - elapsed;
		    if (min_remaining < 0.0 || remaining < min_remaining) {
			min_remaining = remaining;
		    }
		}
	    }
	}

	if (min_remaining < 0.0) {
	    break;
	}

	tv.tv_sec = (time_t)min_remaining;
	tv.tv_usec = (suseconds_t)((min_remaining - tv.tv_sec) * 1000000.0);

	FD_ZERO(&readfds);
	FD_SET(tr->recv_sock, &readfds);

	int ready = select(tr->recv_sock + 1, &readfds, NULL, NULL, &tv);

	if (ready < 0) {
	    if (errno == EINTR)
		continue;
	    perror("select");
	    break;
	} else if (ready == 0) {
	    continue;
	}

	from_len = sizeof(from);
	ret = recvfrom(tr->recv_sock, buffer, sizeof(buffer), 0, (struct sockaddr *)&from, &from_len);

	if (ret < 0) {
	    if (errno == EINTR)
		continue;
	    perror("recvfrom");
	    break;
	}

	if (ret < (ssize_t)sizeof(struct iphdr)) {
	    continue;
	}

	struct iphdr *ip = (struct iphdr *)buffer;
	int ip_hlen = ip->ihl * 4;

	if (ret < ip_hlen + (ssize_t)sizeof(struct icmphdr)) {
	    continue;
	}

	struct icmphdr *icmp = (struct icmphdr *)(buffer + ip_hlen);
	int recv_seq = -1;
	int type = icmp->type;
	int code = icmp->code;

	if (type == ICMP_ECHOREPLY) {
	    if (ntohs(icmp->un.echo.id) == tr->pid) {
		recv_seq = ntohs(icmp->un.echo.sequence);
	    }
	} else if (type == ICMP_TIME_EXCEEDED || type == ICMP_DEST_UNREACH) {
	    if (ret >= ip_hlen + 8 + (ssize_t)sizeof(struct iphdr)) {
		struct iphdr *inner_ip = (struct iphdr *)(buffer + ip_hlen + 8);
		int inner_ip_hlen = inner_ip->ihl * 4;

		if (tr->use_icmp && inner_ip->protocol == IPPROTO_ICMP) {
		    if (ret >= ip_hlen + 8 + inner_ip_hlen + (ssize_t)sizeof(struct icmphdr)) {
			struct icmphdr *inner_icmp = (struct icmphdr *)((char *)inner_ip + inner_ip_hlen);
			if (ntohs(inner_icmp->un.echo.id) == tr->pid) {
			    recv_seq = ntohs(inner_icmp->un.echo.sequence);
			}
		    }
		} else if (!tr->use_icmp && inner_ip->protocol == IPPROTO_UDP) {
		    if (ret >= ip_hlen + 8 + inner_ip_hlen + 4) { // ports (4 bytes)
			uint16_t *inner_udp_ports = (uint16_t *)((char *)inner_ip + inner_ip_hlen);
			uint16_t inner_dst_port = ntohs(inner_udp_ports[1]);
			recv_seq = (int)inner_dst_port - tr->port_base;
		    }
		}
	    }
	}

	if (recv_seq >= 0) {
	    for (int i = 0; i < num_probes; i++) {
		if (seqs[i] == recv_seq && !results[i].got_reply) {
		    results[i].got_reply = true;
		    results[i].type = type;
		    results[i].code = code;
		    results[i].rtt = (get_time_now() - start_times[i]) * 1000.0;
		    resolve_info(tr, &from, &results[i]);
		    if (type == ICMP_ECHOREPLY || type == ICMP_DEST_UNREACH) {
			results[i].is_final = true;
		    }
		    break;
		}
	    }
	}
    }
}
