#include "MLZ.h"
#include "HexDump.h"

#include <iostream>
#include <sstream>

#include <string.h>

class DirectEncoder: public MLZEncoder<DirectEncoder>
{
	friend class MLZEncoder<DirectEncoder>;
	
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
};

class DirectDecoder: public MLZDecoder<DirectDecoder>
{
	friend class MLZDecoder<DirectDecoder>;
		
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
	
	inline bool hasMoreReadable() {
		return length;
	}

	inline void afterLiteral() { }
	
public:
	inline DirectDecoder(const char* in, size_t length): in(in), length(length) {}
};

void test(const char* data)
{	
	const size_t inLength = strlen(data);
	char compressed[4096] = {0,}, decompressed[4096] = {0,};
	size_t decompressedLength;
	
	DirectEncoder enc(compressed);
	size_t compressedLength = enc.compress(data, inLength);
	
	if(compressedLength > MLZBase::wcCompressedLength(inLength)) 
	{
		std::cout << "Wrong compressed length (" << compressedLength << ") for input: '" << data << "'" << std::endl;
		
		std::cout << std::endl << "Compressed:" << std::endl;
		hexDump(compressed, compressedLength);
		
		std::cout << std::endl;
		exit(-1);
	}
	
	DirectDecoder dec(compressed, compressedLength);
	dec.decompress(decompressed, decompressedLength);

	if(decompressedLength != inLength) 
	{
		std::cout << "Wrong decompressed length (" << decompressedLength << ") for input: '" << data << "'" << std::endl;

		std::cout << std::endl << "Compressed:" << std::endl;
		hexDump(compressed, compressedLength);

		std::cout << std::endl << "Decompressed:" << std::endl;
		hexDump(decompressed, decompressedLength);
		
		std::cout << std::endl;
		exit(-1);
	}
	
	if(memcmp(data, decompressed, inLength) != 0) 
	{
		std::cout << "In-out data mismatch for input: '" << data << "'" << std::endl;
		
		std::cout << std::endl << "Compressed:" << std::endl;
		hexDump(compressed, compressedLength);
		
		std::cout << std::endl << "Decompressed:" << std::endl;
		hexDump(decompressed, decompressedLength);
		
		std::cout << std::endl;
		exit(-1);
	}
}

int main()
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
	
	return 0;
}
 
