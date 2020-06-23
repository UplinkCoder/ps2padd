# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id: Makefile.sample 577 2004-09-14 14:41:46Z pixel $

XMODULES = 0

EE_BIN = ps2padd.elf
EE_LDFLAGS = -L./lib
ifeq ($(XMODULES),1)
EE_OBJS = ps2padd.o ps2dev9.o poweroff.o  pktdrv.o pktdrv_rpc.o graph.o font.o xsio2man.o xpadman.o udp_ip.o
EE_LIBS = -lpatches -lpadx -ludp_ip
EE_CFLAGS = -I./include -Dxmodules
else
EE_OBJS = ps2padd.o ps2dev9.o poweroff.o  pktdrv.o pktdrv_rpc.o graph.o font.o udp_ip.o
EE_LIBS = -lpatches -lpad
EE_CFLAGS = -I./include
endif

all: $(EE_BIN) after

clean:
	rm -f *.elf *.o *.a *.s

after:
	$(EE_STRIP) -s $(EE_BIN)
	rm -f *.o *.a *.s
	


ps2dev9.s: 
	bin2s $(PS2SDK)/iop/irx/ps2dev9.irx  $@ ps2dev9_irx
	
poweroff.s: 
	bin2s $(PS2SDK)/iop/irx/poweroff.irx  $@ poweroff_irx
	
pktdrv.s:
	bin2s pktdrv.irx $@	pktdrv_irx

xpadman.s: 
	bin2s $(PS2SDK)/iop/irx/freepad.irx  $@ xpadman_irx

xsio2man.s: 
	bin2s $(PS2SDK)/iop/irx/freesio2.irx  $@ xsio2man_irx
	
include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal

