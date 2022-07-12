#include "includes.h"
#include "string.h"
#include "led.h"

/***********����������***************/
struct sensor_data
{
    int Heartrate; //����
    int HBP;       //Ѫѹ��ѹ
    int LBP;       //Ѫѹ��ѹ
} sensor_data;

const u8 ReadCmd[6] = {0XFD, 0, 0, 0, 0, 0};
u8 hX = 0, lX = 0, heat = 0;

int main()
{
    int i;
    u8 tmp_buf[32] = {"Sunying love Baoqiang"};
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //����NVIC�жϷ���2:2λ��ռ���ȼ���2λ��Ӧ���ȼ�
    delay_init();                                   //��ʱ������ʼ��
    TIM4_Init(9, 7199);                             // Tout�����ʱ�䣩=��ARR+1)(PSC+1)/Tclk =10*7200/72000000s=1ms

    USART1_Init(115200, 0); //���ڳ�ʼ��Ϊ115200

    LED_GPIO_Config();

    //   printf("���ڳ�ʼ���ɹ�\r\n");

    NRF24L01_Init(); //��ʼ��NRF24L01

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
            /* �ȴ��������ݼĴ���Ϊ�� */
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
                //Ѫѹ����
                memset(tmp_buf, 0, 50);
                sprintf(tmp_buf, "BPs:%d,%d", sensor_data.HBP, sensor_data.LBP);

                while (NRF24L01_TxPacket(tmp_buf) != TX_OK)
                {
                    LED1_ON;
                }

                //���ʲ���
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
