/*
 * Written By: John Convertino
 * 
 * Example of using a texture on a primitive.
 * 
 * Move square with D-Pad, press X to change color.
 * 
*/

#include <engine.h>

void createGameObjects(struct s_environment *p_env);
void animate(struct s_environment *p_env);

int main() 
{
  char *p_title = "Sprite Example\nLoaded From CD\nBITMAP to PSX DATA CONV";
  struct s_environment environment;

  initEnv(&environment, 2); // setup the graphics (seen below)
  
  environment.envMessage.p_data = (int *)&environment.gamePad.one;
  environment.envMessage.p_message = NULL;
  environment.envMessage.p_title = p_title;
  
  createGameObjects(&environment);
  
  populateOT(&environment);
  
  populateTextures(&environment);

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
  int index;
  int buffIndex;
  
  p_env->p_primParam[0] = getObjects("\\SAND.XML;1");
  p_env->p_primParam[1] = getObjects("\\SPRITE.XML;1");
  
  for(index = 0; index < p_env->otSize; index++)
  {
    if(p_env->p_primParam[index] != NULL)
    {
      for(buffIndex = 0; buffIndex < DOUBLE_BUF; buffIndex++)
      {
	switch(p_env->p_primParam[index]->type)
	{
	  case TYPE_FT4:
	    p_env->buffer[buffIndex].p_primitive[index].data = calloc(1, sizeof(POLY_FT4));
	    break;
	  case TYPE_SPRITE:
	    p_env->buffer[buffIndex].p_primitive[index].data = calloc(1, sizeof(SPRT));
	    break;
	  default:
	    break;
	}
      }
    }
  } 
}

void animate(struct s_environment *p_env)
{
  static int prevTime = 0;
  
  if(p_env->gamePad.one.fourth.bit.triangle == 0)
  {
    prevTime--;
  }
  
  if(prevTime == 0 || ((VSync(-1) - prevTime) >= 8))
  {
    prevTime = VSync(-1);
    
    if(p_env->gamePad.one.third.bit.up == 0)
    {
      p_env->p_primParam[1]->p_texture->vertex0.y = 192;
      p_env->p_primParam[1]->p_texture->vertex0.x = (p_env->p_primParam[1]->p_texture->vertex0.x + 64) % 256;
    }
    else if(p_env->gamePad.one.third.bit.down == 0)
    {
      p_env->p_primParam[1]->p_texture->vertex0.y = 0;
      p_env->p_primParam[1]->p_texture->vertex0.x = (p_env->p_primParam[1]->p_texture->vertex0.x + 64) % 256;
    }
    else if(p_env->gamePad.one.third.bit.right == 0)
    {
      p_env->p_primParam[1]->p_texture->vertex0.y = 128;
      p_env->p_primParam[1]->p_texture->vertex0.x = (p_env->p_primParam[1]->p_texture->vertex0.x + 64) % 256;
    }
    else if(p_env->gamePad.one.third.bit.left == 0)
    {     
      p_env->p_primParam[1]->p_texture->vertex0.y = 64;
      p_env->p_primParam[1]->p_texture->vertex0.x = (p_env->p_primParam[1]->p_texture->vertex0.x + 64) % 256;
    }
    else
    {
      p_env->p_primParam[1]->p_texture->vertex0.x = 0;
    }
  }
}