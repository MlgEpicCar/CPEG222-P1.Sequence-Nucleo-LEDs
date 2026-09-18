/****************************************************************
* Author: Carlos Munar (MlgEpicCar)
* CPEG222 Project 1: Sequence Nucleo LEDs, 9/17/26
* 
* This program turns on LEDs in an Red -> Blue -> Green -> Red -> ... order. 
* The switches occur when the user presses the blue button. When reset (black button)
* is pressed the program will return to the Red LED being on.
*
* Also please ignore my excessive commenting lol, I want to use this 
* project as something I can review later!
****************************************************************/

#include "stm32f4xx.h"
#include <stdbool.h>

/* Refers to the blue button on the board
IMPORTANT: USER_BUTTON_PIN will normally be LOW but then become HIGH when pressed 
(bc of step-up or smth)*/
#define BUTTON_PIN 13
#define BUTTON_PORT GPIOC

/* NUCLEO-F446ZE onboard LEDs: LD1 green = PB0, LD2 blue = PB7, LD3 red = PB14 */
#define GREEN_LED_PIN  0
#define BLUE_LED_PIN   7
#define RED_LED_PIN    14
#define LED_PORT GPIOB

volatile bool button_pressed = false;
volatile uint32_t button_debounce = 0;

void SysTick_Handler(void) {
    /* Button is HIGH when pressed */
    if (BUTTON_PORT->IDR & (1U << BUTTON_PIN))
    {
        /* SUPER AWESOME DEBOUNCE TIME */
        if (button_debounce == 0)
        {
            button_pressed = true;
            /* 200 ms debounce/cooldown */
            button_debounce = 200;
        }
    }

    /* Decrease cooldown every millisecond */
    if (button_debounce > 0)
    {
        button_debounce--;
    }
}

int main(void)
{
    /* Enable clock for GPIOB and GPIOC */
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN |
                     RCC_AHB1ENR_GPIOCEN;

    /* Bit Manip Magic Breakdown (kinda just notes for my future self ngl)
    *  Goal: We need to edit GPIOB so that we can set our pins mode to output
    * 
    *  Step 1: Start with the onboard LEDs and their port numbers as an example we can use
    *  the blue LED whose port number is 7, in MODER since each pin gets 2 digits this means we
    *  can simply multiply the port number by 2 to get it's designated spot (14 and 15). Another
    *  way to think about it is to realize port num 0 gets digits 0 and 1, port num 1 gets 
    *  digits 2 and 3... regardless of it's port number, multiplying it by the number of digits 
    *  the value you're aiming for is will always lead to what you're looking for
    * 
    *  Step 2: We use 3U to set the value at the two important bits to 11, 3 in binary is 11 and
    *  the U means it's unsigned (not able to be negative, think abosulte value)
    * 
    *  Step 3: We use ~ to flip all of the bits, so 00000000011000000 becomes 11111110011111
    *  we do this to prepare to AND our mask with the original
    * 
    *  Step 4: We use x &= y (a short hand of x = x & y) to and our original dataset with
    *  our mask so that anything that was originally a 0 stays a 0 and anything that was
    *  a 1 stays a 1, but critically our two bits become 00 regardless of what they were before
    */

    /* Clear PB0, PB7, PB14 so we can set them to general purpose output */
    LED_PORT->MODER &= ~((3U << (GREEN_LED_PIN * 2)) |
                       (3U << (BLUE_LED_PIN * 2)) |
                       (3U << (RED_LED_PIN * 2)));
    /* Set PB0, PB7, PB14 as general purpose output (01) */
    LED_PORT->MODER |= (1U << (GREEN_LED_PIN * 2)) |
                    (1U << (BLUE_LED_PIN * 2)) |
                    (1U << (RED_LED_PIN * 2));

    /* Similar to GPIOB, BSRR stores values for our LEDs. BSRR controls if it is on or not,
    however, there is a critial difference, for BSRR the first 16 bits (0-15) would be set to 1
    whenever you want to turn something on, and that same value +16 (16-31) would be set to 1 to
    turn the same thing off. In other words BSRR is write only */

    /* Ensure all LEDs start off */
    LED_PORT->BSRR = (1U << (GREEN_LED_PIN + 16)) |
                  (1U << (BLUE_LED_PIN + 16)) |
                  (1U << (RED_LED_PIN + 16));

    /* bit manip for the button is a lot easier since it's an input (00)*/
    /* Configure Button */
    BUTTON_PORT->MODER &= ~(3U << (BUTTON_PIN * 2));

    /* Configure SysTick*/
    SysTick_Config(SystemCoreClock / 1000);

    /*List of LED addresses to sift through*/
    const uint8_t leds[] = { RED_LED_PIN, GREEN_LED_PIN, BLUE_LED_PIN };

    /* Always start with Red LED*/
    int current_led = 0;
    LED_PORT->BSRR = (1U << leds[current_led]);

    while (1)
    {
        if (button_pressed)
        {
            /* Clear the flag */
            button_pressed = false;

            /* Turn current LED off */
            LED_PORT->BSRR = (1U << (leds[current_led] + 16));

            /* Move to next LED */
            current_led++;

            if (current_led >= 3)
            {
                current_led = 0;
            }

            /* Turn next LED on */
            LED_PORT->BSRR = (1U << leds[current_led]);
        }
    }
}
