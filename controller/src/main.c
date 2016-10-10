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

  struct s_environment environment;
  
  initEnv(&environment, 3); // setup the graphics (seen below)
  
  createGameObjects(&environment);
  
  environment.envMessage.p_message = NULL;
  environment.envMessage.p_data = (int *)&environment.gamePad.one;
  environment.envMessage.p_title = malloc(128);
  
  memcpy(environment.envMessage.p_title, "Controller Example", 128);
  
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
    p_env->p_primParam[index].px = 0;
    p_env->p_primParam[index].py = 0;
    p_env->p_primParam[index].pw = 50;
    p_env->p_primParam[index].ph = 50;
    p_env->p_primParam[index].r0 = rand() % 256;
    p_env->p_primParam[index].g0 = rand() % 256;
    p_env->p_primParam[index].b0 = rand() % 256;
    p_env->p_primParam[index].type = TYPE_F4;
    
    for(buffIndex = 0; buffIndex < DOUBLE_BUF; buffIndex++)
    {
      p_env->buffer[buffIndex].p_primitive[index].data = calloc(1, sizeof(POLY_F4));
    }
  }
}