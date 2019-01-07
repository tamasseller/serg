#ifndef _MLZ_H
#define _MLZ_H

#include <algorithm>

#include <stddef.h>

class MLZBase 
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

template<class Child>
struct MLZEncoder: MLZBase
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
			
			const char* bestStart;
			size_t bestMatchLength = 0;
			
			for(const char* c = std::max(i - maxDiff, in); c < i; c++) {
				size_t matchLength = 0;
				
				for(; matchLength < maxRefLength; matchLength++) 
					if((i + matchLength >= end) || c[matchLength] != i[matchLength])
						break;

				if(bestMatchLength < matchLength) {
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

template<class Child>
struct MLZDecoder: MLZBase
{	
	inline void decompress(char* out, size_t &outLength)
	{
		outLength = 0;
		
		while(static_cast<Child*>(this)->hasMoreReadable())
		{
			size_t l;

			char h = static_cast<Child*>(this)->readHeader();
			if(h < 0) {
				l = -h + (minRefLength - 1);
				auto d = static_cast<size_t>(static_cast<unsigned char>(static_cast<Child*>(this)->readDiff())) + minDiff;
				
				for(int i = 0; i < l; i++) {
					*out = out[-d];
					out++;
				}
			} else {
				l = h + minLitLength;
				
				for(int i = 0; i < l; i++)
					*out++ = static_cast<Child*>(this)->readLiteral();
					
				static_cast<Child*>(this)->afterLiteral();
			}
			
			outLength += l;
		}
	}
};

#endif
