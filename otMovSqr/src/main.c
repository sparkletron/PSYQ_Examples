/*
*/

#include <stdlib.h>
#include <libgte.h>
#include <libgpu.h>
#include <libpad.h>
#include <libetc.h>

#define SCREEN_WIDTH  320 // screen width
#define	SCREEN_HEIGHT 240 // screen height
#define OT_SIZE       6 //size of ordering table
#define DUB_BUFFER    2
u_long __ramsize   = 0x00200000; // force 2 megabytes of RAM
u_long __stacksize = 0x00004000; // force 16 kilobytes of stack

struct s_environment{
  unsigned long ot[OT_SIZE];
  DISPENV disp;
  DRAWENV draw;
};

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

void graphics(struct s_environment *p_env);
void display(struct s_environment *p_env);
void initEnv(struct s_environment *p_env, int numBuf, POLY_F4 *prim, int len);

int main() 
{
  POLY_F4 primArray[OT_SIZE];
  int prevTime = 0;
  int primitive = 0;
  
  struct s_environment environment[DUB_BUFFER];
  
  graphics(environment); // setup the graphics (seen below)
  
  FntLoad(960, 256); // load the font from the BIOS into VRAM/SGRAM
  SetDumpFnt(FntOpen(5, 20, 320, 240, 0, 512)); // screen X,Y | max text length X,Y | autmatic background clear 0,1 | max characters

  PadInitDirect((u_char *)&g_pad[0], (u_char *)&g_pad[1]);
  PadStartCom();

  initEnv(environment, DUB_BUFFER, primArray, OT_SIZE);

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
	primitive = (primitive + 1) % OT_SIZE;
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

    display(environment);
  }

  return 0;
}

void graphics(struct s_environment *p_env)
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
  
  ResetGraph(0);
  
  SetDefDispEnv(&p_env[0].disp, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
  SetDefDispEnv(&p_env[1].disp, 0, SCREEN_HEIGHT, SCREEN_WIDTH, SCREEN_HEIGHT);
  SetDefDrawEnv(&p_env[0].draw, 0, SCREEN_HEIGHT, SCREEN_WIDTH, SCREEN_HEIGHT);
  SetDefDrawEnv(&p_env[1].draw, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
  
  p_env[0].draw.isbg = 1;
  p_env[0].draw.r0 = 0;
  p_env[0].draw.g0 = 0;
  p_env[0].draw.b0 = 0;
  
  p_env[1].draw.isbg = 1;
  p_env[1].draw.r0 = 0;
  p_env[1].draw.g0 = 0;
  p_env[1].draw.b0 = 0;
  
  SetDispMask(1); 
}


void display(struct s_environment *p_env)
{
  static int currBuff = 0;

  DrawSync(0);
  VSync(0);
  currBuff = (currBuff + 1) % DUB_BUFFER;
  PutDrawEnv(&p_env[currBuff].draw);
  PutDispEnv(&p_env[currBuff].disp);
  DrawOTag(p_env[currBuff].ot);
  FntPrint("Ordering Table Example");
  FntFlush(-1);
}

void initEnv(struct s_environment *p_env, int numBuf, POLY_F4 *prim, int len)
{ 
  int index;
  
  for(index = 0; index < numBuf; index++)
  {
      ClearOTag(p_env[index].ot, len);
  }
  
  for(index = 0; index < len; index++)
  {
    SetPolyF4(&prim[index]);
    setRGB0(&prim[index], rand() % 256, rand() % 256, rand() % 256);
    setXY4(&prim[index], 0, 0, 240 / (index + 1), 0, 0, 240 / (index + 1), 240 / (index + 1), 240 / (index + 1));
    AddPrim(&(p_env[0].ot[index]), &prim[index]);
  }
  
  for(index = 0; index < numBuf; index++)
  {
    memcpy((u_char *)p_env[index].ot, (u_char *)p_env[0].ot, len * sizeof(*(p_env[0].ot)));
  }
}