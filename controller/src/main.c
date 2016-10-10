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
  
  initEnv(&environment); // setup the graphics (seen below)
  
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
  
  for(index = 0; index < OT_SIZE; index++)
  {
    p_env->primParam[index].px = 0;
    p_env->primParam[index].py = 0;
    p_env->primParam[index].pw = 50;
    p_env->primParam[index].ph = 50;
    p_env->primParam[index].r0 = rand() % 256;
    p_env->primParam[index].g0 = rand() % 256;
    p_env->primParam[index].b0 = rand() % 256;
    p_env->primParam[index].type = TYPE_F4;
    
    for(buffIndex = 0; buffIndex < DOUBLE_BUF; buffIndex++)
    {
      p_env->buffer[buffIndex].primitive[index].data = calloc(1, sizeof(POLY_F4));
    }
  }
}