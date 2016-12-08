
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/if_ether.h>
#include <linux/if_arp.h>
#include <netinet/ip.h>

int set_light_state(unsigned int light_id, const char *state)
{
    // Set the state of a Hue light by calling the REST API with curl. Need to configure
    // Hue bridge IP and whitelisted user ID here
    const char hue_user_id[] = "<insert user ID here>";
    const char hue_bridge[]  = "philips-hue";

    char curl_cmd_buf[512];
    snprintf(
        curl_cmd_buf,
        sizeof(curl_cmd_buf),
        "curl --request PUT --data '%s' http://%s/api/%s/lights/%i/state",
        state,
        hue_bridge,
        hue_user_id,
        light_id);

    int ret = system(curl_cmd_buf);
    printf("\n");
    return ret;
}

struct arphdr_ether
{
    unsigned char ar_sha[ETH_ALEN]; // Sender hardware address
    unsigned char ar_sip[4];	    // Sender protocol address
    unsigned char ar_tha[ETH_ALEN]; // Target hardware address
    unsigned char ar_tip[4];	    // Target protocol address
};

int main(int argc, char **argv)
{
    int s, err;

    // Socket
    if ((s = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ARP))) == -1)
    {
        perror("socket(): ");
        exit(EXIT_FAILURE);
    }

    // Bind
    struct sockaddr_ll addr;
    addr.sll_family   = AF_PACKET;
    addr.sll_protocol = htons(ETH_P_ARP); 
    addr.sll_ifindex  = 0; // All interfaces
    if ((err = bind(s, (struct sockaddr *) &addr, sizeof(addr))) == -1)
    {
        perror("bind(): ");
        exit(EXIT_FAILURE);
    }

    // Process ARP messages
    while (1)
    {
        // Receive buffer
        char buf[IP_MAXPACKET];
        assert(sizeof(buf) >= sizeof(struct ethhdr) +
                              sizeof(struct arphdr) +
                              sizeof(struct arphdr_ether));
        memset(buf, 0, sizeof(buf));

        // Receive
        ssize_t len;
        if ((len = recv(s, buf, sizeof(buf), 0)) == -1)
        {
            perror("recv(): ");
            break;
        }
        if (len <= (ssize_t) (sizeof(struct ethhdr) + sizeof(struct arphdr)))
        {
            printf("Received short packet (%ib), skipping\n", len);
            continue;
        }

        // Ethernet frame
        struct ethhdr *eth_hdr = (struct ethhdr *) buf;

        if (ntohs(eth_hdr->h_proto) != ETH_P_ARP)
        {
            printf("Received non ARP-protocol ethernet frame (%i), skipping\n",
                ntohs(eth_hdr->h_proto));
            continue;
        }

#ifdef DEBUG_DUMP_PACKETS
        // ARP header
        struct arphdr *arp_hdr = (struct arphdr *) (buf + sizeof(struct ethhdr));
        struct arphdr_ether *arp_hdr_ether =
            (struct arphdr_ether *) (buf + sizeof(struct ethhdr) + sizeof(struct arphdr));

        // Debug dump ethernet frame (see /usr/include/inet/if_ether.h)
        printf("Ethernet destination: %02x:%02x:%02x:%02x:%02x:%02x\n",
            eth_hdr->h_dest[0], eth_hdr->h_dest[1], eth_hdr->h_dest[2],
            eth_hdr->h_dest[3], eth_hdr->h_dest[4], eth_hdr->h_dest[5]);
        printf("Ethernet source: %02x:%02x:%02x:%02x:%02x:%02x\n",
            eth_hdr->h_source[0], eth_hdr->h_source[1], eth_hdr->h_source[2],
            eth_hdr->h_source[3], eth_hdr->h_source[4], eth_hdr->h_source[5]);
        printf("Ethernet protocol ID: %i\n", ntohs(eth_hdr->h_proto));

        // Debug dump ARP header (see /usr/include/inet/if_arp.h)
        printf("ARP hardware type: %i\n", ntohs(arp_hdr->ar_hrd));
        printf("ARP protocol type: %i\n", ntohs(arp_hdr->ar_pro));
        printf("ARP hardware addr len: %i\n", arp_hdr->ar_hln);
        printf("ARP protocol addr len: %i\n", arp_hdr->ar_pln);
        printf("ARP operation: %i\n", ntohs(arp_hdr->ar_op));

        // Debug dump ethernet part of ARP header, if we indeed have an ethernet type header
        if (ntohs(arp_hdr->ar_hrd) == ARPHRD_ETHER)
        {
            assert(len >= (ssize_t) (sizeof(struct ethhdr) +
                                     sizeof(struct arphdr) +
                                     sizeof(struct arphdr_ether)));

            printf("ARP sender hardware address: %02x:%02x:%02x:%02x:%02x:%02x\n",
                arp_hdr_ether->ar_sha[0], arp_hdr_ether->ar_sha[1], arp_hdr_ether->ar_sha[2],
                arp_hdr_ether->ar_sha[3], arp_hdr_ether->ar_sha[4], arp_hdr_ether->ar_sha[5]);
            printf("ARP sender protocol address: %i.%i.%i.%i\n",
                arp_hdr_ether->ar_sip[0], arp_hdr_ether->ar_sip[1],
                arp_hdr_ether->ar_sip[2], arp_hdr_ether->ar_sip[3]);
            printf("ARP target hardware address: %02x:%02x:%02x:%02x:%02x:%02x\n",
                arp_hdr_ether->ar_tha[0], arp_hdr_ether->ar_tha[1], arp_hdr_ether->ar_tha[2],
                arp_hdr_ether->ar_tha[3], arp_hdr_ether->ar_tha[4], arp_hdr_ether->ar_tha[5]);
            printf("ARP target protocol address: %i.%i.%i.%i\n",
                arp_hdr_ether->ar_tip[0], arp_hdr_ether->ar_tip[1],
                arp_hdr_ether->ar_tip[2], arp_hdr_ether->ar_tip[3]);
        }

        printf("\n");
#endif // DEBUG_DUMP_PACKETS

        // String representation of the potential Dash MAC
        char mac_buf[32];
        snprintf(mac_buf, sizeof(mac_buf), "%02x:%02x:%02x:%02x:%02x:%02x",
            eth_hdr->h_source[0], eth_hdr->h_source[1], eth_hdr->h_source[2],
            eth_hdr->h_source[3], eth_hdr->h_source[4], eth_hdr->h_source[5]);

        // Execute button functions on MAC address match
        if (strcmp(mac_buf, "ac:63:be:c1:ca:c8") == 0)
        {
            printf("Button 1 pressed\n");
            set_light_state(14, "{ \"on\" : true }");
        }
        else if (strcmp(mac_buf, "50:f5:da:ee:ab:c1") == 0)
        {
            printf("Button 2 pressed\n");
            set_light_state(12, "{ \"on\" : true }");
        }
        // <Add futher buttons here>
    }

    // Shutdown
    err = close(s);
    assert(err == 0);

    return 0;
}

