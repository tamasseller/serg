#include "Compressor.h"
#include "HexDump.h"

#include <iostream>
#include <fstream>
#include <memory>

int main(int argc, const char *argv[])
{
	if(argc != 3) {
		std::cout << "Minimal LZ* compression utility (with a trivial embeddable decomompressor) Tamás Seller 2019" << std::endl << std::endl;
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
		
	size_t inLength = inputFile.tellg();
	inputFile.seekg (0, std::ios::beg);
	
	std::unique_ptr<char> data(new char[inLength]);
	inputFile.read(data.get(), inLength);
	
	
	size_t compressedLength = inLength;
	std::unique_ptr<char> compressed(Compressor::pack(data.get(), compressedLength));
	
	if(compressedLength > (Compressor::wcCompressedLength(inLength) + 4)) {
		std::cout << "Internal error - invalid output size !!!" << std::endl;
		return -1;
	}
	
	size_t decompressedLength = compressedLength;
	std::unique_ptr<char> decompressed(Compressor::unpack(compressed.get(), decompressedLength));
	
	if(decompressedLength != inLength) {
		std::cout << "Internal error - invalid test output size !!!" << std::endl;
		return -1;
	}

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
 
