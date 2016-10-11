/*
 * Written By: John Convertino
 * 
 * Ordering Table example with shaded primitives.
 * 
 * Move tail with D-Pad, press start or select to keep the tail from moving back to home.
*/

#include "engine.h"

void movSqrTail(struct s_environment *p_env);
void createGameObjects(struct s_environment *p_env);

int main() 
{
  char *p_title = "Ordering Table Example\nTail\n";
  struct s_environment environment;
  
  initEnv(&environment, 10); // setup the graphics (seen below)
  
  environment.envMessage.p_message = NULL;
  environment.envMessage.p_data = (int *)&environment.gamePad.one;;
  environment.envMessage.p_title = p_title;
  
  createGameObjects(&environment);
  
  populateOT(&environment);

  while (1) // draw and display forever
  {
    display(&environment);
    movSqrTail(&environment);
  }

  return 0;
}

//utility functions
void movUp(struct s_environment *p_env, int len)
{
  int index;
  if(p_env->p_primParam[0].py > 0)
  {
    for(index = len - 2; index >= 0; index--)
    {
      p_env->p_primParam[index].py -= 2 * (len - index - 1);
    }
  }
}

void movDown(struct s_environment *p_env, int len)
{
  int index;
  if((p_env->p_primParam[0].py + p_env->p_primParam[0].ph) < SCREEN_HEIGHT)
  {
    for(index = len - 2; index >= 0; index--)
    {
      p_env->p_primParam[index].py += 2 * (len - index - 1);;
    }
  }
}

void movLeft(struct s_environment *p_env, int len)
{
  int index;
  if(p_env->p_primParam[0].px > 0)
  {
    for(index = len - 2; index >= 0; index--)
    {
      p_env->p_primParam[index].px -= 2 * (len - index - 1);
    }
  }
}

void movRight(struct s_environment *p_env, int len)
{
  int index;
  if((p_env->p_primParam[0].px + p_env->p_primParam[0].pw) < SCREEN_WIDTH)
  {
    for(index = len - 2; index >= 0; index--)
    {
      p_env->p_primParam[index].px += 2 * (len - index - 1);
    }
  }
}

//game functions
void movSqrTail(struct s_environment *p_env)
{
  int index; 
  
  if(p_env->gamePad.one.third.bit.up == 0)
  {
    movUp(p_env, p_env->otSize);
  }
  
  if(p_env->gamePad.one.third.bit.left == 0)
  {
    movLeft(p_env, p_env->otSize);
  }
  
  if(p_env->gamePad.one.third.bit.down == 0)
  {
    movDown(p_env, p_env->otSize);
  }

  if(p_env->gamePad.one.third.bit.right == 0)
  {
    movRight(p_env, p_env->otSize);
  }
  
  if(p_env->gamePad.one.third.byte == 0xFF)
  {
    for(index = 0; index < 2; index++)
    {
      if(p_env->p_primParam[p_env->curPrim].px > SCREEN_WIDTH / 2 - 25)
      {
	movLeft(p_env, p_env->otSize);
      }
      
      if(p_env->p_primParam[p_env->curPrim].px  < SCREEN_WIDTH / 2 - 25)
      {
	movRight(p_env, p_env->otSize);
      }
      
      if(p_env->p_primParam[p_env->curPrim].py < SCREEN_HEIGHT / 2 - 25)
      {
	movDown(p_env, p_env->otSize);
      }
      
      if(p_env->p_primParam[p_env->curPrim].py > SCREEN_HEIGHT / 2 -25)
      {
	movUp(p_env, p_env->otSize);
      }
    }
  }
  
  updatePrim(p_env);
}

void createGameObjects(struct s_environment *p_env)
{
  int index;
  int buffIndex;
  
  for(index = 0; index < p_env->otSize; index++)
  {
    p_env->p_primParam[index].px = SCREEN_WIDTH / 2 - 25;
    p_env->p_primParam[index].py = SCREEN_HEIGHT / 2 - 25;
    p_env->p_primParam[index].pw = 50;
    p_env->p_primParam[index].ph = 50;
    p_env->p_primParam[index].r0 = rand() % 256;
    p_env->p_primParam[index].g0 = rand() % 256;
    p_env->p_primParam[index].b0 = rand() % 256;
    p_env->p_primParam[index].r1 = rand() % 256;
    p_env->p_primParam[index].g1 = rand() % 256;
    p_env->p_primParam[index].b1 = rand() % 256;
    p_env->p_primParam[index].r2 = rand() % 256;
    p_env->p_primParam[index].g2 = rand() % 256;
    p_env->p_primParam[index].b2 = rand() % 256;
    p_env->p_primParam[index].r3 = rand() % 256;
    p_env->p_primParam[index].g3 = rand() % 256;
    p_env->p_primParam[index].b3 = rand() % 256;
    p_env->p_primParam[index].type = TYPE_G4;
    
    for(buffIndex = 0; buffIndex < p_env->bufSize; buffIndex++)
    {
      p_env->buffer[buffIndex].p_primitive[index].data = calloc(1, sizeof(POLY_G4));
    }
  }
}