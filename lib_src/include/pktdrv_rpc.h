#ifndef __PKTDRV_RPC_H__
#define __PKTDRV_RPC_H__

int net_rpc_init(void);
int net_init(void);
void net_quit(void);
int net_getethaddr(char *ethaddr);
int net_receive(unsigned char *buffer);
int net_send(unsigned char *buffer, int len);
int net_getqueuedtxpkts(void);

#endif
