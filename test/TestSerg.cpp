#include "TestCommon.h"
#include "Serg.h"

#include <iostream>
#include <sstream>
#include <string.h>
#include <memory>

static inline void c(const char* data, size_t inLength, char* compressed, size_t& compressedLength) 
{
	SergEncoder enc(compressed, compressedLength);
	compressedLength = enc.compress(data, inLength);	
}

static inline void d(const char* compressed, size_t compressedLength, char* decompressed, size_t decompressedLength) 
{
	SergDecoder dec(compressed);
	dec.decompress(decompressed, decompressedLength);
}

static inline void test(const char* str) {
	compDecompTestRun<c, d>(str);
}

static inline void prngTest(int n) {
	prngCompDecompTestRun<c, d>(n);
}

int runSergTests()
{
	test("asdqwe");
	test("asdasd");
	test("asdasdqwe");
	test("foo bar foobar");
	test("abcdabcabaababcabcd");
	test("aaaa");
	test("bbbb");
	test("cccc");
	test("dddd");
	test("bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb");
	test("abc");
	test("bbbba");
	test("baba");
	test("abbcccbba");
	test("abcdbcda");
	    	
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
    prngTest(8*1024*1024);
}
