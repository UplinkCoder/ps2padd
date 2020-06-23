// setup_ip_afl.h

#include <fileio.h>
#include <stdlib.h>
#include "udp_ip.h"

char IP[16], MK[16], GW[16];
int setup_ip_from_mc()
{

    const char* IPCONFIG_DAT_PATHS[] = {
        "mc0:/BIDATA-SYSTEM/IPCONFIG.DAT", //japan 
            "mc0:/BADATA-SYSTEM/IPCONFIG.DAT", //us 
            "mc0:/BEDATA-SYSTEM/IPCONFIG.DAT", //europe
            "mc0:/SYS-CONF/IPCONFIG.DAT", // default location(for most programs) 
            NULL
    };
    int ipcfg_ret = -1, i;
    for (i = 0; ipcfg_ret != 0 && IPCONFIG_DAT_PATHS[i] != NULL; ++i)
    {
        ipcfg_ret = setup_ip(IPCONFIG_DAT_PATHS[i]);
    }

}

int setup_ip(const char* ipconfig_dat_path)
{
    int i;
    int lFD;
    char lBuff[64];
    char lChr;

    //default value in case we don't find ipconfig.dat
    strcpy(IP, "192.168.137.5");
    strcpy(MK, "255.255.255.0");
    strcpy(GW, "192.168.0.1");

    lFD = fioOpen(ipconfig_dat_path, O_RDONLY);
    if (lFD >= 0)
    {
        memset(lBuff, 0, sizeof(lBuff));
        i = fioRead(lFD, lBuff, sizeof(lBuff) - 1);
        fioClose(lFD);

        if (i > 0)
        {
            lBuff[i] = '\0';

            for (i = 0; ((lChr = lBuff[i]) != '\0'); ++i)
                if (lChr == ' ' || lChr == '\r' || lChr == '\n')
                    lBuff[i] = '\0';

            strncpy(IP, lBuff, 15);
            i = strlen(IP) + 1;
            strncpy(MK, lBuff + i, 15);
            i += strlen(MK) + 1;
            strncpy(GW, lBuff + i, 15);

            debugPrint("IP=%s\n", IP);
            debugPrint("MK=%s\n", MK);
            debugPrint("GW=%s\n", GW);

            return 1;

        }
        else
            debugPrint("fioRead failed\n");
    }
    else
    {
        debugPrint("Can't find IPCONFIG.DAT on %s \n", ipconfig_dat_path);
    }

    return 0;
}
