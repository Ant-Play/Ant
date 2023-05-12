using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading.Tasks;

namespace Ant
{
    public static class Noise
    {
        public static float PerlinNoise(float x, float y)
        {
            return PerlinNoise_Native(x, y);
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern float PerlinNoise_Native(float x, float y);
    }
}
