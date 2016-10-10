/*
 * Written By: John Convertino
 * 
 * Example of using a texture on a primitive.
 * 
 * Move square with D-Pad, press X to change color.
 * 
*/

#include <stdlib.h>
#include <libgte.h>
#include <libgpu.h>
#include <libpad.h>
#include <libetc.h>
#include <libgs.h>
#include "image.h"

#define SCREEN_WIDTH  320 // screen width
#define	SCREEN_HEIGHT 240 // screen height
#define OT_SIZE       2 //size of ordering table
#define DOUBLE_BUF    2

u_long __ramsize   = 0x00200000; // force 2 megabytes of RAM
u_long __stacksize = 0x00004000; // force 16 kilobytes of stack

extern unsigned char e_image[];

struct s_gamePad
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
      u_char na2:1;
      u_char na1:1;
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
      u_char l2:1;
      u_char r2:1;
      u_char l1:1;
      u_char r1:1;
      u_char triangle:1;
      u_char circle:1;
      u_char ex:1;
      u_char square:1;
    } bit;
    u_char byte:8;
	  
  } fourth;
};

struct s_timInfo
{
  u_short tpage;
  u_short clut;
};

struct s_environment
{
  int currBuff;
  int prevBuff;
  int otSize;
  int bufSize;
  int px[OT_SIZE];
  int py[OT_SIZE];
  int pw[OT_SIZE];
  int ph[OT_SIZE];
  int r0[OT_SIZE];
  int g0[OT_SIZE];
  int b0[OT_SIZE];
  
  struct
  {
    POLY_FT4 primitive[OT_SIZE];
    unsigned long ot[OT_SIZE];
    DISPENV disp;
    DRAWENV draw;
  } buffer[DOUBLE_BUF];
  
  struct
  {
    struct s_gamePad one;
    struct s_gamePad two;
  } gamePad;
  
  struct s_timInfo timInfo[OT_SIZE];
};

void initEnv(struct s_environment *p_env);
void display(struct s_environment *p_env);
struct s_timInfo getTIMinfo(u_long *p_address);
void populateTPage(struct s_environment *p_env, u_long *p_address[], int len);
void populateOT(struct s_environment *p_env);
void movSqr(struct s_environment *p_env);

int main() 
{
  u_long *p_address[] = {(u_long *)e_image, (u_long *)e_image};
  struct s_environment environment;

  initEnv(&environment); // setup the graphics (seen below)
  
  populateTPage(&environment, p_address, environment.otSize);
  
  populateOT(&environment);

  while (1) // draw and display forever
  {
    display(&environment);
    movSqr(&environment);
  }

  return 0;
}

void initEnv(struct s_environment *p_env)
{
  int index;
  p_env->bufSize = DOUBLE_BUF;
  p_env->otSize = OT_SIZE;
  
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

  for(index = 0; index < p_env->bufSize; index += 2) 
  {
    SetDefDispEnv(&p_env->buffer[index].disp, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    SetDefDrawEnv(&p_env->buffer[index].draw, 0, SCREEN_HEIGHT, SCREEN_WIDTH, SCREEN_HEIGHT);
  }

  for(index = 1; index < p_env->bufSize; index += 2)
  {
    SetDefDispEnv(&p_env->buffer[index].disp, 0, SCREEN_HEIGHT, SCREEN_WIDTH, SCREEN_HEIGHT);
    SetDefDrawEnv(&p_env->buffer[index].draw, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
  }

  for(index = 0; index < p_env->bufSize; index++)
  {
    p_env->buffer[index].draw.isbg = 1;
    p_env->buffer[index].draw.r0 = 0;
    p_env->buffer[index].draw.g0 = 0;
    p_env->buffer[index].draw.b0 = 0;
    
    ClearOTag(p_env->buffer[index].ot, p_env->otSize);
  }
  
  p_env->prevBuff = 0;
  p_env->currBuff = 0;
  
  FntLoad(960, 256); // load the font from the BIOS into VRAM/SGRAM
  SetDumpFnt(FntOpen(5, 20, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 512)); // screen X,Y | max text length X,Y | autmatic background clear 0,1 | max characters
  
  PadInitDirect((u_char *)&p_env->gamePad.one, (u_char *)&p_env->gamePad.two);
  PadStartCom();
  
  SetDispMask(1); 
}


void display(struct s_environment *p_env)
{

  p_env->prevBuff = p_env->currBuff;
  
  //avoid issues with delayed execution
  while(DrawSync(1));
  VSync(0);
  
  p_env->currBuff = (p_env->currBuff + 1) % p_env->bufSize;
  
  PutDrawEnv(&p_env->buffer[p_env->currBuff].draw);
  PutDispEnv(&p_env->buffer[p_env->currBuff].disp);
  
  DrawOTag(p_env->buffer[p_env->currBuff].ot);
  
  FntPrint("Texture Example\nLoad From Header");
  FntFlush(-1);
}

struct s_timInfo getTIMinfo(u_long *p_address)
{
  struct s_timInfo timInfo;
  GsIMAGE timData;
  
  GsGetTimInfo(p_address+1, &timData);
  
  timInfo.tpage = LoadTPage(timData.pixel, timData.pmode, 0, timData.px, timData.py, timData.pw, timData.ph);
  timInfo.clut = LoadClut(timData.clut, timData.cx, timData.cy);
  
  return timInfo;
}

void populateTPage(struct s_environment *p_env, u_long *p_address[], int len)
{
  int index;
  int buffIndex;
  
  for(index = 0; (index < len) && (index < p_env->otSize); index++)
  {
    p_env->timInfo[index] = getTIMinfo(p_address[index]);
  }
  
  for(buffIndex = 0; buffIndex < p_env->bufSize; buffIndex++)
  {
    for(index = 0; (index < len) && (index < p_env->otSize); index++)
    {
      p_env->buffer[buffIndex].primitive[index].tpage = p_env->timInfo[index].tpage;
      p_env->buffer[buffIndex].primitive[index].clut = p_env->timInfo[index].clut;
    }
  }
  
}

void populateOT(struct s_environment *p_env)
{
  int index;
  int buffIndex; 
  
  for(buffIndex = 0; buffIndex < p_env->bufSize; buffIndex++)
  {
    for(index = 0; index < p_env->otSize; index++)
    {
      p_env->px[index] = 0;
      p_env->py[index] = 0;
      p_env->pw[index] = 50;
      p_env->ph[index] = 50;
      p_env->r0[index] = 127;
      p_env->g0[index] = 127;
      p_env->b0[index] = 127;
      
      SetPolyFT4(&p_env->buffer[buffIndex].primitive[index]);
      setRGB0(&p_env->buffer[buffIndex].primitive[index], p_env->r0[index], p_env->g0[index], p_env->b0[index]);
      setXYWH(&p_env->buffer[buffIndex].primitive[index], p_env->px[index], p_env->py[index], p_env->pw[index], p_env->ph[index]);
      setUVWH(&p_env->buffer[buffIndex].primitive[index], p_env->px[index], p_env->py[index], p_env->pw[index], p_env->ph[index]);
      AddPrim(&(p_env->buffer[buffIndex].ot[index]), &(p_env->buffer[buffIndex].primitive[index]));
    }
  }
}

void movSqr(struct s_environment *p_env)
{
  static int prevTime = 0;
  static int primNum = 0;
  
  if(p_env->gamePad.one.fourth.bit.ex == 0)
  {
    if(prevTime == 0 || ((VSync(-1) - prevTime) > 60))
    {
      p_env->r0[primNum] = rand() % 256;
      p_env->g0[primNum] = rand() % 256;
      p_env->b0[primNum] = rand() % 256;
      prevTime = VSync(-1);
    }
  }
  
  if(p_env->gamePad.one.fourth.bit.circle == 0)
  {
    if(prevTime == 0 || ((VSync(-1) - prevTime) > 60))
    {
      primNum = (primNum + 1) % p_env->otSize;
      prevTime = VSync(-1);
    }
  }
  
  if(p_env->gamePad.one.third.bit.up == 0)
  {
    if(p_env->py[primNum] > 0)
    {
      p_env->py[primNum] -= 1;
    }
  }
  
  if(p_env->gamePad.one.third.bit.right == 0)
  {
    if((p_env->px[primNum] + 50) < SCREEN_WIDTH)
    {
      p_env->px[primNum] += 1;
    }
  }
  
  if(p_env->gamePad.one.third.bit.down == 0)
  {
    if((p_env->py[primNum] + 50) < SCREEN_HEIGHT)
    {
      p_env->py[primNum] += 1;
    }
  }
  
  if(p_env->gamePad.one.third.bit.left == 0)
  {
    if(p_env->px[primNum] > 0)
    {
      p_env->px[primNum] -= 1;
    }
  }
  
  setRGB0(&p_env->buffer[p_env->prevBuff].primitive[primNum], p_env->r0[primNum], p_env->g0[primNum], p_env->b0[primNum]);
  setXYWH(&p_env->buffer[p_env->prevBuff].primitive[primNum], p_env->px[primNum], p_env->py[primNum], p_env->pw[primNum], p_env->ph[primNum]);
}