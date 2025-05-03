#ifndef __KEY_H 
#define	__KEY_H

#include "stm32f4xx_hal.h"


// ���õ������������
#define BEEP_OFF    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET)
#define BEEP_ON     HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET)
#define BEEP_TOGGLE HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_4)
#define LED_OFF     HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET)
#define LED_ON      HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET)
#define LED_TOGGLE  HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13)


// ��������
#define KEY1_PIN GPIO_PIN_0
#define KEY1_PORT GPIOA
#define KEY2_PIN GPIO_PIN_4
#define KEY2_PORT GPIOA
#define KEY3_PIN GPIO_PIN_12
#define KEY3_PORT GPIOB

//���尴��״̬���ĸ���״̬
typedef enum
{
	KEY_CHECK,
	KEY_COMFIRM,
	KEY_RELEASE
}KEY_STATE;

//��ⰴ�������µı��
extern uint8_t KEY1flag;
extern uint8_t KEY2flag;
extern uint8_t KEY3flag;

//����״̬����ӦKEY_STATE
extern uint8_t KEY1state;
extern uint8_t KEY2state;
extern uint8_t KEY3state;

//���弸��ģʽ
#define BACK_PACKING 			1
#define SIDE_PACKING 			2
#define BACK_SIDE_PACKING 		3

extern uint8_t 	ModeChoose;    //ģʽѡ��

void key_state(KEY_STATE *key,GPIO_TypeDef *port,uint16_t pin,uint8_t *keyflag);
void modechoose(uint8_t* ModeChoose);
#endif
