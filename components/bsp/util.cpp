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
