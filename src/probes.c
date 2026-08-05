#include "ft_traceroute.h"

static void print_probe_result(t_probe_result *res, char *last_ip, bool resolve_dns)
{
    if (!res->got_reply) {
	printf(" *");
	return;
    }

    if (strncmp(res->ip, last_ip, INET_ADDRSTRLEN) != 0) {
	printf(" ");
	if (resolve_dns) {
	    printf("%s (%s)", res->hostname, res->ip);
	} else {
	    printf("%s", res->ip);
	}
	strncpy(last_ip, res->ip, INET_ADDRSTRLEN);
    }

    printf("  %.3f ms", res->rtt);
}

static bool run_hop(t_traceroute *tr, int ttl)
{
    bool reached_dest = false;
    char last_ip_on_line[INET_ADDRSTRLEN] = {0};
    static int global_seq = 1;

    if (setsockopt(tr->send_sock, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl)) < 0) {
	perror("setsockopt ttl");
	return false;
    }

    printf("%2d ", ttl);
    fflush(stdout);

    for (int i = 0; i < tr->probes_per_hop; i++) {
	int seq = global_seq++;
	if (tr->use_icmp)
	    send_icmp_packet(tr, seq);
	else
	    send_udp_packet(tr, seq);

	double effective_timeout = tr->timeout;
	if (ttl == 1 && effective_timeout == 0.0) {
	    effective_timeout = 3.0;
	} else if (tr->last_rtt_sec > 0.0) {
	    double adaptive = tr->last_rtt_sec * 3.0;
	    if (adaptive < 0.02)
		adaptive = 0.02;
	    if (adaptive < effective_timeout)
		effective_timeout = adaptive;
	}

	t_probe_result res;
	receive_single_packet(tr, seq, effective_timeout, &res);

	if (res.got_reply) {
	    tr->last_rtt_sec = res.rtt / 1000.0;
	}

	print_probe_result(&res, last_ip_on_line, tr->resolve_dns);
	fflush(stdout);

	if (res.is_final)
	    reached_dest = true;
    }

    printf("\n");
    return reached_dest;
}

void run_traceroute(t_traceroute *tr)
{
    for (int ttl = 1; ttl <= tr->max_hops; ttl++) {
	if (run_hop(tr, ttl)) {
	    break;
	}
    }
}
