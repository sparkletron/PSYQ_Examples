/*
 * Written By: John Convertino
 * 
 * Simple Example of using a controller to move a primitive.
 * 
 * Based on psxdev.net hello_world example, using libgs to simplify
 * setup of the display buffer.
 * 
*/

#include <stdlib.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <libpad.h>
#include <libetc.h>

#define SCREEN_WIDTH  320 // screen width
#define	SCREEN_HEIGHT 240 // screen height

u_long __ramsize   = 0x00200000; // force 2 megabytes of RAM
u_long __stacksize = 0x00004000; // force 16 kilobytes of stack

void graphics();
void display(POLY_F4 *f4);

struct
{
  struct
  {
    struct
    {
      u_char status:8;
    } byte;
    
  } first;
  
  union
  {
    struct
    {
      u_char recvSize:4;
      u_char type:4;
    } nibble;
    u_char byte:8;
    
  } second;
  
  union
  {
    struct
    {
      u_char select:1;
      u_char NA2:1;
      u_char NA1:1;
      u_char start:1;
      u_char up:1;
      u_char right:1;
      u_char down:1;
      u_char left:1;
    } bit;
    u_char byte:8;
	  
  } third;
  
  union
  {
    struct
    {
      u_char L2:1;
      u_char R2:1;
      u_char L1:1;
      u_char R1:1;
      u_char triangle:1;
      u_char circle:1;
      u_char ex:1;
      u_char square:1;
    } bit;
    u_char byte:8;
	  
  } fourth;
} g_pad[2];

int main() 
{
  POLY_F4 f4;
  int prevTime = 0;

  graphics(); // setup the graphics (seen below)

  PadInitDirect((u_char *)&g_pad[0], (u_char *)&g_pad[1]);
  //PadInitDirect(gpad[0], gpad[1]);
  PadStartCom();

  SetPolyF4(&f4);
  /*initialize primitive*/
  setRGB0(&f4, 0, 0, 255);
  /*R,G,B = 0, 0, 255*/
  setXY4(&f4, 0, 0, 50, 0, 0, 50, 50, 50);
  /*execute primitive*/

  while (1) // draw and display forever
  {
    if(g_pad[0].fourth.bit.ex == 0)
    {
      if(prevTime == 0 || ((VSync(-1) - prevTime) > 60))
      {
	f4.r0 = rand() % 256;
	f4.g0 = rand() % 256;
	f4.b0 = rand() % 256;
	prevTime = VSync(-1);
      }
    }
    
    if(g_pad[0].third.bit.up == 0)
    {
      f4.y0 -= 1;
      f4.y1 -= 1;
      f4.y2 -= 1;
      f4.y3 -= 1;
      if(f4.y0 < 0)
      {
	f4.y0 = SCREEN_HEIGHT - 50;
	f4.y1 = SCREEN_HEIGHT - 50;
	f4.y2 = SCREEN_HEIGHT;
	f4.y3 = SCREEN_HEIGHT;
      }
    }
    
    if(g_pad[0].third.bit.right == 0)
    {
      f4.x0 += 1;
      f4.x1 += 1;
      f4.x2 += 1;
      f4.x3 += 1;
      if(f4.x1 > SCREEN_WIDTH)
      {
	f4.x0 = 0;
	f4.x1 = 50;
	f4.x2 = 0;
	f4.x3 = 50;
      }
    }
    
    if(g_pad[0].third.bit.down == 0)
    {
      f4.y0 += 1;
      f4.y1 += 1;
      f4.y2 += 1;
      f4.y3 += 1;
      if(f4.y2 > SCREEN_HEIGHT)
      {
	f4.y0 = 0;
	f4.y1 = 0;
	f4.y2 = 50;
	f4.y3 = 50;
      }
    }

    if(g_pad[0].third.bit.left == 0)
    {
      f4.x0 -= 1;
      f4.x1 -= 1;
      f4.x2 -= 1;
      f4.x3 -= 1;
      if(f4.x0 < 0)
      {
	f4.x0 = SCREEN_WIDTH - 50;
	f4.x1 = SCREEN_WIDTH;
	f4.x2 = SCREEN_WIDTH - 50;
	f4.x3 = SCREEN_WIDTH;
      }
    }

    display(&f4);
  }

  return 0;
}

void graphics()
{
  // within the BIOS, if the address 0xBFC7FF52 equals 'E', set it as PAL (1). Otherwise, set it as NTSC (0)
  switch(*(char *)0xbfc7ff52=='E')
  {
  case 'E':
    SetVideoMode(MODE_PAL); 
    break;
  default:
    SetVideoMode(MODE_NTSC); 
    break;	
  }

  GsInitGraph(SCREEN_WIDTH, SCREEN_HEIGHT, GsINTER|GsOFSGPU, 1, 0); // set the graphics mode resolutions. You may also try using 'GsNONINTER' (read LIBOVR46.PDF in PSYQ/DOCS for detailed information)
  GsDefDispBuff(0, 0, 0, SCREEN_HEIGHT); // set the top left coordinates of the two buffers in video memory
  
  FntLoad(960, 256); // load the font from the BIOS into VRAM/SGRAM
  SetDumpFnt(FntOpen(5, 20, 320, 240, 0, 512)); // screen X,Y | max text length X,Y | autmatic background clear 0,1 | max characters
}


void display(POLY_F4 *f4)
{
  GsClearDispArea(0,0,0);
  
  DrawPrim(f4);
  
  FntPrint("Controller Example\nBytes: %d %d %d %d", g_pad[0].first.byte.status, g_pad[0].second.byte, g_pad[0].third.byte, g_pad[0].fourth.byte);
  
  FntFlush(-1);
  DrawSync(0); // wait for all drawing to finish
  VSync(0); // wait for v_blank interrupt
  GsSwapDispBuff(); // flip the double buffers
    
}