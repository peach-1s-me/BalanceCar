#include "Int_TB6612.h"





/**
 * @description: ���Ƶ��A�ķ���
 * @param {uint8_t} direct ������ת������
 * @return {*}
 */
void Int_TB6612_MotorA(uint8_t direct)
{
    if(direct == GO)
    {
        /* ��ת��AIN1=0,AIN2=1 */
       AIN1_L;
       AIN2_H;
    }
    else if(direct == BACK)
    {
        /* ��ת��AIN1=1,AIN2=0 */
        AIN1_H;
        AIN2_L;
    }
    else 
    {
        /* ����ɲ����AIN1=1,AIN2=1 */
        AIN1_H;
        AIN2_H;
    }
}


/**
 * @description: ���Ƶ��B�ķ���
 * @param {uint8_t} direct ������ת������
 * @return {*}
 */
void Int_TB6612_MotorB(uint8_t direct)
{
    if(direct == GO)
    {
        /* ��ת��BIN1=0,BIN2=1 */
       BIN1_L;
       BIN2_H;
    }
    else if(direct == BACK)
    {
        /* ��ת��BIN1=1,BIN2=0 */
        BIN1_H;
        BIN2_L;
    }
    else 
    {
        /* ����ɲ����BIN1=1,BIN2=1 */
        BIN1_H;
        BIN2_H;
    }
}


/**
 * @description: ���ݴ����ŵ�pwmֵ������ת�ٺͷ���
 * @param {int} pwma   A�����������
 * @param {int} pwmb   B�����������
 * @return {*}
 */
void Int_TB6612_SetPWM(int pwma,int pwmb)
{
    /* ���ݴ����ŵ�pwmֵ��1��������2������pwmռ�ձ� */


    /* 1.����pwma */
    /* 1.1 ���ݷ��Ŵ����� */
    if(pwma > 0)
    {
        /* ��ת */
        Int_TB6612_MotorA(GO);
    }
    else if(pwma < 0)
    {
        /* ��ת */
        Int_TB6612_MotorA(BACK);
        /* Ϊ��������ccr��׼����ȡ����ֵ */
        pwma = -pwma;
    }
    else 
    {
        /* ɲ�� */
        Int_TB6612_MotorA(STOP);
    }
    /* 1.2 ���ö�Ӧ��pwmֵ =�� ��Ӧ��ʱ��ͨ��CH4��ccrֵ */
    // TIM4 ->CCR4 = pwma;
    __HAL_TIM_SetCompare(&htim4,TIM_CHANNEL_4,pwma);


    /* 2.����pwmb */
    /* 1.1 ���ݷ��Ŵ����� */
    if(pwmb > 0)
    {
        /* ��ת */
        Int_TB6612_MotorB(GO);
    }
    else if(pwmb < 0)
    {
        /* ��ת */
        Int_TB6612_MotorB(BACK);
        /* Ϊ��������ccr��׼����ȡ����ֵ */
        pwmb = -pwmb;
    }
    else 
    {
        /* ɲ�� */
        Int_TB6612_MotorB(STOP);
    }
    /* 1.2 ���ö�Ӧ��pwmֵ =�� ��Ӧ��ʱ��ͨ��CH3��ccrֵ */
    // TIM4 ->CCR3 = pwmb;
    __HAL_TIM_SetCompare(&htim4,TIM_CHANNEL_3,pwmb);
}



