#include "Endian.h"

union EndianValue
{
	char bytes[2];
	uint16_t value;
};

bool isNetworkEndian(void)
{
	EndianValue endian;
	endian.value = uint16_t(0xff00);
	return endian.bytes[0] == 0xff;
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
