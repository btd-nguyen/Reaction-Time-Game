/**
 * @file UART0.h
 *
 * @brief Header file for the UART0 driver.
 *
 * This file contains the function prototypes and definitions for the UART0 driver.
 *
 * @note Assumes that the frequency of the system clock is 50 MHz.
 *
 * @author Benjamin Nguyen
 */

#include <stdint.h>

// ASCII Character Definitions
#define UART0_CR   0x0D    // Carriage Return
#define UART0_LF   0x0A    // Line Feed
#define UART0_BS   0x08    // Backspace

// UART0 Status Bit Masks
#define UART0_RECEIVE_FIFO_EMPTY_BIT_MASK    0x10
#define UART0_TRANSMIT_FIFO_FULL_BIT_MASK    0x20

// Function Prototypes
void UART0_Init(void);
char UART0_Input_Character(void);
void UART0_Output_Character(char data);
void UART0_Input_String(char *buffer_pointer, uint16_t buffer_size);
void UART0_Output_String(char *pt);
uint32_t UART0_Input_Unsigned_Decimal(void);
void UART0_Output_Unsigned_Decimal(uint32_t n);
void UART0_Output_Newline(void);
void UART0_Clear_Screen(void);