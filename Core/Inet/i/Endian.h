#ifndef __ENDIAN_H
#define __ENDIAN_H

bool isNetworkEndian(void);

uint64_t ntohll(uint64_t value);
uint64_t htonll(uint64_t value);

#endif
