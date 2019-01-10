#ifndef _MLZ_DECODER_H
#define _MLZ_DECODER_H

#include "MlzBase.h"

template<class Child>
struct MlzDecoder: MlzBase
{	
protected:
	inline size_t decompressOne(char* &out)
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
		
		return l;
	}
public:
	inline void decompress(char* out, size_t outLength)
	{
		while(outLength > 0)
			outLength -= decompressOne(out);
	}
};

#endif
