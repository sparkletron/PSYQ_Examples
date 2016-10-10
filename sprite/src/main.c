/*
 * Written By: John Convertino
 * 
 * Example of using a texture on a primitive.
 * 
 * Move square with D-Pad, press X to change color.
 * 
*/

#include "engine.h"
#include "SPRITE.h"
#include "sand.h"

extern unsigned char e_image[];
extern unsigned char e_sand[];

void createGameObjects(struct s_environment *p_env);
int animate(struct s_environment *p_env);

int main() 
{
  u_long *p_address[] = {(u_long *)e_sand, (u_long *)e_image};
  struct s_environment environment;

  initEnv(&environment, 2); // setup the graphics (seen below)
  
  environment.envMessage.p_data = NULL;
  environment.envMessage.p_message = NULL;
  environment.envMessage.p_title = malloc(128);
  
  memcpy(environment.envMessage.p_title, "Sprite Example\nLoaded From Header\n", 36);
  
  createGameObjects(&environment);
  
  populateOT(&environment);
  
  populateTPage(&environment, p_address, environment.otSize);

  while (1) // draw and display forever
  {
    display(&environment);
    animate(&environment);
  }

  return 0;
}

void createGameObjects(struct s_environment *p_env)
{
  int buffIndex;
  
  p_env->p_primParam[0].px = 0;
  p_env->p_primParam[0].py = 0;
  p_env->p_primParam[0].tx = 0;
  p_env->p_primParam[0].ty = 0;
  p_env->p_primParam[0].pw = 320;
  p_env->p_primParam[0].ph = 240;
  p_env->p_primParam[0].tw = 160;
  p_env->p_primParam[0].th = 120;
  p_env->p_primParam[0].r0 = 127;
  p_env->p_primParam[0].g0 = 127;
  p_env->p_primParam[0].b0 = 127;
  p_env->p_primParam[0].type = TYPE_FT4;
  
  for(buffIndex = 0; buffIndex < p_env->bufSize; buffIndex++)
  {
    p_env->buffer[buffIndex].p_primitive[0].data = calloc(1, sizeof(POLY_FT4));
  }
  
  p_env->p_primParam[1].px = SCREEN_WIDTH / 2 - 32;
  p_env->p_primParam[1].py = SCREEN_HEIGHT / 2 - 32;;
  p_env->p_primParam[1].tx = 0;
  p_env->p_primParam[1].ty = 0;
  p_env->p_primParam[1].pw = 64;
  p_env->p_primParam[1].ph = 64;
  p_env->p_primParam[1].r0 = 127;
  p_env->p_primParam[1].g0 = 127;
  p_env->p_primParam[1].b0 = 127;
  p_env->p_primParam[1].type = TYPE_SPRITE;
  
  for(buffIndex = 0; buffIndex < p_env->bufSize; buffIndex++)
  {
    p_env->buffer[buffIndex].p_primitive[1].data = calloc(1, sizeof(SPRT));
  }
}

int animate(struct s_environment *p_env)
{
  if(p_env->prevTime == 0 || ((VSync(-1) - p_env->prevTime) >= 2))
  {
    p_env->prevTime = VSync(-1);
    
    p_env->p_primParam[1].tx = (p_env->p_primParam[1].tx + 64) % 256;
    
    if(p_env->p_primParam[1].tx == 0)
    {
      p_env->p_primParam[1].ty = (p_env->p_primParam[1].ty + 64) % 256;
    }
  }
  updatePrim(p_env);
}