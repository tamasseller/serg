#ifndef _RANS_BASE_H
#define _RANS_BASE_H

#include <stdint.h>

class RansBase 
{
protected:
	static constexpr uint32_t rangeScaleBits = 16;
	static constexpr uint32_t normalizedLowerBound = 1u << 24;
	static constexpr uint32_t maxMult = 1u << (32 - rangeScaleBits);
	static constexpr uint32_t mask = (1u << rangeScaleBits) - 1;
};

#endif
