#include "includes.h"
#include "string.h"
#include "led.h"

/***********传感器数据***************/
struct sensor_data
{
    int Heartrate; //心率
    int HBP;       //血压高压
    int LBP;       //血压低压
} sensor_data;

const u8 ReadCmd[6] = {0XFD, 0, 0, 0, 0, 0};
u8 hX = 0, lX = 0, heat = 0;

int main()
{
    int i;
    u8 tmp_buf[32] = {"Sunying love Baoqiang"};
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //设置NVIC中断分组2:2位抢占优先级，2位响应优先级
    delay_init();                                   //延时函数初始化
    TIM4_Init(9, 7199);                             // Tout（溢出时间）=（ARR+1)(PSC+1)/Tclk =10*7200/72000000s=1ms

    USART1_Init(115200, 0); //串口初始化为115200

    LED_GPIO_Config();

    //   printf("串口初始化成功\r\n");

    NRF24L01_Init(); //初始化NRF24L01

    while (NRF24L01_Check())
    {
        //        printf("NRF24L01 Error\r\n");
        delay_ms(1000);
    }

    //   printf("NRF24L01 OK\r\n");

    delay_ms(1000);

    NRF24L01_TX_Mode();

    delay_ms(2000);

    while (1)
    {

        for (i = 0; i < 6; i++)
        {
            USART_SendData(USART1, ReadCmd[i]);
            /* 等待发送数据寄存器为空 */
            while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET)
                ;
        }

        delay_ms(250); // 250

        if (USART1_RX_STA & 0X8000)
        {
            if (USART1_RX_BUF[0] == 0XFD)
            {
                sensor_data.HBP = USART1_RX_BUF[1];
                sensor_data.LBP = USART1_RX_BUF[2];
                sensor_data.Heartrate = USART1_RX_BUF[3];
                //血压测量
                memset(tmp_buf, 0, 50);
                sprintf(tmp_buf, "BPs:%d,%d", sensor_data.HBP, sensor_data.LBP);

                while (NRF24L01_TxPacket(tmp_buf) != TX_OK)
                {
                    LED1_ON;
                }

                //心率测量
                memset(tmp_buf, 0, 50);
                sprintf(tmp_buf, "Hrt:%d", sensor_data.Heartrate);

                while (NRF24L01_TxPacket(tmp_buf) != TX_OK)
                {
                    printf("Send Failed\r\n");
                }
            }

            USART1_RX_STA = 0;
        }
        delay_ms(10);
    }
}
