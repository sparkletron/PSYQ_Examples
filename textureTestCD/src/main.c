/*
 * Written By: John Convertino
 * 
 * Example of using a texture on a primitive.
 * 
 * Move square with D-Pad, press X to change color.
 * 
*/

#include "engine.h"

extern unsigned char e_image[];

void createGameObjects(struct s_environment *p_env);

int main() 
{
  u_long *p_address[2];
  struct s_environment environment;

  initEnv(&environment); // setup the graphics (seen below)
  
  environment.envMessage.p_data = NULL;
  environment.envMessage.p_message = NULL;
  environment.envMessage.p_title = malloc(128);
  
  memcpy(environment.envMessage.p_title, "Texture Example\nLoaded From CD\n", 128);
  
  createGameObjects(&environment);
  
  p_address[0] = loadTIMfromCD("\\TIM\\YAKKO.TIM;1");
  p_address[1] = p_address[0];
  
  populateTPage(&environment, p_address, environment.otSize);
  
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
    p_env->primParam[index].tx = 0;
    p_env->primParam[index].ty = 0;
    p_env->primParam[index].pw = 50;
    p_env->primParam[index].ph = 50;
    p_env->primParam[index].tw = 50;
    p_env->primParam[index].th = 50;
    p_env->primParam[index].r0 = 127;
    p_env->primParam[index].g0 = 127;
    p_env->primParam[index].b0 = 127;
    p_env->primParam[index].type = TYPE_FT4;
    
    for(buffIndex = 0; buffIndex < DOUBLE_BUF; buffIndex++)
    {
      p_env->buffer[buffIndex].primitive[index].data = calloc(1, sizeof(POLY_FT4));
    }
  }
}