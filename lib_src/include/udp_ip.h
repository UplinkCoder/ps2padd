// functions-include.h
#include "structs.h"

unsigned int itol(char IP[]); 
int net_start(unsigned int,unsigned int,unsigned int);

short get_eth_type(void);
int get_packet (void);

unsigned short htons(unsigned short);
unsigned short ntohs(unsigned short);
unsigned int ntohl(unsigned int);
unsigned int htonl(unsigned int);

int arp_lookup(unsigned int ip,union mac mac);
int arp_insert(unsigned int ip,union mac mac);

int udp_prefill(unsigned int ps2ip, union mac ps2_ethaddr);
int udp_connect(struct udp_pkg *defconptr, unsigned int destip,
	 unsigned short dstport);
int udp_recvfrom (struct udp_pkg *defconptr, void* databuf,
	 unsigned short port);
int udp_recv(void *databuf, unsigned short port);
int udp_sendto(void * databuf, unsigned short len, unsigned int destip,
 	unsigned short dstport, unsigned short srcport);
int	udp_send (struct udp_pkg*, void* databuf, unsigned short srcport);
unsigned short ip_chksum(unsigned char *);
