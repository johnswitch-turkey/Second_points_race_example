/**
  *************************************************************************************************************************
  * @file    motor.c
  * @author  amkl
  * @version V1.0
  * @date    2022-09-22
  * @brief   xxģ��.c�ļ�����
  *************************************************************************************************************************
  * @attention
  *
  *
  *************************************************************************************************************************
  */

/* Includes -------------------------------------------------------------------------------------------------------------*/
#include "motor.h"


extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim3;


/* ���� -----------------------------------------------------------------------------------------------------------------*/


/*�����ʼ������*/
//void Motor_Init(void)
//{
//    GPIO_InitTypeDef GPIO_InitStruct = {0};
//    
//    // ʹ��GPIOʱ��
//    __HAL_RCC_GPIOB_CLK_ENABLE();
//    
//    // ��ʼ��GPIOΪ�������
//    GPIO_InitStruct.Pin = GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
//    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
//    GPIO_InitStruct.Pull = GPIO_NOPULL;
//    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
//    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
//}

/*�޷�����*/
//void Limit(int *motoA, int *motoB, float *servo)
//{
//    if(*motoA >= 6500) *motoA = 6500;
//    if(*motoA <= -6500) *motoA = -6500;
//    
//    if(*motoB >= 6500) *motoB = 6500;
//    if(*motoB <= -6500) *motoB = -6500;

//    if(*servo >= 1900) *servo = 1900;
//    if(*servo <= 1800) *servo = 1800;    
//}

/*����ֵ����*/
int abs(int p)
{
    int q;
    q = p > 0 ? p : (-p);
    return q;
}

/*��ֵ����*/
/*��ڲ�����PID������ɺ������PWMֵ*/
void Load(int moto2, int moto1, uint16_t Target_Position)
{
    // 1.�о������ţ���Ӧ����ת
    if(moto1 > 0) {
        HAL_GPIO_WritePin(Ain1_Port, Ain1_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(Ain2_Port, Ain2_Pin, GPIO_PIN_RESET);
    } else {
        HAL_GPIO_WritePin(Ain1_Port, Ain1_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(Ain2_Port, Ain2_Pin, GPIO_PIN_SET);
    }
    
    if(moto2 > 0) {
        HAL_GPIO_WritePin(Bin1_Port, Bin1_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(Bin2_Port, Bin2_Pin, GPIO_PIN_RESET);
    } else {
        HAL_GPIO_WritePin(Bin1_Port, Bin1_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(Bin2_Port, Bin2_Pin, GPIO_PIN_SET);
    }    
    
    // 2.װ��PWMֵ
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, abs(moto1));
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, abs(moto2));
    
    // 3.װ�ض��PWMֵ
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, Target_Position);
}

/**
 * ������:set_motor_enable
 * ����:ʹ�ܵ��
 * ����:��
 * ���:��
 */
void set_motor_enable(void)  
{
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);  // ʹ��TIM1ͨ��1 PWM
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_4);  // ʹ��TIM1ͨ��4 PWM
}

/**
 * ������:set_motor_disable
 * ����:ʧ�ܵ��
 * ����:��
 * ���:��
 */
void set_motor_disable(void)
{
    HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);  // �ر�TIM1ͨ��1 PWM
    HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_4);  // �ر�TIM1ͨ��4 PWM
}

/*****************************************************END OF FILE*********************************************************/
