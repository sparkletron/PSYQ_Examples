
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

void initEnv(struct s_environment *p_env);
void display(struct s_environment *p_env);
void populateOT(struct s_environment *p_env, POLY_G4 *p_primitive);
void movSqr(struct s_environment *p_env, POLY_G4 *p_primitive);
void movUp(POLY_G4 *p_primitive, int len);
void movDown(POLY_G4 *p_primitive, int len);
void movLeft(POLY_G4 *p_primitive, int len);
void movRight(POLY_G4 *p_primitive, int len);

int main() 
{
  POLY_G4 primitive[OT_SIZE];
  struct s_environment environment;
  
  initEnv(&environment); // setup the graphics (seen below)
  
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
  
  FntPrint("Ordering Table Example\nMoving Square");
  FntFlush(-1);
}

void populateOT(struct s_environment *p_env, POLY_G4 *p_primitive)
{
  int index;
  
  for(index = 0; index < p_env->otSize; index++)
  {
    SetPolyG4(&p_primitive[index]);
    setRGB0(&p_primitive[index], rand() % 256, rand() % 256, rand() % 256);
    setRGB1(&p_primitive[index], rand() % 256, rand() % 256, rand() % 256);
    setRGB2(&p_primitive[index], rand() % 256, rand() % 256, rand() % 256);
    setRGB3(&p_primitive[index], rand() % 256, rand() % 256, rand() % 256);
    setXYWH(&p_primitive[index], SCREEN_WIDTH / 2 - 25, SCREEN_HEIGHT / 2 - 25, 50, 50);
    AddPrim(&(p_env->buffer[p_env->prevBuff].ot[index]), &p_primitive[index]);
  }
}

void movSqr(struct s_environment *p_env, POLY_G4 *p_primitive)
{
  int index;
  
  if(p_env->gamePad.one.third.bit.up == 0)
  {
    movUp(p_primitive, p_env->otSize);
  }
  
  if(p_env->gamePad.one.third.bit.left == 0)
  {
    movLeft(p_primitive, p_env->otSize);
  }
  
  if(p_env->gamePad.one.third.bit.down == 0)
  {
    movDown(p_primitive, p_env->otSize);
  }

  if(p_env->gamePad.one.third.bit.right == 0)
  {
    movRight(p_primitive, p_env->otSize);
  }
  
  if(p_env->gamePad.one.third.byte == 0xFF)
  {
    for(index = 0; index < 2; index++)
    {
      if(p_primitive[0].x0 > SCREEN_WIDTH / 2 - 25)
      {
	movLeft(p_primitive, p_env->otSize);
      }
      
      if(p_primitive[0].x0 < SCREEN_WIDTH / 2 - 25)
      {
	movRight(p_primitive, p_env->otSize);
      }
      
      if(p_primitive[0].y0 < SCREEN_HEIGHT / 2 - 25)
      {
	movDown(p_primitive, p_env->otSize);
      }
      
      if(p_primitive[0].y0 > SCREEN_HEIGHT / 2 -25)
      {
	movUp(p_primitive, p_env->otSize);
      }
    }
  }
}

void movUp(POLY_G4 *p_primitive, int len)
{
  int index;
  if(p_primitive[0].y0 > 0)
  {
    for(index = len - 2; index >= 0; index--)
    {
      p_primitive[index].y0 -= 2 * (len - index - 1);
      p_primitive[index].y1 -= 2 * (len - index - 1);
      p_primitive[index].y2 -= 2 * (len - index - 1);
      p_primitive[index].y3 -= 2 * (len - index - 1);
    }
  }
}

void movDown(POLY_G4 *p_primitive, int len)
{
  int index;
  if(p_primitive[0].y2 < SCREEN_HEIGHT)
  {
    for(index = len - 2; index >= 0; index--)
    {
      p_primitive[index].y0 += 2 * (len - index - 1);
      p_primitive[index].y1 += 2 * (len - index - 1);
      p_primitive[index].y2 += 2 * (len - index - 1);
      p_primitive[index].y3 += 2 * (len - index - 1);
    }
  }
}

void movLeft(POLY_G4 *p_primitive, int len)
{
  int index;
  if(p_primitive[0].x0 > 0)
  {
    for(index = len - 2; index >= 0; index--)
    {
      p_primitive[index].x0 -= 2 * (len - index - 1);
      p_primitive[index].x1 -= 2 * (len - index - 1);
      p_primitive[index].x2 -= 2 * (len - index - 1);
      p_primitive[index].x3 -= 2 * (len - index - 1);
    }
  }
}

void movRight(POLY_G4 *p_primitive, int len)
{
  int index;
  if(p_primitive[0].x1 < SCREEN_WIDTH)
  {
    for(index = len - 2; index >= 0; index--)
    {
      p_primitive[index].x0 += 2 * (len - index - 1);
      p_primitive[index].x1 += 2 * (len - index - 1);
      p_primitive[index].x2 += 2 * (len - index - 1);
      p_primitive[index].x3 += 2 * (len - index - 1);
    }
  }
}