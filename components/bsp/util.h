#ifndef UTIL_H_
#define UTIL_H_

#include "bsp.h"
#include <stdint.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/// @brief Base enumeration (2/8/10/16), invalid returns -1
typedef enum {
    BASE_INVALID = -1,
    BASE_BINARY = 2,
    BASE_OCTAL = 8,
    BASE_DECIMAL = 10,
    BASE_HEXADECIMAL = 16
} NumberBase;


/// @brief Count the number of 1s
/// @param n : The unsigned integer to be checked
/// @return 1's number
uint ulUtilCountOnesWith32Bits(uint n);

/// @brief check and find one location
/// @param n : The unsigned integer to be checked
/// @return -1 : invaild(the 1 have more the one); other : location of 1
int lUtilCheckAndFindOneLocal(uint n);

/// @brief Determine the base of the number represented by the string
/// @param str : Input string (supports positive and negative signs, and common number base prefixes)
/// @return Base (2/8/10/16), invalid returns BASE_INVALID(-1)
NumberBase eUtilGetNumberBase(const char *str);


#ifdef __cplusplus
}
#endif

#endif /*  UTIL_H_ */