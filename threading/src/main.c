/*
 * Written By: John Convertino
 * 
 * Example of using the game pad to move a primitive.
 * 
 * Move square with D-Pad, press X to change color.
 * 
*/

#include <engine.h>

void createGameObjects(struct s_environment *p_env);

int main() 
{
  char *p_title = "Controller Example";
  struct s_environment environment;
  
  initEnv(&environment, 1); // setup the graphics (seen below)
  
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

  for(index = 0; index < p_env->otSize; index++)
  {
    p_env->p_primParam[index] = getObjects("\\TEST.XML;1");

    if(p_env->p_primParam[index] != NULL)
    {
      for(buffIndex = 0; buffIndex < DOUBLE_BUF; buffIndex++)
      {
	p_env->buffer[buffIndex].p_primitive[index].data = calloc(1, sizeof(POLY_F4));
      }
    }
  }
}