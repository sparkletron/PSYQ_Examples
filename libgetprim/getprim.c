/*
 * Started on: 10/21/2016
 * 
 * By: John Convertino
 * 
 * Library for getting primitive data in a cross platform manner. 
 * 
 * 
 */
#include "getprim.h"
#include <string.h>
#include <stdio.h>
#include <yxml.h>

#define DEFAULT_BUFSIZE 2048

//holds data relating to xml parsing
struct
{
  int bufSize;
  char stringBuffer[256];
  
  yxml_t yxml;
  
  char p_stack[DEFAULT_BUFSIZE];
  char const *p_xmlData;
  char const *p_xmlDataBlock;
  char const *p_xmlDataStart;
  
} g_parserData;


//defines of names for xmltypes
#define XML_TYPE_NAME "type"
#define XML_VERTEX_0  "vertex0"
#define XML_VERTEX_1  "vertex1"
#define XML_VERTEX_2  "vertex2"
#define XML_VERTEX_3  "vertex3"
#define XML_VRAM      "vramVertex"
#define XML_X_CORR    "x"
#define XML_Y_CORR    "y"
#define XML_COLOR_0   "color0"
#define XML_COLOR_1   "color1"
#define XML_COLOR_2   "color2"
#define XML_COLOR_3   "color3"
#define XML_RED	      "red"
#define XML_GREEN     "green"
#define XML_BLUE      "blue"
#define XML_WIDTH     "width"
#define XML_HEIGHT    "height"
#define XML_TWIDTH    "twidth"
#define XML_THEIGHT   "theight"
#define XML_TEXTURE   "texture"
#define XML_FILE      "file"

//lookup for prim types, these must be done in the same order as the enum, since I use the index to set the type (ints = enum)
char const * const gc_primType[] = {"TYPE_F4", "TYPE_FT4", "TYPE_G4", "TYPE_GT4", "TYPE_SPRITE", "TYPE_TILE", "END"};

//helper functions
//finds the attribute and stores the result in stringBuffer, returns 0 if found, -1 if not
int findXMLattr(char const * const p_attr);
//finds the element and stores the result in stringBuffer, returns 0 if found, -1 if not
int findXMLelem(char const * const p_elem);
//finds an element name that starts a block of data, returns 0 if found , -1 if not
int findXMLblock(char const * const p_block);
//gets content if attribute or element is found, puts data in stringBuffer, returns 0 if found, -1 if not
int getXMLcontent();
//finds vertex data out of the xml, 0 if found, -1 if not found
int findVertex(struct s_svertex *p_vertex, char const * const p_vertexName);
//finds color data out of the xml, 0 if found, -1 if not found
int findColor(struct s_color *color, char const * const p_colorName);
//sets the current block data pointer so we can loop within the same block of data
void setXMLblock();
//reset data back to start, allows data to be in any order
void resetXMLstart();
//reset data back to where setXMLblock was last called, allows a block to be searched in its range
void resetXMLblock();

//setup get prim data
void initGetPrimData()
{
  g_parserData.p_xmlData = NULL;
  g_parserData.p_xmlDataStart = NULL;
  
  g_parserData.bufSize = DEFAULT_BUFSIZE;
  
  memset(g_parserData.stringBuffer, 0, 256);
  
  yxml_init(&g_parserData.yxml, g_parserData.p_stack, g_parserData.bufSize);
}

//reset get prim
int resetGetPrimData()
{
  memset(g_parserData.stringBuffer, 0, 256);
  
  memset(&g_parserData.yxml, 0, sizeof(g_parserData.yxml));
  
  memset(g_parserData.p_stack, 0, g_parserData.bufSize);
  
  yxml_init(&g_parserData.yxml, g_parserData.p_stack, g_parserData.bufSize);
  
  resetXMLstart();
}

//free data
void freePrimData(struct s_primParam **p_primParam)
{
  if(p_primParam != NULL)
  {
    if((*p_primParam)->p_texture != NULL)
    {
      if((*p_primParam)->p_texture->p_data != NULL)
      {
	free((*p_primParam)->p_texture->p_data);
      }
      
      free((*p_primParam)->p_texture);
    }
    
    free(*p_primParam);
  }
}

//set pointer to data (loaded into pointer else where)
void setXMLdata(char const *p_xmlData)
{
  if(p_xmlData != NULL)
  {
    g_parserData.p_xmlData = p_xmlData;
    g_parserData.p_xmlDataStart = p_xmlData;
  }
}

//get prim data (parse xml)
struct s_primParam *getPrimData()
{
  int index;
  int returnValue = 0;
  
  struct s_primParam *p_primParam;
  
  if(g_parserData.p_xmlData == NULL)
  {
    printf("XML DATA NULL\n");
    return NULL;
  }
  
  p_primParam = calloc(1, sizeof(*p_primParam));
  
  if(p_primParam == NULL)
  {
    printf("BAD ALLOC\n");
    return NULL;
  }
  
  memset(p_primParam, 0, sizeof(*p_primParam));
  
  p_primParam->p_texture = NULL;
  
  if(findXMLattr(XML_TYPE_NAME) < 0)
  {
    printf("DID NOT FIND TYPE NAME\n");
    free(p_primParam);
    return NULL;
  }
  
  for(index = 0; strcmp(gc_primType[index], "END") != 0; index++)
  {
    if(strcmp(gc_primType[index], g_parserData.stringBuffer) == 0)
    {
      p_primParam->type = (enum en_primType)index;
      break;
    }
  }
  
  returnValue = findVertex(&p_primParam->vertex0, XML_VERTEX_0);
  
  if(returnValue < 0)
  {
    printf("COULD NOT FIND VERTEX 0\n");
    free(p_primParam);
    return NULL;
  }
  
  findVertex(&p_primParam->vertex1, XML_VERTEX_1);

  findVertex(&p_primParam->vertex2, XML_VERTEX_2);
  
  findVertex(&p_primParam->vertex3, XML_VERTEX_3);
  
  returnValue = findColor(&p_primParam->color0, XML_COLOR_0);
  
  if(returnValue < 0)
  {
    printf("COULD NOT FIND COLOR 0\n");
    free(p_primParam);
    return NULL;
  }
  
  findColor(&p_primParam->color1, XML_COLOR_1);
  
  findColor(&p_primParam->color1, XML_COLOR_2);
  
  findColor(&p_primParam->color1, XML_COLOR_3);
  
  if(findXMLelem(XML_WIDTH) < 0)
  {
    printf("COULD NOT FIND WIDTH\n");
    free(p_primParam);
    return NULL;
  }

  p_primParam->dimensions.w = atoi(g_parserData.stringBuffer);
  
  resetXMLstart();
  
  if(findXMLelem(XML_HEIGHT) < 0)
  {
    printf("COULD NOT FIND HEIGHT\n");
    free(p_primParam);
    return NULL;
  }
  
  p_primParam->dimensions.h = atoi(g_parserData.stringBuffer);
  
  resetXMLstart();
  
  //get texture info
  if(findXMLblock(XML_TEXTURE) == 0)
  {
    //keep us within this block now that we have found it.
    setXMLblock();
    
    p_primParam->p_texture = calloc(1, sizeof(*p_primParam->p_texture));

    if(p_primParam->p_texture == NULL)
    {
      free(p_primParam);
      return NULL;
    }
    
    returnValue = findVertex(&p_primParam->p_texture->vertex0, XML_VERTEX_0);
    
    if(returnValue < 0)
    {
      printf("COULD NOT FIND VERTEX 0\n");
      free(p_primParam);
      return NULL;
    }
    
    resetXMLblock();
    
    returnValue = findVertex(&p_primParam->p_texture->vramVertex, XML_VRAM);
    
    if(returnValue < 0)
    {
      printf("COULD NOT FIND VRAM\n");
      free(p_primParam);
      return NULL;
    }
    
    resetXMLblock();
      
    if(findXMLelem(XML_TWIDTH) < 0)
    {
      free(p_primParam);
      return NULL;
    }
    
    p_primParam->p_texture->dimensions.w = atoi(g_parserData.stringBuffer);
    
    resetXMLblock();
    
    if(findXMLelem(XML_THEIGHT) < 0)
    {
      free(p_primParam);
      return NULL;
    }
    
    p_primParam->p_texture->dimensions.h = atoi(g_parserData.stringBuffer);
    
    resetXMLblock();
    
    if(findXMLelem(XML_FILE) < 0)
    {
      free(p_primParam);
      return NULL;
    }
    
    strcpy(p_primParam->p_texture->file, g_parserData.stringBuffer);
  }
  
  return p_primParam;
}

//find attributes
int findXMLattr(char const * const p_attr)
{
  yxml_ret_t yxmlState;
  
  do
  {
    yxmlState = yxml_parse(&g_parserData.yxml, *g_parserData.p_xmlData);
    
    switch(yxmlState)
    {
      case YXML_ATTRSTART:
	if(strcmp(g_parserData.yxml.attr, p_attr) == 0)
	{
	  return getXMLcontent();
	}
	break;
      default:
	break;
    }
    g_parserData.p_xmlData++;
  }
  while(*g_parserData.p_xmlData);
  
  return -1;
}

//find elements
int findXMLelem(char const * const p_elem)
{
  yxml_ret_t yxmlState;
  
  do
  {
    yxmlState = yxml_parse(&g_parserData.yxml, *g_parserData.p_xmlData);
    
    switch(yxmlState)
    {
      case YXML_ELEMSTART:
	if(strcmp(g_parserData.yxml.elem, p_elem) == 0)
	{
	  return getXMLcontent();
	}
	break;
      default:
	break;
    }
    g_parserData.p_xmlData++;
  }
  while(*g_parserData.p_xmlData);
  
  return -1;
}

//find element blocks
int findXMLblock(char const * const p_block)
{
  yxml_ret_t yxmlState;

  do
  {
    yxmlState = yxml_parse(&g_parserData.yxml, *g_parserData.p_xmlData);
    
    switch(yxmlState)
    {
      case YXML_ELEMSTART:
	if(strcmp(g_parserData.yxml.elem, p_block) == 0)
	{
	  return 0;
	}
	break;
      default:
	break;
    }
    g_parserData.p_xmlData++;
  }
  while(*g_parserData.p_xmlData);
  
  return -1;
}

//get content when a element match is found (or attribute).
int getXMLcontent()
{
  int len = 0;
  int index = 0;
  yxml_ret_t yxmlState;

  memset(g_parserData.stringBuffer, 0, 256);
  
  do
  {
    yxmlState = yxml_parse(&g_parserData.yxml, *g_parserData.p_xmlData);
    
    switch(yxmlState)
    {
      case YXML_ATTRVAL:
      case YXML_CONTENT:
	switch(g_parserData.yxml.data[0])
	{
	  case '\n':
	  case '>':
	  case '<':
	    break;
	  default:
	    g_parserData.stringBuffer[index] = g_parserData.yxml.data[0];
	    
	    index++;
	    
	    if(index >= 256)
	    {
	      return -1;
	    }
	    break;
	}
	break;
      case YXML_ATTREND:
      case YXML_ELEMEND:
	return 0;
	break;
      default:
	break;
    }
    
    g_parserData.p_xmlData++;
  }
  while(*g_parserData.p_xmlData);
    
  return -1;
}

//find vertex data, helps since this needs to happen 4 or more times
int findVertex(struct s_svertex *p_vertex, char const * const p_vertexName)
{
  if(findXMLblock(p_vertexName) < 0)
  {
    resetXMLstart();
    return -1;
  }
 
  setXMLblock();
 
  if(findXMLelem(XML_X_CORR) < 0)
  {
    resetXMLstart();
    return -1;
  }
  
  p_vertex->vx = atoi(g_parserData.stringBuffer);
  
  resetXMLblock();
  
  if(findXMLelem(XML_Y_CORR) < 0)
  {
    resetXMLstart();
    return -1;
  }
  
  p_vertex->vy = atoi(g_parserData.stringBuffer);
  
  resetXMLstart();
  
  return 0;
}

//find color data, helps since this happens 4 or more times
int findColor(struct s_color *color, char const * const p_colorName)
{
  if(findXMLblock(p_colorName) < 0)
  {
    resetXMLstart();
    return -1;
  }
 
  setXMLblock();
 
  if(findXMLelem(XML_RED) < 0)
  {
    resetXMLstart();
    return -1;
  }
  
  color->r = atoi(g_parserData.stringBuffer);
  
  resetXMLblock();
  
  if(findXMLelem(XML_GREEN) < 0)
  {
    resetXMLstart();
    return -1;
  }
  
  color->g = atoi(g_parserData.stringBuffer);
  
  resetXMLblock();
  
  if(findXMLelem(XML_BLUE) < 0)
  {
    resetXMLstart();
    return -1;
  }
  
  color->b = atoi(g_parserData.stringBuffer);
  
  resetXMLstart();
  
  return 0;
}

//set xml block pointer to current spot
void setXMLblock()
{
  g_parserData.p_xmlDataBlock = g_parserData.p_xmlData;
}

//reset current pointer to start of xml data
void resetXMLstart()
{ 
  setXMLdata(g_parserData.p_xmlDataStart);
}

//reset current pointer back to last setXMLblock call
void resetXMLblock()
{
  g_parserData.p_xmlData = g_parserData.p_xmlDataBlock;
}