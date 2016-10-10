/*
 * Written By: John Convertino
 * 
 * Example of using a texture on a primitive.
 * 
 * Move square with D-Pad, press X to change color.
 * 
*/

#include <stdlib.h>
#include <libgte.h>
#include <libgpu.h>
#include <libpad.h>
#include <libetc.h>
#include <libgs.h>

#define SCREEN_WIDTH  320 // screen width
#define	SCREEN_HEIGHT 240 // screen height
#define OT_SIZE       2 //size of ordering table
#define DOUBLE_BUF    2

extern u_long __ramsize;  //  = 0x00200000;  force 2 megabytes of RAM
extern u_long __stacksize; // = 0x00004000; force 16 kilobytes of stack

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
    u_char byte:8;
    
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
    u_char byte:8;
	  
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
    u_char byte:8;
	  
  } fourth;
};

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
  int curPrim;
  int prevPrim;
  int prevTime;
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

void initEnv(struct s_environment *p_env);
void display(struct s_environment *p_env);
struct s_timInfo getTIMinfo(u_long *p_address); 
void populateTPage(struct s_environment *p_env, u_long *p_address[], int len);
void populateOT(struct s_environment *p_env);
void updatePrim(struct s_environment *p_env);
