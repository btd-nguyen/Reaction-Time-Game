/**
 * @file main.c
 *
 * @brief Main application file for Reaction Time Game.
 *
 * This file contains the main application for the Reaction Time Game project.
 * The game measures how fast a user can press a button in response to a visual stimulus.
 *
 * Features:
 *  - Configurable number of rounds
 *  - UART-based menu system
 *  - Reaction time measurement with validation
 *  - Performance rating system
 *  - Results display via UART
 *
 * Hardware Configuration:
 *  - LEDs: PF1 (Red), PF2 (Blue), PF3 (Green)
 *  - Buttons: PF4 (SW1 - Reaction), PF0 (SW2 - Menu)
 *  - UART: PA0 (RX), PA1 (TX) - 115200 baud configuration
 *
 * @note Assumes 50 MHz system clock frequency.
 *
 * @author Benjamin Nguyen
 */

#include <stdint.h>
#include "GPIO.h"
#include "SysTick_Delay.h"
#include "UART.h"
#include "TM4C123GH6PM.h"

// Game constants
#define MAX_ROUNDS 10
#define MIN_DELAY_MS 1000
#define MAX_DELAY_MS 3000
#define TIMEOUT_MS 2000
#define ANTICIPATION_THRESHOLD_MS 100

// Game result structure
typedef struct
{
    uint32_t reaction_time;
    uint8_t valid;
} RoundResult;

// Global variables
static RoundResult game_results[MAX_ROUNDS];
static uint8_t current_round = 0;
static uint8_t total_rounds = 5;

// Function prototypes
void Display_Menu(void);
void Get_Number_Of_Rounds(void);
void Play_Game(void);
void Display_Results(void);
void Declare_Winner(uint32_t average_time);

int main(void)
{
    // Initializes all peripherals
    SysTick_Init();
    GPIO_Init();
    UART0_Init();
    
    // Enables interrupts globally
    __asm(" CPSIE I");  // Assembly instruction to enable interrupts
    
    // Displays starting message
    UART0_Clear_Screen();
    UART0_Output_String("--- Reaction Time Game ---\r\n\r\n");
    UART0_Output_String("ECE 425 Final Project\r\n\r\n");
    
    while (1)
    {
        Display_Menu();
        
        char choice = UART0_Input_Character();
        UART0_Output_Character(choice);
        UART0_Output_Newline();
        
        switch (choice)
        {
            case '1':
                Get_Number_Of_Rounds();
                break;
                
            case '2':
                Play_Game();
                break;
                
            case '3':
                Display_Results();
                break;
                
            case '4':
                UART0_Output_String("Exiting...\r\n");
                return 0;
                
            default:
                UART0_Output_String("\r\nInvalid choice, try again.\r\n");
        }
    }
}

void Display_Menu(void)
{
    UART0_Output_Newline();
    UART0_Output_String("--- Main Menu ---\r\n\r\n");
    UART0_Output_String("1. Set Number of Rounds (Current: ");
    UART0_Output_Unsigned_Decimal(total_rounds);
    UART0_Output_String(")\r\n\r\n");
    UART0_Output_String("2. Start Game\r\n\r\n");
    UART0_Output_String("3. View Previous Results\r\n\r\n");
    UART0_Output_String("4. Exit\r\n\r\n");
    UART0_Output_String("Enter your choice: ");
}

void Get_Number_Of_Rounds(void)
{
    UART0_Output_Newline();
    UART0_Output_String("Enter number of rounds (1-");
    UART0_Output_Unsigned_Decimal(MAX_ROUNDS);
    UART0_Output_String("): ");
    
    uint32_t rounds = UART0_Input_Unsigned_Decimal();
    UART0_Output_Newline();
    
    if ((rounds >= 1) && (rounds <= MAX_ROUNDS))
    {
        total_rounds = rounds;
        UART0_Output_String("Number of rounds set to: ");
        UART0_Output_Unsigned_Decimal(total_rounds);
        UART0_Output_Newline();
    }
    else
    {
        UART0_Output_String("Invalid number, using default: ");
        UART0_Output_Unsigned_Decimal(total_rounds);
        UART0_Output_Newline();
    }
}

void Play_Game(void)
{
    UART0_Clear_Screen();
    UART0_Output_String("--- Game Starting ---\r\n\r\n");
    UART0_Output_String("Get ready to press SW1 when the red LED turns on.\r\n\r\n");
    
    current_round = 0;
    
    // TEST: Checks if GPIO is working
    UART0_Output_String("Testing LEDs... ");
    LED_On(RED_LED);
    SysTick_Delay_Milliseconds(500);
    LED_On(BLUE_LED);
    SysTick_Delay_Milliseconds(500);
    LED_On(GREEN_LED);
    SysTick_Delay_Milliseconds(500);
    LED_Off(RED_LED | BLUE_LED | GREEN_LED);
    UART0_Output_String("LED test complete.\r\n\r\n");
    
    for (uint8_t i = 0; i < total_rounds; i++)
    {
        UART0_Output_String("Round ");
        UART0_Output_Unsigned_Decimal(i + 1);
        UART0_Output_String(" of ");
        UART0_Output_Unsigned_Decimal(total_rounds);
        UART0_Output_String(" - ");
        
        // Resets game state
        Clear_Button_Flag();
        LED_Off(RED_LED | BLUE_LED | GREEN_LED);
        
        // Countdown from 3
        UART0_Output_String("Countdown: ");
        for (int j = 3; j > 0; j--)
        {
            UART0_Output_Unsigned_Decimal(j);
            UART0_Output_String(" ");
            SysTick_Delay_Milliseconds(2000);
        }
        UART0_Output_Newline();
        
        // Random delay before turning on LED
        uint32_t random_delay = Generate_Random_Delay(MIN_DELAY_MS, MAX_DELAY_MS);
        UART0_Output_String("Random delay: ");
        UART0_Output_Unsigned_Decimal(random_delay);
        UART0_Output_String(" ms\r\n");
        SysTick_Delay_Milliseconds(random_delay);
        
        // Turns on RED LED and starts timing
        UART0_Output_String("Red LED on! Press SW1!\r\n");
        LED_On(RED_LED);
        Set_Start_Time(SysTick_Get_Current_Time());
        GPIO_Enable_Interrupt();
        
        // Waits for button press with timeout
				uint32_t start_wait = SysTick_Get_Current_Time();
				// Already in milliseconds

				while (!Get_Button_Flag())
				{
					uint32_t current_time = SysTick_Get_Current_Time();
					if ((current_time - start_wait) > TIMEOUT_MS)  // Direct comparison in ms
					{
							// Timeout - no response
							game_results[i].valid = 0;
							game_results[i].reaction_time = 0;
							UART0_Output_String("Too slow! No response.\r\n");
							LED_Off(RED_LED);
							LED_On(BLUE_LED);
							SysTick_Delay_Milliseconds(2000);
							LED_Off(BLUE_LED);
					break;
    }
}
        
        GPIO_Disable_Interrupt();
        
        if (Get_Button_Flag())
{
    // Valid response is received
    uint32_t reaction_time_ms = Get_Reaction_Time();  // Already in milliseconds
    
    // Checks for anticipation (too fast)
    if (reaction_time_ms < ANTICIPATION_THRESHOLD_MS)
    {
        game_results[i].valid = 0;
        game_results[i].reaction_time = reaction_time_ms;
        UART0_Output_String("Too fast! Anticipated too early.\r\n");
        LED_Off(RED_LED);
        LED_On(BLUE_LED);
    }
    else
    {
        game_results[i].valid = 1;
        game_results[i].reaction_time = reaction_time_ms;
        UART0_Output_String("Reaction time: ");
        UART0_Output_Unsigned_Decimal(reaction_time_ms);
        UART0_Output_String(" ms\r\n");
        LED_Off(RED_LED);
        LED_On(GREEN_LED);
    }
    
    SysTick_Delay_Milliseconds(2000);
    LED_Off(GREEN_LED | BLUE_LED);
}
        
        Clear_Button_Flag();
        
        // Short delay between rounds
        UART0_Output_String("Round complete. Next round starting...\r\n\r\n");
        SysTick_Delay_Milliseconds(2000);
    }
    
    current_round = total_rounds;
    Display_Results();
}

void Display_Results(void)
{
    UART0_Clear_Screen();
    UART0_Output_String("--- Game Results ---\r\n\r\n");
    
    uint32_t total_valid_time = 0;
    uint8_t valid_responses = 0;
    
    for (uint8_t i = 0; i < current_round; i++)
    {
        UART0_Output_String("Round ");
        UART0_Output_Unsigned_Decimal(i + 1);
        UART0_Output_String(": ");
        
        if (game_results[i].valid)
        {
            UART0_Output_Unsigned_Decimal(game_results[i].reaction_time);
            UART0_Output_String(" ms\r\n");
            total_valid_time += game_results[i].reaction_time;
            valid_responses++;
        }
        else
        {
            if (game_results[i].reaction_time == 0)
            {
                UART0_Output_String("No response\r\n");
            }
            else if (game_results[i].reaction_time < ANTICIPATION_THRESHOLD_MS)
            {
                UART0_Output_String("Anticipated too early\r\n");
            }
        }
    }
    
    if (valid_responses > 0)
    {
        uint32_t average_time = total_valid_time / valid_responses;
        UART0_Output_String("\r\nAverage reaction time: ");
        UART0_Output_Unsigned_Decimal(average_time);
        UART0_Output_String(" ms\r\n");
        Declare_Winner(average_time);
    }
    else
    {
        UART0_Output_String("\r\nNo valid responses recorded.\r\n");
    }
    
    UART0_Output_String("\r\nPress any key to continue...");
    UART0_Input_Character();
    UART0_Output_Newline();
}

void Declare_Winner(uint32_t average_time)
{
    UART0_Output_Newline();
    UART0_Output_String("--- Performance Rating ---\r\n\r\n");
    
    if (average_time < 150)
    {
        UART0_Output_String("Excellent! Pro gamer level!\r\n");
    }
    else if (average_time < 200)
    {
        UART0_Output_String("Very good! Above average!\r\n");
    }
    else if (average_time < 250)
    {
        UART0_Output_String("Good! Average human reaction time.\r\n");
    }
    else if (average_time < 300)
    {
        UART0_Output_String("Ok. Not too bad.\r\n");
    }
    else
    {
        UART0_Output_String("Too slow. Try to be faster!\r\n");
    }
}