/*
 * Written By: John Convertino
 * 
 * Example of using an ordering table with primitives, one primitive chases the player controller one.
 * 
 * Change color with x, move with the D-PAD.
 * 
 */

#include "engine.h"

//create game objects
void createGameObjects(struct s_environment *p_env);
//move routine for chasing block
void movChase(struct s_environment *p_env);
//move routine for player block
void movPlayer(struct s_environment *p_env);

int main() 
{
  char *p_title = "Ordering Table Example\nAtari Attack";
  struct s_environment environment;
  
  initEnv(&environment, 2);

  createGameObjects(&environment);
  
  environment.envMessage.p_title = p_title;
  environment.envMessage.p_message = NULL;
  environment.envMessage.p_data = (int *)&environment.gamePad.one;
  
  populateOT(&environment);

  for(;;)
  {
    movChase(&environment);
    movPlayer(&environment);
    updatePrim(&environment);
    display(&environment);
  }

  return 0;
}

//create game objects
void createGameObjects(struct s_environment *p_env)
{
  int index;
  int buffIndex;
  //list of files to read
  char *fileNames[] = {"\\SQ1.XML;1", "\\SQ2.XML;1"};

  for(index = 0; index < p_env->otSize; index++)
  {
    //get object details from the file
    p_env->p_primParam[index] = getObjects(fileNames[index]);

    if(p_env->p_primParam[index] != NULL)
    {
      //for each buffer create a primitive for each object
      for(buffIndex = 0; buffIndex < p_env->bufSize; buffIndex++)
      {
	p_env->buffer[buffIndex].p_primitive[index].data = calloc(1, sizeof(POLY_F4));
      }
    }
  }
}

//move chase block to chase the other block on scree.
void movChase(struct s_environment *p_env)
{ 
  //if we are in a certain area, stop moving
  if((abs(p_env->p_primParam[1]->transCoor.vy - p_env->p_primParam[0]->transCoor.vy) + 25 < 50) && (abs(p_env->p_primParam[1]->transCoor.vx - p_env->p_primParam[0]->transCoor.vx) + 25 < 50))
  {
    return;
  }
  
  //change color if we are very close
  if((abs(p_env->p_primParam[1]->transCoor.vy - p_env->p_primParam[0]->transCoor.vy) + 25 < 75) && (abs(p_env->p_primParam[1]->transCoor.vx - p_env->p_primParam[0]->transCoor.vx) + 25 < 75))
  {
    p_env->p_primParam[0]->color0.r = 255;
    p_env->p_primParam[0]->color0.g = 0;
    p_env->p_primParam[0]->color0.b = 0;
  }
  else
  {
    p_env->p_primParam[0]->color0.r = 0;
    p_env->p_primParam[0]->color0.g = 0;
    p_env->p_primParam[0]->color0.b = 255;
  }
  
  //move based upon the other blocks position in the vertical.
  if(p_env->p_primParam[1]->transCoor.vy > p_env->p_primParam[0]->transCoor.vy)
  {
    if((p_env->p_primParam[0]->transCoor.vy + p_env->p_primParam[0]->dimensions.h) < SCREEN_HEIGHT)
    {
      p_env->p_primParam[0]->transCoor.vy += 1;
    }
  }
  else
  {
    if(p_env->p_primParam[0]->transCoor.vy > 0)
    {
      p_env->p_primParam[0]->transCoor.vy -= 1;
    }
  }
  
  //move based upon the other blocks position in the horizontal
  if(p_env->p_primParam[1]->transCoor.vx > p_env->p_primParam[0]->transCoor.vx)
  {
    if((p_env->p_primParam[0]->transCoor.vx + p_env->p_primParam[0]->dimensions.w) < SCREEN_WIDTH)
    {
      p_env->p_primParam[0]->transCoor.vx += 1;
    }
  }
  else
  {
    if(p_env->p_primParam[0]->transCoor.vx > 0)
    {
      p_env->p_primParam[0]->transCoor.vx -= 1;
    }
  }
  
  transPrim(p_env->p_primParam[0], p_env);
}

//move player based on controller input
void movPlayer(struct s_environment *p_env)
{  
  if(p_env->gamePad.one.third.bit.up == 0)
  {
    if(p_env->p_primParam[1]->transCoor.vy > 0)
    {
      p_env->p_primParam[1]->transCoor.vy -= 5;
    }
  }
  
  if(p_env->gamePad.one.third.bit.right == 0)
  {
    if((p_env->p_primParam[1]->transCoor.vx + p_env->p_primParam[1]->dimensions.w) < SCREEN_WIDTH)
    {
      p_env->p_primParam[1]->transCoor.vx += 5;
    }
  }
  
  if(p_env->gamePad.one.third.bit.down == 0)
  {
    if((p_env->p_primParam[1]->transCoor.vy + p_env->p_primParam[1]->dimensions.h) < SCREEN_HEIGHT)
    {
      p_env->p_primParam[1]->transCoor.vy += 5;
    }
  }
  
  if(p_env->gamePad.one.third.bit.left == 0)
  {
    if(p_env->p_primParam[1]->transCoor.vx > 0)
    {
      p_env->p_primParam[1]->transCoor.vx -= 5;
    }
  }
  
  transPrim(p_env->p_primParam[1], p_env);
}