/*
 * Written By: John Convertino
 * 
 * Example of using the game pad to move a primitive.
 * 
 * Move square with D-Pad, press X to change color.
 * 
*/

#include <engine.h>
#include <getprim.h>

void createGameObjects(struct s_environment *p_env);

int main() 
{
  char *p_title = "Controller Example";
  struct s_environment environment;
  
  initEnv(&environment, 3); // setup the graphics (seen below)
  
  createGameObjects(&environment);
  
  environment.envMessage.p_message = NULL;
  environment.envMessage.p_data = (int *)&environment.gamePad.one;
  environment.envMessage.p_title = p_title;
  
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
  char *p_buff = NULL;
  struct s_primParam *p_primParam = NULL;
  
  p_buff = (char *)loadFileFromCD("\\TEST.XML;1");
  
  if(p_buff == NULL)
  {
    printf("\nDATA READ FAILD\n");
    return;
  }
  
  initGetPrimData();
  
  setXMLdata(p_buff);
  
  p_primParam = getPrimData();
  
  printf("DATA: %d %d %d %d %d %d %d\n", p_primParam->vertex0.x, p_primParam->vertex0.y, p_primParam->color0.r, p_primParam->color0.g, p_primParam->color0.b,
	 p_primParam->dimensions.w, p_primParam->dimensions.h);
  
  for(index = 0; index < p_env->otSize; index++)
  {
    memcpy(&(p_env->p_primParam[index]), p_primParam, sizeof(struct s_primParam)); 

//     p_env->p_primParam[index].color0.r = rand() % 256;
//     p_env->p_primParam[index].color0.g = rand() % 256;
//     p_env->p_primParam[index].color0.b = rand() % 256;

    
    for(buffIndex = 0; buffIndex < DOUBLE_BUF; buffIndex++)
    {
      p_env->buffer[buffIndex].p_primitive[index].data = calloc(1, sizeof(POLY_F4));
    }
  }
  
  freePrimData(&p_primParam);
}