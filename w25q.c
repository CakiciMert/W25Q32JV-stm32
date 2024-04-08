/*
 * w25q.c
 *
 *  Created on: Apr 8, 2024
 *      Author: senol
 */


#include "main.h"
#include "w25q.h"
#include "stm32f4xx_hal.h"

extern SPI_HandleTypeDef hspi2;

void csLow()
{
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, RESET);
}
void csHigh()
{
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, SET);
}



////////////////////////////////////////


void w25q_Receive(uint8_t *data,uint8_t len)
{
	HAL_SPI_Receive(&w25q_spi, data, len , 3000);
}
void w25q_Transmit(uint8_t *data,uint8_t len)
{
	HAL_SPI_Transmit(&w25q_spi, data, len , 2000);
}


/////////////////////////////////////////////
void w25q_Reset()
{
	uint8_t resetdata[2];
	resetdata[0]=0x66;
	resetdata[1]=0x99;
	csLow();
	w25q_Transmit(resetdata, 2);

	csHigh();
}

uint32_t w25q_ReadID()
{
	uint8_t tData=0x9F;
	uint8_t rData[3];

	csLow();
	w25q_Transmit(&tData, 1);
	w25q_Receive(rData, 3);
	csHigh();

	return (rData[0]<<16) | (rData[1] << 8) | rData[2];

}

void w25q_read(uint32_t start_page,uint8_t offset,uint32_t size,uint8_t *rData )
{
	uint8_t tdata[4];
	tdata[0]=0x03;
	uint32_t memAddr= (256*start_page) +offset;
	tdata[1]= (memAddr >> 16) & 0xff;
	tdata[2]= (memAddr >> 8) & 0xff;
	tdata[3]= (memAddr) & 0xff;
	csLow();

	w25q_Transmit(tdata, 4);
	w25q_Receive(rData, size);
	csHigh();
}
void w25q_fastread(uint32_t start_page,uint8_t offset,uint32_t size,uint8_t *rData ) /// emin değilim kullancaz mı
{
	uint8_t tdata[5];
	tdata[0]=0x03;
	uint32_t memAddr= (256*start_page) + offset;
	tdata[1]= (memAddr >> 16) & 0xff;
	tdata[2]= (memAddr >> 8) & 0xff;
	tdata[3]= (memAddr) & 0xff;
	tdata[4]= 0; // dummyyy clockk
	csLow();

	w25q_Transmit(tdata, 5);
	w25q_Receive(rData, size);
	csHigh();
}

void w25q_writeEnable()
{
	uint8_t tData= 0x06;

	csLow();
	w25q_Transmit(&tData, 1);

	csHigh();

	HAL_Delay(10);/// I am not sure about that
}

void w25q_writeDisable()
{
	uint8_t tData= 0x04;

	csLow();
	w25q_Transmit(&tData, 1);

	csHigh();

	HAL_Delay(10);/// I am not sure about that
}

void w25q_sectorErase(uint16_t sectorNum) /// 400ms time needed
{
	uint8_t tdata[5];
	uint32_t memAddr = sectorNum*4096 ;// 4096 4kb per each sector (if we wanbt delete sector 2 it means start from 4096 to 2*4096)
	////Pages can be erased in groups of 16 (4KB sector erase), groups of 128(32KB block erase), groups of 256 (64KB block erase) or the entire chip (chip erase).
	w25q_writeEnable();

	tdata[0]=0x20;
	tdata[1]= (memAddr >> 16) & 0xff;
	tdata[2]= (memAddr >> 8) & 0xff;
	tdata[3]= (memAddr) & 0xff;

	csLow();
	w25q_Transmit(tdata, 4);
	csHigh();

	w25q_writeDisable();

	while(w25q_Status()!=0)
	{
		uint8_t lmao = w25q_Status();
		HAL_Delay(100);
	}
	//HAL_Delay(500);
}
void w25q_writePage(uint32_t page,uint16_t offset,uint32_t size,uint8_t *data)
{
	uint8_t tdata[266];
	//memset(tdata,0,sizeof(tdata));
	uint32_t startPage = page;
	uint32_t endPage = startPage + ((offset + size - 1)/256) ;
	uint32_t numPage = endPage- startPage+1;

	uint16_t startSector= startPage/16 ;
	uint16_t endSector= endPage/16 ;
	uint16_t numSector= endSector-startSector+1;

	for( uint16_t i=0;i<numSector;i++)
	{
		w25q_sectorErase(startSector++);//
	}
	uint32_t dataPos=0;

	for(uint32_t i=0;i<numPage;i++)
	{
		uint32_t memAddr = (startPage*256)+offset;
		uint16_t bytesRemain =ByteToWrite(size, offset);
		uint32_t index=0;

		w25q_writeEnable();

		tdata[0]=0x02;
		tdata[1]= (memAddr >> 16) & 0xff;
		tdata[2]= (memAddr >> 8) & 0xff;
		tdata[3]= (memAddr) & 0xff;
		index=4;

		uint16_t bytestosend = bytesRemain + index;
		for(uint16_t i=0;i< bytesRemain;i++)
		{
			tdata[index++]=data[i+dataPos];
		}

		csLow();
		w25q_Transmit(tdata, bytestosend);
		csHigh();

		startPage++;
		offset=0;
		size= size-bytesRemain;
		dataPos= dataPos+bytesRemain;

		HAL_Delay(5);
		w25q_writeDisable();
	}
}
uint8_t w25q_Status()
{
	uint8_t tData=0x05;
	uint8_t rData[2];

	csLow();
	w25q_Transmit(&tData, 1);
	w25q_Receive(rData, 2);
	csHigh();

	return (rData[0] << 8) | rData[1];

}
uint16_t ByteToWrite(uint32_t size,uint16_t offset)
{
	if((size + offset) <256 )
	{
		return size;
	}
	else
	{
		return (256-offset);
	}

}
