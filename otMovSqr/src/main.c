/*
 * Written By: John Convertino
 * 
 * Example of using an ordering table with primitives.
 * 
 * Select Squares by pressing circle, change color with x, and move around with the D-PAD.
 * 
*/

#include <engine.h>

void createGameObjects(struct s_environment *p_env);

int main() 
{
  char *p_title = "Ordering Table Example\nMoving Square";
  struct s_environment environment;
  
  initEnv(&environment, 6); // setup the graphics (seen below)

  createGameObjects(&environment);
  
  environment.envMessage.p_title = p_title;
  environment.envMessage.p_message = NULL;
  environment.envMessage.p_data = (int *)&environment.gamePad.one;
  
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

  struct s_primParam *p_primParam[6] = {NULL, NULL, NULL, NULL, NULL, NULL};
  
  p_primParam[0] = getObjects("\\SQ1.XML;1");
  p_primParam[1] = getObjects("\\SQ2.XML;1");
  p_primParam[2] = getObjects("\\SQ3.XML;1");
  p_primParam[3] = getObjects("\\SQ4.XML;1");
  p_primParam[4] = getObjects("\\SQ5.XML;1");
  p_primParam[5] = getObjects("\\SQ6.XML;1");
  
  for(index = 0; index < p_env->otSize; index++)
  {
    if(p_primParam[index] == NULL)
    {
      continue;
    }
    memcpy(&(p_env->p_primParam[index]), p_primParam[index], sizeof(struct s_primParam));
    
    p_env->p_primParam[index].color0.r = rand() % 256;
    p_env->p_primParam[index].color0.g = rand() % 256;
    p_env->p_primParam[index].color0.b = rand() % 256;
    
    freeObjects(&p_primParam[index]);
    
    for(buffIndex = 0; buffIndex < p_env->bufSize; buffIndex++)
    {
      p_env->buffer[buffIndex].p_primitive[index].data = calloc(1, sizeof(POLY_F4));
    }
  }
}