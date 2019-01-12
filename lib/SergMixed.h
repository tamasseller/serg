#ifndef _SERG_Mixed_H
#define _SERG_Mixed_H

#include "MlzEncoder.h"
#include "MlzDecoder.h"
#include "RansEncoder.h"
#include "RansDecoder.h"
#include "RansModel.h"

#include <vector>
#include <math.h>

class SergMixedEncoder: MlzEncoder<SergMixedEncoder>
{
	friend class MlzEncoder<SergMixedEncoder>;
	
	struct BlockHandler {
		virtual void feed(RansModel& litModel, RansModel& diffModel, RansModel& headModel) = 0;
		virtual void process(RansEncoder& encoder, RansModel& litModel, RansModel& diffModel, RansModel& headModel) = 0;
	};

	struct LiteralHandler: BlockHandler {
		const char head, *data;
		size_t length;
		
		inline LiteralHandler(char head, const char *data, size_t length): head(head), data(data), length(length) {}

		inline virtual void feed(RansModel& litModel, RansModel& diffModel, RansModel& headModel)
		{			
			for(const char* i = data; i < data + length; i++)
				litModel.add(*i);
				
			headModel.add(head);
		}

		inline virtual void process(RansEncoder& encoder, RansModel& litModel, RansModel& diffModel, RansModel& headModel)
		{
			for(const char* i = data + length - 1; i >= data; i--)
				encoder.put(litModel, *i);	
				
			headModel.substract(head);
			headModel.update();
			encoder.put(headModel, head);
		}
	};

	struct RefHandler: BlockHandler {
		char head, diff;
		
		inline RefHandler(char head, char diff): head(head), diff(diff) {}

		inline virtual void feed(RansModel& litModel, RansModel& diffModel, RansModel& headModel) {
			diffModel.add(diff);
			headModel.add(head);
		}

		inline virtual void process(RansEncoder& encoder, RansModel& litModel, RansModel& diffModel, RansModel& headModel)
		{
			diffModel.substract(diff);		
			diffModel.update();
			encoder.put(diffModel, diff);

			headModel.substract(head);
			headModel.update();
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

	char* start;
	size_t olength;

public:
	inline SergMixedEncoder(char* start, size_t length): start(start), olength(length) {}
	
	inline size_t compress(const char* const in, size_t length)
	{
		MlzEncoder<SergMixedEncoder>::compress(in, length);
		RansModel litModel, diffModel, headModel;
		
		for(auto &x: blocks)
			x->feed(litModel, diffModel, headModel);

		litModel.update();
			
		for(auto &x: litModel.widths) {
			*start++ = (uint32_t)x >> 8;
			*start++ = x >> 0;
		}
		
		RansEncoder encoder(start, olength - 2 * 256);

		for(auto r = blocks.rbegin(); r != blocks.rend(); r++)
			(*r)->process(encoder, litModel, diffModel, headModel);
		
		return encoder.flush() + 2 * 256;
	}
};

class SergMixedDecoder: public MlzDecoder<SergMixedDecoder>, RansDecoder
{
	friend class MlzDecoder<SergMixedDecoder>;
		
	RansModelBase litModel;
	RansModel diffModel, headModel;
	
	inline char readHeader() {
		auto ret = this->get(headModel);
		headModel.update();
		return ret;
	}

	inline char readDiff() {
		auto ret = this->get(diffModel);
		diffModel.update();
		return ret;
	}
	
	inline char readLiteral() {	
		return this->get(litModel);
	}
	
	inline void afterLiteral() { }

public:
	inline SergMixedDecoder(const char* in): RansDecoder(in + 2 * 256) 
	{
		for(auto &x: litModel.widths) {
			x = (unsigned char)*in++ << 8;
			x |= (unsigned char)*in++;
		}
		
		litModel.updateStarts();
	}	
};

#endif
