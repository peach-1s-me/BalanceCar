#include "Int_Encoder.h"



/**
 * @description: ��ȡ������ģʽ�ļ���ֵ������ɴ����ŵ�ֵ
 * @param {uint8_t} timx Ҫ��ȡ�Ķ�ʱ���ı��
 * @return {*} �����Ĵ����ŵļ���ֵ
 */
int Int_Encoder_ReadCounter(uint8_t timx)
{
    int encoder_value = 0;
    switch (timx)
    {
    case 2:
    {
        /* ��ȡ��ʱ��2�ļ���ֵ������ת���ɴ��������� */
        // encoder_value = (short)TIM2->CNT;
        // TIM2->CNT = 0;
        encoder_value = (short)__HAL_TIM_GET_COUNTER(&htim2);
        __HAL_TIM_SET_COUNTER(&htim2, 0);
        break;
    }
    case 3:
    {
        /* ��ȡ��ʱ��3�ļ���ֵ������ת���ɴ��������� */
        // encoder_value = (short)TIM3->CNT;
        // TIM3->CNT = 0;
        encoder_value = (short)__HAL_TIM_GET_COUNTER(&htim3);
        __HAL_TIM_SET_COUNTER(&htim3, 0);
        break;
    }
    default:
        break;
    }

    /* ���ڵ����װ�����180�ȣ�������ȡʱ��Ҫ�ֶ�ͳһ������� */
    return encoder_value;
}
