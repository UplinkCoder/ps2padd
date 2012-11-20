// udp_ip_test.c
#include <assert.h>
#include <string.h>
#include "../include/structs.h"
#include "../include/udp_ip.h"

// definition for net simulator
int net_rpc_init(void);
int net_init(void);
void net_quit(void);
// thoose can be with a empty impl
int net_getethaddr(char *ethaddr);

int net_receive(unsigned char *buffer);
int net_send(unsigned char *buffer, int len);
int net_getqueuedtxpkts(void);

char globalbuffer[1024]="BUFFA";
char data[512]="DATA";
int size_of_packet;

int main (void) {
net_start(22,0,0);

char arppkt1[] = {
0xbc, 0x05, 0x43, 0xaa, 0xf3, 0x0c, 0x00, 0x1f, 
0xc6, 0xf1, 0xa3, 0x29, 0x08, 0x06, 0x00, 0x01, 
0x08, 0x00, 0x06, 0x04, 0x00, 0x02, 0x00, 0x1f, 
0xc6, 0xf1, 0xa3, 0x29, 0xc0, 0xa8, 0x02, 0x67, 
0xbc, 0x05, 0x43, 0xaa, 0xf3, 0x0c, 0xc0, 0xa8, 
0x02, 0x01 };

struct arp_pkg *testarp = (struct arp_pkg*) arppkt1;
printf("%X \n",testarp->eth_hdr.ProtocolType);
printf("%02llx \n",testarp->eth_hdr.EthDst.mac64);

for (int i=0;i<6;i++) {
printf("%02X",testarp->eth_hdr.EthDst.mac8[i]);	
}

assert (testarp->eth_hdr.ProtocolType == ETH_PROTO_TYPE_ARP);
assert (testarp->eth_hdr.EthDst.mac64 == 0x0cf3aa4305bc );
assert (testarp->eth_hdr.EthSrc.mac64 == 0x29a3f1c61f00);
assert (testarp->SenderHardwareAddr.mac64 == testarp->eth_hdr.EthSrc.mac64);

udp_recv();

}

int net_rpc_init(void) {
	return 1;
}

int net_init(void) {
	return 1;
}
void net_quit(void) {
}

int net_getqueuedtxpkts(void) {
	return 1;
}


int net_getethaddr (char ethaddr[]) {
	ethaddr[0]=0xaa;
	ethaddr[1]=0xbb;
	ethaddr[2]=0xcc;
	ethaddr[3]=0xdd;
	ethaddr[4]=0xee;
	ethaddr[5]=0xff;
	return 1;
}

int net_receive (unsigned char *buffer) {
	memcpy(buffer,globalbuffer,size_of_packet);
return size_of_packet;
}

int net_send(unsigned char *buffer, int len) {
	memcpy(globalbuffer,buffer,len);
	return 1;
}
