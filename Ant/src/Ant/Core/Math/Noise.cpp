#include "antpch.h"
#include "Ant/Core/Math/Noise.h"

#include "Cpp/FastNoiseLite.h"

namespace Ant{

	static FastNoiseLite s_FastNoise;

	float Noise::PerlinNoise(float x, float y)
	{
		s_FastNoise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
		return s_FastNoise.GetNoise(x, y);	// This returns a value between -1 and 1
	}
}