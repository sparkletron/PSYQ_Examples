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
  char *p_title = "Texture Example\nLoaded From CD\nBITMAP to PSX DATA CONV";
  struct s_environment environment;

  initEnv(&environment, 1); // setup the graphics (seen below)
  
  environment.envMessage.p_data = (int *)&environment.gamePad.one;
  environment.envMessage.p_message = NULL;
  environment.envMessage.p_title = p_title;
  
  createGameObjects(&environment);
  
  populateOT(&environment);
  
  populateTextures(&environment);

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

  struct s_primParam *p_primParam = NULL;
  
  p_primParam = getObjects("\\TEXTURE.XML;1");
 
  if(p_primParam == NULL)
  {
    return;
  }
  
  for(index = 0; index < p_env->otSize; index++)
  {
    memcpy(&(p_env->p_primParam[index]), p_primParam, sizeof(struct s_primParam)); 
    
//     printf("\nALLOCATING\n");
//     
//     p_env->p_primParam[index].p_texture = calloc(1, sizeof(struct s_texture));
//     
//     memcpy(&(p_env->p_primParam[index].p_texture), p_primParam->p_texture, sizeof(struct s_texture));
// 
//     printf("\nCOPY DONE\n");
    
    for(buffIndex = 0; buffIndex < DOUBLE_BUF; buffIndex++)
    {
      printf("\nBUFFER\n");
      p_env->buffer[buffIndex].p_primitive[index].data = calloc(1, sizeof(POLY_FT4));
      printf("\nBUFFER DONE\n");
    }
  }
  
  printf("\nSETUP DONE\n");
  
  freeObjects(&p_primParam);
  
  printf("\nLEAVING\n");
}