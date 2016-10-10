#include "engine.h"

u_long __ramsize = 0x00200000;  //force 2 megabytes of RAM
u_long __stacksize = 0x00004000; //force 16 kilobytes of stack

void initEnv(struct s_environment *p_env)
{
  int index;
  
  p_env->bufSize = DOUBLE_BUF;
  p_env->otSize = OT_SIZE;
  p_env->curPrim = 0;
  p_env->prevPrim = 0;
  p_env->prevTime = 0;
  
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

void updatePrim(struct s_environment *p_env)
{
  if(p_env->curPrim != p_env->prevPrim)
  {
    setRGB0(&p_env->buffer[p_env->prevBuff].primitive[p_env->prevPrim], p_env->r0[p_env->prevPrim], p_env->g0[p_env->prevPrim], p_env->b0[p_env->prevPrim]);
    setXYWH(&p_env->buffer[p_env->prevBuff].primitive[p_env->prevPrim], p_env->px[p_env->prevPrim], p_env->py[p_env->prevPrim], p_env->pw[p_env->prevPrim], p_env->ph[p_env->prevPrim]);
    p_env->prevPrim = p_env->curPrim;
  }
  else
  {
    setRGB0(&p_env->buffer[p_env->prevBuff].primitive[p_env->curPrim], p_env->r0[p_env->curPrim], p_env->g0[p_env->curPrim], p_env->b0[p_env->curPrim]);
    setXYWH(&p_env->buffer[p_env->prevBuff].primitive[p_env->curPrim], p_env->px[p_env->curPrim], p_env->py[p_env->curPrim], p_env->pw[p_env->curPrim], p_env->ph[p_env->prevPrim]);
  }
}

 
