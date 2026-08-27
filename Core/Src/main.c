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
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
volatile uint8_t contador_J1 = 0;
volatile uint8_t contador_J2 = 0;

volatile uint32_t ultimo_tiempo_J1 = 0;
volatile uint32_t ultimo_tiempo_J2 = 0;
volatile uint32_t ultimo_tiempo_START = 0;

volatile uint8_t iniciar_carrera = 0;
volatile uint8_t carrera_activa = 0;
volatile uint8_t carrera_terminada = 0;

#define DEBOUNCE_TIME 50
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
/* USER CODE BEGIN PFP */
void Mostrar_J1(uint8_t numero);
void Mostrar_J2(uint8_t numero);

void Ganador_J1(void);
void Ganador_J2(void);

void Apagar_Leds(void);

void Display_Numero(uint8_t numero);
void Display_Apagar(void);

void Cuenta_Regresiva(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/* ============================================================
   MOSTRAR POSICION JUGADOR 1
   ============================================================ */

void Mostrar_J1(uint8_t numero)
{
    /* Apagar todos los LEDs de J1 */
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);

    switch(numero)
    {
        case 1:
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);
            break;

        case 2:
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET);
            break;

        case 3:
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
            break;

        case 4:
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);
            break;

        default:
            break;
    }
}


/* ============================================================
   MOSTRAR POSICION JUGADOR 2
   ============================================================ */

void Mostrar_J2(uint8_t numero)
{
    /* Apagar todos los LEDs de J2 */
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET);

    switch(numero)
    {
        case 1:
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET);
            break;

        case 2:
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_SET);
            break;

        case 3:
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_SET);
            break;

        case 4:
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);
            break;

        default:
            break;
    }
}


/* ============================================================
   INSTRUCCION 8
   APAGAR TODOS LOS LEDS
   ============================================================ */

void Apagar_Leds(void)
{
    /* Jugador 1 */
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);

    /* Jugador 2 */
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET);
}


/* ============================================================
   INSTRUCCION 9
   CONTROL DE LOS SEGMENTOS DEL DISPLAY
   ============================================================ */

void Display_Segmentos(uint8_t a,
                       uint8_t b,
                       uint8_t c,
                       uint8_t d,
                       uint8_t e,
                       uint8_t f,
                       uint8_t g)
{
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8,
                      a ? GPIO_PIN_SET : GPIO_PIN_RESET);

    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9,
                      b ? GPIO_PIN_SET : GPIO_PIN_RESET);

    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_10,
                      c ? GPIO_PIN_SET : GPIO_PIN_RESET);

    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_11,
                      d ? GPIO_PIN_SET : GPIO_PIN_RESET);

    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_12,
                      e ? GPIO_PIN_SET : GPIO_PIN_RESET);

    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2,
                      f ? GPIO_PIN_SET : GPIO_PIN_RESET);

    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7,
                      g ? GPIO_PIN_SET : GPIO_PIN_RESET);
}


/* ============================================================
   MOSTRAR NUMERO EN EL DISPLAY
   ============================================================ */

void Display_Numero(uint8_t numero)
{
    switch(numero)
    {
        case 0:
            Display_Segmentos(1,1,1,1,1,1,0);
            break;

        case 1:
            Display_Segmentos(0,1,1,0,0,0,0);
            break;

        case 2:
            Display_Segmentos(1,1,0,1,1,0,1);
            break;

        case 3:
            Display_Segmentos(1,1,1,1,0,0,1);
            break;

        case 4:
            Display_Segmentos(0,1,1,0,0,1,1);
            break;

        case 5:
            Display_Segmentos(1,0,1,1,0,1,1);
            break;

        default:
            Display_Segmentos(0,0,0,0,0,0,0);
            break;
    }
}


void Display_Apagar(void)
{
    Display_Segmentos(0,0,0,0,0,0,0);
}


/* ============================================================
   INSTRUCCION 10
   GANADOR JUGADOR 1
   ============================================================ */

void Ganador_J1(void)
{
    /* Encender todos los LEDs de J1 */
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);

    /* Apagar todos los LEDs de J2 */
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET);

    /* Mostrar 1 en el display */
    Display_Numero(1);
}


/* ============================================================
   GANADOR JUGADOR 2
   ============================================================ */

void Ganador_J2(void)
{
    /* Apagar todos los LEDs de J1 */
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);

    /* Encender todos los LEDs de J2 */
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);

    /* Mostrar 2 en el display */
    Display_Numero(2);
}


/* ============================================================
   INSTRUCCION 11
   INTERRUPCIONES
   ============================================================ */

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    uint32_t tiempo_actual = HAL_GetTick();


    /* --------------------------------------------------------
       BOTON START - BOTON AZUL DE LA NUCLEO
       -------------------------------------------------------- */

    if(GPIO_Pin == B1_Pin)
    {
        if((tiempo_actual - ultimo_tiempo_START) >= DEBOUNCE_TIME)
        {
            ultimo_tiempo_START = tiempo_actual;

            if(carrera_activa == 0)
            {
                iniciar_carrera = 1;
            }
        }

        return;
    }


    /* Si no estamos jugando, ignorar J1 y J2 */
    if(carrera_activa == 0 || carrera_terminada == 1)
    {
        return;
    }


    /* --------------------------------------------------------
       JUGADOR 1
       -------------------------------------------------------- */

    if(GPIO_Pin == GPIO_PIN_0)
    {
        if((tiempo_actual - ultimo_tiempo_J1) >= DEBOUNCE_TIME)
        {
            ultimo_tiempo_J1 = tiempo_actual;

            contador_J1++;

            if(contador_J1 >= 4)
            {
                contador_J1 = 4;

                carrera_activa = 0;
                carrera_terminada = 1;

                Ganador_J1();
            }
            else
            {
                Mostrar_J1(contador_J1);
            }
        }
    }


    /* --------------------------------------------------------
       JUGADOR 2
       -------------------------------------------------------- */

    if(GPIO_Pin == GPIO_PIN_1)
    {
        if((tiempo_actual - ultimo_tiempo_J2) >= DEBOUNCE_TIME)
        {
            ultimo_tiempo_J2 = tiempo_actual;

            contador_J2++;

            if(contador_J2 >= 4)
            {
                contador_J2 = 4;

                carrera_activa = 0;
                carrera_terminada = 1;

                Ganador_J2();
            }
            else
            {
                Mostrar_J2(contador_J2);
            }
        }
    }
}


/* ============================================================
   INSTRUCCION 12
   CUENTA REGRESIVA
   ============================================================ */

void Cuenta_Regresiva(void)
{
    /* Bloquear a los jugadores */
    carrera_activa = 0;
    carrera_terminada = 0;

    /* Reiniciar contadores */
    contador_J1 = 0;
    contador_J2 = 0;

    /* Apagar LEDs */
    Apagar_Leds();


    /* Cuenta regresiva */

    Display_Numero(5);
    HAL_Delay(1000);

    Display_Numero(4);
    HAL_Delay(1000);

    Display_Numero(3);
    HAL_Delay(1000);

    Display_Numero(2);
    HAL_Delay(1000);

    Display_Numero(1);
    HAL_Delay(1000);

    Display_Numero(0);
    HAL_Delay(1000);


    /* Apagar display */
    Display_Apagar();


    /*
     * Reiniciar tiempos del debounce para evitar
     * pulsaciones anteriores.
     */
    ultimo_tiempo_J1 = HAL_GetTick();
    ultimo_tiempo_J2 = HAL_GetTick();


    /* Habilitar carrera */
    carrera_activa = 1;
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  /* USER CODE BEGIN 2 */
  Mostrar_J1(0);
  Mostrar_J2(0);
  Display_Apagar();

  carrera_activa = 0;
  carrera_terminada = 0;
  iniciar_carrera = 0;
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
      if(iniciar_carrera)
      {
          iniciar_carrera = 0;

          Cuenta_Regresiva();
      }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 84;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, LED1_J1_Pin|LED2_J1_Pin|LED3_J1_Pin|LD2_Pin
                          |LED1_J2_Pin|LED2_J2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LED4_J1_Pin|LED4_J2_Pin|SEG_G_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, LED3_J2_Pin|SEG_A_Pin|SEG_B_Pin|SEG_C_Pin
                          |SEG_D_Pin|SEG_E_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(SEG_F_GPIO_Port, SEG_F_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : B1_Pin BTN_J1_Pin BTN_J2_Pin */
  GPIO_InitStruct.Pin = B1_Pin|BTN_J1_Pin|BTN_J2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : LED1_J1_Pin LED2_J1_Pin LED3_J1_Pin LED1_J2_Pin
                           LED2_J2_Pin */
  GPIO_InitStruct.Pin = LED1_J1_Pin|LED2_J1_Pin|LED3_J1_Pin|LED1_J2_Pin
                          |LED2_J2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : USART_TX_Pin USART_RX_Pin */
  GPIO_InitStruct.Pin = USART_TX_Pin|USART_RX_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : LD2_Pin */
  GPIO_InitStruct.Pin = LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LED4_J1_Pin LED4_J2_Pin SEG_G_Pin */
  GPIO_InitStruct.Pin = LED4_J1_Pin|LED4_J2_Pin|SEG_G_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : LED3_J2_Pin SEG_A_Pin SEG_B_Pin SEG_C_Pin
                           SEG_D_Pin SEG_E_Pin */
  GPIO_InitStruct.Pin = LED3_J2_Pin|SEG_A_Pin|SEG_B_Pin|SEG_C_Pin
                          |SEG_D_Pin|SEG_E_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : SEG_F_Pin */
  GPIO_InitStruct.Pin = SEG_F_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(SEG_F_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);

  HAL_NVIC_SetPriority(EXTI1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI1_IRQn);

  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
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
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
