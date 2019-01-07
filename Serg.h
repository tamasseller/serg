#ifndef _SERG_H
#define _SERG_H

#include "MLZ.h"
#include "Rans.h"
#include "RansModel.h"

#include <vector>

class SergEncoder: MLZEncoder<SergEncoder>, RansEncoder
{
	friend class MLZEncoder<SergEncoder>;
	
	struct BlockHandler {
		virtual void feed(RansModel& model) = 0;
		virtual void process(RansEncoder& encoder, RansModel& model) = 0;
	};

	struct LiteralHandler: BlockHandler {
		const char head, *data;
		size_t length;
		
		inline LiteralHandler(char head, const char *data, size_t length): head(head), data(data), length(length) {}

		inline virtual void feed(RansModel& model)
		{
			for(const char* i = data; i < data + length; i++)
				model.add(*i);
		}

		inline virtual void process(RansEncoder& encoder, RansModel& model)
		{
			for(const char* i = data + length - 1; i >= data; i--)
				model.substract(*i);
		
			model.update();
	
			for(const char* i = data + length - 1; i >= data; i--)
				encoder.put(model, *i);	

			encoder.writeSideChannel(head);
		}
	};

	struct RefHandler: BlockHandler {
		char head, diff;
		
		inline RefHandler(char head, char diff): head(head), diff(diff) {}

		inline virtual void feed(RansModel& model) {}
		
		inline virtual void process(RansEncoder& encoder, RansModel& model)
		{
			encoder.writeSideChannel(diff);
			encoder.writeSideChannel(head);
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

public:
	inline SergEncoder(char* start, size_t length): RansEncoder(start, length) {}
	
	inline size_t compress(const char* const in, size_t length)
	{
		MLZEncoder<SergEncoder>::compress(in, length);
		RansModel model;
		
		for(auto &x: blocks)
			x->feed(model);
		
		for(auto r = blocks.rbegin(); r != blocks.rend(); r++)
			(*r)->process(*this, model);
		
		return RansEncoder::flush();
	}
};

class SergDecoder: public MLZDecoder<SergDecoder>, RansDecoder
{
	friend class MLZDecoder<SergDecoder>;
		
	const char* end;
	RansModel model;
	
	inline char readHeader() {
		return this->readSideChannel();
	}

	inline char readDiff() {
		return this->readSideChannel();
	}
	
	inline char readLiteral() {	
		return this->get(model);
	}
	
	inline bool hasMoreReadable() {
		return in < end;
	}

	inline void afterLiteral() {
		model.update();
	}

public:
	inline SergDecoder(const char* in, size_t length): RansDecoder(in), end(in + length) {}
};

#endif
