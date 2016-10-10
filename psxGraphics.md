#PSX Graphics Notes

#### Library: libgs.h (extended), libgpu.h (general), libgte.h (basic), libetc.h (Get/SetVideoMode)

#### Example: controller, textureTest, textureTestCD, otMovSq, otTail, sprite

### Display Modes

#### Interlaced, Non-Interlaced

* Coming Soon

#### Resolutions

* Coming Soon

### Using Extended Graphics Libarary without ordering tables

The extended graphics library takes care of the double buffers behind the scene for display and draw.

#### To setup graphics:

* Check if memory address is equal to character E, if its E, set video mode to PAL, otherwise, NTSC.
  * SetVideoMode() is the function is use to set the above.
* GsInitGraph() set the graphics resolution, and various modes.
* GsDefDispBuff() set top left coordinates. Sets both buffers, display, and draw.

#### Display Grapics:

* GsClearDispArea(), display area is cleared using IO.
  * GetDrawArea(), GetDrawEnv()... etc looks like there are more complicated ways to draw... dispArea seems to be the wrong choice.
* Draw******() are functions that draw a primitive into the draw buffer.
* DrawSync() waits for all drawing to finish.
* VSynch() wait for v_blank interrupt
* GsSwapDispBuff() flips the double buffers.

### Using Basic Graphics Library

Basic Graphics For the PlayStation, requires more setup, but its ordering tables are simpler (less 3D intergrated from the get go).

#### Display environment:

* SetDefDispEnv() to set fields of DISPENV struct.
* PutDispEnv() to make a display environment current.
* GetDispEnv() gets a pointer to the current display environment.

#### Draw environment:

* SetDefDrawEnv() to set fields in DRAWENV struct.
* PutDrawEnv() to make a draw environment current.
* GetDrawEnv() gets a pointer to the current drawing environment.

#### Sync and Reset:
* SetDispMask(), enables (1) or disables (0) the display.
* ResetGraph() resets the graphics system.
	* Has 3 levels:
		* Level 0: Complete reset of graphics system, unsets mask (disable display).
		* Level 1: Cancels current commands and clears queue.
		* Level 3: Same as 0, but the draw and display environments are preserved.
* Sync has two methods:
	* Polling:
		* DrawSync() can block till drawing queue done, or return number of positions in queue.
		* VSync() deteck the next vertical blank period.
	* Callbacks:
		* VSyncCallback() set a callback function to call at beginning of vertical blank period.
		* DrawSyncCallback() set a callback function that is run when a non-blocking drawing operation completes

#### Packet Double Buffer:

* Ordering table (OT) used as packet buffer. 
* CPU registers OT, GPU draws.
* One OT for registering, the other for drawing, double buffer setup.
* MUST use seprate primitives, store the info for them else where.

#### Primitive Rendering Speed

* Check for bottleneck at CPU or GPU
* DrawSync(), immediate return, bottleneck at CPU, else at GPU (measure latency of GPU).

#### Notes
* Display and Drawing environments point to where we are performing operations in memory. When we switch, we are chaning locations, so now the display buffer will point to where the draw buffer was. This means that one set has a set of corridinates, and the other set are set to there opposities. See "Setup Graphics and double buffer" for an example.

### Ordering Tables

#### Basic Usage

* GTE ordering table is defined as an array of unsigned longs
* ClearOTag(), creates the linked list of primitives, must be called first to initialize the ordering table.
* AddPrim(), adds a primitive to the ordering table.
* DrawOTag(), executes list of primitives for drawing.

#### Info

* Ordering Table can have primitives added to it that serve special functions, such as what to draw (gets added to the primitive at a position you choose in the ordering table).
* Primitives are drawn from the first to last, 0 to array size.
* Z axis sorting is available, but I haven't taken a hard look at it yet.

### Primitives

#### Basic Info

* Smallest drawing command for the graphics system is called a primitive (or packet). 
* Two types of primitives
  * Drawing Primitives: Drawn on screen (Poly, line, Sprites, tiles)
  * Special Primitives: Use to change GPU parameters
* Choices
  * Number of sides (3 or 4)
  * Shadings (Gauraud or flat)
  * Texture Mapping (on or off)
  * Line primitives
  * Sprite and Tile Primitives
  * Special Primitives
* Defined as C Structs
* They do mention that by flipping coordinates you can do a vertical and/or horizontal flip. 

#### Functions
* MargePrim(), Combines primitives
* SetPoly*(), Macros to set primitives, must be called first as this initializes it.
* SetSemiTrans(), Set semi-transparent primitive.
* SetShadeTex(), Turn Shading on or off.
* SetXY*(), Set Size of the primitive
* setXYWH(), Set Size of the primitive by x0, y0 position with a set width and heigth from that corridinate. 
* SetUW*(), Set Size of texture
* setUV*(), Set position of primitive in texture frame.
* setRGB*(), Set color, usually set 0, but can also have more.
* DrawPrim(), basic function to draw primitives if they are not in the ordering table.

#### Sprites
* Sprites in basic graphics library does not have an idea of a texture, it has to be added as a DR_TPAGE.
* Easy way to create a moving sprite is to create a texture sheet with frames equally spaced, then move the sprite object to view a frame (cell) of animation in the order needed.
  * This involves using setUV0() to change where the sprite is looking in the texture.

### Examples for Basic Graphics Library (libgte.h)
#### Environment Struct
```
struct s_timInfo
{
  u_short tpage;
  u_short clut;
};

struct s_environment
{
  int currBuff;
  int prevBuff;
  int otSize;
  int bufSize;
  int px[OT_SIZE];
  int py[OT_SIZE];
  int pw[OT_SIZE];
  int ph[OT_SIZE];
  int r0[OT_SIZE];
  int g0[OT_SIZE];
  int b0[OT_SIZE];
  
  struct
  {
    POLY_FT4 primitive[OT_SIZE];
    unsigned long ot[OT_SIZE];
    DISPENV disp;
    DRAWENV draw;
  } buffer[DOUBLE_BUF];
  
  struct
  {
    struct s_gamePad one;
    struct s_gamePad two;
  } gamePad;
  
  struct s_timInfo timInfo[OT_SIZE];
};
```

##### Setup Graphics, double buffer and Ordering Table
```
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
```
##### Switching between double buffers.
```
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
```

##### Populate Ordering Tables
```
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
```
##### Update Primitives With Common Values
```
void movSqr(struct s_environment *p_env)
{
  static int prevTime = 0;
  static int primNum = 0;
  
  if(p_env->gamePad.one.fourth.bit.ex == 0)
  {
    if(prevTime == 0 || ((VSync(-1) - prevTime) > 60))
    {
      p_env->r0[primNum] = rand() % 256;
      p_env->g0[primNum] = rand() % 256;
      p_env->b0[primNum] = rand() % 256;
      prevTime = VSync(-1);
    }
  }
  
  if(p_env->gamePad.one.fourth.bit.circle == 0)
  {
    if(prevTime == 0 || ((VSync(-1) - prevTime) > 60))
    {
      primNum = (primNum + 1) % p_env->otSize;
      prevTime = VSync(-1);
    }
  }
  
  if(p_env->gamePad.one.third.bit.up == 0)
  {
    if(p_env->py[primNum] > 0)
    {
      p_env->py[primNum] -= 1;
    }
  }
  
  if(p_env->gamePad.one.third.bit.right == 0)
  {
    if((p_env->px[primNum] + 50) < SCREEN_WIDTH)
    {
      p_env->px[primNum] += 1;
    }
  }
  
  if(p_env->gamePad.one.third.bit.down == 0)
  {
    if((p_env->py[primNum] + 50) < SCREEN_HEIGHT)
    {
      p_env->py[primNum] += 1;
    }
  }
  
  if(p_env->gamePad.one.third.bit.left == 0)
  {
    if(p_env->px[primNum] > 0)
    {
      p_env->px[primNum] -= 1;
    }
  }
  
  setRGB0(&p_env->buffer[p_env->prevBuff].primitive[primNum], p_env->r0[primNum], p_env->g0[primNum], p_env->b0[primNum]);
  setXYWH(&p_env->buffer[p_env->prevBuff].primitive[primNum], p_env->px[primNum], p_env->py[primNum], p_env->pw[primNum], p_env->ph[primNum]);
}
```
