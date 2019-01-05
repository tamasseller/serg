#ifndef _HEXDUMP_H
#define _HEXDUMP_H

#include <iostream>

static inline void hexDump(const void* aData, size_t aLength)
{
	static constexpr size_t aWidth = 16;
	const char* const start = static_cast<const char*>(aData);
	const char* const end = start + aLength;
	const char* line = start;
	
	while (line != end)
	{
		std::cout.width(8);
		std::cout.fill('0');
		std::cout << std::hex << line - start << "  ";
		
		size_t lineLength = std::min(aWidth, static_cast<size_t>(end - line));
		
		for (const char* next = line; next != end && next != line + aWidth; ++next) 
		{
			if(next - line == aWidth / 2)
				std::cout << ' ';

			std::cout.width(2);
			std::cout.fill('0');
			std::cout << std::hex << std::uppercase << static_cast<int>(static_cast<unsigned char>(*next)) << " ";			
		}
		
		if (lineLength != aWidth)
			std::cout << std::string((aWidth - lineLength) * 3 + ((lineLength <= (aWidth / 2)) ? 1 : 0), ' ');
		
		std::cout << " |";
		
		for (const char* next = line; next != end && next != line + aWidth; ++next)
			std::cout << (*next < 32 ? '.' : *next);			

		if (lineLength != aWidth)
			std::cout << std::string(aWidth - lineLength, ' ');
			
		std::cout << "|" << std::endl;
		line = line + lineLength;
	}
}

#endif
