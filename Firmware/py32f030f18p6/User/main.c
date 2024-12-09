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

		PA11 - ROW_DATA_IN  Data    14
		PA10 - ROW_SCLK_IN  Clock   11
		PA9  - ROW_RCLK_IN  Latch   12

		SR2

		PB4  - COL_DATA_IN	Data
		PB6  - COL_SCLK_IN	Clock
		PB5  - COL_RCLK_IN	Latch

*/

#define ID_ROW 1
#define ID_COL 2

#define MAX_ROW   32
#define MAX_COL   32
#define MAX_DEPTH 4

static uint8_t screen[MAX_DEPTH][MAX_ROW][MAX_COL/8]; 
    
static void outputBitmapX(uint8_t *ptr) {

	LL_GPIO_TogglePin(GPIOA, LL_GPIO_PIN_8);					// GPIO5

	for (int row=0; row<MAX_ROW; row++) {
		
		LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_9); 					// ROW LATCH

		if (row == 0) {
			LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_11);					// ROW DATA
    } else {
			LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_11);				// ROW DATA
    }

		LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_10);						// ROW CLOCK
		LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_10);					// ROW CLOCK
				
		LL_GPIO_ResetOutputPin(GPIOB, LL_GPIO_PIN_5);						// COL LATCH


		ptr += (MAX_COL/8)-1;
				
		for (int k=0; k<(MAX_COL/8); k++) {

			uint8_t value = *ptr--;
	
			for(int i=0; i<8; i++) {
				
					if (value & 0x1) {
							LL_GPIO_SetOutputPin(GPIOB, LL_GPIO_PIN_4);		// COL DATA
					} else {
							LL_GPIO_ResetOutputPin(GPIOB, LL_GPIO_PIN_4);	// COL DATA
					}

					LL_GPIO_SetOutputPin(GPIOB, LL_GPIO_PIN_6);				// COL CLOCK																					
					LL_GPIO_ResetOutputPin(GPIOB, LL_GPIO_PIN_6);			// COL CLOCK

					value = value >> 1;
			}
		}
				
		ptr++;		
		ptr += (MAX_COL/8);
				
		LL_GPIO_SetOutputPin(GPIOB, LL_GPIO_PIN_5);							// COL LATCH
				
		LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_9);							// ROW LATCH
	}
	
	LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_9);							// ROW LATCH
	LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_11);						// ROW DATA

	LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_10);							// ROW CLOCK
	LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_10);						// ROW CLOCK


	LL_GPIO_ResetOutputPin(GPIOB, LL_GPIO_PIN_5);							// COL LATCH

	for (int k=0; k<4; k++) {

		uint8_t value = 0;

		for(int i=0; i<8; i++) {
			
				if (value & 0x1) {
						LL_GPIO_SetOutputPin(GPIOB, LL_GPIO_PIN_4);			// COL DATA
				} else {
						LL_GPIO_ResetOutputPin(GPIOB, LL_GPIO_PIN_4);		// COL DATA
				}

				LL_GPIO_SetOutputPin(GPIOB, LL_GPIO_PIN_6);					// COL CLOCK																																
				LL_GPIO_ResetOutputPin(GPIOB, LL_GPIO_PIN_6);				// COL CLOCK

				value = value >> 1;
		}
	}
		
	LL_GPIO_SetOutputPin(GPIOB, LL_GPIO_PIN_5);								// COL LATCH

	LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_9);								// ROW LATCH
}




static void outputBitmap(void) {

	static int i = 0;

	outputBitmapX((uint8_t *)&screen[i]);
	
	i++;
	if (i == MAX_DEPTH) {
		i = 0;
	}
}




static void updateBitmap(void) {

    static int col = 0;
    static int row = 0;
    static int lcol = 0;
    static int lrow = 0;
		static int xxx = 3;
		static int lxxx = 3;

    screen[0][lrow][lxxx] &= ~(1 << lcol);    
    screen[0][row][xxx] |= 1 << col;

    lcol = col;
    lrow = row;
		lxxx = xxx;
    
    
    col++;
    if (col >= 8) {
				xxx--;			
        col = 0;
				if (xxx < 0) {
					xxx = 3;
					row++;
					if (row >= 8) {
							row = 0;            
					}
				}
    }    
}





volatile static uint32_t systickCnt;

void SysTick_Handler(void) {
  systickCnt++;
}

const uint8_t nfont[10][8] = {
	
	{ 112, 136,152, 168, 200, 136, 112, 0 },
	{ 32, 96, 32, 32, 32, 32, 112, 0 },
	{ 112, 136, 8, 16, 32, 64, 248, 0 },
	{ 240, 8, 8, 112, 8, 8, 240, 0 },
	{ 136, 136, 136, 248, 8, 8, 8, 0 },
	{ 248, 128, 128, 240, 8, 8, 240, 0 },
	{ 112, 128, 128, 240, 136, 136, 112, 0 },
	{ 248, 8, 8, 16, 32, 32, 32, 0 },
	{ 112, 136, 136, 112, 136, 136, 112, 0 },
	{ 112, 136, 136, 120, 8, 136, 112, 0 },
};


static void drawBlankCharX(int row, int col, uint8_t *ptr) {

	int coff = col % 8;
	ptr += (row * MAX_COL/8) + (col/8);
	
	for (int i=0; i<8; i++) {
		
		*ptr &= ~(252 >> coff);		
		if (coff) {
			*(ptr+1) &= ~(252 << (8-coff));		
		}
		
		ptr += MAX_COL/8;
	}
}

static void drawBlankChar(int row, int col, int depth) {
	
	drawBlankCharX(row, col, (uint8_t *)&screen[depth]);
}




static void drawCharX(int row, int col, uint8_t *ptr, int c) {
	
	int coff = col % 8;
	ptr += (row * MAX_COL/8) + (col/8);
		
	for (int i=0; i<8; i++) {
		
		*ptr &= ~(252 >> coff);		
		*ptr |= nfont[c][i] >> coff;
		if (coff) {
			*(ptr+1) &= ~(252 << (8-coff));		
			*(ptr+1) |= nfont[c][i] << (8-coff);
		}
		
		ptr += MAX_COL/8;
	}
}

static void drawCharY(int row, int col, int depth, int c) {

	drawCharX(row, col, (uint8_t *)&screen[depth], c);
}

static void drawChar(int row, int col, int intensity, int c) {

	for (int i=0; i<=intensity; i++) {
		drawCharY(row, col, i, c);
	}
}

static void updateBitmap2(void) {

	static int i = 0;

	drawChar(0, 0, 0, i);
	
	i++;
	if (i > 9) {
		i = 0;
	}
}

static void updateBitmap3(void) {

	static int i = 0;

	drawChar(0, 8, 1, i); 
	
	i++;
	if (i > 9) {
		i = 0;
	}
}



static void updateBitmap4(void) {

	static int i = 0;

	drawChar(0, 16, 2, i);
	
	i++;
	if (i > 9) {
		i = 0;
	}
}


static void updateBitmap5(void) {

	static int i = 0;

	drawChar(0, 24, 3, i);
	
	i++;
	if (i > 9) {
		i = 0;
	}
}





int main(void)
{
    struct task_s tasks[] = {

		{ 0, 0,  outputBitmap },  /* update a different row every 10ms */
//		{ 0, 50, updateBitmap2 },  /* 1S animations written to bitmap */
//		{ 0, 200, updateBitmap3 },  /* 1S animations written to bitmap */
//		{ 0, 1000, updateBitmap4 },  /* 1S animations written to bitmap */
//		{ 0, 2000, updateBitmap5 },  /* 1S animations written to bitmap */
//		{ 0, 10,  updateBitmap },  /* 50ms animations written to bitmap */
	};
	
	
	
  APP_SystemClockConfig();
  APP_GPIOConfig();

	LL_SYSTICK_EnableIT();

#if 1
	
	for (int i=0; i<1; i++) {
		
		for (int j=0; j<1; j++) {
			
			screen[j][i][0] = 0b11111111;
			screen[j][i][1] = 0b11111111;
			screen[j][i][2] = 0b11111111;
			screen[j][i][3] = 0b11111111;
		}
	}
	


















#else
	
	screen[0][24][0] = 0b11111111;
	screen[0][25][0] = 0b11111110;
	screen[0][26][0] = 0b11111100;
	screen[0][27][0] = 0b11111000;
	screen[0][28][0] = 0b11110000;
	screen[0][29][0] = 0b11100000;
	screen[0][30][0] = 0b11000000;
	screen[0][31][0] = 0b10000000;

	screen[1][24][0] = 0b11111111;
	screen[1][25][0] = 0b11111110;
	screen[1][26][0] = 0b11111100;
	screen[1][27][0] = 0b11111000;
	screen[1][28][0] = 0b11110000;
	screen[1][29][0] = 0b11100000;
	screen[1][30][0] = 0b11000000;
	screen[1][31][0] = 0b10000000;

	screen[2][24][0] = 0b11111111;
	screen[2][25][0] = 0b11111110;
	screen[2][26][0] = 0b11111100;
	screen[2][27][0] = 0b11111000;
	screen[2][28][0] = 0b11110000;
	screen[2][29][0] = 0b11100000;
	screen[2][30][0] = 0b11000000;
	screen[2][31][0] = 0b10000000;

	screen[3][24][0] = 0b11111111;
	screen[3][25][0] = 0b11111110;
	screen[3][26][0] = 0b11111100;
	screen[3][27][0] = 0b11111000;
	screen[3][28][0] = 0b11110000;
	screen[3][29][0] = 0b11100000;
	screen[3][30][0] = 0b11000000;
	screen[3][31][0] = 0b10000000;






	screen[0][24][1] = 0b11111111;
	screen[0][25][1] = 0b11111110;
	screen[0][26][1] = 0b11111100;
	screen[0][27][1] = 0b11111000;
	screen[0][28][1] = 0b11110000;
	screen[0][29][1] = 0b11100000;
	screen[0][30][1] = 0b11000000;
	screen[0][31][1] = 0b10000000;

	screen[1][24][1] = 0b11111111;
	screen[1][25][1] = 0b11111110;
	screen[1][26][1] = 0b11111100;
	screen[1][27][1] = 0b11111000;
	screen[1][28][1] = 0b11110000;
	screen[1][29][1] = 0b11100000;
	screen[1][30][1] = 0b11000000;
	screen[1][31][1] = 0b10000000;

	screen[2][24][1] = 0b11111111;
	screen[2][25][1] = 0b11111110;
	screen[2][26][1] = 0b11111100;
	screen[2][27][1] = 0b11111000;
	screen[2][28][1] = 0b11110000;
	screen[2][29][1] = 0b11100000;
	screen[2][30][1] = 0b11000000;
	screen[2][31][1] = 0b10000000;




	screen[0][24][2] = 0b11111111;
	screen[0][25][2] = 0b11111110;
	screen[0][26][2] = 0b11111100;
	screen[0][27][2] = 0b11111000;
	screen[0][28][2] = 0b11110000;
	screen[0][29][2] = 0b11100000;
	screen[0][30][2] = 0b11000000;
	screen[0][31][2] = 0b10000000;

	screen[1][24][2] = 0b11111111;
	screen[1][25][2] = 0b11111110;
	screen[1][26][2] = 0b11111100;
	screen[1][27][2] = 0b11111000;
	screen[1][28][2] = 0b11110000;
	screen[1][29][2] = 0b11100000;
	screen[1][30][2] = 0b11000000;
	screen[1][31][2] = 0b10000000;



	screen[0][24][3] = 0b11111111;
	screen[0][25][3] = 0b11111110;
	screen[0][26][3] = 0b11111100;
	screen[0][27][3] = 0b11111000;
	screen[0][28][3] = 0b11110000;
	screen[0][29][3] = 0b11100000;
	screen[0][30][3] = 0b11000000;
	screen[0][31][3] = 0b10000000;



drawChar(13, 13, 0, 3);
drawChar(13, 13, 1, 3);
drawChar(13, 13, 2, 3);
drawChar(13, 13, 3, 3);
#endif


  while (1)
  {
#if 0		
    LL_GPIO_TogglePin(GPIOB, LL_GPIO_PIN_5);
    LL_mDelay(1000);
#else		
		struct task_s *task = tasks;
	
		for (unsigned int i=0; i<NELEMS(tasks); i++) {
		 		 
			if ((task->periodTick == 0) ||
				  ((uint32_t)(systickCnt - task->lastTick) >= task->periodTick)) {
                
				(*task->func)();
				task->lastTick = systickCnt;
			}
			
			task++;
		}
#endif		
  }
}

//#define OLDCODE

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


  LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_8, LL_GPIO_MODE_OUTPUT);	// GPIO5


	
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
