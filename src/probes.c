#include "ft_traceroute.h"

int send_probes(t_traceroute *tr)
{
    int reached_destination = 0;
    static int global_seq = 1;

    for (int i = 0; i < tr->probes_per_hop; i++)
    {
        struct timeval start, end;
        gettimeofday(&start, NULL);

        send_icmp_packet(tr, global_seq++);

        t_response res = receive_packet(tr);

        if (res.got_reply)
        {
            gettimeofday(&end, NULL);
            double rtt = (end.tv_sec - start.tv_sec) * 1000.0 +
                         (end.tv_usec - start.tv_usec) / 1000.0;

            if (i == 0 || strcmp(res.ip_str, tr->last_ip) != 0)
            {
                printf(" %s (%s)", res.hostname, res.ip_str);
                strncpy(tr->last_ip, res.ip_str, INET_ADDRSTRLEN);
            }
            printf("  %.3f ms", rtt);

            if (res.type == ICMP_ECHOREPLY)
                reached_destination = 1;
        }
        else
        {
            printf("  *");
        }
        //fflush(stdout);
    }
    return reached_destination;
}
