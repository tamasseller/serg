#ifndef _RANSMODEL_H
#define _RANSMODEL_H

#include <stdint.h>
#include <iostream>

struct RansModel 
{
	static constexpr auto presentMult = 8;
	
	uint32_t counts[256] = {0,};
	uint16_t widths[256] = {0,};
	uint16_t starts[256] = {0,};
	
public:
	inline RansModel() {
		update();
	}
	
	void update()
	{
		uint32_t sum = 0;
		
		for(int i = 0; i < 256; i++)
			sum += counts[i];
			
		sum *= presentMult;
			
		size_t downscale = 0;
		while(sum >= 0x10000 * presentMult) {
			downscale++;
			sum >>= 1;
			std::cout << downscale << std::endl;
		}
		
		sum += 256;
			
		for(int i = 0; i < 256; i++)
			widths[i] = ((presentMult * counts[i] + 1) << (16 - downscale)) / sum;
			
		starts[0] = 0;
		for(int i = 0; i < 255; i++)
			starts[i + 1] = starts[i] + widths[i];
			
	}

	struct Symstat {
		size_t width, cummulated;
		inline Symstat(size_t width, size_t cummulated): width(width), cummulated(cummulated) {}
	};
	
	void add(char c) {
		counts[(unsigned char)c]++;
	}
	
	void substract(char c) {
		counts[(unsigned char)c]--;
	}

	Symstat predict(char c) {
		return Symstat(widths[(unsigned char)c], starts[(unsigned char)c]);
	}

	Symstat identify(size_t in, char &c)
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
		counts[bottom]++;
		return Symstat(widths[bottom], starts[bottom]);
	}
};

#endif
