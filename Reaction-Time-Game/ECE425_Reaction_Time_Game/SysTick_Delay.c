/**
 * @file SysTick_Delay.c
 *
 * @brief Source code for the SysTick Delay driver.
 *
 * This file contains the function definitions for the SysTick Delay driver.
 *
 * @note Assumes that the frequency of the system clock is 16 MHz.
 *
 * SysTick Configuration:
 *  - System Clock: 16 MHz
 *  - Interrupt: Enabled
 *  - Reload Value: Set for 1 ms interrupts
 *
 * @author Benjamin Nguyen
 */

#include "SysTick_Delay.h"
#include "TM4C123GH6PM.h"

// Global variable
static volatile uint32_t systick_counter = 0;

// SysTick register addresses
#define NVIC_ST_CTRL     (*((volatile uint32_t *)0xE000E010))
#define NVIC_ST_RELOAD   (*((volatile uint32_t *)0xE000E014))
#define NVIC_ST_CURRENT  (*((volatile uint32_t *)0xE000E018))

void SysTick_Init(void)
{
    // Disables SysTick during configuration
    NVIC_ST_CTRL = 0;
    
    // Sets reload value for 1 ms interrupts at 16 MHz
    // Reload value = (Desired period * Clock Frequency) - 1
    // For 1 ms: (0.001 * 16,000,000) - 1 = 15999
    NVIC_ST_RELOAD = 15999;  // 1 ms at 16 MHz
    
    // Clears current value
    NVIC_ST_CURRENT = 0;
    
    // Enables SysTick with system clock & interrupt
    // Bit 0: Enable, Bit 1: Interrupt Enable, Bit 2: Clock Source (1 = system clock)
    NVIC_ST_CTRL = 0x07;
}

void SysTick_Delay(uint32_t delay_ms)
{
    systick_counter = 0;
    while (systick_counter < delay_ms);
}

void SysTick_Delay_Milliseconds(uint32_t ms)
{
    SysTick_Delay(ms);  
}

uint32_t SysTick_Get_Current_Time(void)
{
    return systick_counter;  // Returns milliseconds
}

uint32_t Generate_Random_Delay(uint32_t min_ms, uint32_t max_ms)
{
    static uint32_t seed = 12345;
    
    // Simple pseudo-random number generator
    seed = (seed * 1103515245 + 12345) & 0x7FFFFFFF;
    
    return min_ms + (seed % (max_ms - min_ms + 1));
}

// SysTick Interrupt Handler - increments every 1 ms
void SysTick_Handler(void)
{
    systick_counter++;
}