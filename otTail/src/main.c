/*
*/

#include <stdlib.h>
#include <libgte.h>
#include <libgpu.h>
#include <libpad.h>
#include <libetc.h>
#include <libmath.h>

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
void initEnv(struct s_environment *p_env, int numBuf, POLY_G4 *prim, int len);
void movSqr(POLY_G4 *primitive, int len);
void movUp(POLY_G4 *primitive, int len);
void movDown(POLY_G4 *primitive, int len);
void movLeft(POLY_G4 *primitive, int len);
void movRight(POLY_G4 *primitive, int len);

int main() 
{
  POLY_G4 primitive[OT_SIZE];
  struct s_environment environment[DUB_BUFFER];
  
  graphics(environment); // setup the graphics (seen below)
  
  FntLoad(960, 256); // load the font from the BIOS into VRAM/SGRAM
  SetDumpFnt(FntOpen(5, 20, 320, 240, 0, 512)); // screen X,Y | max text length X,Y | autmatic background clear 0,1 | max characters

  PadInitDirect((u_char *)&g_pad[0], (u_char *)&g_pad[1]);
  PadStartCom();

  initEnv(environment, DUB_BUFFER, primitive, OT_SIZE);

  while (1) // draw and display forever
  {
    display(environment);
    movSqr(primitive, OT_SIZE);
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

void initEnv(struct s_environment *p_env, int numBuf, POLY_G4 *prim, int len)
{ 
  int index;
  
  for(index = 0; index < numBuf; index++)
  {
      ClearOTag(p_env[index].ot, len);
  }
  
  for(index = 0; index < len; index++)
  {
    SetPolyG4(&prim[index]);
    setRGB0(&prim[index], rand() % 256, rand() % 256, rand() % 256);
    setRGB1(&prim[index], rand() % 256, rand() % 256, rand() % 256);
    setRGB2(&prim[index], rand() % 256, rand() % 256, rand() % 256);
    setRGB3(&prim[index], rand() % 256, rand() % 256, rand() % 256);
    setXYWH(&prim[index], SCREEN_WIDTH / 2 - 25, SCREEN_HEIGHT / 2 - 25, 50, 50);
    AddPrim(&(p_env[0].ot[index]), &prim[index]);
  }
  
  for(index = 0; index < numBuf; index++)
  {
    memcpy((u_char *)p_env[index].ot, (u_char *)p_env[0].ot, len * sizeof(*(p_env[0].ot)));
  }
}

void movSqr(POLY_G4 *primitive, int len)
{
  int index;
  
  if(g_pad[0].third.bit.up == 0 && g_pad[0].third.bit.right == 0)
  {
    movUp(primitive, len);
    movRight(primitive, len);
  }
  else if(g_pad[0].third.bit.up == 0 && g_pad[0].third.bit.left == 0)
  {
    movUp(primitive, len);
    movLeft(primitive, len);
  }
  else if(g_pad[0].third.bit.down == 0 && g_pad[0].third.bit.right == 0)
  {
    movDown(primitive, len);
    movRight(primitive, len);
  }
  else if(g_pad[0].third.bit.down == 0 && g_pad[0].third.bit.left == 0)
  {
    movDown(primitive, len);
    movLeft(primitive, len);
  }
  else if(g_pad[0].third.bit.up == 0)
  {
    movUp(primitive, len);
  }
  else if(g_pad[0].third.bit.right == 0)
  {
    movRight(primitive, len);
  }
  else if(g_pad[0].third.bit.down == 0)
  {
    movDown(primitive, len);
  }
  else if(g_pad[0].third.bit.left == 0)
  {
    movLeft(primitive, len);
  }
  else
  {
    for(index = 0; index < 2; index++)
    {
      if(primitive[0].x0 > SCREEN_WIDTH / 2 - 25)
      {
	movLeft(primitive, len);
      }
      
      if(primitive[0].x0 < SCREEN_WIDTH / 2 - 25)
      {
	movRight(primitive, len);
      }
      
      if(primitive[0].y0 < SCREEN_HEIGHT / 2 - 25)
      {
	movDown(primitive, len);
      }
      
      if(primitive[0].y0 > SCREEN_HEIGHT / 2 -25)
      {
	movUp(primitive, len);
      }
    }
  }
}

void movUp(POLY_G4 *primitive, int len)
{
  int index;
  if(primitive[0].y0 > 0)
  {
    for(index = len - 2; index >= 0; index--)
    {
      primitive[index].y0 -= 2 * (len - index - 1);
      primitive[index].y1 -= 2 * (len - index - 1);
      primitive[index].y2 -= 2 * (len - index - 1);
      primitive[index].y3 -= 2 * (len - index - 1);
    }
  }
}

void movDown(POLY_G4 *primitive, int len)
{
  int index;
  if(primitive[0].y2 < SCREEN_HEIGHT)
  {
    for(index = len - 2; index >= 0; index--)
    {
      primitive[index].y0 += 2 * (len - index - 1);
      primitive[index].y1 += 2 * (len - index - 1);
      primitive[index].y2 += 2 * (len - index - 1);
      primitive[index].y3 += 2 * (len - index - 1);
    }
  }
}

void movLeft(POLY_G4 *primitive, int len)
{
  int index;
  if(primitive[0].x0 > 0)
  {
    for(index = len - 2; index >= 0; index--)
    {
      primitive[index].x0 -= 2 * (len - index - 1);
      primitive[index].x1 -= 2 * (len - index - 1);
      primitive[index].x2 -= 2 * (len - index - 1);
      primitive[index].x3 -= 2 * (len - index - 1);
    }
  }
}

void movRight(POLY_G4 *primitive, int len)
{
  int index;
  if(primitive[0].x1 < SCREEN_WIDTH)
  {
    for(index = len - 2; index >= 0; index--)
    {
      primitive[index].x0 += 2 * (len - index - 1);
      primitive[index].x1 += 2 * (len - index - 1);
      primitive[index].x2 += 2 * (len - index - 1);
      primitive[index].x3 += 2 * (len - index - 1);
    }
  }
}