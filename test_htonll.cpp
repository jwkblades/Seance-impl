#include <iostream>
using namespace std;

uint64_t htonll(uint64_t value)
{
	uint64_t ret = 0;
	for (std::size_t i = 0; i < 8; i++)
	{
		ret <<= 8;
		ret |= (value & 0xff);
		value >>= 8;
	}
	return ret;
}
int main(void)
{
	uint64_t value = 0xc0ffeecafec0de42LL;
	cout << hex << value << endl << htonll(value) << endl;
	return 0;
}
