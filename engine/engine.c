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

#include <libds.h>
#include <libpad.h>
#include <libetc.h>
#include <libgs.h>
#include <libmcrd.h>
#include <libapi.h>
#include <sys/file.h>
#include <getprim.h>
#include <bmpmanip.h>

u_long __ramsize = 0x00200000;  //force 2 megabytes of RAM
u_long __stacksize = 0x00004000; //force 16 kilobytes of stack

#define BUFSIZE 2048

//utility functions
void swapBuffers(struct s_environment *p_env)
{
  p_env->p_currBuffer = (p_env->p_currBuffer == p_env->buffer ? p_env->buffer + 1 : p_env->buffer);
}

//available functions
void initEnv(struct s_environment *p_env, int numPrim)
{
  int index;
  int bufIndex;
  
  p_env->bufSize = DOUBLE_BUF;
  p_env->otSize = (numPrim < 1 ? 1 : numPrim);
  p_env->primSize = p_env->otSize;
  p_env->primCur = 0;
  p_env->prevTime = 0;
  p_env->p_primParam = NULL;
  
  for(bufIndex = 0; bufIndex < p_env->bufSize; bufIndex++)
  {
    p_env->buffer[bufIndex].p_primitive = calloc(p_env->otSize, sizeof(struct s_primitive));
    p_env->buffer[bufIndex].p_ot = calloc(p_env->otSize, sizeof(unsigned long));
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

  for(bufIndex = 0; bufIndex < p_env->bufSize; bufIndex += 2) 
  {
    SetDefDispEnv(&p_env->buffer[bufIndex].disp, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    SetDefDrawEnv(&p_env->buffer[bufIndex].draw, 0, SCREEN_HEIGHT, SCREEN_WIDTH, SCREEN_HEIGHT);
  }

  for(bufIndex = 1; bufIndex < p_env->bufSize; bufIndex += 2)
  {
    SetDefDispEnv(&p_env->buffer[bufIndex].disp, 0, SCREEN_HEIGHT, SCREEN_WIDTH, SCREEN_HEIGHT);
    SetDefDrawEnv(&p_env->buffer[bufIndex].draw, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
  }

  for(bufIndex = 0; bufIndex < p_env->bufSize; bufIndex++)
  {
    p_env->buffer[bufIndex].draw.isbg = 1;
    p_env->buffer[bufIndex].draw.r0 = 0;
    p_env->buffer[bufIndex].draw.g0 = 0;
    p_env->buffer[bufIndex].draw.b0 = 0;
    
    ClearOTag(p_env->buffer[bufIndex].p_ot, p_env->otSize);
  }
  
  p_env->p_currBuffer = p_env->buffer;
  
  FntLoad(960, 256); // load the font from the BIOS into VRAM/SGRAM
  SetDumpFnt(FntOpen(5, 20, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 512)); // screen X,Y | max text length X,Y | autmatic background clear 0,1 | max characters
  
  //CD init 
  DsInit();
  
  SpuInit();
  
  PadInitDirect((u_char *)&p_env->gamePad.one, (u_char *)&p_env->gamePad.two);
  PadStartCom();
  
  initGetPrimData();
  
  SetDispMask(1); 
}

void setupSound(struct s_environment *p_env)
{ 
  //setup volume and cd into mix
  p_env->soundAttr.mask = (SPU_COMMON_MVOLL | SPU_COMMON_MVOLR | SPU_COMMON_CDVOLL | SPU_COMMON_CDVOLR | SPU_COMMON_CDMIX);
  
  p_env->soundAttr.mvol.left = 0x1FFF;
  p_env->soundAttr.mvol.right = 0x1FFF;
  
  p_env->soundAttr.cd.volume.left = 0x1FFF;
  p_env->soundAttr.cd.volume.right = 0x1FFF;
  
  p_env->soundAttr.cd.mix = SPU_ON;
  
  //set the spu attributes
  SpuSetCommonAttr(&p_env->soundAttr);
  
  SpuSetTransferMode(SPU_TRANSFER_BY_DMA);
}

void playCDtracks(int *p_tracks, int trackNum)
{
  if(DsPlay(2, p_tracks, trackNum) < 0)
  {
    printf("\nNo CD Track Playing\n");
  }
  
  printf("\nCurrent Track: %d\n", DsPlay(3, p_tracks, 1));
}

void display(struct s_environment *p_env)
{
  //avoid issues with delayed execution
  while(DrawSync(1));
  VSync(0);
  //font flush now so contents get drawn
  FntFlush(-1);
  
  PutDrawEnv(&p_env->p_currBuffer->draw);
  PutDispEnv(&p_env->p_currBuffer->disp);
  
  DrawOTag(p_env->p_currBuffer->p_ot);
  
  //exchange reg and draw buffer, so newly registered ot will be drawn, and used draw buffer can now be used for registration.
  swapBuffers(p_env);
  
  //write header before all other font prints
  FntPrint("%s\n%s\n%X", p_env->envMessage.p_title, p_env->envMessage.p_message, *p_env->envMessage.p_data);
}

void populateTextures(struct s_environment *p_env)
{
  int index;
  int buffIndex;
  int returnValue = 0;
  
  printf("\nStarted Getting texture info\n");
  
  for(index = 0; index < p_env->otSize; index++)
  {
    if(p_env->p_primParam[index]->p_texture != NULL)
    {
      printf("\nTEXTURE AT INDEX %d %s\n", index, p_env->p_primParam[index]->p_texture->file);
      
      p_env->p_primParam[index]->p_texture->p_data = (uint8_t *)loadFileFromCD(p_env->p_primParam[index]->p_texture->file, &(p_env->p_primParam[index]->p_texture->size));
      
      if(p_env->p_primParam[index]->p_texture->p_data != NULL)
      {
	printf("\nGETTING RAW\n");
	
	returnValue = bitmapToRAW(&(p_env->p_primParam[index]->p_texture->p_data), p_env->p_primParam[index]->p_texture->size, p_env->p_primParam[index]->p_texture->dimensions.w, p_env->p_primParam[index]->p_texture->dimensions.h);
	
	if(returnValue > 0)
	{
	  p_env->p_primParam[index]->p_texture->size = returnValue;
	}
	else if(returnValue < 0)
	{
	  printf("\nBAD DATA\n");
	  continue;
	}
	
	if(swapRedBlue(p_env->p_primParam[index]->p_texture->p_data, p_env->p_primParam[index]->p_texture->size) < 0)
	{
	  printf("\nSWAP FAILED\n");
	  continue;
	}
	
	p_env->p_primParam[index]->p_texture->id = LoadTPage((u_long *)p_env->p_primParam[index]->p_texture->p_data, 2, 0, p_env->p_primParam[index]->p_texture->vramVertex.x, p_env->p_primParam[index]->p_texture->vramVertex.y, p_env->p_primParam[index]->p_texture->dimensions.w, p_env->p_primParam[index]->p_texture->dimensions.h);
	
	printf("\nTPAGE ID: %d %d %d %d %d\n", p_env->p_primParam[index]->p_texture->id, p_env->p_primParam[index]->p_texture->vertex0.x, p_env->p_primParam[index]->p_texture->vertex0.y, p_env->p_primParam[index]->p_texture->dimensions.w, p_env->p_primParam[index]->p_texture->dimensions.h);
      }
    }
  }
  
  for(buffIndex = 0; buffIndex < p_env->bufSize; buffIndex++)
  {
    for(index = 0; index < p_env->otSize; index++)
    {
      if(p_env->p_primParam[index]->p_texture != NULL)
      {
	p_env->buffer[buffIndex].p_primitive[index].type = p_env->p_primParam[index]->type;
	
	switch(p_env->buffer[buffIndex].p_primitive[index].type)
	{
	  case TYPE_FT4:
	    printf("\nTYPE_FT4\n");
	    ((POLY_FT4 *)p_env->buffer[buffIndex].p_primitive[index].data)->tpage = p_env->p_primParam[index]->p_texture->id;
	    printf("\nID %d CLUT %d\n", ((POLY_FT4 *)p_env->buffer[buffIndex].p_primitive[index].data)->tpage, ((POLY_FT4 *)p_env->buffer[buffIndex].p_primitive[index].data)->clut);
	    break;
	  case TYPE_GT4:
	    ((POLY_GT4 *)p_env->buffer[buffIndex].p_primitive[index].data)->tpage = p_env->p_primParam[index]->p_texture->id;
	    break;
	  case TYPE_SPRITE:
	    printf("\nTYPE_SPRITE\n");
	    SetDrawTPage((DR_TPAGE *)(&p_env->p_primParam[index]->p_texture->tpage), 1, 0, p_env->p_primParam[index]->p_texture->id);
	    AddPrim(&(p_env->buffer[buffIndex].p_ot[index]), &p_env->p_primParam[index]->p_texture->tpage);
	    break;
	  default:
	    printf("\nNon Texture Type at index %d\n", index);
	    break;
	}
      }
    }
  }
}

void *loadFileFromCD(char *p_path, int *op_len)
{
  int sizeSectors = 0;
  int numRemain = 0;
  int prevNumRemain = 0;
  u_char result = 0;
  
  DslFILE fileInfo;
  u_long *file = NULL;
  
  if(DsSearchFile(&fileInfo, p_path) <= 0)
  {
    printf("\nFILE SEARCH FAILED\n");
    return NULL;
  }

  printf("\nFILE SEARCH SUCCESS\n");
  
  sizeSectors = (fileInfo.size + 2047) / 2048;

  printf("\nSECTOR SIZE: %d %d", sizeSectors, fileInfo.size);
  
  file = malloc(sizeSectors * 2048);
  
  if(file == NULL)
  {
    printf("\nALLOCATION FAILED\n");
    return NULL;
  }
  
  printf("\nMEMORY ALLOCATED\n");
  
  DsRead(&fileInfo.pos, sizeSectors, file, DslModeSpeed);
  
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
  
  if(op_len != NULL)
  {
    *op_len = fileInfo.size;
  }
  
  printf("\nREAD COMPLETE\n");
  
  return file;
}

struct s_primParam *getObjects(char *fileName)
{
  char *p_buff = NULL;
  struct s_primParam *p_primParam;
  
  p_buff = (char *)loadFileFromCD(fileName, NULL);
  
  if(p_buff == NULL)
  {
    return NULL;
  }
  
  setXMLdata(p_buff);
  
  p_primParam = getPrimData();
  
  resetGetPrimData();
  
  free(p_buff);
 
  return p_primParam;
}

void freeObjects(struct s_primParam **p_primParam)
{
  freePrimData(p_primParam);
}

void populateOT(struct s_environment *p_env)
{
  int index;
  int buffIndex;
  
  for(index = 0; index < p_env->otSize; index++)
  {    
    for(buffIndex = 0; buffIndex < p_env->bufSize; buffIndex++)
    {
      p_env->buffer[buffIndex].p_primitive[index].type = p_env->p_primParam[index]->type;
      
      switch(p_env->buffer[buffIndex].p_primitive[index].type)
      {
	case TYPE_SPRITE:
	  SetSprt((SPRT *)p_env->buffer[buffIndex].p_primitive[index].data);
	  setXY0((SPRT *)p_env->buffer[buffIndex].p_primitive[index].data, p_env->p_primParam[index]->vertex0.x, p_env->p_primParam[index]->vertex0.y);
	  setWH((SPRT *)p_env->buffer[buffIndex].p_primitive[index].data, p_env->p_primParam[index]->dimensions.w,  p_env->p_primParam[index]->dimensions.h);
	  setUV0((SPRT *)p_env->buffer[buffIndex].p_primitive[index].data, p_env->p_primParam[index]->p_texture->vertex0.x, p_env->p_primParam[index]->p_texture->vertex0.y);
	  setRGB0((SPRT *)p_env->buffer[buffIndex].p_primitive[index].data, p_env->p_primParam[index]->color0.r, p_env->p_primParam[index]->color0.g, p_env->p_primParam[index]->color0.b);
	  break;
	case TYPE_TILE:
	  setTile((TILE *)p_env->buffer[buffIndex].p_primitive[index].data);
	  setXY0((TILE *)p_env->buffer[buffIndex].p_primitive[index].data, p_env->p_primParam[index]->vertex0.x, p_env->p_primParam[index]->vertex0.y);
	  setWH((TILE *)p_env->buffer[buffIndex].p_primitive[index].data, p_env->p_primParam[index]->dimensions.w,  p_env->p_primParam[index]->dimensions.h);
	  setRGB0((TILE *)p_env->buffer[buffIndex].p_primitive[index].data, p_env->p_primParam[index]->color0.r, p_env->p_primParam[index]->color0.g, p_env->p_primParam[index]->color0.b);
	  break;
	case TYPE_F4:
	  SetPolyF4((POLY_F4 *)p_env->buffer[buffIndex].p_primitive[index].data);
	  setXYWH((POLY_F4 *)p_env->buffer[buffIndex].p_primitive[index].data, p_env->p_primParam[index]->vertex0.x, p_env->p_primParam[index]->vertex0.y, p_env->p_primParam[index]->dimensions.w, p_env->p_primParam[index]->dimensions.h);
	  setRGB0((POLY_F4 *)p_env->buffer[buffIndex].p_primitive[index].data, p_env->p_primParam[index]->color0.r, p_env->p_primParam[index]->color0.g, p_env->p_primParam[index]->color0.b);
	  break;
	case TYPE_FT4:
	  SetPolyFT4((POLY_FT4 *)p_env->buffer[buffIndex].p_primitive[index].data);
	  setUVWH((POLY_FT4 *)p_env->buffer[buffIndex].p_primitive[index].data, p_env->p_primParam[index]->p_texture->vertex0.x, p_env->p_primParam[index]->p_texture->vertex0.y, p_env->p_primParam[index]->p_texture->dimensions.w, p_env->p_primParam[index]->p_texture->dimensions.h);
	  setXYWH((POLY_FT4 *)p_env->buffer[buffIndex].p_primitive[index].data, p_env->p_primParam[index]->vertex0.x, p_env->p_primParam[index]->vertex0.y, p_env->p_primParam[index]->dimensions.w, p_env->p_primParam[index]->dimensions.h);
	  setRGB0((POLY_FT4 *)p_env->buffer[buffIndex].p_primitive[index].data, p_env->p_primParam[index]->color0.r, p_env->p_primParam[index]->color0.g, p_env->p_primParam[index]->color0.b);
	  printf("\nTYPE_FT4 SET ID: %d %d %d %d %d\n", p_env->p_primParam[index]->p_texture->id, p_env->p_primParam[index]->p_texture->vertex0.x, p_env->p_primParam[index]->p_texture->vertex0.y, p_env->p_primParam[index]->p_texture->dimensions.w, p_env->p_primParam[index]->p_texture->dimensions.h);
	  break;
	case TYPE_G4:
	  SetPolyG4((POLY_G4 *)p_env->buffer[buffIndex].p_primitive[index].data);
	  setXYWH((POLY_G4 *)p_env->buffer[buffIndex].p_primitive[index].data, p_env->p_primParam[index]->vertex0.x, p_env->p_primParam[index]->vertex0.y, p_env->p_primParam[index]->dimensions.w, p_env->p_primParam[index]->dimensions.h);       
	  setRGB0((POLY_G4 *)p_env->buffer[buffIndex].p_primitive[index].data, p_env->p_primParam[index]->color0.r, p_env->p_primParam[index]->color0.g, p_env->p_primParam[index]->color0.b);
	  setRGB1((POLY_G4 *)p_env->buffer[buffIndex].p_primitive[index].data, p_env->p_primParam[index]->color1.r, p_env->p_primParam[index]->color1.g, p_env->p_primParam[index]->color1.b);
	  setRGB2((POLY_G4 *)p_env->buffer[buffIndex].p_primitive[index].data, p_env->p_primParam[index]->color2.r, p_env->p_primParam[index]->color2.g, p_env->p_primParam[index]->color2.b);
	  setRGB3((POLY_G4 *)p_env->buffer[buffIndex].p_primitive[index].data, p_env->p_primParam[index]->color3.r, p_env->p_primParam[index]->color3.g, p_env->p_primParam[index]->color3.b);
	  break;
	case TYPE_GT4:
	  SetPolyGT4((POLY_GT4 *)p_env->buffer[buffIndex].p_primitive[index].data);
	  setUVWH((POLY_GT4 *)p_env->buffer[buffIndex].p_primitive[index].data, p_env->p_primParam[index]->p_texture->vertex0.x, p_env->p_primParam[index]->p_texture->vertex0.y, p_env->p_primParam[index]->p_texture->dimensions.w, p_env->p_primParam[index]->p_texture->dimensions.h);
	  setXYWH((POLY_GT4 *)p_env->buffer[buffIndex].p_primitive[index].data, p_env->p_primParam[index]->vertex0.x, p_env->p_primParam[index]->vertex0.y, p_env->p_primParam[index]->dimensions.w, p_env->p_primParam[index]->dimensions.h);      
	  setRGB0((POLY_GT4 *)p_env->buffer[buffIndex].p_primitive[index].data, p_env->p_primParam[index]->color0.r, p_env->p_primParam[index]->color0.g, p_env->p_primParam[index]->color0.b);
	  setRGB1((POLY_GT4 *)p_env->buffer[buffIndex].p_primitive[index].data, p_env->p_primParam[index]->color1.r, p_env->p_primParam[index]->color1.g, p_env->p_primParam[index]->color1.b);
	  setRGB2((POLY_GT4 *)p_env->buffer[buffIndex].p_primitive[index].data, p_env->p_primParam[index]->color2.r, p_env->p_primParam[index]->color2.g, p_env->p_primParam[index]->color2.b);
	  setRGB3((POLY_GT4 *)p_env->buffer[buffIndex].p_primitive[index].data, p_env->p_primParam[index]->color3.r, p_env->p_primParam[index]->color3.g, p_env->p_primParam[index]->color3.b);
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
    switch(p_env->p_currBuffer->p_primitive[index].type)
    {
      
      case TYPE_SPRITE:
	setXY0((SPRT *)p_env->p_currBuffer->p_primitive[index].data, p_env->p_primParam[index]->vertex0.x, p_env->p_primParam[index]->vertex0.y);
	setWH((SPRT *)p_env->p_currBuffer->p_primitive[index].data, p_env->p_primParam[index]->dimensions.w,  p_env->p_primParam[index]->dimensions.h);
	setUV0((SPRT *)p_env->p_currBuffer->p_primitive[index].data, p_env->p_primParam[index]->p_texture->vertex0.x, p_env->p_primParam[index]->p_texture->vertex0.y);
	setRGB0((SPRT *)p_env->p_currBuffer->p_primitive[index].data, p_env->p_primParam[index]->color0.r, p_env->p_primParam[index]->color0.g, p_env->p_primParam[index]->color0.b);
	break;
      case TYPE_TILE:
	setXY0((TILE *)p_env->p_currBuffer->p_primitive[index].data, p_env->p_primParam[index]->vertex0.x, p_env->p_primParam[index]->vertex0.y);
	setWH((TILE *)p_env->p_currBuffer->p_primitive[index].data, p_env->p_primParam[index]->dimensions.w,  p_env->p_primParam[index]->dimensions.h);
	setRGB0((TILE *)p_env->p_currBuffer->p_primitive[index].data, p_env->p_primParam[index]->color0.r, p_env->p_primParam[index]->color0.g, p_env->p_primParam[index]->color0.b);
	break;
      case TYPE_F4:
	setXYWH((POLY_F4 *)p_env->p_currBuffer->p_primitive[index].data, p_env->p_primParam[index]->vertex0.x, p_env->p_primParam[index]->vertex0.y, p_env->p_primParam[index]->dimensions.w, p_env->p_primParam[index]->dimensions.h);
	setRGB0((POLY_F4 *)p_env->p_currBuffer->p_primitive[index].data, p_env->p_primParam[index]->color0.r, p_env->p_primParam[index]->color0.g, p_env->p_primParam[index]->color0.b);
	break;
      case TYPE_FT4:
	setUVWH((POLY_FT4 *)p_env->p_currBuffer->p_primitive[index].data, p_env->p_primParam[index]->p_texture->vertex0.x, p_env->p_primParam[index]->p_texture->vertex0.y, p_env->p_primParam[index]->p_texture->dimensions.w, p_env->p_primParam[index]->p_texture->dimensions.h);
	setXYWH((POLY_FT4 *)p_env->p_currBuffer->p_primitive[index].data, p_env->p_primParam[index]->vertex0.x, p_env->p_primParam[index]->vertex0.y, p_env->p_primParam[index]->dimensions.w, p_env->p_primParam[index]->dimensions.h);
	setRGB0((POLY_FT4 *)p_env->p_currBuffer->p_primitive[index].data, p_env->p_primParam[index]->color0.r, p_env->p_primParam[index]->color0.g, p_env->p_primParam[index]->color0.b);
	break;
      case TYPE_G4:
	setXYWH((POLY_G4 *)p_env->p_currBuffer->p_primitive[index].data, p_env->p_primParam[index]->vertex0.x, p_env->p_primParam[index]->vertex0.y, p_env->p_primParam[index]->dimensions.w, p_env->p_primParam[index]->dimensions.h);       
	setRGB0((POLY_G4 *)p_env->p_currBuffer->p_primitive[index].data, p_env->p_primParam[index]->color0.r, p_env->p_primParam[index]->color0.g, p_env->p_primParam[index]->color0.b);
	setRGB1((POLY_G4 *)p_env->p_currBuffer->p_primitive[index].data, p_env->p_primParam[index]->color1.r, p_env->p_primParam[index]->color1.g, p_env->p_primParam[index]->color1.b);
	setRGB2((POLY_G4 *)p_env->p_currBuffer->p_primitive[index].data, p_env->p_primParam[index]->color2.r, p_env->p_primParam[index]->color2.g, p_env->p_primParam[index]->color2.b);
	setRGB3((POLY_G4 *)p_env->p_currBuffer->p_primitive[index].data, p_env->p_primParam[index]->color3.r, p_env->p_primParam[index]->color3.g, p_env->p_primParam[index]->color3.b);
	break;
      case TYPE_GT4:
	setUVWH((POLY_GT4 *)p_env->p_currBuffer->p_primitive[index].data, p_env->p_primParam[index]->p_texture->vertex0.x, p_env->p_primParam[index]->p_texture->vertex0.y, p_env->p_primParam[index]->p_texture->dimensions.w, p_env->p_primParam[index]->p_texture->dimensions.h);
	setXYWH((POLY_GT4 *)p_env->p_currBuffer->p_primitive[index].data, p_env->p_primParam[index]->vertex0.x, p_env->p_primParam[index]->vertex0.y, p_env->p_primParam[index]->dimensions.w, p_env->p_primParam[index]->dimensions.h);      
	setRGB0((POLY_GT4 *)p_env->p_currBuffer->p_primitive[index].data, p_env->p_primParam[index]->color0.r, p_env->p_primParam[index]->color0.g, p_env->p_primParam[index]->color0.b);
	setRGB1((POLY_GT4 *)p_env->p_currBuffer->p_primitive[index].data, p_env->p_primParam[index]->color1.r, p_env->p_primParam[index]->color1.g, p_env->p_primParam[index]->color1.b);
	setRGB2((POLY_GT4 *)p_env->p_currBuffer->p_primitive[index].data, p_env->p_primParam[index]->color2.r, p_env->p_primParam[index]->color2.g, p_env->p_primParam[index]->color2.b);
	setRGB3((POLY_GT4 *)p_env->p_currBuffer->p_primitive[index].data, p_env->p_primParam[index]->color3.r, p_env->p_primParam[index]->color3.g, p_env->p_primParam[index]->color3.b);
	break;
      default:
	printf("\nUnknown Type for update at index %d %d\n", index, p_env->p_currBuffer->p_primitive[index].type);
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
      p_env->primCur = (p_env->primCur + 1) % p_env->otSize;
      p_env->prevTime = VSync(-1);
    }
  }
  
  if(p_env->gamePad.one.fourth.bit.ex == 0)
  {
    if(p_env->prevTime == 0 || ((VSync(-1) - p_env->prevTime) > 60))
    {
      p_env->p_primParam[p_env->primCur]->color0.r = rand() % 256;
      p_env->p_primParam[p_env->primCur]->color0.g = rand() % 256;
      p_env->p_primParam[p_env->primCur]->color0.b = rand() % 256;
      p_env->prevTime = VSync(-1);
    }
  }
  
  if(p_env->gamePad.one.third.bit.up == 0)
  {
    if(p_env->p_primParam[p_env->primCur]->vertex0.y > 0)
    {
      p_env->p_primParam[p_env->primCur]->vertex0.y -= 1;
    }
  }
  
  if(p_env->gamePad.one.third.bit.right == 0)
  {
    if((p_env->p_primParam[p_env->primCur]->vertex0.x + p_env->p_primParam[p_env->primCur]->dimensions.w) < SCREEN_WIDTH)
    {
      p_env->p_primParam[p_env->primCur]->vertex0.x += 1;
    }
  }
  
  if(p_env->gamePad.one.third.bit.down == 0)
  {
    if((p_env->p_primParam[p_env->primCur]->vertex0.y + p_env->p_primParam[p_env->primCur]->dimensions.h) < SCREEN_HEIGHT)
    {
      p_env->p_primParam[p_env->primCur]->vertex0.y += 1;
    }
  }
  
  if(p_env->gamePad.one.third.bit.left == 0)
  {
    if(p_env->p_primParam[p_env->primCur]->vertex0.x > 0)
    {
      p_env->p_primParam[p_env->primCur]->vertex0.x -= 1;
    }
  }
  
  updatePrim(p_env);
}

char *memoryCardRead(uint32_t len)
{
  long cmds;
  long result;
  char *phrase = NULL;
  
  PadStopCom();
  
  phrase = malloc(len);
    
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

  if(MemCardReadData((unsigned long *)phrase, 0, len)  != 0)
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

void memoryCardWrite(char *p_phrase, uint32_t len)
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

  if(MemCardWriteData((unsigned long *)p_phrase, 0, len)  != 0)
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

 
