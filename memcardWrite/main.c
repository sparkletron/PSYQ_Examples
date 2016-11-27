/*
 * Written By: John Convertino
 * 
 * Memory Card Read example, will read the phrase written my the memcardWrite example.
*/

#include "engine.h"

int main() 
{
  char *p_title = "Memory Card Write Example\nWROTE:";
  char *p_message = "I LOVE YOU SO MUCH LONG TIME";
  int len = 0;
  struct s_environment environment;
  
  initEnv(&environment, 0); // setup the graphics (seen below)
  
  len = strlen(p_message);
  
  environment.envMessage.p_title = p_title;
  environment.envMessage.p_message = p_message;
  environment.envMessage.p_data = &len;
  
  memoryCardWrite(p_message, strlen(p_message));

  while (1) // draw and display forever
  {
    display(&environment);
  }

  return 0;
}
