#include "util.h"

/// @brief Count the number of 1s
/// @param n : The unsigned integer to be checked
/// @return 1's number
uint ulUtilCountOnesWith32Bits(uint n)
{
#if defined (__GNUC__)
    return (uint)__builtin_popcount(n);
#else
    uint count = 0;
    while (n != 0) 
    {
        n &= n - 1;  // 清除最右侧的1（关键操作）
        count++;
    }
    return count;
#endif
}

/// @brief check and find one location
/// @param n : The unsigned integer to be checked
/// @return -1 : invaild(the 1 have more the one); other : location of 1
int lUtilCheckAndFindOneLocal(uint n)
{
    // check 1 number, if numbers isn't one, return
    if(1U != ulUtilCountOnesWith32Bits(n))
    {
        // return invaild param
        return -1;
    }
    
    int local = 0;
    // shift and check
    do{
        n >>= 1U;
        local += 1U;
    }while(n);
    
    return local;
}

/// @brief Determine whether a character is a valid character for the specified number base
/// @param c : Character to be determined
/// @param base : Base (2/8/10/16)
/// @return 1 - Effective, 0 - Ineffective
static int prvIsValidChar(char c, NumberBase base) 
{
    switch (base) {
        case BASE_BINARY:
            return (c == '0' || c == '1');
        case BASE_OCTAL:
            return (c >= '0' && c <= '7');
        case BASE_DECIMAL:
            return isdigit((unsigned char)c);
        case BASE_HEXADECIMAL:
            return isxdigit((unsigned char)c);
        default:
            return 0;
    }
}

/// @brief Determine the base of the number represented by the string
/// @param str : Input string (supports positive and negative signs, and common number base prefixes)
/// @return Base (2/8/10/16), invalid returns BASE_INVALID(-1)
NumberBase eUtilGetNumberBase(const char *str) 
{
    if (str == NULL || *str == '\0' || *str == ' ') {
        // Empty string is invalid
        return BASE_INVALID;
    }

    const char *p = str;
    // Skip the positive and negative signs at the beginning 
    // (this does not affect the judgment of the number base)
    if (*p == '+' || *p == '-') {
        p++;
        if (*p == '\0' || *p == ' ') { 
            // Only contains symbols, invalid
            return BASE_INVALID;
        }
    }
    // Check binary prefixes (0b/0B)
    if (*p == '0' && (*(p + 1) == 'b' || *(p + 1) == 'B')) {
        // Skip prefix
        p += 2; 
        if (*p == '\0' || *p == ' ') {
            // No valid characters after the prefix, invalid
            return BASE_INVALID;
        }
        // Verify whether all subsequent characters comply with the binary rules
        while (*p != '\0' && *p != ' ') {
            if (!prvIsValidChar(*p, BASE_BINARY)) {
                return BASE_INVALID;
            }
            p++;
        }
        return BASE_BINARY;
    }
    // Check hexadecimal prefixes (0x/0X)
    if (*p == '0' && (*(p + 1) == 'x' || *(p + 1) == 'X')) {
        // Skip prefix
        p += 2;
        if (*p == '\0' || *p == ' ') {
            // No valid characters after the prefix, invalid
            return BASE_INVALID;
        }
        // Verify whether all subsequent characters comply with the hexadecimal rules
        while (*p != '\0' && *p != ' ') {
            if (!prvIsValidChar(*p, BASE_HEXADECIMAL)) {
                return BASE_INVALID;
            }
            p++;
        }
        return BASE_HEXADECIMAL;
    }
    // Check octal prefixes (starting with 0 and followed by numbers 0-7)
    if (*p == '0') {
        // Skip the 0 prefix
        p++;
        if (*p == '\0' || *p == ' ') {
            // The individual "0" is regarded as a decimal digit.
            return BASE_DECIMAL;
        }
        // Verify whether all subsequent characters comply with the octal rules
        while (*p != '\0' && *p != ' ') {
            if (!prvIsValidChar(*p, BASE_OCTAL)) {
                return BASE_INVALID;
            }
            p++;
        }
        return BASE_OCTAL;
    }
    // The remaining situation is in decimal (consisting only of 0-9)
    while (*p != '\0' && *p != ' ') {
        if (!prvIsValidChar(*p, BASE_DECIMAL)) {
            return BASE_INVALID;
        }
        p++;
    }
    return BASE_DECIMAL;
}