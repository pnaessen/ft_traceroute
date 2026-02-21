#include "ft_traceroute.h"

void send_udp_packet(t_traceroute *tr, int seq)
{
    char packet[32];
    struct sockaddr_in dest;

    memset(packet, 0x42, sizeof(packet)); //Paylod random

    dest = tr->dest_addr;
    dest.sin_port = htons(tr->port_base + seq);

    if (sendto(tr->send_sock, packet, sizeof(packet), 0, (struct sockaddr *)&dest, sizeof(dest)) <=
	0) {
		perror("sendto");
    }
}
