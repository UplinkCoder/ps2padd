// functions-include.h
unsigned int itol(char*);
int net_start(unsigned int,unsigned int,unsigned int);
int arp_handle ();
int arp_insert (unsigned int, unsigned char*);
int arp_lookup (unsigned int, unsigned char*); // IT WRITES THE MAC INTO THE POINTED ADDR
void arp_clean ();
short get_eth_type();
int get_packet ();
unsigned short htons(unsigned short);
unsigned short ntohs(unsigned short);
unsigned int ntohl(unsigned int);
unsigned int htonl(unsigned int);
int	udp_connect (void* , unsigned int, unsigned short);
int udp_recvfrom (void*, void*, unsigned short);
int udp_recv (void*, unsigned short); // does not seem to work donno why
int udp_sendto(void*, unsigned short, unsigned int, unsigned short, 
													unsigned short);
int	udp_send (void*, void*, unsigned short);
unsigned short ip_chksum(const unsigned char*);
