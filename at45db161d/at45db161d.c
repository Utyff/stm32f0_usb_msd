#include "at45db161d.h"

#define SPI_DR8		*(uint8_t *)0x4001300C
#define CS_HIGH		GPIOB -> BSRR = GPIO_BSRR_BS_1
#define CS_LOW		GPIOB -> BSRR = GPIO_BSRR_BR_1

uint8_t SPI_SendByte(uint8_t byte){
	while(!(SPI1 -> SR & SPI_SR_TXE));
	SPI_DR8 = byte;
	while(!(SPI1 -> SR & SPI_SR_RXNE));
	return SPI_DR8;
}

void AT45DB161_Init(){
	RCC -> APB2ENR |= RCC_APB2ENR_SPI1EN;
	RCC -> AHBENR |= RCC_AHBENR_GPIOAEN | RCC_AHBENR_GPIOBEN;
	GPIOA -> MODER |= GPIO_MODER_MODER5_1 | GPIO_MODER_MODER6_1 | GPIO_MODER_MODER7_1;
	GPIOA -> OSPEEDR |= GPIO_OSPEEDER_OSPEEDR5 | GPIO_OSPEEDER_OSPEEDR6 | GPIO_OSPEEDER_OSPEEDR7;
	GPIOB -> MODER |= GPIO_MODER_MODER1_0;
	GPIOB -> OSPEEDR |= GPIO_OSPEEDER_OSPEEDR1;
	CS_HIGH;
	SPI1 -> CR1 = SPI_CR1_SSM | SPI_CR1_SSI | SPI_CR1_MSTR;
	SPI1 -> CR2 = SPI_CR2_DS_0 | SPI_CR2_DS_1 | SPI_CR2_DS_2 | SPI_CR2_FRXTH;
	SPI1 -> CR1 |= SPI_CR1_SPE;
}

void AT45DB161_Read_ID(uint8_t *MID, uint8_t *DID){
	CS_LOW;
	SPI_SendByte(0x9F);
	*MID = SPI_SendByte(0x00);
	*DID = SPI_SendByte(0x00);
	SPI_SendByte(0x00);
	SPI_SendByte(0x00);
	SPI_SendByte(0x00);
	SPI_SendByte(0x00);
	while(SPI1 -> SR & SPI_SR_BSY);
	CS_HIGH;
}

uint8_t AT45DB161_Read_Status(){
	uint8_t temp;
	CS_LOW;
	SPI_SendByte(0xD7);
	temp = SPI_SendByte(0x00);
	while(SPI1 -> SR & SPI_SR_BSY);
	CS_HIGH;
	return temp;
}


void AT45DB161_Read_Data(uint16_t page, uint16_t addr, uint32_t length, uint8_t *out){
	uint32_t i;
	uint8_t temp;
	do {
		temp = AT45DB161_Read_Status();
	} while (!(temp & 0x80));
	if (temp & 0x01){	//512
		i = ((page << 9) | (addr & 0x1FF));
	} else {			//528
		i = ((page << 10) | (addr & 0x3FF));
	}
	CS_LOW;
	SPI_SendByte(0x0B);
	SPI_SendByte(i >> 16);
	SPI_SendByte(i >> 8);
	SPI_SendByte(i);
	SPI_SendByte(0x00);
	for (i = 0; i < length; i++){
		out[i] = SPI_SendByte(0xFF);
	}
	while(SPI1 -> SR & SPI_SR_BSY);
	CS_HIGH;
}

void AT45DB161_PageProgram(uint16_t page, uint8_t *data, uint16_t length){
	uint16_t i;
	uint8_t temp;
	temp = AT45DB161_Read_Status();
	CS_LOW;
	SPI_SendByte(0x84);
	SPI_SendByte(0x00);
	SPI_SendByte(0x00);
	SPI_SendByte(0x00);
	for (i = 0; i < length; i++){
		SPI_SendByte(data[i]);
	}
	while(SPI1 -> SR & SPI_SR_BSY);
	CS_HIGH;
	CS_LOW;
	SPI_SendByte(0x83);
	if (temp & 0x01){	//512
		SPI_SendByte((uint8_t)(page >> 7));
		SPI_SendByte((uint8_t)((page & 0x7F) << 1));
		SPI_SendByte(0x00);
	} else {			//528
		SPI_SendByte((uint8_t)(page >> 6));
		SPI_SendByte((uint8_t)((page & 0x3F) << 2));
		SPI_SendByte(0x00);
	}
	while(SPI1 -> SR & SPI_SR_BSY);
	CS_HIGH;
	while (!(AT45DB161_Read_Status() & 0x80));
}


