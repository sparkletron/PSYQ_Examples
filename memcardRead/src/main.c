/*
 * Written By: John Convertino
 * 
 * Memory Card Read example, will read the phrase written my the memcardWrite example.
*/

#include "engine.h"

int main() 
{
  char *p_title = "Memory Card Read Example\nREAD:";
  struct s_environment environment;
  
  initEnv(&environment, 0); // setup the graphics (seen below)
  
  environment.envMessage.p_title = p_title;
  environment.envMessage.p_message = memoryCardRead();
  environment.envMessage.p_data = (int *)&environment.gamePad.one;

  while (1) // draw and display forever
  {
    display(&environment);
  }

  return 0;
}
