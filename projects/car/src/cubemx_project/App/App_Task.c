#include "App_Task.h"

/* ��ȡ��̬���ݵ��������� */
cat_task_t data_task_handle;
#define DATA_TASK_STACK_SIZE 512 
cat_u8 data_task_stack[DATA_TASK_STACK_SIZE];
#define DATA_TASK_PRIORITY 3
void App_Task_GetData(void *pvParameters);

/* ����PID���Ƶ��������� */
cat_task_t pid_task_handle;
#define PID_TASK_STACK_SIZE 512
cat_u8 pid_task_stack[PID_TASK_STACK_SIZE];
#define PID_TASK_PRIORITY 3
void App_Task_PID(void *pvParameters);

/* ����PID���Ƶ��������� */
cat_task_t display_task_handle;
#define DISPLAY_TASK_STACK_SIZE 512 
cat_u8 display_task_stack[DISPLAY_TASK_STACK_SIZE];
#define DISPLAY_TASK_PRIORITY 3
void App_Task_Display(void *pvParameters);

cat_sem_t pid_sem;

void App_Task_Init(void)
{
    catos_init();

    cat_sem_init(&pid_sem, 0, 1);

    cat_task_create(
        "App_Task_GetData",
        &data_task_handle,
        App_Task_GetData,
        CAT_NULL,
        DATA_TASK_PRIORITY,
        data_task_stack,
        DATA_TASK_STACK_SIZE
        );
    cat_task_create(
        "App_Task_PID",
        &pid_task_handle,
        App_Task_PID,
        CAT_NULL,
        PID_TASK_PRIORITY,
        pid_task_stack,
        PID_TASK_STACK_SIZE
        );
    cat_task_create(
        "App_Task_Display",
        &display_task_handle,
        App_Task_Display,
        CAT_NULL,
        DISPLAY_TASK_PRIORITY,
        display_task_stack,
        DISPLAY_TASK_STACK_SIZE
        );

    /* 2. ���������� */
    catos_start_schedule();
}

/**
 * @description: ��ȡ��̬���ݣ��Ƕȡ�������ֵ
 * @param {void} *pvParameters
 * @return {*}
 */
void App_Task_GetData(void *pvParameters)
{
    (void)pvParameters;
    cat_ubase tick = 0;

    while(1)
    {
        tick = catos_get_systick();
        App_Car_GetAngle();
        /* ��ȡ�����ݣ�֪ͨpid�������� */
        cat_sem_post(&pid_sem);

        tick += catos_ms_to_tick(10);
        cat_task_delay_until(tick);
    }

}

/**
 * @description: ����PID���Ƶ�����
 * @param {void} *pvParameters
 * @return {*}
 */
void App_Task_PID(void *pvParameters)
{
    (void)pvParameters;
    while(1)
    {
        /* �ȴ�����ȡ�������񡱵�֪ͨ */
        cat_sem_get(&pid_sem, 1000);
        App_Car_PID();
    }
}

/**
 * @description: OLED��ʾ����
 * @param {void} *pvParameters
 * @return {*}
 */
void App_Task_Display(void *pvParameters)
{
    (void)pvParameters;
    cat_ubase tick = 0;

    while(1)
    {
        tick = catos_get_systick();
        App_Car_Display();
        tick += catos_ms_to_tick(50);
        cat_task_delay_until(tick);
    }

}
