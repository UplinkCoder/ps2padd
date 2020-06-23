#define debugPrint scr_printf

#include <assert.h>
#include <iopheap.h> 	// something to do with the iop reset
#include <iopcontrol.h>	// something to do with the iop reset
#include <kernel.h>		// important!!
#include <libpad.h>		// needed to use controller functions
#include <loadfile.h>	// for loading .irx files(we need to load mcman, and mcserv)
#include <malloc.h>		// allows for the malloc command
#include <sbv_patches.h>// patches required to load irxs, from mem
#include <sifrpc.h>		// something to do with irx files or something
#include <libgs.h>	
#include <stdio.h>		// standard input output library
//#include <fileio.h>		
#include <tamtypes.h>	// type defines for ps2 stuff
#include <string.h>		// string functions, memset/strcpy/strcat/etc
//#include <ps2ip.h>		// networking functions(send/connect/disconnect/close)
#include <libmc.h>		// library for accessing the memory card
#include <debug.h>
	// code i merged into sub-functions
#include "udp_ip.h"
#include "setup_net.h"
#include "setup_ip_afl.h"
#include "loadmodules.h"


// begin by resetting the iop
void iopReset() {
	scr_printf("IOP-RESET ...");
	SifIopReset("rom0:UDNL rom0:EELOADCNF",0);
	while(!SifIopSync());
	SifExitIopHeap();
	SifLoadFileExit();
	SifExitRpc();
	SifExitCmd();
	SifInitRpc(0);
	scr_printf(" DONE \n");
}
		
	

#define DEBUG_BGCOLOR(col) *((u64 *) 0x120000e0) = (u64) (col)


char* padTypeStr[] = {	"Unsupported controller", "Mouse", "Nejicon",
						"Konami Gun", "Digital", "Analog", "Namco Gun",
						"DualShock"};

static char *padBuf[2]; 
u32 portConnected[2];
u64 paddata[2];
u64 old_pad[2];
u32 new_pad[2];
u32 zro_pad[2];
u8 actDirect[2][6] = { {0,0,0,0,0,0}, {0,0,0,0,0,0}};

void wait_vsync() 
{
	// Enable the vsync interrupt.
	GS_SET_CSR(0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0);

	// Wait for the vsync interrupt.
	while ((GS_SET_CSR(0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0))) { }

	// Disable the vsync interrupt.
	GS_SET_CSR(0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0);
}

void padWait(int port)
{
	/* Wait for request to complete. */
	while(padGetReqState(port, 0) != PAD_RSTAT_COMPLETE)
		wait_vsync();

	/* Wait for pad to be stable. */
	while(padGetState(port, 0) != PAD_STATE_STABLE)
		wait_vsync();
}

void padStartAct(int port, int act, int speed)
{
	if(actDirect[port][act] != speed)
	{
		actDirect[port][act] = speed;	
	
		padSetActDirect(port, 0, actDirect[port]);
		padWait(port);
	}
}

void padStopAct(int port, int act)
{
	padStartAct(port, act, 0);
}
int new_udpconn (void *ptr); 

udp_send_string(udp_pkg* conn, char* str)
{
	const int len = strlen(str);	
	udp_send(conn,str,len);
}

int main(int argc, char **argv)
{
    init_scr();
	iopReset();
	u32 port;
	struct padButtonStatus buttons;
	int dualshock[2];
	int acts[2];
	udp_pkg* connptr = 0;
	
	scr_printf(" - Using PS2SDK free IRX and modded PKTDRV by ps2devman modules -\n");
	loadmodules();	
	scr_printf("Modules Loading Completed\n");
	
	net_start(itol(IP),itol(GW),itol(MK));
	
	padInit(0);
	scr_printf("PadInit called\n");

	
	assert (new_udpconn(connptr)); // size of udp-connecteion-struct
	
	setupNet(connptr);

    udp_send_string(connptr, "Ps2PaD++SRV-PONG");
	
	scr_printf("WE FUCKING SET UP NETWORK \n");

	padBuf[0] = memalign(64, 256);
	padBuf[1] = memalign(64, 256);

	old_pad[0] = 0;
	old_pad[1] = 0;	

	portConnected[0] = 0;
	portConnected[1] = 0;

	dualshock[0] = 0;
	dualshock[1] = 0;

	acts[0] = 0;
	acts[1] = 0;

	padPortOpen(0, 0, padBuf[0]);
	padPortOpen(1, 0, padBuf[1]);

	while(1)
	{	
		for(port=0; port < 2; port++)
		{
			s32 state = padGetState(port, 0);

			if((state == PAD_STATE_STABLE) && (portConnected[port] == 0)) 
			{
				u32 i;
				u8 mTable[8];
				u32 ModeCurId;
				u32 ModeCurOffs;
				u32 ModeCurExId;
				u32 ModeTableNum = padInfoMode(port, 0, PAD_MODETABLE, -1);
				
				scr_printf("Controller (%i) connected\n", port);

				/* Check if dualshock and if so, activate analog mode */
				for(i = 0; i < ModeTableNum; i++)
					mTable[i] = padInfoMode(port, 0, PAD_MODETABLE, i);
				
				/* Works for dualshock2 */
				if((mTable[0] == 4) && (mTable[1] == 7) && (ModeTableNum == 2))
					dualshock[port] = 1;

				/* Active and lock analog mode */
				if(dualshock[port] == 1)
				{
					padSetMainMode(port, 0, PAD_MMODE_DUALSHOCK, PAD_MMODE_LOCK);
					padWait(port);
				}
				
				ModeCurId = padInfoMode(port, 0, PAD_MODECURID, 0);
				ModeCurOffs = padInfoMode(port, 0, PAD_MODECUROFFS, 0);
				ModeCurExId = padInfoMode(port, 0, PAD_MODECUREXID, 0);
				ModeTableNum = padInfoMode(port, 0, PAD_MODETABLE, -1);
				acts[port] = padInfoAct(port, 0, -1, 0);		

				scr_printf("  ModeCurId      : %i (%s)\n", (int)ModeCurId, padTypeStr[ModeCurId]);
				scr_printf("  ModeCurExId    : %i\n", (int)ModeCurExId);
				scr_printf("  ModeTable      : ");
		
				for(i = 0; i < ModeTableNum; i++)
				{
					mTable[i] = padInfoMode(port, 0, PAD_MODETABLE, i);
					scr_printf("%i ", (int)mTable[i]);
				}	

				scr_printf("\n");
				scr_printf("  ModeTableNum   : %i\n", (int)ModeTableNum);
				scr_printf("  ModeCurOffs    : %i\n", (int)ModeCurOffs);
				scr_printf("  NumOfAct       : %i\n", (int)acts[port]);
				scr_printf("  PressMode      : %i\n", (int)padInfoPressMode(port, 0));

	
				if(acts[port] > 0)
				{
					u8 actAlign[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
					u32 i;

					/* Set offsets for motor parameters for SetActDirect. */
					for(i=0; i < acts[port]; i++)
						actAlign[i] = i;

					padSetActAlign(port, 0, actAlign);
					padWait(port);
				}

				scr_printf("  EnterPressMode : %i\n", (int)padEnterPressMode(port, 0));
				padWait(port);

				scr_printf("Ready\n");

				portConnected[port] = 1;
			}

			if((state == PAD_STATE_DISCONN) && (portConnected[port] == 1))
			{ 
				scr_printf("Controller (%i) disconnected\n", port);
				portConnected[port] = 0;
			}

			if(portConnected[port] == 1)
			{
				s32 ret = padRead(port, 0, &buttons);
								
				if(ret != 0)
				{
					memcpy(&paddata[port],&buttons.btns,6); // this SHOULD verify that no data is send double
					//paddata[port] = buttons.btns;//0xffff ^ buttons.btns;
					if (paddata[port]!=old_pad[port]) // send only if button-State has changed
					{old_pad[port] = paddata[port];
					 scr_printf("ljoy_h :%d \t ljoy_v %d\t rjoy_h %d\t rjoy_v %d",buttons.ljoy_h,buttons.ljoy_v,buttons.rjoy_h,buttons.rjoy_v);
					buttons.btns=~buttons.btns;
					udp_send(connptr,&buttons.btns,6);	// SENDING shit on wire
					}
						 
					
				}	
	
			}
		}
		wait_vsync();
	}

	return 0;
}

