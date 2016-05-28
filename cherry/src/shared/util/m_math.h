#ifndef _M_MATH_H_
#define _M_MATH_H_

#include <climits>
#include <type_traits>

class MMath
{
public:
    template <typename T, typename std::enable_if<std::is_unsigned<T>::value, int>::type = 0>
    static T NextPower2Number(T val)
    {
        --val;
        for (std::size_t i = 1; i < sizeof(val) * CHAR_BIT; i <<= 1) {
            val |= (val >> i);
        }
        return ++val;
    }
	template <typename T, typename std::enable_if<std::is_unsigned<T>::value, int>::type = 0>
    static bool IsPowerOf2(T val)
    {
        return val & (val - 1);
    }
	template <typename T, typename std::enable_if<std::is_unsigned<T>::value, int>::type = 0>
    static unsigned short Power2Index(T val)
    {
		unsigned short index = 0;
        while (val >>= 1)
		{
			++index;
		}
        return index;
    }
	template <typename T, typename std::enable_if<std::is_unsigned<T>::value, int>::type = 0>
    static unsigned short NextPower2Index(T val)
    {
		unsigned short index = Power2Index(val);
		if (!IsPowerOf2(val))
		{
			++index;
		}
        return index;
    }
};

#endif
