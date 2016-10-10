/*
 * Written By: John Convertino
 * 
 * Memory Card Read example, will read the phrase written my the memcardWrite example.
*/

#include "engine.h"

int main() 
{
  struct s_environment environment;
  
  initEnv(&environment, 0); // setup the graphics (seen below)
  
  environment.envMessage.p_title = malloc(128);
  environment.envMessage.p_message = malloc(128);
  environment.envMessage.p_data = NULL;
  
  memcpy(environment.envMessage.p_title, "MEMORY CARD WRITE EXAMPLE\nWROTE:", 128);
  memcpy(environment.envMessage.p_message, "I LOVE YOU TOO", 128);
  
  memoryCardWrite(environment.envMessage.p_message);

  while (1) // draw and display forever
  {
    display(&environment);
  }

  return 0;
}
