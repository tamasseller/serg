#ifndef _RANS_ENCODER_H
#define _RANS_ENCODER_H

#include <stddef.h>
#include <string.h>

#include "RansBase.h"

class RansEncoder: RansBase
{
	uint32_t x = normalizedLowerBound;
	char *o, *const out, *const oend;
	
public:
	inline RansEncoder(char* out, size_t outLength): 
		out(out), 
		o(out + outLength - 1), 
		oend(out + outLength) {}

	template<class Model>
	inline void put(Model &model, char data)
	{
		auto r = model.predict(data);

		for(; x >= maxMult * r.width; x >>= 8)
			*o-- = x;

		x = ((x / r.width) << rangeScaleBits) + (x % r.width) + r.cummulated;
	}
	
	inline void writeSideChannel(char data) {
		*o-- = data;
	}
	
	inline size_t flush()
	{
		*o-- = x >> 24;
		*o-- = x >> 16;
		*o-- = x >> 8;
		*o = x >> 0;

		auto ret = oend - o;
		memmove(out, o, ret);
		return ret;
	}
};

#endif
