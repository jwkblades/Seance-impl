#include "Endian.h"

bool isNetworkEndian(void)
{
	uint16_t val = 0xff00;
	return (val & 0xff) == 0;
}

uint64_t ntohll(uint64_t value)
{
	return htonll(value);
}

uint64_t htonll(uint64_t value)
{
	if (isNetworkEndian())
	{
		return value;
	}

	uint64_t ret = 0;
	for (std::size_t i = 0; i < 8; i++)
	{
		ret <<= 8;
		ret |= (value & 0xff);
		value >>= 8;
	}
	return ret;
}
