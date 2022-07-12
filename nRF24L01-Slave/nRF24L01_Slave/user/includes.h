#ifndef _includes_H
#define _includes_H

//#include "includes.h"


#include "stm32f10x.h"
#include "sys.h"
#include "delay.h"
#include "usart1.h"
#include "wdg.h"
#include "led.h"
#include "pit.h"

#include "spi.h"
#include "24l01.h"
//公用函数声明
float map(float input,float input_min,float input_max,float output_min,float output_max);

#endif 
