#PSX Graphics Notes

#### Library: libgs.h (extended), libgpu.h (general), libgte.h (basic), libetc.h (Get/SetVideoMode)

#### Example: controller, textureTest, textureTestCD, otMovSq, otTail

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

#### Functions
* MargePrim(), Combines primitives
* SetPoly*(), Macros to set primitives, must be called first as this initializes it.
* SetSemiTrans(), Set semi-transparent primitive.
* SetShadeTex(), Turn Shading on or off.
* SetXY*(), Set Size of the primitive
* setXYWH(), Set Size of the primitive by x0, y0 position with a set width and heigth from that corridinate. 
* SetUW*(), Set Size of texture
* setRGB*(), Set color, usually set 0, but can also have more.
* DrawPrim(), basic function to draw primitives if they are not in the ordering table.

### Examples for Basic Graphics Library (libgte.h)
#### Environment Struct
```
struct s_environment{
  unsigned long ot[OT_SIZE];
  DISPENV disp;
  DRAWENV draw;
};
```

##### Setup Graphics and double buffer
```
void graphics(struct s_environment *p_env)
{
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
  
  //initialize graphics system, and mask display.
  ResetGraph(0);
  
  //set first display buffer
  SetDefDispEnv(&p_env[0].disp, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
  //set second display buffer to the draw buffer of the previous environment
  SetDefDispEnv(&p_env[1].disp, 0, SCREEN_HEIGHT, SCREEN_WIDTH, SCREEN_HEIGHT);
  //set first draw buffer just below the display buffer
  SetDefDrawEnv(&p_env[0].draw, 0, SCREEN_HEIGHT, SCREEN_WIDTH, SCREEN_HEIGHT);
  //set second draw buffer above its display buffer.
  SetDefDrawEnv(&p_env[1].draw, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
  
  //enable screen clearing on draw, set color to black
  p_env[0].draw.isbg = 1;
  p_env[0].draw.r0 = 0;
  p_env[0].draw.g0 = 0;
  p_env[0].draw.b0 = 0;
  
  //do the same for the other environment
  p_env[1].draw.isbg = 1;
  p_env[1].draw.r0 = 0;
  p_env[1].draw.g0 = 0;
  p_env[1].draw.b0 = 0;
  
  //unmask the display
  SetDispMask(1); 
}
```
##### Switching between double buffers.
```
void display(struct s_environment *p_env)
{
  static int currBuff = 0;

  DrawSync(0);
  VSync(0);
  
  //swap buffers
  currBuff = (currBuff + 1) % DUB_BUFFER;
  PutDrawEnv(&p_env[currBuff].draw);
  PutDispEnv(&p_env[currBuff].disp);
  DrawOTag(p_env[currBuff].ot);
}
```

##### Initialize Environemnt Struct and Ordering Table
```
void initEnv(struct s_environment *p_env, int numBuf, POLY_F4 *prim, int len)
{ 
  int index;
  
  for(index = 0; index < numBuf; index++)
  {
      ClearOTag(p_env[index].ot, len);
  }
  
  for(index = 0; index < len; index++)
  {
    SetPolyF4(&prim[index]);
    setRGB0(&prim[index], rand() % 256, rand() % 256, rand() % 256);
    setXY4(&prim[index], 0, 0, 240 / (index + 1), 0, 0, 240 / (index + 1), 240 / (index + 1), 240 / (index + 1));
    AddPrim(&(p_env[0].ot[index]), &prim[index]);
  }
  
  for(index = 0; index < numBuf; index++)
  {
    memcpy((u_char *)p_env[index].ot, (u_char *)p_env[0].ot, len * sizeof(*(p_env[0].ot)));
  }
}
```
 
