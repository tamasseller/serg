#include "MlzEncoder.h"
#include "MlzDecoder.h"
#include "HexDump.h"

#include "TestCommon.h"

#include <sstream>

class DirectEncoder: public MlzEncoder<DirectEncoder>
{
	friend class MlzEncoder<DirectEncoder>;
	
	char *out, * const start;

	inline void writeLiteral(const char* in, size_t length)
	{
		*out++ = (char)(length - minLitLength);
		memcpy(out, in, length);
		out += length;
	}

	inline void writeRef(size_t diff, size_t length)
	{
		*out++ = (char)- (length - (minRefLength - 1));
		*out++ = diff - minDiff;
	}
	
	inline size_t getWriteOffset()
	{
		return out - start;
	}

public:
	inline DirectEncoder(char* start): out(start), start(start) {}
	
	static inline void c(const char* data, size_t inLength, char* compressed, size_t& compressedLength){
		DirectEncoder enc(compressed);
		compressedLength = enc.compress(data, inLength);
	}
};

class DirectDecoder: public MlzDecoder<DirectDecoder>
{
	friend class MlzDecoder<DirectDecoder>;
		
	const char* in;
	size_t length;

	inline char readHeader() {
		length--;
		return *in++;
	}

	inline char readLiteral() {
		length--;
		return *in++;
	}

	inline char readDiff() {
		length--;
		return *in++;
	}

	inline void afterLiteral() { }
	
public:
	inline DirectDecoder(const char* in, size_t length): in(in), length(length) {}

	static inline void d(const char* compressed, size_t compressedLength, char* decompressed, size_t decompressedLength){
		DirectDecoder dec(compressed, compressedLength);
		dec.decompress(decompressed, decompressedLength);
	}	
};

static inline void test(const char* str) {
	compDecompTestRun<DirectEncoder::c, DirectDecoder::d>(str);
}

static inline void prngTest(int n) {
	prngCompDecompTestRun<DirectEncoder::c, DirectDecoder::d>(n);
} 

int runMlzTests()
{
	test("asdqwe");
	test("asdasd");
	test("asdasdqwe");
	test("foo bar foobar");
	test("abcdabcabaababcabcd");
	
	std::stringstream ss;
	
	for(int i=0; i<64; i++) {
		ss.width(2);
		ss.fill('0');
		ss << std::hex << i << ' ';
	}
	
	test(ss.str().c_str());
	
	std::string temp = ss.str() + ss.str();
	test(temp.c_str());

	std::string temp2 = temp + ss.str();
	test(temp.c_str());
    
    prngTest(8);
    prngTest(16);
	prngTest(128);
    prngTest(1024);
    prngTest(64*1024);
}
 
