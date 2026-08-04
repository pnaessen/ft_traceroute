#include "ft_traceroute.h"

void send_udp_packet(t_traceroute *tr, int seq)
{
    char packet[32];
    struct sockaddr_in dest;

    memset(packet, 0x42, sizeof(packet));

    dest = tr->dest_addr;
    uint32_t raw_port = (uint32_t)tr->port_base + seq - 1;
    uint16_t dst_port = ((raw_port - 1) % 65535) + 1;
    dest.sin_port = htons(dst_port);

    (void)sendto(tr->send_sock, packet, sizeof(packet), 0, (struct sockaddr *)&dest, sizeof(dest));
}
