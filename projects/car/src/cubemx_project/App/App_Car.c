#include "App_Car.h"

short gx, gy, gz;
short ax, ay, az;

float accel_angle;  // ͨ�����ٶȼ�������ĽǶ�
float gyro_y;       // Y����ٶȣ�����ֵת���ɽǶ�
extern float angle; // �������˲���ĽǶ�

int ea, eb;

char bat_str[5];   // 2������λ+1��С����+1��С��λ+1���ַ�����β\0
char ea_str[7];    // 1������λ+5������λ+1���ַ�����β\0
char eb_str[7];    // 1������λ+5������λ+1���ַ�����β\0
char angle_str[7]; // 1������λ+3������λ+1��С����+1��С��λ+1���ַ�����β\0

/* =======ֱ����PID����========= */
float balance_kp = -720.0;  // �ճ��ֵ�Ƶ�𵴵�ֵ=-900����0.6=-540
float balance_kd = 0.72;    // �ճ��ָ�Ƶ�𵴵�ֵ+1.5����0.6=+0.9
float balance_angle = -1.0; // -1.0
/* =======�ٶȻ�PID����========= */
float velocity_kp = 170.0; // +220
float velocity_ki = 0.85;  // +1.10
/* =======ת��PID����========= */
float turn_kp = 0.5; // 0.5

/* ң���˶��ı�־λ */
uint8_t flag_up = 0, flag_down = 0, flag_left = 0, flag_right = 0;
int remote_move = 0; // ң��ǰ���ƶ��Ŀ�����
int remote_turn = 0; // ң�������ƶ��Ŀ�����

/**
 * @description: ����С�����
 * @return {*}
 */
void App_Car_GetAngle(void)
{
    /* 1. ��ȡMPU6050������ */
    Int_MPU6050_Get_Accel(&ax, &ay, &az);
    Int_MPU6050_Get_Gyro(&gx, &gy, &gz);

    /* 2. ͨ�����ٶȼ������ */
    /* atan2�õ����ǻ��ȣ� �Ƕ� = ���� * 180/PI */
    accel_angle = atan2(ax, az) * 180 / PI;

    /* 3. ���ٶȣ�������+-2000��/s ��65536/4000=16.4  */
    /* ע�⣬���ٶȵķ���Ҫ�ͼ��ٶȼ���ĽǶȷ��ŷ���һ�� */
    gyro_y = -gy / 16.4;

    /* 4. �������Ǻͽ��ٶȣ����п������˲� */
    Com_Filter_Kalman(accel_angle, gyro_y);

    // printf("accel_angle=%.1f\r\n", accel_angle);
    // printf("gyro_y=%.1f\r\n", gyro_y);
    // printf("angle=%.1f\r\n", angle);

    /* ����ȡ��������ֵ��Ҳ�ŵ���ȡ�Ƕȵĺ����У������������ݾͿ���ͬƵ */
    ea = Int_Encoder_ReadCounter(2);
    eb = -Int_Encoder_ReadCounter(3);
}

/**
 * @description: ��ʾ��������ص�ѹֵ��������������ֵ������ĽǶ�ֵ
 * @return {*}
 */
void App_Car_Display(void)
{
    /* 1.����ѹֵ */
    double bat_vol = 0.0;
    /* ����������ת����ÿ�ζ�ȡǰ�ֶ�����adc */
    HAL_ADC_Start(&hadc1);
    bat_vol = (HAL_ADC_GetValue(&hadc1) * 3.3 / 4095) * 4;

    cat_sprintf(bat_str, "%3.1f", bat_vol);
    OLED_ShowString(32, 0, (uint8_t *)bat_str, 16, 1);

    /* 2.��������ֵ */
    cat_sprintf(ea_str, "%6d", ea);
    cat_sprintf(eb_str, "%6d", eb);
    OLED_ShowString(24, 16, (uint8_t *)ea_str, 16, 1); // ǰ����EA:�����ַ���x=3*8=24��ʼ;�ڶ��У�y=16
    OLED_ShowString(24, 32, (uint8_t *)eb_str, 16, 1); // ǰ����EB:�����ַ���x=3*8=24��ʼ;�����У�y=32

    /* 3.���Ƕ�ֵ */
    cat_sprintf(angle_str, "%5.1f", angle);
    OLED_ShowString(48, 48, (uint8_t *)angle_str, 16, 1); // ǰ����Angle:�����ַ���x=6*8=48��ʼ;�����У�y=48

    /* 4.ˢд���Դ��� */
    OLED_Refresh();
}

void App_Car_PID(void)
{
    int balance_out = 0;
    int velocity_out = 0;
    int turn_out = 0;
    int pwma = 0, pwmb = 0;
    /* 1.ֱ�������� */
    balance_out = Com_PID_Balance(balance_kp, balance_kd, angle, balance_angle, gy);

    /* 2.�ٶȻ����� */
    /* ���ң��ǰ���ƶ����߼� */
    if (flag_up)
    {
        remote_move = 50;
    }
    else if (flag_down)
    {
        remote_move = -50;
    }
    else
    {
        /* ���û��ǰ����ƣ���Ҫ�����������������ҪӰ��������ƽ����� */
        remote_move = 0;
    }
    velocity_out = Com_PID_Velocity(velocity_kp, velocity_ki, ea, eb, remote_move);

    /* 3.ת�򻷿��� */
    if (flag_left)
    {
        remote_turn += -20;
    }
    else if (flag_right)
    {
        remote_turn += 20;
    }
    else
    {
        /* ���������ת��Ҫ��������� */
        remote_turn = 0;
        /* ������ת���Ž���ת�򻷵Ŀ��� */
        turn_out = Com_PID_Turn(turn_kp, gz);
    }
    /* һֱ��ס�᲻���ۼӣ����ԼӸ��޷� */
    if (remote_turn > 500)
    {
        remote_turn = 500;
    }
    else if (remote_turn < -500)
    {
        remote_turn = -500;
    }

    /* 4.����PID��������õ������ */
    pwma = balance_out + velocity_out + turn_out + remote_turn;
    pwmb = balance_out + velocity_out - turn_out - remote_turn;
    Int_TB6612_SetPWM(pwma, pwmb);
}

/**
 * @description: USART2���жϴ�����������ң����ص��߼�
 * @return {*}
 */
// extern uint8_t buff[1];
// void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
// {
//     if (huart->Instance == USART2)
//     {
//         switch (buff[0])
//         {
//         case 'U':
//             flag_up = 1, flag_down = 0, flag_left = 0, flag_right = 0;
//             break;
//         case 'D':
//             flag_up = 0, flag_down = 1, flag_left = 0, flag_right = 0;
//             break;
//         case 'L':
//             flag_up = 0, flag_down = 0, flag_left = 1, flag_right = 0;
//             break;
//         case 'R':
//             flag_up = 0, flag_down = 0, flag_left = 0, flag_right = 1;
//             break;
//         case 'S':
//             flag_up = 0, flag_down = 0, flag_left = 0, flag_right = 0;
//             break;
//         default:
//             flag_up = 0, flag_down = 0, flag_left = 0, flag_right = 0;
//             break;
//         }
//     }
//     HAL_UART_Receive_IT(&huart2, buff, 1);
// }

void deal_remote_control(cat_u8 cmd)
{
    switch (cmd)
    {
    case 'U':
        flag_up = 1, flag_down = 0, flag_left = 0, flag_right = 0;
        break;
    case 'D':
        flag_up = 0, flag_down = 1, flag_left = 0, flag_right = 0;
        break;
    case 'L':
        flag_up = 0, flag_down = 0, flag_left = 1, flag_right = 0;
        break;
    case 'R':
        flag_up = 0, flag_down = 0, flag_left = 0, flag_right = 1;
        break;
    case 'S':
        flag_up = 0, flag_down = 0, flag_left = 0, flag_right = 0;
        break;
    default:
        flag_up = 0, flag_down = 0, flag_left = 0, flag_right = 0;
        break;
    }
}
