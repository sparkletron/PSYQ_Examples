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
  int tracks[] = {1, 2, 0};
  char *p_title = "Texture Example\nLoaded From CD\nWith Audio\n";
  u_long *p_address[2];
  struct s_environment environment;

  initEnv(&environment, 1); // setup the graphics (seen below)
  
  setupSound(&environment);
  
  environment.envMessage.p_data = (int *)&environment.gamePad.one;
  environment.envMessage.p_message = NULL;
  environment.envMessage.p_title = p_title;
  
  createGameObjects(&environment);
  
  p_address[0] = (u_long *)loadFileFromCD("\\TIM\\YAKKO.TIM;1");
  p_address[1] = p_address[0];
  
  populateTPage(&environment, p_address, environment.otSize);
  
  populateOT(&environment);
  
  playCDtracks(tracks);

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
    p_env->p_primParam[index].textureVertex0.x = 0;
    p_env->p_primParam[index].textureVertex0.y = 0;
    p_env->p_primParam[index].primSize.w = 50;
    p_env->p_primParam[index].primSize.h = 50;
    p_env->p_primParam[index].textureSize.w = 50;
    p_env->p_primParam[index].textureSize.h = 50;
    p_env->p_primParam[index].color0.r = 127;
    p_env->p_primParam[index].color0.g = 127;
    p_env->p_primParam[index].color0.b = 127;
    p_env->p_primParam[index].type = TYPE_FT4;
    
    for(buffIndex = 0; buffIndex < p_env->bufSize; buffIndex++)
    {
      p_env->buffer[buffIndex].p_primitive[index].data = calloc(1, sizeof(POLY_FT4));
    }
  }
}