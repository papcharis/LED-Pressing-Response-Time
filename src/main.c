/*
 * FILE: part2.c
 * THMMY, 8th semester, Microprocessors and peripherals : 2nd assignment
 * Implementation for the Nucleo STM32 - F401RE board 
 * Authors:
 *   Moustaklis Apostolos, 9127, amoustakl@auth.gr
 *   Papadakis Charis , 9128, papadakic@ece.auth.gr
 * Includes the main used in the src folder of the lab code examples / drivers 
 *   
 */

#include <stdio.h>
#include <stdlib.h>
#include <platform.h>
#include <gpio.h>
#include <leds.h>
#include <delay.h>

//Defines used for the cpu cycle calculation
#define  ARM_CM_DEMCR      (*(uint32_t *)0xE000EDFC)
#define  ARM_CM_DWT_CTRL   (*(uint32_t *)0xE0001000)
#define  ARM_CM_DWT_CYCCNT (*(uint32_t *)0xE0001004)

// MODE 0  LED  turns on and then we press the button
// MODE 1  LED  turns off and then we press the button 
# define MODE 0

// Do the experiment for LOOP times 
# define LOOP 5

//Global variables 
int uptime ;
static int buttonPressed = 0;
int randTime;

//Functions 
void LED_response_counter(int mode);
void button_press_isr(int sources);



int main(void) {
  // Initialise LEDs.
  leds_init();
  // Set up debug signals.
  gpio_set_mode(P_DBG_ISR, Output);
  gpio_set_mode(P_DBG_MAIN, Output);
  // Set up on-board switch.
  gpio_set_mode(P_SW, PullUp);
  gpio_set_trigger(P_SW, Rising);
  gpio_set_callback(P_SW, button_press_isr);
  __disable_irq();
  int mode = MODE ;  
  switch (MODE) {
  case 0:
    LED_response_counter(MODE);
    break;
  case 1:
    LED_response_counter(MODE);
    break;
  default:
    printf("Wrong mode please try again \n");

  }
}

//Functions Implementation

//Switch pressed interrupt code
void button_press_isr(int sources) {
  gpio_set(P_DBG_ISR, 1);
  if ((sources << GET_PIN_INDEX(P_SW)) & (1 << GET_PIN_INDEX(P_SW))) {
    buttonPressed = 1;
  }
  gpio_set(P_DBG_ISR, 0);
}

// LED human response counter 
void LED_response_counter(int mode) {
   
  if (ARM_CM_DWT_CTRL != 0) { // See if DWT is available ( CPI Counter Register ) 
    ARM_CM_DEMCR |= 1 << 24; // Set bit 24
    ARM_CM_DWT_CYCCNT = 0; // Cycle count Register 
    ARM_CM_DWT_CTRL |= 1 << 0; // Set bit 0
		
  }

  double avgTimeTakenInCycles;
  double avgTimeTakenInSec;

//Variables for the time calculation
  uint32_t start;
  uint32_t stop;
  uint32_t delta;
  uint32_t overallTime;

  for (int i = 0; i < LOOP; i++) {
  //  uptime = 0;
    leds_set(mode, 0, 0);
    //Random delay time 
    randTime = rand() % 80;
    delay_ms(randTime * 50);
		__enable_irq();
    leds_set(!mode, 0, 0);
		uptime = 1;
    start = ARM_CM_DWT_CYCCNT;
    gpio_toggle(P_DBG_MAIN);
    while (1) {
      //If there is no response continue 
      if (!buttonPressed) {
        continue;
      } 
			//When the buttonPressed gets updated from the interrupt function 
			//Calculate the time 
			else {
        stop = ARM_CM_DWT_CYCCNT;
				uptime = 0;
        leds_set(mode, 0, 0);
				__disable_irq();
      }
			
      delta = stop - start;
      overallTime += delta;
      break;
    }
		
    buttonPressed = 0;
  }

  avgTimeTakenInCycles = overallTime / LOOP;
	//To find the time taken in seconds we divede the time taken in cpu cycls with the cpu frequency
	//The cpu frequency is 16Mhz
  avgTimeTakenInSec = avgTimeTakenInCycles / 16000000;

  printf("The average response time was  \n");
  printf("In cpu cycles : %lf  \n", avgTimeTakenInCycles);
  printf("In seconds : %lf ", avgTimeTakenInSec);

}