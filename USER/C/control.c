/**
  ******************************************************************************
  * @file    control.c
  * @author  amkl
  * @version V1.0
  * @date    2022-09-22
  * @brief   ���ƺ���
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

#include "control.h"

#include "encoder.h"
#include "pid.h"
#include "motor.h"
#include "key.h"
#include "math.h"
#include "mpu6050.h"
#include "uart.h"
#include "decode.h"

/* ȫ�ֱ��� ------------------------------------------------------------------*/
Param_InitTypedef Param;
Flag_InitTypedef Flag;

float pitch, roll, yaw; // ŷ����
#define T 0.158f       // ������
#define L 0.1545f      // �־����
//#define SERVO_INIT 1500 // �����λֵ
#define M_PI 3.14159265358979323846f

int raspi_rxData = 0;    //����ݮ�ɻ�ȡ������
uint8_t ModeChoose=0;

/* Ӳ������ ------------------------------------------------------------------*/


// ִ��������
//#define LED_TOGGLE HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin)
//#define BEEP_ON HAL_GPIO_WritePin(BEEP_GPIO_Port, BEEP_Pin, GPIO_PIN_SET)
//#define BEEP_OFF HAL_GPIO_WritePin(BEEP_GPIO_Port, BEEP_Pin, GPIO_PIN_RESET)

/* ˽�к������� --------------------------------------------------------------*/
static void Back_Parking_Control(void);
static void Side_Parking_Control(void);
static void Back_Side_Parking_Control(void);
static void Timer_Count_Handler(void);

/* ����ʵ�� ------------------------------------------------------------------*/

/**
  * @brief �ⲿ�жϻص�����
  * @param GPIO_Pin: �����жϵ�����
  */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    static uint16_t Time_Cnt = 0;
    
    // �������жϴ��� (PB5)
    if(GPIO_Pin == GPIO_PIN_5 )
    {
        // ��ȡ��̬����
//        if(Flag.Is_Go_straight || Flag.Is_Turn_Car) {
            mpu_dmp_get_data(&pitch, &roll, &yaw);
//        }
        
        // ��ȡ����������
        Param.UnitTime_Motor1Pluse = (short)Read_Speed(2);
        Param.UnitTime_Motor2Pluse = (short)Read_Speed(4);
        
        // ���Ƽ���
        Position_PID_Servo_Realize();
        Param.Motor1_PWM_Out = VelocityRing_MOTOR1_Control();
        Param.Motor2_PWM_Out = VelocityRing_MOTOR2_Control();
        
        // ������ƺ�ִ��
        Limit(&Param.Motor2_PWM_Out, &Param.Motor1_PWM_Out, &Param.Servo_Target_Position);
        Load(Param.Motor2_PWM_Out, Param.Motor1_PWM_Out, Param.Servo_Target_Position);
        
        // ״ָ̬ʾ�ƣ�10���жϷ�תһ�Σ�
        if(++Time_Cnt >= 10) {
            Time_Cnt = 0;
            LED_TOGGLE;
        }
        
        // ��ʱ����������
        Timer_Count_Handler();
    }
		/*
    // ����1�ж�
    else if(GPIO_Pin == KEY1_PIN) {
        HAL_Delay(15);
        if(HAL_GPIO_ReadPin(KEY1_PORT, KEY1_PIN) == GPIO_PIN_SET) {
            Param.ModeChoose = BACK_PACKING;
            Flag.Run_Step = 1;
        }
    }
    // ����2�ж�
    else if(GPIO_Pin == KEY2_PIN) {
        HAL_Delay(15);
        if(HAL_GPIO_ReadPin(KEY2_PORT, KEY2_PIN) == GPIO_PIN_SET) {
            Param.ModeChoose = SIDE_PACKING;
            Flag.Run_Step = 1;
        }
    }
    // ����3�ж�
    else if(GPIO_Pin == KEY3_PIN) {
        HAL_Delay(15);
        if(HAL_GPIO_ReadPin(KEY3_PORT, KEY3_PIN) == GPIO_PIN_SET) {
            Param.ModeChoose = BACK_SIDE_PACKING;
            Flag.Run_Step = 1;
        }
    }
		*/
}

/**
  * @brief ����������
  */
void Control_Proc(void)
{
		modechoose(&ModeChoose);
    switch(ModeChoose)
    {
        case BACK_PACKING:
            Back_Parking_Control();
            break;
            
        case SIDE_PACKING:
            Side_Parking_Control();
            break;
            
        case BACK_SIDE_PACKING:
            Back_Side_Parking_Control();
            break;
            
        default:
            // Ĭ��ģʽ����
            Kinematic_Analysis(0, 0, 0);
            break;
    }
}

/**
  * @brief ����������
  */
static void Back_Parking_Control(void)
{
    switch(Flag.Run_Step)
    {
        case 1: // ֱ�м��
            if(Flag.Is_Go_straight) {
                Kinematic_Analysis(60, 60, yaw);
                //openMv_Proc();
								raspi_rxData=raspi_rx();
                if(raspi_rxData == 1) {
                    Flag.Is_Go_straight = 0;
                    Flag.Is_Stop_Car = 1;
                    Flag.Run_Step = 2;
                }
            }
            break;
            
        case 2: // ͣ��1��
            if(Flag.Is_Stop_Car && !Flag.Start_Count) {
                Kinematic_Analysis(0, 0, 0.0);
                Flag.Start_Count = 1;
                Param.Timer_threshold_value = 100;
                BEEP_ON;
            }
            if(Flag.Is_Timer_Up) {
                BEEP_OFF;
                Flag.Start_Count = 0;
                Flag.Is_Timer_Up = 0;
                Flag.Is_Stop_Car = 0;
                Flag.Is_Start_Astern = 1;
                Flag.Run_Step = 3;
            }
            break;
            
        case 3: // ���ٺ���0.72s
            if(Flag.Is_Start_Astern == 1 && !Flag.Start_Count) {
                Kinematic_Analysis(-60, -60, SERVO_INIT);
                Flag.Start_Count = 1;
                Param.Timer_threshold_value = 72;
            }
            if(Flag.Is_Timer_Up) {
                Flag.Start_Count = 0;
                Flag.Is_Timer_Up = 0;
                Flag.Run_Step = 4;
            }
            break;
            
        case 4: // ��ת60�㵹��
            if(Flag.Is_Start_Astern == 1 && !Flag.Start_Count) {
                Kinematic_Analysis(-20, -100, SERVO_INIT + 300);
                Flag.Is_Turn_Car = 1;
            }
            if(abs((int)yaw) >= 80) {
                Flag.Is_Turn_Car = 0;
                Flag.Is_Start_Astern = 2;
                Flag.Run_Step = 5;
            }
            break;
            
        case 5: // ��ͷ��������1.5s
            if(Flag.Is_Start_Astern == 2 && !Flag.Start_Count) {
                Kinematic_Analysis(-100, -100, 0.0);
                Flag.Start_Count = 1;
                Param.Timer_threshold_value = 150;
            }
            if(Flag.Is_Timer_Up) {
                Flag.Start_Count = 0;
                Flag.Is_Timer_Up = 0;
                Flag.Is_Start_Astern = 0;
                Flag.Is_Stop_Car = 1;
                Flag.Run_Step = 6;
            }
            break;
            
        case 6: // ͣ��5��
            if(Flag.Is_Stop_Car && !Flag.Start_Count) {
                Kinematic_Analysis(0, 0, 0.0);
                Flag.Start_Count = 1;
                Param.Timer_threshold_value = 500;
                BEEP_ON;
            }
            if(Flag.Is_Timer_Up) {
                BEEP_OFF;
                Flag.Start_Count = 0;
                Flag.Is_Timer_Up = 0;
                Flag.Is_Stop_Car = 0;
                Flag.Run_Step = 7;
            }
            break;
            
        case 7: // ֱ�г���0.85s
            if(!Flag.Is_Stop_Car) {
                Kinematic_Analysis(100, 100, 0.0);
                Flag.Start_Count = 1;
                Param.Timer_threshold_value = 85;
            }
            if(Flag.Is_Timer_Up) {
                Flag.Start_Count = 0;
                Flag.Is_Timer_Up = 0;
                Flag.Run_Step = 8;
            }
            break;
            
        case 8: // ��ת60�����
            if(!Flag.Start_Count) {
                Kinematic_Analysis(100, 200, SERVO_INIT + 300);
                Flag.Start_Count = 1;
                Param.Timer_threshold_value = 150;
            }
            if(Flag.Is_Timer_Up) {
                Flag.Start_Count = 0;
                Flag.Is_Timer_Up = 0;
                Flag.Is_Stop_Car = 1;
                Flag.Run_Step = 9;
            }
            break;
            
        case 9: // ����ͣ��
            if(Flag.Is_Stop_Car && !Flag.Start_Count) {
                Kinematic_Analysis(0, 0, 0.0);
							Usart2_SendString("1finish");
            }
            break;
            
        default:
            Flag.Run_Step = 1;
            break;
    }
}

/**
  * @brief �෽ͣ������
  */
static void Side_Parking_Control(void)
{
    switch(Flag.Run_Step)
    {
        case 1: // ֱ�м��
            if(Flag.Is_Go_straight) {
                Kinematic_Analysis(100, 100, yaw);
//                openMv_Proc();
									raspi_rxData=raspi_rx();
                if(raspi_rxData == 1) {
                    Flag.Is_Go_straight = 0;
                    Flag.Is_Stop_Car = 1;
                    Flag.Run_Step = 2;
                }
            }
            break;
            
        case 2: // ͣ��1��
            if(Flag.Is_Stop_Car && !Flag.Start_Count) {
                Kinematic_Analysis(0, 0, 0.0);
                Flag.Start_Count = 1;
                Param.Timer_threshold_value = 100;
                BEEP_ON;
            }
            if(Flag.Is_Timer_Up) {
                BEEP_OFF;
                Flag.Start_Count = 0;
                Flag.Is_Timer_Up = 0;
                Flag.Is_Stop_Car = 0;
                Flag.Is_Start_Astern = 1;
                Flag.Run_Step = 3;
            }
            break;
            
        case 3: // ��ת60�㵹��
            if(Flag.Is_Start_Astern == 1 && !Flag.Start_Count) {
                Kinematic_Analysis(0, -200, SERVO_INIT + 300);
                Flag.Is_Turn_Car = 1;
            }
            if(abs((int)yaw) >= 40) {
                Flag.Is_Turn_Car = 0;
                Flag.Is_Start_Astern = 2;
                Flag.Run_Step = 4;
            }
            break;
            
        case 4: // ��������1s
            if(Flag.Is_Start_Astern == 2 && !Flag.Start_Count) {
                Kinematic_Analysis(-100, -100, SERVO_INIT);
                Flag.Start_Count = 1;
                Param.Timer_threshold_value = 100;
            }
            if(Flag.Is_Timer_Up) {
                Flag.Start_Count = 0;
                Flag.Is_Timer_Up = 0;
                Flag.Is_Start_Astern = 3;
                Flag.Run_Step = 5;
            }
            break;
            
        case 5: // ��ת60�㵹��
            if(Flag.Is_Start_Astern == 3 && !Flag.Start_Count) {
                Kinematic_Analysis(-230, -100, SERVO_INIT - 300);
                Flag.Is_Turn_Car = 1;
            }
            if(abs((int)yaw) <= 5) {
                Flag.Is_Turn_Car = 0;
                Flag.Is_Start_Astern = 0;
                Flag.Run_Step = 6;
            }
            break;
            
        case 6: // ֱ�е���λ��
            if(Flag.Is_Start_Astern == 0 && !Flag.Start_Count) {
                Kinematic_Analysis(100, 100, SERVO_INIT);
                Flag.Start_Count = 1;
                Param.Timer_threshold_value = 100;
            }
            if(Flag.Is_Timer_Up) {
                Flag.Start_Count = 0;
                Flag.Is_Timer_Up = 0;
                Flag.Is_Stop_Car = 1;
                Flag.Run_Step = 7;
            }
            break;
            
        case 7: // ͣ��5��
            if(Flag.Is_Stop_Car && !Flag.Start_Count) {
                Kinematic_Analysis(0, 0, 0.0);
                Flag.Start_Count = 1;
                Param.Timer_threshold_value = 500;
                BEEP_ON;
            }
            if(Flag.Is_Timer_Up) {
                BEEP_OFF;
                Flag.Start_Count = 0;
                Flag.Is_Timer_Up = 0;
                Flag.Is_Stop_Car = 0;
                Flag.Run_Step = 8;
            }
            break;
            
        case 8: // ����0.6s
            if(!Flag.Is_Timer_Up && !Flag.Start_Count) {
                Kinematic_Analysis(-100, -100, SERVO_INIT);
                Flag.Start_Count = 1;
                Param.Timer_threshold_value = 60;
            }
            if(Flag.Is_Timer_Up) {
                Flag.Start_Count = 0;
                Flag.Is_Timer_Up = 0;
                Flag.Run_Step = 9;
            }
            break;
            
        case 9: // ��ת60�����
            if(!Flag.Is_Stop_Car) {
                Kinematic_Analysis(200, 0, SERVO_INIT - 300);
                Flag.Is_Turn_Car = 1;
            }
            if(abs((int)yaw) >= 40) {
                Flag.Is_Turn_Car = 0;
                Flag.Run_Step = 10;
            }
            break;
            
        case 10: // ����ֱ��0.8s
            if(!Flag.Is_Timer_Up && !Flag.Start_Count) {
                Kinematic_Analysis(100, 100, 0.0);
                Flag.Start_Count = 1;
                Param.Timer_threshold_value = 80;
            }
            if(Flag.Is_Timer_Up) {
                Flag.Start_Count = 0;
                Flag.Is_Timer_Up = 0;
                Flag.Run_Step = 11;
            }
            break;
            
        case 11: // ��ת60���λ
            if(!Flag.Start_Count) {
                Kinematic_Analysis(100, 200, SERVO_INIT + 300);
                Flag.Is_Turn_Car = 1;
            }
            if(abs((int)yaw) <= 5) {
                Flag.Is_Turn_Car = 0;
                Flag.Is_Stop_Car = 1;
                Flag.Run_Step = 12;
            }
            break;
            
        case 12: // ����ͣ��
            if(Flag.Is_Stop_Car && !Flag.Start_Count) {
                Kinematic_Analysis(0, 0, 0.0);
								Usart2_SendString("2finish");     
						}
            break;
            
        default:
            Flag.Run_Step = 1;
            break;
    }
}

/**
  * @brief ����ͣ������
  */
static void Back_Side_Parking_Control(void)
{
    switch(Flag.Run_Step)
    {
        case 1: // ֱ�м��
            if(Flag.Is_Go_straight) {
                Kinematic_Analysis(100, 100, yaw);
//                openMv_Proc();
								raspi_rxData=raspi_rx();
                if(raspi_rxData == 1) {
                    Flag.Is_Go_straight = 0;
                    Flag.Is_Stop_Car = 1;
                    Flag.Run_Step = 2;
                }
            }
            break;
            
        case 2: // ͣ��1��
            if(Flag.Is_Stop_Car && !Flag.Start_Count) {
                Kinematic_Analysis(0, 0, 0.0);
                Flag.Start_Count = 1;
                Param.Timer_threshold_value = 100;
                BEEP_ON;
            }
            if(Flag.Is_Timer_Up) {
                BEEP_OFF;
                Flag.Start_Count = 0;
                Flag.Is_Timer_Up = 0;
                Flag.Is_Stop_Car = 0;
                Flag.Is_Start_Astern = 1;
                Flag.Run_Step = 3;
            }
            break;
            
        case 3: // ����0.72s
            if(Flag.Is_Start_Astern == 1 && !Flag.Start_Count) {
                Kinematic_Analysis(-100, -100, SERVO_INIT);
                Flag.Start_Count = 1;
                Param.Timer_threshold_value = 72;
            }
            if(Flag.Is_Timer_Up) {
                Flag.Start_Count = 0;
                Flag.Is_Timer_Up = 0;
                Flag.Run_Step = 4;
            }
            break;
            
        case 4: // ��ת60�㵹��
            if(Flag.Is_Start_Astern == 1 && !Flag.Start_Count) {
                Kinematic_Analysis(-50, -150, SERVO_INIT + 300);
                Flag.Is_Turn_Car = 1;
            }
            if(abs((int)yaw) >= 80) {
                Flag.Is_Turn_Car = 0;
                Flag.Is_Start_Astern = 2;
                Flag.Run_Step = 5;
            }
            break;
            
        case 5: // ��ͷ��������1.5s
            if(Flag.Is_Start_Astern == 2 && !Flag.Start_Count) {
                Kinematic_Analysis(-100, -100, 0.0);
                Flag.Start_Count = 1;
                Param.Timer_threshold_value = 150;
            }
            if(Flag.Is_Timer_Up) {
                Flag.Start_Count = 0;
                Flag.Is_Timer_Up = 0;
                Flag.Is_Start_Astern = 0;
                Flag.Is_Stop_Car = 1;
                Flag.Run_Step = 6;
            }
            break;
            
        case 6: // ͣ��5��
            if(Flag.Is_Stop_Car && !Flag.Start_Count) {
                Kinematic_Analysis(0, 0, 0.0);
                Flag.Start_Count = 1;
                Param.Timer_threshold_value = 500;
                BEEP_ON;
            }
            if(Flag.Is_Timer_Up) {
                BEEP_OFF;
                Flag.Start_Count = 0;
                Flag.Is_Timer_Up = 0;
                Flag.Is_Stop_Car = 0;
                Flag.Run_Step = 7;
            }
            break;
            
        case 7: // ֱ�г���0.75s
            if(!Flag.Is_Stop_Car) {
                Kinematic_Analysis(100, 100, 0.0);
                Flag.Start_Count = 1;
                Param.Timer_threshold_value = 75;
            }
            if(Flag.Is_Timer_Up) {
                Flag.Start_Count = 0;
                Flag.Is_Timer_Up = 0;
                Flag.Run_Step = 8;
            }
            break;
            
        case 8: // ��ת60��׼���෽
            if(!Flag.Start_Count) {
                Kinematic_Analysis(0, 200, SERVO_INIT + 300);
                Flag.Is_Turn_Car = 1;
            }
            if(abs((int)yaw) <= 5) {
                Flag.Is_Turn_Car = 0;
                Flag.Is_Go_straight = 1;
                //Param.openMV_Data = 0;
							//����ִ�вⷽ���������ź�
                Usart2_SendString("1finish");
                Flag.Run_Step = 9;
            }
            break;
            
        case 9: // ֱ�м��෽
            if(Flag.Is_Go_straight) {
                Kinematic_Analysis(100, 100, yaw);
//                openMv_Proc();
									raspi_rxData=raspi_rx();
                if(raspi_rxData == 2) {
                    Flag.Is_Go_straight = 0;
                    Flag.Is_Stop_Car = 1;
                    Flag.Run_Step = 10;
                }
            }
            break;
            
        case 10: // ͣ��1��
            if(Flag.Is_Stop_Car && !Flag.Start_Count) {
                Kinematic_Analysis(0, 0, 0.0);
                Flag.Start_Count = 1;
                Param.Timer_threshold_value = 100;
                BEEP_ON;
            }
            if(Flag.Is_Timer_Up) {
                BEEP_OFF;
                Flag.Start_Count = 0;
                Flag.Is_Timer_Up = 0;
                Flag.Is_Stop_Car = 0;
                Flag.Is_Start_Astern = 1;
                Flag.Run_Step = 11;
            }
            break;
            
        case 11: // ��ת60�㵹��
            if(Flag.Is_Start_Astern == 1 && !Flag.Start_Count) {
                Kinematic_Analysis(0, -150, SERVO_INIT + 300);
                Flag.Is_Turn_Car = 1;
            }
            if(abs((int)yaw) == 40) {
                Flag.Is_Turn_Car=0;//������ת��
//								Flag.Start_Count=0;//����������
//								Flag.Is_Timer_Up=0;//��ʱʱ�䵽��־λ����
								Flag.Is_Start_Astern=2;//��ʼ�����ڶ���
								Flag.Run_Step=12;//��ת��һ��
            }
            break;
            
        case 12: // ��������1s
            if(Flag.Is_Start_Astern == 2 && !Flag.Start_Count) {
                Kinematic_Analysis(-100,-100,SERVO_INIT);								
							  Flag.Start_Count=1;//��ʼ��ʱ
							  Param.Timer_threshold_value=100;//��ʱ1s
            }
            if(Flag.Is_Timer_Up) { //��ʱʱ�䵽
                Flag.Start_Count=0;//����������
								Flag.Is_Timer_Up=0;//��ʱʱ�䵽��־λ����
								Flag.Is_Start_Astern=3;//���㵹������
                Flag.Run_Step = 13;
            }
            break;
            
        case 13: // ��ת60�㵹��
            if(Flag.Is_Start_Astern == 3 && !Flag.Start_Count) {
                Kinematic_Analysis(-200, -100, SERVO_INIT - 300);
                Flag.Is_Turn_Car = 1;  //���㵹������//��ʼת��
            }
            if(abs((int)yaw) <= 5) {
//                Flag.Is_Turn_Car = 0; //������ת��
//							  Flag.Is_Timer_Up=0;//��ʱʱ�䵽��־λ����
							  Flag.Start_Count=0; //����������
                Flag.Is_Start_Astern = 0; //���㵹������
                Flag.Run_Step = 14;
            }
            break;
            
        case 14: // ֱ�е���λ��
            if(Flag.Is_Start_Astern == 0 && !Flag.Start_Count) {
                Kinematic_Analysis(100, 100, SERVO_INIT);
                Flag.Start_Count = 1; //��ʼ��ʱ
                Param.Timer_threshold_value = 100; //��ʱ1s
            }
            if(Flag.Is_Timer_Up) { //��ʱʱ�䵽
                Flag.Start_Count = 0; //����������
                Flag.Is_Timer_Up = 0; //��ʱʱ�䵽��־λ����
                Flag.Is_Stop_Car = 1; //��ͣ��
                Flag.Run_Step = 15;
            }
            break;
            
        case 15: // ͣ��5�� //ͣ�����������������죬��ʱ5s
            if(Flag.Is_Stop_Car && !Flag.Start_Count) {
                Kinematic_Analysis(0, 0, 0.0);
                Flag.Start_Count = 1; //��ʼ��ʱ
                Param.Timer_threshold_value = 500; //��ʱ5s
                BEEP_ON; //��������
            }
            if(Flag.Is_Timer_Up) { //��ʱʱ�䵽
                BEEP_OFF; //�ط�����
                Flag.Start_Count = 0; //����������
                Flag.Is_Timer_Up = 0; //��ʱʱ�䵽��־λ����
                Flag.Is_Stop_Car = 0; //��ͣ��
                Flag.Run_Step = 16; //��ת��һ��
            }
            break;
						
            /*����*/
        case 16: // ����0.6s
            if(!Flag.Is_Timer_Up && !Flag.Start_Count) {
                Kinematic_Analysis(-100, -100, SERVO_INIT);
                Flag.Start_Count = 1; //��ʼ��ʱ
                Param.Timer_threshold_value = 60; //��ʱ0.6s
            }
            if(Flag.Is_Timer_Up) {
                Flag.Start_Count = 0; //����������
                Flag.Is_Timer_Up = 0; //��ʱʱ�䵽��־λ����
							  Flag.Is_Stop_Car=0; //��ͣ��
                Flag.Run_Step = 17;
            }
            break;
            
        case 17: // ��ת60�㣬ǰ��������ת�����
            if(!Flag.Is_Stop_Car) {
                Kinematic_Analysis(150, 0, SERVO_INIT - 300);
                Flag.Is_Turn_Car = 1; // ��ʼת��
            }
            if(abs((int)yaw) >= 40) { // �Ƕ�Ϊ-40�����
							  Flag.Is_Turn_Car=0; // ������ת��
//								Flag.Start_Count=0; // ����������
//                Flag.Is_Turn_Car = 0; // ��ʱʱ�䵽��־λ����
                Flag.Run_Step = 18; // ��ת��һ��
            }
            break;
            
        case 18: // ������ֱ�У�������ת�䣬��ʱ0.8s
            if(!Flag.Is_Timer_Up && !Flag.Start_Count) {
                Kinematic_Analysis(100, 100, 0.0);
                Flag.Start_Count = 1; // ��ʼ��ʱ
                Param.Timer_threshold_value = 80; // ��ʱ0.8s
            }
            if(Flag.Is_Timer_Up) { // ��ʱʱ�䵽
                Flag.Start_Count = 0; // ����������
                Flag.Is_Timer_Up = 0; // ��ʱʱ�䵽��־λ����
                Flag.Run_Step = 19; // ��ת��һ��
            }
            break;
            
        case 19: // ��ת60���λ
            if(!Flag.Start_Count) {
                Kinematic_Analysis(100, 150, SERVO_INIT + 300.0);
                Flag.Is_Turn_Car = 1; // ��ʼת��
            }
            if(abs((int)yaw) <= 5) {
                Flag.Is_Turn_Car = 0; // ������ת��
//                Flag.Is_Stop_Car = 1; // ����������
//							  Flag.Is_Timer_Up=0; // ��ʱʱ�䵽��־λ����
								Flag.Is_Stop_Car=1; // ͣ��	
                Flag.Run_Step = 20; // ��ת��һ��
            }
            break;
            
        case 20: // ����ͣ��
            if(Flag.Is_Stop_Car && !Flag.Start_Count) {
                Kinematic_Analysis(0, 0, 0.0);
							Usart2_SendString("2finish");
            }
            break;
            
        default:
            Flag.Run_Step = 1;
            break;
    }
}

/**
  * @brief ���PID����
  */
void Position_PID_Servo_Realize(void)
{
    Param.Servo_Speed = Position_PID_Servo(Param.Servo_Target_Val);
    Param.Servo_Target_Position += Param.Servo_Speed;
}

/**
  * @brief �˶�ѧ����
  */
void Kinematic_Analysis(float velocity1, float velocity2, float angle)
{
    // �Ƕ��޷� (��60��)
    angle = angle > 60.0f ? 60.0f : (angle < -60.0f ? -60.0f : angle);
    
    // ֱ��ģʽ
    if(Flag.Is_Go_straight || Flag.Is_Start_Astern != 1) {
        PID.Motor1_Velocity_Target_Val = velocity1;
        PID.Motor2_Velocity_Target_Val = velocity2;
    }
    // ����ת��ģʽ
    else if(Flag.Is_Start_Astern == 1) {
        float tan_angle = tanf(angle * M_PI / 180.0f);
        PID.Motor1_Velocity_Target_Val = velocity1 * (1 - T * tan_angle / (2 * L));
        PID.Motor2_Velocity_Target_Val = velocity1 * (1 + T * tan_angle / (2 * L));
    }
    
    // ���Ŀ��λ�ü���
    Param.Servo_Target_Val = SERVO_INIT + angle * 5;
}

/**
  * @brief ��ʱ��������
  */
static void Timer_Count_Handler(void)
{
    if(Flag.Start_Count) {
        static uint16_t Timer_Cnt = 0;
        if(++Timer_Cnt >= Param.Timer_threshold_value) {
            Timer_Cnt = 0;
            Flag.Is_Timer_Up = 1;
        }
    }
}

/**
  * @brief ���ƺ���
  */
void Limit(int *motoA, int *motoB, float *servo)
{
    *motoA = (*motoA > 6500) ? 6500 : ((*motoA < -6500) ? -6500 : *motoA);
    *motoB = (*motoB > 6500) ? 6500 : ((*motoB < -6500) ? -6500 : *motoB);
    *servo = (*servo > 260) ? 260 : ((*servo < 36) ? 36 : *servo);
}



/*************************************END OF FILE************************************/
