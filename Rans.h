#ifndef _RANS_H
#define _RANS_H

#include <stdint.h>
#include <stddef.h>
#include <string.h>

class RansBase 
{
protected:
	static constexpr uint32_t rangeScaleBits = 16;
	static constexpr uint32_t normalizedLowerBound = 1u << 24;
	static constexpr uint32_t maxMult = 1u << (32 - rangeScaleBits);
	static constexpr uint32_t mask = (1u << rangeScaleBits) - 1;
};

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
