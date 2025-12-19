/**
 * @file SysTick_Delay.h
 *
 * @brief Header file for the SysTick Delay driver.
 *
 * This file contains the function prototypes and definitions for the SysTick Delay driver.
 *
 * @note Assumes that the frequency of the system clock is 16 MHz.
 *
 * @author Benjamin Nguyen
 */

#include <stdint.h>

// Function Prototypes
void SysTick_Init(void);
void SysTick_Delay(uint32_t delay_ms);
void SysTick_Delay_Milliseconds(uint32_t ms);
uint32_t SysTick_Get_Current_Time(void);  // Returns time in milliseconds
uint32_t Generate_Random_Delay(uint32_t min_ms, uint32_t max_ms);