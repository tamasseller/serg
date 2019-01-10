#ifndef _SERG_H
#define _SERG_H

#include "MlzEncoder.h"
#include "MlzDecoder.h"
#include "RansEncoder.h"
#include "RansDecoder.h"
#include "RansModel.h"

#include <vector>
#include <math.h>

struct StaticModel: RansModel
{
	StaticModel()
	{
		for(int i=0; i<256; i++) {
			char c = i;
			int sq = (c * c) >> 10;
			if(sq <= 15)
				counts[i] = 1 << (15 - sq);
		}
			
		update();
	}
};

class SergEncoder: MlzEncoder<SergEncoder>, RansEncoder
{
	friend class MlzEncoder<SergEncoder>;
	
	struct BlockHandler {
		virtual void feed(RansModel& litModel, RansModel& diffModel) = 0;
		virtual void process(RansEncoder& encoder, RansModel& litModel, RansModel& diffModel, RansModel& headModel) = 0;
	};

	struct LiteralHandler: BlockHandler {
		const char head, *data;
		size_t length;
		
		inline LiteralHandler(char head, const char *data, size_t length): head(head), data(data), length(length) {}

		inline virtual void feed(RansModel& litModel, RansModel& diffModel)
		{
			for(const char* i = data; i < data + length; i++)
				litModel.add(*i);
		}

		inline virtual void process(RansEncoder& encoder, RansModel& litModel, RansModel& diffModel, RansModel& headModel)
		{
			for(const char* i = data + length - 1; i >= data; i--)
				litModel.substract(*i);
		
			litModel.update();
	
			for(const char* i = data + length - 1; i >= data; i--)
				encoder.put(litModel, *i);	
				
			encoder.put(headModel, head);
		}
	};

	struct RefHandler: BlockHandler {
		char head, diff;
		
		inline RefHandler(char head, char diff): head(head), diff(diff) {}

		inline virtual void feed(RansModel& litModel, RansModel& diffModel) {
			diffModel.add(diff);
		}
		
		inline virtual void process(RansEncoder& encoder, RansModel& litModel, RansModel& diffModel, RansModel& headModel)
		{
			diffModel.substract(diff);		
			diffModel.update();
			encoder.put(diffModel, diff);
			encoder.put(headModel, head);
		}
	};
	
	std::vector<BlockHandler*> blocks;

	inline void writeLiteral(const char* in, size_t length) {
		blocks.push_back(new LiteralHandler((char)(length - minLitLength), in, length));
	}

	inline void writeRef(size_t diff, size_t length) {
		blocks.push_back(new RefHandler((char) -(length - (minRefLength - 1)), diff - minDiff));
	}
	
	inline size_t getWriteOffset() {
		return 0;
	}
	
	static inline int calcDev(uint32_t *hist) 
	{
		uint64_t a = 0, sum = 0;
		
		for(int i=0; i<256; i++) {
			sum += hist[i];
			a += (int)hist[i] * (char)i * (char)i;
		}
		
		return sqrtf(a / sum);
	}

public:
	inline SergEncoder(char* start, size_t length): RansEncoder(start, length) {}
	
	inline size_t compress(const char* const in, size_t length)
	{
		MlzEncoder<SergEncoder>::compress(in, length);
		RansModel litModel, diffModel;
		StaticModel headModel;
		
		for(auto &x: blocks)
			x->feed(litModel, diffModel);

		for(auto r = blocks.rbegin(); r != blocks.rend(); r++)
			(*r)->process(*this, litModel, diffModel, headModel);
		
		return RansEncoder::flush();
	}
};

class SergDecoder: public MlzDecoder<SergDecoder>, RansDecoder
{
	friend class MlzDecoder<SergDecoder>;
		
	RansModel litModel, diffModel;
	StaticModel headModel;
	
	inline char readHeader() {
		return this->get(headModel);
	}

	inline char readDiff() {
		auto ret = this->get(diffModel);
		diffModel.update();
		return ret;
	}
	
	inline char readLiteral() {	
		return this->get(litModel);
	}
	
	inline void afterLiteral() {
		litModel.update();
	}

public:
	inline SergDecoder(const char* in): RansDecoder(in) {}	
};

#endif
