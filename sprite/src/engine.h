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
#include <libmcrd.h>
#include <libapi.h>
#include <libds.h>
#include <sys/file.h>

#define SCREEN_WIDTH  320 // screen width
#define	SCREEN_HEIGHT 240 // screen height
#define OT_SIZE       2 //size of ordering table
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
  struct s_primitive primitive[OT_SIZE];
  unsigned long ot[OT_SIZE];
  DISPENV disp;
  DRAWENV draw;
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
    int *p_data;
  } envMessage;
  
  struct
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
  } primParam[OT_SIZE];
  
  struct s_buffer buffer[DOUBLE_BUF];
  
  struct s_buffer *p_drawBuffer;
  struct s_buffer *p_regBuffer;
  
  struct
  {
    struct s_gamePad one;
    struct s_gamePad two;
  } gamePad;
};

void initEnv(struct s_environment *p_env);
void display(struct s_environment *p_env);
u_long *loadTIMfromCD(char *p_path);
struct s_timInfo getTIMinfo(u_long *p_address); 
void populateTPage(struct s_environment *p_env, u_long *p_address[], int len);
void populateOT(struct s_environment *p_env);
void updatePrim(struct s_environment *p_env);
void movPrim(struct s_environment *p_env);
char *memoryCardRead();
void memoryCardWrite(char *p_phrase);
