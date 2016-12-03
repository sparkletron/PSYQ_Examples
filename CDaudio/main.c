/*
 * Written By: John Convertino
 * 
 * Example of using a texture on a primitive, and CD audio.
 * 
 * Move square with D-Pad, press X to change color.
 * 
 */

#include <engine.h>

//game objects creation
void createGameObjects(struct s_environment *p_env);

int main() 
{
  //tracks from disc that contain audio we want (track 1 is data, 2 is silence).
  int tracks[] = {3, 4, 0};
  char *p_title = "Texture Example\nLoaded From CD\nBITMAP to PSX DATA CONV\nAUDIO TRACK\n";
  struct s_environment environment;

  //init environment for engine
  initEnv(&environment, 1);
  
  //needed for sound
  setupSound(&environment);
  
  //set the data and message pointers for the environment
  environment.envMessage.p_data = (int *)&environment.gamePad.one;
  environment.envMessage.p_message = NULL;
  environment.envMessage.p_title = p_title;
  
  createGameObjects(&environment);
  
  populateTextures(&environment);
  
  populateOT(&environment);
  
  playCDtracks(tracks, 0);

  //display and move primitive forever
  for(;;)
  {
    display(&environment);
    movPrim(&environment);
  }

  //no cleanup, we never quit
  return 0;
}

//create objects for our engine
void createGameObjects(struct s_environment *p_env)
{
  int index;
  int buffIndex;
  
  //otSize is the same as the number of primitives
  for(index = 0; index < p_env->otSize; index++)
  {
    //all of them will be based on this type
    p_env->p_primParam[index] = getObjects("\\TEXTURE.XML;1");
 
    if(p_env->p_primParam[index] != NULL)
    {
      //allocate for each buffer its primitive type
      for(buffIndex = 0; buffIndex < DOUBLE_BUF; buffIndex++)
      {
	p_env->buffer[buffIndex].p_primitive[index].data = calloc(1, sizeof(POLY_FT4));
      }
    }
  } 
}