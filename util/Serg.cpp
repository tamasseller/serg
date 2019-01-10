#include "Serg.h"

#include <iostream>
#include <fstream>
#include <memory>

#include "../test/HexDump.h"

int main(int argc, const char *argv[])
{
	if(argc != 3) {
		std::cout << "Self Extracting Resource Generator - compression utility - Tamás Seller 2019" << std::endl << std::endl;
		std::cout << "\tUsage: " << argv[0] << " <input file> <output file>" << std::endl << std::endl;
		return -1;
	}

	const char *inputFileName = argv[1];
	const char *outputFileName = argv[2];

	std::ifstream inputFile(inputFileName, std::ios::in | std::ios::binary | std::ios::ate);
	
	if (!inputFile.is_open())  {
		std::cout << "Unable to open input file '" << inputFileName << "'" << std::endl;	
		return -1;
	}
		
	const size_t inLength = inputFile.tellg();
	inputFile.seekg (0, std::ios::beg);
	
	std::unique_ptr<char> data(new char[inLength]);
	inputFile.read(data.get(), inLength);
	
	const auto cBuffLength = inLength * 10;
	std::unique_ptr<char> compressed(new char[cBuffLength]); // XXX has to be large enough, but still...
	
	SergEncoder enc(compressed.get(), cBuffLength);
	size_t compressedLength = enc.compress(data.get(), inLength);

	std::unique_ptr<char> decompressed(new char[inLength]);
	
	SergDecoder dec(compressed.get());
	dec.decompress(decompressed.get(), inLength);
	
	if(memcmp(decompressed.get(), data.get(), inLength) != 0) {
		std::cout << "Internal error - invalid test output data !!!" << std::endl;		
		return -1;
	}
	
	std::ofstream outputFile(outputFileName, std::ios::out | std::ios::trunc);
	if (!outputFile.is_open())  {
		std::cout << "Unable to open output file '" << inputFileName << "'" << std::endl;	
		return -1;
	}
	
	outputFile.write(compressed.get(), compressedLength);
	
	std::cout << inputFileName << " -> " << outputFileName << "  " << ((100 * compressedLength) / inLength) << "%" << std::endl;
	
	return 0;
}
 