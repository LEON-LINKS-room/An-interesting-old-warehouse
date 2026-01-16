/*******************************************************************************
MIT License

Copyright (c) 2022 LEON-LINKS-room

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*******************************************************************************/
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/uart.h"
#include "cli_lite.h"

void caclu_add(void);
void caclu_sub(void);
void caclu_mul(void);
void caclu_div(void);

/* 命令列表 新的命令在此注册 */
_cmd_table cmd_table[]={
    {(void *)caclu_add,"add","add [parm1] [parm2]"},
    {(void *)caclu_sub,"sub","sub [parm1] [parm2]"},
    {(void *)caclu_mul,"mul","mul [parm1] [parm2]"},
    {(void *)caclu_div,"div","div [parm1] [parm2]"},
};

int cmdnum = sizeof(cmd_table)/sizeof(_cmd_table);
char token[CMD_PARMNUM][CMD_LONGTH]={0};
static uint16_t rx_index = 0;
static uint16_t cursor_pos = 0;

static char cmd_history[CMD_HISTORY_NUM][CMD_MAX_LEN];
static int history_count = 0;
static int history_index = -1;

uint8_t rx_buffer[USART_REC_LEN]={0};
static esc_state_t esc_state = ESC_IDLE;
bool cmd_deal_ok = 1;

/* 串口发送打印 */
void cli_printf(const char *fmt, ...)
{
    char buf[128] = {0};

    va_list args;
    va_start(args, fmt);
    int len = vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    if (len < 0) {
        return;
    }

    if (len > sizeof(buf)) {
        len = sizeof(buf);
    }

    uart_write_bytes(1, buf, len);
}

/* 串口发送回显 */
void uart_echo(uint8_t *data,uint16_t len){
    uart_write_bytes(1,data,len);
}

/* 判断前缀 */
static bool str_start_with(const char *str, const char *prefix){
    while (*prefix){
        if (*str++ != *prefix++)
            return false;
    }
    return true;
}

/* 保存历史命令 */
static void history_save(const char *cmd){
    if (cmd[0] == '\0') return;

    int idx = history_count % CMD_HISTORY_NUM;
    strncpy(cmd_history[idx], cmd, CMD_MAX_LEN - 1);
    cmd_history[idx][CMD_MAX_LEN - 1] = '\0';

    history_count++;
    history_index = history_count;
}

/* 清除当前行 */
static void clear_line(void){
    while (cursor_pos > 0){
        uint8_t bs_seq[3] = {'\b', ' ', '\b'};
        uart_echo(bs_seq, 3);
        cursor_pos--;
    }
}

/* 方向键←处理 */
static void cursor_left(void){
    if (cursor_pos > 0){
        uart_echo((uint8_t *)"\b", 1);
        cursor_pos--;
    }
}

/* 方向键→处理 */
static void cursor_right(void){
    if (cursor_pos < rx_index){
        uart_echo(&rx_buffer[cursor_pos], 1);
        cursor_pos++;
    }
}

/* 方向键↑处理 */
static void cmd_history_up(void){
    if (history_count == 0) return;
    int oldest = history_count - CMD_HISTORY_NUM;
    if (oldest < 0) oldest = 0;

    if (history_index <= oldest)
        return;

    history_index--;

    while (cursor_pos < rx_index){
        cursor_right();
    }
        
    clear_line();

    strcpy((char *)rx_buffer,
           cmd_history[history_index % CMD_HISTORY_NUM]);

    rx_index = strlen((char *)rx_buffer);
    cursor_pos = rx_index;
    uart_echo(rx_buffer, rx_index);
}

/* 方向键↓处理 */
static void cmd_history_down(void){
    if (history_count == 0) return;

    if (history_index >= history_count)
        return;
    
    history_index++;

    if (history_index == history_count)
    {
        while (cursor_pos < rx_index)
            cursor_right();

        clear_line();
        rx_index = 0;
        cursor_pos = 0;
        rx_buffer[0] = '\0';
        return;
    }

    while (cursor_pos < rx_index){
        cursor_right();
    }

    clear_line();
    strcpy((char *)rx_buffer,
           cmd_history[history_index % CMD_HISTORY_NUM]);

    rx_index = strlen((char *)rx_buffer);
    cursor_pos = rx_index;
    uart_echo(rx_buffer, rx_index);
}

/* 串口接收回调 */
void cli_deal(uint8_t rx_data){

    if (!cmd_deal_ok) goto rx_exit;

    if (esc_state != ESC_IDLE){
        if (esc_state == ESC_START){
            esc_state = (rx_data == '[') ? ESC_BRACKET : ESC_IDLE;
            goto rx_exit;
        }
        else if (esc_state == ESC_BRACKET){
            esc_state = ESC_IDLE;
            switch (rx_data){
                case 'A': cmd_history_up();   break;
                case 'B': cmd_history_down(); break;
                case 'C': cursor_right();     break;
                case 'D': cursor_left();      break;
                default: break;
            }
            goto rx_exit;
        }
    }

    if (rx_data == 0x1B){
        esc_state = ESC_START;
        goto rx_exit;
    }

    if (rx_data == CMD_CR || rx_data == CMD_LF){
        rx_buffer[rx_index] = '\0';
        history_save((char *)rx_buffer);

        rx_index = 0;
        cursor_pos = 0;
        esc_state = ESC_IDLE;
        cmd_deal_ok = 0;
        goto rx_exit;
    }

    if (rx_data == CMD_BS){
        if (cursor_pos > 0){
            memmove(&rx_buffer[cursor_pos - 1],
                    &rx_buffer[cursor_pos],
                    rx_index - cursor_pos);

            rx_index--;
            cursor_pos--;

            uart_echo((uint8_t *)"\b", 1);
            uart_echo(&rx_buffer[cursor_pos],
                    rx_index - cursor_pos);
            uart_echo((uint8_t *)" ", 1);

            for (int i = 0; i <= rx_index - cursor_pos; i++)
                uart_echo((uint8_t *)"\b", 1);
        }
        goto rx_exit;
    }

    if (rx_data == CMD_HT){
        uint8_t match = 0;
        const char *last = NULL;

        rx_buffer[rx_index] = '\0';

        for (int i = 0; i < cmdnum; i++){
            if (str_start_with(cmd_table[i].name,
                            (char *)rx_buffer)){
                match++;
                last = cmd_table[i].name;
            }
        }

        if (match == 1 && last){
            const char *p = last + rx_index;
            while (*p && rx_index < USART_REC_LEN - 1){
                rx_buffer[rx_index++] = *p;
                uart_echo((uint8_t *)p, 1);
                p++;
            }
            cursor_pos = rx_index;
        }
        else if (match > 1){
            const char nl[] = "\r\n";
            uart_echo((uint8_t *)nl, 2);
            for (int i = 0; i < cmdnum; i++){
                if (str_start_with(cmd_table[i].name,
                                (char *)rx_buffer)){
                    uart_echo((uint8_t *)cmd_table[i].name,
                            strlen(cmd_table[i].name));
                    uart_echo((uint8_t *)nl, 2);
                }
            }
            const char prompt[] = "[LEON]@LINKS:";
            uart_echo((uint8_t *)prompt, strlen(prompt));
            uart_echo(rx_buffer, rx_index);
        }
        goto rx_exit;
    }

    if (rx_data >= 0x20 && rx_data <= 0x7E){
        if (rx_index < USART_REC_LEN - 1){
            memmove(&rx_buffer[cursor_pos + 1],
                    &rx_buffer[cursor_pos],
                    rx_index - cursor_pos);

            rx_buffer[cursor_pos++] = rx_data;
            rx_index++;

            uart_echo(&rx_buffer[cursor_pos - 1],
                    rx_index - cursor_pos + 1);

            for (int i = 0; i < rx_index - cursor_pos; i++)
                uart_echo((uint8_t *)"\b", 1);
        }
    }

rx_exit:
    process_cmd();
}

/* 执行命令 */
void execute_cmd(void){
    int cmd_is_find = 0;
    if(strlen(token[0])!=0){
        if(!strcmp(token[0],"cmd")){
			cli_printf("-------------------- Cmd Table --------------------\r\n");
            for(int i=0;i<cmdnum;i++){
                cli_printf("cmd:%s    eg:%s\r\n",cmd_table[i].name,cmd_table[i].example);
            }
            cli_printf("---------------------------------------------------\r\n");
        }
        else{
            for(int j=0;j<cmdnum;j++){
                if(!strcmp(token[0],cmd_table[j].name)){
                    cmd_table[j].func();
                    cmd_is_find++;
                }
            }
            if(cmd_is_find==0){
                cli_printf("Cmd Error!\r\n");
            }
        }
    }

	memset(token,0,CMD_PARMNUM*CMD_LONGTH);
}

/* 命令处理函数 */
void process_cmd(void){
    char tmp_data='\0';
    int buf_count=0;
    int parm_count=0;
    int str_count=0;

    if(cmd_deal_ok==0){
		int rx_len = strlen((const char*)rx_buffer);
        for(int i=0;i<rx_len;i++){
            tmp_data = rx_buffer[buf_count];
            if((tmp_data!='\r')||(tmp_data!='\n')||(tmp_data!='\0')){
                if(tmp_data!=' '){
                    token[parm_count][str_count] = tmp_data;
                    if(str_count<CMD_LONGTH-1){
                        str_count++;
                    }
                }
                else{
                    token[parm_count][str_count] = '\0';
                    if(parm_count<CMD_PARMNUM-1){
                        parm_count++;
                    }
                    str_count = 0;
                }
            }
            if(buf_count<USART_REC_LEN-1){
                buf_count++;
            }
        }
		cli_printf("\r\n");
        execute_cmd();
        memset(rx_buffer,0,USART_REC_LEN);
        cmd_deal_ok = 1;
        cli_printf("[LEON]@LINKS:");
    }
}

/* 命令示例 */
void caclu_add(void){
    int parm1=0,parm2=0;

    if(strlen(token[1])==0){
        cli_printf("cmd parm invalid!\r\n");
        return;
    }
    if(strlen(token[2])==0){
        cli_printf("cmd parm invalid!\r\n");
        return;
    }

    parm1 = atoi(token[1]);
    parm2 = atoi(token[2]);
    cli_printf("add result %d \r\n",parm1+parm2);
}

void caclu_sub(void){
    int parm1=0,parm2=0;

    if(strlen(token[1])==0){
        cli_printf("cmd parm invalid!\r\n");
        return;
    }
    if(strlen(token[2])==0){
        cli_printf("cmd parm invalid!\r\n");
        return;
    }

    parm1 = atoi(token[1]);
    parm2 = atoi(token[2]);
    cli_printf("sub result %d \r\n",parm1-parm2);
}

void caclu_mul(void){
    int parm1=0,parm2=0;

    if(strlen(token[1])==0){
        cli_printf("cmd parm invalid!\r\n");
        return;
    }
    if(strlen(token[2])==0){
        cli_printf("cmd parm invalid!\r\n");
        return;
    }

    parm1 = atoi(token[1]);
    parm2 = atoi(token[2]);
    cli_printf("mul result %d \r\n",parm1*parm2);
}

void caclu_div(void){
    int parm1=0,parm2=0;

    if(strlen(token[1])==0){
        cli_printf("cmd parm invalid!\r\n");
        return;
    }
    if(strlen(token[2])==0){
        cli_printf("cmd parm invalid!\r\n");
        return;
    }

    parm1 = atoi(token[1]);
    parm2 = atoi(token[2]);
    cli_printf("div result %d \r\n",parm1/parm2);
}
