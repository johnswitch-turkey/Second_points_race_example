/**
  ******************************************************************************
  * @file    usart2.c
  * @author  amkl
  * @version V1.0
  * @date    2022-09-22
  * @brief   USART2 HAL��ʵ��
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "uart.h"
#include "string.h"
#include "stm32f4xx_hal.h"  // �������оƬ�ͺŵ���

extern UART_HandleTypeDef huart2;  // �����ⲿ����� huart2

/* ȫ�ֱ��� ------------------------------------------------------------------*/
uint8_t USART2_RX_BUF[USART2_RX_LEN] = {0}; // ���ջ�����
uint8_t USART2_RX_STA = 0;                  // ����״̬���
uint8_t USART2_newData;
uint8_t start_receive=0;
//volatile uint8_t USART2_REC_CNT_LEN = 0;     // �������ݼ���


/* CubeMX����˵�� ------------------------------------------------------------*/
/**
  * USART3���ò��裺
  * 1. ��Connectivity��ѡ��USART3
  * 2. ģʽѡ��Ϊ"Asynchronous"
  * 3. �������ã�
  *    - Baud Rate: 115200 (���������޸�)
  *    - Word Length: 8 Bits
  *    - Stop Bits: 1
  *    - Parity: None
  *    - Hardware Flow Control: None
  * 4. GPIO���ã�
  *    - PB10: USART3_TX �� Alternate Function Push-Pull
  *    - PB11: USART3_RX �� Input Floating
  * 5. NVIC���ã�
  *    - ����USART3ȫ���ж�
  */

/* ����ʵ�� ------------------------------------------------------------------*/

/**
  * @brief USART3��ʼ��
  * @param baudrate ������
  */
void Usart2_Init(uint32_t baudrate)
{
    // CubeMX�����ɳ�ʼ�����룬ͨ����main.c��MX_USART3_UART_Init()��
    // �˴�ֻ�����������ж�
    HAL_UART_Receive_IT(&huart2, &USART2_RX_BUF[0], 1);
}

/**
  * @brief ���͵��ֽ�
  * @param Data Ҫ���͵�����
  */
void Usart2_SendByte(uint8_t Data)
{
    HAL_UART_Transmit(&huart2, &Data, 1, HAL_MAX_DELAY);
}

/**
  * @brief �����ַ���
  * @param str Ҫ���͵��ַ���
  */
void Usart2_SendString(char *str)
{
    HAL_UART_Transmit(&huart2, (uint8_t *)str, strlen(str), HAL_MAX_DELAY);
}

/**
  * @brief USART3�жϻص�����
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if(huart->Instance == USART2)
    {
				start_receive=1;
        USART2_RX_BUF[USART2_RX_STA&0x7FFF]=USART2_newData;
				USART2_RX_STA++;
        
        // ��黺�������
        if(USART2_RX_STA >= USART2_RX_LEN)
        {
            USART2_RX_STA = 0;
        }
        
        // �������������ж�
        HAL_UART_Receive_IT(&huart2, &USART2_newData, 1);
    }
}

/**
  * @brief OpenMV���ݴ���
  */
/*
void openmv_proc(void)
{
    if(strstr((const char*)usart2_rx_buf, "end") != null)
    {
        if(strcmp((const char*)usart2_rx_buf, "stopend") == 0)
        {
            param.openmv_data = 1;
        }
        
        usart2_rec_cnt_len = 0;
        memset(usart2_rx_buf, 0, usart2_rec_len);
        
        // ���¿�ʼ����
        hal_uart_receive_it(&huart2, &usart2_rx_buf[0], 1);
    }
}
*/



/*************************************END OF FILE************************************/
