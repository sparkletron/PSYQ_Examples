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
void animate(struct s_environment *p_env);

int main() 
{
  char *p_title = "Sprite Example\nLoaded From Header\n";
  u_long *p_address[] = {(u_long *)e_sand, (u_long *)e_image};
  struct s_environment environment;

  initEnv(&environment, 2); // setup the graphics (seen below)
  
  environment.envMessage.p_data = (int *)&environment.gamePad.one;
  environment.envMessage.p_message = NULL;
  environment.envMessage.p_title = p_title;
  
  createGameObjects(&environment);
  
  populateOT(&environment);
  
  populateTPage(&environment, p_address, environment.otSize);

  while (1) // draw and display forever
  {
    display(&environment);
    animate(&environment);
    movPrim(&environment);
  }

  return 0;
}

void createGameObjects(struct s_environment *p_env)
{
  int buffIndex;
  
  p_env->p_primParam[0].vertex0.x = 0;
  p_env->p_primParam[0].vertex0.y = 0;
  p_env->p_primParam[0].textureVertex0.x = 0;
  p_env->p_primParam[0].textureVertex0.y = 0;
  p_env->p_primParam[0].primSize.w = 320;
  p_env->p_primParam[0].primSize.h = 240;
  p_env->p_primParam[0].textureSize.w = 160;
  p_env->p_primParam[0].textureSize.h = 120;
  p_env->p_primParam[0].color0.r = 127;
  p_env->p_primParam[0].color0.g = 127;
  p_env->p_primParam[0].color0.b = 127;
  p_env->p_primParam[0].type = TYPE_FT4;
  
  for(buffIndex = 0; buffIndex < p_env->bufSize; buffIndex++)
  {
    p_env->buffer[buffIndex].p_primitive[0].data = calloc(1, sizeof(POLY_FT4));
  }
  
  p_env->p_primParam[1].vertex0.x = SCREEN_WIDTH / 2 - 32;
  p_env->p_primParam[1].vertex0.y = SCREEN_HEIGHT / 2 - 32;;
  p_env->p_primParam[1].textureVertex0.x = 0;
  p_env->p_primParam[1].textureVertex0.y = 0;
  p_env->p_primParam[1].primSize.w = 64;
  p_env->p_primParam[1].primSize.h = 64;
  p_env->p_primParam[1].color0.r = 127;
  p_env->p_primParam[1].color0.g = 127;
  p_env->p_primParam[1].color0.b = 127;
  p_env->p_primParam[1].type = TYPE_SPRITE;
  
  for(buffIndex = 0; buffIndex < p_env->bufSize; buffIndex++)
  {
    p_env->buffer[buffIndex].p_primitive[1].data = calloc(1, sizeof(SPRT));
  }
}

void animate(struct s_environment *p_env)
{
  static int prevTime = 0;
  
  if(p_env->gamePad.one.fourth.bit.triangle == 0)
  {
    prevTime--;
  }
  
  if(prevTime == 0 || ((VSync(-1) - prevTime) >= 4))
  {
    prevTime = VSync(-1);
    
    p_env->p_primParam[1].textureVertex0.x = (p_env->p_primParam[1].textureVertex0.x + 64) % 256;
    
    if(p_env->p_primParam[1].textureVertex0.x == 0)
    {
      p_env->p_primParam[1].textureVertex0.y = (p_env->p_primParam[1].textureVertex0.y + 64) % 256;
    }
  }
}