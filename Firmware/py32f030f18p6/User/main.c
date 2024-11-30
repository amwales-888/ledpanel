/**
  ******************************************************************************
  * @file    main.c
  * @author  MCU Application Team
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) Puya Semiconductor Co.
  * All rights reserved.</center></h2>
  *
  * <h2><center>&copy; Copyright (c) 2016 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

#include "main.h"

static void APP_SystemClockConfig(void);
static void APP_GPIOConfig(void);

#define NELEMS(x)  (sizeof(x) / sizeof((x)[0]))

struct task_s { 

	uint32_t lastTick;
	uint32_t periodTick;
	void (*func)(void);
};


/*

		SR1

		PA11 - ROW_DATA_IN  Data
		PA10 - ROW_SCLK_IN  Clock
		PA9  - ROW_RCLK_IN  Latch

		SR2

		PB4  - COL_DATA_IN	Data
		PB6  - COL_SCLK_IN	Clock
		PB5  - COL_RCLK_IN	Latch

*/

#define ID_ROW 1
#define ID_COL 2

void serialToParallelOutByte(int id, uint8_t value, int flag) {

    switch (id) {
        case ID_ROW: {
					
						// Row Shift Register ( Common Cathode )

						if (flag == 1) {
							LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_9);
						}
														
            for(int i=0; i<8; i++) {

								uint8_t x = (uint8_t)1 << i;                
                if (value & x) {
										LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_11);
                } else {
										LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_11);
                }

								LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_10);
								
								__nop();
								__nop();
								__nop();
								__nop();
																
								LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_10);
            }

						if (flag == 2) {
							LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_9);
						}
            break;
        }
        case ID_COL: {
            
						// Column Shift Register ( Anode )

						if (flag == 1) {
							LL_GPIO_ResetOutputPin(GPIOB, LL_GPIO_PIN_5);
						}

            for(int i=7; i>=0; i--) {

                uint8_t x = (uint8_t)1 << i;                
                if (value & x) {
										LL_GPIO_SetOutputPin(GPIOB, LL_GPIO_PIN_4);
                } else {
										LL_GPIO_ResetOutputPin(GPIOB, LL_GPIO_PIN_4);
                }

								LL_GPIO_SetOutputPin(GPIOB, LL_GPIO_PIN_6);
								
								__nop();
								__nop();
								__nop();
								__nop();
								
								LL_GPIO_ResetOutputPin(GPIOB, LL_GPIO_PIN_6);

            }

						if (flag == 2) {
							LL_GPIO_SetOutputPin(GPIOB, LL_GPIO_PIN_5);
						}
            break;
        }
    }    
}

#define MAX_ROW 32
#define MAX_COL 32

static uint8_t screen[MAX_ROW][MAX_COL/8];
    
void outputBitmap(void) {

	for (int row=0; row<MAX_ROW; row++) {

		serialToParallelOutByte(ID_ROW, 0, 1); 
		serialToParallelOutByte(ID_ROW, 0, 0); 
		serialToParallelOutByte(ID_ROW, 0, 0); 
		serialToParallelOutByte(ID_ROW, 0, 2); 

		serialToParallelOutByte(ID_COL, screen[row][0], 1);         
		serialToParallelOutByte(ID_COL, screen[row][1], 0);         
		serialToParallelOutByte(ID_COL, screen[row][2], 0);         
		serialToParallelOutByte(ID_COL, screen[row][3], 2);         

		if (row < 8) {
			serialToParallelOutByte(ID_ROW, 1 << (row % 8), 1); 
			serialToParallelOutByte(ID_ROW, 0, 0); 
			serialToParallelOutByte(ID_ROW, 0, 0); 
			serialToParallelOutByte(ID_ROW, 0, 2); 
		} else if (row < 16) {
			serialToParallelOutByte(ID_ROW, 0, 1); 
			serialToParallelOutByte(ID_ROW, 1 << (row % 8), 0); 
			serialToParallelOutByte(ID_ROW, 0, 0); 
			serialToParallelOutByte(ID_ROW, 0, 2); 
		} else if (row < 24) {
			serialToParallelOutByte(ID_ROW, 0, 1); 
			serialToParallelOutByte(ID_ROW, 0, 0); 
			serialToParallelOutByte(ID_ROW, 1 << (row % 8), 0); 
			serialToParallelOutByte(ID_ROW, 0, 2); 
		} else if (row < 32) {
			serialToParallelOutByte(ID_ROW, 0, 1); 
			serialToParallelOutByte(ID_ROW, 0, 0); 
			serialToParallelOutByte(ID_ROW, 0, 0); 
			serialToParallelOutByte(ID_ROW, 1 << (row % 8), 2); 
		}
		
		
	}
	
	
}

void updateBitmap(void) {

    static int col = 0;
    static int row = 0;
    static int lcol = 0;
    static int lrow = 0;
    
    screen[lrow][0] &= ~(1 << lcol);    
    screen[row][0] |= 1 << col;

    lcol = col;
    lrow = row;
    
    
    col++;
    if (col >= 8) {
        col = 0;
        row++;
        if (row >= 8) {
            row = 0;            
        }
    }    
}

void updateBitmap2(void) {

#define roffset 24
#define coffset 3
	
    static int col = 0;
    static int row = 0;
    static int lcol = 0;
    static int lrow = 0;
    
    screen[lrow+roffset][0+coffset] &= ~(1 << lcol);    
    screen[row+roffset][0+coffset] |= 1 << col;

    lcol = col;
    lrow = row;
    
    
    col++;
    if (col >= 8) {
        col = 0;
        row++;
        if (row >= 8) {
            row = 0;            
        }
    }    
}

volatile static uint32_t systickCnt;

void SysTick_Handler(void) {
  systickCnt++;
}

/**
 End of File
*/

int main(void)
{
    struct task_s tasks[] = {

		{ 0, 0,  outputBitmap },  /* update a different row every 10ms */
		{ 0, 10,  updateBitmap },  /* 50ms animations written to bitmap */
		{ 0, 1000, updateBitmap2 },  /* 1S animations written to bitmap */
	};
	
	
	
  APP_SystemClockConfig();
  APP_GPIOConfig();

	LL_SYSTICK_EnableIT();

//	screen[0][0] = 0b10000001;
//	screen[7][0] = 0b10000001;

	screen[24][0] = 0b10000001;
	screen[31][0] = 0b10000001;


	screen[0][3] = 0b10000001;
	screen[7][3] = 0b10000001;

//	screen[24][3] = 0b10000001;
//	screen[31][3] = 0b10000001;
	
	
  while (1)
  {
#if 0		
    LL_GPIO_TogglePin(GPIOB, LL_GPIO_PIN_5);
    LL_mDelay(1000);
#else		
		struct task_s *task = tasks;
	
		for (unsigned int i=0; i<NELEMS(tasks); i++) {
		 		 
			if ((uint32_t)(systickCnt - task->lastTick) >= task->periodTick) {
                
				(*task->func)();
				task->lastTick = systickCnt;
			}
			
			task++;
		}
#endif		
  }
}

#define OLDCODE

#ifndef OLDCODE
static void APP_SystemClockConfig(void)
{
  LL_UTILS_ClkInitTypeDef ClkInit = { LL_RCC_SYSCLK_DIV_1, LL_RCC_APB1_DIV_1 };

  LL_RCC_HSI_Enable();
  LL_RCC_HSI_SetCalibFreq(LL_RCC_HSICALIBRATION_24MHz);
  while (LL_RCC_HSI_IsReady() != 1);
  LL_PLL_ConfigSystemClock_HSI(&ClkInit);
  SysTick_Config(48000);
  NVIC_SetPriority(SysTick_IRQn, 0);
}
#else
static void APP_SystemClockConfig(void)
{
  LL_UTILS_ClkInitTypeDef UTILS_ClkInitStruct;
  //Enable internal high speed
  LL_RCC_HSI_Enable();
  //Set to 24MHz, the frequency can be fine-tuned here, and the higher the value, the faster the frequency
  LL_RCC_HSI_SetCalibFreq(LL_RCC_HSICALIBRATION_24MHz);
  //Wait for stabilization
  while (LL_RCC_HSI_IsReady() != 1);
  //AHB does not divide the frequency
  UTILS_ClkInitStruct.AHBCLKDivider = LL_RCC_SYSCLK_DIV_1;
  //APB is not crossover
  UTILS_ClkInitStruct.APB1CLKDivider = LL_RCC_APB1_DIV_1;
  //Set the system clock source to PLL+HSI, note the method name 
  LL_PLL_ConfigSystemClock_HSI(&UTILS_ClkInitStruct);
  //Update the settings for SysTick
  LL_InitTick(48000000U, 1000U);
}
#endif

static void APP_GPIOConfig(void)
{
  LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOA);
  LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_9, LL_GPIO_MODE_OUTPUT);
  LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_10, LL_GPIO_MODE_OUTPUT);
  LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_11, LL_GPIO_MODE_OUTPUT);
	
  LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOB);
  LL_GPIO_SetPinMode(GPIOB, LL_GPIO_PIN_4, LL_GPIO_MODE_OUTPUT);
  LL_GPIO_SetPinMode(GPIOB, LL_GPIO_PIN_5, LL_GPIO_MODE_OUTPUT);
  LL_GPIO_SetPinMode(GPIOB, LL_GPIO_PIN_6, LL_GPIO_MODE_OUTPUT);	
}

void APP_ErrorHandler(void)
{
  while (1);
}

#ifdef  USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
  while (1);
}
#endif /* USE_FULL_ASSERT */
