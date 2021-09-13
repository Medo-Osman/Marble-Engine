#ifndef MATHUTILITIES_H
#define MATHUTILITIES_H

#include <stdlib.h>
#include <ctime>

static float frand(float low, float high)
{
	return low + static_cast<float> (rand()) / (static_cast<float> (RAND_MAX / (high - low)));
}

static float lerpF(float from, float to, float weight)
{
	return from + weight * (to - from);
}

#endif // !MATHUTILITIES_H