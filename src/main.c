#include "ft_traceroute.h"

void init_traceroute(t_traceroute *tr, char *host) {

    memset(tr, 0, sizeof(t_traceroute));
    tr->hostname = host;
    tr->max_hops = 30;
    tr->pid = getpid() & 0xFFFF;
    tr->probes_per_hop = 3;

}

int main(int argc, char **argv)
{
    t_traceroute tr;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <destination>\n", argv[0]);
        return EXIT_FAILURE;
    }

    init_traceroute(&tr, argv[1]);

    if (setup_socket(&tr) < 0)
        return EXIT_FAILURE;

    if (resolve_dns(tr.hostname, &tr.dest_addr) != EXIT_SUCCESS)
        return EXIT_FAILURE;

    printf("traceroute to %s (%s), %d hops max\n", tr.hostname,
           inet_ntoa(tr.dest_addr.sin_addr), tr.max_hops);

    for (int ttl = 1; ttl <= tr.max_hops; ttl++) {
        tr.ttl = ttl;

        if (setsockopt(tr.sockfd, IPPROTO_IP, IP_TTL, &tr.ttl, sizeof(tr.ttl)) < 0) {
            perror("setsockopt ttl");
            break;
        }

        printf("%2d ", ttl);
        // send 3 loop_probes
        // print result


        // if (reached_destination) break;

        printf("\n");
    }

    close(tr.sockfd);
    return EXIT_SUCCESS;
}
