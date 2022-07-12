#ifndef __pit_H
#define __pit_H
#include "includes.h"


extern u32 haomiao_xitong;

//通用定时器3中断初始化
//这里时钟选择为APB1的2倍，而APB1为36M
//arr：自动重装值。
//psc：时钟预分频数
//Tclk=72000000秒  72等于微秒
//这里使用的是定时器6!
void TIM4_Init(u16 arr,u16 psc);//Tout（溢出时间）=（ARR+1)(PSC+1)/Tclk 
 
#endif
