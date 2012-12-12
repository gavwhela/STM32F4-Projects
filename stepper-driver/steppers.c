#include "stm32f4xx.h"
#include "system_stm32f4xx.h"
#include "stm32f4xx_conf.h" // again, added because ST didn't put it here ?

// Different Stepper Modes, use only one
#define HIGH_TORQUE
// #define MICRO_STEPPING
// #define LOW_POWER

// BYTES defines the pins turned on in a timestep
// MASKS defines the pins turned off in a timestep
#ifdef HIGH_TORQUE
static unsigned char NUM_STEPS = 4;
static unsigned int BYTES[] = {0x4000, 0x2000, 0x1000, 0x8000};
static unsigned int MASKS[] = {0x1000, 0x8000, 0x4000, 0x2000};
#endif 
#ifdef MICRO_STEPPING
static unsigned char NUM_STEPS = 8;
static unsigned int BYTES[] = {0x0000, 0x4000, 0x0000, 0x2000, 0x0000, 0x1000, 0x0000, 0x8000};
static unsigned int MASKS[] = {0x1000, 0x0000, 0x8000, 0x0000, 0x4000, 0x0000, 0x2000, 0x0000};
#endif
#ifdef LOW_POWER 
static unsigned char NUM_STEPS = 4;
static unsigned int BYTES[] = {0x8000, 0x4000, 0x2000, 0x1000};
static unsigned int MASKS[] = {0x1000, 0x8000, 0x4000, 0x2000};
#endif

// Gets call by TIM2 regularly and updates the pins.
void TIM2_IRQHandler(void) {
  static unsigned char state = 0;
  if (TIM2->SR & TIM_SR_UIF){
    unsigned int bits = BYTES[state];
    unsigned int mask = MASKS[state];
    GPIOB->ODR &= ~mask;
    GPIOB->ODR |=  bits;
    state = (state + 1) % NUM_STEPS;
  }
  TIM2->SR = 0; // Reset the interrupt;
}

int main(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN; // enable the clock to GPIOB
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN; // enable TIM2 clock
    
    GPIOB->MODER |= (0x1 << 2*12); //set pin 12 to be general purpose IO
    GPIOB->MODER |= (0x1 << 2*13); //set pin 13 to be general purpose IO
    GPIOB->MODER |= (0x1 << 2*14); //set pin 14 to be general purpose IO
    GPIOB->MODER |= (0x1 << 2*15); //set pin 15 to be general purpose IO

    NVIC->ISER[0] |= 1 << (TIM2_IRQn); // enable the TIM2 IRQ

    TIM2->PSC = SystemCoreClock;
    TIM2->DIER |= TIM_DIER_UIE; // enable update interrupt
    TIM2->ARR = 300;
    TIM2->CR1 |= TIM_CR1_ARPE | TIM_CR1_CEN; // autoreload on, counter enabled

    TIM2->EGR = 1; // trigger update event to reload timer registers

    // Do Nothing.
    while(1);

    return 0;
}
