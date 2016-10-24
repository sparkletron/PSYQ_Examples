/*
 *
 *
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bmpmanip.h"

#define HEADER_SIZE    40
#define BMP_16BIT      0x10
#define BMP_COLOR_MASK 0x1F
//helper functions
//detects bitmap image, if it exists and is of the right type
//this will return the offset, if its not a 16 bit bitmap -1, raw data, 0
int detectBMP(uint8_t const *p_data, int len);
//set semiTrans to 1, will set the color specified to semiTransparent, 0 ignores color and sets all semiTrans
//bits back to 0
void setTransBit(uint8_t *op_data, uint8_t red, uint8_t green, uint8_t blue, uint8_t semiTrans, int len, int bmp);


//sets semi trans bit in image data to one, use on any data, will detect bitmap and go to offset, or just start at beginning for raw data
//0 or greater for success, -1 not a valid 16 bit file
int addSemiTrans(uint8_t *op_data, uint8_t red, uint8_t green, uint8_t blue, int len)
{
  int returnValue = 0;
  
  returnValue = detectBMP(op_data, len);
  
  switch(returnValue)
  {
    case -1:
      break;
    case 0:
    default:
      setTransBit(op_data, red, green, blue, 1, len, returnValue);
      break;
  }
  
  return returnValue;
}

//clears SemiTransBit, use on any data
int removeSemiTrans(uint8_t *op_data, int len)
{
  int returnValue = 0;
  
  returnValue = detectBMP(op_data, len);
  
  switch(returnValue)
  {
    case -1:
      break;
    case 0:
    default:
      setTransBit(op_data, 0, 0, 0, 0, len, returnValue);
      break;
  }
  
  return returnValue;
}

//convert bitmap data to RAW, if no header detected, or incorrect header, this does nothing.
//RAW data is converted to ABGR (MSB to LSB) from the bmp ARGB (16 bit images only)
int bitmapToRAW(uint8_t **op_data, int len)
{
  int returnValue = 0;
  
  uint8_t *p_temp = NULL;
  
  returnValue = detectBMP(*op_data, len);

  switch(returnValue)
  {
    case -1:
      break;
    case 0:
      break;
    default:
      memmove(*op_data, &((*op_data)[returnValue]), len - returnValue);
      p_temp = realloc(*op_data, len - returnValue);
      if(p_temp != NULL)
      {
	*op_data = p_temp;
	returnValue = len - returnValue;
      }
      else
      {
	returnValue = -1;
      }
      break;
  }

  return returnValue;
  
}

int swapRedBlue(uint8_t *op_data, int len)
{
  int index = 0;
  
  int returnValue = 0;
  
  //is not raw data, detected bitmap
  returnValue = detectBMP(op_data, len);

  if(returnValue < 0)
  {
    return -1;
  }
  
  for(index = returnValue; index < len; index += 2)
  {
    uint8_t top = 0;
    uint8_t middle = 0;
    uint8_t bottom = 0;
    uint8_t semiTransD = 0;
    uint16_t data = 0;

    data = ((op_data[index+1] << 8) | op_data[index]);

    semiTransD = (data & 0x8000) >> 15;
    top = (data & 0x7C00) >> 10;
    middle = (data & 0x03E0) >> 5;
    bottom = (data & 0x001F);

    data = 0;

    data |= (semiTransD << 15) | (bottom << 10) | (middle << 5) | top;

    op_data[index] = data & 0x00FF;

    op_data[index+1] = (data >> 8) & 0x00FF;
  }
  
  return returnValue;
}

//swap bytes around, works with bitmap or raw data
//0 or greater success, -1 failure
int reverseData(uint8_t *op_data, int len)
{
  int index;
  int halfLen = 0;
  int returnValue = 0;
  
  returnValue = detectBMP(op_data, len);
  
  switch(returnValue)
  {
    case -1:
      break;
    case 0:
    default:
      halfLen = (len - returnValue)/2;
      for(index = returnValue; index < halfLen; index++)
      {
	uint8_t tempData = op_data[index];
	op_data[index] = op_data[len - index - 1];
	op_data[len - index - 1] = tempData;
      }
      break;
  }
  
  return returnValue;
}

//detects bitmap image, if it exists and is of the right type
//this will return the offset, if its not a 16 bit bitmap -1, raw data, 0
int detectBMP(uint8_t const *p_data, int len)
{
  int index = 0;
  int dataOffset = 0;
  //not enough data to be a bitmap, must be larger than the header
  if(len < HEADER_SIZE)
  {
    return 0;
  }
  
  for(index = 0; index < HEADER_SIZE; index++)
  {
    uint8_t data = p_data[index];
    
    switch(index)
    {
      case 0:
	if(p_data[index] != 'B')
	{
	  return 0;
	}
	break;
      case 1:
	if(p_data[index] != 'M')
	{
	  return 0;
	}
	break;
      case 10:
      case 11:
      case 12:
      case 13:
	dataOffset |= p_data[index] << ((index - 10) * 8);
	break;
      case 28:
	if(p_data[index] != BMP_16BIT)
	{
	  return -1;
	}
	break;
      default:
	break;
    }
  }
  
  return dataOffset;
}

//set semiTrans to 1, will set the color specified to semiTransparent, 0 ignores color and sets all semiTrans
//bits back to 0
void setTransBit(uint8_t *op_data, uint8_t red, uint8_t green, uint8_t blue, uint8_t semiTrans, int len, int bmp)
{
  int index = 0;
  
  for(index = bmp; index < len; index += 2)
  {
    uint8_t redD = 0;
    uint8_t greenD = 0;
    uint8_t blueD = 0;
    uint16_t data = 0;

    data = ((op_data[index+1] << 8) | op_data[index]);
    
    if(bmp > 0)
    {
      redD = (data & 0x7C00) >> 10;
      greenD = (data & 0x03E0) >> 5;
      blueD = (data & 0x001F);
    }
    else
    {
      blueD = (data & 0x7C00) >> 10;
      greenD = (data & 0x03E0) >> 5;
      redD = (data & 0x001F);
    }

    if((semiTrans == 0) || ((red & BMP_COLOR_MASK) == redD) && ((blue & BMP_COLOR_MASK) == blueD) && ((green & BMP_COLOR_MASK) == greenD))
    {
      op_data[index+1] |= (semiTrans & 0x01) << 7;
      op_data[index+1] &= (((semiTrans & 0x01) << 7) | 0x7F);
    }
  }
}