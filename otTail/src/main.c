/*
*/

#include <stdlib.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <libpad.h>
//#include <libetc.h>

#define SCREEN_WIDTH  320 // screen width
#define	SCREEN_HEIGHT 240 // screen height

u_long __ramsize   = 0x00200000; // force 2 megabytes of RAM
u_long __stacksize = 0x00004000; // force 16 kilobytes of stack

void graphics();
void display();
void initOTwPrim(POLY_F4 *prim, int len);

unsigned long g_ot[6];

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
  POLY_F4 primArray[6];
  int prevTime = 0;
  int primitive = 0;
  
  graphics(); // setup the graphics (seen below)
  FntLoad(960, 256); // load the font from the BIOS into VRAM/SGRAM
  SetDumpFnt(FntOpen(5, 20, 320, 240, 0, 512)); // screen X,Y | max text length X,Y | autmatic background clear 0,1 | max characters
  
  ClearOTag(g_ot, 6);

  PadInitDirect((u_char *)&g_pad[0], (u_char *)&g_pad[1]);
  PadStartCom();

  initOTwPrim(primArray, 6);

  while (1) // draw and display forever
  {
    if(g_pad[0].fourth.bit.ex == 0)
    {
      if(prevTime == 0 || ((VSync(-1) - prevTime) > 60))
      {
	primArray[primitive].r0 = rand() % 256;
	primArray[primitive].g0 = rand() % 256;
	primArray[primitive].b0 = rand() % 256;
	prevTime = VSync(-1);
      }
    }
    else if(g_pad[0].fourth.bit.circle == 0)
    {
      if(prevTime == 0 || ((VSync(-1) - prevTime) > 60))
      {
	primitive = (primitive + 1) % 6;
	prevTime = VSync(-1);
      }
    }
    else if(g_pad[0].third.bit.up == 0)
    {
      printf("\nUp: %d\n", primArray[primitive].y0);
      if(primArray[primitive].y0 > 0)
      {
	primArray[primitive].y0 -= 1;
	primArray[primitive].y1 -= 1;
	primArray[primitive].y2 -= 1;
	primArray[primitive].y3 -= 1;
      }
    }
    else if(g_pad[0].third.bit.right == 0)
    {
      printf("\nRight %d\n", primArray[primitive].x1);
      if(primArray[primitive].x1 < SCREEN_WIDTH)
      {
	primArray[primitive].x0 += 1;
	primArray[primitive].x1 += 1;
	primArray[primitive].x2 += 1;
	primArray[primitive].x3 += 1;
      }

    }
    else if(g_pad[0].third.bit.down == 0)
    {
      printf("\nDown %d\n", primArray[primitive].y2);
      if(primArray[primitive].y2 < SCREEN_HEIGHT)
      {
	primArray[primitive].y0 += 1;
	primArray[primitive].y1 += 1;
	primArray[primitive].y2 += 1;
	primArray[primitive].y3 += 1;
      }
    }
    else if(g_pad[0].third.bit.left == 0)
    {
      printf("\nLeft %d\n", primArray[primitive].x0);
      if(primArray[primitive].x0 > 0)
      {
	primArray[primitive].x0 -= 1;
	primArray[primitive].x1 -= 1;
	primArray[primitive].x2 -= 1;
	primArray[primitive].x3 -= 1;
      }

    }

    display();
  }

  return 0;
}

void graphics()
{
  // within the BIOS, if the address 0xBFC7FF52 equals 'E', set it as PAL (1). Otherwise, set it as NTSC (0)
  switch(*(char *)0xbfc7ff52=='E')
  {
	  case 'E':
		  SetVideoMode(1); 
		  break;
	  default:
		  SetVideoMode(0); 
		  break;	
  }

  GsInitGraph(SCREEN_WIDTH, SCREEN_HEIGHT, GsINTER|GsOFSGPU, 1, 0); // set the graphics mode resolutions. You may also try using 'GsNONINTER' (read LIBOVR46.PDF in PSYQ/DOCS for detailed information)
  GsDefDispBuff(0, 0, 0, SCREEN_HEIGHT); // set the top left coordinates of the two buffers in video memory
}


void display()
{
  GsClearDispArea(0,0,0);
  DrawOTag(g_ot);
  FntPrint("Ordering Table Example");
  FntFlush(-1);
  DrawSync(0); // wait for all drawing to finish
  VSync(0); // wait for v_blank interrupt
  GsSwapDispBuff(); // flip the double buffers
}

void initOTwPrim(POLY_F4 *prim, int len)
{
  int index;
  for(index = 0; index < len; index++)
  {
    SetPolyF4(&prim[index]);
    setRGB0(&prim[index], rand() % 256, 127, rand() % 256);
    setXY4(&prim[index], 0, 0, 120 / (index + 1), 0, 0, 120 / (index + 1), 120 / (index + 1), 120 / (index + 1));
    AddPrim(&g_ot[index], &prim[index]);
  }
}