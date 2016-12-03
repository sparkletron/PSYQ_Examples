/*
 * Written By: John Convertino
 * 
 * Example of using a texture on a primitive.
 * 
 * Move square with D-Pad, press X to change color.
 * 
 */

#include <engine.h>

//create game objects
void createGameObjects(struct s_environment *p_env);

int main() 
{
  char *p_title = "Texture Example\nLoaded From CD\nBITMAP to PSX DATA CONV";
  struct s_environment environment;

  initEnv(&environment, 1);
  
  environment.envMessage.p_data = (int *)&environment.gamePad.one;
  environment.envMessage.p_message = NULL;
  environment.envMessage.p_title = p_title;
  
  createGameObjects(&environment);
    
  populateTextures(&environment);
  
  populateOT(&environment);

  for(;;)
  {
    display(&environment);
    movPrim(&environment);
  }

  return 0;
}

//create game objects
void createGameObjects(struct s_environment *p_env)
{
  int index;
  int buffIndex;
  
  //for the number of objects, create them all based on the same xml file
  for(index = 0; index < p_env->otSize; index++)
  {
    p_env->p_primParam[index] = getObjects("\\TEXTURE.XML;1");
 
    if(p_env->p_primParam[index] != NULL)
    {
      //create primtive in both buffers for object.
      for(buffIndex = 0; buffIndex < DOUBLE_BUF; buffIndex++)
      {
	p_env->buffer[buffIndex].p_primitive[index].data = calloc(1, sizeof(POLY_FT4));
      }
    }
  } 
}