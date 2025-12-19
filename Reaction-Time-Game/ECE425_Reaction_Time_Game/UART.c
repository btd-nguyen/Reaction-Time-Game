/**
 * @file UART0.c
 *
 * @brief Source code for the UART0 driver.
 *
 * This file contains the function definitions for the UART0 driver.
 *
 * @note Assumes that the frequency of the system clock is 50 MHz.
 *
 * UART0 Configuration:
 *  - Baud Rate: 115200
 *  - Data Length: 8 bits
 *  - Stop Bit: 1
 *  - Parity: Disabled
 *  - Clock Source: System Clock / 16
 *  - Pins: PA1 (U0TX), PA0 (U0RX)
 *
 * @author Benjamin Nguyen
 */

#include "UART.h"
#include "TM4C123GH6PM.h"

void UART0_Init(void)
{
    // Enables the clock to the UART0 module by setting the
    // R0 bit (Bit 0) in the RCGCUART register
    SYSCTL->RCGCUART |= 0x01;
    
    // Enables the clock to Port A by setting the
    // R0 bit (Bit 0) in the RCGCGPIO register
    SYSCTL->RCGCGPIO |= 0x01;
    
    // Disables the UART0 module before configuration by clearing
    // the UARTEN bit (Bit 0) in the CTL register
    UART0->CTL &= ~0x0001;
    
    // Configures the UART0 module to use the system clock (50 MHz)
    // divided by 16 by clearing the HSE bit (Bit 5) in the CTL register
    UART0->CTL &= ~0x0020;
    
    // Sets the baud rate by writing the DIVINT field (Bits 15 to 0)
    // and the DIVFRAC field (Bits 5 to 0) in the IBRD and FBRD registers, respectively.
    // The integer part of the calculated constant will be written to the IBRD register,
    // while the fractional part will be written to the FBRD register.
    // BRD = (System Clock Frequency) / (16 * Baud Rate)
    // BRDI = (50,000,000) / (16 * 115200) = 27.12673611 (IBRD = 27)
    // BRDF = ((0.12673611 * 64 + 0.5) = 8.611 (FBRD = 8)
    UART0->IBRD = 27;
    UART0->FBRD = 8;
    
    // Configures the data length to 8 bits by
    // writing 0x3 to WLEN (Bits 6 to 5) in the LCRH register
    UART0->LCRH |= 0x60;
    
    // Enables both transmit and receive FIFOs by
    // setting FEN bit (Bit 4) in the LCRH register
    UART0->LCRH |= 0x10;
    
    // Uses 1 stop bit by clearing STP2 bit (Bit 3) in the LCRH register
    UART0->LCRH &= ~0x08;
    
    // Disables the parity by clearing PEN bit (Bit 1) in the LCRH register
    UART0->LCRH &= ~0x02;
    
    // Re-enables the UART0 module by setting UARTEN bit (Bit 0) in the CTL register
    UART0->CTL |= 0x01;
    
    // Enables alternate functions for PA1 (TX) and PA0 (RX) by setting Bits 1 to 0 in AFSEL
    GPIOA->AFSEL |= 0x03;
    
    // Clears PMC1 (Bits 7 to 4) and PMC0 (Bits 3 to 0) in the PCTL register
    // for alternate function configuration
    GPIOA->PCTL &= ~0x000000FF;
    
    // Configures PA1 for UART0 TX (U0TX) by setting PMC1 (Bits 7 to 4) to 0x1
    GPIOA->PCTL |= 0x00000010;
    
    // Configures PA0 for UART0 RX (U0RX) by setting PMC0 (Bits 3 to 0) to 0x1
    GPIOA->PCTL |= 0x00000001;
    
    // Enables digital functionality for PA1 and PA0 by
    // setting Bits 1 to 0 in the DEN register
    GPIOA->DEN |= 0x03;
}

char UART0_Input_Character(void)
{
    // Waits until RX FIFO is not empty
    while ((UART0->FR & UART0_RECEIVE_FIFO_EMPTY_BIT_MASK) != 0);
    
    return (char)(UART0->DR & 0xFF);
}

void UART0_Output_Character(char data)
{
    // Waits until TX FIFO is not full
    while ((UART0->FR & UART0_TRANSMIT_FIFO_FULL_BIT_MASK) != 0);
    
    UART0->DR = data;
}

void UART0_Input_String(char *buffer_pointer, uint16_t buffer_size)
{
    int length = 0;
    char character = UART0_Input_Character();
    
    while (character != UART0_CR)
    {
        if (character == UART0_BS)
        {
            if (length)
            {
                buffer_pointer--;
                length--;
                UART0_Output_Character(UART0_BS);
            }
        }
        else if (length < buffer_size)
        {
            *buffer_pointer = character;
            buffer_pointer++;
            length++;
            UART0_Output_Character(character);
        }
        character = UART0_Input_Character();
    }
    *buffer_pointer = 0;
}

void UART0_Output_String(char *pt)
{
    while (*pt)
    {
        UART0_Output_Character(*pt);
        pt++;
    }
}

uint32_t UART0_Input_Unsigned_Decimal(void)
{
    uint32_t number = 0;
    uint32_t length = 0;
    char character = UART0_Input_Character();
    
    // Accepts until <enter> is typed
    while (character != UART0_CR)
    {
        if ((character >= '0') && (character <= '9'))
        {
            // The "number" will overflow if it is above 4,294,967,295
            number = (10 * number) + (character - '0');
            length++;
            UART0_Output_Character(character);
        }
        
        // If the input is a backspace, then the return number is
        // changed and a backspace will be outputted to the screen
        else if ((character == UART0_BS) && length)
        {
            number /= 10;
            length--;
            UART0_Output_Character(character);
        }
        
        character = UART0_Input_Character();
    }
    
    return number;
}

void UART0_Output_Unsigned_Decimal(uint32_t n)
{
    // Uses recursion to convert a decimal number
    // of unspecified length as an ASCII string
    if (n >= 10)
    {
        UART0_Output_Unsigned_Decimal(n / 10);
        n = n % 10;
    }
    
    // n is between 0 and 9
    UART0_Output_Character(n + '0');
}

void UART0_Output_Newline(void)
{
    UART0_Output_Character(UART0_CR);
    UART0_Output_Character(UART0_LF);
}

void UART0_Clear_Screen(void)
{
    // ANSI escape codes to clear screen and move the cursor to the home position
    UART0_Output_String("\033[2J\033[H");
}