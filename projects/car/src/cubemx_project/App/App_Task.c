#include "App_Task.h"

/* 获取姿态数据的任务配置 */
cat_task_t data_task_handle;
#define DATA_TASK_STACK_SIZE 512 
cat_u8 data_task_stack[DATA_TASK_STACK_SIZE];
#define DATA_TASK_PRIORITY 3
void App_Task_GetData(void *pvParameters);

/* 进行PID控制的任务配置 */
cat_task_t pid_task_handle;
#define PID_TASK_STACK_SIZE 512
cat_u8 pid_task_stack[PID_TASK_STACK_SIZE];
#define PID_TASK_PRIORITY 3
void App_Task_PID(void *pvParameters);

/* 进行PID控制的任务配置 */
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

    /* 2. 启动调度器 */
    catos_start_schedule();
}

/**
 * @description: 获取姿态数据：角度、编码器值
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
        /* 获取完数据，通知pid控制任务 */
        cat_sem_post(&pid_sem);

        tick += catos_ms_to_tick(10);
        cat_task_delay_until(tick);
    }

}

/**
 * @description: 进行PID控制的任务
 * @param {void} *pvParameters
 * @return {*}
 */
void App_Task_PID(void *pvParameters)
{
    (void)pvParameters;
    while(1)
    {
        /* 等待“获取数据任务”的通知 */
        cat_sem_get(&pid_sem, 1000);
        App_Car_PID();
    }
}

/**
 * @description: OLED显示任务
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
