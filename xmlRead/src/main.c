/*
 *	Test of yxml
 */ 

#include <stdlib.h>
#include <strings.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <libds.h>
#include "yxml.h"

#define SCREEN_WIDTH  320 // screen width
#define	SCREEN_HEIGHT 240 // screen height
#define BUFSIZE 2048

u_long __ramsize   = 0x00200000; // force 2 megabytes of RAM
u_long __stacksize = 0x00004000; // force 16 kilobytes of stack

void *loadFileFromCD(char *p_path);
void graphics();
void display();

char g_arrayBuffer[256][256];
int g_row = 0;

int main()
{
  int fd = 0;
  int index = 0;
  int bufSize = 0;
  char *p_buffer = NULL;
  
  yxml_t yxml;
  yxml_ret_t returnValue;
  
  char stack[BUFSIZE];
  char attrValue[256];
  char string[256];
  
  graphics();
  FntLoad(960, 256); // load the font from the BIOS into VRAM/SGRAM
  SetDumpFnt(FntOpen(5, 20, 320, 240, 0, 512));
  
  p_buffer = (char *)loadFileFromCD("\\TEST.XML;1");
  
  yxml_init(&yxml, stack, BUFSIZE);
  
  do {
    returnValue = yxml_parse(&yxml, *p_buffer);
    
    switch(returnValue)
    {
      case YXML_ELEMSTART:
	strcpy(&g_arrayBuffer[g_row][0], yxml.elem);
	g_row++;
	break;
      case YXML_ATTRSTART:
	strcpy(&g_arrayBuffer[g_row][0], yxml.attr);
	g_row++;
	break;
      case YXML_ATTRVAL:
	if(yxml.data[0] != '\n')
	{
	  attrValue[index] = yxml.data[0];
	  index++;
	  index = index % 256;
	}
	break;
      case YXML_CONTENT:
	if(yxml.data[0] != '\n')
	{
	  string[index] = yxml.data[0];
	  index++;
	  index = index % 256;
	}
	break;
      case YXML_ATTREND:
	strcpy(&g_arrayBuffer[g_row][0], attrValue);
	g_row++;
	index = 0;
	memset(string, 0, 256);
	break;
      case YXML_ELEMEND:
	if(*(p_buffer + 1) != 0)
	{
	  strcpy(&g_arrayBuffer[g_row][0], string);
	  g_row++;
	  index = 0;
	  memset(string, 0, 256);
	}
      default:
	break;
    }
    
    p_buffer++;
  } while(*p_buffer && returnValue >= 0);

  printf("\nDONE WITH XML\n");
  
  for(;;)
  {
    display();
  }
  
  return 0;
}

void *loadFileFromCD(char *p_path)
{
  int sizeSectors = 0;
  int numRemain = 0;
  int prevNumRemain = 0;
  u_char result = 0;
  
  DslFILE fileInfo;
  u_long *file = NULL;
  
  //CD init 
  DsInit();
  
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

  printf("\nREAD COMPLETE\n");
  
  DsClose();
  
  return file;
}

void graphics()
{
  // within the BIOS, if the address 0xBFC7FF52 equals 'E', set it as PAL (1). Otherwise, set it as NTSC (0)
  switch(*(char *)0xbfc7ff52=='E')
  {
    case 'E':
      SetVideoMode(1); 
      break;
    default:
      SetVideoMode(0); 
      break;	
  }

  GsInitGraph(SCREEN_WIDTH, SCREEN_HEIGHT, GsINTER|GsOFSGPU, 1, 0); // set the graphics mode resolutions. You may also try using 'GsNONINTER' (read LIBOVR46.PDF in PSYQ/DOCS for detailed information)
  GsDefDispBuff(0, 0, 0, SCREEN_HEIGHT); // set the top left coordinates of the two buffers in video memory
}


void display()
{
  int index;
  
  GsClearDispArea(0,0,0);
  
  for(index = 0; index < g_row; index++)
  {
    FntPrint("%d, %s\n", index, &g_arrayBuffer[index][0]);
  }
  FntFlush(-1);
  DrawSync(0); // wait for all drawing to finish
  VSync(0); // wait for v_blank interrupt
  GsSwapDispBuff(); // flip the double buffers
}
