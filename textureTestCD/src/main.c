/*
 * Written By: John Convertino
 * 
 * Example of using textures loaded from the CD
 * 
 * Move using the D-PAD, change color by pressing X.
 * 
*/

#include <stdlib.h>
#include <libgte.h>
#include <libgpu.h>
#include <libpad.h>
#include <libetc.h>
#include <libgs.h>
#include <libds.h>

#define SCREEN_WIDTH  320 // screen width
#define	SCREEN_HEIGHT 240 // screen height
#define OT_SIZE       1 //size of ordering table
#define DOUBLE_BUF    2

u_long __ramsize   = 0x00200000; // force 2 megabytes of RAM
u_long __stacksize = 0x00004000; // force 16 kilobytes of stack

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

struct s_environment
{
  int currBuff;
  int prevBuff;
  int otSize;
  int bufSize;
  
  struct
  {
    unsigned long ot[OT_SIZE];
    DISPENV disp;
    DRAWENV draw;
  } buffer[DOUBLE_BUF];
  
  struct
  {
    struct s_gamePad one;
    struct s_gamePad two;
  } gamePad;
};

struct s_timInfo
{
  u_short tpage;
  u_short clut;
};

void initEnv(struct s_environment *p_env);
void display(struct s_environment *p_env);
u_long *loadTIMfromCD(char *p_path);
struct s_timInfo getTIMinfo(u_long *p_address);
void populateOT(struct s_environment *p_env, POLY_FT4 *p_primitive);
void movSqr(struct s_environment *p_env, POLY_FT4 *p_primitive);

int main() 
{
  u_long *timImage = NULL;
  POLY_FT4 primitive[OT_SIZE];
  struct s_environment environment;
  struct s_timInfo timInfo;
  
  initEnv(&environment); // setup the graphics (seen below)
  
  timImage = loadTIMfromCD("\\TIM\\YAKKO.TIM;1");
  
  timInfo = getTIMinfo(timImage);
  
  primitive[0].tpage = timInfo.tpage;
  primitive[0].clut = timInfo.clut;
  
  populateOT(&environment, primitive);

  while (1) // draw and display forever
  {
    display(&environment);
    movSqr(&environment, primitive);
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
  
  memcpy((u_char *)p_env->buffer[p_env->currBuff].ot, (u_char *)p_env->buffer[p_env->prevBuff].ot, OT_SIZE * sizeof(*(p_env->buffer[p_env->prevBuff].ot)));
  
  DrawOTag(p_env->buffer[p_env->currBuff].ot);
  
  FntPrint("Texture Example\nLoad From CD");
  FntFlush(-1);
}

u_long *loadTIMfromCD(char *p_path)
{
  int sizeSectors = 0;
  int numRemain = 0;
  int prevNumRemain = 0;
  u_char result = 0;
  
  DslFILE fileInfo;
  u_long *image = NULL;
  
  //CD init 
  DsInit();
  
  if(DsSearchFile(&fileInfo, p_path) <= 0)
  {
    printf("\nFILE SEARCH FAILED\n");
  }
  else
  {
    printf("\nFILE SEARCH SUCCESS\n");
  }
  
  sizeSectors = (fileInfo.size + 2047) / 2048;

  printf("\nSECTOR SIZE: %d %d", sizeSectors, fileInfo.size);
  
  image = malloc(sizeSectors * 2048);
  
  if(image == NULL)
  {
    printf("\nALLOCATION FAILED\n");
  }
  else
  {
    printf("\nMEMORY ALLOCATED\n");
  }
  
  DsRead(&fileInfo.pos, sizeSectors, image, DslModeSpeed);
  
  do
  {
    numRemain = DsReadSync(&result);
    
    if(numRemain != prevNumRemain)
    {
      printf("\nNUM REMAIN: %d\n", numRemain);
      prevNumRemain = numRemain;
    }
  }
  while(numRemain);

  printf("\nREAD COMPLETE\n");
  
  DsClose();
  
  return image;
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

void populateOT(struct s_environment *p_env, POLY_FT4 *p_primitive)
{
  int index;
  
  for(index = 0; index < p_env->otSize; index++)
  {
    SetPolyFT4(&p_primitive[index]);
    setRGB0(&p_primitive[index], 127, 127, 127);
    setXYWH(&p_primitive[index], 0, 0, 50, 50);
    setUVWH(&p_primitive[index], 0, 0, 50, 50);
    AddPrim(&(p_env->buffer[p_env->prevBuff].ot[index]), &p_primitive[index]);
  }
}

void movSqr(struct s_environment *p_env, POLY_FT4 *p_primitive)
{
  static int prevTime = 0;
  static int primNum = 0;
  
  if(p_env->gamePad.one.fourth.bit.ex == 0)
  {
    if(prevTime == 0 || ((VSync(-1) - prevTime) > 60))
    {
      p_primitive[primNum].r0 = rand() % 256;
      p_primitive[primNum].g0 = rand() % 256;
      p_primitive[primNum].b0 = rand() % 256;
      prevTime = VSync(-1);
    }
  }
  
  if(p_env->gamePad.one.fourth.bit.circle == 0)
  {
    if(prevTime == 0 || ((VSync(-1) - prevTime) > 60))
    {
      primNum = (primNum + 1) % OT_SIZE;
      prevTime = VSync(-1);
    }
  }
  
  if(p_env->gamePad.one.third.bit.up == 0)
  {
    if(p_primitive[primNum].y0 > 0)
    {
      p_primitive[primNum].y0 -= 1;
      p_primitive[primNum].y1 -= 1;
      p_primitive[primNum].y2 -= 1;
      p_primitive[primNum].y3 -= 1;
    }
  }
  
  if(p_env->gamePad.one.third.bit.right == 0)
  {
    if(p_primitive[primNum].x1 < SCREEN_WIDTH)
    {
      p_primitive[primNum].x0 += 1;
      p_primitive[primNum].x1 += 1;
      p_primitive[primNum].x2 += 1;
      p_primitive[primNum].x3 += 1;
    }
  }
  
  if(p_env->gamePad.one.third.bit.down == 0)
  {
    if(p_primitive[primNum].y2 < SCREEN_HEIGHT)
    {
      p_primitive[primNum].y0 += 1;
      p_primitive[primNum].y1 += 1;
      p_primitive[primNum].y2 += 1;
      p_primitive[primNum].y3 += 1;
    }
  }
  
  if(p_env->gamePad.one.third.bit.left == 0)
  {
    if(p_primitive[primNum].x0 > 0)
    {
      p_primitive[primNum].x0 -= 1;
      p_primitive[primNum].x1 -= 1;
      p_primitive[primNum].x2 -= 1;
      p_primitive[primNum].x3 -= 1;
    }
  }
}