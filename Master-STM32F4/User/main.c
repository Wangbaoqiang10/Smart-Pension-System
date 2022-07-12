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
*                               ����
******************************************************************
*/
/* �����߳̿��ƿ� */
static rt_thread_t data_thread = RT_NULL;
static rt_thread_t onenet_thread = RT_NULL;
static rt_thread_t nrf_thread = RT_NULL;
static rt_thread_t alarm_thread = RT_NULL;
/* �����ź������ƿ� */
rt_sem_t test_sem = RT_NULL;

/************************* ȫ�ֱ������� ****************************/
/*
 * ��������дӦ�ó����ʱ�򣬿�����Ҫ�õ�һЩȫ�ֱ�����
 */

/* ��غ궨�� */
extern u8 USART_RX_BUF[USART_REC_LEN];
u8 tmp_buf[33];

/*
*************************************************************************
*                             ��������
*************************************************************************
*/
static void data_thread_entry(void *parameter);
static void onenet_thread_entry(void *parameter);
static void nrf_thread_entry(void *parameter);
static void alarm_thread_entry(void *parameter);

/*
*************************************************************************
*                             main ����
*************************************************************************
*/
/**
 * @brief  ������
 * @param  ��
 * @retval ��
 */
int main(void)
{

    rt_kprintf("OneNet��ƽ̨��\n");
    ESP8266_Init(); //��ʼ��ESP8266
    rt_kprintf("Hardware init OK\n\n");

    while (OneNet_DevLink()) //����OneNET
        delay_ms(500);

    /* ����һ���ź��� */
    test_sem = rt_sem_create("test_sem",        /* ��Ϣ�������� */
                             0,                 /* �ź�����ʼֵ��Ĭ����һ���ź��� */
                             RT_IPC_FLAG_FIFO); /* �ź���ģʽ FIFO(0x00)*/

    if (test_sem != RT_NULL)
        rt_kprintf("�ź��������ɹ���\n\n");

    data_thread =                           /* �߳̿��ƿ�ָ�� */
        rt_thread_create("usart",           /* �߳����� */
                         data_thread_entry, /* �߳���ں��� */
                         RT_NULL,           /* �߳���ں������� */
                         1024,              /* �߳�ջ��С */
                         2,                 /* �̵߳����ȼ� */
                         20);               /* �߳�ʱ��Ƭ */

    /* �����̣߳��������� */
    if (data_thread != RT_NULL)
    {
        rt_kprintf("Zigbee �߳� �ɹ�\n");
        rt_thread_startup(data_thread);
    }

    else
        return -1;

    onenet_thread =                           /* �߳̿��ƿ�ָ�� */
        rt_thread_create("key",               /* �߳����� */
                         onenet_thread_entry, /* �߳���ں��� */
                         RT_NULL,             /* �߳���ں������� */
                         512,                 /* �߳�ջ��С */
                         2,                   /* �̵߳����ȼ� */
                         20);                 /* �߳�ʱ��Ƭ */

    /* �����̣߳��������� */
    if (onenet_thread != RT_NULL)
    {
        rt_kprintf("OneNet �߳� �ɹ�\n");
        rt_thread_startup(onenet_thread);
    }
    else
        return -1;

    nrf_thread =                           /* �߳̿��ƿ�ָ�� */
        rt_thread_create("nRF24L01",       /* �߳����� */
                         nrf_thread_entry, /* �߳���ں��� */
                         RT_NULL,          /* �߳���ں������� */
                         512,              /* �߳�ջ��С */
                         2,                /* �̵߳����ȼ� */
                         20);              /* �߳�ʱ��Ƭ */

    /* �����̣߳��������� */
    if (nrf_thread != RT_NULL)
    {
        rt_kprintf("nRF24L01 �߳� �ɹ�\n");
        rt_thread_startup(nrf_thread);
    }
    else
        return -1;

    alarm_thread =                           /* �߳̿��ƿ�ָ�� */
        rt_thread_create("Alarm",            /* �߳����� */
                         alarm_thread_entry, /* �߳���ں��� */
                         RT_NULL,            /* �߳���ں������� */
                         128,                /* �߳�ջ��С */
                         2,                  /* �̵߳����ȼ� */
                         20);                /* �߳�ʱ��Ƭ */

    /* �����̣߳��������� */
    if (alarm_thread != RT_NULL)
    {
        rt_kprintf("Alarm �߳� �ɹ�\n");
        rt_thread_startup(alarm_thread);
    }
    else
        return -1;
}

/*
*************************************************************************
*                             �̶߳���
*************************************************************************
*/
static void data_thread_entry(void *parameter)
{
    rt_err_t uwRet = RT_EOK;
    int i, j;
    u8 nrf_data[3];
    char data[5];
    char sensor_name[4][6] = {"E1", "E2", "Hrt", "BPs"};

    /* ������һ������ѭ�������ܷ��� */
    while (1)
    {
        j = 0;
        uwRet = rt_sem_take(test_sem, /* ��ȡ�����жϵ��ź��� */
                            0);       /* �ȴ�ʱ�䣺0 */

        if (RT_EOK == uwRet)
        {
            rt_kprintf("�յ�����:%s\n", USART_RX_BUF);

            //�¶ȣ�ʪ�ȣ���Ȼ�� ��ȡ
            if (is_begin_with(USART_RX_BUF, sensor_name[0]))
            {

                /***********�¶Ȳɼ�***************/
                data[0] = USART_RX_BUF[5];
                data[1] = USART_RX_BUF[6];
                sensor_data.temperature = atoi(data);
                memset(data, 0, sizeof(data));
                /**************ʪ�Ȳɼ�**************/
                data[0] = USART_RX_BUF[10];
                data[1] = USART_RX_BUF[11];
                sensor_data.humiture = atoi(data);
                memset(data, 0, sizeof(data));
                /**************��Ȼ��Ũ�Ȳɼ�**************/
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

            //��������������ǿ��
            if (is_begin_with(USART_RX_BUF, sensor_name[1]))
            {
                /**************���������ɼ�**************/
                for (i = strlen(USART_RX_BUF) - 3; i < strlen(USART_RX_BUF) + 1; i++)
                {
                    data[j] = USART_RX_BUF[i];
                    j++;
                }

                j = 0;
                sensor_data.air = atoi(data);

                memset(data, 0, sizeof(data));

                /**************����ǿ�Ȳɼ�**************/
                for (i = 5; i < strlen(USART_RX_BUF) - 10; i++)
                {
                    data[j] = USART_RX_BUF[i];
                    j++;
                }

                j = 0;
                sensor_data.light = atoi(data);
                memset(data, 0, sizeof(data));
            }

            memset(USART_RX_BUF, 0, USART_REC_LEN); /* ���� */
        }

        //��������  Hrt��88
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

        //Ѫѹ������ѹ������ѹ�� ��ʽ��BPs:175,76
        if (is_begin_with(tmp_buf, sensor_name[3]))
        {
            for (i = 4; i < 7; i++)
            {
                nrf_data[j] = tmp_buf[i];
                j++;
            }

            j = 0;
            sensor_data.systolic = atoi(nrf_data); //����ѹ
            memset(nrf_data, 0, sizeof(nrf_data));

            for (i = 8; i < 10; i++)
            {
                nrf_data[j] = tmp_buf[i];
                j++;
            }

            j = 0;
            sensor_data.diastolic = atoi(nrf_data); //����ѹ
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
        OneNet_SendData(); //��������
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
        mode = 0; //ѡ�� nRF24L01  ����ģʽ

        if (mode == 0) // RXģʽ
        {
            // printf("NRF24L01 RX_MODE\n");
            //   printf("Received Data\n");
            NRF24L01_RX_Mode();

            while (1)
            {
                if (NRF24L01_RxPacket(tmp_buf) == 0) //һ�����յ���Ϣ,����ʾ����.
                {
                    tmp_buf[32] = 0; //�����ַ���������
                    // rt_kprintf("nRF24L01 Received: %s \r\n", tmp_buf);
                }
                else
                    delay_us(100);

                t++;

                if (t == 10000) //��Լ1s�Ӹı�һ��״̬
                {
                    t = 0;
                    LED0 = !LED0;
                }
            };
        }
        else // TXģʽ
        {
            // printf("NRF24L01 TX_Mode\n");
            NRF24L01_TX_Mode();
            mode = ' '; //�ӿո����ʼ

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

                    tmp_buf[32] = 0; //���������
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
/***********�����߳�***************/
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
/**�ж�str1�Ƿ���str2��ͷ

* ����Ƿ���1

* ���Ƿ���0

* ������-1

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
