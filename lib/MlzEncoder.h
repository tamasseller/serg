#ifndef _MLZ_ENCODER_H
#define _MLZ_ENCODER_H

#include <algorithm>

#include "MlzBase.h"

template<class Child>
struct MlzEncoder: MlzBase
{	
	inline size_t compress(const char* const in, size_t length)
	{
		const char* const end = in + length;
		const char* i = in, *last = in;
		
		while(i < end) {
			auto diff = i - last;
			
			if(diff >= maxLitLength) {
				static_cast<Child*>(this)->writeLiteral(last, diff);
				last = i;
			}
			
			diff = i - last;
			
			const char* bestStart;
			size_t bestMatchLength = 0;
			
			for(const char* c = std::max(i - maxDiff, in); c < i; c++) {
				size_t matchLength = 0;
				
				for(; matchLength < maxRefLength; matchLength++) 
					if((i + matchLength >= end) || c[matchLength] != i[matchLength])
						break;

				if(bestMatchLength <= matchLength) {
					bestMatchLength = matchLength;
					bestStart = c;
				}
			}
			
			if(bestMatchLength >= minRefLength) {
				if(diff >= minLitLength)
			 		static_cast<Child*>(this)->writeLiteral(last, diff);
				static_cast<Child*>(this)->writeRef(i - bestStart, bestMatchLength);
				i += bestMatchLength;
				last = i;
			} else
				i++;
		}
		
		if(end != last)
			static_cast<Child*>(this)->writeLiteral(last, i - last);
		
		return static_cast<Child*>(this)->getWriteOffset();
	}
};

#endif
