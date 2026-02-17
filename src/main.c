#include "ft_traceroute.h"

void init_traceroute(t_traceroute *tr, char *host)
{

    memset(tr, 0, sizeof(t_traceroute));
    tr->hostname = host;
    tr->max_hops = 30;
    tr->pid = getpid() & 0xFFFF;
    tr->probes_per_hop = 3;
}

#include "ft_traceroute.h"

int main(int argc, char **argv)
{
    t_traceroute tr;

    if (argc != 2)
	return (printf("Usage: ft_traceroute <dest>\n"), 1);

    init_traceroute(&tr, argv[1]);

    if (resolve_dns(tr.hostname, &tr.dest_addr) != EXIT_SUCCESS)
	return (1);

    if (setup_socket(&tr) < 0)
	return (1);

    printf("traceroute to %s (%s), %d hops max, 60 byte packets\n", tr.hostname,
	   inet_ntoa(tr.dest_addr.sin_addr), tr.max_hops);

    for (int ttl = 1; ttl <= tr.max_hops; ttl++) {
	printf("%2d ", ttl);

	if (setsockopt(tr.sockfd, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl)) < 0) {
	    perror("setsockopt ttl");
	    break;
	}

	// int reached = send_probes(&tr, ttl);

	// printf("\n");
	// if (reached)
	//     break; //  ICMP_ECHOREPLY
	printf("\n");
    }

    close(tr.sockfd);
    return (0);
}
