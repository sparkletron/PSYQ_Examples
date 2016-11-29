/*
 * Written By: John Convertino
 * 
 * Example of using an ordering table with primitives, one primitive chases the player controller one.
 * 
 * Change color with x, move with the D-PAD.
 * 
*/

#include "engine.h"

void createGameObjects(struct s_environment *p_env);
void movChase(struct s_environment *p_env);
void movPlayer(struct s_environment *p_env);

int main() 
{
  char *p_title = "Ordering Table Example\nAtari Attack";
  struct s_environment environment;
  
  initEnv(&environment, 2); // setup the graphics (seen below)

  createGameObjects(&environment);
  
  environment.envMessage.p_title = p_title;
  environment.envMessage.p_message = NULL;
  environment.envMessage.p_data = (int *)&environment.gamePad.one;
  
  populateOT(&environment);

  while (1) // draw and display forever
  {
    movChase(&environment);
    movPlayer(&environment);
    updatePrim(&environment);
    display(&environment);
  }

  return 0;
}

void createGameObjects(struct s_environment *p_env)
{
  int index;
  int buffIndex;
  char *fileNames[] = {"\\SQ1.XML;1", "\\SQ2.XML;1"};

  for(index = 0; index < p_env->otSize; index++)
  {
    p_env->p_primParam[index] = getObjects(fileNames[index]);

    if(p_env->p_primParam[index] != NULL)
    {
      for(buffIndex = 0; buffIndex < p_env->bufSize; buffIndex++)
      {
	p_env->buffer[buffIndex].p_primitive[index].data = calloc(1, sizeof(POLY_F4));
      }
    }
  }
}

void movChase(struct s_environment *p_env)
{ 
  if((abs(p_env->p_primParam[1]->transCoor.vy - p_env->p_primParam[0]->transCoor.vy) + 25 < 50) && (abs(p_env->p_primParam[1]->transCoor.vx - p_env->p_primParam[0]->transCoor.vx) + 25 < 50))
  {
    return;
  }
  
  if((abs(p_env->p_primParam[1]->transCoor.vy - p_env->p_primParam[0]->transCoor.vy) + 25 < 75) && (abs(p_env->p_primParam[1]->transCoor.vx - p_env->p_primParam[0]->transCoor.vx) + 25 < 75))
  {
    p_env->p_primParam[0]->color0.r = 255;
    p_env->p_primParam[0]->color0.g = 0;
    p_env->p_primParam[0]->color0.b = 0;
  }
  else
  {
    p_env->p_primParam[0]->color0.r = 0;
    p_env->p_primParam[0]->color0.g = 0;
    p_env->p_primParam[0]->color0.b = 255;
  }
  

  if(p_env->p_primParam[1]->transCoor.vy > p_env->p_primParam[0]->transCoor.vy)
  {
    if((p_env->p_primParam[0]->transCoor.vy + p_env->p_primParam[0]->dimensions.h) < SCREEN_HEIGHT)
    {
      p_env->p_primParam[0]->transCoor.vy += 1;
    }
  }
  else
  {
    if(p_env->p_primParam[0]->transCoor.vy > 0)
    {
      p_env->p_primParam[0]->transCoor.vy -= 1;
    }
  }
  
  if(p_env->p_primParam[1]->transCoor.vx > p_env->p_primParam[0]->transCoor.vx)
  {
    if((p_env->p_primParam[0]->transCoor.vx + p_env->p_primParam[0]->dimensions.w) < SCREEN_WIDTH)
    {
      p_env->p_primParam[0]->transCoor.vx += 1;
    }
  }
  else
  {
    if(p_env->p_primParam[0]->transCoor.vx > 0)
    {
      p_env->p_primParam[0]->transCoor.vx -= 1;
    }
  }
  
  transPrim(p_env->p_primParam[0], p_env);
}

void movPlayer(struct s_environment *p_env)
{  
  if(p_env->gamePad.one.third.bit.up == 0)
  {
    if(p_env->p_primParam[1]->transCoor.vy > 0)
    {
      p_env->p_primParam[1]->transCoor.vy -= 5;
    }
  }
  
  if(p_env->gamePad.one.third.bit.right == 0)
  {
    if((p_env->p_primParam[1]->transCoor.vx + p_env->p_primParam[1]->dimensions.w) < SCREEN_WIDTH)
    {
      p_env->p_primParam[1]->transCoor.vx += 5;
    }
  }
  
  if(p_env->gamePad.one.third.bit.down == 0)
  {
    if((p_env->p_primParam[1]->transCoor.vy + p_env->p_primParam[1]->dimensions.h) < SCREEN_HEIGHT)
    {
      p_env->p_primParam[1]->transCoor.vy += 5;
    }
  }
  
  if(p_env->gamePad.one.third.bit.left == 0)
  {
    if(p_env->p_primParam[1]->transCoor.vx > 0)
    {
      p_env->p_primParam[1]->transCoor.vx -= 5;
    }
  }
  
  transPrim(p_env->p_primParam[1], p_env);
}