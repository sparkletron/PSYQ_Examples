/*
 * Written By: John Convertino
 * 
 * Memory Card Read example, will read the phrase written my the memcardWrite example.
*/

#include "engine.h"

int main() 
{
  struct s_environment environment;
  
  initEnv(&environment); // setup the graphics (seen below)
  
  environment.envMessage.p_title = malloc(128);
  environment.envMessage.p_message = memoryCardRead();
  environment.envMessage.p_data = NULL;

  memcpy(environment.envMessage.p_title, "Memory Card Read Example\nREAD:", 128);
  
  while (1) // draw and display forever
  {
    display(&environment);
  }

  return 0;
}
