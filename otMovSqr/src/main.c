/*
 * Written By: John Convertino
 * 
 * Example of using an ordering table with primitives.
 * 
 * Select Squares by pressing circle, change color with x, and move around with the D-PAD.
 * 
*/

#include "engine.h"

void createGameObjects(struct s_environment *p_env);

int main() 
{
  char *p_title = "Ordering Table Example\nMoving Square";
  struct s_environment environment;
  
  initEnv(&environment, 6); // setup the graphics (seen below)

  createGameObjects(&environment);
  
  environment.envMessage.p_title = p_title;
  environment.envMessage.p_message = NULL;
  environment.envMessage.p_data = (int *)&environment.gamePad.one;
  
  populateOT(&environment);

  while (1) // draw and display forever
  {
    display(&environment);
    movPrim(&environment);
  }

  return 0;
}

void createGameObjects(struct s_environment *p_env)
{
  int index;
  int buffIndex;
  
  for(index = 0; index < p_env->otSize; index++)
  {
    p_env->p_primParam[index].vertex0.x = 0;
    p_env->p_primParam[index].vertex0.y = 0;
    p_env->p_primParam[index].primSize.w = 240 / (index + 1);
    p_env->p_primParam[index].primSize.h = 240 / (index + 1);
    p_env->p_primParam[index].color0.r = rand() % 256;
    p_env->p_primParam[index].color0.g = rand() % 256;
    p_env->p_primParam[index].color0.b = rand() % 256;
    p_env->p_primParam[index].type = TYPE_F4;
    
    for(buffIndex = 0; buffIndex < p_env->bufSize; buffIndex++)
    {
      p_env->buffer[buffIndex].p_primitive[index].data = calloc(1, sizeof(POLY_F4));
    }
  }
}