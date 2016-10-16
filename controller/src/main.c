/*
 * Written By: John Convertino
 * 
 * Example of using the game pad to move a primitive.
 * 
 * Move square with D-Pad, press X to change color.
 * 
*/

#include "engine.h"

void createGameObjects(struct s_environment *p_env);

int main() 
{
  char *p_title = "Controller Example";
  struct s_environment environment;
  
  initEnv(&environment, 3); // setup the graphics (seen below)
  
  createGameObjects(&environment);
  
  environment.envMessage.p_message = NULL;
  environment.envMessage.p_data = (int *)&environment.gamePad.one;
  environment.envMessage.p_title = p_title;
  
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
    p_env->p_primParam[index].primSize.w = 50;
    p_env->p_primParam[index].primSize.h = 50;
    p_env->p_primParam[index].color0.r = rand() % 256;
    p_env->p_primParam[index].color0.g = rand() % 256;
    p_env->p_primParam[index].color0.b = rand() % 256;
    p_env->p_primParam[index].type = TYPE_F4;
    
    for(buffIndex = 0; buffIndex < DOUBLE_BUF; buffIndex++)
    {
      p_env->buffer[buffIndex].p_primitive[index].data = calloc(1, sizeof(POLY_F4));
    }
  }
}