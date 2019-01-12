#ifndef _RANSMODEL_H
#define _RANSMODEL_H

#include <stdint.h>
#include <iostream>

struct RansModelBase
{
	uint16_t widths[256] = {0,};
	uint16_t starts[256] = {0,};
	
	inline void updateStarts()
	{
		uint32_t cumm = 0;
		for(int i = 0; i < 256; i++) {
			starts[i] = cumm;
			cumm += widths[i];
		}
	}
	
	struct Symstat {
		size_t width, cummulated;
		inline Symstat(size_t width, size_t cummulated): width(width), cummulated(cummulated) {}
	};
	
	inline Symstat predict(char c) {
		return Symstat(widths[(unsigned char)c], starts[(unsigned char)c]);
	}

	inline Symstat identify(size_t in, char &c)
	{
		size_t bottom=0, top=255;
		
		while(bottom != top)
		{
			size_t mid = (bottom + top + 1) / 2;
			
			if(in < starts[mid])
				top = mid-1;
			else
				bottom = mid;
		}

		c = bottom;
		return Symstat(widths[bottom], starts[bottom]);
	}
};

struct RansModel: RansModelBase
{
	uint32_t counts[256] = {0,};
	
public:
	inline void add(char c) {
		counts[(unsigned char)c]++;
	}
	
	inline void substract(char c) {
		counts[(unsigned char)c]--;
	}

	inline RansModel() {
		update();
	}
	
	inline Symstat identify(size_t in, char &c)
	{
		auto ret = RansModelBase::identify(in, c);
		counts[(unsigned char)c]++;
		return ret;
	}
	
	inline void update()
	{
		uint32_t sum = 0, max = 0;
		for(int i = 0; i < 256; i++) {
			sum += counts[i];
			if(max < counts[i])
				max = counts[i];
		}			

		size_t downscale = 0;
		while(max >= 0x10000) {
			downscale++;
			max >>= 1;
			sum -= sum >> 1;
		}

		if(sum) {
			const uint32_t factor = (1 << (16 - downscale)) - 256;

			for(int i = 0; i < 256; i++)
				widths[i] = counts[i] * factor / sum + 1;
		} else {
			for(int i = 0; i < 256; i++)
				widths[i] = 256;
		}

		updateStarts();
	}
};

#endif
