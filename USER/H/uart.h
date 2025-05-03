/**
  ******************************************************************************
  * @file    usart3.h
  * @brief   USART3����ͷ�ļ�
  ******************************************************************************
  */

#ifndef __USART3_H
#define __USART3_H

#include "main.h"
#include "control.h"  // ����Param�ṹ�嶨��

/* ���������� */
//#define USART2_REC_LEN 128

#define USART2_RX_LEN 16
/* �������� */
void Usart2_Init(uint32_t baudrate);
void Usart2_SendByte(uint8_t Data);
void Usart2_SendString(char *str);
//void openMv_Proc(void);

/* �ⲿ�������� */
//extern uint8_t USART2_RX_BUF[USART2_REC_LEN];
//extern volatile uint8_t USART2_REC_CNT_LEN;

extern uint8_t USART2_RX_STA;
extern uint8_t USART2_newData;
extern uint8_t USART2_RX_BUF[USART2_RX_LEN];
extern uint8_t start_receive;
#endif /* __USART3_H */
