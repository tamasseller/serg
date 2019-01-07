#include "Rans.h"
#include "RansModel.h"

#include "HexDump.h"

#include <iostream>
#include <string.h>
#include <stdint.h>
#include <cstdlib>
#include <ctime>
#include <memory>

struct DummyModel 
{	
	struct Symbol {
		char c;
		size_t width;		
		inline Symbol(char c, size_t width): c(c), width(width) {}
	};
	
	struct Symstat {
		size_t width, cummulated;
		inline Symstat(size_t width, size_t cummulated): width(width), cummulated(cummulated) {}
	};
	
	Symbol syms[4] = {
		Symbol('a', 0x4000),
		Symbol('b', 0x8000),
		Symbol('c', 0x0001),
		Symbol('d', 0x3fff)
	};
	
	Symstat predict(char c)
	{
		size_t cumm = 0;
		for(int i = 0; i < sizeof(syms)/sizeof(syms[0]); i++) {
			if(syms[i].c == c)
				return Symstat(syms[i].width, cumm);
			
			cumm += syms[i].width;
		}
	}

	Symstat identify(size_t in, char &c)
	{
		size_t cumm = 0;
		for(int i = 0; i < sizeof(syms)/sizeof(syms[0]); i++) {
			if(cumm <= in && in < cumm + syms[i].width ) {
				c = syms[i].c;
				return Symstat(syms[i].width, cumm);
			}
			
			cumm += syms[i].width;
		}
	}
	
	void update(){}
	void add(char){}
	void substract(char){}
};

template<class Model>
void encodeBlock(RansEncoder &encoder, Model &model, const char* top, const char* bottom)
{
	for(const char* i = top - 1; i >= bottom; i--)
		model.substract(*i);
		
	model.update();
	
	for(const char* i = top - 1; i >= bottom; i--)
		encoder.put(model, *i);	
}

template<class Model>
void compress(const char* in, size_t length, char* out, size_t &outLength)
{
	RansEncoder encoder(out, outLength);
	Model model;
	
	for(int idx = 0; idx < length; idx++)
		model.add(in[idx]);

	const auto n = length % 3;
	encodeBlock(encoder, model, in + length, in + length - n);
	length -= n;
	
	for(const char* i = in + length; i != in; i -= 3)
		encodeBlock(encoder, model, i, i - 3);
		
	outLength = encoder.flush();
}

template<class Model>
void decompress(const char* in, size_t length, char* out, size_t outLength)
{
	RansDecoder decoder(in);
	Model model;

	for(char* o = out; o < out + outLength;) {
		for(int i = 0; i < 3 && o < out + outLength; i++)
			*o++ = decoder.get(model);
			
		model.update();
	}
		
	decoder.check();
}

void compressSwitched(const char* in, size_t length, char* out, size_t &outLength)
{
	RansEncoder encoder(out, outLength);
	DummyModel model;
	const char* i = in + length - 1;
	
	while(i >= in) {
		encoder.put(model, *i--);
		encoder.writeSideChannel('a');
	}
	
	outLength = encoder.flush();
}

void decompressSwitched(const char* in, size_t length, char* out, size_t outLength)
{
	RansDecoder decoder(in);
	DummyModel model;

	for(char* o = out; o < out + outLength;) {
		decoder.readSideChannel();
		*o++ = decoder.get(model);
	}
		
	decoder.check();
}


template<void (*compress)(const char*, size_t, char*, size_t&), void (*decompress)(const char*, size_t, char* out, size_t)>
void doTest(const char* data)
{
	const size_t inLength = strlen(data);
	char compressed[4096] = {0,}, decompressed[4096] = {0,};
	size_t compressedLength = sizeof(compressed);
	
	compress(data, inLength, compressed, compressedLength);
	
	decompress(compressed, compressedLength, decompressed, inLength);
	
	if(memcmp(data, decompressed, inLength) != 0) 
	{
		std::cout << "In-out data mismatch for input: '" << data << "'" << std::endl;
		
		std::cout << std::endl << "Compressed:" << std::endl;
		hexDump(compressed, compressedLength);
		
		std::cout << std::endl << "Decompressed:" << std::endl;
		hexDump(decompressed, inLength);
		
		std::cout << std::endl;
		exit(-1);
	}
}

void test(const char* data)
{
	doTest<compress<DummyModel>, decompress<DummyModel>>(data);
	doTest<compress<RansModel>, decompress<RansModel>>(data);
	doTest<compressSwitched, decompressSwitched>(data);
}

void prngTest(int n)
{
	std::unique_ptr<char> b(new char[n+1]);
	
	for(int i=0; i < n; i++)
		b.get()[i] = 'a' + (std::rand() % 4);
		
	b.get()[n] = '\0';
	test(b.get());
}

int main()
{
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
	
    std::srand(0x1337);
    
    prngTest(8);
    prngTest(16);
    prngTest(128);
    prngTest(1024);
    
    return 0;
}
