#include "mc.h"

#if defined(__aarch64__) || defined(__arm__)
#include "mc_c.hpp"
#else
#include "mc_nsse2.hpp"
#endif