#include "TestCommon.h"
#include "Serg.h"
#include "SergStatic.h"
#include "SergMixed.h"


#include <iostream>
#include <sstream>
#include <string.h>
#include <memory>

template<class Coder>
static inline void c(const char* data, size_t inLength, char* compressed, size_t& compressedLength) 
{
	Coder enc(compressed, compressedLength);
	compressedLength = enc.compress(data, inLength);	
}

template<class Coder>
static inline void d(const char* compressed, size_t compressedLength, char* decompressed, size_t decompressedLength) 
{
	Coder dec(compressed);
	dec.decompress(decompressed, decompressedLength);
}

static inline void test(const char* str) {
	compDecompTestRun<c<SergEncoder>, d<SergDecoder>>(str);
	compDecompTestRun<c<SergStaticEncoder>, d<SergStaticDecoder>>(str);
	compDecompTestRun<c<SergMixedEncoder>, d<SergMixedDecoder>>(str);
}

static inline void prngTest(int n) {
	prngCompDecompTestRun<c<SergEncoder>, d<SergDecoder>>(n);
	prngCompDecompTestRun<c<SergStaticEncoder>, d<SergStaticDecoder>>(n);
	prngCompDecompTestRun<c<SergMixedEncoder>, d<SergMixedDecoder>>(n);
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
