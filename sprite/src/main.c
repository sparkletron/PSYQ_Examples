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
void movSprite(struct s_environment *p_env);
void movEnemy(struct s_environment *p_env);

int main() 
{
  char *p_title = "Sprite Example\nLoaded From CD\nBITMAP to PSX DATA CONV";
  struct s_environment environment;

  initEnv(&environment, 3); // setup the graphics (seen below)
  
  environment.envMessage.p_data = (int *)&environment.gamePad.one;
  environment.envMessage.p_message = NULL;
  environment.envMessage.p_title = p_title;
  
  createGameObjects(&environment);
  
  populateOT(&environment);
  
  populateTextures(&environment);

  while (1) // draw and display forever
  {
    display(&environment);
    movSprite(&environment);
    movEnemy(&environment);
    updatePrim(&environment);
  }

  return 0;
}

void createGameObjects(struct s_environment *p_env)
{
  int index;
  int buffIndex;
  
  p_env->p_primParam[0] = getObjects("\\SAND.XML;1");
  p_env->p_primParam[1] = getObjects("\\SPRITE.XML;1");
  p_env->p_primParam[2] = getObjects("\\ESPRITE.XML;1");
  
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

void animate(struct s_environment *p_env, int *op_prevTime, int sprite, int yoffset)
{
  if(*op_prevTime == 0 || ((VSync(-1) - *op_prevTime) >= 8))
  {
    *op_prevTime = VSync(-1);
    
    p_env->p_primParam[sprite]->p_texture->vertex0.y = yoffset;
    p_env->p_primParam[sprite]->p_texture->vertex0.x = (p_env->p_primParam[sprite]->p_texture->vertex0.x + 64) % 256;
  }
}

void movSprite(struct s_environment *p_env)
{ 
  static int prevTime = 0;
  static int prevAnimTime = 0;
  int movAmount = 1;
  
  if(p_env->gamePad.one.fourth.bit.triangle == 0)
  {
    movAmount = 2;
    prevAnimTime--;
  }
   
  if(p_env->gamePad.one.fourth.bit.ex == 0)
  {
    if(prevTime == 0 || ((VSync(-1) - prevTime) > 60))
    {
      p_env->p_primParam[1]->color0.r = rand() % 256;
      p_env->p_primParam[1]->color0.g = rand() % 256;
      p_env->p_primParam[1]->color0.b = rand() % 256;
      prevTime = VSync(-1);
    }
  }
  else if(p_env->gamePad.one.third.bit.up == 0)
  {
    if(p_env->p_primParam[1]->vertex0.y > 0)
    {
      p_env->p_primParam[1]->vertex0.y -= movAmount;
      animate(p_env, &prevAnimTime, 1, 192);
    }
    else
    {
      p_env->p_primParam[1]->p_texture->vertex0.x = 0;
    }
  }
  else if(p_env->gamePad.one.third.bit.right == 0)
  {
    if((p_env->p_primParam[1]->vertex0.x + p_env->p_primParam[1]->dimensions.w) < SCREEN_WIDTH)
    {
      p_env->p_primParam[1]->vertex0.x += movAmount;
      animate(p_env, &prevAnimTime, 1, 128);
    }
    else
    {
      p_env->p_primParam[1]->p_texture->vertex0.x = 0;
    }
  }
  else if(p_env->gamePad.one.third.bit.down == 0)
  {
    if((p_env->p_primParam[1]->vertex0.y + p_env->p_primParam[1]->dimensions.h) < SCREEN_HEIGHT)
    {
      p_env->p_primParam[1]->vertex0.y += movAmount;
      animate(p_env, &prevAnimTime, 1, 0);
    }
    else
    {
      p_env->p_primParam[1]->p_texture->vertex0.x = 0;
    }
  }
  else if(p_env->gamePad.one.third.bit.left == 0)
  {
    if(p_env->p_primParam[1]->vertex0.x > 0)
    {
      p_env->p_primParam[1]->vertex0.x -= movAmount;
      animate(p_env, &prevAnimTime, 1, 64);
    }
    else
    {
      p_env->p_primParam[1]->p_texture->vertex0.x = 0;
    }
  }
  else
  {
    p_env->p_primParam[1]->p_texture->vertex0.x = 0;
  }
  
}

void movEnemy(struct s_environment *p_env)
{ 
  static int prevAnimTime = 0;
  
  if((abs(p_env->p_primParam[1]->vertex0.y - p_env->p_primParam[0]->vertex0.y) + 25 < 50) && (abs(p_env->p_primParam[1]->vertex0.x - p_env->p_primParam[0]->vertex0.x) + 25 < 50))
  {
    p_env->p_primParam[2]->p_texture->vertex0.x = 0;
    return;
  }

  if(p_env->p_primParam[1]->vertex0.y > p_env->p_primParam[2]->vertex0.y)
  {
    if((p_env->p_primParam[2]->vertex0.y + p_env->p_primParam[2]->dimensions.h) < SCREEN_HEIGHT)
    {
      p_env->p_primParam[2]->vertex0.y += 1;
      animate(p_env, &prevAnimTime, 2, 0);
    }
    else
    {
      p_env->p_primParam[2]->p_texture->vertex0.x = 0;
    }
  }
  else if(p_env->p_primParam[1]->vertex0.y < p_env->p_primParam[2]->vertex0.y)
  {
    if(p_env->p_primParam[2]->vertex0.y > 0)
    {
      p_env->p_primParam[2]->vertex0.y -= 1;
      animate(p_env, &prevAnimTime, 2, 192);
    }
    else
    {
      p_env->p_primParam[2]->p_texture->vertex0.x = 0;
    }
  } 
  else if(p_env->p_primParam[1]->vertex0.x > p_env->p_primParam[2]->vertex0.x)
  {
    if((p_env->p_primParam[2]->vertex0.x + p_env->p_primParam[2]->dimensions.w) < SCREEN_WIDTH)
    {
      p_env->p_primParam[2]->vertex0.x += 1;
      animate(p_env, &prevAnimTime, 2, 128);
    }
    else
    {
      p_env->p_primParam[2]->p_texture->vertex0.x = 0;
    }
  }
  else if(p_env->p_primParam[1]->vertex0.x < p_env->p_primParam[2]->vertex0.x)
  {
    if(p_env->p_primParam[2]->vertex0.x > 0)
    {
      p_env->p_primParam[2]->vertex0.x -= 1;
      animate(p_env, &prevAnimTime, 2, 64);
    }
    else
    {
      p_env->p_primParam[2]->p_texture->vertex0.x = 0;
    }
  }
  else
  {
    p_env->p_primParam[2]->p_texture->vertex0.x = 0;
  }
}