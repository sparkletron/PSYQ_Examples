/*
 * Written By: John Convertino
 * 
 * Write the phrase "I LOVE YOU" to a memory card, use memcardRead example to read and display the string.
 * 
*/

#include <stdlib.h>
#include <libgte.h>
#include <libgpu.h>
#include <libpad.h>
#include <libetc.h>
#include <libmcrd.h>
#include <sys/file.h>
#include <libapi.h>

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

char *g_phrase = "I LOVE YOU";

void initEnv(struct s_environment *p_env);
void display(struct s_environment *p_env);
void memoryCardWrite(char *p_phrase);

int main() 
{
  struct s_environment environment;
  
  initEnv(&environment); // setup the graphics (seen below)
  
  memoryCardWrite(g_phrase);

  while (1) // draw and display forever
  {
    display(&environment);
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
  
  FntPrint("Memory Card Write\nWrote: %s", g_phrase);
  FntFlush(-1);
}

void memoryCardWrite(char *p_phrase)
{
  long cmds;
  long result;
  
  PadStopCom();
    
  MemCardInit(1);

  MemCardStart();

  if(MemCardSync(0, &cmds, &result) <= 0)
  {
    printf("\nSync Failed\n");
  }

  MemCardAccept(0);

  if(MemCardSync(0, &cmds, &result) <= 0)
  {
    printf("\nSync Failed\n");
  }

  if(MemCardOpen(0, "test", O_WRONLY) != 0)
  {
    printf("\nOpen Issue\n");
  }

  if(MemCardWriteData((unsigned long *)p_phrase, 0, 128)  != 0)
  {
    printf("\nWrite Issue\n");
  }

  if(MemCardSync(0, &cmds, &result) <= 0)
  {
    printf("\nSync Failed\n");
  }

  MemCardClose();

  MemCardStop();
  
  PadStartCom();
}