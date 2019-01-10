#ifndef _TEST_COMMON_H
#define _TEST_COMMON_H

#include "HexDump.h"

#include <string.h>
#include <memory>

template<void (*compress)(const char*, size_t, char*, size_t&), void (*decompress)(const char*, size_t, char* out, size_t)>
void compDecompTestRun(const char* data)
{
	const size_t inLength = strlen(data);
	size_t compressedLength = 16 * 1024 * 1024;
	std::unique_ptr<char[]> compressed(new char[compressedLength]);
	std::unique_ptr<char[]> decompressed(new char[compressedLength]);
	
	compress(data, inLength, compressed.get(), compressedLength);
	decompress(compressed.get(), compressedLength, decompressed.get(), inLength);
	
	if(memcmp(data, decompressed.get(), inLength) != 0)  {
		if(inLength < 1024) {
			std::cout << "In-out data mismatch for input: '" << data << "'" << std::endl;
			
			std::cout << std::endl << "Compressed:" << std::endl;
			hexDump(compressed.get(), compressedLength);

			std::cout << std::endl << "Decompressed:" << std::endl;
			hexDump(decompressed.get(), inLength);
		} else
			std::cout << "In-out data mismatch" << std::endl;
		
		for(int i = 0; i < inLength; i++) {
			if(data[i] != decompressed.get()[i]) {
				std::cout << std::endl << "First difference at 0x" << std::hex << i << std::endl;
				
				size_t offset = std::max(0, (i - 15) & ~15);
				size_t end = std::min(inLength, offset + 48);
				
				std::cout << std::endl << "Input +0x"  << std::hex << offset << ":" << std::endl;
				hexDump(data + offset, end - offset);

				std::cout << std::endl << "Decompressed  +0x"  << std::hex << offset << ":" << std::endl;
				hexDump(decompressed.get() + offset, end - offset);
				
				break;
			}
		}
		
		std::cout << std::endl;
		exit(-1);
	}
}

static inline std::unique_ptr<char[]> prngTestData(int n)
{
	std::unique_ptr<char[]> b(new char[n+1]);
	
	for(int i=0; i < n; i++)
		b.get()[i] = std::rand() % 64 + 64;
		
	b.get()[n] = '\0';
	return b;
}

template<void (*compress)(const char*, size_t, char*, size_t&), void (*decompress)(const char*, size_t, char* out, size_t)>
void prngCompDecompTestRun(int n)
{
	auto data = prngTestData(n);
	compDecompTestRun<compress, decompress>(data.get());
}

#endif
