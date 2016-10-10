/*
 * Written By: John Convertino
 * 
 * Primitive engine, simple methods for getting primitives on screen, along with some other neat features.
 * 
 * Can Do:
 * 	-Memory Card Access
 * 	-Texture From CD
 * 	-Sprites, Tiles, and all polys.
 *	-Add textures to correct type 
 * 	-Game Pad
 * 
 * Can NOT Do:
 * 	-3D
 * 	-Sound
 * 
 * Version: 1.0
 * 
*/

#include "engine.h"

u_long __ramsize = 0x00200000;  //force 2 megabytes of RAM
u_long __stacksize = 0x00004000; //force 16 kilobytes of stack

//utility functions
void swapBuffers(struct s_environment *p_env)
{
  struct s_buffer *tempBuffer = p_env->p_drawBuffer;
  
  p_env->p_drawBuffer = p_env->p_regBuffer;
  
  p_env->p_regBuffer = tempBuffer;
}

//available functions
void initEnv(struct s_environment *p_env, int numPrim)
{
  int index;
  
  p_env->bufSize = DOUBLE_BUF;
  p_env->otSize = (numPrim < 1 ? 1 : numPrim);
  p_env->curPrim = 0;
  p_env->prevTime = 0;
  
  for(index = 0; index < p_env->bufSize; index++)
  {
    p_env->buffer[index].p_primitive = calloc(p_env->otSize, sizeof(struct s_primitive));
    p_env->buffer[index].p_ot = calloc(p_env->otSize, sizeof(unsigned long));
  }
  
  p_env->p_primParam = calloc(p_env->otSize, sizeof(struct s_primParam));
  
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
    
    ClearOTag(p_env->buffer[index].p_ot, p_env->otSize);
  }
  
  p_env->p_drawBuffer = &p_env->buffer[0];
  p_env->p_regBuffer = &p_env->buffer[1];
  
  FntLoad(960, 256); // load the font from the BIOS into VRAM/SGRAM
  SetDumpFnt(FntOpen(5, 20, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 512)); // screen X,Y | max text length X,Y | autmatic background clear 0,1 | max characters
  
  PadInitDirect((u_char *)&p_env->gamePad.one, (u_char *)&p_env->gamePad.two);
  PadStartCom();
  
  SetDispMask(1); 
}


void display(struct s_environment *p_env)
{
  //avoid issues with delayed execution
  while(DrawSync(1));
  VSync(0);
  
  //exchange reg and draw buffer, so newly registered ot will be drawn, and used draw buffer can now be used for registration.
  swapBuffers(p_env);
  
  PutDrawEnv(&p_env->p_drawBuffer->draw);
  PutDispEnv(&p_env->p_drawBuffer->disp);
  
  DrawOTag(p_env->p_drawBuffer->p_ot);
  
  FntPrint("%s\n%s\n%X", p_env->envMessage.p_title, p_env->envMessage.p_message, *p_env->envMessage.p_data);
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
  
  timInfo.tpageID = LoadTPage(timData.pixel, timData.pmode, 0, timData.px, timData.py, timData.pw, timData.ph);
  timInfo.clutID = LoadClut(timData.clut, timData.cx, timData.cy);
  
  return timInfo;
}

void populateTPage(struct s_environment *p_env, u_long *p_address[], int len)
{
  int index;
  int buffIndex;
  
  for(index = 0; (index < len) && (index < p_env->otSize); index++)
  {
    if(p_address[index] != NULL)
    {
      p_env->p_primParam[index].timInfo = getTIMinfo(p_address[index]);
    }
  }
  
  for(buffIndex = 0; buffIndex < p_env->bufSize; buffIndex++)
  {
    for(index = 0; (index < len) && (index < p_env->otSize); index++)
    {
      p_env->buffer[buffIndex].p_primitive[index].type = p_env->p_primParam[index].type;
      
      switch(p_env->buffer[buffIndex].p_primitive[index].type)
      {
	case TYPE_FT4:
	  ((POLY_FT4 *)p_env->buffer[buffIndex].p_primitive[index].data)->tpage = p_env->p_primParam[index].timInfo.tpageID;
	  ((POLY_FT4 *)p_env->buffer[buffIndex].p_primitive[index].data)->clut = p_env->p_primParam[index].timInfo.clutID;
	  break;
	case TYPE_GT4:
	  ((POLY_GT4 *)p_env->buffer[buffIndex].p_primitive[index].data)->tpage = p_env->p_primParam[index].timInfo.tpageID;
	  ((POLY_GT4 *)p_env->buffer[buffIndex].p_primitive[index].data)->clut = p_env->p_primParam[index].timInfo.clutID;
	  break;
	case TYPE_SPRITE:
	  SetDrawTPage(&p_env->p_primParam[index].tpage, 1, 0, p_env->p_primParam[index].timInfo.tpageID);
	  AddPrim(&(p_env->buffer[buffIndex].p_ot[index]), &p_env->p_primParam[index].tpage);
	  break;
	default:
	  printf("\nNon Texture Type at index %d\n", index);
	  break;
      }
    }
  }
}

void populateOT(struct s_environment *p_env)
{
  int index;
  int buffIndex;
  
  for(index = 0; index < p_env->otSize; index++)
  {    
    for(buffIndex = 0; buffIndex < p_env->bufSize; buffIndex++)
    {
      p_env->buffer[buffIndex].p_primitive[index].type = p_env->p_primParam[index].type;
      
      switch(p_env->buffer[buffIndex].p_primitive[index].type)
      {
	case TYPE_SPRITE:
	  SetSprt((SPRT *)p_env->buffer[buffIndex].p_primitive[index].data);
	  setXY0((SPRT *)p_env->buffer[buffIndex].p_primitive[index].data, p_env->p_primParam[index].px, p_env->p_primParam[index].py);
	  setWH((SPRT *)p_env->buffer[buffIndex].p_primitive[index].data, p_env->p_primParam[index].pw,  p_env->p_primParam[index].ph);
	  setUV0((SPRT *)p_env->buffer[buffIndex].p_primitive[index].data, p_env->p_primParam[index].tx, p_env->p_primParam[index].ty);
	  setRGB0((SPRT *)p_env->buffer[buffIndex].p_primitive[index].data, p_env->p_primParam[index].r0, p_env->p_primParam[index].g0, p_env->p_primParam[index].b0);
	  break;
	case TYPE_TILE:
	  setTile((TILE *)p_env->buffer[buffIndex].p_primitive[index].data);
	  setXY0((TILE *)p_env->buffer[buffIndex].p_primitive[index].data, p_env->p_primParam[index].px, p_env->p_primParam[index].py);
	  setWH((TILE *)p_env->buffer[buffIndex].p_primitive[index].data, p_env->p_primParam[index].pw,  p_env->p_primParam[index].ph);
	  setRGB0((TILE *)p_env->buffer[buffIndex].p_primitive[index].data, p_env->p_primParam[index].r0, p_env->p_primParam[index].g0, p_env->p_primParam[index].b0);
	  break;
	case TYPE_F4:
	  SetPolyF4((POLY_F4 *)p_env->buffer[buffIndex].p_primitive[index].data);
	  setXYWH((POLY_F4 *)p_env->buffer[buffIndex].p_primitive[index].data, p_env->p_primParam[index].px, p_env->p_primParam[index].py, p_env->p_primParam[index].pw, p_env->p_primParam[index].ph);
	  setRGB0((POLY_F4 *)p_env->buffer[buffIndex].p_primitive[index].data, p_env->p_primParam[index].r0, p_env->p_primParam[index].g0, p_env->p_primParam[index].b0);
	  break;
	case TYPE_FT4:
	  SetPolyFT4((POLY_FT4 *)p_env->buffer[buffIndex].p_primitive[index].data);
	  setUVWH((POLY_FT4 *)p_env->buffer[buffIndex].p_primitive[index].data, p_env->p_primParam[index].tx, p_env->p_primParam[index].ty, p_env->p_primParam[index].tw, p_env->p_primParam[index].th);
	  setXYWH((POLY_FT4 *)p_env->buffer[buffIndex].p_primitive[index].data, p_env->p_primParam[index].px, p_env->p_primParam[index].py, p_env->p_primParam[index].pw, p_env->p_primParam[index].ph);
	  setRGB0((POLY_FT4 *)p_env->buffer[buffIndex].p_primitive[index].data, p_env->p_primParam[index].r0, p_env->p_primParam[index].g0, p_env->p_primParam[index].b0);
	  break;
	case TYPE_G4:
	  SetPolyG4((POLY_G4 *)p_env->buffer[buffIndex].p_primitive[index].data);
	  setXYWH((POLY_G4 *)p_env->buffer[buffIndex].p_primitive[index].data, p_env->p_primParam[index].px, p_env->p_primParam[index].py, p_env->p_primParam[index].pw, p_env->p_primParam[index].ph);       
	  setRGB0((POLY_G4 *)p_env->buffer[buffIndex].p_primitive[index].data, p_env->p_primParam[index].r0, p_env->p_primParam[index].g0, p_env->p_primParam[index].b0);
	  setRGB1((POLY_G4 *)p_env->buffer[buffIndex].p_primitive[index].data, p_env->p_primParam[index].r1, p_env->p_primParam[index].g1, p_env->p_primParam[index].b1);
	  setRGB2((POLY_G4 *)p_env->buffer[buffIndex].p_primitive[index].data, p_env->p_primParam[index].r2, p_env->p_primParam[index].g2, p_env->p_primParam[index].b2);
	  setRGB3((POLY_G4 *)p_env->buffer[buffIndex].p_primitive[index].data, p_env->p_primParam[index].r3, p_env->p_primParam[index].g3, p_env->p_primParam[index].b3);
	  break;
	case TYPE_GT4:
	  SetPolyGT4((POLY_GT4 *)p_env->buffer[buffIndex].p_primitive[index].data);
	  setUVWH((POLY_GT4 *)p_env->buffer[buffIndex].p_primitive[index].data, p_env->p_primParam[index].tx, p_env->p_primParam[index].ty, p_env->p_primParam[index].tw, p_env->p_primParam[index].th);
	  setXYWH((POLY_GT4 *)p_env->buffer[buffIndex].p_primitive[index].data, p_env->p_primParam[index].px, p_env->p_primParam[index].py, p_env->p_primParam[index].pw, p_env->p_primParam[index].ph);      
	  setRGB0((POLY_GT4 *)p_env->buffer[buffIndex].p_primitive[index].data, p_env->p_primParam[index].r0, p_env->p_primParam[index].g0, p_env->p_primParam[index].b0);
	  setRGB1((POLY_GT4 *)p_env->buffer[buffIndex].p_primitive[index].data, p_env->p_primParam[index].r1, p_env->p_primParam[index].g1, p_env->p_primParam[index].b1);
	  setRGB2((POLY_GT4 *)p_env->buffer[buffIndex].p_primitive[index].data, p_env->p_primParam[index].r2, p_env->p_primParam[index].g2, p_env->p_primParam[index].b2);
	  setRGB3((POLY_GT4 *)p_env->buffer[buffIndex].p_primitive[index].data, p_env->p_primParam[index].r3, p_env->p_primParam[index].g3, p_env->p_primParam[index].b3);
	  break;
	default:
	  printf("\nERROR, NO TYPE DEFINED AT INDEX %d\n", index);
	  break;
      }
      
      AddPrim(&(p_env->buffer[buffIndex].p_ot[index]), p_env->buffer[buffIndex].p_primitive[index].data);
    }
  }
}

void updatePrim(struct s_environment *p_env)
{
  int index;
  
  for(index = 0; index < p_env->otSize; index++)
  {
    switch(p_env->p_regBuffer->p_primitive[index].type)
    {
      case TYPE_SPRITE:
	setXY0((SPRT *)p_env->p_regBuffer->p_primitive[index].data, p_env->p_primParam[index].px, p_env->p_primParam[index].py);
	setWH((SPRT *)p_env->p_regBuffer->p_primitive[index].data, p_env->p_primParam[index].pw,  p_env->p_primParam[index].ph);
	setUV0((SPRT *)p_env->p_regBuffer->p_primitive[index].data, p_env->p_primParam[index].tx, p_env->p_primParam[index].ty);
	setRGB0((SPRT *)p_env->p_regBuffer->p_primitive[index].data, p_env->p_primParam[index].r0, p_env->p_primParam[index].g0, p_env->p_primParam[index].b0);
	break;
      case TYPE_TILE:
	setXY0((TILE *)p_env->p_regBuffer->p_primitive[index].data, p_env->p_primParam[index].px, p_env->p_primParam[index].py);
	setWH((TILE *)p_env->p_regBuffer->p_primitive[index].data, p_env->p_primParam[index].pw,  p_env->p_primParam[index].ph);
	setRGB0((TILE *)p_env->p_regBuffer->p_primitive[index].data, p_env->p_primParam[index].r0, p_env->p_primParam[index].g0, p_env->p_primParam[index].b0);
	break;
      case TYPE_F4:
	setXYWH((POLY_F4 *)p_env->p_regBuffer->p_primitive[index].data, p_env->p_primParam[index].px, p_env->p_primParam[index].py, p_env->p_primParam[index].pw, p_env->p_primParam[index].ph);
	setRGB0((POLY_F4 *)p_env->p_regBuffer->p_primitive[index].data, p_env->p_primParam[index].r0, p_env->p_primParam[index].g0, p_env->p_primParam[index].b0);
	break;
      case TYPE_FT4:
	setUVWH((POLY_FT4 *)p_env->p_regBuffer->p_primitive[index].data, p_env->p_primParam[index].tx, p_env->p_primParam[index].ty, p_env->p_primParam[index].tw, p_env->p_primParam[index].th);
	setXYWH((POLY_FT4 *)p_env->p_regBuffer->p_primitive[index].data, p_env->p_primParam[index].px, p_env->p_primParam[index].py, p_env->p_primParam[index].pw, p_env->p_primParam[index].ph);
	setRGB0((POLY_FT4 *)p_env->p_regBuffer->p_primitive[index].data, p_env->p_primParam[index].r0, p_env->p_primParam[index].g0, p_env->p_primParam[index].b0);
	break;
      case TYPE_G4:
	setXYWH((POLY_G4 *)p_env->p_regBuffer->p_primitive[index].data, p_env->p_primParam[index].px, p_env->p_primParam[index].py, p_env->p_primParam[index].pw, p_env->p_primParam[index].ph);       
	setRGB0((POLY_G4 *)p_env->p_regBuffer->p_primitive[index].data, p_env->p_primParam[index].r0, p_env->p_primParam[index].g0, p_env->p_primParam[index].b0);
	setRGB1((POLY_G4 *)p_env->p_regBuffer->p_primitive[index].data, p_env->p_primParam[index].r1, p_env->p_primParam[index].g1, p_env->p_primParam[index].b1);
	setRGB2((POLY_G4 *)p_env->p_regBuffer->p_primitive[index].data, p_env->p_primParam[index].r2, p_env->p_primParam[index].g2, p_env->p_primParam[index].b2);
	setRGB3((POLY_G4 *)p_env->p_regBuffer->p_primitive[index].data, p_env->p_primParam[index].r3, p_env->p_primParam[index].g3, p_env->p_primParam[index].b3);
	break;
      case TYPE_GT4:
	setUVWH((POLY_GT4 *)p_env->p_regBuffer->p_primitive[index].data, p_env->p_primParam[index].tx, p_env->p_primParam[index].ty, p_env->p_primParam[index].tw, p_env->p_primParam[index].th);
	setXYWH((POLY_GT4 *)p_env->p_regBuffer->p_primitive[index].data, p_env->p_primParam[index].px, p_env->p_primParam[index].py, p_env->p_primParam[index].pw, p_env->p_primParam[index].ph);      
	setRGB0((POLY_GT4 *)p_env->p_regBuffer->p_primitive[index].data, p_env->p_primParam[index].r0, p_env->p_primParam[index].g0, p_env->p_primParam[index].b0);
	setRGB1((POLY_GT4 *)p_env->p_regBuffer->p_primitive[index].data, p_env->p_primParam[index].r1, p_env->p_primParam[index].g1, p_env->p_primParam[index].b1);
	setRGB2((POLY_GT4 *)p_env->p_regBuffer->p_primitive[index].data, p_env->p_primParam[index].r2, p_env->p_primParam[index].g2, p_env->p_primParam[index].b2);
	setRGB3((POLY_GT4 *)p_env->p_regBuffer->p_primitive[index].data, p_env->p_primParam[index].r3, p_env->p_primParam[index].g3, p_env->p_primParam[index].b3);
	break;
      default:
	printf("\nUnknown Type for update at index %d %d\n", index, p_env->p_regBuffer->p_primitive[index].type);
	break;
    }
  }
}

void movPrim(struct s_environment *p_env)
{  
  if(p_env->gamePad.one.fourth.bit.circle == 0)
  {
    if(p_env->prevTime == 0 || ((VSync(-1) - p_env->prevTime) > 60))
    {
      p_env->curPrim = (p_env->curPrim + 1) % p_env->otSize;
      p_env->prevTime = VSync(-1);
    }
  }
  
  if(p_env->gamePad.one.fourth.bit.ex == 0)
  {
    if(p_env->prevTime == 0 || ((VSync(-1) - p_env->prevTime) > 60))
    {
      p_env->p_primParam[p_env->curPrim].r0 = rand() % 256;
      p_env->p_primParam[p_env->curPrim].g0 = rand() % 256;
      p_env->p_primParam[p_env->curPrim].b0 = rand() % 256;
      p_env->prevTime = VSync(-1);
    }
  }
  
  if(p_env->gamePad.one.third.bit.up == 0)
  {
    if(p_env->p_primParam[p_env->curPrim].py > 0)
    {
      p_env->p_primParam[p_env->curPrim].py -= 1;
    }
  }
  
  if(p_env->gamePad.one.third.bit.right == 0)
  {
    if((p_env->p_primParam[p_env->curPrim].px + p_env->p_primParam[p_env->curPrim].pw) < SCREEN_WIDTH)
    {
      p_env->p_primParam[p_env->curPrim].px += 1;
    }
  }
  
  if(p_env->gamePad.one.third.bit.down == 0)
  {
    if((p_env->p_primParam[p_env->curPrim].py + p_env->p_primParam[p_env->curPrim].ph) < SCREEN_HEIGHT)
    {
      p_env->p_primParam[p_env->curPrim].py += 1;
    }
  }
  
  if(p_env->gamePad.one.third.bit.left == 0)
  {
    if(p_env->p_primParam[p_env->curPrim].px > 0)
    {
      p_env->p_primParam[p_env->curPrim].px -= 1;
    }
  }
  
  updatePrim(p_env);
}

char *memoryCardRead()
{
  long cmds;
  long result;
  char *phrase = NULL;
  
  PadStopCom();
  
  phrase = malloc(128);
    
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

  if(MemCardOpen(0, "test", O_RDONLY) != 0)
  {
    printf("\nOpen Issue\n");
  }

  if(MemCardReadData((unsigned long *)phrase, 0, 128)  != 0)
  {
    printf("\nRead Issue\n");
  }

  if(MemCardSync(0, &cmds, &result) <= 0)
  {
    printf("\nSync Failed\n");
  }

  MemCardClose();

  MemCardStop();
  
  PadStartCom();
  
  return phrase;
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

 
