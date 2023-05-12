﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace Ant
{
    [StructLayout(LayoutKind.Sequential)]
    public struct Vector3
    {
        public float X;
        public float Y;
        public float Z;

        public Vector3(float value)
        {
            X = Y = Z = value;
        }

        public Vector3(float x, float y, float z)
        {
            X = x;
            Y = y;
            Z = z;
        }

        public Vector3(Vector2 vector)
        {
            X = vector.X;
            Y = vector.Y;
            Z = 0.0f;
        }

        public Vector3(Vector4 vector)
        {
            X = vector.X;
            Y = vector.Y;
            Z = vector.Z;
        }

        public Vector2 XY
        {
            get => new Vector2(X, Y);
            set { X = value.X; Y = value.Y; }
        }
        public Vector2 XZ
        {
            get => new Vector2(X, Z);
            set { X = value.X; Z = value.Y; }
        }
        public Vector2 YZ
        {
            get => new Vector2(Y, Z);
            set { Y = value.X; Z = value.Y; }
        }
    }
}
