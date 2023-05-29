using System;

namespace Ant
{
    public static class Mathf
    {

        public const float Epsilon = 0.00001f;
        public const float PI = (float)Math.PI;
        public const float PIonTwo = (float)(Math.PI / 2.0f);
        public const float TwoPI = (float)(Math.PI * 2.0);

        public const float Deg2Rad = PI / 180.0f;
        public const float Rad2Deg = 180.0f / PI;

        public static float Sin(float value) => (float)Math.Sin(value);
        public static float Cos(float value) => (float)Math.Cos(value);
        public static float Acos(float value) => (float)Math.Acos(value);

        public static float Clamp(float value, float min, float max)
        {
            if (value < min)
                return min;
            return value > max ? max : value;
        }

        public static float Asin(float x) => (float)Math.Asin(x);
        public static float Atan(float x) => (float)Math.Atan(x);
        public static float Atan2(float y, float x) => (float)Math.Atan2(y, x);

        public static float Min(float v0, float v1) => v0 < v1 ? v0 : v1;
        public static float Max(float v0, float v1) => v0 > v1 ? v0 : v1;

        public static float Sqrt(float value) => (float)Math.Sqrt(value);

        public static float Abs(float value) => Math.Abs(value);
        public static int Abs(int value) => Math.Abs(value);


        public static Vector3 Abs(Vector3 value)
        {
            return new Vector3(Math.Abs(value.X), Math.Abs(value.Y), Math.Abs(value.Z));
        }

        public static float Lerp(float p1, float p2, float t) => Interpolate.Linear(p1, p2, t);
        public static Vector3 Lerp(Vector3 p1, Vector3 p2, float t) => Interpolate.Linear(p1, p2, t);

        public static float Floor(float value) => (float)Math.Floor(value);

        // not the same as a%b
        public static float Modulo(float a, float b) => a - b * (float)Math.Floor(a / b);

        public static float Distance(float p1, float p2) => Abs(p1 - p2);

        public static float PerlinNoise(float x, float y) => Noise.Perlin(x, y);

        public static int CeilToInt(float value) => (int)Math.Ceiling(value);
        public static int FloorToInt(float value) => (int)Math.Floor(value);
    }
}