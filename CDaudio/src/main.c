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

int main() 
{
  int tracks[] = {1, 0};
  char *p_title = "Texture Example\nLoaded From CD\nBITMAP to PSX DATA CONV\nAUDIO TRACK\n";
  struct s_environment environment;

  initEnv(&environment, 1); // setup the graphics (seen below)
  
  environment.envMessage.p_data = (int *)&environment.gamePad.one;
  environment.envMessage.p_message = NULL;
  environment.envMessage.p_title = p_title;
  
  createGameObjects(&environment);
    
  populateTextures(&environment);
  
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
    p_env->p_primParam[index] = getObjects("\\TEXTURE.XML;1");
 
    if(p_env->p_primParam[index] != NULL)
    {
      for(buffIndex = 0; buffIndex < DOUBLE_BUF; buffIndex++)
      {
	p_env->buffer[buffIndex].p_primitive[index].data = calloc(1, sizeof(POLY_FT4));
      }
    }
  } 
}