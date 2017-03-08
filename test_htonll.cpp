#include "Endian.h"

#include <iostream>
using namespace std;

int main(void)
{
	uint64_t value = 0xc0ffeecafec0de42LL;
	cout << hex << value << endl << htonll(value) << endl;
	return 0;
}
