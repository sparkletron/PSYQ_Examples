#PSX Graphics Notes

#### Library: libgs.h (extended), libgpu.h (general), libgte.h (basic), libetc.h (Get/SetVideoMode)

#### Example: controller, textureTest, textureTestCD, otMovSq, otTail, sprite

### Display Modes

#### Interlaced, Non-Interlaced

* Interlaced mode allows for the same frame rate but every other line is drawn each time.
  * Even lines, next Vsync, Odd Lines
  * Set interlace mode by setting isinter to 1
    * Set automatically for a heigth of 480
      * Otherwise double buffering will be impossible

#### Resolutions

* Display Area has these modes:
  * Width of 256, 320, 360, 512, or 640.
  * Height of 240 or 480.
    * 480 Height, automatically interlaced, switch between lines becomes double buffer.
* Screen Area can specifiy the area actually show.
  * Standard of (0,0) to (256, 240)
  * Smaller Underscan, larger, overscan
  * Width can not be set to anything greater than 256.
  * Pixel size does not change
* 24-Bit mode flag exists in the display environment variable
  * variable name: isrgb24
  * set to 1 to enable 24 bit mode instead of 16 bit.

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
* isbg, in the draw structure turns draw screen clearing off or on. 0 its off, 1 its on.

### Ordering Tables and Z Sorting

#### Basic Usage

* GTE ordering table is defined as an array of unsigned longs
* ClearOTag(), creates the linked list of primitives, must be called first to initialize the ordering table.
* Clear0TagR(), same as above but deals with lists that have been z sorted, 0 is closer and the end of the list is farthest, opposite of how how the ordering table is drawn. (last on top)
* AddPrim(), adds a primitive to the ordering table.
* DrawOTag(), executes list of primitives for drawing.

#### Info

* Ordering Table can have primitives added to it that serve special functions, such as what to draw (gets added to the primitive at a position you choose in the ordering table).
* Primitives are drawn from the first to last, 0 to ordering table size. Meaning the last one drawn is on top.
* Z axis sorting is available, based on the order of the ordering table
  * primitives can be linked in a list to form primitives at the same level (tag option).
  * addPrim does this automatically if you add primitives to the same ordering table location.
  * RotTransPers(), performs coordinate and persepective transformation.
  * see psxGeometry.md for geometry info.
* You may add primitives at any position, you can skip at any position.

### Primitives

#### Basic Info

* Can be linked together, useful when z sorting.
  * Can be used to keep a group above anthor in the ordering table.
  * tag is the member used for linking.
  * Can link in things such as texture pages for sprites to set the texture.
  * Manualy link or add anthor primitive to the ordering table in the same location as the previous
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
//vertex is old, should be renamed to vector, left to save myself from rewriting (moved on to metis engine).
enum en_primType {TYPE_F4, TYPE_FT4, TYPE_G4, TYPE_GT4, TYPE_SPRITE, TYPE_TILE};

struct s_gamePad
{
  uint8_t status:8;

  uint8_t recvSize:4;
  uint8_t type:4;
  
  uint8_t select:1;
  uint8_t na2:1;
  uint8_t na1:1;
  uint8_t start:1;
  uint8_t up:1;
  uint8_t right:1;
  uint8_t down:1;
  uint8_t left:1;

  uint8_t l2:1;
  uint8_t r2:1;
  uint8_t l1:1;
  uint8_t r1:1;
  uint8_t triangle:1;
  uint8_t circle:1;
  uint8_t ex:1;
  uint8_t square:1;
};

struct s_xmlData
{
  char string[256];
};

struct s_primitive
{
  void *data;
  enum en_primType type;
};

struct s_buffer
{
  struct s_primitive *p_primitive;
  unsigned long *p_ot;
  DISPENV disp;
  DRAWENV draw;
};

struct s_svertex
{
  int16_t vx;
  int16_t vy;
  int16_t vz;
  int16_t pad;
};

struct s_lvertex
{
  int32_t vx;
  int32_t vy;
  int32_t vz;
  int32_t pad;
};

struct s_matrix
{
  int16_t m[3][3];
  int32_t t[3];
};

struct s_dimensions
{
  uint32_t w;
  uint32_t h;
};

struct s_color
{
  uint8_t r;
  uint8_t g;
  uint8_t b;
};

struct s_tpage
{
  unsigned long *tag;
  unsigned long code[2];
};

struct s_texture
{
  unsigned short id;
  uint32_t size;
  
  char file[256];
  
  struct s_svertex vertex0;
  struct s_svertex vramVertex;
  struct s_dimensions dimensions;
  struct s_tpage tpage;
  
  uint8_t *p_data;
};

struct s_primParam
{
  enum en_primType type;
  
  struct s_lvertex transCoor;
  struct s_lvertex scaleCoor;
  struct s_svertex rotCoor;
  
  struct s_lvertex realCoor;
  
  struct s_matrix matrix;
  
  struct s_svertex vertex0;
  struct s_svertex vertex1;
  struct s_svertex vertex2;
  struct s_svertex vertex3;
  
  struct s_color color0;
  struct s_color color1;
  struct s_color color2;
  struct s_color color3;
  
  struct s_dimensions dimensions;
  
  struct s_texture *p_texture;
};

struct s_environment
{
  int primCur;
  int primSize;
  int otSize;
  int bufSize;
  int prevTime;
  
  struct s_primParam **p_primParam;
  
  struct s_buffer buffer[DOUBLE_BUF];
  
  struct s_buffer *p_currBuffer;
  
  struct 
  {
    char *p_title;
    char *p_message;
    int  *p_data;
  } envMessage;
  
  struct
  {
    struct s_gamePad one;
    struct s_gamePad two;
  } gamePad;
  
  SpuCommonAttr soundAttr;
};

```

##### Setup Graphics, double buffer and Ordering Table
```
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
  switch(*(char *)0xbfc7ff52)
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
```
##### Switching between double buffers.
```
void swapBuffers(struct s_environment *p_env)
{
  struct s_buffer *tempBuffer = p_env->p_drawBuffer;
  
  p_env->p_drawBuffer = p_env->p_regBuffer;
  
  p_env->p_regBuffer = tempBuffer;
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
```

##### Populate Ordering Tables
```
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
```
##### Update Primitives With Common Values
```
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
```
##### Move a primitive via translation
```
void transPrim(struct s_primParam *p_primParam)
{
  p_primParam->realCoor.vx = p_primParam->transCoor.vx - p_primParam->vertex0.vx;
  p_primParam->realCoor.vy = p_primParam->transCoor.vy - p_primParam->vertex0.vy;
  p_primParam->realCoor.vz = p_primParam->transCoor.vz;
  
  RotMatrix((SVECTOR *)&p_primParam->rotCoor, (MATRIX *)&p_primParam->matrix);
  ScaleMatrixL((MATRIX *)&p_primParam->matrix, (VECTOR *)&p_primParam->scaleCoor);
  TransMatrix((MATRIX *)&p_primParam->matrix, (VECTOR *)&p_primParam->realCoor);
}

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

  transPrim(p_env->p_primParam[p_env->primCur]);
  
  updatePrim(p_env);
}
```
