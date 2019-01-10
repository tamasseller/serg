#ifndef _MLZ_BASE_H
#define _MLZ_BASE_H

#include <stddef.h>

class MlzBase 
{
protected:
	static constexpr auto litHeadLength = 1;
	static constexpr auto minLitLength = 1;
	static constexpr auto maxLitLength = minLitLength + 127;

	static constexpr auto refHeadLength = 2;
	static constexpr auto minRefLength = refHeadLength + 1;
	static constexpr auto maxRefLength = minRefLength + 127;
	static constexpr auto minDiff = 1;
	static constexpr auto maxDiff = minDiff + 255;
public:
	static inline constexpr size_t wcCompressedLength(size_t inLength) {
		return ((inLength + maxLitLength - 1) / maxLitLength) * (maxLitLength + litHeadLength);
	}
};

#endif
