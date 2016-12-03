/*
 * Written By: John Convertino
 * 
 * Memory Card Write example, will write the phrase tp be read by the memcardRead example.
 * 
 */

#include "engine.h"

int main() 
{
  char *p_title = "Memory Card Write Example\nWROTE:";
  char *p_message = "I LOVE YOU SO MUCH LONG TIME";
  int len = 0;
  struct s_environment environment;
  
  initEnv(&environment, 0);
  
  len = strlen(p_message);
  
  environment.envMessage.p_title = p_title;
  environment.envMessage.p_message = p_message;
  environment.envMessage.p_data = &len;
  
  memoryCardWrite(p_message, strlen(p_message));

  for(;;)
  {
    display(&environment);
  }

  return 0;
}
