#include "sys.h"



//THUMBָ�֧�ֻ������
//�������·���ʵ��ִ�л��ָ��WFI  
__asm void WFI_SET(void)
{
	//__ASM volatile("wfi");
	WFI;
		  
}
//�ر������ж�
__asm void INTX_DISABLE(void)
{		  
	//__ASM volatile("cpsid i");
	CPSID I;
}
//���������ж�
__asm void INTX_ENABLE(void)
{
	//__ASM volatile("cpsie i");
	CPSIE I;		  
}
//����ջ����ַ
//addr:ջ����ַ
__asm void MSR_MSP(u32 addr) 
{
    MSR MSP, r0 			//set Main Stack value
    BX r14
}
