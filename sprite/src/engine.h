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

#include <stdlib.h>
#include <libgte.h>
#include <libgpu.h>
#include <libpad.h>
#include <libetc.h>
#include <libgs.h>
#include <libmcrd.h>
#include <libapi.h>
#include <libds.h>
#include <sys/file.h>

#define SCREEN_WIDTH  320 // screen width
#define	SCREEN_HEIGHT 240 // screen height
#define DOUBLE_BUF    2

extern u_long __ramsize;  //  = 0x00200000;  force 2 megabytes of RAM
extern u_long __stacksize; // = 0x00004000; force 16 kilobytes of stack

enum en_primType {TYPE_F4, TYPE_FT4, TYPE_G4, TYPE_GT4, TYPE_SPRITE, TYPE_TILE};

struct s_gamePad
{
  struct
  {
    struct
    {
      u_char status:8;
    } byte;
    
  } first;
  
  union
  {
    struct
    {
      u_char recvSize:4;
      u_char type:4;
    } nibble;
    u_char byte;
    
  } second;
  
  union
  {
    struct
    {
      u_char select:1;
      u_char na2:1;
      u_char na1:1;
      u_char start:1;
      u_char up:1;
      u_char right:1;
      u_char down:1;
      u_char left:1;
    } bit;
    u_char byte;
	  
  } third;
  
  union
  {
    struct
    {
      u_char l2:1;
      u_char r2:1;
      u_char l1:1;
      u_char r1:1;
      u_char triangle:1;
      u_char circle:1;
      u_char ex:1;
      u_char square:1;
    } bit;
    u_char byte;
	  
  } fourth;
};

struct s_timInfo
{
  u_short tpageID;
  u_short clutID;
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

struct s_primParam
{
  int px;
  int py;
  int pw;
  int ph;
  int tx;
  int ty;
  int tw;
  int th;
  int r0;
  int g0;
  int b0;
  int r1;
  int g1;
  int b1;
  int r2;
  int g2;
  int b2;
  int r3;
  int g3;
  int b3;
  enum en_primType type;
  DR_TPAGE tpage;
  struct s_timInfo timInfo;
};

struct s_environment
{
  int otSize;
  int bufSize;
  
  int curPrim;
  
  int prevTime;
  
  struct 
  {
    char *p_title;
    char *p_message;
    int  *p_data;
  } envMessage;
  
  struct s_primParam *p_primParam;
  
  struct s_buffer buffer[DOUBLE_BUF];
  
  struct s_buffer *p_drawBuffer;
  struct s_buffer *p_regBuffer;
  
  struct
  {
    struct s_gamePad one;
    struct s_gamePad two;
  } gamePad;
};

//setup environment and set the number of primitives (sets otSize (ordering table size) to this).
void initEnv(struct s_environment *p_env, int numPrim);
//update display
void display(struct s_environment *p_env);
//load a tim from CD, return address to load tim from in memory.
u_long *loadTIMfromCD(char *p_path);
//get tim info
struct s_timInfo getTIMinfo(u_long *p_address); 
//load tim info from memory address and set it as a texture page (must be called after populateOT.
void populateTPage(struct s_environment *p_env, u_long *p_address[], int len);
//call to populate the ordering table with primitives.
void populateOT(struct s_environment *p_env);
//call to update the position of primitives if it has been altered
void updatePrim(struct s_environment *p_env);
//simple move routine to keep primitives within the screen
void movPrim(struct s_environment *p_env);
//The below need TLC, they are fixed to 128 size, more for show than use at this point.
//read from the memory card and return pointer to data
char *memoryCardRead();
//write to the memory card using data passed to it.
void memoryCardWrite(char *p_phrase);
