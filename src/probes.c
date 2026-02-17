#include "ft_traceroute.h"


static t_probe_result perform_one_probe(t_traceroute *tr, int seq)
{
    t_probe_result  result;
    double          start_time;

    memset(&result, 0, sizeof(result));

    start_time = get_time_now();

    send_icmp_packet(tr, seq);

    receive_packet(tr, &result, seq);

    if (result.got_reply) {
        result.rtt = (get_time_now() - start_time) * 1000.0;
        if (result.type == ICMP_ECHOREPLY || result.type == ICMP_DEST_UNREACH)
            result.is_final = true;
    }

    return result;
}

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

    if (setsockopt(tr->sockfd, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl)) < 0) {
	perror("setsockopt ttl");
	return false;
    }

    printf("%2d ", ttl);

    for (int i = 0; i < tr->probes_per_hop; i++) {
	t_probe_result res = perform_one_probe(tr, global_seq++);

	print_probe_result(&res, last_ip_on_line, tr->resolve_dns);

	if (res.is_final)
	    reached_dest = 1;

	// fflush(stdout);
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
