#pragma once

#if defined(__GNUC__) || defined(__clang__)
#  define ALIGNED(x) __attribute__ ((aligned(x)))
#elif defined(_MSC_VER)
#  define ALIGNED(x) __declspec(align(x))
#else
#  error "Unknown compiler; can't define ALIGN"
#endif
