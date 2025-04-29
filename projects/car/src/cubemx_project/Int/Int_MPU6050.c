#include "Int_MPU6050.h"

/**
 * @description: ��ȡָ���Ĵ���һ���ֽ�����
 * @param {uint8_t} reg_addr
 * @param {uint8_t} *receive_byte
 * @return {*}
 */
uint8_t Int_MPU6050_ReadByte(uint8_t reg_addr, uint8_t *receive_byte)
{
    return HAL_I2C_Mem_Read(&hi2c2, MPU_IIC_ADDR << 1, reg_addr, I2C_MEMADD_SIZE_8BIT, receive_byte, 1, 2000);
}

/**
 * @description: ��ȡָ���Ĵ�������ֽ�����
 * @param {uint8_t} reg_addr
 * @param {uint8_t} *receive_buff
 * @param {uint8_t} size
 * @return {*}
 */
uint8_t Int_MPU6050_ReadBytes(uint8_t reg_addr, uint8_t *receive_buff, uint8_t size)
{
    return HAL_I2C_Mem_Read(&hi2c2, MPU_IIC_ADDR << 1, reg_addr, I2C_MEMADD_SIZE_8BIT, receive_buff, size, 2000);
}

/**
 * @description: ��MPU6050д��һ���ֽ�
 * @param {uint8_t} reg_addr
 * @param {uint8_t} write_byte
 * @return {*}
 */
uint8_t Int_MPU6050_WriteByte(uint8_t reg_addr, uint8_t write_byte)
{
    return HAL_I2C_Mem_Write(&hi2c2, MPU_IIC_ADDR << 1, reg_addr, I2C_MEMADD_SIZE_8BIT, &write_byte, 1, 2000);
}

/**
 * @description: ��MPU6050д�����ֽ�
 * @param {uint8_t} reg_addr
 * @param {uint8_t} *write_bytes
 * @param {uint8_t} size
 * @return {*}
 */
uint8_t Int_MPU6050_WriteBytes(uint8_t reg_addr, uint8_t *write_bytes, uint8_t size)
{
    return HAL_I2C_Mem_Write(&hi2c2, MPU_IIC_ADDR << 1, reg_addr, I2C_MEMADD_SIZE_8BIT, write_bytes, size, 2000);
}

/**
 * @description: ���ݲ��������õ�ͨ�˲���
 * @param {uint16_t} rate
 * @return {*}
 */
void Int_MPU6050_Set_DLPF_CFG(uint16_t rate)
{
    /* �������� ������ >= 2*���� �Ų���ʧ�� ===�� ���� <= ������/2 */
    uint8_t cfg = 0;
    rate = rate / 2;
    if (rate > 188)
    {
        cfg = 1;
    }
    else if (rate > 98)
    {
        cfg = 2;
    }
    else if (rate > 42)
    {
        cfg = 3;
    }
    else if (rate > 20)
    {
        cfg = 4;
    }
    else if (rate > 10)
    {
        cfg = 5;
    }
    else
    {
        cfg = 6;
    }
    Int_MPU6050_WriteByte(MPU_CFG_REG, cfg << 0);
}

/**
 * @description: ���������ǵĲ�����
 * @param {uint16_t} rate
 * @return {*}
 */
void Int_MPU6050_SetGyroRate(uint16_t rate)
{
    /* ������=���Ƶ��/(1+��Ƶֵ) ===�� ��Ƶֵ = (���Ƶ��/������) -1 */
    uint8_t sample_div = 0;
    /* 1. �������޷�:��Ʒ˵����12ҳ�����Ƶ����Сֵ=4�������8k���������������1k */
    if (rate < 4)
    {
        rate = 4;
    }
    else if (rate > 1000)
    {
        rate = 1000;
    }

    /* 2. ���������Ĳ����ʣ��������Ƶֵ */
    sample_div = 1000 / rate - 1;

    /* 3. ����Ƶֵ���õ��Ĵ����� */
    Int_MPU6050_WriteByte(MPU_SAMPLE_RATE_REG, sample_div);

    /* 4. ���ݲ�����ȥ���õ�ͨ�˲��� */
    Int_MPU6050_Set_DLPF_CFG(rate);
}

/**
 * @description: ��ʼ��
 * @return {*}
 */
void Int_MPU6050_Init(void)
{
    uint8_t dev_id = 0;

    /* 2 ��λ -���ӳ�һ�� -�� ���� */
    Int_MPU6050_WriteByte(MPU_PWR_MGMT1_REG, 0x80);
    HAL_Delay(300);
    Int_MPU6050_WriteByte(MPU_PWR_MGMT1_REG, 0x00);

    /* 3 ���������� +-2000��/s, fsr=3 */
    Int_MPU6050_WriteByte(MPU_GYRO_CFG_REG, 3 << 3);

    /* 4 ���ٶ����� */
    Int_MPU6050_WriteByte(MPU_ACCEL_CFG_REG, 0 << 3);

    /* 5 �����������ã���ѡ����FIFO���ڶ�IIC���ж� */
    Int_MPU6050_WriteByte(MPU_INT_EN_REG, 0x00);    // �ر������ж�
    Int_MPU6050_WriteByte(MPU_USER_CTRL_REG, 0x00); // �رյڶ�IIC
    Int_MPU6050_WriteByte(MPU_FIFO_EN_REG, 0x00);   // �ر�FIFO

    /* 6 ϵͳʱ��Դ�������ǲ����ʡ���ͨ�˲������� */
    /* ����ʱ��Դ֮ǰ��ȷ��������������һ��id */
    Int_MPU6050_ReadByte(MPU_DEVICE_ID_REG, &dev_id);
    if (dev_id == MPU_IIC_ADDR)
    {
        /* 6.1 ����ʱ��Դ��������X���ʱ�ӣ����ȸ��� */
        Int_MPU6050_WriteByte(MPU_PWR_MGMT1_REG, 0x01);
        /* 6.2 ���������ǲ����ʡ���ͨ�˲� */
        Int_MPU6050_SetGyroRate(100);
        /* 6.3 �������������˳�����ģʽ��������������״̬*/
        Int_MPU6050_WriteByte(MPU_PWR_MGMT2_REG, 0x00);
    }
}

/**
 * @description: ��ȡ�����ǽ��ٶȵ����ݣ�ƴ�����ֽ�������ת�ɴ����ŵ�����
 * @param {short} *gx
 * @param {short} *gy
 * @param {short} *gz
 * @return {*}
 */
void Int_MPU6050_Get_Gyro(short *gx, short *gy, short *gz)
{
    uint8_t buff[6];
    Int_MPU6050_ReadBytes(MPU_GYRO_XOUTH_REG, buff, 6);

    /*
        buff[0]:���ٶ�X���8λ
        buff[1]:���ٶ�X���8λ
        buff[2]:���ٶ�Y���8λ
        buff[3]:���ٶ�Y���8λ
        buff[4]:���ٶ�Z���8λ
        buff[5]:���ٶ�Z���8λ
     */
    *gx = ((short)buff[0] << 8) | buff[1];
    *gy = ((short)buff[2] << 8) | buff[3];
    *gz = ((short)buff[4] << 8) | buff[5];
}

/**
 * @description: ��ȡ���ٶȼƵļ��ٶ����ݣ�ƴ�����ֽ�������ת�ɴ����ŵ�����
 * @param {short} *ax
 * @param {short} *ay
 * @param {short} *az
 * @return {*}
 */
void Int_MPU6050_Get_Accel(short *ax, short *ay, short *az)
{
    uint8_t buff[6];
    Int_MPU6050_ReadBytes(MPU_ACCEL_XOUTH_REG, buff, 6);

    /*
        buff[0]:���ٶ�X���8λ
        buff[1]:���ٶ�X���8λ
        buff[2]:���ٶ�Y���8λ
        buff[3]:���ٶ�Y���8λ
        buff[4]:���ٶ�Z���8λ
        buff[5]:���ٶ�Z���8λ
     */
    *ax = ((short)buff[0] << 8) | buff[1];
    *ay = ((short)buff[2] << 8) | buff[3];
    *az = ((short)buff[4] << 8) | buff[5];
}
