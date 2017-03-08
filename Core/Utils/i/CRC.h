#ifndef __CRC_H
#define __CRC_H

#include <cstdint>

class CRC32
{
public:
	static uint32_t calculate(uint32_t crc, char* buffer, std::size_t length);
private:
	static void generateCRC32Table(void);
	static bool crc32TableCalculated;
	static uint32_t crc32Table[256];
};

#endif
