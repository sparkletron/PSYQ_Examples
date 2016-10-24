/*
 * Written By: John Convertino
 * 
 * Ordering Table example with shaded primitives.
 * 
 * Move tail with D-Pad, press start or select to keep the tail from moving back to home.
*/

#include <engine.h>

void movSqrTail(struct s_environment *p_env);
void createGameObjects(struct s_environment *p_env);

int main() 
{
  char *p_title = "Ordering Table Example\nTail\n";
  struct s_environment environment;
  
  initEnv(&environment, 10);
  
  environment.envMessage.p_message = NULL;
  environment.envMessage.p_data = (int *)&environment.gamePad.one;;
  environment.envMessage.p_title = p_title;
  
  createGameObjects(&environment);
  
  populateOT(&environment);

  while (1) // draw and display forever
  {
    display(&environment);
    movSqrTail(&environment);
  }

  return 0;
}

//utility functions
void movUp(struct s_environment *p_env, int len)
{
  int index;
  if(p_env->p_primParam[0]->vertex0.y > 0)
  {
    for(index = len - 2; index >= 0; index--)
    {
      p_env->p_primParam[index]->vertex0.y -= 2 * (len - index - 1);
    }
  }
}

void movDown(struct s_environment *p_env, int len)
{
  int index;
  if((p_env->p_primParam[0]->vertex0.y + p_env->p_primParam[0]->dimensions.h) < SCREEN_HEIGHT)
  {
    for(index = len - 2; index >= 0; index--)
    {
      p_env->p_primParam[index]->vertex0.y += 2 * (len - index - 1);;
    }
  }
}

void movLeft(struct s_environment *p_env, int len)
{
  int index;
  if(p_env->p_primParam[0]->vertex0.x > 0)
  {
    for(index = len - 2; index >= 0; index--)
    {
      p_env->p_primParam[index]->vertex0.x -= 2 * (len - index - 1);
    }
  }
}

void movRight(struct s_environment *p_env, int len)
{
  int index;
  if((p_env->p_primParam[0]->vertex0.x + p_env->p_primParam[0]->dimensions.w) < SCREEN_WIDTH)
  {
    for(index = len - 2; index >= 0; index--)
    {
      p_env->p_primParam[index]->vertex0.x += 2 * (len - index - 1);
    }
  }
}

//game functions
void movSqrTail(struct s_environment *p_env)
{
  int index; 
  
  if(p_env->gamePad.one.third.bit.up == 0)
  {
    movUp(p_env, p_env->otSize);
  }
  
  if(p_env->gamePad.one.third.bit.left == 0)
  {
    movLeft(p_env, p_env->otSize);
  }
  
  if(p_env->gamePad.one.third.bit.down == 0)
  {
    movDown(p_env, p_env->otSize);
  }

  if(p_env->gamePad.one.third.bit.right == 0)
  {
    movRight(p_env, p_env->otSize);
  }
  
  if(p_env->gamePad.one.third.byte == 0xFF)
  {
    for(index = 0; index < 2; index++)
    {
      if(p_env->p_primParam[p_env->primCur]->vertex0.x > SCREEN_WIDTH / 2 - 25)
      {
	movLeft(p_env, p_env->otSize);
      }
      
      if(p_env->p_primParam[p_env->primCur]->vertex0.x  < SCREEN_WIDTH / 2 - 25)
      {
	movRight(p_env, p_env->otSize);
      }
      
      if(p_env->p_primParam[p_env->primCur]->vertex0.y < SCREEN_HEIGHT / 2 - 25)
      {
	movDown(p_env, p_env->otSize);
      }
      
      if(p_env->p_primParam[p_env->primCur]->vertex0.y > SCREEN_HEIGHT / 2 -25)
      {
	movUp(p_env, p_env->otSize);
      }
    }
  }
  
  updatePrim(p_env);
}

void createGameObjects(struct s_environment *p_env)
{
  int index;
  int buffIndex;
  
  for(index = 0; index < p_env->otSize; index++)
  {
    p_env->p_primParam[index] = getObjects("\\SQ1.XML;1");

    if(p_env->p_primParam[index] != NULL)
    {
      p_env->p_primParam[index]->color0.r = rand() % 256;
      p_env->p_primParam[index]->color0.g = rand() % 256;
      p_env->p_primParam[index]->color0.b = rand() % 256;
      p_env->p_primParam[index]->color1.r = rand() % 256;
      p_env->p_primParam[index]->color1.g = rand() % 256;
      p_env->p_primParam[index]->color1.b = rand() % 256;
      p_env->p_primParam[index]->color2.r = rand() % 256;
      p_env->p_primParam[index]->color2.g = rand() % 256;
      p_env->p_primParam[index]->color2.b = rand() % 256;
      p_env->p_primParam[index]->color3.r = rand() % 256;
      p_env->p_primParam[index]->color3.g = rand() % 256;
      p_env->p_primParam[index]->color3.b = rand() % 256;
      
      for(buffIndex = 0; buffIndex < p_env->bufSize; buffIndex++)
      {
	p_env->buffer[buffIndex].p_primitive[index].data = calloc(1, sizeof(POLY_G4));
      }
    }
  }
}