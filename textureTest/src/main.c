/*
 * Written By: John Convertino
 * 
 * Example of using a texture on a primitive.
 * 
 * Move square with D-Pad, press X to change color.
 * 
*/

#include "engine.h"
#include "image.h"

extern unsigned char e_image[];

void createGameObjects(struct s_environment *p_env);

int main() 
{
  u_long *p_address[] = {(u_long *)e_image, (u_long *)e_image};
  struct s_environment environment;

  initEnv(&environment, 2); // setup the graphics (seen below)
  
  environment.envMessage.p_data = NULL;
  environment.envMessage.p_message = NULL;
  environment.envMessage.p_title = malloc(128);
  
  memcpy(environment.envMessage.p_title, "Texture Example\nLoaded From Header\n", 128);
  
  createGameObjects(&environment);
  
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
  
  for(index = 0; index < p_env->otSize; index++)
  {
    p_env->p_primParam[index].px = 0;
    p_env->p_primParam[index].py = 0;
    p_env->p_primParam[index].tx = 0;
    p_env->p_primParam[index].ty = 0;
    p_env->p_primParam[index].pw = 50;
    p_env->p_primParam[index].ph = 50;
    p_env->p_primParam[index].tw = 50;
    p_env->p_primParam[index].th = 50;
    p_env->p_primParam[index].r0 = 127;
    p_env->p_primParam[index].g0 = 127;
    p_env->p_primParam[index].b0 = 127;
    p_env->p_primParam[index].type = TYPE_FT4;
    
    for(buffIndex = 0; buffIndex < p_env->bufSize; buffIndex++)
    {
      p_env->buffer[buffIndex].p_primitive[index].data = calloc(1, sizeof(POLY_FT4));
    }
  }
}