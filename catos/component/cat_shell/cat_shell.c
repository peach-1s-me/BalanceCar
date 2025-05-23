/**
 * @file cat_shell.c
 * @brief 
 * @author amoigus (648137125@qq.com)
 * @version 0.1
 * @date 2021-06-05
 * 
 * @copyright Copyright (c) 2021
 * 
 * @par 修改日志：
 * Date              Version Author      Description
 * 2021-06-05 1.0    amoigus             内容
 */

#include "cat_shell.h"

#include "cat_lib.h"
#include "cat_log.h"
#include "cat_task.h"

#include "cat_assert.h"

IMPORT_SECTION(cat_shell_cmd)

#if (CAT_USE_SECTION == 0)
    cat_shell_cmd_t *cat_cmds[CAT_MAX_CMD_NUM] = {0};
    cat_i32 cmd_num = 0;
#endif //(CAT_USE_SECTION ==0)

cat_i32 cat_shell_init(cat_shell_instance_t *inst, cat_shell_config_t *cfg)
{
    cat_i32 ret = 0;
    cat_u16 i = 0;

    /* 检查参数 */
    if(cfg->buffer == CAT_NULL)
    {
        ret = -1;
    }
    else
    {
        inst->buffer.buf                        = (char *)cfg->buffer;
        inst->buffer.buf_size                   = cfg->buf_size;
        inst->buffer.current_combine_key_val    = 0;
        inst->buffer.current_combine_key_offset = 0;
        inst->buffer.length                     = 0;
        inst->buffer.arg_num                    = 0;
        
        /* 初始化历史命令缓冲区 */
        for(i=0; i<CAT_MAX_HISTORY; i++)
        {
            inst->history.historys[i]           = inst->buffer.buf + CAT_BUF_SIZE * (i + 1);
        }
        

        inst->cmd_list.cmd_base = (cat_shell_cmd_t *)(SECTION_START(cat_shell_cmd));
        inst->cmd_list.cmd_num = (cat_u16)(((cat_ubase)SECTION_END(cat_shell_cmd) - (cat_ubase)SECTION_START(cat_shell_cmd)) / sizeof(cat_shell_cmd_t));

        inst->status.is_inited = 1;
    }

    return ret;
}

void cat_shell_task_entry(void *arg)
{
    cat_u8 ch;
    cat_shell_instance_t *shell_inst = CAT_NULL;

    if(CAT_NULL != arg)
    {
        shell_inst = (cat_shell_instance_t *)arg;
    }
    else{
        CLOG_ERROR("[cat_shell] must give a shell instance to start shell task");
    }

    cat_printf("\r\nCatOS version %s build with %s\r\n", CATOS_VERSION, CATOS_BUILD_COMPILER);
    cat_printf(GREET_MSG_SMALL);
	cat_printf("cat>");
    while(1)
    {
        ch = cat_shell_port_getc();

        /* 有时候会有空字符输入，用判断避开assert */
        if(0 != ch)
        {
            cat_handle_input_char(shell_inst, ch);
        }
    }
}

void cat_handle_input_char(cat_shell_instance_t *inst, char data)
{
    CAT_ASSERT(data);

    cat_u8 wanted_offset = 0;//向左的偏移，从左到右即是从先到后，用来匹配新输入的键值
    cat_u32 current_mask = 0x00000000;//当前已经累计的键值掩码,用来匹配命令表中键值
    if(CAT_HAS_AREADY_NONE(inst->buffer.current_combine_key_val))
    {
        /* 当前未累计键值 */
        wanted_offset = 24;
        current_mask = 0x00000000;
    }
    else if(CAT_HAS_AREADY_ONE(inst->buffer.current_combine_key_val))
    {
        /* 当前累计了一个键值，希望读到第二个组合键键值 */
        wanted_offset = 16;
        current_mask = 0xff000000;
    }
    else if(CAT_HAS_AREADY_TWO(inst->buffer.current_combine_key_val))
    {
        /* 当前累计了两个键值，希望读到第三个组合键键值*/
        wanted_offset = 8;
        current_mask = 0xffff0000;
    }
    else
    {
        /* 当前累计了三个键值，希望读到第四个组合键键值，目前最多支持四个键值组合*/
        wanted_offset = 0;
        current_mask = 0xffffff00;
    }


    cat_shell_cmd_t *temp_cmd = CAT_NULL;
    CAT_FOREACH_CMD(temp_cmd)
    {
        if(temp_cmd->type == CAT_CMD_TYPE_KEY)
        {
            /* 匹配： */
            if(
                ((temp_cmd->content.key.value & current_mask) == inst->buffer.current_combine_key_val) &&
                ((temp_cmd->content.key.value & (0xff << wanted_offset)) == (cat_u32)(data << wanted_offset))
            )
            {
                /* 至此说明已经有的累计键值加上当前键值，和列表中这个按键键值前序匹配 */
                inst->buffer.current_combine_key_val |= data << wanted_offset;
                
                /* 将data清空，防止被后面识别为普通输入 */
                data = 0x0;

                /* 如果完全匹配，即判断是否键值到此为止 */
                if(
                    (wanted_offset == 0) || /* 说明当前读入的键值已经是第四个 */
                    ((temp_cmd->content.key.value & (0xff << (wanted_offset - 8))) == 0x00000000) /* 或者该命令项在当前偏移之后都为零，即没有下一个键值 */
                )
                {
                    if(CAT_NULL != temp_cmd->content.key.action)
                    {
                        temp_cmd->content.key.action(inst);
                    }
                    inst->buffer.current_combine_key_val = 0x00000000;
                }/* 完全匹配 */
            }/* 前序匹配 */
        }/* 按键类型 */ 
    }

    /* 如果是特定按键，则此时data已经被置零，若没有则说明是普通输入 */
    if(data != 0x00)
    {
        inst->buffer.current_combine_key_val = 0x00000000;
        inst->buffer.buf[inst->buffer.length++] = data;
        cat_shell_port_putc(data);
    }
}

cat_shell_cmd_t *cat_seek_cmd(cat_shell_instance_t *inst)
{
    cat_shell_cmd_t *ret = CAT_NULL;
    cat_shell_cmd_t *temp_cmd = CAT_NULL;
    CAT_FOREACH_CMD(temp_cmd)
    {
        if(temp_cmd->type == CAT_CMD_TYPE_CMD)
        {
            if(cat_strcmp(temp_cmd->content.cmd.name, (const char *)inst->buffer.buf) == 0)
            {
                ret = temp_cmd;
            }/* if(cat_strcmp(temp_cmd->content.cmd.name, inst->buffer.buf) == 0) */
        }/* if(temp_cmd->type == CAT_CMD_TYPE_CMD) */
    }/* CAT_FOREACH_CMD(temp_cmd) */

    return ret;
}

void cat_parse_args(cat_shell_instance_t *inst)
{

    cat_u16 i = 0;

    /* 初始化，清空上一次的记录 */
    inst->buffer.arg_num = 0;
    for(i=0; i<CAT_MAX_CMD_ARGS; i++)
    {
        inst->buffer.args[i] = CAT_NULL;
    }

    /* 要先跳过命令的名称 */
    i = 0;
    while(' ' != inst->buffer.buf[i])
    {
        i++;
    }

    for(; i<inst->buffer.length; i++)
    {
        /* 一个空格分隔 */
        if(
            (' ' == inst->buffer.buf[i]) && 
            (' ' != inst->buffer.buf[i+1]) &&
            ('\0' != inst->buffer.buf[i+1])
        )
        {
            inst->buffer.buf[i] = '\0';
            if(inst->buffer.arg_num < CAT_MAX_CMD_ARGS)
            {
                inst->buffer.args[inst->buffer.arg_num++] = &(inst->buffer.buf[i+1]);
            }
            else
            {
                cat_printf("[catos] over max arg num:%d !\r\n", CAT_MAX_CMD_ARGS);
            } /* if(inst->buffer.arg_num < CAT_MAX_CMD_ARGS) */
        } /* if */
    } /* for(; i<inst->buffer.length; i++) */
}

void cat_execute_cmd(cat_shell_instance_t *inst)
{
    cat_shell_cmd_t *target_cmd = CAT_NULL;

    cat_parse_args(inst);

    target_cmd = cat_seek_cmd(inst);

    if(CAT_NULL == target_cmd)
    {
        cat_printf("[catos] cmd %s not found !\r\n", inst->buffer.buf);
    }
    else
    {
        if(CAT_NULL != target_cmd->content.cmd.do_cmd)
        {
            target_cmd->content.cmd.do_cmd(inst);
        }
    }

#if 0
    CAT_FOREACH_CMD(temp_cmd)
    {
        if(temp_cmd->type == CAT_CMD_TYPE_CMD)
        {
            if(cat_strcmp(temp_cmd->content.cmd.name, inst->buffer.buf) == 0)
            {
                if(CAT_NULL != temp_cmd->content.cmd.do_cmd)
                {
                    

                    /* todo: 增加传参功能 */
                    temp_cmd->content.cmd.do_cmd(inst);
                    break;
                }/* if(CAT_NULL != temp_cmd->content.cmd.do_cmd) */
            }/* if(cat_strcmp(temp_cmd->content.cmd.name, inst->buffer.buf) == 0) */
        }/* if(temp_cmd->type == CAT_CMD_TYPE_CMD) */
    }/* CAT_FOREACH_CMD(temp_cmd) */
#endif

}


/* history 相关*/
void cat_history_save(cat_shell_instance_t *shell_inst)
{
    cat_u16 cur = shell_inst->history.current;

    /* 如果当前已经是该命令则不添加 */
    cat_u32 not_exist = cat_strcmp((char *)shell_inst->history.historys[cur], (char *)shell_inst->buffer.buf);
    if(0 == not_exist)
    {
        cur++;
        if(cur == CAT_MAX_HISTORY)
        {
            cur = 0;
        }
        shell_inst->history.current = cur;
        
        return;
    }

    cat_strcpy((char *)shell_inst->history.historys[cur], (char *)shell_inst->buffer.buf, CAT_MAX_HIS_LEN);

    /* 使用上方向键时会先cur-- */
    cur++;
    if(CAT_MAX_HISTORY == cur)
    {
        cur = 0;
    }
    shell_inst->history.current = cur;
}

void cat_history_up(cat_shell_instance_t *shell_inst)
{
    cat_u16 cur = shell_inst->history.current;
    if(0 == cur)
    {
        cur = CAT_MAX_HISTORY - 1;
    }
    else
    {
        cur--;
    }
    shell_inst->history.current = cur;

    cat_strcpy((char *)shell_inst->buffer.buf, shell_inst->history.historys[cur], CAT_MAX_HIS_LEN);
    shell_inst->buffer.length = cat_strlen(shell_inst->buffer.buf);
}

void cat_history_down(cat_shell_instance_t *shell_inst)
{
    cat_u16 cur = shell_inst->history.current;
    cur++;
    if(CAT_MAX_HISTORY == cur)
    {
        cur = 0;
    }
    shell_inst->history.current = cur;

    cat_strcpy(shell_inst->buffer.buf, shell_inst->history.historys[cur], CAT_MAX_HIS_LEN);
    shell_inst->buffer.length = cat_strlen(shell_inst->buffer.buf);
}


