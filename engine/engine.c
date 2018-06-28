/*
 * Started: 10/01/2016
 * By: John Convertino
 * electrobs@gmail.com
 * 
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
 * 
 * Version: 1.0
 * 
*/


#include "engine.h"

#include <libds.h>
#include <libsio.h>
#include <libpad.h>
#include <libetc.h>
#include <libgs.h>
#include <libgte.h>
#include <libmcrd.h>
#include <libapi.h>
#include <sys/file.h>
#include <getprim.h>
#include <bmpmanip.h>

u_long __ramsize = 0x00200000;  //force 2 megabytes of RAM
u_long __stacksize = 0x00004000; //force 16 kilobytes of stack

#define BUFSIZE 2048

//utility functions
//swap buffer, if the current buffer equals to first, move to the next, else use the first
void swapBuffers(struct s_environment *p_env)
{
  p_env->p_currBuffer = (p_env->p_currBuffer == p_env->buffer ? p_env->buffer + 1 : p_env->buffer);
}

//for debugging matrixs. Prints all info to debug console
void prmatrix(MATRIX *m)
{
        printf("mat=[%8d,%8d,%8d]\n",m->m[0][0],m->m[0][1],m->m[0][2]);
        printf("    [%8d,%8d,%8d]\n",m->m[1][0],m->m[1][1],m->m[1][2]);
        printf("    [%8d,%8d,%8d]\n",m->m[2][0],m->m[2][1],m->m[2][2]);
        printf("    [%8d,%8d,%8d]\n\n",m->t[0],m->t[1],m->t[2]);
}

//clear vram so artifacts are not displayed on screen
void clearVRAM()
{
  RECT vramArea;
  
  printf("\nCLEARING VRAM\n");
  
  setRECT(&vramArea, 0, 0, 1024, 512);
  
  ClearImage(&vramArea, 0, 0, 0);
  
  while(DrawSync(1));
  
  printf("\nDONE CLEARING VRAM\n");
}

//available functions
//init environment
void initEnv(struct s_environment *p_env, int numPrim)
{
  int index;
  int bufIndex;
  
  //setup struct
  memset(p_env, 0, sizeof(*p_env));
  p_env->bufSize = DOUBLE_BUF;
  p_env->otSize = (numPrim < 1 ? 1 : numPrim);
  p_env->primSize = p_env->otSize;
  p_env->primCur = 0;
  p_env->prevTime = 0;
  p_env->p_primParam = NULL;
  
  //allocate ordering table and primitive list
  for(bufIndex = 0; bufIndex < p_env->bufSize; bufIndex++)
  {
    p_env->buffer[bufIndex].p_primitive = calloc(p_env->otSize, sizeof(struct s_primitive));
    p_env->buffer[bufIndex].p_ot = calloc(p_env->otSize, sizeof(unsigned long));
  }
  
  //allocate number of primitives
  p_env->p_primParam = calloc(p_env->otSize, sizeof(struct s_primParam));
  
  // within the BIOS, if the address 0xBFC7FF52 equals 'E', set it as PAL (1). Otherwise, set it as NTSC (0)
  switch(*(char *)0xbfc7ff52)
  {
    case 'E':
      SetVideoMode(MODE_PAL); 
      break;
    default:
      SetVideoMode(MODE_NTSC); 
      break;	
  }
  
  //reset graphics
  ResetCallback();
  ResetGraph(0);

  //setup graphics double buffering 
  SetDefDispEnv(&p_env->buffer[0].disp, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
  SetDefDrawEnv(&p_env->buffer[0].draw, 0, SCREEN_HEIGHT, SCREEN_WIDTH, SCREEN_HEIGHT);
  SetDefDispEnv(&p_env->buffer[1].disp, 0, SCREEN_HEIGHT, SCREEN_WIDTH, SCREEN_HEIGHT);
  SetDefDrawEnv(&p_env->buffer[1].draw, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

  //setup draw environment for both buffers
  for(bufIndex = 0; bufIndex < p_env->bufSize; bufIndex++)
  {
    //black background, and isbg set to reset when a new draw starts
    p_env->buffer[bufIndex].draw.isbg = 1;
    p_env->buffer[bufIndex].draw.r0 = 0;
    p_env->buffer[bufIndex].draw.g0 = 0;
    p_env->buffer[bufIndex].draw.b0 = 0;
    
    //clear ordering table
    ClearOTag(p_env->buffer[bufIndex].p_ot, p_env->otSize);
  }
  
  //set current buffer
  p_env->p_currBuffer = p_env->buffer;
  
  //setup geometry
  InitGeom();
  
  SetGraphDebug(0); //0 = no checks, 1 = checks, 2 = dump of registered and drawn
  
  SetGeomScreen(1024); // this sets distance h (eye to screen)
  
  //CD init 
  DsInit();
  
  //sound
  SpuInit();
  
  //game pad
  PadInitDirect((u_char *)&p_env->gamePad.one, (u_char *)&p_env->gamePad.two);
  PadStartCom();
  
  //get prim data
  initGetPrimData();
  
  AddSIO(9600);
  
  clearVRAM();
  
  //font print debug info
  FntLoad(960, 256);
  SetDumpFnt(FntOpen(5, 20, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 512));
  
  //allow display to be seen
  SetDispMask(1); 
}

//setup sound for the play station
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

//play CDDA audio tracks
void playCDtracks(int *p_tracks, int trackNum)
{
  if(DsPlay(2, p_tracks, trackNum) < 0)
  {
    printf("\nNo CD Track Playing\n");
  }
  
  printf("\nCurrent Track: %d\n", DsPlay(3, p_tracks, 1));
}

//update display
void display(struct s_environment *p_env)
{
  //avoid issues with delayed execution
  while(DrawSync(1));
  VSync(0);
  //font flush now so contents get drawn
  FntFlush(-1);
  
  //put the current buffer contents up
  PutDrawEnv(&p_env->p_currBuffer->draw);
  PutDispEnv(&p_env->p_currBuffer->disp);
  
  //start drawings
  DrawOTag(p_env->p_currBuffer->p_ot);
  
  //exchange reg and draw buffer, so newly registered ot will be drawn, and used draw buffer can now be used for registration.
  swapBuffers(p_env);
  
  //write header before all other font prints
  FntPrint("%s\n%s\n%X", p_env->envMessage.p_title, p_env->envMessage.p_message, *p_env->envMessage.p_data);
}

//populate textures to VRAM
void populateTextures(struct s_environment *p_env)
{
  int index;
  int buffIndex;
  int returnValue = 0;
  
  printf("\nStarted Getting texture info\n");
  
  //use texture info if it exists
  for(index = 0; index < p_env->otSize; index++)
  {
    if(p_env->p_primParam[index]->p_texture != NULL)
    {
      printf("\nTEXTURE AT INDEX %d %s\n", index, p_env->p_primParam[index]->p_texture->file);
      
      //load texture file from CD
      p_env->p_primParam[index]->p_texture->p_data = (uint8_t *)loadFileFromCD(p_env->p_primParam[index]->p_texture->file, &(p_env->p_primParam[index]->p_texture->size));
      
      if(p_env->p_primParam[index]->p_texture->p_data != NULL)
      {
	printf("\nGETTING RAW\n");
	
	//convert bitmaps to raw data
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
	
	//convert color space
	if(swapRedBlue(p_env->p_primParam[index]->p_texture->p_data, p_env->p_primParam[index]->p_texture->size) < 0)
	{
	  printf("\nSWAP FAILED\n");
	  continue;
	}
	
	//load via LoadTPage (can also be done with loadImage, and some checking that transfer is over, then use getIDtpage.
	p_env->p_primParam[index]->p_texture->id = LoadTPage((u_long *)p_env->p_primParam[index]->p_texture->p_data, 2, 0, p_env->p_primParam[index]->p_texture->vramVertex.vx, p_env->p_primParam[index]->p_texture->vramVertex.vy, p_env->p_primParam[index]->p_texture->dimensions.w, p_env->p_primParam[index]->p_texture->dimensions.h);

	//wait for transfer to finish
	while(DrawSync(1));
	
	//data transfered, free some memory
	free(p_env->p_primParam[index]->p_texture->p_data);
	p_env->p_primParam[index]->p_texture->p_data = NULL;
      }
    }
  }
  
  //update id info to primitives
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

//load files from CD using high level library
void *loadFileFromCD(char *p_path, uint32_t *op_len)
{
  int sizeSectors = 0;
  int numRemain = 0;
  int prevNumRemain = 0;
  u_char result = 0;
  
  DslFILE fileInfo;
  u_long *file = NULL;
  
  //search for file to get size
  if(DsSearchFile(&fileInfo, p_path) <= 0)
  {
    printf("\nFILE SEARCH FAILED\n");
    return NULL;
  }

  printf("\nFILE SEARCH SUCCESS\n");
  
  //read is in sectors, convert size to number of sectors
  sizeSectors = (fileInfo.size + 2047) / 2048;

  printf("\nSECTOR SIZE: %d %d", sizeSectors, fileInfo.size);
  
  file = malloc(sizeSectors * 2048);
  
  if(file == NULL)
  {
    printf("\nALLOCATION FAILED\n");
    return NULL;
  }
  
  printf("\nMEMORY ALLOCATED\n");
  
  //read, and keep reading till numRemain is 0
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
  
  //return length if needed
  if(op_len != NULL)
  {
    *op_len = fileInfo.size;
  }
  
  printf("\nREAD COMPLETE\n");
  
  return file;
}

//get object data from xml files
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

//clean up objects
void freeObjects(struct s_primParam **p_primParam)
{
  freePrimData(p_primParam);
}

//setup primitives for ordering table and add them
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
	  setXY0((SPRT *)p_env->buffer[buffIndex].p_primitive[index].data, p_env->p_primParam[index]->vertex0.vx, p_env->p_primParam[index]->vertex0.vy);
	  setWH((SPRT *)p_env->buffer[buffIndex].p_primitive[index].data, p_env->p_primParam[index]->dimensions.w,  p_env->p_primParam[index]->dimensions.h);
	  setUV0((SPRT *)p_env->buffer[buffIndex].p_primitive[index].data, p_env->p_primParam[index]->p_texture->vertex0.vx, p_env->p_primParam[index]->p_texture->vertex0.vy);
	  setRGB0((SPRT *)p_env->buffer[buffIndex].p_primitive[index].data, p_env->p_primParam[index]->color0.r, p_env->p_primParam[index]->color0.g, p_env->p_primParam[index]->color0.b);
	  	  
	  //update abstract primitive with psx parameters created at its creation
	  p_env->p_primParam[index]->vertex0.vz = 1024;
	  
	  p_env->p_primParam[index]->transCoor.vz = 0;
	  
	  p_env->p_primParam[index]->scaleCoor.vx = ONE;
	  p_env->p_primParam[index]->scaleCoor.vy = ONE;
	  p_env->p_primParam[index]->scaleCoor.vz = ONE;
	  break;
	case TYPE_TILE:
	  setTile((TILE *)p_env->buffer[buffIndex].p_primitive[index].data);
	  setXY0((TILE *)p_env->buffer[buffIndex].p_primitive[index].data, p_env->p_primParam[index]->vertex0.vx, p_env->p_primParam[index]->vertex0.vy);
	  setWH((TILE *)p_env->buffer[buffIndex].p_primitive[index].data, p_env->p_primParam[index]->dimensions.w,  p_env->p_primParam[index]->dimensions.h);
	  setRGB0((TILE *)p_env->buffer[buffIndex].p_primitive[index].data, p_env->p_primParam[index]->color0.r, p_env->p_primParam[index]->color0.g, p_env->p_primParam[index]->color0.b);
	  
	  //update abstract primitive with psx parameters created at its creation
	  p_env->p_primParam[index]->vertex0.vz = 1024;
	  
	  p_env->p_primParam[index]->transCoor.vz = 0;
	  
	  p_env->p_primParam[index]->scaleCoor.vx = ONE;
	  p_env->p_primParam[index]->scaleCoor.vy = ONE;
	  p_env->p_primParam[index]->scaleCoor.vz = ONE;
	  break;
	case TYPE_F4:
	  SetPolyF4((POLY_F4 *)p_env->buffer[buffIndex].p_primitive[index].data);
	  setXYWH((POLY_F4 *)p_env->buffer[buffIndex].p_primitive[index].data, p_env->p_primParam[index]->vertex0.vx, p_env->p_primParam[index]->vertex0.vy, p_env->p_primParam[index]->dimensions.w, p_env->p_primParam[index]->dimensions.h);
	  setRGB0((POLY_F4 *)p_env->buffer[buffIndex].p_primitive[index].data, p_env->p_primParam[index]->color0.r, p_env->p_primParam[index]->color0.g, p_env->p_primParam[index]->color0.b);
	  
	  //update abstract primitive with psx parameters created at its creation
	  p_env->p_primParam[index]->vertex0.vz = 1024;
	  
	  p_env->p_primParam[index]->vertex1.vx = ((POLY_F4 *)p_env->buffer[buffIndex].p_primitive[index].data)->x1;
	  p_env->p_primParam[index]->vertex1.vy = ((POLY_F4 *)p_env->buffer[buffIndex].p_primitive[index].data)->y1;
	  p_env->p_primParam[index]->vertex1.vz = 1024;
	  
	  p_env->p_primParam[index]->vertex2.vx = ((POLY_F4 *)p_env->buffer[buffIndex].p_primitive[index].data)->x2;
	  p_env->p_primParam[index]->vertex2.vy = ((POLY_F4 *)p_env->buffer[buffIndex].p_primitive[index].data)->y2;
	  p_env->p_primParam[index]->vertex2.vz = 1024;
	  
	  p_env->p_primParam[index]->vertex3.vx = ((POLY_F4 *)p_env->buffer[buffIndex].p_primitive[index].data)->x3;
	  p_env->p_primParam[index]->vertex3.vy = ((POLY_F4 *)p_env->buffer[buffIndex].p_primitive[index].data)->y3;
	  p_env->p_primParam[index]->vertex3.vz = 1024;
	  
	  p_env->p_primParam[index]->transCoor.vz = 0;
	  
	  p_env->p_primParam[index]->scaleCoor.vx = ONE;
	  p_env->p_primParam[index]->scaleCoor.vy = ONE;
	  p_env->p_primParam[index]->scaleCoor.vz = ONE;
	  break;
	case TYPE_FT4:
	  SetPolyFT4((POLY_FT4 *)p_env->buffer[buffIndex].p_primitive[index].data);
	  setUVWH((POLY_FT4 *)p_env->buffer[buffIndex].p_primitive[index].data, p_env->p_primParam[index]->p_texture->vertex0.vx, p_env->p_primParam[index]->p_texture->vertex0.vy, p_env->p_primParam[index]->p_texture->dimensions.w, p_env->p_primParam[index]->p_texture->dimensions.h);
	  setXYWH((POLY_FT4 *)p_env->buffer[buffIndex].p_primitive[index].data, p_env->p_primParam[index]->vertex0.vx, p_env->p_primParam[index]->vertex0.vy, p_env->p_primParam[index]->dimensions.w, p_env->p_primParam[index]->dimensions.h);
	  setRGB0((POLY_FT4 *)p_env->buffer[buffIndex].p_primitive[index].data, p_env->p_primParam[index]->color0.r, p_env->p_primParam[index]->color0.g, p_env->p_primParam[index]->color0.b);
	  
	  //update abstract primitive with psx parameters created at its creation
	  p_env->p_primParam[index]->vertex0.vz = 1024;
	  
	  p_env->p_primParam[index]->vertex1.vx = ((POLY_FT4 *)p_env->buffer[buffIndex].p_primitive[index].data)->x1;
	  p_env->p_primParam[index]->vertex1.vy = ((POLY_FT4 *)p_env->buffer[buffIndex].p_primitive[index].data)->y1;
	  p_env->p_primParam[index]->vertex1.vz = 1024;
	  
	  p_env->p_primParam[index]->vertex2.vx = ((POLY_FT4 *)p_env->buffer[buffIndex].p_primitive[index].data)->x2;
	  p_env->p_primParam[index]->vertex2.vy = ((POLY_FT4 *)p_env->buffer[buffIndex].p_primitive[index].data)->y2;
	  p_env->p_primParam[index]->vertex2.vz = 1024;
	  
	  p_env->p_primParam[index]->vertex3.vx = ((POLY_FT4 *)p_env->buffer[buffIndex].p_primitive[index].data)->x3;
	  p_env->p_primParam[index]->vertex3.vy = ((POLY_FT4 *)p_env->buffer[buffIndex].p_primitive[index].data)->y3;
	  p_env->p_primParam[index]->vertex3.vz = 1024;
	  
	  p_env->p_primParam[index]->transCoor.vz = 0;
	  
	  p_env->p_primParam[index]->scaleCoor.vx = ONE;
	  p_env->p_primParam[index]->scaleCoor.vy = ONE;
	  p_env->p_primParam[index]->scaleCoor.vz = ONE;
	  break;
	case TYPE_G4:
	  SetPolyG4((POLY_G4 *)p_env->buffer[buffIndex].p_primitive[index].data);
	  setXYWH((POLY_G4 *)p_env->buffer[buffIndex].p_primitive[index].data, p_env->p_primParam[index]->vertex0.vx, p_env->p_primParam[index]->vertex0.vy, p_env->p_primParam[index]->dimensions.w, p_env->p_primParam[index]->dimensions.h);       
	  setRGB0((POLY_G4 *)p_env->buffer[buffIndex].p_primitive[index].data, p_env->p_primParam[index]->color0.r, p_env->p_primParam[index]->color0.g, p_env->p_primParam[index]->color0.b);
	  setRGB1((POLY_G4 *)p_env->buffer[buffIndex].p_primitive[index].data, p_env->p_primParam[index]->color1.r, p_env->p_primParam[index]->color1.g, p_env->p_primParam[index]->color1.b);
	  setRGB2((POLY_G4 *)p_env->buffer[buffIndex].p_primitive[index].data, p_env->p_primParam[index]->color2.r, p_env->p_primParam[index]->color2.g, p_env->p_primParam[index]->color2.b);
	  setRGB3((POLY_G4 *)p_env->buffer[buffIndex].p_primitive[index].data, p_env->p_primParam[index]->color3.r, p_env->p_primParam[index]->color3.g, p_env->p_primParam[index]->color3.b);
	  	  
	  //update abstract primitive with psx parameters created at its creation
	  p_env->p_primParam[index]->vertex0.vz = 1024;
	  
	  p_env->p_primParam[index]->vertex1.vx = ((POLY_G4 *)p_env->buffer[buffIndex].p_primitive[index].data)->x1;
	  p_env->p_primParam[index]->vertex1.vy = ((POLY_G4 *)p_env->buffer[buffIndex].p_primitive[index].data)->y1;
	  p_env->p_primParam[index]->vertex1.vz = 1024;
	  
	  p_env->p_primParam[index]->vertex2.vx = ((POLY_G4 *)p_env->buffer[buffIndex].p_primitive[index].data)->x2;
	  p_env->p_primParam[index]->vertex2.vy = ((POLY_G4 *)p_env->buffer[buffIndex].p_primitive[index].data)->y2;
	  p_env->p_primParam[index]->vertex2.vz = 1024;
	  
	  p_env->p_primParam[index]->vertex3.vx = ((POLY_G4 *)p_env->buffer[buffIndex].p_primitive[index].data)->x3;
	  p_env->p_primParam[index]->vertex3.vy = ((POLY_G4 *)p_env->buffer[buffIndex].p_primitive[index].data)->y3;
	  p_env->p_primParam[index]->vertex3.vz = 1024;
	  
	  p_env->p_primParam[index]->transCoor.vz = 0;
	  
	  p_env->p_primParam[index]->scaleCoor.vx = ONE;
	  p_env->p_primParam[index]->scaleCoor.vy = ONE;
	  p_env->p_primParam[index]->scaleCoor.vz = ONE;
	  break;
	case TYPE_GT4:
	  SetPolyGT4((POLY_GT4 *)p_env->buffer[buffIndex].p_primitive[index].data);
	  setUVWH((POLY_GT4 *)p_env->buffer[buffIndex].p_primitive[index].data, p_env->p_primParam[index]->p_texture->vertex0.vx, p_env->p_primParam[index]->p_texture->vertex0.vy, p_env->p_primParam[index]->p_texture->dimensions.w, p_env->p_primParam[index]->p_texture->dimensions.h);
	  setXYWH((POLY_GT4 *)p_env->buffer[buffIndex].p_primitive[index].data, p_env->p_primParam[index]->vertex0.vx, p_env->p_primParam[index]->vertex0.vy, p_env->p_primParam[index]->dimensions.w, p_env->p_primParam[index]->dimensions.h);      
	  setRGB0((POLY_GT4 *)p_env->buffer[buffIndex].p_primitive[index].data, p_env->p_primParam[index]->color0.r, p_env->p_primParam[index]->color0.g, p_env->p_primParam[index]->color0.b);
	  setRGB1((POLY_GT4 *)p_env->buffer[buffIndex].p_primitive[index].data, p_env->p_primParam[index]->color1.r, p_env->p_primParam[index]->color1.g, p_env->p_primParam[index]->color1.b);
	  setRGB2((POLY_GT4 *)p_env->buffer[buffIndex].p_primitive[index].data, p_env->p_primParam[index]->color2.r, p_env->p_primParam[index]->color2.g, p_env->p_primParam[index]->color2.b);
	  setRGB3((POLY_GT4 *)p_env->buffer[buffIndex].p_primitive[index].data, p_env->p_primParam[index]->color3.r, p_env->p_primParam[index]->color3.g, p_env->p_primParam[index]->color3.b);
	  	  
	  //update abstract primitive with psx parameters created at its creation
	  p_env->p_primParam[index]->vertex0.vz = 1024;
	  
	  p_env->p_primParam[index]->vertex1.vx = ((POLY_GT4 *)p_env->buffer[buffIndex].p_primitive[index].data)->x1;
	  p_env->p_primParam[index]->vertex1.vy = ((POLY_GT4 *)p_env->buffer[buffIndex].p_primitive[index].data)->y1;
	  p_env->p_primParam[index]->vertex1.vz = 1024;
	  
	  p_env->p_primParam[index]->vertex2.vx = ((POLY_GT4 *)p_env->buffer[buffIndex].p_primitive[index].data)->x2;
	  p_env->p_primParam[index]->vertex2.vy = ((POLY_GT4 *)p_env->buffer[buffIndex].p_primitive[index].data)->y2;
	  p_env->p_primParam[index]->vertex2.vz = 1024;
	  
	  p_env->p_primParam[index]->vertex3.vx = ((POLY_GT4 *)p_env->buffer[buffIndex].p_primitive[index].data)->x3;
	  p_env->p_primParam[index]->vertex3.vy = ((POLY_GT4 *)p_env->buffer[buffIndex].p_primitive[index].data)->y3;
	  p_env->p_primParam[index]->vertex3.vz = 1024;
	  
	  p_env->p_primParam[index]->transCoor.vz = 0;
	  
	  p_env->p_primParam[index]->scaleCoor.vx = ONE;
	  p_env->p_primParam[index]->scaleCoor.vy = ONE;
	  p_env->p_primParam[index]->scaleCoor.vz = ONE;
	  break;
	default:
	  printf("\nERROR, NO TYPE DEFINED AT INDEX %d\n", index);
	  break;
      }
      
      transPrim(p_env->p_primParam[index], p_env);
      
      AddPrim(&(p_env->buffer[buffIndex].p_ot[index]), p_env->buffer[buffIndex].p_primitive[index].data);
    }
  }
}

//update native primitives for the play station via matrix math 
void updatePrim(struct s_environment *p_env)
{
  int index;
  long depthCue;
  long flag;
  
  for(index = 0; index < p_env->otSize; index++)
  {
    SetRotMatrix((MATRIX *)&p_env->p_primParam[index]->matrix);
    SetTransMatrix((MATRIX *)&p_env->p_primParam[index]->matrix);
    
    switch(p_env->p_currBuffer->p_primitive[index].type)
    {
      case TYPE_SPRITE:
	RotTransPers((SVECTOR *)&p_env->p_primParam[index]->vertex0,
		     (long *)&((SPRT *)p_env->p_currBuffer->p_primitive[index].data)->x0,
		     &depthCue, &flag);
	setWH((SPRT *)p_env->p_currBuffer->p_primitive[index].data, p_env->p_primParam[index]->dimensions.w,  p_env->p_primParam[index]->dimensions.h);
	setUV0((SPRT *)p_env->p_currBuffer->p_primitive[index].data, p_env->p_primParam[index]->p_texture->vertex0.vx, p_env->p_primParam[index]->p_texture->vertex0.vy);
	setRGB0((SPRT *)p_env->p_currBuffer->p_primitive[index].data, p_env->p_primParam[index]->color0.r, p_env->p_primParam[index]->color0.g, p_env->p_primParam[index]->color0.b);
	break;
      case TYPE_TILE:
	RotTransPers((SVECTOR *)&p_env->p_primParam[index]->vertex0,
		     (long *)&((TILE *)p_env->p_currBuffer->p_primitive[index].data)->x0,
		     &depthCue, &flag);
	setXY0((TILE *)p_env->p_currBuffer->p_primitive[index].data, p_env->p_primParam[index]->vertex0.vx, p_env->p_primParam[index]->vertex0.vy);
	setWH((TILE *)p_env->p_currBuffer->p_primitive[index].data, p_env->p_primParam[index]->dimensions.w,  p_env->p_primParam[index]->dimensions.h);
	setRGB0((TILE *)p_env->p_currBuffer->p_primitive[index].data, p_env->p_primParam[index]->color0.r, p_env->p_primParam[index]->color0.g, p_env->p_primParam[index]->color0.b);
	break;
      case TYPE_F4:
	RotTransPers4((SVECTOR *)&p_env->p_primParam[index]->vertex0, 
		      (SVECTOR *)&p_env->p_primParam[index]->vertex1, 
		      (SVECTOR *)&p_env->p_primParam[index]->vertex2, 
		      (SVECTOR *)&p_env->p_primParam[index]->vertex3,
		      (long *)&(((POLY_F4 *)p_env->p_currBuffer->p_primitive[index].data)->x0),
		      (long *)&(((POLY_F4 *)p_env->p_currBuffer->p_primitive[index].data)->x1),
		      (long *)&(((POLY_F4 *)p_env->p_currBuffer->p_primitive[index].data)->x2),
		      (long *)&(((POLY_F4 *)p_env->p_currBuffer->p_primitive[index].data)->x3),
		      &depthCue, &flag);
	setRGB0((POLY_F4 *)p_env->p_currBuffer->p_primitive[index].data, p_env->p_primParam[index]->color0.r, p_env->p_primParam[index]->color0.g, p_env->p_primParam[index]->color0.b);
	break;
      case TYPE_FT4:
	RotTransPers4((SVECTOR *)&p_env->p_primParam[index]->vertex0,
		      (SVECTOR *)&p_env->p_primParam[index]->vertex1,
		      (SVECTOR *)&p_env->p_primParam[index]->vertex2, 
		      (SVECTOR *)&p_env->p_primParam[index]->vertex3,
		      (long *)(&(((POLY_FT4 *)p_env->p_currBuffer->p_primitive[index].data)->x0)),
		      (long *)(&(((POLY_FT4 *)p_env->p_currBuffer->p_primitive[index].data)->x1)),
		      (long *)(&(((POLY_FT4 *)p_env->p_currBuffer->p_primitive[index].data)->x2)),
		      (long *)(&(((POLY_FT4 *)p_env->p_currBuffer->p_primitive[index].data)->x3)),
		      &depthCue, &flag);
	setUVWH((POLY_FT4 *)p_env->p_currBuffer->p_primitive[index].data, p_env->p_primParam[index]->p_texture->vertex0.vx, p_env->p_primParam[index]->p_texture->vertex0.vy, p_env->p_primParam[index]->p_texture->dimensions.w, p_env->p_primParam[index]->p_texture->dimensions.h);
	setRGB0((POLY_FT4 *)p_env->p_currBuffer->p_primitive[index].data, p_env->p_primParam[index]->color0.r, p_env->p_primParam[index]->color0.g, p_env->p_primParam[index]->color0.b);
	break;
      case TYPE_G4:
	RotTransPers4((SVECTOR *)&p_env->p_primParam[index]->vertex0, 
		      (SVECTOR *)&p_env->p_primParam[index]->vertex1, 
		      (SVECTOR *)&p_env->p_primParam[index]->vertex2, 
		      (SVECTOR *)&p_env->p_primParam[index]->vertex3,
		      (long *)&((POLY_G4 *)p_env->p_currBuffer->p_primitive[index].data)->x0,
		      (long *)&((POLY_G4 *)p_env->p_currBuffer->p_primitive[index].data)->x1,
		      (long *)&((POLY_G4 *)p_env->p_currBuffer->p_primitive[index].data)->x2,
		      (long *)&((POLY_G4 *)p_env->p_currBuffer->p_primitive[index].data)->x3,
		      &depthCue, &flag);      
	setRGB0((POLY_G4 *)p_env->p_currBuffer->p_primitive[index].data, p_env->p_primParam[index]->color0.r, p_env->p_primParam[index]->color0.g, p_env->p_primParam[index]->color0.b);
	setRGB1((POLY_G4 *)p_env->p_currBuffer->p_primitive[index].data, p_env->p_primParam[index]->color1.r, p_env->p_primParam[index]->color1.g, p_env->p_primParam[index]->color1.b);
	setRGB2((POLY_G4 *)p_env->p_currBuffer->p_primitive[index].data, p_env->p_primParam[index]->color2.r, p_env->p_primParam[index]->color2.g, p_env->p_primParam[index]->color2.b);
	setRGB3((POLY_G4 *)p_env->p_currBuffer->p_primitive[index].data, p_env->p_primParam[index]->color3.r, p_env->p_primParam[index]->color3.g, p_env->p_primParam[index]->color3.b);
	break;
      case TYPE_GT4:
	RotTransPers4((SVECTOR *)&p_env->p_primParam[index]->vertex0, 
		      (SVECTOR *)&p_env->p_primParam[index]->vertex1, 
		      (SVECTOR *)&p_env->p_primParam[index]->vertex2, 
		      (SVECTOR *)&p_env->p_primParam[index]->vertex3,
		      (long *)&((POLY_GT4 *)p_env->p_currBuffer->p_primitive[index].data)->x0,
		      (long *)&((POLY_GT4 *)p_env->p_currBuffer->p_primitive[index].data)->x1,
		      (long *)&((POLY_GT4 *)p_env->p_currBuffer->p_primitive[index].data)->x2,
		      (long *)&((POLY_GT4 *)p_env->p_currBuffer->p_primitive[index].data)->x3,
		      &depthCue, &flag);
	setUVWH((POLY_GT4 *)p_env->p_currBuffer->p_primitive[index].data, p_env->p_primParam[index]->p_texture->vertex0.vx, p_env->p_primParam[index]->p_texture->vertex0.vy, p_env->p_primParam[index]->p_texture->dimensions.w, p_env->p_primParam[index]->p_texture->dimensions.h);    
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

//use the abstract primitive to generate its native sister primitives updated coordinates
void transPrim(struct s_primParam *p_primParam, struct s_environment *p_env)
{
  p_primParam->realCoor.vx = p_primParam->transCoor.vx - p_primParam->vertex0.vx - p_env->screenCoor.vx;
  p_primParam->realCoor.vy = p_primParam->transCoor.vy - p_primParam->vertex0.vy - p_env->screenCoor.vy;
  p_primParam->realCoor.vz = p_primParam->transCoor.vz;
  
  RotMatrix((SVECTOR *)&p_primParam->rotCoor, (MATRIX *)&p_primParam->matrix);
  ScaleMatrixL((MATRIX *)&p_primParam->matrix, (VECTOR *)&p_primParam->scaleCoor);
  TransMatrix((MATRIX *)&p_primParam->matrix, (VECTOR *)&p_primParam->realCoor);
}

//generic method for moving a primitive
void movPrim(struct s_environment *p_env)
{ 
  static int prevTime = 0;

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
      p_env->p_primParam[p_env->primCur]->scaleCoor.vx += 512;
      p_env->p_primParam[p_env->primCur]->scaleCoor.vy += 512;
      p_env->prevTime = VSync(-1);
    }
  }
  
  if(p_env->gamePad.one.fourth.bit.triangle == 0)
  {
    if(prevTime == 0 || ((VSync(-1) - prevTime) > 5))
    {
      p_env->p_primParam[p_env->primCur]->rotCoor.vz += 128;
      prevTime = VSync(-1);
    }
  }
  
  if(p_env->gamePad.one.fourth.bit.square == 0)
  {
    if(prevTime == 0 || ((VSync(-1) - prevTime) > 5))
    {
      p_env->p_primParam[p_env->primCur]->transCoor.vz += 32;
      prevTime = VSync(-1);
    }
  }
  
  if(p_env->gamePad.one.third.bit.up == 0)
  {
    if(p_env->p_primParam[p_env->primCur]->transCoor.vy > 0)
    {
      p_env->p_primParam[p_env->primCur]->transCoor.vy -= 1;
    }
  }
  
  if(p_env->gamePad.one.third.bit.right == 0)
  {
    if((p_env->p_primParam[p_env->primCur]->transCoor.vx + p_env->p_primParam[p_env->primCur]->dimensions.w) < SCREEN_WIDTH)
    {
      p_env->p_primParam[p_env->primCur]->transCoor.vx += 1;
    }
  }
  
  if(p_env->gamePad.one.third.bit.down == 0)
  {
    if((p_env->p_primParam[p_env->primCur]->transCoor.vy + p_env->p_primParam[p_env->primCur]->dimensions.h) < SCREEN_HEIGHT)
    {
      p_env->p_primParam[p_env->primCur]->transCoor.vy += 1;
    }
  }
  
  if(p_env->gamePad.one.third.bit.left == 0)
  {
    if(p_env->p_primParam[p_env->primCur]->transCoor.vx > 0)
    {
      p_env->p_primParam[p_env->primCur]->transCoor.vx -= 1;
    }
  }

  transPrim(p_env->p_primParam[p_env->primCur], p_env);
  
  updatePrim(p_env);
}

//read data from a memory card
//BUG card sync seems to not work correcly
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

//write data to card
//BUG card sync seems to not work correcly
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


 
