#include <stm32f0xx.h>

uint8_t SPI_SendByte(uint8_t byte);
void AT45DB161_Init();
void AT45DB161_Read_ID(uint8_t *MID, uint8_t *DID);
uint8_t AT45DB161_Read_Status();
void AT45DB161_Read_Data(uint16_t page, uint16_t addr, uint32_t length, uint8_t *out);
void AT45DB161_PageProgram(uint16_t page, uint8_t *data, uint16_t length);
