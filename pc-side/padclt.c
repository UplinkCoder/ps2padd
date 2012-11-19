#include <stdio.h>
#include <windows.h>
#include <winioctl.h>
#include <winsock2.h>
#include "ppjioctl.h"
#include "ps2pad.h"

//#include "padproto.h"
//#include "functions.c"


#define	NUM_ANALOG	8	/* Number of analog values which we will provide */
#define	NUM_DIGITAL	16	/* Number of digital values which we will provide */

#pragma pack(push,1)		/* All fields in structure must be byte aligned. */
typedef struct
{
  unsigned long Signature;	/* Signature to identify packet to PPJoy IOCTL */
  char NumAnalog;		/* Num of analog values we pass */
  long Analog[NUM_ANALOG];	/* Analog values */
  char NumDigital;		/* Num of digital values we pass */
  char Digital[NUM_DIGITAL];	/* Digital values */
} JOYSTICK_STATE;
#pragma pack(pop)
int startWinsock (void);

int
main (int argc, char **argv)
{
  HANDLE h;
  JOYSTICK_STATE JoyState;
  DWORD RetSize;
  DWORD err;
  long *Analog;
  char *Digital, *DevName;

  char rc, msg[16],ch;
  int buf,opt=1;
  SOCKET sock;
  SOCKADDR_IN client, server;
  int serverLen = sizeof (SOCKADDR_IN);

  struct gamepad gamepad1, gamepad2;

  DevName = "\\\\.\\PPJoyIOCTL1";
  if (argc == 2)
    DevName = argv[1];

  /* Open a handle to the control device for the first virtual joystick. */
  /* Virtual joystick devices are names PPJoyIOCTL1 to PPJoyIOCTL16. */
  h =
    CreateFile (DevName, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_EXISTING,
		0, NULL);

  /* Make sure we could open the device! */
  if (h == INVALID_HANDLE_VALUE)
    {
      printf
	("CreateFile failed with error code %d trying to open %s device\n maybe ppjoy is not installed",
	 GetLastError (), DevName);
      return 1;
    }

   rc = startWinsock();
   if (rc != 0)
   {
      printf("ERROR: startWinsock faild, error code: %d\n", buf);
      return(1);
   }
       sock = socket(AF_INET, SOCK_DGRAM, 0);
   if (sock == INVALID_SOCKET)
   {
      printf("Error: Socket could not be created, error code: %d\n", WSAGetLastError());
      return(1);
   }
   rc = setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (char *)&opt, 1);
   if (rc==SOCKET_ERROR) {printf ("Socket could not be made bcast");}
   client.sin_family      = AF_INET;
   client.sin_port		  = htons(59804);
   client.sin_addr.s_addr = inet_addr("0.0.0.0");
   server.sin_addr.s_addr = inet_addr("255.255.255.255");
   server.sin_port        = htons(861);
   server.sin_family      = AF_INET;


  rc=bind(sock,(SOCKADDR*)&client,sizeof(client));
  if(rc==SOCKET_ERROR)
  {
    printf("Erorr: could not bind socket, fehler code: %d\n",WSAGetLastError());
    return 1;
  }

   memset(&msg, 0, sizeof(msg));
   strncpy((char *)&msg, "Ps2PaD++CLT-PING", 16);
   msg[16] = '\0';
   rc      = sendto(sock, (char *)&msg, sizeof(msg), 0, (SOCKADDR *)&server, serverLen);
   if (rc == SOCKET_ERROR)
   {
      printf("Error: could not send, fehler code: %d\n", WSAGetLastError());
      return(1);
   }
   printf("%i Bytes of %s were send \n %s is Server-IP and %i is Server-Port \n", rc, msg, inet_ntoa(server.sin_addr), ntohs(server.sin_port));
   rc = recvfrom(sock, (char *)&msg, sizeof(msg), 0, (SOCKADDR *)&server, (int *)&serverLen);
   if (rc == SOCKET_ERROR)
   {
      printf("Error: could not recive, fehler code: %d\n", WSAGetLastError());
      return(1);
   }
   printf("%i Bytes of %s were recived \n %s is Server-IP and %i is Server-Port \n", rc, msg, inet_ntoa(server.sin_addr), ntohs(server.sin_port));

   rc = connect(sock, (SOCKADDR *)&server, serverLen);
   if (rc == SOCKET_ERROR)
   {
      printf("Error: could not Connect to Server, fehler code: %d\n", WSAGetLastError());
      return(1);
   } else printf("Connected \n");

   strncpy((char *)&msg, "PaD-REQ1//Ps2PaD", 16);
   msg[16] = '\0';
   rc      = sendto(sock, (char *)&msg, sizeof(msg), 0, (SOCKADDR *)&server, serverLen);
   if (rc == SOCKET_ERROR)
   {
      printf("Error: could not send, fehler code: %d\n", WSAGetLastError());
      return(1);
   }





  /* Initialise the IOCTL data structure */
  JoyState.Signature = JOYSTICK_STATE_V1;
  JoyState.NumAnalog = NUM_ANALOG;	/* Number of analog values */
  Analog = JoyState.Analog;	/* Keep a pointer to the analog array for easy updating */
  JoyState.NumDigital = NUM_DIGITAL;	/* Number of digital values */
  Digital = JoyState.Digital;	/* Digital array */

  printf("Before while-loop");
  memset(&gamepad1, 0, sizeof(gamepad1)); // init gamepad struct with NULL

      /* get PaDState from PS2 and push it into Buffers */
	
	  while (strncmp(msg,"server_going_offline",16)&&(ch=getch())!=27))
	  {
	  //recv (sock,(char*)&gamepad1, 6, 0);
	  recv (sock,(char*)&msg, 16, 0);
	  memcpy(&gamepad1,&msg,6);
	  printf ("%X \n", gamepad1);
	  printf ("up : %i\t down : %i\t left: %i\t right:%i \n",gamepad1.up,gamepad1.down,gamepad1.left,gamepad1.right);

	  Digital[0]  = gamepad1.triangle;
      Digital[1]  = gamepad1.cross;
      Digital[2]  = gamepad1.square;
      Digital[3]  = gamepad1.circle;

	  Digital[4]  = gamepad1.L2;
      Digital[5]  = gamepad1.R2;
	  Digital[6]  = gamepad1.L1;
      Digital[7]  = gamepad1.R1;

	  Digital[8]  = gamepad1.select;
	  Digital[9]  = gamepad1.start;
	  Digital[10] = gamepad1.L3;
      Digital[11] = gamepad1.R3;

      Digital[12] = gamepad1.up;
      Digital[13] = gamepad1.right;
      Digital[14] = gamepad1.down;
      Digital[15] = gamepad1.left;


      Analog[0]  = gamepad1.LS_X;
      Analog[1]  = gamepad1.LS_Y;
      Analog[2]  = gamepad1.RS_X;
      Analog[3]  = gamepad1.RS_Y;

	  //memset(&gamepad1,0,sizeof(gamepad1));

      /* Send request to PPJoy for processing. */
      /* Currently there is no Return Code from PPJoy, this may be added at a */
      /* later stage. So we pass a 0 byte output buffer.                      */

      if (!DeviceIoControl
	  (h, IOCTL_PPORTJOY_SET_STATE, &JoyState, sizeof (JoyState), NULL, 0,
	   &RetSize, NULL))
	{
	  err = GetLastError ();
	  if (err == 2)
	    {
	      printf
		("Underlying joystick device deleted. Exiting read loop\n");
	      break;
	    }
	  printf ("DeviceIoControl error %d\n", err);
	}
    }

  printf ("Server goes off \nClient goes off too");
  CloseHandle (h);
  return 0;
}

int
startWinsock (void)
{
  WSADATA wsa;
  return WSAStartup (MAKEWORD (2, 0), &wsa);
}
