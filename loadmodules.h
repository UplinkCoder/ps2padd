// loadmodule.h
extern void iopReset(void);
#ifdef xmodules
// XPADMAN IRX
extern u8* xpadman_irx;
extern int size_xpadman_irx;
// XSIO2MAN IRX
extern u8* xsio2man_irx;
extern int size_xsio2man_irx;
#endif
// POWEROFF IRX
extern u8* poweroff_irx;
extern int size_poweroff_irx;
// PS2DEV9 IRX
extern u8* ps2dev9_irx;
extern int size_ps2dev9_irx;
// PKTDRV IRX
extern u8* pktdrv_irx;
extern int size_pktdrv_irx;	



void loadmodules(void)
{
	int ret,rv;
	sbv_patch_enable_lmb();
	sbv_patch_disable_prefix_check();
	
	ret = SifLoadModule("rom0:SIO2MAN",0,NULL);
	if(ret<0)
	scr_printf("SIO2MAN Failed!\n");
	else
	scr_printf("SIO2MAN OK!\n");
	
	ret = SifLoadModule("rom0:MCMAN",0,NULL);
	if(ret<0)
	scr_printf("MCMAN Failed!\n");
	else
	scr_printf("MCMAN OK!\n");
	
	ret = SifLoadModule("rom0:MCSERV",0,NULL);
	if(ret<0)
	scr_printf("MCSERV Failed!\n");
	else
	scr_printf("MCSERV OK!\n");
	
	setup_ip_from_mc();

	
	rv=SifExecModuleBuffer( &poweroff_irx, size_poweroff_irx, 0, NULL, &ret);
	if(rv<0)
	scr_printf("POWEROFF Failed!\n");
	else
	scr_printf("POWEROFF OK!\n");
				
	rv=SifExecModuleBuffer( &ps2dev9_irx, size_ps2dev9_irx, 0, NULL, &ret);
	if(rv<0)
	scr_printf("PS2DEV9 Failed!\n");
	else
	scr_printf("PS2DEV9 OK!\n");
		
	rv=SifExecModuleBuffer( &pktdrv_irx, size_pktdrv_irx, 0, NULL, &ret);
	if(rv<0)
	scr_printf("PKTDRV Failed!\n");
	else
	scr_printf("PKTDRV OK!\n");
	
#ifdef xmodules 

	iopReset();
	
	SifExecModuleBuffer( &xsio2man_irx, size_xsio2man_irx, 0, NULL, &ret);
	if(ret<0)
	scr_printf("XSIO2MAN Failed!\n");
	else
	scr_printf("XSIO2MAN OK!\n");
	
	SifExecModuleBuffer( &xpadman_irx, size_xpadman_irx, 0, NULL, &ret);
	if(ret<0)
	scr_printf("XPADMAN Failed!\n");
	else
	scr_printf("XPADMAN OK!\n");
#else 	
	ret = SifLoadModule("rom0:PADMAN",0,NULL);
	if(ret<0)
	scr_printf("PADMAN Failed!\n");
	else
	scr_printf("PADMAN OK!\n");
#endif 
}

