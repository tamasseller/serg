#include "Serg.h"

#include "HexDump.h"

#include <iostream>
#include <sstream>

#include <string.h>

#include <memory>

void test(const char* data)
{	
	const size_t inLength = strlen(data);
	char compressed[2 * 1024 * 1024] = {0,}, decompressed[2 * 1024 * 1024] = {0,};
	
	float entropy;
	SergEncoder enc(compressed, sizeof(compressed));
	size_t compressedLength = enc.compress(data, inLength, entropy);
	
	SergDecoder dec(compressed, compressedLength);
	dec.decompress(decompressed, inLength);
	
	if(memcmp(data, decompressed, inLength) != 0) 
	{
		if(inLength < 1024) {
			std::cout << "In-out data mismatch for input: '" << data << "'" << std::endl;
			
			std::cout << std::endl << "Compressed:" << std::endl;
			hexDump(compressed, compressedLength);

			std::cout << std::endl << "Decompressed:" << std::endl;
			hexDump(decompressed, inLength);
		} else
			std::cout << "In-out data mismatch" << std::endl;
		
		for(int i = 0; i < inLength; i++) {
			if(data[i] != decompressed[i]) {
				std::cout << std::endl << "First difference at 0x" << std::hex << i << std::endl;
				
				size_t offset = std::max(0, (i - 15) & ~15);
				size_t end = std::min(inLength, offset + 48);
				
				std::cout << std::endl << "Input +0x"  << std::hex << offset << ":" << std::endl;
				hexDump(data + offset, end - offset);

				std::cout << std::endl << "Decompressed  +0x"  << std::hex << offset << ":" << std::endl;
				hexDump(decompressed + offset, end - offset);
				
				break;
			}
		}	
		
		std::cout << std::endl;
		exit(-1);
	}
}


void prngTest(int n)
{
	std::unique_ptr<char> b(new char[n+1]);
	
	for(int i=0; i < n; i++)
		b.get()[i] = std::rand() % 64 + 64;
		
	b.get()[n] = '\0';
	test(b.get());
}


int main()
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
		
	std::srand(0x1337);
    
    prngTest(8);
    prngTest(16);
    prngTest(128);
    prngTest(1024);
    prngTest(1024*1024);

	return 0;
}
 
