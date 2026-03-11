/**
 * @author Tohar Markovich
 * main connects LEDs to PC0, PC1, and PC2, outputting a counter from 0 to 7
 * repeated when PA4 is pulled down
 */


#include "main.h"

#define BIT4 0x0010 // bit 4 logic high
#define DELAY 400 // software delay
#define PC_MASK 0x3 // mask used to reset PC0, PC1, and PC2
#define PC2_SHIFT 4 // shift 4 to access PC2 MODER (2 bits, RW)
#define PA4_SHIFT 8 // shift 8 to access PA4 MODER (2 bits, RW)
#define PUPDR_SHIFT 8 // shift 8 to access PA4 PUPDR (2 bits, RW)
#define MAX_COUNT 0x7 // max count, used for counter and to reset LED bits

int main(void)
{
	// turns on clock to GPIO banks A and C
  RCC->AHB2ENR |= (RCC_AHB2ENR_GPIOAEN | RCC_AHB2ENR_GPIOCEN);

  // bank C as GPIO mode
  //GPIOC->MODER &= ~(GPIO_MODER_MODE13);
 // GPIOC->PUPDR &= ~(GPIO_PUPDR_PUPD13);

  /* configure PC0 PC1 PC2 as output */

  /* PC0 */
  GPIOC->MODER &= ~PC_MASK;
  GPIOC->MODER |= 1;

  /* PC1 */
  GPIOC->MODER &= ~(PC_MASK << 2);
  GPIOC->MODER |= (1 << 2);

  /* PC2 */
  GPIOC->MODER &= ~(PC_MASK << PC2_SHIFT);
  GPIOC->MODER |= BIT4;

  // configure button for input
  GPIOA->MODER &= ~(GPIO_MODER_MODE4);

  GPIOA->PUPDR &= ~(PC_MASK << PA4_SHIFT);
  GPIOA->PUPDR |= (1 << PUPDR_SHIFT);

  // to supply 3.3V, need PC0 PC1 PC2 low in logic

  uint8_t counter = 0;

  HAL_Init();

  while (1) {
	  if (!(GPIOA->IDR & BIT4)) { // if PA4 pulled low
		  GPIOC->ODR &= ~MAX_COUNT; // reset PC0, PC1, PC2
		  GPIOC->ODR |= (counter & MAX_COUNT); // set PC0, PC1, PC2 based on counter
		  // adjust counter, only requires setting to 0 after the count reaches 7
		  if (counter != MAX_COUNT) counter = (counter % MAX_COUNT) + 1;
		  else counter = 0;
	  }
	  HAL_Delay(DELAY); // software delay
  }// end while

}// end main


