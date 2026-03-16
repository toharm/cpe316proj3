#include "main.h"
#include "adc_helpers.h"
#include "tim2.h"
#include "fft.h"
#include "pid.h"

volatile uint16_t adc_value;
volatile float converted;

static PIDController pid = {
    .Kp           = 1.0f,
    .Ki           = 0.1f,
    .Kd           = 0.01f,
    .target_freq  = 500.0f,
    .last_freq    = 0.0f,
    .dt           = PID_DT,
    .integral_sum = 0.0f,
    .output       = 0.0f,
};

static float win_buf[N];
static int   win_write = 0;
static int   new_count = 0;

static float fft_re[N];
static float fft_im[N];
static float fft_tmp[2 * N];

static uint32_t adc_read_idx = 0;

static void set_pwm_duty(float pid_output)
{
    uint32_t arr = TIM2->ARR;

    if (pid_output < 0.0f)       pid_output = 0.0f;
    if (pid_output > (float)arr) pid_output = (float)arr;

    TIM2->CCR1 = (uint32_t)pid_output;
}

static float dominant_frequency(void)
{
    float max_mag = 0.0f;
    int   max_bin = 1;

    for (uint32_t i = 1; i <= N / 2; i++) {
        float mag = fft_re[i] * fft_re[i] + fft_im[i] * fft_im[i];
        // && mag > (100/BIN_RES)
        if (mag > max_mag) {
            max_mag = mag;
            max_bin = i;
        }
    }

    return (float)max_bin * BIN_RES;
}

static void load_window_to_fft(void)
{
    for (uint32_t i = 0; i < N; i++) {
        fft_re[i] = win_buf[(win_write + i) & (N - 1)];
        fft_im[i] = 0.0f;
    }
}

static void drain_adc_samples(void)
{
    uint32_t dma_write = (BUFFER_SIZE - DMA1_Channel1->CNDTR) & (BUFFER_SIZE - 1);

    while (adc_read_idx != dma_write) {
        uint16_t raw = adc_buffer[adc_read_idx];
        adc_read_idx = (adc_read_idx + 1) & (BUFFER_SIZE - 1);

        win_buf[win_write] = (raw / 4096.0f) * 2.0f - 1.0f;
        win_write = (win_write + 1) & (N - 1);
        new_count++;
    }
}


/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);

volatile float actual_freq;

int main(void)
{

	HAL_Init();
	SystemClock_Config();  /* 32 MHz */
	ADC_Init();
	init_tim2();

	/* Pre-fill the window before first FFT */
	while (new_count < N) {
		drain_adc_samples();
	}
	new_count = 0;

	while (1)
	{
		/* 1. Pull new samples from DMA */
		drain_adc_samples();

		/* 2. Only run FFT+PID once we have ADVANCE new samples (~20 ms) */
		if (new_count >= ADVANCE) {
			new_count = 0;

			/* 3. Snapshot sliding window into FFT arrays */
			load_window_to_fft();

			/* 4. Run FFT */
			fft(fft_re, fft_im, N, fft_tmp);

			/* 5. Extract dominant frequency */
			actual_freq = dominant_frequency();

			/* 6. Run PID */
			//PID_update(&pid, actual_freq);

			/* 7. Apply to PWM */
			//set_pwm_duty(pid.output);
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
