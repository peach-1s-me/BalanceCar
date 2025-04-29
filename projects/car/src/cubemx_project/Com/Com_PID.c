#include "Com_PID.h"

/**
 * @description: ֱ����PD����
 * @param {float} kp  pϵ��
 * @param {float} kd  dϵ��
 * @param {float} kalman_angle �������˲���ĽǶ�
 * @param {float} balance_angle ������ƽ��ʱ�ĽǶ�
 * @param {short} gy y����ٶ�ԭʼֵ
 * @return {*}
 */
int Com_PID_Balance(float kp, float kd, float kalman_angle, float balance_angle, short gy)
{
    /* 1.����Ƕ�ƫ�� */
    float bias_angle = 0.0f;
    bias_angle = kalman_angle - balance_angle;
    /* 2.����PID���� */
    return kp * bias_angle + kd * gy;
}

/**
 * @description: �ٶȻ�PI����
 * @param {float} kp pϵ��
 * @param {float} ki iϵ��
 * @param {int} encoder_a A��������ı���ֵ
 * @param {int} encoder_b B��������ı���ֵ
 * @param {int} remove_move ң��ǰ���˶��Ŀ�����
 * @return {*}
 */
int Com_PID_Velocity(float kp, float ki, int encoder_a, int encoder_b,int remove_move)
{
    int bias_velocity = 0;
    static int least_velocity = 0; // �ٶȻ���ֵ���ۼ�ֵ��
    static int last_velocity = 0;  // ��һ�ε��ٶ�ƫ��ֵ
    /* 1.�����ٶ�ƫ�� */
    bias_velocity = (encoder_a + encoder_b) - 0;

    /* 2.���ٶ�ƫ����һ���˲���һ�׵�ͨ������Ϊ����ʹ�õ��ڵķ�ʽ�������ٶȻ���Ӱ�죬ֻ�Ǹ��� */
    /* ���ձ��ε�ֵ = k*�ϴε�ֵ + ��1-k��*���ε�ֵ */
    bias_velocity = 0.8 * last_velocity + 0.2 * bias_velocity;
    /* ���汾�ε�ƫ��ֵ����һ�μ����� */
    last_velocity = bias_velocity;

    /* 3.�������(û�г�ʱ�䣬��ͷ����ki�Ĵ�С�Ϳ���) */
    least_velocity += bias_velocity;
    /* ң��ǰ���˶�����λ���������� */
    least_velocity -= remove_move;

    /* 4.�����޷���һ����ֲ�������Ҫ�����޷������⣩ */
    /* ���ٶ�ƫ����ۼӣ� �ٶ�ȡֵ��-32768~32767֮�䣬���ǻ�������1/3�ķ�Χ�� */
    if(least_velocity > 10000)
    {
        least_velocity = 10000;
    }
    else if(least_velocity < -10000)
    {
        least_velocity = -10000;
    }

    /* 5.����PID */
    return kp * bias_velocity + ki * least_velocity;
}

/**
 * @description: ת��P���ƣ��������ã�
 * @param {float} kp pϵ��
 * @param {short} gz z����ٶ�
 * @return {*}
 */
int Com_PID_Turn(float kp,short gz)
{
    /* kp*Z����ٶ� */
    return kp*(gz-0);
}
