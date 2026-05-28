#ifndef MBASE_H
#define MBASE_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>


#ifndef MATH_PI
#define MATH_PI 3.1415926f
#endif
#ifndef min
#define min(x, y) ((x) < (y) ? (x) : (y))
#endif
#ifndef max
#define max(x, y) ((x) > (y) ? (x) : (y))
#endif
#ifndef clamp
#define clamp(x, min, max) ((x) < (min) ? (min) : (x) > (max) ? (max) : (x))
#endif

#endif
