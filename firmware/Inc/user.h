//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////
// C bit for interfacing with main.c

#ifdef __cplusplus
extern "C" {
#endif

//////////////////////////////////////////////////////////////////////

extern SPI_HandleTypeDef hspi1;
extern DMA_HandleTypeDef hdma_spi1_tx;
extern TIM_HandleTypeDef htim17;
extern TIM_HandleTypeDef htim1 ;

//////////////////////////////////////////////////////////////////////

void user_main();

//////////////////////////////////////////////////////////////////////
// C++ only from here

#ifdef __cplusplus
}

//////////////////////////////////////////////////////////////////////

constexpr nullptr_t null = nullptr;

using uint = unsigned int;
using uint64 = uint64_t;
using uint32 = uint32_t;
using uint16 = uint16_t;
using uint8 = uint8_t;
using byte = uint8_t;
using int64 = int64_t;
using int32 = int32_t;
using int16 = int16_t;
using int8 = int8_t;
using uintptr = uint32_t;

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

//////////////////////////////////////////////////////////////////////

inline int sgn(int x)
{
    return (x >> 31) | (x != 0);
}

//////////////////////////////////////////////////////////////////////

template <typename T, uint32 N> static constexpr inline uint32 countof(T const (&)[N])
{
    return N;
}

//////////////////////////////////////////////////////////////////////

inline int atomic_exchange(int volatile *addr, int new_value)
{
    __disable_irq();
 	int ret = *addr;
    *addr = new_value;
    __enable_irq();
    return ret;
}

#endif // __cplusplus
