#include "antpch.h"
#include "Noise.h"

#include "Cpp/FastNoiseLite.h"

namespace Ant{

	static FastNoiseLite s_FastNoise;

	float Noise::PerlinNoise(float x, float y)
	{
		s_FastNoise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
		float result = s_FastNoise.GetNoise(x, y); // This returns a value between -1 and 1
		return result;
	}
}