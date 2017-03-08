#include "CRC.h"
#include "RAIIMutex.h"

bool CRC32::crc32TableCalculated = false;
uint32_t CRC32::crc32Table[256] = {};

void CRC32::generateCRC32Table(void)
{
	uint32_t crc;
	for (std::size_t i = 0; i < 256; i++)
	{
		crc = i;
		for (std::size_t j = 0; j < 8; j++)
		{
			crc = (0xedb88320L * (crc & 1)) ^ (crc >> 1);
		}
		crc32Table[i] = crc;
	}
}

uint32_t CRC32::calculate(uint32_t crc, char* buffer, std::size_t length)
{
	if (!crc32TableCalculated)
	{
		// By checking the boolean twice, we ensure that we aren't locking resources
		// the majority of the time, and even if we do wind up locking multiple times
		// it won't be _every_ time. Just the first few if multiple threads hit this
		// all at about the same time.
		RAIIMutex crcTableLock(&crc32Table);
		if (!crc32TableCalculated)
		{
			generateCRC32Table();
			crc32TableCalculated = true;
		}
	}

	uint32_t newCrc = crc ^ 0xffffffffL;

	for (std::size_t i = 0; i < length; i++)
	{
		newCrc = crc32Table[(newCrc ^ buffer[i]) & 0xff] ^ (newCrc >> 8);
	}

	return newCrc ^ 0xffffffffL;
}
