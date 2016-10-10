/*
 * Written By: John Convertino
 * 
 * Example of using a texture on a primitive.
 * 
 * Move square with D-Pad, press X to change color.
 * 
*/

#include "engine.h"
#include "image.h"

extern unsigned char e_image[];

void movSqr(struct s_environment *p_env);

int main() 
{
  u_long *p_address[] = {(u_long *)e_image, (u_long *)e_image};
  struct s_environment environment;

  initEnv(&environment); // setup the graphics (seen below)
  
  populateTPage(&environment, p_address, environment.otSize);
  
  populateOT(&environment);

  while (1) // draw and display forever
  {
    display(&environment);
    movSqr(&environment);
  }

  return 0;
}

void movSqr(struct s_environment *p_env)
{  
  if(p_env->gamePad.one.fourth.bit.circle == 0)
  {
    if(p_env->prevTime == 0 || ((VSync(-1) - p_env->prevTime) > 60))
    {
      p_env->curPrim = (p_env->curPrim + 1) % p_env->otSize;
      p_env->prevTime = VSync(-1);
    }
  }
  
  if(p_env->gamePad.one.fourth.bit.ex == 0)
  {
    if(p_env->prevTime == 0 || ((VSync(-1) - p_env->prevTime) > 60))
    {
      p_env->r0[p_env->curPrim] = rand() % 256;
      p_env->g0[p_env->curPrim] = rand() % 256;
      p_env->b0[p_env->curPrim] = rand() % 256;
      p_env->prevTime = VSync(-1);
    }
  }
  
  if(p_env->gamePad.one.third.bit.up == 0)
  {
    if(p_env->py[p_env->curPrim] > 0)
    {
      p_env->py[p_env->curPrim] -= 1;
    }
  }
  
  if(p_env->gamePad.one.third.bit.right == 0)
  {
    if((p_env->px[p_env->curPrim] + p_env->pw[p_env->curPrim]) < SCREEN_WIDTH)
    {
      p_env->px[p_env->curPrim] += 1;
    }
  }
  
  if(p_env->gamePad.one.third.bit.down == 0)
  {
    if((p_env->py[p_env->curPrim] + p_env->ph[p_env->curPrim]) < SCREEN_HEIGHT)
    {
      p_env->py[p_env->curPrim] += 1;
    }
  }
  
  if(p_env->gamePad.one.third.bit.left == 0)
  {
    if(p_env->px[p_env->curPrim] > 0)
    {
      p_env->px[p_env->curPrim] -= 1;
    }
  }
  
  updatePrim(p_env);
}