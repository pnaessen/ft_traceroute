#include "ft_traceroute.h"

static void set_defaults(t_traceroute *tr)
{
    memset(tr, 0, sizeof(t_traceroute));
    tr->max_hops = DEF_MAX_HOPS;
    tr->probes_per_hop = DEF_PROBES_PER_HOP;
    tr->resolve_dns = true;
    tr->pid = getpid() & 0xFFFF;
}

int main(int argc, char **argv)
{
    t_traceroute tr;

    if (geteuid() != 0) {
	fprintf(stderr, "ft_traceroute: root privileges required.\n");
	return (EXIT_FAILURE);
    }

    set_defaults(&tr);
    if (parse_args(argc, argv, &tr) != EXIT_SUCCESS) {
	return (EXIT_FAILURE);
    }

    if (resolve_dns(tr.target, &tr.dest_addr) != EXIT_SUCCESS) {
	return (EXIT_FAILURE);
    }

    if (create_socket(&tr) != EXIT_SUCCESS) {
	return (EXIT_FAILURE);
    }

    char ip_str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &tr.dest_addr.sin_addr, ip_str, INET_ADDRSTRLEN);

    printf("traceroute to %s (%s), %d hops max, 60 byte packets\n", tr.target, ip_str, tr.max_hops);

    run_traceroute(&tr);

    close(tr.sockfd);
    return (EXIT_SUCCESS);
}
