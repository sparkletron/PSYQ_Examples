/*
*/

#include <stdlib.h>
#include <libgte.h>
#include <libgpu.h>
#include <libpad.h>
#include <libgs.h>
#include <libetc.h>
#include "SPRITE.h"
#include "sand.h"

#define SCREEN_WIDTH  320 // screen width
#define	SCREEN_HEIGHT 240 // screen height
#define OT_SIZE       2 //size of ordering table
#define DOUBLE_BUF    2

u_long __ramsize   = 0x00200000; // force 2 megabytes of RAM
u_long __stacksize = 0x00004000; // force 16 kilobytes of stack

extern unsigned char e_image[];
extern unsigned char e_sand[];

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

struct s_character
{
  SPRT sprite;
  DR_TPAGE tpage;
  struct s_timInfo timInfo;
};

struct s_background
{
  POLY_FT4 poly;
  struct s_timInfo timInfo;
};


void initEnv(struct s_environment *p_env);
void display(struct s_environment *p_env);
struct s_timInfo getTIMinfo(u_long *p_address);
void setBackground(struct s_environment *p_env, struct s_background *p_background, int pos);
void setCharacter(struct s_environment *p_env, struct s_character *p_character, int pos);
int animate(SPRT *p_sprite);

int main() 
{
  struct s_character character;
  struct s_background background;
  struct s_environment environment;
  
  //setup playstation hardware and buffers
  initEnv(&environment);
  
  //get TIM images first for textures
  character.timInfo = getTIMinfo((u_long *)e_image);
  background.timInfo = getTIMinfo((u_long *)e_sand);
  
  setCharacter(&environment, &character, 1);
  
  setBackground(&environment, &background, 0);


  while (1) // draw and display forever
  {
    display(&environment);
    animate(&character.sprite);
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
  
  FntPrint("Sprite Example");
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

void setBackground(struct s_environment *p_env, struct s_background *p_background, int pos)
{
  SetPolyFT4(&p_background->poly);
  
  p_background->poly.tpage = p_background->timInfo.tpage;
  p_background->poly.clut = p_background->timInfo.clut;
  setXYWH(&p_background->poly, 0, 0, 320, 240);
  setRGB0(&p_background->poly, 127, 127, 127);
  setUVWH(&p_background->poly, 0, 0, 160, 120);
  
  AddPrim(&p_env->buffer[p_env->prevBuff].ot[pos], &p_background->poly);
}

void setCharacter(struct s_environment *p_env, struct s_character *p_character, int pos)
{
  SetSprt(&p_character->sprite);
  
  SetDrawTPage(&p_character->tpage, 1, 0, p_character->timInfo.tpage);
  
  setXY0(&p_character->sprite, SCREEN_WIDTH / 2 - 32, SCREEN_HEIGHT / 2 - 32);
  setWH(&p_character->sprite, 64, 64);
  setRGB0(&p_character->sprite, 127,127,127);
  setUV0(&p_character->sprite, 0, 0);
  
  AddPrim(&p_env->buffer[p_env->prevBuff].ot[pos], &p_character->sprite);
  AddPrim(&p_env->buffer[p_env->prevBuff].ot[pos], &p_character->tpage);
}

int animate(SPRT *p_sprite)
{
  static int prevtime = 0;
  static xCell = 0;
  static yCell = 0;
  
  if(prevtime == 0 || ((VSync(-1) - prevtime) >= 2))
  {
    prevtime = VSync(-1);
    
    xCell = (xCell + 64) % 256;
    
    if(xCell == 0)
    {
      yCell = (yCell + 64) % 256;
    }
    
    setUV0(p_sprite, xCell, yCell);
  }
}