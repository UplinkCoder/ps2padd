/*! \file ps2pad.h
    \brief Header that holds the gamepad data sturcture  
  */
/*! \struct gamepad
    \brief A structue that holds the gamepads data.
   
    Details.
*/


struct gamepad
{
// begin first byte
unsigned select : 1; 
unsigned L3 : 1;    
unsigned R3 : 1; 
unsigned start : 1; 

unsigned up : 1;
unsigned right : 1;
unsigned down : 1;
unsigned left : 1;
// begin second byte
unsigned L2 : 1;
unsigned R2 : 1;
unsigned L1 : 1;
unsigned R1 : 1;

unsigned triangle : 1;
unsigned circle : 1;
unsigned cross : 1;
unsigned square : 1;
// begin thrid byte
unsigned char RS_X;
// begin fourth byte
unsigned char RS_Y;
// begin fifth byte
unsigned char LS_X; 
// begin sixth byte 
unsigned char LS_Y; 
} __attribute__((packed));
