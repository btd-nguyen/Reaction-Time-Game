/**
 * @file GPIO.h
 *
 * @brief Header file for the GPIO driver.
 *
 * This file contains the function prototypes and definitions for the GPIO driver.
 *
 * @note
 *
 * @author Benjamin Nguyen
 */

#include <stdint.h>

// LED Definitions
#define RED_LED    0x02    // PF1
#define BLUE_LED   0x04    // PF2
#define GREEN_LED  0x08    // PF3

// Button Definitions
#define SW1        0x10    // PF4
#define SW2        0x01    // PF0

// Function Prototypes
void GPIO_Init(void);
void GPIO_Enable_Interrupt(void);
void GPIO_Disable_Interrupt(void);
void LED_On(uint8_t color);
void LED_Off(uint8_t color);
void LED_Toggle(uint8_t color);
uint8_t SW1_Pressed(void);
uint8_t SW2_Pressed(void);
uint8_t Get_Button_Flag(void);
void Clear_Button_Flag(void);
uint32_t Get_Reaction_Time(void);
void Set_Start_Time(uint32_t time);