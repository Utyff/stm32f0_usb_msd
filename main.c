#include <stm32f0xx.h>
#include "scsi.h"

void SetClockHSI48(){
	RCC -> APB1ENR |= RCC_APB1ENR_CRSEN;
	RCC -> CR2 |= RCC_CR2_HSI48ON;
	while (!(RCC -> CR2 & RCC_CR2_HSI48RDY));
	RCC -> CFGR3 &= ~RCC_CFGR3_USBSW;
	FLASH -> ACR = FLASH_ACR_PRFTBE | FLASH_ACR_LATENCY;
	CRS -> CR |= CRS_CR_AUTOTRIMEN;
	CRS -> CR |= CRS_CR_CEN;
	RCC -> CFGR |= RCC_CFGR_SW;
}

int main(void)
{
	SetClockHSI48();
	AT45DB161_Init();
	//Инициализируем USB
	USB_Init();

    while(1)
    {
    	Enumerate(0);
    	SCSI_Execute(1);
    }
}
