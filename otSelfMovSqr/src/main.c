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
void movRandom(struct s_environment *p_env);
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
    movRandom(&environment);
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
  
  for(index = 0; index < p_env->otSize; index++)
  {
    p_env->p_primParam[index].px = (SCREEN_WIDTH - 50) * index;
    p_env->p_primParam[index].py = (SCREEN_HEIGHT - 50) * index;
    p_env->p_primParam[index].pw = 50;
    p_env->p_primParam[index].ph = 50;
    p_env->p_primParam[index].r0 = rand() % 256;
    p_env->p_primParam[index].g0 = rand() % 256;
    p_env->p_primParam[index].b0 = rand() % 256;
    p_env->p_primParam[index].type = TYPE_F4;
    
    for(buffIndex = 0; buffIndex < p_env->bufSize; buffIndex++)
    {
      p_env->buffer[buffIndex].p_primitive[index].data = calloc(1, sizeof(POLY_F4));
    }
  }
}

void movRandom(struct s_environment *p_env)
{ 
  static int prevTime = 0;
  
  static int up = 0;
  static int left = 0;
  static int right = 0;
  static int down = 0;
  
  if((abs(p_env->p_primParam[1].py - p_env->p_primParam[0].py) + 25 < 50) && (abs(p_env->p_primParam[1].px - p_env->p_primParam[0].px) + 25 < 50))
  {
    return;
  }
  
  if((abs(p_env->p_primParam[1].py - p_env->p_primParam[0].py) + 25 < 75) && (abs(p_env->p_primParam[1].px - p_env->p_primParam[0].px) + 25 < 75))
  {
    p_env->p_primParam[0].r0 = 255;
    p_env->p_primParam[0].g0 = 0;
    p_env->p_primParam[0].b0 = 0;
  }
  else
  {
    p_env->p_primParam[0].r0 = 0;
    p_env->p_primParam[0].g0 = 0;
    p_env->p_primParam[0].b0 = 255;
  }
  
  if(prevTime == 0 || ((VSync(-1) - prevTime) > 5))
  {
    prevTime = VSync(-1);
    
    if(p_env->p_primParam[1].py > p_env->p_primParam[0].py)
    {
      up = 0;
      down = 1;
    }
    else
    {
      up = 1;
      down = 0;
    }
    
    if(p_env->p_primParam[1].px > p_env->p_primParam[0].px)
    {
      right = 1;
      left = 0;
    }
    else
    {
      right = 0;
      left = 1;
    }
  }
  
  if(up)
  {
    if(p_env->p_primParam[0].py > 0)
    {
      p_env->p_primParam[0].py -= 1;
    }
  }
  
  if(right)
  {
    if((p_env->p_primParam[0].px + p_env->p_primParam[0].pw) < SCREEN_WIDTH)
    {
      p_env->p_primParam[0].px += 1;
    }
  }
  
  if(down)
  {
    if((p_env->p_primParam[0].py + p_env->p_primParam[0].ph) < SCREEN_HEIGHT)
    {
      p_env->p_primParam[0].py += 1;
    }
  }
  
  if(left)
  {
    if(p_env->p_primParam[0].px > 0)
    {
      p_env->p_primParam[0].px -= 1;
    }
  }
}

void movPlayer(struct s_environment *p_env)
{  
  if(p_env->gamePad.one.fourth.bit.ex == 0)
  {
    if(p_env->prevTime == 0 || ((VSync(-1) - p_env->prevTime) > 60))
    {
      p_env->p_primParam[1].r0 = rand() % 256;
      p_env->p_primParam[1].g0 = rand() % 256;
      p_env->p_primParam[1].b0 = rand() % 256;
      p_env->prevTime = VSync(-1);
    }
  }
  
  if(p_env->gamePad.one.third.bit.up == 0)
  {
    if(p_env->p_primParam[1].py > 0)
    {
      p_env->p_primParam[1].py -= 2;
    }
  }
  
  if(p_env->gamePad.one.third.bit.right == 0)
  {
    if((p_env->p_primParam[1].px + p_env->p_primParam[1].pw) < SCREEN_WIDTH)
    {
      p_env->p_primParam[1].px += 2;
    }
  }
  
  if(p_env->gamePad.one.third.bit.down == 0)
  {
    if((p_env->p_primParam[1].py + p_env->p_primParam[1].ph) < SCREEN_HEIGHT)
    {
      p_env->p_primParam[1].py += 2;
    }
  }
  
  if(p_env->gamePad.one.third.bit.left == 0)
  {
    if(p_env->p_primParam[1].px > 0)
    {
      p_env->p_primParam[1].px -= 2;
    }
  }
}