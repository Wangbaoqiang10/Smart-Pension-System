#include "led.h"


void LED_Init(void)
{
	 GPIO_InitTypeDef  GPIO_InitStructure;
		
	 RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOC , ENABLE);	 //使能PC端口时钟
		
	 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;//LED--PC13
	 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
	 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
	 GPIO_Init(GPIOC, &GPIO_InitStructure);					 //根据设定参数初始化GPIOC0-7
	 GPIO_SetBits(GPIOC,GPIO_Pin_13);						 //GPIOC0-7 输出高
}
