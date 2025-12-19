/**
 * @file GPIO.c
 *
 * @brief Source code for the GPIO driver.
 *
 * This file contains the function definitions for the GPIO driver.
 *
 * @note
 *
 * GPIO Configuration:
 *  - PF1 (Red LED): Output
 *  - PF2 (Blue LED): Output
 *  - PF3 (Green LED): Output
 *  - PF4 (SW1): Input with interrupt
 *  - PF0 (SW2): Input with pull-up
 *
 * @author Benjamin Nguyen
 */

#include "GPIO.h"
#include "TM4C123GH6PM.h"
#include "SysTick_Delay.h"

// Global variables
static volatile uint8_t button_flag = 0;
static volatile uint32_t reaction_time = 0;
static volatile uint32_t start_time = 0;

void GPIO_Init(void)
{
    // Enables clock for Port F
    SYSCTL->RCGCGPIO |= 0x20;
    
    // Waits for clock to stabilize
    while((SYSCTL->PRGPIO & 0x20) == 0);
    
    // Unlocks PF0
    GPIOF->LOCK = 0x4C4F434B;
    GPIOF->CR |= 0x01;
    
    // Sets direction: PF1-3 as output, PF0,PF4 as input
    GPIOF->DIR = 0x0E;
    GPIOF->DEN = 0x1F;
    
    // Enables pull-up for PF0 and PF4
    GPIOF->PUR = 0x11;
    
    // Clears any pending interrupts
    GPIOF->ICR = 0x11;
}

void GPIO_Enable_Interrupt(void)
{
    // Clears any pending interrupt for PF4
    GPIOF->ICR = 0x10;
    
    // Configures interrupt for PF4
    GPIOF->IS &= ~0x10;     // Edge-sensitive
    GPIOF->IBE &= ~0x10;    // Single edge
    GPIOF->IEV &= ~0x10;    // Falling edge
    GPIOF->IM |= 0x10;      // Enables interrupt for PF4
    
    // Enables interrupt in NVIC
    NVIC->ISER[0] |= 1 << 30;
}

void GPIO_Disable_Interrupt(void)
{
    // Disables interrupt for PF4
    GPIOF->IM &= ~0x10;
    
    // Disables interrupt in NVIC
    NVIC->ICER[0] |= 1 << 30;
}

void LED_On(uint8_t color)
{
    GPIOF->DATA |= color;
}

void LED_Off(uint8_t color)
{
    GPIOF->DATA &= ~color;
}

void LED_Toggle(uint8_t color)
{
    GPIOF->DATA ^= color;
}

uint8_t SW1_Pressed(void)
{
    return !(GPIOF->DATA & SW1);
}

uint8_t SW2_Pressed(void)
{
    return !(GPIOF->DATA & SW2);
}

uint8_t Get_Button_Flag(void)
{
    return button_flag;
}

void Clear_Button_Flag(void)
{
    button_flag = 0;
}

uint32_t Get_Reaction_Time(void)
{
    return reaction_time;
}

void Set_Start_Time(uint32_t time)
{
    start_time = time;
}

// GPIO Port F Interrupt Handler
void GPIOF_Handler(void)
{
    if (GPIOF->MIS & SW1)
    {
        GPIOF->ICR = SW1;
        reaction_time = SysTick_Get_Current_Time() - start_time;  // Already in milliseconds
        button_flag = 1;
    }
}