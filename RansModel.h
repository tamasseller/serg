#ifndef _RANSMODEL_H
#define _RANSMODEL_H

#include <stdint.h>
#include <iostream>

struct RansModel 
{
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

		size_t downscale = 0;
		while(sum >= 0x10000) {
			downscale++;
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

		uint32_t cumm = 0;
		for(int i = 0; i < 255; i++) {
			starts[i] = cumm;
			cumm += widths[i];
		}
		
		uint32_t min = -1u, max = 0;	
		for(int i = 0; i < 255; i++) {
			if(min > widths[i])
				min = widths[i];
			if(max < widths[i])
				max = widths[i];
		}

//		std::cout << std::hex << min << " " << std::hex << max << std::endl;
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
