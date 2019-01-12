#ifndef _SERG_STATIC_H
#define _SERG_STATIC_H

#include "MlzEncoder.h"
#include "MlzDecoder.h"
#include "RansEncoder.h"
#include "RansDecoder.h"
#include "RansModel.h"

#include <vector>
#include <math.h>

class SergStaticEncoder: MlzEncoder<SergStaticEncoder>
{
	friend class MlzEncoder<SergStaticEncoder>;
	
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
				
			encoder.put(headModel, head);
		}
	};

	struct RefHandler: BlockHandler {
		char head, diff;
		
		inline RefHandler(char head, char diff): head(head), diff(diff) {}

		inline virtual void feed(RansModel& litModel, RansModel& diffModel, RansModel& headModel)
		{
			diffModel.add(diff);
			headModel.add(head);
		}

		inline virtual void process(RansEncoder& encoder, RansModel& litModel, RansModel& diffModel, RansModel& headModel)
		{
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

	char* start;
	size_t olength;
	
	inline void saveModel(char* &start, RansModel &model)
	{
		for(auto x: model.widths) {
			*start++ = (uint32_t)x >> 8;
			*start++ = x >> 0;
		}
	}

public:
	inline SergStaticEncoder(char* start, size_t length): start(start), olength(length) {}
	
	inline size_t compress(const char* const in, size_t length)
	{
		MlzEncoder<SergStaticEncoder>::compress(in, length);
		RansModel litModel, diffModel, headModel;
		
		for(auto &x: blocks)
			x->feed(litModel, diffModel, headModel);

		litModel.update();
		diffModel.update();
		headModel.update();

		saveModel(start, litModel);
		saveModel(start, diffModel);
		saveModel(start, headModel);
		
		RansEncoder encoder(start, olength - 3 * 2 * 256);

		for(auto r = blocks.rbegin(); r != blocks.rend(); r++)
			(*r)->process(encoder, litModel, diffModel, headModel);
		
		return encoder.flush() + 3 * 2 * 256;
	}
};

class SergStaticDecoder: public MlzDecoder<SergStaticDecoder>, RansDecoder
{
	friend class MlzDecoder<SergStaticDecoder>;
		
	RansModelBase litModel, diffModel, headModel;
	
	inline char readHeader() {
		return this->get(headModel);
	}

	inline char readDiff() {
		return this->get(diffModel);
	}
	
	inline char readLiteral() {	
		return this->get(litModel);
	}
	
	inline void afterLiteral() { }
	
	inline void loadModel(const char* &start, RansModelBase &model)
	{
		for(auto &x: model.widths) {
			x = (unsigned char)*start++ << 8;
			x |= (unsigned char)*start++;
		}
		
		model.updateStarts();
	}

public:
	inline SergStaticDecoder(const char* in): RansDecoder(in + 3 * 2 * 256) 
	{
		loadModel(in, litModel);
		loadModel(in, diffModel);
		loadModel(in, headModel);
	}	
};

#endif
