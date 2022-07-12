#ifndef __BEEP_H
#define	__BEEP_H


#include "stm32f4xx.h"


/* ������������ӵ�GPIO�˿�, �û�ֻ��Ҫ�޸�����Ĵ��뼴�ɸı���Ƶķ��������� */
#define BEEP_GPIO_PORT    	GPIOD			              /* GPIO�˿� */
#define BEEP_GPIO_CLK 	    RCC_AHB1Periph_GPIOD		/* GPIO�˿�ʱ�� */
#define BEEP_GPIO_PIN		  	GPIO_Pin_8			      /* ���ӵ���������GPIO */


/* ���κ꣬��������������һ��ʹ�� */
#define BEEP(a)	if (a)	\
					GPIO_SetBits(BEEP_GPIO_PORT,BEEP_GPIO_PIN);\
					else		\
					GPIO_ResetBits(BEEP_GPIO_PORT,BEEP_GPIO_PIN)
					
					
/* �ߵ�ƽʱ���������� */
#define BEEP_ON  BEEP(1)
#define BEEP_OFF BEEP(0)



void BEEP_GPIO_Config(void);
					
#endif /* __BEEP_H */
