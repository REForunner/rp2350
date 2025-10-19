#ifndef UTIL_H_
#define UTIL_H_

#include "bsp.h"
#include <stdint.h>

/// @brief Count the number of 1s
/// @param n : The unsigned integer to be checked
/// @return 1's number
uint ulUtilCountOnesWith32Bits(uint n);

/// @brief check and find one location
/// @param n : The unsigned integer to be checked
/// @return -1 : invaild(the 1 have more the one); other : location of 1
int lUtilCheckAndFindOneLocal(uint n);

#endif /*  UTIL_H_ */