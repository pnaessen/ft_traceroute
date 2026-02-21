#include "ft_traceroute.h"

void send_icmp_packet(t_traceroute *tr, int seq)
{
    char packet[64];
    struct icmphdr *icmp;

    memset(packet, 0, sizeof(packet));
    icmp = (struct icmphdr *)packet;

    icmp->type = ICMP_ECHO;
    icmp->code = 0;
    icmp->un.echo.id = htons(tr->pid);
    icmp->un.echo.sequence = htons(seq);

    memset(packet + sizeof(struct icmphdr), 0, sizeof(packet) - sizeof(struct icmphdr));

    icmp->checksum = 0;
    icmp->checksum = calculate_checksum((unsigned short *)packet, sizeof(packet));

    if (sendto(tr->send_sock, packet, sizeof(packet), 0, (struct sockaddr *)&tr->dest_addr,
	       sizeof(tr->dest_addr)) <= 0) {
	// perror("sendto");
    }
}
