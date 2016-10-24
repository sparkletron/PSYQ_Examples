#PSX Texture Notes

#### Library: libgs.h, libgpu.h, libgte.h

#### Example: textureTest

### Utilitys

#### Conversion

* TIMUTIL.EXE = Converts bitmap (24 bit, no color space info) images to TIM images.
* TIMTOOL.EXE = Check location of TIM image in PlayStation video memory.
* bin2h.exe = Converts TIM images to array of 8 bit values. 

#### Texture Mapping:

* Textures, 2D images mapped onto surfaces.
* Stored in areas of the frame buffer called texture pages.
* Pages are 256 x 256 bitmaps.
* Upper left X coordinate must be a multiple of 64, and Y a multiple of 256 (in frame buffer).
* Three Pixel formats, each primitive can have its own.
	* 4 bit, clut-based, 16 colors, 64 wide.
	* 8 bit, clut-based, 256 colors, 128 wide
		* CLUT is a Color lookup table, also known as a palette.
	* 16 bit, Direct RGB, 32767 colors, 256 wide. (15 bit RGB, 1 semi-transparent status bit). 
* Can be done with primitives and sprites, but sprites do not specify a texture, and must have the current texture page set.

#### Functions

* LoadImage(), transfer data to the frame buffer from memory.
* MoveImage(), transfer data between two places in the frame buffer.
* StoreImage(), transer data from frame buffer to memory.
* GetTpage(), calculates texture page ID and returns it for use.
* GetClut(), use to process color lookup table.
* LoadTPage(), transfer data to the frame buffer from memory. Returns the ID of the texture.
* LoadClut(), transfer CLUT to the frame buffer from memroy. Returns the ID of the clut.
* GsGetTimInfo(), load TIM info from a image in a buffer, this info is put into a GsIMAGE struct.

#### Notes
* When loading offset the pointer by 32 bits (4 bytes) this information is not used by the GsGetTimInfo() function.
* 16 Bit BITMAPS and 16 bit TIM files are very similar, the headers are different, but the data is almost identical.
  * TIM Image data is A1B5G5R5
  * BMP Image data is X1R5G5B5
  * Red and blue are swapped.
  * Only other edition needed is an X, Y for placement in the VRAM.
  * No clut needed for 16 bit image data.

#### Creating a texture

1. Create a 24 bit bitmap no bigger than 256x256 (look above at texture mapping for pixel formats).
2. Start TIMUTIL.EXE in PSYQ.
3. Open bitmap with TIMUTIL.EXE.
4. Set dialog to 4,8, or 16 bit CLUT.
5. Set X, Y position, this sets where in memory the texture exists (set outside the display buffer).
6. Press Convert button, will export bitmap as a TIM (prompts for directory).
7. Open with TIMTOOL.EXE to verify texture.

### Example

##### Loading TIM from CD
```
DslFILE fileInfo;
int sizeSectors = 0;

u_long *image = NULL;

//CD init 
DsInit();

if(DsSearchFile(&fileInfo, "\\TIM\\YAKKO.TIM;1") <= 0)
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
```

##### Using TIM image loaded to cpu memory (move to VRAM).

```
POLY_FT4 f4;
GsIMAGE g_timData;

//setup textured primitive
SetPolyFT4(&f4);
setRGB0(&f4, 128, 128, 128);

//add image to primitive
GsGetTimInfo((u_long *)(image+4), &g_timData);

//load texture page, and return its id to the textured primitve
f4.tpage = LoadTPage(g_timData.pixel, g_timData.pmode, 0, g_timData.px, g_timData.py, g_timData.pw, g_timData.ph);

//load clut, and return its id to the texture primitive
f4.clut = LoadClut(g_timData.clut, g_timData.cx, g_timData.cy);

//can also use setXYWH(), and setUVWH() instead, though the below can allow you to flip the texture.
//set primitive coordinates, and set its texture coordinates
setXY4(&f4, 0, 0, 50, 0, 0, 50, 50, 50);
setUV4(&f4, 0, 0, 50, 0, 0, 50, 50, 50);

```

##### Populate Texture Page Function
```
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
```

##### Load Texture Page Function
```
struct s_timInfo
{
  u_short tpage;
  u_short clut;
};

struct s_timInfo getTIMinfo(u_long *p_address)
{
  struct s_timInfo timInfo;
  GsIMAGE timData;
  
  GsGetTimInfo(p_address+1, &timData);
  
  timInfo.tpage = LoadTPage(timData.pixel, timData.pmode, 0, timData.px, timData.py, timData.pw, timData.ph);
  timInfo.clut = LoadClut(timData.clut, timData.cx, timData.cy);
  
  return timInfo;
}
```
