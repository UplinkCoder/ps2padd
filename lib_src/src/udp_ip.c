#include <string.h>
#include <stdlib.h>

#include "../include/pktdrv_rpc.h"
#include "../include/structs.h"
#include "../include/udp_ip.h"

// does not really belong here
int arp_entry_count=-1;
int buffer_count=0;
uint8_t     recvbuffer[1024] __attribute__((aligned (64))); //will be sent thru dma to iop therefore has to be 64-bit aligned 
uint8_t     sendbuffer[1024] __attribute__((aligned (64)));
int     lastpacketlength;

typedef struct udp_pkg udp_pkg;
typedef struct arp_entry arp_entry;

void arp_clean(void);

udp_pkg  *udp_pre_pkg; // GLOBAL VARIABLE THAT HOLDS DEFAULT VALUES


unsigned int itol(char* ip) {
    //convert IP address string into an unsigned long
    unsigned long r;
    char *p2;
    int i, n, v;
    
    p2 = (char *) &r;
    for (i = 0, n = 0, v = 0; i < strlen(ip) + 1; i++) {
    	if ((ip[i] >= '0') && (ip[i] <= '9'))
    		v = v * 10 + ip[i] - '0';
    	else if ((ip[i] == '.') || (ip[i] == '\0')) {
    		*p2 = (unsigned char) (v & 255);
    		v = 0;
    		p2++;
    		n++;
    		if (n == 4)
    			break;
    	}
    }
    
    return r;
}

// startup net init PKTDRV&RPC&ARP_TABLE

int net_start(unsigned int IP, unsigned int GW, unsigned int MK) {
    union mac ps2_ethaddr;
    
    if (net_rpc_init() && net_init()) {
    	net_getethaddr(ps2_ethaddr.mac8);
    	arp_clean();
    	udp_prefill(IP, ps2_ethaddr); // prefill is in testing
    	return 1;
    } else {
    	return -1;
	}
}

int get_packet() {
    lastpacketlength = net_receive(recvbuffer);
    if (lastpacketlength)
    	return 1;
    else
    	return 0;
}

/* Functions for processing ethernet header */

short get_eth_type() {
    struct eth_hdr *eth_pkg;
    short eth_type;
    eth_pkg = (struct eth_hdr *) recvbuffer;
    eth_type = eth_pkg->ProtocolType;
    return eth_type;
}

/*----------------------------------------------*/

// Functions for handling ARP 
struct arp_entry arp_table[MAX_ARP_TABLE_SIZE];

int arp_handle() {
    struct arp_pkg *arp_packet;
    arp_packet = (struct arp_pkg *) recvbuffer;
    int rv;
    
    if (arp_packet->Operation == 0x100
    		&& arp_packet->TargetProtocolAddr
    				== udp_pre_pkg->ip_hdr.SrcIPAddr) {
    
    	// build up an ARP response (invert src and dst)
    
    	arp_packet->eth_hdr.EthDst.mac64 = arp_packet->eth_hdr.EthSrc.mac64; // useing Sender as Dest
        arp_packet->eth_hdr.EthSrc.mac64 = udp_pre_pkg->eth_hdr.EthSrc.mac64; // myaddr as Src
    	arp_packet->Operation = 0x200;
        arp_packet->TargetHardwareAddr.mac64 = arp_packet->SenderHardwareAddr.mac64;
        arp_packet->SenderHardwareAddr.mac64 = udp_pre_pkg->eth_hdr.EthSrc.mac64;
    	
    	arp_packet->TargetProtocolAddr = arp_packet->SenderProtocolAddr;
    	arp_packet->SenderProtocolAddr = udp_pre_pkg->ip_hdr.SrcIPAddr;
    
    	net_send(recvbuffer, (sizeof(struct arp_pkg))); // CHANGE FUNCTION TO NOT USE RECVBUFFER FOR SENDING
    	rv = arp_lookup(arp_packet->TargetProtocolAddr,
    			arp_packet->TargetHardwareAddr);
    		if (rv == 0) {
    			arp_insert(arp_packet->TargetProtocolAddr,
    					arp_packet->TargetHardwareAddr);
    	}
    }
    return 1;
}

// IT WRITES THE MAC INTO THE POINTED ADDR
int arp_lookup(unsigned int ip, union mac mac) {
    int i;
    struct arp_entry *tabptr;
    for (i = 0; i <= arp_entry_count; i++) {
    	tabptr = &arp_table[i];
    	if (tabptr->ip == ip) {
    	mac.mac64 = tabptr->mac.mac64;
    		return 1;
    	}
    }
    return ERR_ARP_NOT_IN_TABLE;
}

void arp_clean(void) {
    struct arp_entry *tabptr;
    tabptr = &arp_table[0];
    memset(tabptr, 0, (MAX_ARP_TABLE_SIZE) * sizeof(struct arp_entry));
    arp_entry_count = 0;
}

int arp_insert(unsigned int ip, union mac mac) {
    struct arp_entry *tabptr;
    int rv;
    rv = arp_lookup(ip, mac);
    if (!rv) {
    	if (arp_entry_count <= MAX_ARP_TABLE_SIZE) {
    		tabptr = &arp_table[arp_entry_count];
    		tabptr->mac.mac64 = mac.mac64;
    		tabptr->ip = ip;
    		++arp_entry_count;
    		return 1;
    	} else {
    		return ERR_ARP_TABLE_FULL;
    	}
    }
    return 1;
}

unsigned short htons(unsigned short n) {
    return ((n & 0xff) << 8) | ((n & 0xff00) >> 8);
}

unsigned short ntohs(unsigned short n) {
    return ((n & 0xff) << 8) | ((n & 0xff00) >> 8);
}

unsigned int htonl(unsigned int n) {
    return ((n & 0xff) << 24) | ((n & 0xff00) << 8) | ((n & 0xff0000UL) >> 8)
    		| ((n & 0xff000000UL) >> 24);
}

unsigned int ntohl(unsigned int n) {
    return ((n & 0xff) << 24) | ((n & 0xff00) << 8) | ((n & 0xff0000UL) >> 8)
    		| ((n & 0xff000000UL) >> 24);
}

int udp_prefill(unsigned int ps2ip, union mac ps2_ethaddr) {
    udp_pre_pkg = (udp_pkg*) malloc(sizeof(udp_pkg));
    //  ETH HDR
    ps2_ethaddr.mac64 = udp_pre_pkg->eth_hdr.EthSrc.mac64;
    udp_pre_pkg->eth_hdr.ProtocolType = ETH_PROTO_TYPE_IP;
    // IP HDR
    udp_pre_pkg->ip_hdr.Version = 0x45;
    udp_pre_pkg->ip_hdr.TimeToLive = 64;
    udp_pre_pkg->ip_hdr.Protocol = IP_PROTO_UDP;
    udp_pre_pkg->ip_hdr.SrcIPAddr = ps2ip;
    return 1;
}

// INTERN

/* udp_connect prefills a udp_pkg */

int udp_connect(udp_pkg *defconptr, unsigned int destip, unsigned short dstport) {
    memcpy(defconptr, udp_pre_pkg, sizeof(udp_pkg));
    
    if (!arp_lookup(destip, defconptr->eth_hdr.EthDst))
    	return ERR_ARP_NOT_IN_TABLE;
    
    defconptr->ip_hdr.DstIPAddr = destip;
    defconptr->udp_hdr.UDPDstPort = htons(dstport);
    return 1;
}

int udp_send(udp_pkg *defconptr, void *databuf, unsigned short len) {
    udp_pkg *udp_packet;
    udp_packet = (udp_pkg*) sendbuffer;
    memcpy(udp_packet, defconptr, sizeof(udp_pkg));
    udp_packet->ip_hdr.TotalLength = htons(
    		sizeof(struct udp_pkg) + len - sizeof(struct eth_hdr));
    udp_packet->ip_hdr.HeaderChecksum = ip_chksum(&udp_packet->ip_hdr.Version);
    udp_packet->udp_hdr.Length = htons(len + 8);
    memcpy(sendbuffer + sizeof(udp_pkg), databuf, len);
    net_send(sendbuffer, (sizeof(udp_pkg) + len));
    return 1;
    }

int udp_sendto(void *databuf, unsigned short len, unsigned int destip, unsigned short dstport, unsigned short srcport) {
    udp_pkg *udp_packet;
    udp_packet = (udp_pkg*) sendbuffer;
    if (udp_connect(udp_packet, destip, dstport) > 0) {
    	udp_packet->ip_hdr.TotalLength = htons(
    			sizeof(udp_pkg) + len - sizeof(struct eth_hdr));
    	udp_packet->ip_hdr.HeaderChecksum = ip_chksum(
    			&udp_packet->ip_hdr.Version);
    	udp_packet->udp_hdr.Length = htons(len + 8);
    	udp_packet->udp_hdr.UDPDstPort = dstport;
    	memcpy(sendbuffer + sizeof(udp_pkg), databuf, len);
    	net_send(sendbuffer, (sizeof(udp_pkg) + len));
    	return 1;
    }
    return ERR_ARP_NOT_IN_TABLE;
}

int udp_recv(void *databuf, unsigned short dstport) {
    udp_pkg *udp_packet;
    int rv = -1;
    while (1) {
    	rv = get_packet();
    	if (rv > 0) {
    		udp_packet = (udp_pkg*) recvbuffer;
    		if (udp_packet->udp_hdr.UDPDstPort == htons(dstport)) {
    			memcpy(databuf, recvbuffer + sizeof(struct udp_pkg),
    			ntohs(udp_packet->udp_hdr.Length)-sizeof(struct udp_hdr));
    			return (htons(
    					ntohs(udp_packet->udp_hdr.Length)
    							- sizeof(struct udp_hdr)));
    		}
    	}
    }
    return 0;
}

int udp_recvfrom(udp_pkg *defconptr, void *databuf, unsigned short dstport) {
    udp_pkg *udp_packet;
    int rv = -1;
    while (1) {
    	rv = get_packet();
    	if (rv > 0) {
    		udp_packet = (udp_pkg*) recvbuffer;
    		dstport = ntohs(dstport);
    		if (udp_packet->udp_hdr.UDPDstPort == dstport) {
    			memcpy(databuf, recvbuffer + sizeof(struct udp_pkg),
    					ntohs(udp_packet->udp_hdr.Length)
    							- sizeof(struct udp_hdr));
    			arp_insert(udp_packet->ip_hdr.SrcIPAddr,
    					udp_packet->eth_hdr.EthSrc);
    			udp_connect(defconptr, udp_packet->ip_hdr.SrcIPAddr,
    					ntohs(udp_packet->udp_hdr.UDPSrcPort));
    			udp_packet = (udp_pkg*) defconptr;
    			udp_packet->udp_hdr.UDPSrcPort = dstport;
    			return ntohs(udp_packet->udp_hdr.Length)
    					- sizeof(struct udp_hdr);
    		}
    	}
    }
    return 0;
}

// taken from the uIP-stack by Adam Dunkels

unsigned short ip_chksum(unsigned char * data) {
    unsigned short t;
    unsigned short sum = 0;
    const unsigned char *dataptr;
    const unsigned char *last_byte;
    
    dataptr = data;
    last_byte = data + 19;
    
    while (dataptr < last_byte) { /* At least two more bytes */
    	t = (dataptr[0] << 8) + dataptr[1];
    	sum += t;
    	if (sum < t) {
    		sum++; /* carry */
    	}
    	dataptr += 2;
    }
    
    if (dataptr == last_byte) {
    	t = (dataptr[0] << 8) + 0;
    	sum += t;
    	if (sum < t) {
    		sum++; /* carry */
    	}
    }
    
    /* Return sum in host byte order. */
    return htons(~sum);
}
