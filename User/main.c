#include "board.h"
#include "rtthread.h"
#include <string.h>                                                                                                                                                                                                                                                                                                                                                    b                                                                                                                                                                        cvv                                                                                               u
#include <stdlib.h>
#include <bsp_led.h>
#include <bsp_beep.h>

struct sensor_data
{
    int temperature;
    int humiture;
    int light;
    int smog;
    int air;
    int heartrate;
    int systolic;
    int diastolic;
} sensor_data;

unsigned char *dataPtr = NULL;
int is_begin_with(const char *str1, char *str2);

/*
******************************************************************
*                               变量
******************************************************************
*/
/* 定义线程控制块 */
static rt_thread_t data_thread = RT_NULL;
static rt_thread_t onenet_thread = RT_NULL;
static rt_thread_t nrf_thread = RT_NULL;
static rt_thread_t alarm_thread = RT_NULL;
/* 定义信号量控制块 */
rt_sem_t test_sem = RT_NULL;

/************************* 全局变量声明 ****************************/
/*
 * 当我们在写应用程序的时候，可能需要用到一些全局变量。
 */

/* 相关宏定义 */
extern u8 USART_RX_BUF[USART_REC_LEN];
u8 tmp_buf[33];

/*
*************************************************************************
*                             函数声明
*************************************************************************
*/
static void data_thread_entry(void *parameter);
static void onenet_thread_entry(void *parameter);
static void nrf_thread_entry(void *parameter);
static void alarm_thread_entry(void *parameter);

/*
*************************************************************************
*                             main 函数
*************************************************************************
*/
/**
 * @brief  主函数
 * @param  无
 * @retval 无
 */
int main(void)
{

    rt_kprintf("OneNet云平台！\n");
    ESP8266_Init(); //初始化ESP8266
    rt_kprintf("Hardware init OK\n\n");

    while (OneNet_DevLink()) //接入OneNET
        delay_ms(500);

    /* 创建一个信号量 */
    test_sem = rt_sem_create("test_sem",        /* 消息队列名字 */
                             0,                 /* 信号量初始值，默认有一个信号量 */
                             RT_IPC_FLAG_FIFO); /* 信号量模式 FIFO(0x00)*/

    if (test_sem != RT_NULL)
        rt_kprintf("信号量创建成功！\n\n");

    data_thread =                           /* 线程控制块指针 */
        rt_thread_create("usart",           /* 线程名字 */
                         data_thread_entry, /* 线程入口函数 */
                         RT_NULL,           /* 线程入口函数参数 */
                         1024,              /* 线程栈大小 */
                         2,                 /* 线程的优先级 */
                         20);               /* 线程时间片 */

    /* 启动线程，开启调度 */
    if (data_thread != RT_NULL)
    {
        rt_kprintf("Zigbee 线程 成功\n");
        rt_thread_startup(data_thread);
    }

    else
        return -1;

    onenet_thread =                           /* 线程控制块指针 */
        rt_thread_create("key",               /* 线程名字 */
                         onenet_thread_entry, /* 线程入口函数 */
                         RT_NULL,             /* 线程入口函数参数 */
                         512,                 /* 线程栈大小 */
                         2,                   /* 线程的优先级 */
                         20);                 /* 线程时间片 */

    /* 启动线程，开启调度 */
    if (onenet_thread != RT_NULL)
    {
        rt_kprintf("OneNet 线程 成功\n");
        rt_thread_startup(onenet_thread);
    }
    else
        return -1;

    nrf_thread =                           /* 线程控制块指针 */
        rt_thread_create("nRF24L01",       /* 线程名字 */
                         nrf_thread_entry, /* 线程入口函数 */
                         RT_NULL,          /* 线程入口函数参数 */
                         512,              /* 线程栈大小 */
                         2,                /* 线程的优先级 */
                         20);              /* 线程时间片 */

    /* 启动线程，开启调度 */
    if (nrf_thread != RT_NULL)
    {
        rt_kprintf("nRF24L01 线程 成功\n");
        rt_thread_startup(nrf_thread);
    }
    else
        return -1;

    alarm_thread =                           /* 线程控制块指针 */
        rt_thread_create("Alarm",            /* 线程名字 */
                         alarm_thread_entry, /* 线程入口函数 */
                         RT_NULL,            /* 线程入口函数参数 */
                         128,                /* 线程栈大小 */
                         2,                  /* 线程的优先级 */
                         20);                /* 线程时间片 */

    /* 启动线程，开启调度 */
    if (alarm_thread != RT_NULL)
    {
        rt_kprintf("Alarm 线程 成功\n");
        rt_thread_startup(alarm_thread);
    }
    else
        return -1;
}

/*
*************************************************************************
*                             线程定义
*************************************************************************
*/
static void data_thread_entry(void *parameter)
{
    rt_err_t uwRet = RT_EOK;
    int i, j;
    u8 nrf_data[3];
    char data[5];
    char sensor_name[4][6] = {"E1", "E2", "Hrt", "BPs"};

    /* 任务都是一个无限循环，不能返回 */
    while (1)
    {
        j = 0;
        uwRet = rt_sem_take(test_sem, /* 获取串口中断的信号量 */
                            0);       /* 等待时间：0 */

        if (RT_EOK == uwRet)
        {
            rt_kprintf("收到数据:%s\n", USART_RX_BUF);

            //温度，湿度，天然气 读取
            if (is_begin_with(USART_RX_BUF, sensor_name[0]))
            {

                /***********温度采集***************/
                data[0] = USART_RX_BUF[5];
                data[1] = USART_RX_BUF[6];
                sensor_data.temperature = atoi(data);
                memset(data, 0, sizeof(data));
                /**************湿度采集**************/
                data[0] = USART_RX_BUF[10];
                data[1] = USART_RX_BUF[11];
                sensor_data.humiture = atoi(data);
                memset(data, 0, sizeof(data));
                /**************天然气浓度采集**************/
                j = 0;

                for (i = 17; i < strlen(USART_RX_BUF); i++)
                {
                    data[j] = USART_RX_BUF[i];
                    j++;
                }

                j = 0;
                sensor_data.smog = atoi(data);
                memset(data, 0, sizeof(data));
            }

            //空气质量、光照强度
            if (is_begin_with(USART_RX_BUF, sensor_name[1]))
            {
                /**************空气质量采集**************/
                for (i = strlen(USART_RX_BUF) - 3; i < strlen(USART_RX_BUF) + 1; i++)
                {
                    data[j] = USART_RX_BUF[i];
                    j++;
                }

                j = 0;
                sensor_data.air = atoi(data);

                memset(data, 0, sizeof(data));

                /**************光照强度采集**************/
                for (i = 5; i < strlen(USART_RX_BUF) - 10; i++)
                {
                    data[j] = USART_RX_BUF[i];
                    j++;
                }

                j = 0;
                sensor_data.light = atoi(data);
                memset(data, 0, sizeof(data));
            }

            memset(USART_RX_BUF, 0, USART_REC_LEN); /* 清零 */
        }

        //心率数据  Hrt：88
        if (is_begin_with(tmp_buf, sensor_name[2]))
        {
            for (i = 4; i < sizeof(tmp_buf); i++)
            {
                nrf_data[j] = tmp_buf[i];
                j++;
            }

            j = 0;
            sensor_data.heartrate = atoi(nrf_data);
            memset(nrf_data, 0, sizeof(nrf_data));
        }

        //血压（舒张压、收缩压） 格式：BPs:175,76
        if (is_begin_with(tmp_buf, sensor_name[3]))
        {
            for (i = 4; i < 7; i++)
            {
                nrf_data[j] = tmp_buf[i];
                j++;
            }

            j = 0;
            sensor_data.systolic = atoi(nrf_data); //收缩压
            memset(nrf_data, 0, sizeof(nrf_data));

            for (i = 8; i < 10; i++)
            {
                nrf_data[j] = tmp_buf[i];
                j++;
            }

            j = 0;
            sensor_data.diastolic = atoi(nrf_data); //舒张压
            memset(nrf_data, 0, sizeof(nrf_data));
        }

        delay_ms(2000);
    }
}

static void onenet_thread_entry(void *parameter)
{

    while (1)
    {
        LED1_ON;
        LED2_ON;
        UsartPrintf(USART1, "\r\n OneNet_SendData \r\n");
        OneNet_SendData(); //发送数据
        ESP8266_Clear();
        LED1_OFF;
        LED2_OFF;

        dataPtr = ESP8266_GetIPD(0);

        if (dataPtr != NULL)
            OneNet_RevPro(dataPtr);

        delay_ms(5000);
    }
}

static void nrf_thread_entry(void *parameter)
{
    u8 key, mode;
    u16 t = 0;

    while (1)
    {
        mode = 0; //选择 nRF24L01  工作模式

        if (mode == 0) // RX模式
        {
            // printf("NRF24L01 RX_MODE\n");
            //   printf("Received Data\n");
            NRF24L01_RX_Mode();

            while (1)
            {
                if (NRF24L01_RxPacket(tmp_buf) == 0) //一旦接收到信息,则显示出来.
                {
                    tmp_buf[32] = 0; //加入字符串结束符
                    // rt_kprintf("nRF24L01 Received: %s \r\n", tmp_buf);
                }
                else
                    delay_us(100);

                t++;

                if (t == 10000) //大约1s钟改变一次状态
                {
                    t = 0;
                    LED0 = !LED0;
                }
            };
        }
        else // TX模式
        {
            // printf("NRF24L01 TX_Mode\n");
            NRF24L01_TX_Mode();
            mode = ' '; //从空格键开始

            while (1)
            {
                if (NRF24L01_TxPacket(tmp_buf) == TX_OK)
                {
                    //  printf("Send Data\n");

                    //     printf("Send :%s\r\n", tmp_buf);

                    key = mode;

                    for (t = 0; t < 32; t++)
                    {
                        key++;

                        if (key > ('~'))
                            key = ' ';

                        tmp_buf[t] = key;
                    }

                    mode++;

                    if (mode > '~')
                        mode = ' ';

                    tmp_buf[32] = 0; //加入结束符
                }
                else
                {
                    printf("Send Failed \n");
                };

                LED0 = !LED0;

                rt_thread_delay(3);
            };
        }
    }
}
/***********报警线程***************/
static void alarm_thread_entry(void *parameter)
{
    while (1)
    {
        if (sensor_data.temperature > 28)
            BEEP_ON;
        else if (sensor_data.humiture > 85)
            BEEP_ON;
        else if (sensor_data.smog > 580)
            BEEP_ON;
        else if (sensor_data.air > 500)
            BEEP_ON;
        else if (sensor_data.heartrate > 100)
            BEEP_ON;
        else if (sensor_data.diastolic < 70)
            BEEP_ON;
        else if (sensor_data.systolic > 150)
            BEEP_ON;
        else
            BEEP_OFF;

        delay_ms(500);

        // rt_kprintf("Beep on / off \r\n");
    }
}
/**判断str1是否以str2开头

* 如果是返回1

* 不是返回0

* 出错返回-1

* */

int is_begin_with(const char *str1, char *str2)
{

    if (str1 == NULL || str2 == NULL)

        return -1;

    int len1 = strlen(str1);

    int len2 = strlen(str2);

    if ((len1 < len2) || (len1 == 0 || len2 == 0))

        return -1;

    char *p = str2;

    int i = 0;

    while (*p != '\0')

    {

        if (*p != str1[i])

            return 0;

        p++;

        i++;
    }

    return 1;
}

/********************************END OF FILE****************************/
