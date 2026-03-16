/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include <math.h>
#include "main.h"
#include "adc_helpers.h"
#include "tim2.h"
#include "fft.h"
#include "fft_scheduler.h"
#include "PWM.h"
#include "PID.h"
#include "song_player.h"

#define MAX(a, b) ((a) > (b) ? (a) : (b))

/* --- do_re_mi song (Do = A4 = 440 Hz) --- */
/* Scale used: A major-ish mapping for solfège:
   Do = A4  (440.00)
   Re = B4  (493.88)
   Mi = C#5 (554.37)
   Fa = D5  (587.33)
   Sol = E5 (659.25)
   La = F#5 (739.99)
   Ti = G#5 (830.61)
   Do' = A5 (880.00)
*/

/* Use the SongNote type from song_player.h */
static const SongNote do_re_mi[] = {
    /* "Do — a deer, a female deer" */
    { 440.00f, 2.0f },   /* Do  quarter */
    { 493.88f, 1.0f },   /* Re  half (a / deer) */

    { 554.37f, 2.0f },   /* Mi  quarter */
    { 440.00f, 1.0f },   /* Do  quarter */
    { 554.37f, 2.0f },   /* Mi  quarter */
    { 440.00f, 1.0f },   /* Do  quarter */
    { 554.37f, 3.0f },   /* Mi  quarter */

    { 493.88f, 2.0f },   /* Re  half (a female deer) */
    { 554.37f, 1.0f },   /* Mi  half (a drop of) */
	{ 587.33f, 2.0f },   /*Fa*/
    { 554.37f, 1.0f },   /* Mi  half (a drop of) */
    { 493.88f, 1.0f },   /* Re  half (a female deer) */
	{ 587.33f, 2.0f },   /*Fa*/

    { 554.37f, 2.0f },   /* Mi  half (a drop of) */
	{ 587.33f, 1.0f },   /*Fa*/
    { 659.25f, 2.0f },   /* Sol quarter */
    { 554.37f, 1.0f },   /* Mi  half (a drop of) */
    { 659.25f, 2.0f },   /* Sol quarter */
    { 554.37f, 1.0f },   /* Mi  half (golden sun) */
    { 659.25f, 2.0f },   /* Sol quarter */

	{ 587.33f, 2.0f },   /*Fa*/
    { 659.25f, 1.0f },   /* Sol quarter */
    { 739.99f, 2.0f },   /* LA quarter */
    { 659.25f, 1.0f },   /* Sol quarter */
	{ 587.33f, 1.0f },   /*Fa*/
    { 739.99f, 2.0f },   /* LA quarter */

    { 659.25f, 2.0f },   /* Sol quarter */
    { 440.00f, 1.0f },   /* Do  quarter */
    { 493.88f, 1.0f },   /* Re  half (a female deer) */
    { 554.37f, 1.0f },   /* Mi  quarter */
	{ 587.33f, 1.0f },   /*Fa*/
    { 659.25f, 1.0f },   /* Sol quarter */
    { 739.99f, 2.0f },   /* LA quarter */

    { 739.99f, 2.0f },   /* LA quarter */
	{ 493.88f, 1.0f },   /* Re  half (a female deer) */
	{ 554.37f, 1.0f },   /* Mi  quarter */
	{ 587.33f, 1.0f },   /*Fa*/
	{ 659.25f, 1.0f },   /* Sol quarter */
	{ 739.99f, 1.0f },   /* LA quarter */
	{ 830.61f, 2.0f },   /*TI*/

	{ 830.61f, 2.0f },   /*TI*/
	{ 554.37f, 1.0f },   /* Mi  quarter */
	{ 587.33f, 1.0f },   /*Fa*/
	{ 659.25f, 1.0f },   /* Sol quarter */
	{ 739.99f, 1.0f },   /* LA quarter */
	{ 830.61f, 1.0f },   /*TI*/
	{ 880.00f, 2.0f },   /*DOOO*/

	{ 880.00f, 1.0f },   /*DOOO*/
	{ 830.61f, 1.0f },   /*TI*/
	{ 739.99f, 1.0f },   /* LA quarter */
	{ 587.33f, 2.0f },   /*Fa*/
	{ 830.61f, 1.0f },   /*TI*/
	{ 659.25f, 2.0f },   /* Sol quarter */
	{ 880.00f, 1.0f },   /*DOOO*/
	{ 659.25f, 1.0f },   /* Sol quarter */
	{ 554.37f, 1.0f },   /* Mi  quarter */
	{ 493.88f, 1.0f },   /* Re  half (a female deer) */
    { 440.00f, 1.0f },   /* Do  quarter */
};
static const size_t DO_RE_MI_LEN = sizeof(do_re_mi) / sizeof(do_re_mi[0]);


/* Use the SongNote type from song_player.h */
static const SongNote twinkle[] = {
    /* "Do — a deer, a female deer" */
    { 440.00f, 1.0f },   /* Do  quarter */
    { 440.00f, 1.0f },   /* Do  quarter */
    { 659.25f, 1.0f },   /* Sol quarter */

    { 554.37f, 2.0f },   /* Mi  quarter */
    { 440.00f, 1.0f },   /* Do  quarter */
    { 554.37f, 3.0f },   /* Mi  quarter */

    { 493.88f, 2.0f },   /* Re  half (a female deer) */
    { 554.37f, 1.0f },   /* Mi  half (a drop of) */
	{ 587.33f, 2.0f },   /*Fa*/
    { 554.37f, 1.0f },   /* Mi  half (a drop of) */
    { 493.88f, 1.0f },   /* Re  half (a female deer) */
	{ 587.33f, 2.0f },   /*Fa*/

    { 554.37f, 2.0f },   /* Mi  half (a drop of) */
	{ 587.33f, 1.0f },   /*Fa*/
    { 659.25f, 2.0f },   /* Sol quarter */
    { 554.37f, 1.0f },   /* Mi  half (a drop of) */
    { 659.25f, 2.0f },   /* Sol quarter */
    { 554.37f, 1.0f },   /* Mi  half (golden sun) */
    { 659.25f, 2.0f },   /* Sol quarter */

	{ 587.33f, 2.0f },   /*Fa*/
    { 659.25f, 1.0f },   /* Sol quarter */
    { 739.99f, 2.0f },   /* LA quarter */
    { 659.25f, 1.0f },   /* Sol quarter */
	{ 587.33f, 1.0f },   /*Fa*/
    { 739.99f, 2.0f },   /* LA quarter */

    { 659.25f, 2.0f },   /* Sol quarter */
    { 440.00f, 1.0f },   /* Do  quarter */
    { 493.88f, 1.0f },   /* Re  half (a female deer) */
    { 554.37f, 1.0f },   /* Mi  quarter */
	{ 587.33f, 1.0f },   /*Fa*/
    { 659.25f, 1.0f },   /* Sol quarter */
    { 739.99f, 2.0f },   /* LA quarter */

    { 739.99f, 2.0f },   /* LA quarter */
	{ 493.88f, 1.0f },   /* Re  half (a female deer) */
	{ 554.37f, 1.0f },   /* Mi  quarter */
	{ 587.33f, 1.0f },   /*Fa*/
	{ 659.25f, 1.0f },   /* Sol quarter */
	{ 739.99f, 1.0f },   /* LA quarter */
	{ 830.61f, 2.0f },   /*TI*/

	{ 830.61f, 2.0f },   /*TI*/
	{ 554.37f, 1.0f },   /* Mi  quarter */
	{ 587.33f, 1.0f },   /*Fa*/
	{ 659.25f, 1.0f },   /* Sol quarter */
	{ 739.99f, 1.0f },   /* LA quarter */
	{ 830.61f, 1.0f },   /*TI*/
	{ 880.00f, 2.0f },   /*DOOO*/

	{ 880.00f, 1.0f },   /*DOOO*/
	{ 830.61f, 1.0f },   /*TI*/
	{ 739.99f, 1.0f },   /* LA quarter */
	{ 587.33f, 2.0f },   /*Fa*/
	{ 830.61f, 1.0f },   /*TI*/
	{ 659.25f, 2.0f },   /* Sol quarter */
	{ 880.00f, 1.0f },   /*DOOO*/
	{ 659.25f, 1.0f },   /* Sol quarter */
	{ 554.37f, 1.0f },   /* Mi  quarter */
	{ 493.88f, 1.0f },   /* Re  half (a female deer) */
    { 440.00f, 1.0f },   /* Do  quarter */
};
static const size_t TWINKLE = sizeof(twinkle) / sizeof(twinkle[0]);

PIDController flute_pid;

/* single callback — player only sets the target (player won't mute) */
static void set_target(uint16_t hz)
{
    /* forward to your existing PID target setter */
    PID_SetTarget(&flute_pid, hz);
}

static uint16_t goal_hz = 600;


static uint16_t fft_input[BUFFER_SIZE];
static uint16_t fft_mags[BUFFER_SIZE / 2];

volatile float peak_freq;
void SystemClock_Config(void);

int main(void)
{
    HAL_Init();
    SystemClock_Config();

    fft_mag_init();
    ADC_Init();
    init_tim2();
    fft_scheduler_init();
    pwm_init(20000);      // 50 Hz, both outputs start at 0%


    PID_Init(&flute_pid,
             0.5f,     // Kp
             0.0f,     // Ki
             0.008f,    // Kd
             0.016f,    // dt = 10 ms loop
             -99.0f,  // output min
             99.0f,   // output max
             -50.0f,   // integral min
             50.0f);   // integral max

    PID_SetTarget(&flute_pid, goal_hz);

    SongPlayer *player = SongPlayer_Init(set_target, 75);
    SongPlayer_SetSong(player, do_re_mi, DO_RE_MI_LEN, 1);
    SongPlayer_Start(player, 0);

    while (1)
    {
        uint16_t end_index;
        uint16_t peak_bin;
        float peak_bin_interp;

        fft_scheduler_poll();
        SongPlayer_Poll(player);

        if (fft_scheduler_take_pending_end_index(&end_index)) {
            copy_latest_window(fft_input, end_index);
            fft_compute_magnitudes_u16(fft_input, fft_mags);


            peak_bin = fft_find_peak_bin(fft_mags, 1);
            peak_bin_interp = fft_refine_peak_bin_parabolic(fft_mags, peak_bin);
            peak_freq = (peak_bin_interp * SAMPLE_RATE_HZ) / BUFFER_SIZE;
            if (peak_freq < 400 || peak_freq > 1500){
            	update_duty_2(0);    // CH2 = 75%
            	update_duty_1(0);
            } else{
            	float control;
            	float duty;

            	control = PID_Update(&flute_pid, peak_freq);
            	duty = MAX(fabsf(control),25);


            	if (control < 0.0f) {
            		if(duty < 20){
                	    update_duty_1(80);
            		}
            	    update_duty_1(duty);
            	    update_duty_2(0.0f);
            	}
            	else if (control > 0.0f) {
            		if(duty < 20){
						update_duty_1(80);

					}
            	    update_duty_1(0.0f);
            	    update_duty_2(duty);
            	}
            	else {
            	    update_duty_1(0.0f);
            	    update_duty_2(0.0f);
            	}
            }
        }
    }
}


void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
  //RCC_OscInitStruct.MSIState = RCC_MSI_ON;  //datasheet says NOT to turn on the MSI then change the frequency.
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_10;
	/* from stm32l4xx_hal_rcc.h
	#define RCC_MSIRANGE_0                 MSI = 100 KHz
	#define RCC_MSIRANGE_1                 MSI = 200 KHz
	#define RCC_MSIRANGE_2                 MSI = 400 KHz
	#define RCC_MSIRANGE_3                 MSI = 800 KHz
	#define RCC_MSIRANGE_4                 MSI = 1 MHz
	#define RCC_MSIRANGE_5                 MSI = 2 MHz
	#define RCC_MSIRANGE_6                 MSI = 4 MHz
	#define RCC_MSIRANGE_7                 MSI = 8 MHz
	#define RCC_MSIRANGE_8                 MSI = 16 MHz
	#define RCC_MSIRANGE_9                 MSI = 24 MHz
	#define RCC_MSIRANGE_10                MSI = 32 MHz
	#define RCC_MSIRANGE_11                MSI = 48 MHz   dont use this one*/
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;  //datasheet says NOT to turn on the MSI then change the frequency.
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
