#ifndef _RANS_DECODER_H
#define _RANS_DECODER_H

#include "RansBase.h"

class RansDecoder: RansBase
{
	uint32_t x;
	
protected:
	const char* in;
	
public:
	inline RansDecoder(const char* i): in(i) 
	{
		x = (unsigned char)*in++ << 0;
		x |= (unsigned char)*in++ << 8;
		x |= (unsigned char)*in++ << 16;
		x |= (unsigned char)*in++ << 24;
	}
	
	template<class Model>
	inline char get(Model& model) {
		char ret;
		auto low = x & mask;
		auto r = model.identify(low, ret);
		
		x = r.width * (x >> rangeScaleBits) + low - r.cummulated;
		
		while (x < normalizedLowerBound)
			x = (x << 8) | (unsigned char)*in++;
		
		return ret;
	}
	
	inline char readSideChannel() {
		return *in++;
	}
	
	inline bool check() {
		return x == normalizedLowerBound;
	}
};

#endif
