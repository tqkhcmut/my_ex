#include "delay.h"

__IO unsigned int tick_ms = 0;
__IO unsigned int delay_count = 0;

void Delay_Init(void)
{
  if (SysTick_Config(SystemCoreClock / 1000))
  { 
    /* Capture error */ 
    while (1);
  }
}
void Delay(int ms_time)
{
  delay_count = ms_time;
  while(delay_count);
}
unsigned int Millis(void)
{
  return tick_ms;
}