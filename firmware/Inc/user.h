#pragma once

#define null 0

typedef unsigned int uint;

typedef uint64_t uint64;
typedef uint32_t uint32;
typedef uint16_t uint16;
typedef uint8_t  uint8;
typedef uint8_t  byte;

typedef int64_t int64;
typedef int32_t int32;
typedef int16_t int16;
typedef int8_t  int8;

typedef int bool;

#define true 1
#define false 0

extern SPI_HandleTypeDef hspi1;
extern DMA_HandleTypeDef hdma_spi1_tx;
extern TIM_HandleTypeDef htim17;

void begin();
void loop();

//////////////////////////////////////////////////////////////////////

inline int abs(int x)
{
    return (x < 0) ? -x : x;
}

//////////////////////////////////////////////////////////////////////

inline int min(int x, int y)
{
    return (x < y) ? x : y;
}

//////////////////////////////////////////////////////////////////////

inline int max(int x, int y)
{
    return (x > y) ? x : y;
}
