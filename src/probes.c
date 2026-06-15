#include "ft_traceroute.h"

static void print_probe_result(t_probe_result *res, char *last_ip, bool resolve_dns)
{
    if (!res->got_reply) {
	printf(" *");
	return;
    }

    if (strncmp(res->ip, last_ip, INET_ADDRSTRLEN) != 0) {
	if (resolve_dns && res->hostname[0] != '\0') {
	    printf(" %s (%s)", res->hostname, res->ip);
	} else {
	    printf(" %s", res->ip);
	}
	strncpy(last_ip, res->ip, INET_ADDRSTRLEN);
    }

    printf("  %.3f ms", res->rtt);
}

static bool run_hop(t_traceroute *tr, int ttl)
{
    int reached_dest = 0;
    char last_ip_on_line[INET_ADDRSTRLEN] = {0};
    static int global_seq = 1;

    if (setsockopt(tr->send_sock, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl)) < 0) {
	perror("setsockopt ttl");
	return false;
    }

    printf("%2d ", ttl);

    t_probe_result results[tr->probes_per_hop];
    double start_times[tr->probes_per_hop];
    int seqs[tr->probes_per_hop];

    memset(results, 0, sizeof(results));

    for (int i = 0; i < tr->probes_per_hop; i++) {
	seqs[i] = global_seq++;
	start_times[i] = get_time_now();
	if (tr->use_icmp)
	    send_icmp_packet(tr, seqs[i]);
	else
	    send_udp_packet(tr, seqs[i]);
    }

    receive_hop_packets(tr, results, seqs, start_times, tr->probes_per_hop);

    for (int i = 0; i < tr->probes_per_hop; i++) {
	print_probe_result(&results[i], last_ip_on_line, tr->resolve_dns);
	if (results[i].is_final)
	    reached_dest = 1;
	//fflush(stdout);
    }

    printf("\n");
    return (reached_dest == 1);
}

void run_traceroute(t_traceroute *tr)
{
    for (int ttl = 1; ttl <= tr->max_hops; ttl++) {
	if (run_hop(tr, ttl)) {
	    break;
	}
    }
}
