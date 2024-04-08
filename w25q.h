/*
 * w25q.h
 *
 *  Created on: Apr 8, 2024
 *      Author: senol
 */

#ifndef INC_W25Q_H_
#define INC_W25Q_H_

#define w25q_spi hspi2
#define NumBlock 256
#define Device_ID 0x6017

#endif /* INC_W25Q_H_ */

void csLow();
void csHigh();
void w25q_Transmit(uint8_t *data,uint8_t len);
void w25q_Receive(uint8_t *data,uint8_t len);
void w25q_Reset();
uint32_t w25q_ReadID();

void w25q_read(uint32_t start_page,uint8_t offset,uint32_t size,uint8_t *rData );
void w25q_fastread(uint32_t start_page,uint8_t offset,uint32_t size,uint8_t *rData );

void w25q_writeEnable();
void w25q_writeDisable();
void w25q_sectorErase(uint16_t sectorNum);
uint16_t ByteToWrite(uint32_t size,uint16_t offset);
void w25q_writePage(uint32_t page,uint16_t offset,uint32_t size,uint8_t *data);
uint8_t w25q_Status();
