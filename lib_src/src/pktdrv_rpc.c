
#include <stdio.h>
#include <kernel.h>
#include <sifrpc.h>
#include <string.h>

#include "../include/pktdrv_rpc.h"

//#include "debug.h"

#define PKTDRV_IRX	0x8112006

/* rpc client */
static struct t_SifRpcClientData 	cd0;
static unsigned int 			buff[2048/4] 		__attribute__((aligned(64)));
static unsigned int 			_intr_data[128/4] 	__attribute__((aligned(64)));

static int completion_sema;

static void _recv_intr()
{
	iSignalSema(completion_sema);
}

/* RPC calling rule
    @param func    procedure to invoke
    @param arg1    optional argument       <=buff[0]
    @param arg2    optional argument       <=buff[1]
    etc...
    @returns value returned by RPC server  =>buff[0]
    @returns other results in buff[0] and following ones
*/

int net_init(void)
{
	WaitSema(completion_sema);
	SifWriteBackDCache(_intr_data, 128);
	SifWriteBackDCache(buff, 4);
	SifCallRpc(&cd0, 0, 0, buff, 0, buff, 4, _recv_intr, _intr_data);
	FlushCache(0);

	return buff[0];
}

void net_quit(void)
{
	WaitSema(completion_sema);
	SifWriteBackDCache(_intr_data, 128);
	SifCallRpc(&cd0, 1, 0, buff, 0, buff, 0, _recv_intr, _intr_data);
	FlushCache(0);
}

int net_getethaddr(char *ethaddr)
{
	if (!ethaddr) return 0;

	WaitSema(completion_sema);
	SifWriteBackDCache(_intr_data, 128);
	SifWriteBackDCache(buff, 4*3);
	SifCallRpc(&cd0, 2, 0, buff, 0, buff, 4*3 , _recv_intr, _intr_data);
	FlushCache(0);

	*((unsigned int *)ethaddr)=buff[1];
	*((unsigned short *)&ethaddr[4])=buff[2];
	
	return buff[0];
}

//recv_buf must be aligned on 64 bytes boundaries (16 actually and data size multiple of 16)
int net_receive(unsigned char *recvbuf)
{
	int n=0;

	if (recvbuf)
	{
		WaitSema(completion_sema);
		if( !IS_UNCACHED_SEG(recvbuf))
			SifWriteBackDCache(recvbuf, 1536);
		SifWriteBackDCache(_intr_data, 128);
		SifWriteBackDCache(buff, 8);
		buff[0]=(int)recvbuf;
		SifCallRpc(&cd0, 3, 0, buff, 4, buff, 4,
				 _recv_intr, _intr_data);
		FlushCache(0);

		n=buff[0];
	}

	return n;
}

//send_buf must be aligned on 64 bytes boundaries (16 actually and data size multiple of 16)
int net_send(unsigned char *sendbuf, int len)
{
	int n=0;

	if ((sendbuf)&&(len))
	{
		WaitSema(completion_sema);
		if( !IS_UNCACHED_SEG(sendbuf))
			SifWriteBackDCache(sendbuf, (len+15)&~15);
		SifWriteBackDCache(_intr_data, 128);
		SifWriteBackDCache(buff, 8);
		buff[0]=(int)sendbuf;
		buff[1]=len;
		SifCallRpc(&cd0, 4, 0, buff, 8, buff, 4,
				 _recv_intr, _intr_data);
		FlushCache(0);

		n=buff[0];
	}

	return n;
}

int net_getqueuedtxpkts(void)
{
	WaitSema(completion_sema);
	SifWriteBackDCache(_intr_data, 128);
	SifWriteBackDCache(buff, 4);
	SifCallRpc(&cd0, 5, 0, buff, 0, buff, 4, _recv_intr, _intr_data);
	FlushCache(0);

	return buff[0];
}

int net_rpc_init()
{
	int i;
	ee_sema_t compSema;

	memset(&cd0, 0, sizeof(cd0));

	while(1)
	{
		if(SifBindRpc(&cd0, PKTDRV_IRX, 0) < 0)
		{
//			debugPrint("SifBindRpc failed\n");
			return 0;
		}

		if(cd0.server != 0) 
			break;

		i = 0x10000;
		while(i--);
	}


	compSema.init_count = 1;
	compSema.max_count = 1;
	compSema.option = 0;
	completion_sema = CreateSema(&compSema);
	if (completion_sema < 0)
	{
//		debugPrint("completion semaphore creation failed\n");
		return 0;
	}

	return 1;
}

